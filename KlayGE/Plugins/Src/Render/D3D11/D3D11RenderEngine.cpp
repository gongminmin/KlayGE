// D3D11RenderEngine.cpp
// KlayGE D3D11渲染引擎类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2009-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 升级到DXGI 1.1 (2010.2.8)
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KFL/Hash.hpp>

#include <KlayGE/D3D11/D3D11Adapter.hpp>
#include <KlayGE/D3D11/D3D11RenderWindow.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>

#include <algorithm>
#include <functional>

#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include "NV3DVision.hpp"

namespace
{
	using namespace KlayGE;

	static std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11ShaderResourceView * const *)> const ShaderSetShaderResources[] =
	{
		std::mem_fn(&ID3D11DeviceContext::VSSetShaderResources),
		std::mem_fn(&ID3D11DeviceContext::PSSetShaderResources),
		std::mem_fn(&ID3D11DeviceContext::GSSetShaderResources),
		std::mem_fn(&ID3D11DeviceContext::CSSetShaderResources),
		std::mem_fn(&ID3D11DeviceContext::HSSetShaderResources),
		std::mem_fn(&ID3D11DeviceContext::DSSetShaderResources)
	};
	KLAYGE_STATIC_ASSERT(std::size(ShaderSetShaderResources) == ShaderObject::ST_NumShaderTypes);

	static std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11SamplerState * const *)> const ShaderSetSamplers[] =
	{
		std::mem_fn(&ID3D11DeviceContext::VSSetSamplers),
		std::mem_fn(&ID3D11DeviceContext::PSSetSamplers),
		std::mem_fn(&ID3D11DeviceContext::GSSetSamplers),
		std::mem_fn(&ID3D11DeviceContext::CSSetSamplers),
		std::mem_fn(&ID3D11DeviceContext::HSSetSamplers),
		std::mem_fn(&ID3D11DeviceContext::DSSetSamplers)
	};
	KLAYGE_STATIC_ASSERT(std::size(ShaderSetSamplers) == ShaderObject::ST_NumShaderTypes);

	static std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11Buffer * const *)> const ShaderSetConstantBuffers[] =
	{
		std::mem_fn(&ID3D11DeviceContext::VSSetConstantBuffers),
		std::mem_fn(&ID3D11DeviceContext::PSSetConstantBuffers),
		std::mem_fn(&ID3D11DeviceContext::GSSetConstantBuffers),
		std::mem_fn(&ID3D11DeviceContext::CSSetConstantBuffers),
		std::mem_fn(&ID3D11DeviceContext::HSSetConstantBuffers),
		std::mem_fn(&ID3D11DeviceContext::DSSetConstantBuffers)
	};
	KLAYGE_STATIC_ASSERT(std::size(ShaderSetConstantBuffers) == ShaderObject::ST_NumShaderTypes);
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D11RenderEngine::D3D11RenderEngine()
		: num_so_buffs_(0),
			dsv_ptr_cache_(nullptr),
			inv_timestamp_freq_(0),
			device_lost_event_(nullptr), device_lost_reg_cookie_(0), thread_pool_wait_(nullptr)
	{
		native_shader_fourcc_ = MakeFourCC<'D', 'X', 'B', 'C'>::value;
		native_shader_version_ = 5;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		// Dynamic loading because these dlls can't be loaded on WinXP
		mod_dxgi_ = ::LoadLibraryEx(TEXT("dxgi.dll"), nullptr, 0);
		if (nullptr == mod_dxgi_)
		{
			::MessageBoxW(nullptr, L"Can't load dxgi.dll", L"Error", MB_OK);
		}
		mod_d3d11_ = ::LoadLibraryEx(TEXT("d3d11.dll"), nullptr, 0);
		if (nullptr == mod_d3d11_)
		{
			::MessageBoxW(nullptr, L"Can't load d3d11.dll", L"Error", MB_OK);
		}

		if (mod_dxgi_ != nullptr)
		{
			DynamicCreateDXGIFactory1_ = reinterpret_cast<CreateDXGIFactory1Func>(::GetProcAddress(mod_dxgi_, "CreateDXGIFactory1"));
		}
		if (mod_d3d11_ != nullptr)
		{
			DynamicD3D11CreateDevice_ = reinterpret_cast<D3D11CreateDeviceFunc>(::GetProcAddress(mod_d3d11_, "D3D11CreateDevice"));
		}
#else
		DynamicCreateDXGIFactory1_ = ::CreateDXGIFactory1;
		DynamicD3D11CreateDevice_ = ::D3D11CreateDevice;
#endif

		IDXGIFactory1* gi_factory;
		TIFHR(DynamicCreateDXGIFactory1_(IID_IDXGIFactory1, reinterpret_cast<void**>(&gi_factory)));
		gi_factory_1_ = MakeCOMPtr(gi_factory);
		dxgi_sub_ver_ = 1;

		IDXGIFactory2* gi_factory2;
		gi_factory->QueryInterface(IID_IDXGIFactory2, reinterpret_cast<void**>(&gi_factory2));
		if (gi_factory2 != nullptr)
		{
			gi_factory_2_ = MakeCOMPtr(gi_factory2);
			dxgi_sub_ver_ = 2;

			IDXGIFactory3* gi_factory3;
			gi_factory->QueryInterface(IID_IDXGIFactory3, reinterpret_cast<void**>(&gi_factory3));
			if (gi_factory3 != nullptr)
			{
				gi_factory_3_ = MakeCOMPtr(gi_factory3);
				dxgi_sub_ver_ = 3;

				IDXGIFactory4* gi_factory4;
				gi_factory->QueryInterface(IID_IDXGIFactory4, reinterpret_cast<void**>(&gi_factory4));
				if (gi_factory4 != nullptr)
				{
					gi_factory_4_ = MakeCOMPtr(gi_factory4);
					dxgi_sub_ver_ = 4;

					IDXGIFactory5* gi_factory5;
					gi_factory->QueryInterface(IID_IDXGIFactory5, reinterpret_cast<void**>(&gi_factory5));
					if (gi_factory5 != nullptr)
					{
						gi_factory_5_ = MakeCOMPtr(gi_factory5);
						dxgi_sub_ver_ = 5;
					}
				}
			}
		}

		adapterList_.Enumerate(gi_factory_1_);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D11RenderEngine::~D3D11RenderEngine()
	{
		this->Destroy();
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & D3D11RenderEngine::Name() const
	{
		static std::wstring const name(L"Direct3D11 Render Engine");
		return name;
	}

	void D3D11RenderEngine::BeginFrame()
	{
		if (Context::Instance().Config().perf_profiler)
		{
			d3d_imm_ctx_->Begin(timestamp_disjoint_query_.get());
		}

		RenderEngine::BeginFrame();
	}

	void D3D11RenderEngine::EndFrame()
	{
		RenderEngine::EndFrame();

		if (Context::Instance().Config().perf_profiler)
		{
			d3d_imm_ctx_->End(timestamp_disjoint_query_.get());
		}
	}

	void D3D11RenderEngine::UpdateGPUTimestampsFrequency()
	{
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint;
		while (S_OK != d3d_imm_ctx_->GetData(timestamp_disjoint_query_.get(), &disjoint, sizeof(disjoint), 0));
		inv_timestamp_freq_ = disjoint.Disjoint ? 0 : (1.0 / disjoint.Frequency);
	}

	IDXGIFactory1* D3D11RenderEngine::DXGIFactory1() const
	{
		return gi_factory_1_.get();
	}

	IDXGIFactory2* D3D11RenderEngine::DXGIFactory2() const
	{
		return gi_factory_2_.get();
	}

	IDXGIFactory3* D3D11RenderEngine::DXGIFactory3() const
	{
		return gi_factory_3_.get();
	}

	IDXGIFactory4* D3D11RenderEngine::DXGIFactory4() const
	{
		return gi_factory_4_.get();
	}

	IDXGIFactory5* D3D11RenderEngine::DXGIFactory5() const
	{
		return gi_factory_5_.get();
	}

	uint8_t D3D11RenderEngine::DXGISubVer() const
	{
		return dxgi_sub_ver_;
	}

	ID3D11Device* D3D11RenderEngine::D3DDevice() const
	{
		return d3d_device_.get();
	}

	ID3D11Device1* D3D11RenderEngine::D3DDevice1() const
	{
		return d3d_device_1_.get();
	}

	ID3D11Device2* D3D11RenderEngine::D3DDevice2() const
	{
		return d3d_device_2_.get();
	}

	ID3D11Device3* D3D11RenderEngine::D3DDevice3() const
	{
		return d3d_device_3_.get();
	}

	ID3D11Device4* D3D11RenderEngine::D3DDevice4() const
	{
		return d3d_device_4_.get();
	}

	ID3D11DeviceContext* D3D11RenderEngine::D3DDeviceImmContext() const
	{
		return d3d_imm_ctx_.get();
	}

	ID3D11DeviceContext1* D3D11RenderEngine::D3DDeviceImmContext1() const
	{
		return d3d_imm_ctx_1_.get();
	}

	ID3D11DeviceContext2* D3D11RenderEngine::D3DDeviceImmContext2() const
	{
		return d3d_imm_ctx_2_.get();
	}

	ID3D11DeviceContext3* D3D11RenderEngine::D3DDeviceImmContext3() const
	{
		return d3d_imm_ctx_3_.get();
	}

	uint8_t D3D11RenderEngine::D3D11RuntimeSubVer() const
	{
		return d3d_11_runtime_sub_ver_;
	}

	D3D_FEATURE_LEVEL D3D11RenderEngine::DeviceFeatureLevel() const
	{
		return d3d_feature_level_;
	}

	// 获取D3D适配器列表
	/////////////////////////////////////////////////////////////////////////////////
	D3D11AdapterList const & D3D11RenderEngine::D3DAdapters() const
	{
		return adapterList_;
	}

	// 获取当前适配器
	/////////////////////////////////////////////////////////////////////////////////
	D3D11Adapter& D3D11RenderEngine::ActiveAdapter() const
	{
		return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		D3D11RenderWindowPtr win = MakeSharedPtr<D3D11RenderWindow>(&this->ActiveAdapter(),
			name, settings);

		native_shader_platform_name_ = "d3d_11_0";
		switch (d3d_feature_level_)
		{
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			vs_profile_ = "vs_5_0";
			ps_profile_ = "ps_5_0";
			gs_profile_ = "gs_5_0";
			cs_profile_ = "cs_5_0";
			hs_profile_ = "hs_5_0";
			ds_profile_ = "ds_5_0";
			break;

		default:
			KFL_UNREACHABLE("Invalid feature level");
		}

		this->ResetRenderStates();
		this->BindFrameBuffer(win);

		if (STM_LCDShutter == settings.stereo_method)
		{
			stereo_method_ = SM_None;

			if ((dxgi_sub_ver_ >= 2) && gi_factory_2_->IsWindowedStereoEnabled())
			{
				stereo_method_ = SM_DXGI;
			}

			if (SM_None == stereo_method_)
			{
				if (win->Adapter().Description().find(L"NVIDIA", 0) != std::wstring::npos)
				{
					stereo_method_ = SM_NV3DVision;

					RenderFactory& rf = Context::Instance().RenderFactoryInstance();

					uint32_t const w = win->Width();
					uint32_t const h = win->Height();
					stereo_nv_3d_vision_tex_ = rf.MakeTexture2D(w * 2, h + 1, 1, 1,
						settings.color_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

					stereo_nv_3d_vision_fb_ = rf.MakeFrameBuffer();
					stereo_nv_3d_vision_fb_->Attach(FrameBuffer::ATT_Color0,
						rf.Make2DRenderView(*stereo_nv_3d_vision_tex_, 0, 1, 0));

					NVSTEREOIMAGEHEADER sih;
					sih.dwSignature = NVSTEREO_IMAGE_SIGNATURE;
					sih.dwBPP = NumFormatBits(settings.color_fmt);
					sih.dwFlags = SIH_SWAP_EYES;
					sih.dwWidth = w * 2; 
					sih.dwHeight = h;

					ElementInitData init_data;
					init_data.data = &sih;
					init_data.row_pitch = sizeof(sih);
					init_data.slice_pitch = init_data.row_pitch;
					TexturePtr sih_tex = rf.MakeTexture2D(sizeof(sih) / NumFormatBytes(settings.color_fmt),
						1, 1, 1, settings.color_fmt, 1, 0, EAH_GPU_Read, init_data);

					sih_tex->CopyToSubTexture2D(*stereo_nv_3d_vision_tex_,
						0, 0, 0, h, sih_tex->Width(0), 1,
						0, 0, 0, 0, sih_tex->Width(0), 1);
				}
				else if (win->Adapter().Description().find(L"AMD", 0) != std::wstring::npos)
				{
					stereo_method_ = SM_AMDQuadBuffer;
				}
			}
		}

		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		desc.MiscFlags = 0;

		ID3D11Query* disjoint_query;
		d3d_device_->CreateQuery(&desc, &disjoint_query);
		timestamp_disjoint_query_ = MakeCOMPtr(disjoint_query);
	}

	void D3D11RenderEngine::CheckConfig(RenderSettings& settings)
	{
		KFL_UNUSED(settings);
	}

	void D3D11RenderEngine::D3DDevice(ID3D11Device* device, ID3D11DeviceContext* imm_ctx, D3D_FEATURE_LEVEL feature_level)
	{
		this->DetectD3D11Runtime(device, imm_ctx);

		d3d_feature_level_ = feature_level;
		Verify(device != nullptr);

		this->FillRenderDeviceCaps();

		if (d3d_11_runtime_sub_ver_ >= 4)
		{
			device_lost_event_ = ::CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
			if (device_lost_event_ != nullptr)
			{
				thread_pool_wait_ = ::CreateThreadpoolWait(D3D11RenderEngine::OnDeviceLost, this, nullptr);
				if (thread_pool_wait_ != nullptr)
				{
					::SetThreadpoolWait(thread_pool_wait_, device_lost_event_, nullptr);
					TIFHR(d3d_device_4_->RegisterDeviceRemovedEvent(device_lost_event_, &device_lost_reg_cookie_));
				}
			}
		}
	}

	void D3D11RenderEngine::ResetRenderStates()
	{
		RasterizerStateDesc default_rs_desc;
		DepthStencilStateDesc default_dss_desc;
		BlendStateDesc default_bs_desc;

		vertex_shader_cache_ = nullptr;
		pixel_shader_cache_ = nullptr;
		geometry_shader_cache_ = nullptr;
		compute_shader_cache_ = nullptr;
		hull_shader_cache_ = nullptr;
		domain_shader_cache_ = nullptr;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRenderStateObject(default_rs_desc, default_dss_desc, default_bs_desc);

		auto d3d_cur_rs_obj = checked_cast<D3D11RenderStateObject*>(cur_rs_obj_.get());
		rasterizer_state_cache_ = d3d_cur_rs_obj->D3DRasterizerState();
		depth_stencil_state_cache_ = d3d_cur_rs_obj->D3DDepthStencilState();
		stencil_ref_cache_ = 0;
		blend_state_cache_ = d3d_cur_rs_obj->D3DBlendState();
		blend_factor_cache_ = Color(1, 1, 1, 1);
		sample_mask_cache_ = 0xFFFFFFFF;

		d3d_imm_ctx_->RSSetState(rasterizer_state_cache_);
		d3d_imm_ctx_->OMSetDepthStencilState(depth_stencil_state_cache_, stencil_ref_cache_);
		d3d_imm_ctx_->OMSetBlendState(blend_state_cache_, &blend_factor_cache_.r(), sample_mask_cache_);

		topology_type_cache_ = RenderLayout::TT_PointList;
		d3d_imm_ctx_->IASetPrimitiveTopology(D3D11Mapping::Mapping(topology_type_cache_));

		input_layout_cache_ = nullptr;
		d3d_imm_ctx_->IASetInputLayout(input_layout_cache_);

		memset(&viewport_cache_, 0, sizeof(viewport_cache_));

		if (!vb_cache_.empty())
		{
			vb_cache_.assign(vb_cache_.size(), nullptr);
			vb_stride_cache_.assign(vb_stride_cache_.size(), 0);
			vb_offset_cache_.assign(vb_offset_cache_.size(), 0);
			d3d_imm_ctx_->IASetVertexBuffers(0, static_cast<UINT>(vb_cache_.size()),
				&vb_cache_[0], &vb_stride_cache_[0], &vb_offset_cache_[0]);
			vb_cache_.clear();
			vb_stride_cache_.clear();
			vb_offset_cache_.clear();
		}

		ib_cache_ = nullptr;
		d3d_imm_ctx_->IASetIndexBuffer(ib_cache_, DXGI_FORMAT_R16_UINT, 0);

		for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			if (!shader_srv_ptr_cache_[i].empty())
			{
				std::fill(shader_srv_ptr_cache_[i].begin(), shader_srv_ptr_cache_[i].end(), static_cast<ID3D11ShaderResourceView*>(nullptr));
				ShaderSetShaderResources[i](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_srv_ptr_cache_[i].size()), &shader_srv_ptr_cache_[i][0]);
				shader_srvsrc_cache_[i].clear();
				shader_srv_ptr_cache_[i].clear();
			}

			if (!shader_sampler_ptr_cache_[i].empty())
			{
				std::fill(shader_sampler_ptr_cache_[i].begin(), shader_sampler_ptr_cache_[i].end(), static_cast<ID3D11SamplerState*>(nullptr));
				ShaderSetSamplers[i](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_sampler_ptr_cache_[i].size()), &shader_sampler_ptr_cache_[i][0]);
				shader_sampler_ptr_cache_[i].clear();
			}

			if (!shader_cb_ptr_cache_[i].empty())
			{
				std::fill(shader_cb_ptr_cache_[i].begin(), shader_cb_ptr_cache_[i].end(), static_cast<ID3D11Buffer*>(nullptr));
				ShaderSetConstantBuffers[i](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_cb_ptr_cache_[i].size()), &shader_cb_ptr_cache_[i][0]);
				shader_cb_ptr_cache_[i].clear();
			}
		}
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(d3d_device_);
		BOOST_ASSERT(fb);
	}

	// 设置当前Stream output目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::DoBindSOBuffers(RenderLayoutPtr const & rl)
	{
		uint32_t num_buffs = rl ? rl->NumVertexStreams() : 0;
		if (num_buffs > 0)
		{
			std::vector<void*> so_src(num_buffs, nullptr);
			std::vector<ID3D11Buffer*> d3d11_buffs(num_buffs);
			std::vector<UINT> d3d11_buff_offsets(num_buffs, 0);
			for (uint32_t i = 0; i < num_buffs; ++ i)
			{
				D3D11GraphicsBuffer* d3d11_buf = checked_cast<D3D11GraphicsBuffer*>(rl->GetVertexStream(i).get());

				so_src[i] = d3d11_buf;
				d3d11_buffs[i] = d3d11_buf->D3DBuffer();
			}

			for (uint32_t i = 0; i < num_buffs; ++ i)
			{
				if (so_src[i] != nullptr)
				{
					this->DetachSRV(so_src[i], 0, 1);
				}
			}

			d3d_imm_ctx_->SOSetTargets(static_cast<UINT>(num_buffs), &d3d11_buffs[0], &d3d11_buff_offsets[0]);

			num_so_buffs_ = num_buffs;
		}
		else if (num_so_buffs_ > 0)
		{
			std::vector<ID3D11Buffer*> d3d11_buffs(num_so_buffs_, nullptr);
			std::vector<UINT> d3d11_buff_offsets(num_so_buffs_, 0);
			d3d_imm_ctx_->SOSetTargets(static_cast<UINT>(num_so_buffs_), &d3d11_buffs[0], &d3d11_buff_offsets[0]);

			num_so_buffs_ = num_buffs;
		}
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t const num_vertex_streams = rl.NumVertexStreams();
		uint32_t const all_num_vertex_stream = num_vertex_streams + (rl.InstanceStream() ? 1 : 0);

		D3D11RenderLayout const & d3d_rl = *checked_cast<D3D11RenderLayout const *>(&rl);
		d3d_rl.Active();

		auto const & vbs = d3d_rl.VBs();
		auto const & strides = d3d_rl.Strides();
		auto const & offsets = d3d_rl.Offsets();
		if (all_num_vertex_stream != 0)
		{
			if ((vb_cache_.size() != all_num_vertex_stream) || (vb_cache_ != vbs)
				|| (vb_stride_cache_ != strides) || (vb_offset_cache_ != offsets))
			{
				d3d_imm_ctx_->IASetVertexBuffers(0, all_num_vertex_stream, &vbs[0], &strides[0], &offsets[0]);
				vb_cache_ = vbs;
				vb_stride_cache_ = strides;
				vb_offset_cache_ = offsets;
			}

			auto layout = d3d_rl.InputLayout(tech.Pass(0).GetShaderObject(effect).get());
			if (layout != input_layout_cache_)
			{
				d3d_imm_ctx_->IASetInputLayout(layout);
				input_layout_cache_ = layout;
			}
		}
		else
		{
			if (!vb_cache_.empty())
			{
				vb_cache_.assign(vb_cache_.size(), nullptr);
				vb_stride_cache_.assign(vb_stride_cache_.size(), 0);
				vb_offset_cache_.assign(vb_offset_cache_.size(), 0);
				d3d_imm_ctx_->IASetVertexBuffers(0, static_cast<UINT>(vb_cache_.size()),
					&vb_cache_[0], &vb_stride_cache_[0], &vb_offset_cache_[0]);
				vb_cache_.clear();
				vb_stride_cache_.clear();
				vb_offset_cache_.clear();
			}

			input_layout_cache_ = nullptr;
			d3d_imm_ctx_->IASetInputLayout(input_layout_cache_);
		}

		uint32_t const vertex_count = static_cast<uint32_t>(rl.UseIndices() ? rl.NumIndices() : rl.NumVertices());

		RenderLayout::topology_type tt = rl.TopologyType();
		if (tech.HasTessellation())
		{
			switch (tt)
			{
			case RenderLayout::TT_PointList:
				tt = RenderLayout::TT_1_Ctrl_Pt_PatchList;
				break;

			case RenderLayout::TT_LineList:
				tt = RenderLayout::TT_2_Ctrl_Pt_PatchList;
				break;

			case RenderLayout::TT_TriangleList:
				tt = RenderLayout::TT_3_Ctrl_Pt_PatchList;
				break;

			default:
				break;
			}
		}
		if (topology_type_cache_ != tt)
		{
			d3d_imm_ctx_->IASetPrimitiveTopology(D3D11Mapping::Mapping(tt));
			topology_type_cache_ = tt;
		}

		uint32_t prim_count;
		switch (tt)
		{
		case RenderLayout::TT_PointList:
			prim_count = vertex_count;
			break;

		case RenderLayout::TT_LineList:
		case RenderLayout::TT_LineList_Adj:
			prim_count = vertex_count / 2;
			break;

		case RenderLayout::TT_LineStrip:
		case RenderLayout::TT_LineStrip_Adj:
			prim_count = vertex_count - 1;
			break;

		case RenderLayout::TT_TriangleList:
		case RenderLayout::TT_TriangleList_Adj:
			prim_count = vertex_count / 3;
			break;

		case RenderLayout::TT_TriangleStrip:
		case RenderLayout::TT_TriangleStrip_Adj:
			prim_count = vertex_count - 2;
			break;

		default:
			if ((tt >= RenderLayout::TT_1_Ctrl_Pt_PatchList)
				&& (tt <= RenderLayout::TT_32_Ctrl_Pt_PatchList))
			{
				prim_count = vertex_count / (tt - RenderLayout::TT_1_Ctrl_Pt_PatchList + 1);
			}
			else
			{
				KFL_UNREACHABLE("Invalid topology type");
			}
			break;
		}

		uint32_t const num_instances = rl.NumInstances();

		num_primitives_just_rendered_ += num_instances * prim_count;
		num_vertices_just_rendered_ += num_instances * vertex_count;

		if (rl.UseIndices())
		{
			ID3D11Buffer* d3dib = checked_cast<D3D11GraphicsBuffer*>(rl.GetIndexStream().get())->D3DBuffer();
			if (ib_cache_ != d3dib)
			{
				d3d_imm_ctx_->IASetIndexBuffer(d3dib, D3D11Mapping::MappingFormat(rl.IndexStreamFormat()), 0);
				ib_cache_ = d3dib;
			}
		}
		else
		{
			if (ib_cache_)
			{
				d3d_imm_ctx_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
				ib_cache_ = nullptr;
			}
		}

		uint32_t const num_passes = tech.NumPasses();
		GraphicsBuffer const * indirect_buff = rl.GetIndirectArgs().get();
		if (indirect_buff)
		{
			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);
					d3d_imm_ctx_->DrawIndexedInstancedIndirect(
						checked_cast<D3D11GraphicsBuffer const *>(indirect_buff)->D3DBuffer(),
						rl.IndirectArgsOffset());
					pass.Unbind(effect);
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);
					d3d_imm_ctx_->DrawInstancedIndirect(
						checked_cast<D3D11GraphicsBuffer const *>(indirect_buff)->D3DBuffer(),
						rl.IndirectArgsOffset());
					pass.Unbind(effect);
				}
			}
		}
		else
		{
			if (rl.UseIndices())
			{
				uint32_t const num_indices = rl.NumIndices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);
					d3d_imm_ctx_->DrawIndexedInstanced(num_indices, num_instances, rl.StartIndexLocation(), rl.StartVertexLocation(), rl.StartInstanceLocation());
					pass.Unbind(effect);
				}
			}
			else
			{
				uint32_t const num_vertices = rl.NumVertices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);
					d3d_imm_ctx_->DrawInstanced(num_vertices, num_instances, rl.StartVertexLocation(), rl.StartInstanceLocation());
					pass.Unbind(effect);
				}
			}
		}

		num_draws_just_called_ += num_passes;
	}

	void D3D11RenderEngine::DoDispatch(RenderEffect const & effect, RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++ i)
		{
			auto& pass = tech.Pass(i);

			pass.Bind(effect);
			d3d_imm_ctx_->Dispatch(tgx, tgy, tgz);
			pass.Unbind(effect);
		}

		num_dispatches_just_called_ += num_passes;
	}

	void D3D11RenderEngine::DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
		GraphicsBufferPtr const & buff_args, uint32_t offset)
	{
		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++ i)
		{
			auto& pass = tech.Pass(i);

			pass.Bind(effect);
			d3d_imm_ctx_->DispatchIndirect(checked_cast<D3D11GraphicsBuffer*>(buff_args.get())->D3DBuffer(), offset);
			pass.Unbind(effect);
		}

		num_dispatches_just_called_ += num_passes;
	}

	void D3D11RenderEngine::ForceFlush()
	{
		d3d_imm_ctx_->Flush();
	}

	TexturePtr const & D3D11RenderEngine::ScreenDepthStencilTexture() const
	{
		return checked_cast<D3D11RenderWindow*>(screen_frame_buffer_.get())->D3DDepthStencilBuffer();
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		D3D11_RECT rc = { static_cast<LONG>(x), static_cast<LONG>(y),
			static_cast<LONG>(width), static_cast<LONG>(height) };
		d3d_imm_ctx_->RSSetScissorRects(1, &rc);
	}

	void D3D11RenderEngine::GetCustomAttrib(std::string_view name, void* value) const
	{
		size_t const name_hash = HashRange(name.begin(), name.end());
		if (CT_HASH("D3D_DEVICE") == name_hash)
		{
			*static_cast<ID3D11Device**>(value) = d3d_device_.get();
		}
		else if (CT_HASH("D3D_IMM_CONTEXT") == name_hash)
		{
			*static_cast<ID3D11DeviceContext**>(value) = d3d_imm_ctx_.get();
		}
		else if (CT_HASH("FEATURE_LEVEL") == name_hash)
		{
			*static_cast<D3D_FEATURE_LEVEL*>(value) = d3d_feature_level_;
		}
		else if (CT_HASH("DXGI_FACTORY") == name_hash)
		{
			*static_cast<IDXGIFactory1**>(value) = gi_factory_1_.get();
		}
	}

	void D3D11RenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_cast<D3D11RenderWindow*>(screen_frame_buffer_.get())->Resize(width, height);
	}

	void D3D11RenderEngine::DoDestroy()
	{
		if ((device_lost_reg_cookie_ != 0) && d3d_device_4_)
		{
			d3d_device_4_->UnregisterDeviceRemoved(device_lost_reg_cookie_);
			device_lost_reg_cookie_ = 0;
		}
		if (device_lost_event_ != nullptr)
		{
			::CloseHandle(device_lost_event_);
			device_lost_event_ = nullptr;
		}
		if (thread_pool_wait_ != nullptr)
		{
			::CloseThreadpoolWait(thread_pool_wait_);
			thread_pool_wait_ = nullptr;
		}

		adapterList_.Destroy();

		rasterizer_state_cache_ = nullptr;
		depth_stencil_state_cache_ = nullptr;
		blend_state_cache_ = nullptr;
		vertex_shader_cache_ = nullptr;
		pixel_shader_cache_ = nullptr;
		geometry_shader_cache_ = nullptr;
		compute_shader_cache_ = nullptr;
		hull_shader_cache_ = nullptr;
		domain_shader_cache_ = nullptr;
		input_layout_cache_ = nullptr;
		vb_cache_.clear();
		ib_cache_ = nullptr;

		for (size_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			shader_srvsrc_cache_[i].clear();
			shader_srv_ptr_cache_[i].clear();
			shader_sampler_ptr_cache_[i].clear();
			shader_cb_ptr_cache_[i].clear();
		}
		render_uav_ptr_cache_.clear();
		render_uav_init_count_cache_.clear();
		compute_uav_ptr_cache_.clear();
		compute_uav_init_count_cache_.clear();
		rtv_ptr_cache_.clear();
		dsv_ptr_cache_ = nullptr;

		stereo_nv_3d_vision_fb_.reset();
		stereo_nv_3d_vision_tex_.reset();

		vertex_format_.clear();
		texture_format_.clear();
		rendertarget_format_.clear();

		timestamp_disjoint_query_.reset();
		inv_timestamp_freq_ = 0;

		d3d_imm_ctx_->ClearState();
		d3d_imm_ctx_->Flush();

		d3d_imm_ctx_.reset();
		d3d_imm_ctx_1_.reset();
		d3d_imm_ctx_2_.reset();
		d3d_imm_ctx_3_.reset();
		d3d_device_.reset();
		d3d_device_1_.reset();
		d3d_device_2_.reset();
		d3d_device_3_.reset();
		d3d_device_4_.reset();
		gi_factory_1_.reset();
		gi_factory_2_.reset();
		gi_factory_3_.reset();
		gi_factory_4_.reset();
		gi_factory_5_.reset();

		DynamicCreateDXGIFactory1_ = nullptr;
		DynamicD3D11CreateDevice_ = nullptr;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		::FreeLibrary(mod_d3d11_);
		::FreeLibrary(mod_dxgi_);
#endif
	}

	void D3D11RenderEngine::DoSuspend()
	{
		if (d3d_11_runtime_sub_ver_ >= 2)
		{
			IDXGIDevice3* dxgi_device = nullptr;
			d3d_device_->QueryInterface(IID_IDXGIDevice3, reinterpret_cast<void**>(&dxgi_device));
			if (dxgi_device != nullptr)
			{
				dxgi_device->Trim();
				dxgi_device->Release();
			}
		}
	}

	void D3D11RenderEngine::DoResume()
	{
		// TODO
	}

	bool D3D11RenderEngine::FullScreen() const
	{
		return checked_cast<D3D11RenderWindow*>(screen_frame_buffer_.get())->FullScreen();
	}

	void D3D11RenderEngine::FullScreen(bool fs)
	{
		checked_cast<D3D11RenderWindow*>(screen_frame_buffer_.get())->FullScreen(fs);
	}

	bool D3D11RenderEngine::VertexFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(vertex_format_.begin(), vertex_format_.end(), elem_fmt);
		return (iter != vertex_format_.end()) && (*iter == elem_fmt);
	}

	bool D3D11RenderEngine::TextureFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(texture_format_.begin(), texture_format_.end(), elem_fmt);
		return (iter != texture_format_.end()) && (*iter == elem_fmt);
	}

	bool D3D11RenderEngine::RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality)
	{
		auto iter = rendertarget_format_.find(elem_fmt);
		if (iter != rendertarget_format_.end())
		{
			for (auto const & p : iter->second)
			{
				if ((sample_count == p.first) && (sample_quality < p.second))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool D3D11RenderEngine::UAVFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(uav_format_.begin(), uav_format_.end(), elem_fmt);
		return (iter != uav_format_.end()) && (*iter == elem_fmt);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::FillRenderDeviceCaps()
	{
		BOOST_ASSERT(d3d_device_);

		switch (d3d_feature_level_)
		{
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			caps_.max_shader_model
				= (d3d_feature_level_ > D3D_FEATURE_LEVEL_12_0) ? ShaderModel(5, 1) : ShaderModel(5, 0);
			caps_.max_texture_width = caps_.max_texture_height = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
			caps_.max_texture_depth = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps_.max_texture_cube_size = D3D11_REQ_TEXTURECUBE_DIMENSION;
			caps_.max_texture_array_length = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
			caps_.max_vertex_texture_units = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps_.max_pixel_texture_units = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps_.max_geometry_texture_units = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps_.max_simultaneous_rts = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
			caps_.max_simultaneous_uavs = D3D11_PS_CS_UAV_REGISTER_COUNT;
			caps_.cs_support = true;
			caps_.tess_method = TM_Hardware;
			caps_.max_vertex_streams = D3D11_STANDARD_VERTEX_ELEMENT_COUNT;
			caps_.max_texture_anisotropy = D3D11_MAX_MAXANISOTROPY;
			break;

		default:
			KFL_UNREACHABLE("Invalid feature level");
		}

		if (d3d_11_runtime_sub_ver_ >= 1)
		{
			D3D11_FEATURE_DATA_ARCHITECTURE_INFO arch_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_ARCHITECTURE_INFO, &arch_feature, sizeof(arch_feature));
			caps_.is_tbdr = arch_feature.TileBasedDeferredRenderer ? true : false;
		}
		else
		{
			caps_.is_tbdr = false;
		}
		if (d3d_11_runtime_sub_ver_ >= 2)
		{
			D3D11_FEATURE_DATA_D3D9_SIMPLE_INSTANCING_SUPPORT d3d11_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_D3D9_SIMPLE_INSTANCING_SUPPORT, &d3d11_feature, sizeof(d3d11_feature));
			caps_.hw_instancing_support = d3d11_feature.SimpleInstancingSupported ? true : false;
		}
		else
		{
			caps_.hw_instancing_support = true;
		}
		caps_.instance_id_support = true;
		caps_.stream_output_support = true;
		caps_.alpha_to_coverage_support = true;
		caps_.primitive_restart_support = true;
		{
			D3D11_FEATURE_DATA_THREADING mt_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_THREADING, &mt_feature, sizeof(mt_feature));
			caps_.multithread_rendering_support = mt_feature.DriverCommandLists ? true : false;
			caps_.multithread_res_creating_support = mt_feature.DriverConcurrentCreates ? true : false;
			caps_.arbitrary_multithread_rendering_support = caps_.multithread_rendering_support;
		}
		caps_.mrt_independent_bit_depths_support = true;
		if (d3d_11_runtime_sub_ver_ >= 1)
		{
			D3D11_FEATURE_DATA_D3D11_OPTIONS d3d11_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &d3d11_feature, sizeof(d3d11_feature));
			caps_.logic_op_support = d3d11_feature.OutputMergerLogicOp ? true : false;
		}
		else
		{
			caps_.logic_op_support = false;
		}
		caps_.independent_blend_support = true;
		caps_.draw_indirect_support = true;
		caps_.no_overwrite_support = true;
		if (d3d_11_runtime_sub_ver_ >= 1)
		{
			D3D11_FEATURE_DATA_D3D9_OPTIONS d3d11_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_D3D9_OPTIONS, &d3d11_feature, sizeof(d3d11_feature));
			caps_.full_npot_texture_support = d3d11_feature.FullNonPow2TextureSupport ? true : false;
		}
		else
		{
			caps_.full_npot_texture_support = false;
		}
		caps_.render_to_texture_array_support = true;
		caps_.explicit_multi_sample_support = true;
		caps_.load_from_buffer_support = true;
		caps_.uavs_at_every_stage_support = (d3d_feature_level_ >= D3D_FEATURE_LEVEL_11_1);
		if (d3d_11_runtime_sub_ver_ >= 3)
		{
			D3D11_FEATURE_DATA_D3D11_OPTIONS2 d3d11_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &d3d11_feature, sizeof(d3d11_feature));
			caps_.rovs_support = d3d11_feature.ROVsSupported ? true : false;
		}
		else
		{
			caps_.rovs_support = false;
		}
		caps_.gs_support = true;
		caps_.hs_support = true;
		caps_.ds_support = true;

		bool check_16bpp_fmts = (dxgi_sub_ver_ >= 2);
		bool check_uav_fmts = false;
		if (d3d_11_runtime_sub_ver_ >= 3)
		{
			D3D11_FEATURE_DATA_D3D11_OPTIONS2 feature_data;
			if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &feature_data, sizeof(feature_data))))
			{
				check_uav_fmts = feature_data.TypedUAVLoadAdditionalFormats ? true : false;
			}
		}

		if (!check_uav_fmts)
		{
			uav_format_.push_back(EF_R32F);
			uav_format_.push_back(EF_R32UI);
			uav_format_.push_back(EF_R32I);
		}

		std::pair<ElementFormat, DXGI_FORMAT> const fmts[] = 
		{
			std::make_pair(EF_A8, DXGI_FORMAT_A8_UNORM),
			std::make_pair(EF_R5G6B5, DXGI_FORMAT_B5G6R5_UNORM),
			std::make_pair(EF_A1RGB5, DXGI_FORMAT_B5G5R5A1_UNORM),
			std::make_pair(EF_ARGB4, DXGI_FORMAT_B4G4R4A4_UNORM),
			std::make_pair(EF_R8, DXGI_FORMAT_R8_UNORM),
			std::make_pair(EF_SIGNED_R8, DXGI_FORMAT_R8_SNORM),
			std::make_pair(EF_GR8, DXGI_FORMAT_R8G8_UNORM),
			std::make_pair(EF_SIGNED_GR8, DXGI_FORMAT_R8G8_SNORM),
			std::make_pair(EF_ARGB8, DXGI_FORMAT_B8G8R8A8_UNORM),
			std::make_pair(EF_ABGR8, DXGI_FORMAT_R8G8B8A8_UNORM),
			std::make_pair(EF_SIGNED_ABGR8, DXGI_FORMAT_R8G8B8A8_SNORM),
			std::make_pair(EF_A2BGR10, DXGI_FORMAT_R10G10B10A2_UNORM),
			std::make_pair(EF_SIGNED_A2BGR10, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM),
			std::make_pair(EF_R8UI, DXGI_FORMAT_R8_UINT),
			std::make_pair(EF_R8I, DXGI_FORMAT_R8_SINT),
			std::make_pair(EF_GR8UI, DXGI_FORMAT_R8G8_UINT),
			std::make_pair(EF_GR8I, DXGI_FORMAT_R8G8_SINT),
			std::make_pair(EF_ABGR8UI, DXGI_FORMAT_R8G8B8A8_UINT),
			std::make_pair(EF_ABGR8I, DXGI_FORMAT_R8G8B8A8_SINT),
			std::make_pair(EF_A2BGR10UI, DXGI_FORMAT_R10G10B10A2_UINT),
			std::make_pair(EF_R16, DXGI_FORMAT_R16_UNORM),
			std::make_pair(EF_SIGNED_R16, DXGI_FORMAT_R16_SNORM),
			std::make_pair(EF_GR16, DXGI_FORMAT_R16G16_UNORM),
			std::make_pair(EF_SIGNED_GR16, DXGI_FORMAT_R16G16_SNORM),
			std::make_pair(EF_ABGR16, DXGI_FORMAT_R16G16B16A16_UNORM),
			std::make_pair(EF_SIGNED_ABGR16, DXGI_FORMAT_R16G16B16A16_SNORM),
			std::make_pair(EF_R16UI, DXGI_FORMAT_R16_UINT),
			std::make_pair(EF_R16I, DXGI_FORMAT_R16_SINT),
			std::make_pair(EF_GR16UI, DXGI_FORMAT_R16G16_UINT),
			std::make_pair(EF_GR16I, DXGI_FORMAT_R16G16_SINT),
			std::make_pair(EF_ABGR16UI, DXGI_FORMAT_R16G16B16A16_UINT),
			std::make_pair(EF_ABGR16I, DXGI_FORMAT_R16G16B16A16_SINT),
			std::make_pair(EF_R32UI, DXGI_FORMAT_R32_UINT),
			std::make_pair(EF_R32I, DXGI_FORMAT_R32_SINT),
			std::make_pair(EF_GR32UI, DXGI_FORMAT_R32G32_UINT),
			std::make_pair(EF_GR32I, DXGI_FORMAT_R32G32_SINT),
			std::make_pair(EF_BGR32UI, DXGI_FORMAT_R32G32B32_UINT),
			std::make_pair(EF_BGR32I, DXGI_FORMAT_R32G32B32_SINT),
			std::make_pair(EF_ABGR32UI, DXGI_FORMAT_R32G32B32A32_UINT),
			std::make_pair(EF_ABGR32I, DXGI_FORMAT_R32G32B32A32_SINT),
			std::make_pair(EF_R16F, DXGI_FORMAT_R16_FLOAT),
			std::make_pair(EF_GR16F, DXGI_FORMAT_R16G16_FLOAT),
			std::make_pair(EF_B10G11R11F, DXGI_FORMAT_R11G11B10_FLOAT),
			std::make_pair(EF_ABGR16F, DXGI_FORMAT_R16G16B16A16_FLOAT),
			std::make_pair(EF_R32F, DXGI_FORMAT_R32_FLOAT),
			std::make_pair(EF_GR32F, DXGI_FORMAT_R32G32_FLOAT),
			std::make_pair(EF_BGR32F, DXGI_FORMAT_R32G32B32_FLOAT),
			std::make_pair(EF_ABGR32F, DXGI_FORMAT_R32G32B32A32_FLOAT),
			std::make_pair(EF_BC1, DXGI_FORMAT_BC1_UNORM),
			std::make_pair(EF_BC2, DXGI_FORMAT_BC2_UNORM),
			std::make_pair(EF_BC3, DXGI_FORMAT_BC3_UNORM),
			std::make_pair(EF_BC4, DXGI_FORMAT_BC4_UNORM),
			std::make_pair(EF_SIGNED_BC4, DXGI_FORMAT_BC4_SNORM),
			std::make_pair(EF_BC5, DXGI_FORMAT_BC5_UNORM),
			std::make_pair(EF_SIGNED_BC5, DXGI_FORMAT_BC5_SNORM),
			std::make_pair(EF_BC6, DXGI_FORMAT_BC6H_UF16),
			std::make_pair(EF_SIGNED_BC6, DXGI_FORMAT_BC6H_SF16),
			std::make_pair(EF_BC7, DXGI_FORMAT_BC7_UNORM),
			std::make_pair(EF_D16, DXGI_FORMAT_D16_UNORM),
			std::make_pair(EF_D24S8, DXGI_FORMAT_D24_UNORM_S8_UINT),
			std::make_pair(EF_D32F, DXGI_FORMAT_D32_FLOAT),
			std::make_pair(EF_ARGB8_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB),
			std::make_pair(EF_ABGR8_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
			std::make_pair(EF_BC1_SRGB, DXGI_FORMAT_BC1_UNORM_SRGB),
			std::make_pair(EF_BC2_SRGB, DXGI_FORMAT_BC2_UNORM_SRGB),
			std::make_pair(EF_BC3_SRGB, DXGI_FORMAT_BC3_UNORM_SRGB),
			std::make_pair(EF_BC7_SRGB, DXGI_FORMAT_BC7_UNORM_SRGB)
		};

		UINT s;
		for (auto const & fmt : fmts)
		{
			if ((caps_.max_shader_model < ShaderModel(5, 0))
				&& ((EF_BC6 == fmt.first) || (EF_SIGNED_BC6 == fmt.first)
					|| (EF_BC7 == fmt.first) || (EF_BC7_SRGB == fmt.first)))
			{
				continue;
			}

			if (!check_16bpp_fmts
				&& ((EF_R5G6B5 == fmt.first) || (EF_A1RGB5 == fmt.first) || (EF_ARGB4 == fmt.first)))
			{
				continue;
			}

			d3d_device_->CheckFormatSupport(fmt.second, &s);
			if (s != 0)
			{
				if (IsDepthFormat(fmt.first))
				{
					DXGI_FORMAT depth_fmt;
					switch (fmt.first)
					{
					case EF_D16:
						depth_fmt = DXGI_FORMAT_R16_TYPELESS;
						break;

					case EF_D24S8:
						depth_fmt = DXGI_FORMAT_R24G8_TYPELESS;
						break;

					case EF_D32F:
					default:
						depth_fmt = DXGI_FORMAT_R32_TYPELESS;
						break;
					}

					UINT s1;
					d3d_device_->CheckFormatSupport(depth_fmt, &s1);
					if (s1 != 0)
					{
						if (s1 & D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER)
						{
							vertex_format_.push_back(fmt.first);
						}
						if (s1 & (D3D11_FORMAT_SUPPORT_TEXTURE1D | D3D11_FORMAT_SUPPORT_TEXTURE2D
							| D3D11_FORMAT_SUPPORT_TEXTURE3D | D3D11_FORMAT_SUPPORT_TEXTURECUBE
							| D3D11_FORMAT_SUPPORT_SHADER_LOAD | D3D11_FORMAT_SUPPORT_SHADER_SAMPLE))
						{
							texture_format_.push_back(fmt.first);
						}
					}
				}
				else
				{
					if (s & D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER)
					{
						vertex_format_.push_back(fmt.first);
					}
					if ((s & (D3D11_FORMAT_SUPPORT_TEXTURE1D | D3D11_FORMAT_SUPPORT_TEXTURE2D
						| D3D11_FORMAT_SUPPORT_TEXTURE3D | D3D11_FORMAT_SUPPORT_TEXTURECUBE))
						&& (s & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE))
					{
						texture_format_.push_back(fmt.first);
					}

					if (check_uav_fmts)
					{
						D3D11_FEATURE_DATA_FORMAT_SUPPORT2 format_support = { fmt.second , 0};
						HRESULT hr = d3d_device_->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2,
							&format_support, sizeof(format_support));
						if (SUCCEEDED(hr)
							&& ((format_support.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
							&& ((format_support.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0))
						{
							uav_format_.push_back(fmt.first);
						}
					}
				}

				if (s & (D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET
					| D3D11_FORMAT_SUPPORT_DEPTH_STENCIL))
				{
					UINT count = 1;
					UINT quality;
					while (count <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT)
					{
						if (SUCCEEDED(d3d_device_->CheckMultisampleQualityLevels(fmt.second, count, &quality)))
						{
							if (quality > 0)
							{
								rendertarget_format_[fmt.first].emplace_back(count, quality);
								count <<= 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
				}
			}
		}

		std::sort(vertex_format_.begin(), vertex_format_.end());
		vertex_format_.erase(std::unique(vertex_format_.begin(), vertex_format_.end()), vertex_format_.end());
		std::sort(texture_format_.begin(), texture_format_.end());
		texture_format_.erase(std::unique(texture_format_.begin(), texture_format_.end()), texture_format_.end());
		std::sort(uav_format_.begin(), uav_format_.end());
		uav_format_.erase(std::unique(uav_format_.begin(), uav_format_.end()), uav_format_.end());

		caps_.vertex_format_support = [this](ElementFormat elem_fmt)
			{
				return this->VertexFormatSupport(elem_fmt);
			};
		caps_.texture_format_support = [this](ElementFormat elem_fmt)
			{
				return this->TextureFormatSupport(elem_fmt);
			};
		caps_.rendertarget_format_support = [this](ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality)
			{
				return this->RenderTargetFormatSupport(elem_fmt, sample_count, sample_quality);
			};
		caps_.uav_format_support = [this](ElementFormat elem_fmt)
			{
				return this->UAVFormatSupport(elem_fmt);
			};

		caps_.depth_texture_support = (caps_.texture_format_support(EF_D24S8) || caps_.texture_format_support(EF_D16));
		caps_.fp_color_support = ((caps_.texture_format_support(EF_B10G11R11F) && caps_.rendertarget_format_support(EF_B10G11R11F, 1, 0))
			|| (caps_.texture_format_support(EF_ABGR16F) && caps_.rendertarget_format_support(EF_ABGR16F, 1, 0)));
		caps_.pack_to_rgba_required = !(caps_.texture_format_support(EF_R16F) && caps_.rendertarget_format_support(EF_R16F, 1, 0)
			&& caps_.texture_format_support(EF_R32F) && caps_.rendertarget_format_support(EF_R32F, 1, 0));
	}

	void D3D11RenderEngine::DetectD3D11Runtime(ID3D11Device* device, ID3D11DeviceContext* imm_ctx)
	{
		d3d_device_ = MakeCOMPtr(device);
		d3d_imm_ctx_ = MakeCOMPtr(imm_ctx);
		d3d_11_runtime_sub_ver_ = 0;

		ID3D11Device1* d3d_device_1 = nullptr;
		device->QueryInterface(IID_ID3D11Device1, reinterpret_cast<void**>(&d3d_device_1));
		if (d3d_device_1)
		{
			ID3D11DeviceContext1* d3d_imm_ctx_1 = nullptr;
			imm_ctx->QueryInterface(IID_ID3D11DeviceContext1, reinterpret_cast<void**>(&d3d_imm_ctx_1));
			if (d3d_imm_ctx_1)
			{
				d3d_device_1_ = MakeCOMPtr(d3d_device_1);
				d3d_imm_ctx_1_ = MakeCOMPtr(d3d_imm_ctx_1);
				d3d_11_runtime_sub_ver_ = 1;

				ID3D11Device2* d3d_device_2 = nullptr;
				device->QueryInterface(IID_ID3D11Device2, reinterpret_cast<void**>(&d3d_device_2));
				if (d3d_device_2)
				{
					ID3D11DeviceContext2* d3d_imm_ctx_2 = nullptr;
					imm_ctx->QueryInterface(IID_ID3D11DeviceContext2, reinterpret_cast<void**>(&d3d_imm_ctx_2));
					if (d3d_imm_ctx_2)
					{
						d3d_device_2_ = MakeCOMPtr(d3d_device_2);
						d3d_imm_ctx_2_ = MakeCOMPtr(d3d_imm_ctx_2);
						d3d_11_runtime_sub_ver_ = 2;

						ID3D11Device3* d3d_device_3 = nullptr;
						device->QueryInterface(IID_ID3D11Device3, reinterpret_cast<void**>(&d3d_device_3));
						if (d3d_device_3)
						{
							ID3D11DeviceContext3* d3d_imm_ctx_3 = nullptr;
							imm_ctx->QueryInterface(IID_ID3D11DeviceContext3, reinterpret_cast<void**>(&d3d_imm_ctx_3));
							if (d3d_imm_ctx_3)
							{
								d3d_device_3_ = MakeCOMPtr(d3d_device_3);
								d3d_imm_ctx_3_ = MakeCOMPtr(d3d_imm_ctx_3);
								d3d_11_runtime_sub_ver_ = 3;

								ID3D11Device4* d3d_device_4 = nullptr;
								device->QueryInterface(IID_ID3D11Device4, reinterpret_cast<void**>(&d3d_device_4));
								if (d3d_device_4)
								{
									d3d_device_4_ = MakeCOMPtr(d3d_device_4);
									d3d_11_runtime_sub_ver_ = 4;
								}
							}
							else
							{
								d3d_device_3->Release();
								d3d_device_3 = nullptr;
							}
						}
					}
					else
					{
						d3d_device_2->Release();
						d3d_device_2 = nullptr;
					}
				}
			}
			else
			{
				d3d_device_1->Release();
				d3d_device_1 = nullptr;
			}
		}
	}

	void D3D11RenderEngine::StereoscopicForLCDShutter(int32_t eye)
	{
		uint32_t const width = mono_tex_->Width(0);
		uint32_t const height = mono_tex_->Height(0);
		D3D11RenderWindow* win = checked_cast<D3D11RenderWindow*>(screen_frame_buffer_.get());

		switch (stereo_method_)
		{
		case SM_DXGI:
			{
				RenderView const * view = (0 == eye) ? win->D3DBackBufferRTV().get() : win->D3DBackBufferRightEyeRTV().get();
				ID3D11RenderTargetView* rtv = checked_cast<D3D11RenderTargetRenderView const *>(view)->D3DRenderTargetView();
				this->OMSetRenderTargets(1, &rtv, nullptr);

				D3D11_VIEWPORT vp;
				vp.TopLeftX = 0;
				vp.TopLeftY = 0;
				vp.Width = static_cast<float>(width);
				vp.Height = static_cast<float>(height);
				vp.MinDepth = 0;
				vp.MaxDepth = 1;

				this->RSSetViewports(1, &vp);
				stereoscopic_pp_->SetParam(3, eye);
				stereoscopic_pp_->Render();
			}
			break;

		case SM_NV3DVision:
			{
				this->BindFrameBuffer(stereo_nv_3d_vision_fb_);

				D3D11_VIEWPORT vp;
				vp.TopLeftY = 0;
				vp.Width = static_cast<float>(width);
				vp.Height = static_cast<float>(height);
				vp.MinDepth = 0;
				vp.MaxDepth = 1;
				vp.TopLeftX = (0 == eye) ? 0 : vp.Width;

				this->RSSetViewports(1, &vp);
				stereoscopic_pp_->SetParam(3, eye);
				stereoscopic_pp_->Render();
		
				if (0 == eye)
				{
					ID3D11Resource* back = checked_cast<D3D11Texture2D*>(win->D3DBackBuffer().get())->D3DResource();
					ID3D11Resource* stereo = checked_cast<D3D11Texture2D*>(stereo_nv_3d_vision_tex_.get())->D3DResource();

					D3D11_BOX box;
					box.left = 0;
					box.right = width;
					box.top = 0;
					box.bottom = height;
					box.front = 0;
					box.back = 1;
					d3d_imm_ctx_->CopySubresourceRegion(back, 0, 0, 0, 0, stereo, 0, &box);
				}
			}
			break;

		case SM_AMDQuadBuffer:
			{
				RenderView const * view = win->D3DBackBufferRTV().get();
				ID3D11RenderTargetView* rtv = checked_cast<D3D11RenderTargetRenderView const *>(view)->D3DRenderTargetView();
				this->OMSetRenderTargets(1, &rtv, nullptr);

				D3D11_VIEWPORT vp;
				vp.TopLeftX = 0;
				vp.TopLeftY = (0 == eye) ? 0 : static_cast<float>(win->StereoRightEyeHeight());
				vp.Width = static_cast<float>(width);
				vp.Height = static_cast<float>(height);
				vp.MinDepth = 0;
				vp.MaxDepth = 1;

				this->RSSetViewports(1, &vp);
				stereoscopic_pp_->SetParam(3, eye);
				stereoscopic_pp_->Render();
			}
			break;

		default:
			break;
		}
	}

	void D3D11RenderEngine::RSSetState(ID3D11RasterizerState* ras)
	{
		if (rasterizer_state_cache_ != ras)
		{
			d3d_imm_ctx_->RSSetState(ras);
			rasterizer_state_cache_ = ras;
		}
	}

	void D3D11RenderEngine::OMSetDepthStencilState(ID3D11DepthStencilState* ds, uint16_t stencil_ref)
	{
		if ((depth_stencil_state_cache_ != ds) || (stencil_ref_cache_ != stencil_ref))
		{
			d3d_imm_ctx_->OMSetDepthStencilState(ds, stencil_ref);
			depth_stencil_state_cache_ = ds;
			stencil_ref_cache_ = stencil_ref;
		}
	}

	void D3D11RenderEngine::OMSetBlendState(ID3D11BlendState* bs, Color const & blend_factor, uint32_t sample_mask)
	{
		if ((blend_state_cache_ != bs) || (blend_factor_cache_ != blend_factor) || (sample_mask_cache_ != sample_mask))
		{
			d3d_imm_ctx_->OMSetBlendState(bs, &blend_factor.r(), sample_mask);
			blend_state_cache_ = bs;
			blend_factor_cache_ = blend_factor;
			sample_mask_cache_ = sample_mask;
		}
	}

	void D3D11RenderEngine::VSSetShader(ID3D11VertexShader* shader)
	{
		if (vertex_shader_cache_ != shader)
		{
			d3d_imm_ctx_->VSSetShader(shader, nullptr, 0);
			vertex_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::PSSetShader(ID3D11PixelShader* shader)
	{
		if (pixel_shader_cache_ != shader)
		{
			d3d_imm_ctx_->PSSetShader(shader, nullptr, 0);
			pixel_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::GSSetShader(ID3D11GeometryShader* shader)
	{
		if (geometry_shader_cache_ != shader)
		{
			d3d_imm_ctx_->GSSetShader(shader, nullptr, 0);
			geometry_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::CSSetShader(ID3D11ComputeShader* shader)
	{
		if (compute_shader_cache_ != shader)
		{
			d3d_imm_ctx_->CSSetShader(shader, nullptr, 0);
			compute_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::HSSetShader(ID3D11HullShader* shader)
	{
		if (hull_shader_cache_ != shader)
		{
			d3d_imm_ctx_->HSSetShader(shader, nullptr, 0);
			hull_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::DSSetShader(ID3D11DomainShader* shader)
	{
		if (domain_shader_cache_ != shader)
		{
			d3d_imm_ctx_->DSSetShader(shader, nullptr, 0);
			domain_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::RSSetViewports(UINT NumViewports, D3D11_VIEWPORT const * pViewports)
	{
		if (NumViewports > 1)
		{
			d3d_imm_ctx_->RSSetViewports(NumViewports, pViewports);
		}
		else
		{
			if (!(MathLib::equal(pViewports->TopLeftX, viewport_cache_.TopLeftX)
				&& MathLib::equal(pViewports->TopLeftY, viewport_cache_.TopLeftY)
				&& MathLib::equal(pViewports->Width, viewport_cache_.Width)
				&& MathLib::equal(pViewports->Height, viewport_cache_.Height)
				&& MathLib::equal(pViewports->MinDepth, viewport_cache_.MinDepth)
				&& MathLib::equal(pViewports->MaxDepth, viewport_cache_.MaxDepth)))
			{
				viewport_cache_ = *pViewports;
				d3d_imm_ctx_->RSSetViewports(NumViewports, pViewports);
			}
		}
	}

	void D3D11RenderEngine::OMSetRenderTargets(UINT num_rtvs, ID3D11RenderTargetView* const * rtvs, ID3D11DepthStencilView* dsv)
	{
		if ((rtv_ptr_cache_.size() != num_rtvs) || (dsv_ptr_cache_ != dsv)
			|| (memcmp(&rtv_ptr_cache_[0], rtvs, num_rtvs * sizeof(rtvs[0])) != 0))
		{
			d3d_imm_ctx_->OMSetRenderTargets(num_rtvs, rtvs, dsv);

			rtv_ptr_cache_.assign(rtvs, rtvs + num_rtvs);
			dsv_ptr_cache_ = dsv;
		}
	}

	void D3D11RenderEngine::OMSetRenderTargetsAndUnorderedAccessViews(UINT num_rtvs, ID3D11RenderTargetView* const * rtvs,
		ID3D11DepthStencilView* dsv,
		UINT uav_start_slot, UINT num_uavs, ID3D11UnorderedAccessView* const * uavs, UINT const * uav_init_counts)
	{
		if ((rtv_ptr_cache_.size() != num_rtvs) || (dsv_ptr_cache_ != dsv)
			|| (render_uav_ptr_cache_.size() < uav_start_slot + num_uavs)
			|| (memcmp(&rtv_ptr_cache_[0], rtvs, num_rtvs * sizeof(rtvs[0])) != 0)
			|| (memcmp(&render_uav_ptr_cache_[uav_start_slot], uavs, num_uavs * sizeof(uavs[0])) != 0)
			|| (memcmp(&render_uav_init_count_cache_[uav_start_slot], uav_init_counts, num_uavs * sizeof(uav_init_counts[0])) != 0))
		{
			d3d_imm_ctx_->OMSetRenderTargetsAndUnorderedAccessViews(num_rtvs, rtvs, dsv,
				uav_start_slot, num_uavs, uavs, uav_init_counts);

			rtv_ptr_cache_.assign(rtvs, rtvs + num_rtvs);
			dsv_ptr_cache_ = dsv;

			if (render_uav_ptr_cache_.size() < uav_start_slot + num_uavs)
			{
				render_uav_ptr_cache_.resize(uav_start_slot + num_uavs);
				render_uav_init_count_cache_.resize(render_uav_ptr_cache_.size());
			}
			memcpy(&render_uav_ptr_cache_[uav_start_slot], uavs, num_uavs * sizeof(uavs[0]));
			memcpy(&render_uav_init_count_cache_[uav_start_slot], uav_init_counts, num_uavs * sizeof(uav_init_counts[0]));
		}
	}

	void D3D11RenderEngine::CSSetUnorderedAccessViews(UINT start_slot, UINT num_uavs, ID3D11UnorderedAccessView* const * uavs,
		UINT const * uav_init_counts)
	{
		if ((compute_uav_ptr_cache_.size() < start_slot + num_uavs)
			|| (memcmp(&compute_uav_ptr_cache_[start_slot], uavs, num_uavs * sizeof(uavs[0])) != 0)
			|| (memcmp(&compute_uav_init_count_cache_[start_slot], uav_init_counts, num_uavs * sizeof(uav_init_counts[0])) != 0))
		{
			d3d_imm_ctx_->CSSetUnorderedAccessViews(start_slot, num_uavs, uavs, uav_init_counts);

			if (compute_uav_ptr_cache_.size() < start_slot + num_uavs)
			{
				compute_uav_ptr_cache_.resize(start_slot + num_uavs);
				compute_uav_init_count_cache_.resize(compute_uav_ptr_cache_.size());
			}
			memcpy(&compute_uav_ptr_cache_[start_slot], uavs, num_uavs * sizeof(uavs[0]));
			memcpy(&compute_uav_init_count_cache_[start_slot], uav_init_counts, num_uavs * sizeof(uav_init_counts[0]));
		}
	}

	void D3D11RenderEngine::SetShaderResources(ShaderObject::ShaderType st,
			std::vector<std::tuple<void*, uint32_t, uint32_t>> const & srvsrcs,
			std::vector<ID3D11ShaderResourceView*> const & srvs)
	{
		if (shader_srv_ptr_cache_[st] != srvs)
		{
			size_t const old_size = shader_srv_ptr_cache_[st].size();
			shader_srv_ptr_cache_[st] = srvs;
			if (old_size > srvs.size())
			{
				shader_srv_ptr_cache_[st].resize(old_size, nullptr);
			}

			ShaderSetShaderResources[st](d3d_imm_ctx_.get(), 0,
				static_cast<UINT>(shader_srv_ptr_cache_[st].size()), &shader_srv_ptr_cache_[st][0]);

			shader_srvsrc_cache_[st] = srvsrcs;
			shader_srv_ptr_cache_[st].resize(srvs.size());
		}
	}

	void D3D11RenderEngine::SetSamplers(ShaderObject::ShaderType st, std::vector<ID3D11SamplerState*> const & samplers)
	{
		if (shader_sampler_ptr_cache_[st] != samplers)
		{
			ShaderSetSamplers[st](d3d_imm_ctx_.get(), 0, static_cast<UINT>(samplers.size()), &samplers[0]);

			shader_sampler_ptr_cache_[st] = samplers;
		}
	}

	void D3D11RenderEngine::SetConstantBuffers(ShaderObject::ShaderType st, std::vector<ID3D11Buffer*> const & cbs)
	{
		if (shader_cb_ptr_cache_[st] != cbs)
		{
			ShaderSetConstantBuffers[st](d3d_imm_ctx_.get(), 0, static_cast<UINT>(cbs.size()), &cbs[0]);

			shader_cb_ptr_cache_[st] = cbs;
		}
	}

	void D3D11RenderEngine::DetachSRV(void* rtv_src, uint32_t rt_first_subres, uint32_t rt_num_subres)
	{
		for (uint32_t st = 0; st < ShaderObject::ST_NumShaderTypes; ++ st)
		{
			bool cleared = false;
			for (uint32_t i = 0; i < shader_srvsrc_cache_[st].size(); ++ i)
			{
				if (std::get<0>(shader_srvsrc_cache_[st][i]))
				{
					if (std::get<0>(shader_srvsrc_cache_[st][i]) == rtv_src)
					{
						uint32_t const first = std::get<1>(shader_srvsrc_cache_[st][i]);
						uint32_t const last = first + std::get<2>(shader_srvsrc_cache_[st][i]);
						uint32_t const rt_first = rt_first_subres;
						uint32_t const rt_last = rt_first_subres + rt_num_subres;
						if (((first >= rt_first) && (first < rt_last))
							|| ((last >= rt_first) && (last < rt_last))
							|| ((rt_first >= first) && (rt_first < last))
							|| ((rt_last >= first) && (rt_last < last)))
						{
							shader_srv_ptr_cache_[st][i] = nullptr;
							cleared = true;
						}
					}
				}
			}

			if (cleared)
			{
				ShaderSetShaderResources[st](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_srv_ptr_cache_[st].size()), &shader_srv_ptr_cache_[st][0]);
			}
		}
	}

	void D3D11RenderEngine::InvalidRTVCache()
	{
		rtv_ptr_cache_.clear();
	}

	HRESULT D3D11RenderEngine::D3D11CreateDevice(IDXGIAdapter* pAdapter,
			D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
			D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
			ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const
	{
		return DynamicD3D11CreateDevice_(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion,
			ppDevice, pFeatureLevel, ppImmediateContext);
	}

	void D3D11RenderEngine::OnDeviceLost(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WAIT wait, TP_WAIT_RESULT wait_result)
	{
		KFL_UNUSED(instance);
		KFL_UNUSED(context);
		KFL_UNUSED(wait);
		KFL_UNUSED(wait_result);

		// TODO
	}
}
