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
#include <KFL/ThrowErr.hpp>
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

#include <KlayGE/D3D11/D3D11RenderWindow.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS_METRO
using namespace Windows::UI::Core;
#endif

namespace
{
	using namespace KlayGE;

	boost::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11ShaderResourceView * const *)> ShaderSetShaderResources[ShaderObject::ST_NumShaderTypes] =
	{
		boost::mem_fn(&ID3D11DeviceContext::VSSetShaderResources),
		boost::mem_fn(&ID3D11DeviceContext::PSSetShaderResources),
		boost::mem_fn(&ID3D11DeviceContext::GSSetShaderResources),
		boost::mem_fn(&ID3D11DeviceContext::CSSetShaderResources),
		boost::mem_fn(&ID3D11DeviceContext::HSSetShaderResources),
		boost::mem_fn(&ID3D11DeviceContext::DSSetShaderResources)
	};
	
	boost::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11SamplerState * const *)> ShaderSetSamplers[ShaderObject::ST_NumShaderTypes] =
	{
		boost::mem_fn(&ID3D11DeviceContext::VSSetSamplers),
		boost::mem_fn(&ID3D11DeviceContext::PSSetSamplers),
		boost::mem_fn(&ID3D11DeviceContext::GSSetSamplers),
		boost::mem_fn(&ID3D11DeviceContext::CSSetSamplers),
		boost::mem_fn(&ID3D11DeviceContext::HSSetSamplers),
		boost::mem_fn(&ID3D11DeviceContext::DSSetSamplers)
	};

	boost::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11Buffer * const *)> ShaderSetConstantBuffers[ShaderObject::ST_NumShaderTypes] =
	{
		boost::mem_fn(&ID3D11DeviceContext::VSSetConstantBuffers),
		boost::mem_fn(&ID3D11DeviceContext::PSSetConstantBuffers),
		boost::mem_fn(&ID3D11DeviceContext::GSSetConstantBuffers),
		boost::mem_fn(&ID3D11DeviceContext::CSSetConstantBuffers),
		boost::mem_fn(&ID3D11DeviceContext::HSSetConstantBuffers),
		boost::mem_fn(&ID3D11DeviceContext::DSSetConstantBuffers)
	};
}

// Stereo Blit defines
#define NVSTEREO_IMAGE_SIGNATURE 0x4433564E		//NV3D

struct NVSTEREOIMAGEHEADER
{
	unsigned int dwSignature;
	unsigned int dwWidth;
	unsigned int dwHeight;
	unsigned int dwBPP;
	unsigned int dwFlags;
};

// Flags in the dwFlags fiels of the _Nv_Stereo_Image_Header structure above
#define     SIH_SWAP_EYES               0x00000001
#define     SIH_SCALE_TO_FIT            0x00000002

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D11RenderEngine::D3D11RenderEngine()
		: num_so_buffs_(0)
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		// Dynamic loading because these dlls can't be loaded on WinXP
		mod_dxgi_ = ::LoadLibraryW(L"dxgi.dll");
		if (nullptr == mod_dxgi_)
		{
			::MessageBoxW(nullptr, L"Can't load dxgi.dll", L"Error", MB_OK);
		}
		mod_d3d11_ = ::LoadLibraryW(L"D3D11.dll");
		if (nullptr == mod_d3d11_)
		{
			::MessageBoxW(nullptr, L"Can't load d3d11.dll", L"Error", MB_OK);
		}
		mod_d3dcompiler_ = ::LoadLibraryW(L"d3dcompiler_46.dll");
		if (nullptr == mod_d3dcompiler_)
		{
			::MessageBoxW(nullptr, L"Can't load d3dcompiler_46.dll", L"Error", MB_OK);
		}

		if (mod_dxgi_ != nullptr)
		{
			DynamicCreateDXGIFactory1_ = reinterpret_cast<CreateDXGIFactory1Func>(::GetProcAddress(mod_dxgi_, "CreateDXGIFactory1"));
		}
		if (mod_d3d11_ != nullptr)
		{
			DynamicD3D11CreateDevice_ = reinterpret_cast<D3D11CreateDeviceFunc>(::GetProcAddress(mod_d3d11_, "D3D11CreateDevice"));
		}
		if (mod_d3dcompiler_ != nullptr)
		{
			DynamicD3DCompile_ = reinterpret_cast<D3DCompileFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DCompile"));
			DynamicD3DReflect_ = reinterpret_cast<D3DReflectFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DReflect"));
			DynamicD3DStripShader_ = reinterpret_cast<D3DStripShaderFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DStripShader"));
		}

		IDXGIFactory1* gi_factory;
		TIF(DynamicCreateDXGIFactory1_(IID_IDXGIFactory1, reinterpret_cast<void**>(&gi_factory)));
		gi_factory_ = MakeCOMPtr(gi_factory);
#else
		IDXGIFactory2* gi_factory;
		TIF(CreateDXGIFactory1(IID_IDXGIFactory2, reinterpret_cast<void**>(&gi_factory)));
		gi_factory_ = MakeCOMPtr(gi_factory);
