// RenderEngine.cpp
// KlayGE 渲染引擎类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2003-2009
// Homepage: http://www.klayge.org
//
// 3.10.0
// 增加了Dispatch (2009.12.22)
//
// 3.9.0
// 支持Stream Output (2009.5.14)
//
// 3.6.0
// 去掉了RenderTarget，直接使用FrameBuffer (2007.6.20)
//
// 3.3.0
// 统一了RenderState (2006.5.21)
//
// 2.8.0
// 简化了StencilBuffer相关操作 (2005.7.20)
//
// 2.7.1
// ViewMatrix和ProjectionMatrix改为const (2005.7.10)
//
// 2.4.0
// 增加了NumPrimitivesJustRendered和NumVerticesJustRendered (2005.3.21)
//
// 2.0.3
// 优化了RenderEffect的设置 (2004.2.16)
// 去掉了VO_2D (2004.3.1)
// 去掉了SoftwareBlend (2004.3.10)
//
// 2.0.0
// 初次建立(2003.10.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/PerfProfiler.hpp>

#include <boost/lexical_cast.hpp>

#include <KlayGE/RenderEngine.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderEngine()
		: num_primitives_just_rendered_(0), num_vertices_just_rendered_(0),
			num_draws_just_called_(0), num_dispatches_just_called_(0),
			cur_front_stencil_ref_(0),
			cur_back_stencil_ref_(0),
			cur_blend_factor_(1, 1, 1, 1),
			cur_sample_mask_(0xFFFFFFFF),
			default_fov_(PI / 4), default_render_width_scale_(1), default_render_height_scale_(1),
			motion_frames_(0),
			stereo_method_(STM_None), stereo_separation_(0),
			fb_stage_(0), force_line_mode_(false)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::~RenderEngine()
	{
	}

	void RenderEngine::Suspend()
	{
		// TODO
		this->DoSuspend();
	}

	void RenderEngine::Resume()
	{
		// TODO
		this->DoResume();
	}

	void RenderEngine::BeginFrame()
	{
		this->BindFrameBuffer(default_frame_buffers_[0]);
	}

	void RenderEngine::BeginPass()
	{
	}

	void RenderEngine::EndPass()
	{
	}

	void RenderEngine::EndFrame()
	{
	}

	void RenderEngine::UpdateGPUTimestampsFrequency()
	{
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::CreateRenderWindow(std::string const & name, RenderSettings& settings)
	{
		if (settings.stereo_method != STM_OculusVR)
		{
			stereo_separation_ = settings.stereo_separation;
		}
		this->DoCreateRenderWindow(name, settings);
		this->CheckConfig(settings);
		RenderDeviceCaps const & caps = this->DeviceCaps();

		screen_frame_buffer_ = cur_frame_buffer_;

		uint32_t const screen_width = screen_frame_buffer_->Width();
		uint32_t const screen_height = screen_frame_buffer_->Height();
		float const screen_aspect = static_cast<float>(screen_width) / screen_height;
		if (!MathLib::equal(screen_aspect, static_cast<float>(settings.width) / settings.height))
		{
			settings.width = static_cast<uint32_t>(settings.height * screen_aspect + 0.5f);
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		pp_rl_ = rf.MakeRenderLayout();
		pp_rl_->TopologyType(RenderLayout::TT_TriangleStrip);

		float2 pos[] =
		{
			float2(-1, +1),
			float2(+1, +1),
			float2(-1, -1),
			float2(+1, -1)
		};
		GraphicsBufferPtr pp_pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pos), &pos[0]);
		pp_rl_->BindVertexStream(pp_pos_vb, std::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

		uint32_t const render_width = static_cast<uint32_t>(settings.width * default_render_width_scale_ + 0.5f);
		uint32_t const render_height = static_cast<uint32_t>(settings.height * default_render_height_scale_ + 0.5f);

		hdr_enabled_ = settings.hdr;
		if (settings.hdr)
		{
			hdr_pp_ = MakeSharedPtr<HDRPostProcess>(settings.fft_lens_effects);
			skip_hdr_pp_ = SyncLoadPostProcess("Copy.ppml", "copy");
		}

		ppaa_enabled_ = settings.ppaa ? 1 : 0;
		gamma_enabled_ = settings.gamma;
		color_grading_enabled_ = settings.color_grading;
		if (settings.ppaa || settings.color_grading || settings.gamma)
		{
			for (size_t i = 0; i < 12; ++ i)
			{
				ldr_pps_[i] = SyncLoadPostProcess("PostToneMapping.ppml",
					"PostToneMapping" + boost::lexical_cast<std::string>(i));
			}

			ldr_pp_ = ldr_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}

		bool need_resize = false;
		if (!settings.hide_win)
		{
			need_resize = ((render_width != screen_width) || (render_height != screen_height));

			resize_pps_[0] = SyncLoadPostProcess("Resizer.ppml", "bilinear");
			resize_pps_[1] = MakeSharedPtr<BicubicFilteringPostProcess>();

			float const scale_x = static_cast<float>(screen_width) / render_width;
			float const scale_y = static_cast<float>(screen_height) / render_height;

			float2 pos_scale;
			if (scale_x < scale_y)
			{
				pos_scale.x() = 1;
				pos_scale.y() = (scale_x * render_height) / screen_height;
			}
			else
			{
				pos_scale.x() = (scale_y * render_width) / screen_width;
				pos_scale.y() = 1;
			}

			for (size_t i = 0; i < 2; ++ i)
			{
				resize_pps_[i]->SetParam(0, pos_scale);
			}
		}

		for (int i = 0; i < 4; ++ i)
		{
			default_frame_buffers_[i] = screen_frame_buffer_;
		}

		RenderViewPtr ds_view;
		if (hdr_pp_ || ldr_pp_ || (settings.stereo_method != STM_None))
		{
			ds_tex_ = this->ScreenDepthStencilTexture();
			if (ds_tex_ && (screen_width == render_width) && (screen_height == render_height))
			{
				ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
			}
			else
			{
				if (caps.texture_format_support(EF_D32F) || caps.texture_format_support(EF_D24S8)
					|| caps.texture_format_support(EF_D16))
				{
					ElementFormat fmt;
					if ((settings.depth_stencil_fmt != EF_Unknown)
						&& caps.texture_format_support(settings.depth_stencil_fmt))
					{
						fmt = settings.depth_stencil_fmt;
					}
					else
					{
						BOOST_ASSERT(caps.texture_format_support(EF_D16));

						fmt = EF_D16;
					}
					ds_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
					ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
				}
				else
				{
					ElementFormat fmt;
					if ((settings.depth_stencil_fmt != EF_Unknown)
						&& caps.rendertarget_format_support(settings.depth_stencil_fmt, 1, 0))
					{
						fmt = settings.depth_stencil_fmt;
					}
					else
					{
						BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

						fmt = EF_D16;
					}
					ds_view = rf.Make2DDepthStencilRenderView(render_width, render_height, fmt, 1, 0);
				}
			}
		}

		if (settings.stereo_method != STM_None)
		{
			mono_frame_buffer_ = rf.MakeFrameBuffer();
			mono_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.texture_format_support(settings.color_fmt) && caps.rendertarget_format_support(settings.color_fmt, 1, 0))
			{
				fmt = settings.color_fmt;
			}
			else
			{
				if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
				{
					fmt = EF_ABGR8;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));

					fmt = EF_ARGB8;
				}
			}

			mono_tex_ = rf.MakeTexture2D(screen_width, screen_height, 1, 1,
				fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			mono_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*mono_tex_, 0, 1, 0));

			default_frame_buffers_[0] = default_frame_buffers_[1]
				= default_frame_buffers_[2] = mono_frame_buffer_;

			overlay_frame_buffer_ = rf.MakeFrameBuffer();
			overlay_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			overlay_tex_ = rf.MakeTexture2D(screen_width, screen_height, 1, 1,
				fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			overlay_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*overlay_tex_, 0, 1, 0));

			RenderViewPtr screen_size_ds_view;
			if (need_resize)
			{
				screen_size_ds_view = rf.Make2DDepthStencilRenderView(screen_width, screen_height, ds_view->Format(), 1, 0);
			}
			else
			{
				screen_size_ds_view = ds_view;
			}
			overlay_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, screen_size_ds_view);
		}
		else
		{
			if (need_resize)
			{
				resize_frame_buffer_ = rf.MakeFrameBuffer();
				resize_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

				ElementFormat fmt;
				if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
				{
					fmt = EF_ABGR8;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));

					fmt = EF_ARGB8;
				}

				resize_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1,
					fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				resize_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*resize_tex_, 0, 1, 0));

				ElementFormat ds_fmt;
				if ((settings.depth_stencil_fmt != EF_Unknown) && caps.rendertarget_format_support(settings.depth_stencil_fmt, 1, 0))
				{
					ds_fmt = settings.depth_stencil_fmt;
				}
				else
				{
					BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

					ds_fmt = EF_D16;
				}
				resize_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil,
					rf.Make2DDepthStencilRenderView(render_width, render_height, ds_fmt, 1, 0));

				default_frame_buffers_[0] = default_frame_buffers_[1]
					= default_frame_buffers_[2] = resize_frame_buffer_;
			}
		}
		
		if (ldr_pp_)
		{
			ldr_frame_buffer_ = rf.MakeFrameBuffer();
			ldr_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));

				fmt = EF_ARGB8;
			}
			ElementFormat fmt_srgb = MakeSRGB(fmt);
			if (caps.texture_format_support(fmt_srgb) && caps.rendertarget_format_support(fmt_srgb, 1, 0))
			{
				fmt = fmt_srgb;
			}

			ldr_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			ldr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ldr_tex_, 0, 1, 0));
			ldr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			default_frame_buffers_[0] = default_frame_buffers_[1] = ldr_frame_buffer_;
		}

		if (hdr_pp_)
		{
			hdr_frame_buffer_ = rf.MakeFrameBuffer();
			hdr_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.fp_color_support)
			{
				if (caps.texture_format_support(EF_B10G11R11F) && caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
				{
					fmt = EF_B10G11R11F;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_ABGR16F) && caps.rendertarget_format_support(EF_ABGR16F, 1, 0));
					fmt = EF_ABGR16F;
				}
			}
			else
			{
				if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
				{
					fmt = EF_ABGR8;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));
					fmt = EF_ARGB8;
				}

				ElementFormat fmt_srgb = MakeSRGB(fmt);
				if (caps.rendertarget_format_support(fmt_srgb, 1, 0))
				{
					fmt = fmt_srgb;
				}
			}
			hdr_tex_ = rf.MakeTexture2D(render_width, render_height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			hdr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0, 1, 0));
			hdr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			default_frame_buffers_[0] = hdr_frame_buffer_;
		}

		this->BindFrameBuffer(default_frame_buffers_[0]);
		this->Stereo(settings.stereo_method);

