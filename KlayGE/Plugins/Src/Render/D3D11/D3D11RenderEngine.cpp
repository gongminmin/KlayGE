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
#define INITGUID
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
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
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>

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
	{
		// Dynamic loading because these dlls can't be loaded on WinXP
		mod_dxgi_ = ::LoadLibraryW(L"dxgi.dll");
		if (NULL == mod_dxgi_)
		{
			::MessageBoxW(NULL, L"Can't load dxgi.dll", L"Error", MB_OK);
		}
		mod_d3d11_ = ::LoadLibraryW(L"D3D11.dll");
		if (NULL == mod_d3d11_)
		{
			::MessageBoxW(NULL, L"Can't load d3d11.dll", L"Error", MB_OK);
		}

		if (mod_dxgi_ != NULL)
		{
			DynamicCreateDXGIFactory1_ = reinterpret_cast<CreateDXGIFactory1Func>(::GetProcAddress(mod_dxgi_, "CreateDXGIFactory1"));
		}

		if (mod_d3d11_ != NULL)
		{
			DynamicD3D11CreateDeviceAndSwapChain_ = reinterpret_cast<D3D11CreateDeviceAndSwapChainFunc>(::GetProcAddress(mod_d3d11_, "D3D11CreateDeviceAndSwapChain"));
		}

		IDXGIFactory1* gi_factory;
		TIF(DynamicCreateDXGIFactory1_(IID_IDXGIFactory1, reinterpret_cast<void**>(&gi_factory)));
		gi_factory_ = MakeCOMPtr(gi_factory);

		adapterList_.Enumerate(gi_factory_);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D11RenderEngine::~D3D11RenderEngine()
	{
		cur_frame_buffer_.reset();
		screen_frame_buffer_.reset();
		stereo_frame_buffers_[0].reset();
		stereo_frame_buffers_[1].reset();

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
		bool gotMsg;
		MSG  msg;

		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

		FrameBuffer& fb = *this->ScreenFrameBuffer();
		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if (fb.Active())
			{
				gotMsg = (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
			}
			else
			{
				gotMsg = (::GetMessage(&msg, NULL, 0, 0) != 0);
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
			stereo_lr_tex_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(w * 2, h + 1, 1, 1,
				win->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

			NVSTEREOIMAGEHEADER sih;
			sih.dwSignature = NVSTEREO_IMAGE_SIGNATURE;
			sih.dwBPP = NumFormatBits(win->Format());
			sih.dwFlags = SIH_SWAP_EYES;
			sih.dwWidth = w * 2; 
			sih.dwHeight = h;

			ElementInitData init_data;
			init_data.data = &sih;
			init_data.row_pitch = sizeof(sih);
			init_data.slice_pitch = init_data.row_pitch;
			TexturePtr sih_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(sizeof(sih) / NumFormatBytes(win->Format()),
				1, 1, 1, win->Format(), 1, 0, EAH_GPU_Read, &init_data);

			sih_tex->CopyToTexture2D(*stereo_lr_tex_, 0, sih_tex->Width(0), 1, 0, h, sih_tex->Width(0), 1, 0, 0);
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

		rasterizer_state_cache_ = checked_pointer_cast<D3D11RasterizerStateObject>(cur_rs_obj_)->D3DRasterizerState();
		depth_stencil_state_cache_ = checked_pointer_cast<D3D11DepthStencilStateObject>(cur_dss_obj_)->D3DDepthStencilState();
		stencil_ref_cache_ = 0;
		blend_state_cache_ = checked_pointer_cast<D3D11BlendStateObject>(cur_bs_obj_)->D3DBlendState();
		blend_factor_cache_ = Color(1, 1, 1, 1);
		sample_mask_cache_ = 0xFFFFFFFF;

		d3d_imm_ctx_->RSSetState(rasterizer_state_cache_.get());
		d3d_imm_ctx_->OMSetDepthStencilState(depth_stencil_state_cache_.get(), stencil_ref_cache_);
		d3d_imm_ctx_->OMSetBlendState(blend_state_cache_.get(), &blend_factor_cache_.r(), sample_mask_cache_);

		topology_type_cache_ = RenderLayout::TT_PointList;
		d3d_imm_ctx_->IASetPrimitiveTopology(D3D11Mapping::Mapping(topology_type_cache_));

		input_layout_cache_.reset();
		d3d_imm_ctx_->IASetInputLayout(input_layout_cache_.get());
	}

	ID3D11InputLayoutPtr D3D11RenderEngine::CreateD3D11InputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> const & elems, std::vector<D3D11_SIGNATURE_PARAMETER_DESC> const & signature, ID3DBlobPtr const & vs_code)
	{
		for (BOOST_AUTO(iter, input_layout_bank_.begin()); iter != input_layout_bank_.end(); ++ iter)
		{
			if ((iter->first.input_elems.size() == elems.size()) && (iter->first.signature.size() == signature.size()))
			{
				bool match = true;
				for (size_t i = 0; i < elems.size(); ++ i)
				{
					D3D11_INPUT_ELEMENT_DESC const & lhs = iter->first.input_elems[i];
					D3D11_INPUT_ELEMENT_DESC const & rhs = elems[i];
					if ((std::string(lhs.SemanticName) != std::string(rhs.SemanticName))
						|| (lhs.SemanticIndex != rhs.SemanticIndex)
						|| (lhs.Format != rhs.Format)
						|| (lhs.InputSlot != rhs.InputSlot)
						|| (lhs.AlignedByteOffset != rhs.AlignedByteOffset)
						|| (lhs.InputSlotClass != rhs.InputSlotClass)
						|| (lhs.InstanceDataStepRate != rhs.InstanceDataStepRate))
					{
						match = false;
						break;
					}
				}
				if (match)
				{
					for (size_t i = 0; i < signature.size(); ++ i)
					{
						D3D11_SIGNATURE_PARAMETER_DESC const & lhs = iter->first.signature[i];
						D3D11_SIGNATURE_PARAMETER_DESC const & rhs = signature[i];
						if ((std::string(lhs.SemanticName) != std::string(rhs.SemanticName))
							|| (lhs.SemanticIndex != rhs.SemanticIndex)
							|| (lhs.Register != rhs.Register)
							|| (lhs.SystemValueType != rhs.SystemValueType)
							|| (lhs.ComponentType != rhs.ComponentType)
							|| (lhs.Mask != rhs.Mask)
							|| (lhs.ReadWriteMask != rhs.ReadWriteMask)
							|| (lhs.Stream != rhs.Stream))
						{
							match = false;
							break;
						}
					}
				}

				if (match)
				{
					return iter->second;
				}
			}
		}

		ID3D11InputLayout* ia;
		TIF(d3d_device_->CreateInputLayout(&elems[0], static_cast<UINT>(elems.size()), vs_code->GetBufferPointer(), vs_code->GetBufferSize(), &ia));
		ID3D11InputLayoutPtr ret = MakeCOMPtr(ia);

		input_layout_bank_.push_back(std::make_pair(InputElementTag(elems, signature), ret));

		return ret;
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
			std::vector<ID3D11Buffer*> d3d11_buffs(num_buffs);
			std::vector<UINT> d3d11_buff_offsets(num_buffs, 0);
			for (uint32_t i = 0; i < num_buffs; ++ i)
			{
				d3d11_buffs[i] = checked_pointer_cast<D3D11GraphicsBuffer>(rl->GetVertexStream(i))->D3DBuffer().get();
			}

			d3d_imm_ctx_->SOSetTargets(static_cast<UINT>(num_buffs), &d3d11_buffs[0], &d3d11_buff_offsets[0]);
		}
		else
		{
			d3d_imm_ctx_->SOSetTargets(0, NULL, NULL);
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
		uint32_t this_num_vertex_stream = rl.NumVertexStreams() + (rl.InstanceStream() ? 1 : 0);

		std::vector<ID3D11Buffer*> vbs(this_num_vertex_stream);
		std::vector<UINT> strides(this_num_vertex_stream);
		std::vector<UINT> offsets(this_num_vertex_stream);
		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			GraphicsBufferPtr const & stream = rl.GetVertexStream(i);

			D3D11GraphicsBuffer const & d3dvb = *checked_pointer_cast<D3D11GraphicsBuffer>(stream);
			vbs[i] = d3dvb.D3DBuffer().get();
			strides[i] = rl.VertexSize(i);
			offsets[i] = 0;
		}
		if (rl.InstanceStream())
		{
			uint32_t number = rl.NumVertexStreams();
			GraphicsBufferPtr stream = rl.InstanceStream();

			D3D11GraphicsBuffer const & d3dvb = *checked_pointer_cast<D3D11GraphicsBuffer>(stream);
			vbs[number] = d3dvb.D3DBuffer().get();
			strides[number] = rl.InstanceSize();
			offsets[number] = 0;
		}

		if (this_num_vertex_stream != 0)
		{
			d3d_imm_ctx_->IASetVertexBuffers(0, this_num_vertex_stream, &vbs[0], &strides[0], &offsets[0]);

			D3D11RenderLayout const & d3d_rl(*checked_cast<D3D11RenderLayout const *>(&rl));
			D3D11ShaderObjectPtr const & shader = checked_pointer_cast<D3D11ShaderObject>(tech.Pass(0)->GetShaderObject());
			ID3D11InputLayoutPtr const & layout = d3d_rl.InputLayout(shader->VSSignature(), shader->VSCode());
			if (layout != input_layout_cache_)
			{
				d3d_imm_ctx_->IASetInputLayout(layout.get());
				input_layout_cache_ = layout;
			}
		}
		else
		{
			ID3D11Buffer* null_vbs[] = { NULL };
			UINT stride = 0;
			UINT offset = 0;
			d3d_imm_ctx_->IASetVertexBuffers(0, 1, null_vbs, &stride, &offset);
			input_layout_cache_.reset();
			d3d_imm_ctx_->IASetInputLayout(NULL);
		}

		uint32_t const vertex_count = static_cast<uint32_t>(rl.UseIndices() ? rl.NumIndices() : rl.NumVertices());

		if (topology_type_cache_ != rl.TopologyType())
		{
			d3d_imm_ctx_->IASetPrimitiveTopology(D3D11Mapping::Mapping(rl.TopologyType()));
			topology_type_cache_ = rl.TopologyType();
		}

		uint32_t prim_count;
		RenderLayout::topology_type tt = rl.TopologyType();
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
			if ((rl.TopologyType() >= RenderLayout::TT_1_Ctrl_Pt_PatchList)
				&& (rl.TopologyType() <= RenderLayout::TT_32_Ctrl_Pt_PatchList))
			{
				prim_count = vertex_count / 3;
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
		if (rl.InstanceStream())
		{
			if (rl.UseIndices())
			{
				D3D11GraphicsBuffer& d3dib(*checked_pointer_cast<D3D11GraphicsBuffer>(rl.GetIndexStream()));
				d3d_imm_ctx_->IASetIndexBuffer(d3dib.D3DBuffer().get(), D3D11Mapping::MappingFormat(rl.IndexStreamFormat()), 0);

				uint32_t const num_indices = rl.NumIndices();
				uint32_t const num_instances = rl.NumInstance();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					d3d_imm_ctx_->DrawIndexedInstanced(num_indices, num_instances, 0, 0, 0);
					pass->Unbind();
				}
			}
			else
			{
				d3d_imm_ctx_->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);

				uint32_t const num_vertices = rl.NumVertices();
				uint32_t const num_instances = rl.NumInstance();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					d3d_imm_ctx_->DrawInstanced(num_vertices, num_instances, 0, 0);
					pass->Unbind();
				}
			}
		}
		else
		{
			if (rl.UseIndices())
			{
				D3D11GraphicsBuffer& d3dib(*checked_pointer_cast<D3D11GraphicsBuffer>(rl.GetIndexStream()));
				d3d_imm_ctx_->IASetIndexBuffer(d3dib.D3DBuffer().get(), D3D11Mapping::MappingFormat(rl.IndexStreamFormat()), 0);

				uint32_t const num_indices = rl.NumIndices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					d3d_imm_ctx_->DrawIndexed(num_indices, 0, 0);
					pass->Unbind();
				}
			}
			else
			{
				d3d_imm_ctx_->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);

				uint32_t const num_vertices = rl.NumVertices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					d3d_imm_ctx_->Draw(num_vertices, 0);
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

	// 获取模板位数
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t D3D11RenderEngine::StencilBufferBitDepth()
	{
		BOOST_ASSERT(d3d_device_);

		ID3D11DepthStencilView* view;
		d3d_imm_ctx_->OMGetRenderTargets(0, NULL, &view);
		ID3D11DepthStencilViewPtr ds_view = MakeCOMPtr(view);
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ds_view->GetDesc(&desc);

		if (DXGI_FORMAT_D24_UNORM_S8_UINT == desc.Format)
		{
			return 8;
		}
		else
		{
			return 0;
		}
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		D3D11_RECT rc = { x, y, width, height };
		d3d_imm_ctx_->RSSetScissorRects(1, &rc);
	}

	void D3D11RenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_pointer_cast<D3D11RenderWindow>(screen_frame_buffer_)->Resize(width, height);
	}

	bool D3D11RenderEngine::FullScreen() const
	{
		return checked_pointer_cast<D3D11RenderWindow>(screen_frame_buffer_)->FullScreen();
	}

	void D3D11RenderEngine::FullScreen(bool fs)
	{
		checked_pointer_cast<D3D11RenderWindow>(screen_frame_buffer_)->FullScreen(fs);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderEngine::FillRenderDeviceCaps()
	{
		BOOST_ASSERT(d3d_device_);

		if (d3d_feature_level_ <= D3D_FEATURE_LEVEL_9_3)
		{
			caps_.max_vertex_texture_units = 0;
			caps_.max_texture_height = caps_.max_texture_width = 4096;
			caps_.max_texture_depth = 256;
			caps_.max_texture_cube_size = 512;
			caps_.max_texture_array_length = 0;
			caps_.max_simultaneous_rts = 4;
			caps_.max_vertices = 1048575;
			caps_.max_indices = 1048575;
			caps_.alpha_to_coverage_support = false;
		}
		else
		{
			caps_.max_vertex_texture_units = 16;
			caps_.max_texture_height = caps_.max_texture_width = 8192;
			caps_.max_texture_depth = 2048;
			caps_.max_texture_cube_size = 8192;
			caps_.max_texture_array_length = 512;
			caps_.max_simultaneous_rts = 8;
			caps_.max_vertices = 8388607;
			caps_.max_indices = 16777215;
			caps_.alpha_to_coverage_support = true;
		}
		caps_.max_vertex_streams = 16;
		caps_.max_texture_anisotropy = 16;
		caps_.hw_instancing_support = true;
		caps_.depth_texture_support = true;
		caps_.bc1_support = true;
		caps_.bc2_support = true;
		caps_.bc3_support = true;
		{
			D3D11_FEATURE_DATA_THREADING mt_feature;
			d3d_device_->CheckFeatureSupport(D3D11_FEATURE_THREADING, &mt_feature, sizeof(mt_feature));
			caps_.multithread_rendering_support = mt_feature.DriverCommandLists ? true : false;
			caps_.multithread_res_creating_support = mt_feature.DriverConcurrentCreates ? true : false;
		}
		switch (d3d_feature_level_)
		{
		case D3D_FEATURE_LEVEL_11_0:
			caps_.max_shader_model = 5;
			caps_.stream_output_support = true;
			caps_.max_pixel_texture_units = 32;
			caps_.max_geometry_texture_units = 32;
			caps_.primitive_restart_support = true;
			caps_.argb8_support = true;
			caps_.bc4_support = true;
			caps_.bc5_support = true;
			caps_.bc6_support = true;
			caps_.bc7_support = true;
			caps_.gs_support = true;
			caps_.cs_support = true;
			caps_.hs_support = true;
			caps_.ds_support = true;
			break;

		case D3D_FEATURE_LEVEL_10_1:
		case D3D_FEATURE_LEVEL_10_0:
			caps_.max_shader_model = 4;
			caps_.stream_output_support = true;
			caps_.max_pixel_texture_units = 32;
			caps_.max_geometry_texture_units = 32;
			caps_.primitive_restart_support = true;
			caps_.argb8_support = false;
			caps_.bc4_support = true;
			caps_.bc5_support = true;
			caps_.bc6_support = false;
			caps_.bc7_support = false;
			caps_.gs_support = true;
			caps_.hs_support = false;
			caps_.ds_support = false;
			{
				D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS cs4_feature;
				d3d_device_->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &cs4_feature, sizeof(cs4_feature));
				caps_.cs_support = cs4_feature.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x ? true : false;
			}
			break;

		default:
			caps_.max_shader_model = 2;
			caps_.stream_output_support = false;
			caps_.max_pixel_texture_units = 16;
			caps_.max_geometry_texture_units = 0;
			caps_.primitive_restart_support = false;
			caps_.argb8_support = true;
			caps_.bc4_support = false;
			caps_.bc5_support = false;
			caps_.bc6_support = false;
			caps_.bc7_support = false;
			caps_.gs_support = false;
			caps_.cs_support = false;
			caps_.hs_support = false;
			caps_.ds_support = false;
			break;
		}
	}

	void D3D11RenderEngine::StereoscopicForLCDShutter()
	{
		uint32_t const w = stereo_colors_[0]->Width(0);
		uint32_t const h = stereo_colors_[0]->Height(0);
		stereo_colors_[0]->CopyToTexture2D(*stereo_lr_tex_, 0, w, h, 0, 0, w, h, 0, 0);
		stereo_colors_[1]->CopyToTexture2D(*stereo_lr_tex_, 0, w, h, w, 0, w, h, 0, 0);

		ID3D11Texture2DPtr back = checked_pointer_cast<D3D11RenderWindow>(this->ScreenFrameBuffer())->D3DBackBuffer();
		ID3D11Texture2DPtr stereo = checked_pointer_cast<D3D11Texture2D>(stereo_lr_tex_)->D3DTexture();

		D3D11_BOX box;
		box.left = 0;
		box.right = w;
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
			d3d_imm_ctx_->VSSetShader(shader.get(), NULL, 0);
			vertex_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::PSSetShader(ID3D11PixelShaderPtr const & shader)
	{
		if (pixel_shader_cache_ != shader)
		{
			d3d_imm_ctx_->PSSetShader(shader.get(), NULL, 0);
			pixel_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::GSSetShader(ID3D11GeometryShaderPtr const & shader)
	{
		if (geometry_shader_cache_ != shader)
		{
			d3d_imm_ctx_->GSSetShader(shader.get(), NULL, 0);
			geometry_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::CSSetShader(ID3D11ComputeShaderPtr const & shader)
	{
		if (compute_shader_cache_ != shader)
		{
			d3d_imm_ctx_->CSSetShader(shader.get(), NULL, 0);
			compute_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::HSSetShader(ID3D11HullShaderPtr const & shader)
	{
		if (hull_shader_cache_ != shader)
		{
			d3d_imm_ctx_->HSSetShader(shader.get(), NULL, 0);
			hull_shader_cache_ = shader;
		}
	}

	void D3D11RenderEngine::DSSetShader(ID3D11DomainShaderPtr const & shader)
	{
		if (domain_shader_cache_ != shader)
		{
			d3d_imm_ctx_->DSSetShader(shader.get(), NULL, 0);
			domain_shader_cache_ = shader;
		}
	}
}