#endif

		adapterList_.Enumerate(gi_factory_);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D11RenderEngine::~D3D11RenderEngine()
	{
		cur_frame_buffer_.reset();
		screen_frame_buffer_.reset();
		mono_tex_.reset();
		stereo_lr_tex_.reset();
		stereo_lr_3d_vision_tex_.reset();

		rasterizer_state_cache_.reset();
		depth_stencil_state_cache_.reset();
		blend_state_cache_.reset();
		vertex_shader_cache_.reset();
		pixel_shader_cache_.reset();
		geometry_shader_cache_.reset();
		compute_shader_cache_.reset();
		hull_shader_cache_.reset();
		domain_shader_cache_.reset();
		input_layout_cache_.reset();
		ib_cache_.reset();

		input_layout_bank_.clear();

		d3d_imm_ctx_->ClearState();
		d3d_imm_ctx_.reset();
		d3d_device_.reset();
		gi_factory_.reset();

		// Some other resources may still alive, so don't free them
		//::FreeLibrary(mod_d3d11_);
		//::FreeLibrary(mod_dxgi_);
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & D3D11RenderEngine::Name() const
	{
		static std::wstring const name(L"Direct3D11 Render Engine");
		return name;
	}

	// 获取D3D接口
	/////////////////////////////////////////////////////////////////////////////////
	IDXGIFactory1Ptr const & D3D11RenderEngine::DXGIFactory() const
	{
		return gi_factory_;
	}

	// 获取D3D Device接口
	/////////////////////////////////////////////////////////////////////////////////
	ID3D11DevicePtr const & D3D11RenderEngine::D3DDevice() const
	{
		return d3d_device_;
	}

	// 获取D3D Device Context接口
	/////////////////////////////////////////////////////////////////////////////////
	ID3D11DeviceContextPtr const & D3D11RenderEngine::D3DDeviceImmContext() const
	{
		return d3d_imm_ctx_;
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
	D3D11AdapterPtr const & D3D11RenderEngine::ActiveAdapter() const
	{
		return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
	}

	// 开始渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::StartRendering()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		bool gotMsg;
		MSG  msg;

		::PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);

		FrameBuffer& fb = *this->ScreenFrameBuffer();
		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if (fb.Active())
			{
				gotMsg = (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0);
			}
			else
			{
				gotMsg = (::GetMessage(&msg, nullptr, 0, 0) != 0);
			}

			if (gotMsg)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			else
			{
				// 在空余时间渲染帧 (没有等待的消息)
				if (fb.Active())
				{
					Context::Instance().SceneManagerInstance().Update();
					fb.SwapBuffers();
				}
			}
		}