#ifndef KLAYGE_SHIP
		PerfProfiler& profiler = PerfProfiler::Instance();
		hdr_pp_perf_ = profiler.CreatePerfRange(0, "HDR PP");
		ldr_pp_perf_ = profiler.CreatePerfRange(0, "LDR PP");
		resize_pp_perf_ = profiler.CreatePerfRange(0, "Resize PP");
		stereoscopic_pp_perf_ = profiler.CreatePerfRange(0, "Stereoscopic PP");
#endif
	}

	void RenderEngine::DestroyRenderWindow()
	{
		if (cur_frame_buffer_)
		{
			cur_frame_buffer_->OnUnbind();
		}
		cur_frame_buffer_.reset();

		stereoscopic_pp_.reset();
		for (size_t i = 0; i < 2; ++ i)
		{
			resize_pps_[i].reset();
		}
		for (size_t i = 0; i < 12; ++ i)
		{
			ldr_pps_[i].reset();
		}
		ldr_pp_.reset();
		skip_hdr_pp_.reset();
		hdr_pp_.reset();

		screen_frame_buffer_.reset();
		overlay_frame_buffer_.reset();
		mono_frame_buffer_.reset();
		resize_frame_buffer_.reset();
		ldr_frame_buffer_.reset();
		hdr_frame_buffer_.reset();

		overlay_tex_.reset();
		mono_tex_.reset();
		resize_tex_.reset();
		hdr_tex_.reset();
		hdr_tex_.reset();
		ds_tex_.reset();
		ldr_tex_.reset();

		for (int i = 3; i >= 0; -- i)
		{
			default_frame_buffers_[i].reset();
		}
	}

	void RenderEngine::CheckConfig(RenderSettings& /*settings*/)
	{
	}

	// 设置当前渲染状态对象
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::SetStateObjects(RasterizerStateObjectPtr const & rs_obj,
		DepthStencilStateObjectPtr const & dss_obj, uint16_t front_stencil_ref, uint16_t back_stencil_ref,
		BlendStateObjectPtr const & bs_obj, Color const & blend_factor, uint32_t sample_mask)
	{
		if (cur_rs_obj_ != rs_obj)
		{
			if (force_line_mode_)
			{
				RasterizerStateDesc desc = rs_obj->GetDesc();
				desc.polygon_mode = PM_Line;
				cur_line_rs_obj_ = Context::Instance().RenderFactoryInstance().MakeRasterizerStateObject(desc);
				cur_line_rs_obj_->Active();
			}
			else
			{
				rs_obj->Active();
			}
			cur_rs_obj_ = rs_obj;
		}

		if ((cur_dss_obj_ != dss_obj) || (cur_front_stencil_ref_ != front_stencil_ref) || (cur_back_stencil_ref_ != back_stencil_ref))
		{
			dss_obj->Active(front_stencil_ref, back_stencil_ref);
			cur_dss_obj_ = dss_obj;
			cur_front_stencil_ref_ = front_stencil_ref;
			cur_back_stencil_ref_ = back_stencil_ref;
		}

		if ((cur_bs_obj_ != bs_obj) || (cur_blend_factor_ != blend_factor) || (cur_sample_mask_ != sample_mask))
		{
			bs_obj->Active(blend_factor, sample_mask);
			cur_bs_obj_ = bs_obj;
			cur_blend_factor_ = blend_factor;
			cur_sample_mask_ = sample_mask;
		}
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::BindFrameBuffer(FrameBufferPtr const & fb)
	{
		FrameBufferPtr new_fb;
		if (fb)
		{
			new_fb = fb;
		}
		else
		{
			new_fb = this->DefaultFrameBuffer();
		}

		if ((fb != new_fb) || (fb && fb->Dirty()))
		{
			if (cur_frame_buffer_)
			{
				cur_frame_buffer_->OnUnbind();
			}

			cur_frame_buffer_ = new_fb;
			cur_frame_buffer_->OnBind();

			this->DoBindFrameBuffer(cur_frame_buffer_);
		}
	}

	// 获取当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	FrameBufferPtr const & RenderEngine::CurFrameBuffer() const
	{
		return cur_frame_buffer_;
	}

	// 获取默认渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	FrameBufferPtr const & RenderEngine::DefaultFrameBuffer() const
	{
		return default_frame_buffers_[fb_stage_];
	}

	// 获取屏幕渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	FrameBufferPtr const & RenderEngine::ScreenFrameBuffer() const
	{
		return screen_frame_buffer_;
	}

	FrameBufferPtr const & RenderEngine::OverlayFrameBuffer() const
	{
		return overlay_frame_buffer_;
	}

	void RenderEngine::BindSOBuffers(RenderLayoutPtr const & rl)
	{
		so_buffers_ = rl;
		this->DoBindSOBuffers(rl);
	}

	// 渲染一个vb
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::Render(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl)
	{
		this->DoRender(effect, tech, rl);
	}

	void RenderEngine::Dispatch(RenderEffect const & effect, RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		this->DoDispatch(effect, tech, tgx, tgy, tgz);
	}

	void RenderEngine::DispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
		GraphicsBufferPtr const & buff_args, uint32_t offset)
	{
		this->DoDispatchIndirect(effect, tech, buff_args, offset);
	}

	// 上次Render()所渲染的图元数
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t RenderEngine::NumPrimitivesJustRendered()
	{
		uint32_t const ret = num_primitives_just_rendered_;
		num_primitives_just_rendered_ = 0;
		return ret;
	}

	// 上次Render()所渲染的顶点数
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t RenderEngine::NumVerticesJustRendered()
	{
		uint32_t const ret = num_vertices_just_rendered_;
		num_vertices_just_rendered_ = 0;
		return ret;
	}

	uint32_t RenderEngine::NumDrawsJustCalled()
	{
		uint32_t const ret = num_draws_just_called_;
		num_draws_just_called_ = 0;
		return ret;
	}

	uint32_t RenderEngine::NumDispatchesJustCalled()
	{
		uint32_t const ret = num_dispatches_just_called_;
		num_dispatches_just_called_ = 0;
		return ret;
	}

	// 获取渲染设备能力
	/////////////////////////////////////////////////////////////////////////////////
	RenderDeviceCaps const & RenderEngine::DeviceCaps() const
	{
		return caps_;
	}

	void RenderEngine::GetCustomAttrib(std::string const & /*name*/, void* /*value*/)
	{
	}

	void RenderEngine::SetCustomAttrib(std::string const & /*name*/, void* /*value*/)
	{
	}

	void RenderEngine::Resize(uint32_t width, uint32_t height)
	{
		uint32_t const old_screen_width = default_frame_buffers_[3]->Width();
		uint32_t const old_screen_height = default_frame_buffers_[3]->Height();
		uint32_t const new_screen_width = width;
		uint32_t const new_screen_height = height;
		uint32_t const new_render_width = static_cast<uint32_t>(new_screen_width * default_render_height_scale_ + 0.5f);
		uint32_t const new_render_height = static_cast<uint32_t>(new_screen_height * default_render_height_scale_ + 0.5f);
		if ((old_screen_width != new_screen_width) || (old_screen_height != new_screen_height))
		{
			this->DoResize(new_screen_width, new_screen_height);

			RenderSettings const & settings = Context::Instance().Config().graphics_cfg;
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			RenderViewPtr ds_view;
			if (hdr_pp_ || ldr_pp_ || (stereo_method_ != STM_None))
			{
				ElementFormat fmt = ds_tex_->Format();
				ds_tex_ = this->ScreenDepthStencilTexture();
				if (ds_tex_ && (new_screen_width == new_render_width) && (new_screen_height == new_render_height))
				{
					ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
				}
				else
				{
					if (caps.texture_format_support(EF_D32F) || caps.texture_format_support(EF_D24S8)
						|| caps.texture_format_support(EF_D16))
					{
						ds_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
						ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
					}
					else
					{
						if ((settings.depth_stencil_fmt != EF_Unknown)
							&& caps.rendertarget_format_support(settings.depth_stencil_fmt, 1, 0))
						{
							fmt = settings.depth_stencil_fmt;
						}
						else
						{
							BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

							fmt = EF_D16;
						}
						ds_view = rf.Make2DDepthStencilRenderView(new_render_width, new_render_height, fmt, 1, 0);
					}
				}
			}

			default_frame_buffers_[0] = default_frame_buffers_[1] = default_frame_buffers_[2] = default_frame_buffers_[3]
				= screen_frame_buffer_;

			if (stereo_method_ != STM_None)
			{
				ElementFormat fmt = mono_tex_->Format();
				mono_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				mono_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*mono_tex_, 0, 1, 0));

				default_frame_buffers_[0] = default_frame_buffers_[1] = default_frame_buffers_[2] = mono_frame_buffer_;
			}
			else
			{
				bool need_resize = ((new_render_width != new_screen_width) || (new_render_height != new_screen_height));
				if (need_resize)
				{
					if (!resize_frame_buffer_)
					{
						resize_frame_buffer_ = rf.MakeFrameBuffer();
						resize_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;
					}

					ElementFormat fmt;
					if (resize_tex_)
					{
						fmt = resize_tex_->Format();
					}
					else
					{
						if (caps.texture_format_support(EF_ABGR8) && caps.rendertarget_format_support(EF_ABGR8, 1, 0))
						{
							fmt = EF_ABGR8;
						}
						else
						{
							BOOST_ASSERT(caps.texture_format_support(EF_ARGB8) && caps.rendertarget_format_support(EF_ARGB8, 1, 0));

							fmt = EF_ARGB8;
						}
					}

					resize_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
					resize_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*resize_tex_, 0, 1, 0));

					ElementFormat ds_fmt;
					if ((settings.depth_stencil_fmt != EF_Unknown) && caps.rendertarget_format_support(settings.depth_stencil_fmt, 1, 0))
					{
						ds_fmt = settings.depth_stencil_fmt;
					}
					else
					{
						BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

						ds_fmt = EF_D16;
					}
					resize_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil,
						rf.Make2DDepthStencilRenderView(new_render_width, new_render_height, ds_fmt, 1, 0));

					float const scale_x = static_cast<float>(new_screen_width) / new_render_width;
					float const scale_y = static_cast<float>(new_screen_height) / new_render_height;

					float2 pos_scale;
					if (scale_x < scale_y)
					{
						pos_scale.x() = 1;
						pos_scale.y() = (scale_x * new_render_height) / new_screen_height;
					}
					else
					{
						pos_scale.x() = (scale_y * new_render_width) / new_screen_width;
						pos_scale.y() = 1;
					}

					for (size_t i = 0; i < 2; ++ i)
					{
						resize_pps_[i]->SetParam(0, pos_scale);
					}

					default_frame_buffers_[0] = default_frame_buffers_[1] = default_frame_buffers_[2] = resize_frame_buffer_;
				}
			}
			if (ldr_pp_)
			{
				ElementFormat fmt = ldr_tex_->Format();
				ldr_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				ldr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ldr_tex_, 0, 1, 0));
				ldr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

				default_frame_buffers_[0] = default_frame_buffers_[1] = ldr_frame_buffer_;
			}
			if (hdr_pp_)
			{
				ElementFormat fmt = hdr_tex_->Format();
				hdr_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
				hdr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0, 1, 0));
				hdr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

				default_frame_buffers_[0] = hdr_frame_buffer_;
			}

			pp_chain_dirty_ = true;
		}
		else
		{
			this->DoResize(old_screen_width, old_screen_height);
		}

		this->BindFrameBuffer(default_frame_buffers_[0]);

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);
	}

	void RenderEngine::PostProcess(bool skip)
	{
		if (pp_chain_dirty_)
		{
			this->AssemblePostProcessChain();
			pp_chain_dirty_ = false;
		}

		fb_stage_ = 1;

#ifndef KLAYGE_SHIP
		hdr_pp_perf_->Begin();
#endif
		if (hdr_enabled_)
		{
			if (skip)
			{
				skip_hdr_pp_->Apply();
			}
			else
			{
				hdr_tex_->BuildMipSubLevels();
				hdr_pp_->Apply();
			}
		}
		else
		{
			if (skip_hdr_pp_)
			{
				skip_hdr_pp_->Apply();
			}
		}
#ifndef KLAYGE_SHIP
		hdr_pp_perf_->End();
#endif

		fb_stage_ = 2;

#ifndef KLAYGE_SHIP
		ldr_pp_perf_->Begin();
#endif
		if (ppaa_enabled_ || gamma_enabled_ || color_grading_enabled_)
		{
			if (skip)
			{
				ldr_pps_[0]->Apply();
			}
			else
			{
				ldr_pp_->Apply();
			}
		}
		else
		{
			if (ldr_pps_[0])
			{
				ldr_pps_[0]->Apply();
			}
		}
#ifndef KLAYGE_SHIP
		ldr_pp_perf_->End();
#endif

		fb_stage_ = 3;

		uint32_t const screen_width = default_frame_buffers_[3]->Width();
		uint32_t const screen_height = default_frame_buffers_[3]->Height();
		uint32_t const render_width = default_frame_buffers_[0]->Width();
		uint32_t const render_height = default_frame_buffers_[0]->Height();
		bool const need_resize = ((render_width != screen_width) || (render_height != screen_height));
		if (resize_pps_[0])
		{
			if ((STM_None == stereo_method_) && need_resize)
			{
#ifndef KLAYGE_SHIP
				resize_pp_perf_->Begin();
#endif
				float const scale_x = static_cast<float>(screen_width) / render_width;
				float const scale_y = static_cast<float>(screen_height) / render_height;

				if (!MathLib::equal(scale_x, scale_y))
				{
					this->DefaultFrameBuffer()->Attached(FrameBuffer::ATT_Color0)->ClearColor(Color(0, 0, 0, 0));
				}

				if ((scale_x > 2) || (scale_y > 2))
				{
					resize_pps_[1]->Apply();
				}
				else
				{
					resize_pps_[0]->Apply();
				}
#ifndef KLAYGE_SHIP
				resize_pp_perf_->End();
#endif
			}
		}

		if (!this->ScreenDepthStencilTexture() || need_resize)
		{
			RenderViewPtr const & ds_view = this->DefaultFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil);
			if (ds_view)
			{
				ds_view->ClearDepth(1.0f);
			}
		}

		fb_stage_ = 0;
	}

	void RenderEngine::HDREnabled(bool hdr)
	{
		if (hdr_pp_)
		{
			pp_chain_dirty_ = true;
			hdr_enabled_ = hdr;
		}
	}

	void RenderEngine::PPAAEnabled(int aa)
	{
		if (ldr_pp_)
		{
			pp_chain_dirty_ = true;
			ppaa_enabled_ = aa;
			ldr_pp_ = ldr_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	void RenderEngine::GammaEnabled(bool gamma)
	{
		if (ldr_pp_)
		{
			pp_chain_dirty_ = true;
			gamma_enabled_ = gamma;
			ldr_pp_ = ldr_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	void RenderEngine::ColorGradingEnabled(bool cg)
	{
		if (ldr_pp_)
		{
			pp_chain_dirty_ = true;
			color_grading_enabled_ = cg;
			ldr_pp_ = ldr_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	void RenderEngine::Refresh()
	{
		if (Context::Instance().AppInstance().MainWnd()->Active())
		{
			Context::Instance().SceneManagerInstance().Update();

#ifndef KLAYGE_SHIP
			PerfProfiler::Instance().CollectData();
#endif
		}
	}

	void RenderEngine::Stereoscopic()
	{
		if (stereo_method_ != STM_None)
		{
			fb_stage_ = 3;

#ifndef KLAYGE_SHIP
			stereoscopic_pp_perf_->Begin();
#endif

			this->BindFrameBuffer(screen_frame_buffer_);
			if (stereoscopic_pp_)
			{
				Camera const & camera = *screen_frame_buffer_->GetViewport()->camera;

				stereoscopic_pp_->SetParam(0, stereo_separation_);
				stereoscopic_pp_->SetParam(1, camera.NearPlane());

				float const q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
				float2 near_q(camera.NearPlane() * q, q);
				stereoscopic_pp_->SetParam(2, near_q);
			}
			if (stereo_method_ != STM_LCDShutter)
			{
				if (STM_OculusVR == stereo_method_)
				{
					Viewport const & vp = *screen_frame_buffer_->GetViewport();

					float w = 0.5f;
					float h = 1;
					float x_left = 0;
					float x_right = 0.5f;
					float y = 0;

					float aspect = static_cast<float>(vp.width / 2) / vp.height;

					float3 lens_center(x_left + (w + ovr_x_center_offset_ * 0.5f) * 0.5f,
						x_right + (w - ovr_x_center_offset_ * 0.5f) * 0.5f, y + h * 0.5f);
					float3 screen_center(x_left + w * 0.5f, x_right + w * 0.5f, y + h * 0.5f);

					float scale_factor = 1.0f / ovr_scale_;

					float2 scale((w / 2) * scale_factor, (h / 2) * scale_factor * aspect);
					float2 scale_in(2 / w, 2 / h / aspect);

					stereoscopic_pp_->SetParam(3, lens_center);
					stereoscopic_pp_->SetParam(4, screen_center);
					stereoscopic_pp_->SetParam(5, scale);
					stereoscopic_pp_->SetParam(6, scale_in);
					stereoscopic_pp_->SetParam(7, ovr_hmd_warp_param_);
					stereoscopic_pp_->SetParam(8, ovr_chrom_ab_param_);
				}

				stereoscopic_pp_->Render();
			}
			else
			{
				this->StereoscopicForLCDShutter(1);
				this->StereoscopicForLCDShutter(0);

				this->BindFrameBuffer(screen_frame_buffer_);
			}

#ifndef KLAYGE_SHIP
			stereoscopic_pp_perf_->End();
#endif

			fb_stage_ = 0;
		}
	}

	void RenderEngine::StereoscopicForLCDShutter(int32_t /*eye*/)
	{
	}

	void RenderEngine::Stereo(StereoMethod method)
	{
		stereo_method_ = method;
		if (stereo_method_ != STM_None)
		{
			std::string pp_name;
			switch (stereo_method_)
			{
			case STM_ColorAnaglyph_RedCyan:
				pp_name = "stereoscopic_red_cyan";
				break;

			case STM_ColorAnaglyph_YellowBlue:
				pp_name = "stereoscopic_yellow_blue";
				break;

			case STM_ColorAnaglyph_GreenRed:
				pp_name = "stereoscopic_green_red";
				break;

			case STM_HorizontalInterlacing:
				pp_name = "stereoscopic_hor_interlacing";
				break;

			case STM_VerticalInterlacing:
				pp_name = "stereoscopic_ver_interlacing";
				break;

			case STM_Horizontal:
				pp_name = "stereoscopic_horizontal";
				break;

			case STM_Vertical:
				pp_name = "stereoscopic_vertical";
				break;

			case STM_LCDShutter:
				pp_name = "stereoscopic_lcd_shutter";
				break;

			case STM_OculusVR:
				pp_name = "stereoscopic_oculus_vr";
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			stereoscopic_pp_ = SyncLoadPostProcess("Stereoscopic.ppml", pp_name);
		}

		pp_chain_dirty_ = true;
	}

	void RenderEngine::AssemblePostProcessChain()
	{
		if (ldr_pp_)
		{
			for (size_t i = 0; i < 12; ++ i)
			{
				ldr_pps_[i]->OutputPin(0, TexturePtr());
			}
		}				
		if (hdr_pp_)
		{
			hdr_pp_->OutputPin(0, TexturePtr());
			skip_hdr_pp_->OutputPin(0, TexturePtr());
		}

		if (stereo_method_ != STM_None)
		{
			if (stereoscopic_pp_)
			{
				if (ldr_pp_)
				{
					for (size_t i = 0; i < 12; ++ i)
					{
						ldr_pps_[i]->OutputPin(0, mono_tex_);
					}
				}
				if (hdr_pp_)
				{
					hdr_pp_->OutputPin(0, mono_tex_);
					skip_hdr_pp_->OutputPin(0, mono_tex_);
				}

				stereoscopic_pp_->InputPin(0, mono_tex_);
				stereoscopic_pp_->InputPin(1, ds_tex_);
				stereoscopic_pp_->InputPin(2, overlay_tex_);
			}
		}
		else
		{
			uint32_t const screen_width = default_frame_buffers_[3]->Width();
			uint32_t const screen_height = default_frame_buffers_[3]->Height();
			uint32_t const render_width = default_frame_buffers_[0]->Width();
			uint32_t const render_height = default_frame_buffers_[0]->Height();
			bool need_resize = ((render_width != screen_width) || (render_height != screen_height));
			if (need_resize)
			{
				if (ldr_pp_)
				{
					for (size_t i = 0; i < 12; ++ i)
					{
						ldr_pps_[i]->OutputPin(0, resize_tex_);
					}
				}
				if (hdr_pp_)
				{
					hdr_pp_->OutputPin(0, resize_tex_);
					skip_hdr_pp_->OutputPin(0, resize_tex_);
				}

				if (resize_pps_[0])
				{
					for (size_t i = 0; i < 2; ++ i)
					{
						resize_pps_[i]->InputPin(0, resize_tex_);
					}
				}
			}
		}

		if (ldr_pp_)
		{
			if (hdr_pp_)
			{
				hdr_pp_->OutputPin(0, ldr_tex_);
				skip_hdr_pp_->OutputPin(0, ldr_tex_);
			}

			for (size_t i = 0; i < 12; ++ i)
			{
				ldr_pps_[i]->InputPin(0, ldr_tex_);
			}
		}

		if (hdr_pp_)
		{
			hdr_pp_->InputPin(0, hdr_tex_);
			skip_hdr_pp_->InputPin(0, hdr_tex_);
		}
	}

	void RenderEngine::ForceLineMode(bool line)
	{
		if (force_line_mode_ != line)
		{
			force_line_mode_ = line;

			if (cur_rs_obj_)
			{
				if (force_line_mode_)
				{
					RasterizerStateDesc desc = cur_rs_obj_->GetDesc();
					desc.polygon_mode = PM_Line;
					cur_line_rs_obj_ = Context::Instance().RenderFactoryInstance().MakeRasterizerStateObject(desc);
					cur_line_rs_obj_->Active();
				}
				else
				{
					cur_rs_obj_->Active();
				}
			}
		}
	}

	void RenderEngine::Destroy()
	{
		cur_frame_buffer_.reset();
		screen_frame_buffer_.reset();
		ds_tex_.reset();
		hdr_frame_buffer_.reset();
		hdr_tex_.reset();
		ldr_frame_buffer_.reset();
		ldr_tex_.reset();
		resize_frame_buffer_.reset();
		resize_tex_.reset();
		mono_frame_buffer_.reset();
		mono_tex_.reset();
		for (int i = 0; i < 4; ++ i)
		{
			default_frame_buffers_[i].reset();
		}

		overlay_frame_buffer_.reset();
		overlay_tex_.reset();

		so_buffers_.reset();

		cur_rs_obj_.reset();
		cur_line_rs_obj_.reset();
		cur_dss_obj_.reset();
		cur_bs_obj_.reset();

		pp_rl_.reset();

		hdr_pp_.reset();
		skip_hdr_pp_.reset();
		ldr_pp_.reset();
		resize_pps_[0].reset();
		resize_pps_[1].reset();
		stereoscopic_pp_.reset();

		for (int i = 0; i < 12; ++ i)
		{
			ldr_pps_[i].reset();
		}

#ifndef KLAYGE_SHIP
		hdr_pp_perf_.reset();
		ldr_pp_perf_.reset();
		resize_pp_perf_.reset();
		stereoscopic_pp_perf_.reset();
#endif

		this->DoDestroy();
	}
}
