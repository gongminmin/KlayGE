// D3D9RenderEngine.cpp
// KlayGE D3D9渲染引擎类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 去掉了固定流水线 (2005.8.18)
//
// 2.8.0
// 增加了RenderDeviceCaps (2005.7.17)
// 简化了StencilBuffer相关操作 (2005.7.20)
//
// 2.7.0
// 改进了Render (2005.6.16)
// 去掉了TextureCoordSet (2005.6.26)
// TextureAddressingMode, TextureFiltering和TextureAnisotropy移到Texture中 (2005.6.27)
//
// 2.4.0
// 增加了PolygonMode (2005.3.20)
//
// 2.0.4
// 去掉了WorldMatrices (2004.4.3)
//
// 2.0.3
// 优化了Render (2004.2.22)
//
// 2.0.1
// 重构了Render (2003.10.10)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/Viewport.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <KlayGE/D3D9/D3D9RenderWindow.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>
#include <KlayGE/D3D9/D3D9RenderEffect.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9RenderLayout.hpp>

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#endif

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9RenderEngine::D3D9RenderEngine()
	{
		// Create our Direct3D object
		d3d_ = MakeCOMPtr(Direct3DCreate9(D3D_SDK_VERSION));
		Verify(d3d_ != ID3D9Ptr());

		adapterList_.Enumerate(d3d_);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9RenderEngine::~D3D9RenderEngine()
	{
		render_tech_.reset();
		cur_render_target_.reset();

		d3dDevice_.reset();
		d3d_.reset();
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & D3D9RenderEngine::Name() const
	{
		static std::wstring const name(L"Direct3D9 Render Engine");
		return name;
	}

	// 获取D3D接口
	/////////////////////////////////////////////////////////////////////////////////
	ID3D9Ptr const & D3D9RenderEngine::D3DObject() const
	{
		return d3d_;
	}

	// 获取D3D Device接口
	/////////////////////////////////////////////////////////////////////////////////
	ID3D9DevicePtr const & D3D9RenderEngine::D3DDevice() const
	{
		return d3dDevice_;
	}

	// 获取D3D适配器列表
	/////////////////////////////////////////////////////////////////////////////////
	D3D9AdapterList const & D3D9RenderEngine::D3DAdapters() const
	{
		return adapterList_;
	}

	// 获取当前适配器
	/////////////////////////////////////////////////////////////////////////////////
	D3D9Adapter const & D3D9RenderEngine::ActiveAdapter() const
	{
		return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
	}

	// 开始渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StartRendering()
	{
		bool gotMsg;
		MSG  msg;

		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

		RenderTarget& renderTarget(*this->CurRenderTarget());
		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if (renderTarget.Active())
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
				if (renderTarget.Active())
				{
					renderTarget.Update();
				}
			}
		}
	}

	// 设置清除颜色
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::ClearColor(Color const & clr)
	{
		clearClr_ = D3DCOLOR_COLORVALUE(clr.r(), clr.g(), clr.b(), clr.a());
	}

	// 清空缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::Clear(uint32_t masks)
	{
		uint32_t flags = 0;
		if (masks & CBM_Color)
		{
			flags |= D3DCLEAR_TARGET;
		}
		if (masks & CBM_Depth)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		if (masks & CBM_Stencil)
		{
			flags |= D3DCLEAR_STENCIL;
		}

		TIF(d3dDevice_->Clear(0, NULL, flags, clearClr_, 1, 0));
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindowPtr D3D9RenderEngine::CreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		D3D9RenderWindowPtr win(new D3D9RenderWindow(d3d_, this->ActiveAdapter(),
			name, settings));
		default_render_target_ = win;

		d3dDevice_ = win->D3DDevice();
		Verify(d3dDevice_ != ID3D9DevicePtr());

		this->FillRenderDeviceCaps();

		this->BindRenderTarget(win);

		bool has_depth_buf = IsDepthFormat(settings.depth_stencil_fmt);
		this->SetRenderState(RST_DepthEnable, has_depth_buf);
		this->SetRenderState(RST_DepthMask, has_depth_buf);

		if (caps_.hw_instancing_support)
		{
			RenderInstance = boost::bind(&D3D9RenderEngine::DoRenderHWInstance, this, _1);
		}
		else
		{
			RenderInstance = boost::bind(&D3D9RenderEngine::DoRenderSWInstance, this, _1);
		}

		this->InitRenderStates();

		return win;
	}

	// 设置当前渲染目标，该渲染目标必须已经在列表中
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoBindRenderTarget(RenderTargetPtr rt)
	{
		BOOST_ASSERT(d3dDevice_);
		BOOST_ASSERT(rt);
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::BeginFrame()
	{
		BOOST_ASSERT(d3dDevice_);

		TIF(d3dDevice_->BeginScene());
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoRender(RenderLayout const & rl)
	{
		if (rl.InstanceStream() && !rl.UseIndices())
		{
			this->DoRenderSWInstance(rl);
		}
		else
		{
			this->RenderInstance(rl);
		}
	}

	void D3D9RenderEngine::DoRenderSWInstance(RenderLayout const & rl)
	{
		BOOST_ASSERT(d3dDevice_);
		BOOST_ASSERT(rl.NumVertexStreams() != 0);
		
		uint32_t const num_instance = rl.NumInstance();

		if (num_instance > 1)
		{
			BOOST_ASSERT(rl.InstanceStream());

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderLayoutPtr tmp_rl = rf.MakeRenderLayout(rl.Type());
			GraphicsBufferPtr tmp_inst_vs;

			rl.ExpandInstance(tmp_inst_vs, 0);

			for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
			{
				tmp_rl->BindVertexStream(rl.GetVertexStream(i), rl.VertexStreamFormat(i));
			}
			tmp_rl->BindVertexStream(tmp_inst_vs, rl.InstanceStreamFormat());
			if (rl.UseIndices())
			{
				tmp_rl->BindIndexStream(rl.GetIndexStream(), rl.IndexStreamFormat());
			}

			this->RenderRLSWInstance(*tmp_rl);

			for (uint32_t i = 1; i < num_instance; ++ i)
			{
				rl.ExpandInstance(tmp_inst_vs, i);
				this->RenderRLSWInstance(*tmp_rl);
			}
		}
		else
		{
			this->RenderRLSWInstance(rl);
		}
	}

	void D3D9RenderEngine::DoRenderHWInstance(RenderLayout const & rl)
	{
		uint32_t const last_num_vertex_stream = static_cast<uint32_t>(active_vertex_streams_.size());
		for (uint32_t i = 0; i < last_num_vertex_stream; ++ i)
		{
			if (active_vertex_streams_[i])
			{
				active_vertex_streams_[i]->Deactive(i);
			}
			TIF(d3dDevice_->SetStreamSourceFreq(i, 1UL));
		}
		active_vertex_streams_.resize(0);

		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			GraphicsBufferPtr stream = rl.GetVertexStream(i);

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			d3d9vb.Active(i, rl.VertexSize(i));

			TIF(d3dDevice_->SetStreamSourceFreq(i,
				D3DSTREAMSOURCE_INDEXEDDATA | rl.VertexStreamFrequency(i)));

			active_vertex_streams_.push_back(boost::static_pointer_cast<D3D9VertexBuffer>(stream));
		}
		if (rl.InstanceStream())
		{
			uint32_t number = rl.NumVertexStreams();
			GraphicsBufferPtr stream = rl.InstanceStream();

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			d3d9vb.Active(number, rl.InstanceSize());

			TIF(d3dDevice_->SetStreamSourceFreq(number,
				D3DSTREAMSOURCE_INSTANCEDATA | 1UL));

			active_vertex_streams_.push_back(boost::static_pointer_cast<D3D9VertexBuffer>(stream));
		}

		this->RenderRL(rl);
	}

	void D3D9RenderEngine::RenderRLSWInstance(RenderLayout const & rl)
	{
		uint32_t const last_num_vertex_stream = static_cast<uint32_t>(active_vertex_streams_.size());
		for (uint32_t i = 0; i < last_num_vertex_stream; ++ i)
		{
			if (active_vertex_streams_[i])
			{
				active_vertex_streams_[i]->Deactive(i);
			}
		}
		active_vertex_streams_.resize(0);

		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			GraphicsBufferPtr stream = rl.GetVertexStream(i);

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			d3d9vb.Active(i, rl.VertexSize(i));

			active_vertex_streams_.push_back(boost::static_pointer_cast<D3D9VertexBuffer>(stream));
		}

		this->RenderRL(rl);
	}

	void D3D9RenderEngine::RenderRL(RenderLayout const & rl)
	{
		D3DPRIMITIVETYPE primType;
		uint32_t primCount;
		D3D9Mapping::Mapping(primType, primCount, rl);

		numPrimitivesJustRendered_ += primCount;
		numVerticesJustRendered_ += rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();

		D3D9RenderLayout const & d3d9_rl(*checked_cast<D3D9RenderLayout const *>(&rl));
		TIF(d3dDevice_->SetVertexDeclaration(d3d9_rl.VertexDeclaration().get()));

		if (active_index_stream_)
		{
			active_index_stream_->Deactive();
			active_index_stream_.reset();
		}

		uint32_t num_passes = render_tech_->NumPasses();
		if (rl.UseIndices())
		{
			D3D9IndexBuffer& d3dib(*checked_pointer_cast<D3D9IndexBuffer>(rl.GetIndexStream()));
			d3dib.SwitchFormat(rl.IndexStreamFormat());
			d3dib.Active();

			for (uint32_t i = 0; i < num_passes; ++ i)
			{
				RenderPassPtr pass = render_tech_->Pass(i);

				pass->Begin();
				TIF(d3dDevice_->DrawIndexedPrimitive(primType, 0, 0,
					static_cast<UINT>(rl.NumVertices()), 0, primCount));
				pass->End();
			}

			active_index_stream_ = boost::static_pointer_cast<D3D9IndexBuffer>(rl.GetIndexStream());
		}
		else
		{
			for (uint32_t i = 0; i < num_passes; ++ i)
			{
				RenderPassPtr pass = render_tech_->Pass(i);

				pass->Begin();
				TIF(d3dDevice_->DrawPrimitive(primType, 0, primCount));
				pass->End();
			}
		}
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::EndFrame()
	{
		BOOST_ASSERT(d3dDevice_);

		TIF(d3dDevice_->EndScene());
	}

	// 初始化渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::InitRenderStates()
	{
		render_states_[RST_PolygonMode]		= PM_Fill;
		render_states_[RST_ShadeMode]		= SM_Gouraud;
		render_states_[RST_CullMode]		= CM_AntiClockwise;
		render_states_[RST_Clipping]		= true;

		render_states_[RST_BlendEnable]		= false;
		render_states_[RST_BlendOp]			= BOP_Add;
		render_states_[RST_SrcBlend]		= ABF_One;
		render_states_[RST_DestBlend]		= ABF_Zero;
		render_states_[RST_BlendOpAlpha]	= BOP_Add;
		render_states_[RST_SrcBlendAlpha]	= ABF_One;
		render_states_[RST_DestBlendAlpha]	= ABF_Zero;
			
		render_states_[RST_DepthEnable]			= true;
		render_states_[RST_DepthMask]			= true;
		render_states_[RST_DepthFunc]			= CF_LessEqual;
		render_states_[RST_PolygonOffsetFactor]	= 0;
		render_states_[RST_PolygonOffsetUnits]	= 0;

		render_states_[RST_FrontStencilEnable]		= false;
		render_states_[RST_FrontStencilFunc]		= CF_AlwaysPass;
		render_states_[RST_FrontStencilRef]			= 0;
		render_states_[RST_FrontStencilMask]		= 0xFFFFFFFF;
		render_states_[RST_FrontStencilFail]		= SOP_Keep;
		render_states_[RST_FrontStencilDepthFail]	= SOP_Keep;
		render_states_[RST_FrontStencilPass]		= SOP_Keep;
		render_states_[RST_FrontStencilWriteMask]	= 0xFFFFFFFF;
		render_states_[RST_BackStencilEnable]		= false;
		render_states_[RST_BackStencilFunc]			= CF_AlwaysPass;
		render_states_[RST_BackStencilRef]			= 0;
		render_states_[RST_BackStencilMask]			= 0xFFFFFFFF;
		render_states_[RST_BackStencilFail]			= SOP_Keep;
		render_states_[RST_BackStencilDepthFail]	= SOP_Keep;
		render_states_[RST_BackStencilPass]			= SOP_Keep;
		render_states_[RST_BackStencilWriteMask]	= 0xFFFFFFFF;

		render_states_[RST_ColorMask0] = 0xF;
		render_states_[RST_ColorMask1] = 0xF;
		render_states_[RST_ColorMask2] = 0xF;
		render_states_[RST_ColorMask3] = 0xF;

		for (size_t i = 0; i < RST_NUM_RENDER_STATES; ++ i)
		{
			dirty_render_states_[i] = false;
		}
	}

	// 刷新渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoFlushRenderStates()
	{
		BOOST_ASSERT(d3dDevice_);

		if (dirty_render_states_[RST_PolygonMode])
		{
			d3dDevice_->SetRenderState(D3DRS_FILLMODE,
				D3D9Mapping::Mapping(static_cast<PolygonMode>(render_states_[RST_PolygonMode])));
		}
		if (dirty_render_states_[RST_ShadeMode])
		{
			d3dDevice_->SetRenderState(D3DRS_SHADEMODE,
				D3D9Mapping::Mapping(static_cast<ShadeMode>(render_states_[RST_ShadeMode])));
		}
		if (dirty_render_states_[RST_CullMode])
		{
			d3dDevice_->SetRenderState(D3DRS_CULLMODE, D3D9Mapping::Mapping(static_cast<CullMode>(render_states_[RST_CullMode])));
		}
		if (dirty_render_states_[RST_Clipping])
		{
			d3dDevice_->SetRenderState(D3DRS_CLIPPING, render_states_[RST_Clipping]);
		}

		if (dirty_render_states_[RST_BlendEnable])
		{
			d3dDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE,
				render_states_[RST_BlendEnable]);
			d3dDevice_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,
				render_states_[RST_BlendEnable]);
		}
		if (dirty_render_states_[RST_BlendOp])
		{
			d3dDevice_->SetRenderState(D3DRS_BLENDOP,
				D3D9Mapping::Mapping(static_cast<BlendOperation>(render_states_[RST_BlendOp])));
		}
		if (dirty_render_states_[RST_SrcBlend])
		{
			d3dDevice_->SetRenderState(D3DRS_SRCBLEND,
				D3D9Mapping::Mapping(static_cast<AlphaBlendFactor>(render_states_[RST_SrcBlend])));
		}
		if (dirty_render_states_[RST_DestBlend])
		{
			d3dDevice_->SetRenderState(D3DRS_DESTBLEND,
				D3D9Mapping::Mapping(static_cast<AlphaBlendFactor>(render_states_[RST_DestBlend])));
		}
		if (dirty_render_states_[RST_BlendOpAlpha])
		{
			d3dDevice_->SetRenderState(D3DRS_BLENDOPALPHA,
				D3D9Mapping::Mapping(static_cast<BlendOperation>(render_states_[RST_BlendOpAlpha])));
		}
		if (dirty_render_states_[RST_SrcBlendAlpha])
		{
			d3dDevice_->SetRenderState(D3DRS_SRCBLENDALPHA,
				D3D9Mapping::Mapping(static_cast<AlphaBlendFactor>(render_states_[RST_SrcBlendAlpha])));
		}
		if (dirty_render_states_[RST_DestBlendAlpha])
		{
			d3dDevice_->SetRenderState(D3DRS_DESTBLENDALPHA,
				D3D9Mapping::Mapping(static_cast<AlphaBlendFactor>(render_states_[RST_DestBlendAlpha])));
		}
			
		if (dirty_render_states_[RST_DepthEnable])
		{
			d3dDevice_->SetRenderState(D3DRS_ZENABLE,
				render_states_[RST_DepthEnable] ? D3DZB_TRUE : D3DZB_FALSE);
		}
		if (dirty_render_states_[RST_DepthMask])
		{
			d3dDevice_->SetRenderState(D3DRS_ZWRITEENABLE,
				render_states_[RST_DepthMask] ? D3DZB_TRUE : D3DZB_FALSE);
		}
		if (dirty_render_states_[RST_DepthFunc])
		{
			d3dDevice_->SetRenderState(D3DRS_ZFUNC,
				D3D9Mapping::Mapping(static_cast<CompareFunction>(render_states_[RST_DepthFunc])));
		}
		if (dirty_render_states_[RST_PolygonOffsetFactor])
		{
			d3dDevice_->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,
				render_states_[RST_PolygonOffsetFactor]);
		}
		if (dirty_render_states_[RST_PolygonOffsetUnits])
		{
			d3dDevice_->SetRenderState(D3DRS_DEPTHBIAS,
				render_states_[RST_PolygonOffsetUnits]);
		}

		if (dirty_render_states_[RST_FrontStencilFunc])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILFUNC,
				D3D9Mapping::Mapping(static_cast<CompareFunction>(render_states_[RST_FrontStencilFunc])));
		}
		if (dirty_render_states_[RST_FrontStencilRef])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILREF,
				render_states_[RST_FrontStencilRef]);
		}
		if (dirty_render_states_[RST_FrontStencilMask])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILMASK,
				render_states_[RST_FrontStencilMask]);
		}
		if (dirty_render_states_[RST_FrontStencilFail])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILFAIL,
				D3D9Mapping::Mapping(static_cast<StencilOperation>(render_states_[RST_FrontStencilFail])));
		}
		if (dirty_render_states_[RST_FrontStencilDepthFail])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILZFAIL,
				D3D9Mapping::Mapping(static_cast<StencilOperation>(render_states_[RST_FrontStencilDepthFail])));
		}
		if (dirty_render_states_[RST_FrontStencilPass])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILPASS,
				D3D9Mapping::Mapping(static_cast<StencilOperation>(render_states_[RST_FrontStencilPass])));
		}
		if (dirty_render_states_[RST_FrontStencilWriteMask])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK,
				render_states_[RST_FrontStencilWriteMask]);
		}
		if (dirty_render_states_[RST_BackStencilFunc])
		{
			d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFUNC,
				D3D9Mapping::Mapping(static_cast<CompareFunction>(render_states_[RST_BackStencilFunc])));
		}
		if (dirty_render_states_[RST_BackStencilRef])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILREF,
				render_states_[RST_BackStencilRef]);
		}
		if (dirty_render_states_[RST_BackStencilMask])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILMASK,
				render_states_[RST_BackStencilMask]);
		}
		if (dirty_render_states_[RST_BackStencilFail])
		{
			d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFAIL,
				D3D9Mapping::Mapping(static_cast<StencilOperation>(render_states_[RST_BackStencilFail])));
		}
		if (dirty_render_states_[RST_BackStencilDepthFail])
		{
			d3dDevice_->SetRenderState(D3DRS_CCW_STENCILZFAIL,
				D3D9Mapping::Mapping(static_cast<StencilOperation>(render_states_[RST_BackStencilDepthFail])));
		}
		if (dirty_render_states_[RST_BackStencilPass])
		{
			d3dDevice_->SetRenderState(D3DRS_CCW_STENCILPASS,
				D3D9Mapping::Mapping(static_cast<StencilOperation>(render_states_[RST_BackStencilPass])));
		}
		if (dirty_render_states_[RST_BackStencilWriteMask])
		{
			d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK,
				render_states_[RST_BackStencilWriteMask]);
		}
		if (dirty_render_states_[RST_FrontStencilEnable] || dirty_render_states_[RST_BackStencilEnable])
		{
			if (render_states_[RST_FrontStencilEnable] && render_states_[RST_BackStencilEnable])
			{
				d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
			}
			else
			{
				if (render_states_[RST_FrontStencilEnable])
				{
					d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, true);
				}
				else
				{
					if (render_states_[RST_BackStencilEnable])
					{
						d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
						d3dDevice_->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
					}
					else
					{
						d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, false);
						d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, false);
					}
				}
			}
		}

		if (dirty_render_states_[RST_ScissorEnable])
		{
			d3dDevice_->SetRenderState(D3DRS_SCISSORTESTENABLE,
				render_states_[RST_ScissorEnable]);
		}

		if (dirty_render_states_[RST_ColorMask0])
		{
			d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE,
				D3D9Mapping::MappingColorMask(render_states_[RST_ColorMask0]));
		}
		if (dirty_render_states_[RST_ColorMask1])
		{
			d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE,
				D3D9Mapping::MappingColorMask(render_states_[RST_ColorMask1]));
		}
		if (dirty_render_states_[RST_ColorMask2])
		{
			d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE,
				D3D9Mapping::MappingColorMask(render_states_[RST_ColorMask2]));
		}
		if (dirty_render_states_[RST_ColorMask3])
		{
			d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE,
				D3D9Mapping::MappingColorMask(render_states_[RST_ColorMask3]));
		}

		for (size_t i = 0; i < RST_NUM_RENDER_STATES; ++ i)
		{
			dirty_render_states_[i] = false;
		}
	}

	// 设置纹理
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::SetSampler(uint32_t stage, SamplerPtr const & sampler)
	{
		BOOST_ASSERT(d3dDevice_);

		if (!sampler || !sampler->GetTexture())
		{
			TIF(d3dDevice_->SetTexture(stage, NULL));
		}
		else
		{
			TexturePtr texture = sampler->GetTexture();

			D3D9Texture const & d3d9Tex(*checked_pointer_cast<D3D9Texture>(texture));
			TIF(d3dDevice_->SetTexture(stage, d3d9Tex.D3DBaseTexture().get()));

			TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_BORDERCOLOR,
				D3D9Mapping::MappingToUInt32Color(sampler->BorderColor())));

			// Set addressing mode
			TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSU,
				D3D9Mapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_U))));
			TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSV,
				D3D9Mapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_V))));
			TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSW,
				D3D9Mapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_W))));

			{
				uint32_t tfc;
				switch (texture->Type())
				{
				case Texture::TT_1D:
					tfc = caps_.texture_1d_filter_caps;
					break;

				case Texture::TT_2D:
					tfc = caps_.texture_2d_filter_caps;
					break;

				case Texture::TT_3D:
					tfc = caps_.texture_3d_filter_caps;
					break;

				case Texture::TT_Cube:
					tfc = caps_.texture_cube_filter_caps;
					break;

				default:
					BOOST_ASSERT(false);
					tfc = 0;
					break;
				}

				Sampler::TexFilterOp filter = sampler->Filtering();
				if (Sampler::TFO_Anisotropic == filter)
				{
					if (0 == (tfc & Sampler::TFO_Anisotropic))
					{
						filter = Sampler::TFO_Trilinear;
					}
				}
				if (Sampler::TFO_Trilinear == filter)
				{
					if (0 == (tfc & Sampler::TFO_Trilinear))
					{
						filter = Sampler::TFO_Bilinear;
					}
				}
				if (Sampler::TFO_Bilinear == filter)
				{
					if (0 == (tfc & Sampler::TFO_Bilinear))
					{
						filter = Sampler::TFO_Point;
					}
				}
				if (Sampler::TFO_Point == filter)
				{
					if (0 == (tfc & Sampler::TFO_Point))
					{
						filter = Sampler::TFO_None;
					}
				}

				// Set filter
				switch (filter)
				{
				case Sampler::TFO_None:
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_NONE));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_NONE));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
					break;

				case Sampler::TFO_Point:
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_POINT));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT));
					break;

				case Sampler::TFO_Bilinear:
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT));
					break;

				case Sampler::TFO_Trilinear:
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
					break;

				case Sampler::TFO_Anisotropic:
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
					TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
					break;
				}
			}

			// Set anisotropy
			BOOST_ASSERT(sampler->Anisotropy() < caps_.max_texture_anisotropy);
			TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, sampler->Anisotropy()));

			// Set max mip level
			TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, sampler->MaxMipLevel()));

			// Set mip map lod bias
			float bias = sampler->MipMapLodBias();
			TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPMAPLODBIAS, *reinterpret_cast<DWORD*>(&bias)));

			TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_SRGBTEXTURE, IsSRGB(texture->Format())));
		}
	}

	// 关闭某个纹理阶段
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DisableSampler(uint32_t stage)
	{
		BOOST_ASSERT(d3dDevice_);

		TIF(d3dDevice_->SetTexture(stage, NULL));
	}

	// 获取模板位数
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t D3D9RenderEngine::StencilBufferBitDepth()
	{
		BOOST_ASSERT(d3dDevice_);

		IDirect3DSurface9* surf;
		D3DSURFACE_DESC surfDesc;
		d3dDevice_->GetDepthStencilSurface(&surf);
		ID3D9SurfacePtr surf_ptr = MakeCOMPtr(surf);
		surf_ptr->GetDesc(&surfDesc);

		if (D3DFMT_D24S8 == surfDesc.Format)
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
	void D3D9RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		RECT rc = { x, y, width, height };
		d3dDevice_->SetScissorRect(&rc);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::FillRenderDeviceCaps()
	{
		BOOST_ASSERT(d3dDevice_);

		D3DCAPS9 d3d_caps;
		d3dDevice_->GetDeviceCaps(&d3d_caps);

		caps_ = D3D9Mapping::Mapping(d3d_caps);
	}

	// 响应设备丢失
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::OnLostDevice()
	{
	}
	
	// 响应设备复位
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::OnResetDevice()
	{
		this->BindRenderTarget(cur_render_target_);
	}
}