#else
		FrameBufferPtr fb = this->ScreenFrameBuffer();
		while (!checked_pointer_cast<D3D11RenderWindow>(fb)->Closed())
		{
			if (fb->Active())
			{
				CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
				Context::Instance().SceneManagerInstance().Update();
				fb->SwapBuffers();
			}
			else
			{
				CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
#endif
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		motion_frames_ = settings.motion_frames;

		D3D11RenderWindowPtr win = MakeSharedPtr<D3D11RenderWindow>(gi_factory_, this->ActiveAdapter(),
			name, settings);

		switch (d3d_feature_level_)
		{
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
		case D3D_FEATURE_LEVEL_11_1:
#endif
		case D3D_FEATURE_LEVEL_11_0:
			vs_profile_ = "vs_5_0";
			ps_profile_ = "ps_5_0";
			gs_profile_ = "gs_5_0";
			cs_profile_ = "cs_5_0";
			hs_profile_ = "hs_5_0";
			ds_profile_ = "ds_5_0";
			break;

		case D3D_FEATURE_LEVEL_10_1:
			vs_profile_ = "vs_4_1";
			ps_profile_ = "ps_4_1";
			gs_profile_ = "gs_4_1";
			cs_profile_ = "cs_4_1";
			hs_profile_ = "";
			ds_profile_ = "";
			break;

		case D3D_FEATURE_LEVEL_10_0:
			vs_profile_ = "vs_4_0";
			ps_profile_ = "ps_4_0";
			gs_profile_ = "gs_4_0";
			cs_profile_ = "cs_4_0";
			hs_profile_ = "";
			ds_profile_ = "";
			break;

		default:
			vs_profile_ = "vs_4_0_level_9_3";
			ps_profile_ = "ps_4_0_level_9_3";
			gs_profile_ = "";
			cs_profile_ = "";
			hs_profile_ = "";
			ds_profile_ = "";
			break;
		}

		this->ResetRenderStates();
		this->BindFrameBuffer(win);

		if (STM_LCDShutter == render_settings_.stereo_method)
		{
			uint32_t const w = win->Width();
			uint32_t const h = win->Height();
			stereo_lr_3d_vision_tex_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(w * 2, h + 1, 1, 1,
				render_settings_.color_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);

			NVSTEREOIMAGEHEADER sih;
			sih.dwSignature = NVSTEREO_IMAGE_SIGNATURE;
			sih.dwBPP = NumFormatBits(render_settings_.color_fmt);
			sih.dwFlags = SIH_SWAP_EYES;
			sih.dwWidth = w * 2; 
			sih.dwHeight = h;

			ElementInitData init_data;
			init_data.data = &sih;
			init_data.row_pitch = sizeof(sih);
			init_data.slice_pitch = init_data.row_pitch;
			TexturePtr sih_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(sizeof(sih) / NumFormatBytes(render_settings_.color_fmt),
				1, 1, 1, render_settings_.color_fmt, 1, 0, EAH_GPU_Read, &init_data);

			sih_tex->CopyToSubTexture2D(*stereo_lr_3d_vision_tex_,
				0, 0, 0, h, sih_tex->Width(0), 1,
				0, 0, 0, 0, sih_tex->Width(0), 1);
		}
	}

	void D3D11RenderEngine::D3DDevice(ID3D11DevicePtr const & device, ID3D11DeviceContextPtr const & imm_ctx, D3D_FEATURE_LEVEL feature_level)
	{
		d3d_device_ = device;
		d3d_imm_ctx_ = imm_ctx;
		d3d_feature_level_ = feature_level;
		Verify(d3d_device_ != ID3D11DevicePtr());

		this->FillRenderDeviceCaps();
	}

	void D3D11RenderEngine::ResetRenderStates()
	{
		RasterizerStateDesc default_rs_desc;
		DepthStencilStateDesc default_dss_desc;
		BlendStateDesc default_bs_desc;

		vertex_shader_cache_.reset();
		pixel_shader_cache_.reset();
		geometry_shader_cache_.reset();
		compute_shader_cache_.reset();
		hull_shader_cache_.reset();
		domain_shader_cache_.reset();

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRasterizerStateObject(default_rs_desc);
		cur_dss_obj_ = rf.MakeDepthStencilStateObject(default_dss_desc);
		cur_bs_obj_ = rf.MakeBlendStateObject(default_bs_desc);

		rasterizer_state_cache_ = checked_cast<D3D11RasterizerStateObject*>(cur_rs_obj_.get())->D3DRasterizerState();
		depth_stencil_state_cache_ = checked_cast<D3D11DepthStencilStateObject*>(cur_dss_obj_.get())->D3DDepthStencilState();
		stencil_ref_cache_ = 0;
		blend_state_cache_ = checked_cast<D3D11BlendStateObject*>(cur_bs_obj_.get())->D3DBlendState();
		blend_factor_cache_ = Color(1, 1, 1, 1);
		sample_mask_cache_ = 0xFFFFFFFF;

		d3d_imm_ctx_->RSSetState(rasterizer_state_cache_.get());
		d3d_imm_ctx_->OMSetDepthStencilState(depth_stencil_state_cache_.get(), stencil_ref_cache_);
		d3d_imm_ctx_->OMSetBlendState(blend_state_cache_.get(), &blend_factor_cache_.r(), sample_mask_cache_);

		topology_type_cache_ = RenderLayout::TT_PointList;
		d3d_imm_ctx_->IASetPrimitiveTopology(D3D11Mapping::Mapping(topology_type_cache_));

		input_layout_cache_.reset();
		d3d_imm_ctx_->IASetInputLayout(input_layout_cache_.get());

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

		ib_cache_.reset();
		d3d_imm_ctx_->IASetIndexBuffer(ib_cache_.get(), DXGI_FORMAT_R16_UINT, 0);

		for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			if (!shader_srv_cache_[i].empty())
			{
				std::fill(shader_srv_ptr_cache_[i].begin(), shader_srv_ptr_cache_[i].end(), static_cast<ID3D11ShaderResourceView*>(nullptr));
				ShaderSetShaderResources[i](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_srv_ptr_cache_[i].size()), &shader_srv_ptr_cache_[i][0]);
				shader_srvsrc_cache_[i].clear();
				shader_srv_cache_[i].clear();
				shader_srv_ptr_cache_[i].clear();
			}

			if (!shader_sampler_cache_[i].empty())
			{
				std::fill(shader_sampler_ptr_cache_[i].begin(), shader_sampler_ptr_cache_[i].end(), static_cast<ID3D11SamplerState*>(nullptr));
				ShaderSetSamplers[i](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_sampler_ptr_cache_[i].size()), &shader_sampler_ptr_cache_[i][0]);
				shader_sampler_cache_[i].clear();
				shader_sampler_ptr_cache_[i].clear();
			}

			if (!shader_cb_cache_[i].empty())
			{
				std::fill(shader_cb_ptr_cache_[i].begin(), shader_cb_ptr_cache_[i].end(), static_cast<ID3D11Buffer*>(nullptr));
				ShaderSetConstantBuffers[i](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_cb_ptr_cache_[i].size()), &shader_cb_ptr_cache_[i][0]);
				shader_cb_cache_[i].clear();
				shader_cb_ptr_cache_[i].clear();
			}
		}
	}

	ID3D11InputLayoutPtr const & D3D11RenderEngine::CreateD3D11InputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> const & elems, size_t signature, std::vector<uint8_t> const & vs_code)
	{
		size_t elems_signature = 0;
		typedef KLAYGE_DECLTYPE(elems) ElemsType;
		KLAYGE_FOREACH(ElemsType::const_reference elem, elems)
		{
			size_t seed = boost::hash_range(elem.SemanticName, elem.SemanticName + strlen(elem.SemanticName));
			boost::hash_combine(seed, elem.SemanticIndex);
			boost::hash_combine(seed, static_cast<uint32_t>(elem.Format));
			boost::hash_combine(seed, elem.InputSlot);
			boost::hash_combine(seed, elem.AlignedByteOffset);
			boost::hash_combine(seed, static_cast<uint32_t>(elem.InputSlotClass));
			boost::hash_combine(seed, elem.InstanceDataStepRate);

			boost::hash_combine(elems_signature, seed);
		}

		boost::hash_combine(signature, elems_signature);

		KLAYGE_AUTO(iter, input_layout_bank_.find(signature));
		if (iter != input_layout_bank_.end())
		{
			return iter->second;
		}
		else
		{
			ID3D11InputLayout* ia;
			TIF(d3d_device_->CreateInputLayout(&elems[0], static_cast<UINT>(elems.size()), &vs_code[0], vs_code.size(), &ia));
			ID3D11InputLayoutPtr ret = MakeCOMPtr(ia);

			KLAYGE_AUTO(in, input_layout_bank_.insert(std::make_pair(signature, ret)));

			return in.first->second;
		}
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
	{
		UNREF_PARAM(fb);

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
				d3d11_buffs[i] = d3d11_buf->D3DBuffer().get();
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
		else
		{
			std::vector<ID3D11Buffer*> d3d11_buffs(num_so_buffs_, nullptr);
			std::vector<UINT> d3d11_buff_offsets(num_so_buffs_, 0);
			d3d_imm_ctx_->SOSetTargets(static_cast<UINT>(num_so_buffs_), &d3d11_buffs[0], &d3d11_buff_offsets[0]);

			num_so_buffs_ = num_buffs;
		}
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::BeginFrame()
	{
	}

	// 开始一个Pass
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::BeginPass()
	{
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::DoRender(RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t const num_vertex_streams = rl.NumVertexStreams();
		uint32_t const all_num_vertex_stream = num_vertex_streams + (rl.InstanceStream() ? 1 : 0);

		std::vector<ID3D11Buffer*> vbs(all_num_vertex_stream);
		std::vector<UINT> strides(all_num_vertex_stream);
		std::vector<UINT> offsets(all_num_vertex_stream);
		for (uint32_t i = 0; i < num_vertex_streams; ++ i)
		{
			GraphicsBufferPtr const & stream = rl.GetVertexStream(i);

			D3D11GraphicsBuffer const & d3dvb = *checked_cast<D3D11GraphicsBuffer*>(stream.get());
			vbs[i] = d3dvb.D3DBuffer().get();
			strides[i] = rl.VertexSize(i);
			offsets[i] = 0;
		}
		if (rl.InstanceStream())
		{
			uint32_t number = num_vertex_streams;
			GraphicsBufferPtr stream = rl.InstanceStream();

			D3D11GraphicsBuffer const & d3dvb = *checked_cast<D3D11GraphicsBuffer*>(stream.get());
			vbs[number] = d3dvb.D3DBuffer().get();
			strides[number] = rl.InstanceSize();
			offsets[number] = 0;
		}

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

			D3D11RenderLayout const & d3d_rl = *checked_cast<D3D11RenderLayout const *>(&rl);
			D3D11ShaderObject const & shader = *checked_cast<D3D11ShaderObject*>(tech.Pass(0)->GetShaderObject().get());
			ID3D11InputLayoutPtr const & layout = d3d_rl.InputLayout(shader.VSSignature(), *shader.VSCode());
			if (layout != input_layout_cache_)
			{
				d3d_imm_ctx_->IASetInputLayout(layout.get());
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

			input_layout_cache_.reset();
			d3d_imm_ctx_->IASetInputLayout(nullptr);
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
				BOOST_ASSERT(false);
				prim_count = 0;
			}
			break;
		}

		numPrimitivesJustRendered_ += prim_count;
		numVerticesJustRendered_ += vertex_count;

		uint32_t const num_passes = tech.NumPasses();
		uint32_t const num_instances = rl.NumInstances();
		if (num_instances > 1)
		{
			if (rl.UseIndices())
			{
				D3D11GraphicsBuffer& d3dib = *checked_cast<D3D11GraphicsBuffer*>(rl.GetIndexStream().get());
				if (ib_cache_ != d3dib.D3DBuffer())
				{
					d3d_imm_ctx_->IASetIndexBuffer(d3dib.D3DBuffer().get(), D3D11Mapping::MappingFormat(rl.IndexStreamFormat()), 0);
					ib_cache_ = d3dib.D3DBuffer();
				}

				uint32_t const num_indices = rl.NumIndices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					d3d_imm_ctx_->DrawIndexedInstanced(num_indices, num_instances, rl.StartIndexLocation(), rl.StartVertexLocation(), rl.StartInstanceLocation());
					pass->Unbind();
				}
			}
			else
			{
				if (ib_cache_)
				{
					d3d_imm_ctx_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
					ib_cache_.reset();
				}

				uint32_t const num_vertices = rl.NumVertices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					d3d_imm_ctx_->DrawInstanced(num_vertices, num_instances, rl.StartVertexLocation(), rl.StartInstanceLocation());
					pass->Unbind();
				}
			}
		}
		else
		{
			if (rl.UseIndices())
			{
				D3D11GraphicsBuffer& d3dib = *checked_cast<D3D11GraphicsBuffer*>(rl.GetIndexStream().get());
				if (ib_cache_ != d3dib.D3DBuffer())
				{
					d3d_imm_ctx_->IASetIndexBuffer(d3dib.D3DBuffer().get(), D3D11Mapping::MappingFormat(rl.IndexStreamFormat()), 0);
					ib_cache_ = d3dib.D3DBuffer();
				}

				uint32_t const num_indices = rl.NumIndices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					d3d_imm_ctx_->DrawIndexed(num_indices, rl.StartIndexLocation(), rl.StartVertexLocation());
					pass->Unbind();
				}
			}
			else
			{
				if (ib_cache_)
				{
					d3d_imm_ctx_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
					ib_cache_.reset();
				}

				uint32_t const num_vertices = rl.NumVertices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					d3d_imm_ctx_->Draw(num_vertices, rl.StartVertexLocation());
					pass->Unbind();
				}
			}
		}
	}

	void D3D11RenderEngine::DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++ i)
		{
			RenderPassPtr const & pass = tech.Pass(i);

			pass->Bind();
			d3d_imm_ctx_->Dispatch(tgx, tgy, tgz);
			pass->Unbind();
		}
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::EndFrame()
	{
	}

	// 结束一个Pass
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::EndPass()
	{
	}

	void D3D11RenderEngine::ForceFlush()
	{
		d3d_imm_ctx_->Flush();
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		D3D11_RECT rc = { static_cast<LONG>(x), static_cast<LONG>(y),
			static_cast<LONG>(width), static_cast<LONG>(height) };
		d3d_imm_ctx_->RSSetScissorRects(1, &rc);
	}

	void D3D11RenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_cast<D3D11RenderWindow*>(screen_frame_buffer_.get())->Resize(width, height);
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
		return vertex_format_.find(elem_fmt) != vertex_format_.end();
	}

	bool D3D11RenderEngine::TextureFormatSupport(ElementFormat elem_fmt)
	{
		return texture_format_.find(elem_fmt) != texture_format_.end();
	}

	bool D3D11RenderEngine::RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality)
	{
		KLAYGE_AUTO(iter, rendertarget_format_.find(elem_fmt));
		if (iter != rendertarget_format_.end())
		{
			typedef KLAYGE_DECLTYPE(iter->second) RTType;
			KLAYGE_FOREACH(RTType::const_reference p, iter->second)
			{
				if ((sample_count == p.first) && (sample_quality < p.second))
				{
					return true;
				}
			}
		}
		return false;
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::FillRenderDeviceCaps()
	{
		BOOST_ASSERT(d3d_device_);

		if (d3d_feature_level_ <= D3D_FEATURE_LEVEL_9_3)
		{
			caps_.max_vertex_texture_units = 0;
			caps_.max_texture_height = caps_.max_texture_width = (D3D_FEATURE_LEVEL_9_3 == d3d_feature_level_) ? 4096 : 2048;
			caps_.max_texture_depth = 256;
			caps_.max_texture_cube_size = (D3D_FEATURE_LEVEL_9_3 == d3d_feature_level_) ? 4096 : 512;
			caps_.max_texture_array_length = 1;
			caps_.max_simultaneous_rts = (D3D_FEATURE_LEVEL_9_3 == d3d_feature_level_) ? 4 : 1;
			caps_.max_simultaneous_uavs = 0;
			caps_.alpha_to_coverage_support = false;
			caps_.mrt_independent_bit_depths_support = false;
		}
		else
		{
			caps_.max_vertex_texture_units = 16;
			caps_.max_texture_height = caps_.max_texture_width = 8192;
			caps_.max_texture_depth = 2048;
			caps_.max_texture_cube_size = 8192;
			caps_.max_texture_array_length = 512;
			caps_.max_simultaneous_rts = 8;
			caps_.max_simultaneous_uavs = D3D11_PS_CS_UAV_REGISTER_COUNT;
			caps_.alpha_to_coverage_support = true;
			caps_.mrt_independent_bit_depths_support = true;
		}
		if (d3d_feature_level_ <= D3D_FEATURE_LEVEL_9_2)
		{
			caps_.standard_derivatives_support = false;
		}
		else
		{
			caps_.standard_derivatives_support = true;
		}
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
		{
			D3D11_FEATURE_DATA_ARCHITECTURE_INFO arch_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_ARCHITECTURE_INFO, &arch_feature, sizeof(arch_feature));
			caps_.is_tbdr = arch_feature.TileBasedDeferredRenderer ? true : false;
		}
		{
			D3D11_FEATURE_DATA_D3D11_OPTIONS d3d11_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &d3d11_feature, sizeof(d3d11_feature));
			caps_.logic_op_support = d3d11_feature.OutputMergerLogicOp ? true : false;
		}
