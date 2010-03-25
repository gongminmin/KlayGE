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

#include <KlayGE/D3D9/D3D9RenderWindow.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9RenderLayout.hpp>
#include <KlayGE/D3D9/D3D9ShaderObject.hpp>

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "d3d9.lib")
#endif

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9RenderEngine::D3D9RenderEngine()
		: last_num_vertex_stream_(0),
			conditional_render_(true),
			vertex_shader_cache_(NULL), pixel_shader_cache_(NULL)
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
		cur_frame_buffer_.reset();
		screen_frame_buffer_.reset();
		stereo_frame_buffers_[0].reset();
		stereo_frame_buffers_[1].reset();

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
	void D3D9RenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		motion_frames_ = settings.motion_frames;

		D3D9RenderWindowPtr win = MakeSharedPtr<D3D9RenderWindow>(d3d_, this->ActiveAdapter(),
			name, settings);

		win->Attach(FrameBuffer::ATT_Color0, MakeSharedPtr<D3D9SurfaceRenderView>(win->D3DBackBuffer()));
		if (win->D3DDepthStencilBuffer())
		{
			win->Attach(FrameBuffer::ATT_DepthStencil, MakeSharedPtr<D3D9SurfaceRenderView>(win->D3DDepthStencilBuffer()));
		}

		this->BindFrameBuffer(win);
	}

	void D3D9RenderEngine::D3DDevice(ID3D9DevicePtr const & device)
	{
		d3dDevice_ = device;
		Verify(d3dDevice_ != ID3D9DevicePtr());

		this->FillRenderDeviceCaps();
		this->InitRenderStates();

		if (caps_.hw_instancing_support)
		{
			RenderInstance = boost::bind(&D3D9RenderEngine::DoRenderHWInstance, this, _1, _2);
		}
		else
		{
			RenderInstance = boost::bind(&D3D9RenderEngine::DoRenderSWInstance, this, _1, _2);
		}
	}

	void D3D9RenderEngine::InitRenderStates()
	{
		RasterizerStateDesc default_rs_desc;
		DepthStencilStateDesc default_dss_desc;
		BlendStateDesc default_bs_desc;
		SamplerStateDesc default_ss_desc;

		d3dDevice_->SetRenderState(D3DRS_FILLMODE, D3D9Mapping::Mapping(default_rs_desc.polygon_mode));
		d3dDevice_->SetRenderState(D3DRS_SHADEMODE, D3D9Mapping::Mapping(default_rs_desc.shade_mode));
		d3dDevice_->SetRenderState(D3DRS_CULLMODE, D3D9Mapping::Mapping(default_rs_desc.cull_mode, default_rs_desc.front_face_ccw));
		d3dDevice_->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, float_to_uint32(default_rs_desc.polygon_offset_factor));
		d3dDevice_->SetRenderState(D3DRS_DEPTHBIAS, float_to_uint32(default_rs_desc.polygon_offset_units));
		d3dDevice_->SetRenderState(D3DRS_CLIPPING, default_rs_desc.depth_clip_enable);
		d3dDevice_->SetRenderState(D3DRS_SCISSORTESTENABLE, default_rs_desc.scissor_enable);
		d3dDevice_->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, default_rs_desc.multisample_enable);
		render_states_cache_[D3DRS_FILLMODE] = D3D9Mapping::Mapping(default_rs_desc.polygon_mode);
		render_states_cache_[D3DRS_SHADEMODE] = D3D9Mapping::Mapping(default_rs_desc.shade_mode);
		render_states_cache_[D3DRS_CULLMODE] = D3D9Mapping::Mapping(default_rs_desc.cull_mode, default_rs_desc.front_face_ccw);
		render_states_cache_[D3DRS_SLOPESCALEDEPTHBIAS] = float_to_uint32(default_rs_desc.polygon_offset_factor);
		render_states_cache_[D3DRS_DEPTHBIAS] = float_to_uint32(default_rs_desc.polygon_offset_units);
		render_states_cache_[D3DRS_CLIPPING] = default_rs_desc.depth_clip_enable;
		render_states_cache_[D3DRS_SCISSORTESTENABLE] = default_rs_desc.scissor_enable;
		render_states_cache_[D3DRS_MULTISAMPLEANTIALIAS] = default_rs_desc.multisample_enable;

		d3dDevice_->SetRenderState(D3DRS_ZENABLE, default_dss_desc.depth_enable ? D3DZB_TRUE : D3DZB_FALSE);
		d3dDevice_->SetRenderState(D3DRS_ZWRITEENABLE, default_dss_desc.depth_write_mask ? D3DZB_TRUE : D3DZB_FALSE);
		d3dDevice_->SetRenderState(D3DRS_ZFUNC, D3D9Mapping::Mapping(default_dss_desc.depth_func));
		render_states_cache_[D3DRS_ZENABLE] = default_dss_desc.depth_enable ? D3DZB_TRUE : D3DZB_FALSE;
		render_states_cache_[D3DRS_ZWRITEENABLE] = default_dss_desc.depth_write_mask ? D3DZB_TRUE : D3DZB_FALSE;
		render_states_cache_[D3DRS_ZFUNC] = D3D9Mapping::Mapping(default_dss_desc.depth_func);

		if (default_dss_desc.front_stencil_enable && default_dss_desc.back_stencil_enable)
		{
			d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
			render_states_cache_[D3DRS_TWOSIDEDSTENCILMODE] = true;
		}
		else
		{
			if (default_dss_desc.front_stencil_enable)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, true);
				render_states_cache_[D3DRS_STENCILENABLE] = true;
			}
			else
			{
				if (default_dss_desc.back_stencil_enable)
				{
					d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
					d3dDevice_->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
					render_states_cache_[D3DRS_TWOSIDEDSTENCILMODE] = true;
					render_states_cache_[D3DRS_STENCILFUNC] = D3DCMP_ALWAYS;
				}
				else
				{
					d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, false);
					d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, false);
					render_states_cache_[D3DRS_STENCILENABLE] = false;
					render_states_cache_[D3DRS_TWOSIDEDSTENCILMODE] = false;
				}
			}
		}

		d3dDevice_->SetRenderState(D3DRS_STENCILFUNC, D3D9Mapping::Mapping(default_dss_desc.front_stencil_func));
		d3dDevice_->SetRenderState(D3DRS_STENCILREF, 0);
		d3dDevice_->SetRenderState(D3DRS_STENCILMASK, default_dss_desc.front_stencil_read_mask);
		d3dDevice_->SetRenderState(D3DRS_STENCILFAIL, D3D9Mapping::Mapping(default_dss_desc.front_stencil_fail));
		d3dDevice_->SetRenderState(D3DRS_STENCILZFAIL, D3D9Mapping::Mapping(default_dss_desc.front_stencil_depth_fail));
		d3dDevice_->SetRenderState(D3DRS_STENCILPASS, D3D9Mapping::Mapping(default_dss_desc.front_stencil_pass));
		d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, default_dss_desc.front_stencil_write_mask);
		render_states_cache_[D3DRS_STENCILFUNC] = D3D9Mapping::Mapping(default_dss_desc.front_stencil_func);
		render_states_cache_[D3DRS_STENCILREF] = 0;
		render_states_cache_[D3DRS_STENCILMASK] = default_dss_desc.front_stencil_read_mask;
		render_states_cache_[D3DRS_STENCILFAIL] = D3D9Mapping::Mapping(default_dss_desc.front_stencil_fail);
		render_states_cache_[D3DRS_STENCILZFAIL] = D3D9Mapping::Mapping(default_dss_desc.front_stencil_depth_fail);
		render_states_cache_[D3DRS_STENCILPASS] = D3D9Mapping::Mapping(default_dss_desc.front_stencil_pass);
		render_states_cache_[D3DRS_STENCILWRITEMASK] = default_dss_desc.front_stencil_write_mask;

		d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFUNC, D3D9Mapping::Mapping(default_dss_desc.back_stencil_func));
		d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFAIL, D3D9Mapping::Mapping(default_dss_desc.back_stencil_fail));
		d3dDevice_->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3D9Mapping::Mapping(default_dss_desc.back_stencil_depth_fail));
		d3dDevice_->SetRenderState(D3DRS_CCW_STENCILPASS, D3D9Mapping::Mapping(default_dss_desc.back_stencil_pass));
		render_states_cache_[D3DRS_CCW_STENCILFUNC] = D3D9Mapping::Mapping(default_dss_desc.back_stencil_func);
		render_states_cache_[D3DRS_CCW_STENCILFAIL] = D3D9Mapping::Mapping(default_dss_desc.back_stencil_fail);
		render_states_cache_[D3DRS_CCW_STENCILZFAIL] = D3D9Mapping::Mapping(default_dss_desc.back_stencil_depth_fail);
		render_states_cache_[D3DRS_CCW_STENCILPASS] = D3D9Mapping::Mapping(default_dss_desc.back_stencil_pass);

		if (this->DeviceCaps().alpha_to_coverage_support)
		{
			if (default_bs_desc.alpha_to_coverage_enable)
			{
				d3dDevice_->SetRenderState(D3DRS_ADAPTIVETESS_Y,
					static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value));
				render_states_cache_[D3DRS_ADAPTIVETESS_Y] = static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value);
			}
			else
			{
				d3dDevice_->SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
				render_states_cache_[D3DRS_ADAPTIVETESS_Y] = D3DFMT_UNKNOWN;
			}
		}
		d3dDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, default_bs_desc.blend_enable[0]);
		d3dDevice_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		d3dDevice_->SetRenderState(D3DRS_BLENDOP, D3D9Mapping::Mapping(default_bs_desc.blend_op[0]));
		d3dDevice_->SetRenderState(D3DRS_SRCBLEND, D3D9Mapping::Mapping(default_bs_desc.src_blend[0]));
		d3dDevice_->SetRenderState(D3DRS_DESTBLEND, D3D9Mapping::Mapping(default_bs_desc.dest_blend[0]));
		d3dDevice_->SetRenderState(D3DRS_BLENDOPALPHA, D3D9Mapping::Mapping(default_bs_desc.blend_op_alpha[0]));
		d3dDevice_->SetRenderState(D3DRS_SRCBLENDALPHA, D3D9Mapping::Mapping(default_bs_desc.src_blend_alpha[0]));
		d3dDevice_->SetRenderState(D3DRS_DESTBLENDALPHA, D3D9Mapping::Mapping(default_bs_desc.dest_blend_alpha[0]));
		d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE, D3D9Mapping::MappingColorMask(default_bs_desc.color_write_mask[0]));
		d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE1, D3D9Mapping::MappingColorMask(default_bs_desc.color_write_mask[1]));
		d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE2, D3D9Mapping::MappingColorMask(default_bs_desc.color_write_mask[2]));
		d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE3, D3D9Mapping::MappingColorMask(default_bs_desc.color_write_mask[3]));
		render_states_cache_[D3DRS_ALPHABLENDENABLE] = default_bs_desc.blend_enable[0];
		render_states_cache_[D3DRS_SEPARATEALPHABLENDENABLE] = true;
		render_states_cache_[D3DRS_BLENDOP] = D3D9Mapping::Mapping(default_bs_desc.blend_op[0]);
		render_states_cache_[D3DRS_SRCBLEND] = D3D9Mapping::Mapping(default_bs_desc.src_blend[0]);
		render_states_cache_[D3DRS_DESTBLEND] = D3D9Mapping::Mapping(default_bs_desc.dest_blend[0]);
		render_states_cache_[D3DRS_BLENDOPALPHA] = D3D9Mapping::Mapping(default_bs_desc.blend_op_alpha[0]);
		render_states_cache_[D3DRS_SRCBLENDALPHA] = D3D9Mapping::Mapping(default_bs_desc.src_blend_alpha[0]);
		render_states_cache_[D3DRS_DESTBLENDALPHA] = D3D9Mapping::Mapping(default_bs_desc.dest_blend_alpha[0]);
		render_states_cache_[D3DRS_COLORWRITEENABLE] = D3D9Mapping::MappingColorMask(default_bs_desc.color_write_mask[0]);
		render_states_cache_[D3DRS_COLORWRITEENABLE1] = D3D9Mapping::MappingColorMask(default_bs_desc.color_write_mask[1]);
		render_states_cache_[D3DRS_COLORWRITEENABLE2] = D3D9Mapping::MappingColorMask(default_bs_desc.color_write_mask[2]);
		render_states_cache_[D3DRS_COLORWRITEENABLE3] = D3D9Mapping::MappingColorMask(default_bs_desc.color_write_mask[3]);

		samplers_cache_[0].resize(this->DeviceCaps().max_vertex_texture_units);
		samplers_cache_[1].resize(this->DeviceCaps().max_pixel_texture_units);
		for (int type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			for (uint32_t i = 0, i_end = static_cast<uint32_t>(samplers_cache_[type].size()); i < i_end; ++ i)
			{
				uint32_t stage = i;
				if (ShaderObject::ST_VertexShader == type)
				{
					stage += D3DVERTEXTEXTURESAMPLER0;
				}

				d3dDevice_->SetTexture(stage, NULL);
				d3dDevice_->SetSamplerState(stage, D3DSAMP_SRGBTEXTURE, false);
				d3dDevice_->SetSamplerState(stage, D3DSAMP_BORDERCOLOR, D3D9Mapping::MappingToUInt32Color(default_ss_desc.border_clr));
				d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSU, D3D9Mapping::Mapping(default_ss_desc.addr_mode_u));
				d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSV, D3D9Mapping::Mapping(default_ss_desc.addr_mode_v));
				d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSW, D3D9Mapping::Mapping(default_ss_desc.addr_mode_w));
				samplers_cache_[type][i].first = NULL;
				samplers_cache_[type][i].second[D3DSAMP_SRGBTEXTURE] = false;
				samplers_cache_[type][i].second[D3DSAMP_BORDERCOLOR] = D3D9Mapping::MappingToUInt32Color(default_ss_desc.border_clr);
				samplers_cache_[type][i].second[D3DSAMP_ADDRESSU] = D3D9Mapping::Mapping(default_ss_desc.addr_mode_u);
				samplers_cache_[type][i].second[D3DSAMP_ADDRESSV] = D3D9Mapping::Mapping(default_ss_desc.addr_mode_v);
				samplers_cache_[type][i].second[D3DSAMP_ADDRESSW] = D3D9Mapping::Mapping(default_ss_desc.addr_mode_w);

				uint32_t min_filter, mag_filter, mip_filter;
				if (default_ss_desc.filter & TFOE_Min_Linear)
				{
					min_filter = D3DTEXF_LINEAR;
				}
				else
				{
					min_filter = D3DTEXF_POINT;
				}
				if (default_ss_desc.filter & TFOE_Mag_Linear)
				{
					mag_filter = D3DTEXF_LINEAR;
				}
				else
				{
					mag_filter = D3DTEXF_POINT;
				}
				if (default_ss_desc.filter & TFOE_Mip_Linear)
				{
					mip_filter = D3DTEXF_LINEAR;
				}
				else
				{
					mip_filter = D3DTEXF_POINT;
				}
				if (default_ss_desc.filter & TFOE_Anisotropic)
				{
					min_filter = D3DTEXF_ANISOTROPIC;
				}
				d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, min_filter);
				d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, mag_filter);
				d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, mip_filter);
				samplers_cache_[type][i].second[D3DSAMP_MINFILTER] = min_filter;
				samplers_cache_[type][i].second[D3DSAMP_MAGFILTER] = mag_filter;
				samplers_cache_[type][i].second[D3DSAMP_MIPFILTER] = mip_filter;

				d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, default_ss_desc.max_anisotropy);
				d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, static_cast<DWORD>(default_ss_desc.max_lod));
				d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPMAPLODBIAS, float_to_uint32(default_ss_desc.mip_map_lod_bias));
				samplers_cache_[type][i].second[D3DSAMP_MAXANISOTROPY] = default_ss_desc.max_anisotropy;
				samplers_cache_[type][i].second[D3DSAMP_MAXMIPLEVEL] = static_cast<DWORD>(default_ss_desc.max_lod);
				samplers_cache_[type][i].second[D3DSAMP_MIPMAPLODBIAS] = float_to_uint32(default_ss_desc.mip_map_lod_bias);
			}
		}

		vertex_shader_cache_ = NULL;
		pixel_shader_cache_ = NULL;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRasterizerStateObject(default_rs_desc);
		cur_dss_obj_ = rf.MakeDepthStencilStateObject(default_dss_desc);
		cur_bs_obj_ = rf.MakeBlendStateObject(default_bs_desc);
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
	{
		UNREF_PARAM(fb);

		BOOST_ASSERT(d3dDevice_);
		BOOST_ASSERT(fb);
	}

	// 设置当前Stream output目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoBindSOBuffers(RenderLayoutPtr const & /*rl*/)
	{
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::BeginFrame()
	{
		BOOST_ASSERT(d3dDevice_);

		TIF(d3dDevice_->BeginScene());
	}

	// 开始一个Pass
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::BeginPass()
	{
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoRender(RenderTechnique const & tech, RenderLayout const & rl)
	{
		if (conditional_render_)
		{
			if (rl.InstanceStream() && !rl.UseIndices())
			{
				this->DoRenderSWInstance(tech, rl);
			}
			else
			{
				this->RenderInstance(tech, rl);
			}
		}
	}

	void D3D9RenderEngine::DoDispatch(RenderTechnique const & /*tech*/, uint32_t /*tgx*/, uint32_t /*tgy*/, uint32_t /*tgz*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9RenderEngine::DoRenderSWInstance(RenderTechnique const & tech, RenderLayout const & rl)
	{
		BOOST_ASSERT(d3dDevice_);
		BOOST_ASSERT(rl.NumVertexStreams() != 0);

		uint32_t const num_instance = rl.NumInstance();

		if (num_instance > 1)
		{
			BOOST_ASSERT(rl.InstanceStream());

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderLayoutPtr tmp_rl = rf.MakeRenderLayout();
			tmp_rl->TopologyType(rl.TopologyType());
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

			this->RenderRLSWInstance(tech, *tmp_rl);

			for (uint32_t i = 1; i < num_instance; ++ i)
			{
				rl.ExpandInstance(tmp_inst_vs, i);
				this->RenderRLSWInstance(tech, *tmp_rl);
			}
		}
		else
		{
			this->RenderRLSWInstance(tech, rl);
		}
	}

	void D3D9RenderEngine::DoRenderHWInstance(RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t this_num_vertex_stream = rl.NumVertexStreams();
		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			GraphicsBufferPtr const & stream = rl.GetVertexStream(i);

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			TIF(d3dDevice_->SetStreamSource(i, d3d9vb.D3D9Buffer().get(), 0, rl.VertexSize(i)));
		}
		if (rl.InstanceStream())
		{
			for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
			{
				TIF(d3dDevice_->SetStreamSourceFreq(i,
					D3DSTREAMSOURCE_INDEXEDDATA | rl.VertexStreamFrequency(i)));
			}

			uint32_t number = rl.NumVertexStreams();
			GraphicsBufferPtr stream = rl.InstanceStream();

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			TIF(d3dDevice_->SetStreamSource(number, d3d9vb.D3D9Buffer().get(), 0, rl.InstanceSize()));

			TIF(d3dDevice_->SetStreamSourceFreq(number,
				D3DSTREAMSOURCE_INSTANCEDATA | 1UL));

			++ this_num_vertex_stream;
		}
		else
		{
			for (uint32_t i = 0; i < rl.NumVertexStreams() + 1; ++ i)
			{
				TIF(d3dDevice_->SetStreamSourceFreq(i, 1UL));
			}
		}

		for (uint32_t i = this_num_vertex_stream; i < last_num_vertex_stream_; ++ i)
		{
			TIF(d3dDevice_->SetStreamSource(i, NULL, 0, 0));
			TIF(d3dDevice_->SetStreamSourceFreq(i, 1UL));
		}

		last_num_vertex_stream_ = this_num_vertex_stream;

		this->RenderRL(tech, rl);
	}

	void D3D9RenderEngine::RenderRLSWInstance(RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t this_num_vertex_stream = rl.NumVertexStreams();
		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			GraphicsBufferPtr const & stream = rl.GetVertexStream(i);

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			TIF(d3dDevice_->SetStreamSource(i, d3d9vb.D3D9Buffer().get(), 0, rl.VertexSize(i)));
		}

		for (uint32_t i = this_num_vertex_stream; i < last_num_vertex_stream_; ++ i)
		{
			TIF(d3dDevice_->SetStreamSource(i, NULL, 0, 0));
			TIF(d3dDevice_->SetStreamSourceFreq(i, 1UL));
		}

		last_num_vertex_stream_ = this_num_vertex_stream;

		this->RenderRL(tech, rl);
	}

	void D3D9RenderEngine::RenderRL(RenderTechnique const & tech, RenderLayout const & rl)
	{
		D3DPRIMITIVETYPE primType;
		uint32_t primCount;
		D3D9Mapping::Mapping(primType, primCount, rl);

		numPrimitivesJustRendered_ += primCount;
		numVerticesJustRendered_ += rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();

		D3D9RenderLayout const & d3d9_rl(*checked_cast<D3D9RenderLayout const *>(&rl));
		TIF(d3dDevice_->SetVertexDeclaration(d3d9_rl.VertexDeclaration().get()));

		uint32_t num_passes = tech.NumPasses();
		if (rl.UseIndices())
		{
			D3D9IndexBuffer& d3dib(*checked_pointer_cast<D3D9IndexBuffer>(rl.GetIndexStream()));
			d3dib.SwitchFormat(rl.IndexStreamFormat());
			d3dDevice_->SetIndices(d3dib.D3D9Buffer().get());

			for (uint32_t i = 0; i < num_passes; ++ i)
			{
				RenderPassPtr pass = tech.Pass(i);

				pass->Bind();
				TIF(d3dDevice_->DrawIndexedPrimitive(primType, 0, 0,
					static_cast<UINT>(rl.NumVertices()), 0, primCount));
				pass->Unbind();
			}
		}
		else
		{
			d3dDevice_->SetIndices(NULL);

			for (uint32_t i = 0; i < num_passes; ++ i)
			{
				RenderPassPtr pass = tech.Pass(i);

				pass->Bind();
				TIF(d3dDevice_->DrawPrimitive(primType, 0, primCount));
				pass->Unbind();
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

	// 结束一个Pass
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::EndPass()
	{
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

	void D3D9RenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_pointer_cast<D3D9RenderWindow>(screen_frame_buffer_)->Resize(width, height);
	}

	bool D3D9RenderEngine::FullScreen() const
	{
		return checked_pointer_cast<D3D9RenderWindow>(screen_frame_buffer_)->FullScreen();
	}

	void D3D9RenderEngine::FullScreen(bool fs)
	{
		checked_pointer_cast<D3D9RenderWindow>(screen_frame_buffer_)->FullScreen(fs);
	}

	void D3D9RenderEngine::ConditionalRender(bool cr)
	{
		conditional_render_ = cr;
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
		this->InitRenderStates();
	}

	// 响应设备复位
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::OnResetDevice()
	{
		this->BindFrameBuffer(cur_frame_buffer_);
	}

	void D3D9RenderEngine::SetRenderState(D3DRENDERSTATETYPE state, uint32_t value)
	{
		if (render_states_cache_[state] != value)
		{
			d3dDevice_->SetRenderState(state, value);
			render_states_cache_[state] = value;
		}
	}

	void D3D9RenderEngine::SetTexture(uint32_t sampler, IDirect3DBaseTexture9* texture)
	{
		ShaderObject::ShaderType st;
		uint32_t stage;
		if (sampler < D3DVERTEXTEXTURESAMPLER0)
		{
			st = ShaderObject::ST_PixelShader;
			stage = sampler;
		}
		else
		{
			st = ShaderObject::ST_VertexShader;
			stage = sampler - D3DVERTEXTEXTURESAMPLER0;
		}

		if (samplers_cache_[st][stage].first != texture)
		{
			d3dDevice_->SetTexture(sampler, texture);
			samplers_cache_[st][stage].first = texture;
		}
	}

	void D3D9RenderEngine::SetSamplerState(uint32_t sampler, D3DSAMPLERSTATETYPE type, uint32_t value)
	{
		ShaderObject::ShaderType st;
		uint32_t stage;
		if (sampler < D3DVERTEXTEXTURESAMPLER0)
		{
			st = ShaderObject::ST_PixelShader;
			stage = sampler;
		}
		else
		{
			st = ShaderObject::ST_VertexShader;
			stage = sampler - D3DVERTEXTEXTURESAMPLER0;
		}

		if (samplers_cache_[st][stage].second[type] != value)
		{
			d3dDevice_->SetSamplerState(sampler, type, value);
			samplers_cache_[st][stage].second[type] = value;
		}
	}

	void D3D9RenderEngine::SetVertexShader(IDirect3DVertexShader9* shader)
	{
		if (vertex_shader_cache_ != shader)
		{
			d3dDevice_->SetVertexShader(shader);
			vertex_shader_cache_ = shader;
		}
	}

	void D3D9RenderEngine::SetPixelShader(IDirect3DPixelShader9* shader)
	{
		if (pixel_shader_cache_ != shader)
		{
			d3dDevice_->SetPixelShader(shader);
			pixel_shader_cache_ = shader;
		}
	}

	void D3D9RenderEngine::SetVertexShaderConstantB(uint32_t start_reg, BOOL const * constant_data, uint32_t reg_count)
	{
		d3dDevice_->SetVertexShaderConstantB(start_reg, constant_data, reg_count);
	}

	void D3D9RenderEngine::SetPixelShaderConstantB(uint32_t start_reg, BOOL const * constant_data, uint32_t reg_count)
	{
		d3dDevice_->SetPixelShaderConstantB(start_reg, constant_data, reg_count);
	}

	void D3D9RenderEngine::SetVertexShaderConstantI(uint32_t start_reg, int const * constant_data, uint32_t reg_count)
	{
		d3dDevice_->SetVertexShaderConstantI(start_reg, constant_data, reg_count);
	}

	void D3D9RenderEngine::SetPixelShaderConstantI(uint32_t start_reg, int const * constant_data, uint32_t reg_count)
	{
		d3dDevice_->SetPixelShaderConstantI(start_reg, constant_data, reg_count);
	}

	void D3D9RenderEngine::SetVertexShaderConstantF(uint32_t start_reg, float const * constant_data, uint32_t reg_count)
	{
		d3dDevice_->SetVertexShaderConstantF(start_reg, constant_data, reg_count);
	}

	void D3D9RenderEngine::SetPixelShaderConstantF(uint32_t start_reg, float const * constant_data, uint32_t reg_count)
	{
		d3dDevice_->SetPixelShaderConstantF(start_reg, constant_data, reg_count);
	}
}