#else
		caps_.is_tbdr = false;
		caps_.logic_op_support = false;
#endif
		caps_.max_vertex_streams = 16;
		caps_.max_texture_anisotropy = (D3D_FEATURE_LEVEL_9_1 == d3d_feature_level_) ? 2 : 16;
		caps_.hw_instancing_support = (d3d_feature_level_ >= D3D_FEATURE_LEVEL_9_3);
		caps_.instance_id_support = (d3d_feature_level_ >= D3D_FEATURE_LEVEL_10_0);
		{
			D3D11_FEATURE_DATA_THREADING mt_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_THREADING, &mt_feature, sizeof(mt_feature));
			caps_.multithread_rendering_support = mt_feature.DriverCommandLists ? true : false;
			caps_.multithread_res_creating_support = mt_feature.DriverConcurrentCreates ? true : false;
		}
		switch (d3d_feature_level_)
		{
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
		case D3D_FEATURE_LEVEL_11_1:
#endif
		case D3D_FEATURE_LEVEL_11_0:
			caps_.max_shader_model = 5;
			caps_.stream_output_support = true;
			caps_.max_pixel_texture_units = 32;
			caps_.max_geometry_texture_units = 32;
			caps_.primitive_restart_support = true;
			caps_.gs_support = true;
			caps_.cs_support = true;
			caps_.hs_support = true;
			caps_.ds_support = true;
			caps_.tess_method = TM_Hardware;
			break;

		case D3D_FEATURE_LEVEL_10_1:
		case D3D_FEATURE_LEVEL_10_0:
			caps_.max_shader_model = 4;
			caps_.stream_output_support = true;
			caps_.max_pixel_texture_units = 32;
			caps_.max_geometry_texture_units = 32;
			caps_.primitive_restart_support = true;
			caps_.gs_support = true;
			caps_.hs_support = false;
			caps_.ds_support = false;
			{
				D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS cs4_feature;
				d3d_device_->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &cs4_feature, sizeof(cs4_feature));
				caps_.cs_support = cs4_feature.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x ? true : false;
			}
			caps_.tess_method = TM_Instanced;
			break;

		default:
			caps_.max_shader_model = 2;
			caps_.stream_output_support = false;
			caps_.max_pixel_texture_units = 16;
			caps_.max_geometry_texture_units = 0;
			caps_.primitive_restart_support = false;
			caps_.gs_support = false;
			caps_.cs_support = false;
			caps_.hs_support = false;
			caps_.ds_support = false;
			caps_.tess_method = TM_No;
			break;
		}

		std::pair<ElementFormat, DXGI_FORMAT> fmts[] = 
		{
			std::make_pair(EF_A8, DXGI_FORMAT_A8_UNORM),
			std::make_pair(EF_R8, DXGI_FORMAT_R8_UNORM),
			std::make_pair(EF_SIGNED_R8, DXGI_FORMAT_R8_SNORM),
			std::make_pair(EF_GR8, DXGI_FORMAT_R8G8_UNORM),
			std::make_pair(EF_SIGNED_GR8, DXGI_FORMAT_R8G8_SNORM),
			std::make_pair(EF_ARGB8, DXGI_FORMAT_B8G8R8A8_UNORM),
			std::make_pair(EF_ABGR8, DXGI_FORMAT_R8G8B8A8_UNORM),
			std::make_pair(EF_SIGNED_ABGR8, DXGI_FORMAT_R8G8B8A8_SNORM),
			std::make_pair(EF_A2BGR10, DXGI_FORMAT_R10G10B10A2_UNORM),
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
		for (size_t i = 0; i < sizeof(fmts) / sizeof(fmts[0]); ++ i)
		{
			if ((caps_.max_shader_model < 5)
				&& ((EF_BC6 == fmts[i].first) || (EF_SIGNED_BC6 == fmts[i].first)
					|| (EF_BC7 == fmts[i].first) || (EF_BC7_SRGB == fmts[i].first)))
			{
				continue;
			}

			d3d_device_->CheckFormatSupport(fmts[i].second, &s);
			if (s != 0)
			{
				if (IsDepthFormat(fmts[i].first))
				{
					DXGI_FORMAT depth_fmt;
					switch (fmts[i].first)
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

					d3d_device_->CheckFormatSupport(depth_fmt, &s);
					if (s != 0)
					{
						vertex_format_.insert(fmts[i].first);
						texture_format_.insert(fmts[i].first);
					}
				}
				else
				{
					vertex_format_.insert(fmts[i].first);
					texture_format_.insert(fmts[i].first);
				}

				UINT count = 1;
				UINT quality;
				while (count <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT)
				{
					if (SUCCEEDED(d3d_device_->CheckMultisampleQualityLevels(fmts[i].second, count, &quality)))
					{
						if (quality > 0)
						{
							rendertarget_format_[fmts[i].first].push_back(std::make_pair(count, quality));
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

		caps_.vertex_format_support = boost::bind<bool>(&D3D11RenderEngine::VertexFormatSupport, this, _1);
		caps_.texture_format_support = boost::bind<bool>(&D3D11RenderEngine::TextureFormatSupport, this, _1);
		caps_.rendertarget_format_support = boost::bind<bool>(&D3D11RenderEngine::RenderTargetFormatSupport, this, _1, _2, _3);
	}

	void D3D11RenderEngine::StereoscopicForLCDShutter()
	{
		uint32_t const w = stereo_lr_tex_->Width(0);
		uint32_t const h = stereo_lr_tex_->Height(0);
		stereo_lr_tex_->CopyToSubTexture2D(*stereo_lr_3d_vision_tex_, 0, 0, 0, 0, w, h, 0, 0, 0, 0, w, h);

		ID3D11Texture2DPtr back = checked_cast<D3D11RenderWindow*>(this->ScreenFrameBuffer().get())->D3DBackBuffer();
		ID3D11Texture2DPtr stereo = checked_cast<D3D11Texture2D*>(stereo_lr_3d_vision_tex_.get())->D3DTexture();

		D3D11_BOX box;
		box.left = 0;
		box.right = w / 2;
		box.top = 0;
		box.bottom = h;
		box.front = 0;
		box.back = 1;
		d3d_imm_ctx_->CopySubresourceRegion(back.get(), 0, 0, 0, 0, stereo.get(), 0, &box);
	}

	void D3D11RenderEngine::RSSetState(ID3D11RasterizerStatePtr const & ras)
	{
		if (rasterizer_state_cache_ != ras)
		{
			d3d_imm_ctx_->RSSetState(ras.get());
			rasterizer_state_cache_ = ras;
		}
	}

	void D3D11RenderEngine::OMSetDepthStencilState(ID3D11DepthStencilStatePtr const & ds, uint16_t stencil_ref)
	{
		if ((depth_stencil_state_cache_ != ds) || (stencil_ref_cache_ != stencil_ref))
		{
			d3d_imm_ctx_->OMSetDepthStencilState(ds.get(), stencil_ref);
			depth_stencil_state_cache_ = ds;
			stencil_ref_cache_ = stencil_ref;
		}
	}

	void D3D11RenderEngine::OMSetBlendState(ID3D11BlendStatePtr const & bs, Color const & blend_factor, uint32_t sample_mask)
	{
		if ((blend_state_cache_ != bs) || (blend_factor_cache_ != blend_factor) || (sample_mask_cache_ != sample_mask))
		{
			d3d_imm_ctx_->OMSetBlendState(bs.get(), &blend_factor.r(), sample_mask);
			blend_state_cache_ = bs;
			blend_factor_cache_ = blend_factor;
			sample_mask_cache_ = sample_mask;
		}
	}

	void D3D11RenderEngine::VSSetShader(ID3D11VertexShaderPtr const & shader)
	{
		if (vertex_shader_cache_ != shader)
		{
			d3d_imm_ctx_->VSSetShader(shader.get(), nullptr, 0);
			vertex_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::PSSetShader(ID3D11PixelShaderPtr const & shader)
	{
		if (pixel_shader_cache_ != shader)
		{
			d3d_imm_ctx_->PSSetShader(shader.get(), nullptr, 0);
			pixel_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::GSSetShader(ID3D11GeometryShaderPtr const & shader)
	{
		if (geometry_shader_cache_ != shader)
		{
			d3d_imm_ctx_->GSSetShader(shader.get(), nullptr, 0);
			geometry_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::CSSetShader(ID3D11ComputeShaderPtr const & shader)
	{
		if (compute_shader_cache_ != shader)
		{
			d3d_imm_ctx_->CSSetShader(shader.get(), nullptr, 0);
			compute_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::HSSetShader(ID3D11HullShaderPtr const & shader)
	{
		if (hull_shader_cache_ != shader)
		{
			d3d_imm_ctx_->HSSetShader(shader.get(), nullptr, 0);
			hull_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::DSSetShader(ID3D11DomainShaderPtr const & shader)
	{
		if (domain_shader_cache_ != shader)
		{
			d3d_imm_ctx_->DSSetShader(shader.get(), nullptr, 0);
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

	void D3D11RenderEngine::SetShaderResources(ShaderObject::ShaderType st, std::vector<tuple<void*, uint32_t, uint32_t> > const & srvsrcs, std::vector<ID3D11ShaderResourceViewPtr> const & srvs)
	{
		if (shader_srv_cache_[st] != srvs)
		{
			if (shader_srv_cache_[st].size() > srvs.size())
			{
				shader_srv_ptr_cache_[st].assign(shader_srv_cache_[st].size(), nullptr);
			}
			else
			{
				shader_srv_ptr_cache_[st].resize(srvs.size());
			}

			for (size_t i = 0; i < srvs.size(); ++ i)
			{
				shader_srv_ptr_cache_[st][i] = srvs[i].get();
			}

			ShaderSetShaderResources[st](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_srv_ptr_cache_[st].size()), &shader_srv_ptr_cache_[st][0]);

			shader_srvsrc_cache_[st] = srvsrcs;
			shader_srv_cache_[st] = srvs;
			shader_srv_ptr_cache_[st].resize(srvs.size());
		}
	}

	void D3D11RenderEngine::SetSamplers(ShaderObject::ShaderType st, std::vector<ID3D11SamplerStatePtr> const & samplers)
	{
		if (shader_sampler_cache_[st] != samplers)
		{
			if (shader_sampler_cache_[st].size() > samplers.size())
			{
				shader_sampler_ptr_cache_[st].assign(shader_sampler_cache_[st].size(), nullptr);
			}
			else
			{
				shader_sampler_ptr_cache_[st].resize(samplers.size());
			}

			for (size_t i = 0; i < samplers.size(); ++ i)
			{
				shader_sampler_ptr_cache_[st][i] = samplers[i].get();
			}

			ShaderSetSamplers[st](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_sampler_ptr_cache_[st].size()), &shader_sampler_ptr_cache_[st][0]);

			shader_sampler_cache_[st] = samplers;
			shader_sampler_ptr_cache_[st].resize(samplers.size());
		}
	}

	void D3D11RenderEngine::SetConstantBuffers(ShaderObject::ShaderType st, std::vector<ID3D11BufferPtr> const & cbs)
	{
		if (shader_cb_cache_[st] != cbs)
		{
			if (shader_cb_cache_[st].size() > cbs.size())
			{
				shader_cb_ptr_cache_[st].assign(shader_cb_cache_[st].size(), nullptr);
			}
			else
			{
				shader_cb_ptr_cache_[st].resize(cbs.size());
			}

			for (size_t i = 0; i < cbs.size(); ++ i)
			{
				shader_cb_ptr_cache_[st][i] = cbs[i].get();
			}

			ShaderSetConstantBuffers[st](d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_cb_ptr_cache_[st].size()), &shader_cb_ptr_cache_[st][0]);

			shader_cb_cache_[st] = cbs;
			shader_cb_ptr_cache_[st].resize(cbs.size());
		}
	}

	void D3D11RenderEngine::DetachSRV(void* rtv_src, uint32_t rt_first_subres, uint32_t rt_num_subres)
	{
		for (uint32_t st = 0; st < ShaderObject::ST_NumShaderTypes; ++ st)
		{
			bool cleared = false;
			for (uint32_t i = 0; i < shader_srvsrc_cache_[st].size(); ++ i)
			{
				if (get<0>(shader_srvsrc_cache_[st][i]))
				{
					if (get<0>(shader_srvsrc_cache_[st][i]) == rtv_src)
					{
						uint32_t const first = get<1>(shader_srvsrc_cache_[st][i]);
						uint32_t const last = first + get<2>(shader_srvsrc_cache_[st][i]);
						uint32_t const rt_first = rt_first_subres;
						uint32_t const rt_last = rt_first_subres + rt_num_subres;
						if (((first >= rt_first) && (first < rt_last))
							|| ((last >= rt_first) && (last < rt_last))
							|| ((rt_first >= first) && (rt_first < last))
							|| ((rt_last >= first) && (rt_last < last)))
						{
							shader_srv_cache_[st][i].reset();
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

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
	HRESULT D3D11RenderEngine::D3D11CreateDevice(IDXGIAdapter* pAdapter,
					D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
					D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
					ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const
	{
		return DynamicD3D11CreateDevice_(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion,
			ppDevice, pFeatureLevel, ppImmediateContext);
	}

	HRESULT D3D11RenderEngine::D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
					D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
					LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs) const
	{
		return DynamicD3DCompile_(pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, pEntrypoint,
					pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);
	}
	
	HRESULT D3D11RenderEngine::D3DReflect(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector) const
	{
		return DynamicD3DReflect_(pSrcData, SrcDataSize, pInterface, ppReflector);
	}

	HRESULT D3D11RenderEngine::D3DStripShader(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags, ID3DBlob** ppStrippedBlob) const
	{
		return DynamicD3DStripShader_(pShaderBytecode, BytecodeLength, uStripFlags, ppStrippedBlob);
	}
#else
	HRESULT D3D11RenderEngine::D3D11CreateDevice(IDXGIAdapter* pAdapter,
					D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
					D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
					ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const
	{
		return ::D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion,
			ppDevice, pFeatureLevel, ppImmediateContext);
	}
#endif
}
