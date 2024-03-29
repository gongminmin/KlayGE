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

#include <KFL/CXX20/format.hpp>
#include <KFL/ErrorHandling.hpp>
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
#include <KlayGE/RenderView.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/PerfProfiler.hpp>

#include <mutex>
#include <string>

#include <KlayGE/RenderEngine.hpp>

namespace
{
	std::mutex default_mtl_instance_mutex;
	std::mutex mtl_cb_instance_mutex;
	std::mutex mesh_cb_instance_mutex;
	std::mutex model_cb_instance_mutex;
	std::mutex camera_cb_instance_mutex;
	std::mutex mipmapper_instance_mutex;
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderEngine()
		: num_primitives_just_rendered_(0), num_vertices_just_rendered_(0),
			num_draws_just_called_(0), num_dispatches_just_called_(0),
			default_fov_(PI / 4), default_render_width_scale_(1), default_render_height_scale_(1),
			stereo_method_(STM_None), stereo_separation_(0),
			fb_stage_(0), force_line_mode_(false)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::~RenderEngine() noexcept = default;

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

		screen_frame_buffer_camera_node_ =
			MakeSharedPtr<SceneNode>(L"CameraNode", SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		screen_frame_buffer_camera_node_->AddComponent(screen_frame_buffer_->Viewport()->Camera());

		uint32_t const screen_width = screen_frame_buffer_->Width();
		uint32_t const screen_height = screen_frame_buffer_->Height();
		float const screen_aspect = static_cast<float>(screen_width) / screen_height;
		if (!MathLib::equal(screen_aspect, static_cast<float>(settings.width) / settings.height))
		{
			settings.width = static_cast<uint32_t>(settings.height * screen_aspect + 0.5f);
		}

		auto& context = Context::Instance();
		RenderFactory& rf = context.RenderFactoryInstance();

		pp_rl_ = rf.MakeRenderLayout();
		pp_rl_->TopologyType(RenderLayout::TT_TriangleStrip);

		float2 const pp_pos[] =
		{
			float2(-1, +1),
			float2(+1, +1),
			float2(-1, -1),
			float2(+1, -1)
		};
		GraphicsBufferPtr pp_pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(pp_pos), &pp_pos[0]);
		pp_rl_->BindVertexStream(pp_pos_vb, VertexElement(VEU_Position, 0, EF_GR32F));

		vpp_rl_ = rf.MakeRenderLayout();
		vpp_rl_->TopologyType(RenderLayout::TT_TriangleList);

		float3 const vpp_pos[] =
		{
			float3(-1, +1, -1),
			float3(+1, +1, -1),
			float3(-1, -1, -1),
			float3(+1, -1, -1),
			float3(-1, +1, +1),
			float3(+1, +1, +1),
			float3(-1, -1, +1),
			float3(+1, -1, +1)
		};
		GraphicsBufferPtr vpp_pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vpp_pos), &vpp_pos[0]);
		vpp_rl_->BindVertexStream(vpp_pos_vb, VertexElement(VEU_Position, 0, EF_BGR32F));

		uint16_t constexpr vpp_indices[] =
		{
			0, 1, 3, 3, 2, 0,
			1, 5, 7, 7, 3, 1,
			5, 4, 6, 6, 7, 5,
			4, 0, 2, 2, 6, 4,
			4, 5, 1, 1, 0, 4,
			2, 3, 7, 7, 6, 2
		};
		GraphicsBufferPtr vpp_ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vpp_indices), &vpp_indices[0]);
		vpp_rl_->BindIndexStream(vpp_ib, EF_R16UI);

		uint32_t const render_width = static_cast<uint32_t>(settings.width * default_render_width_scale_ + 0.5f);
		uint32_t const render_height = static_cast<uint32_t>(settings.height * default_render_height_scale_ + 0.5f);

		hdr_enabled_ = settings.hdr;
		if (settings.hdr)
		{
			hdr_pp_ = MakeSharedPtr<HDRPostProcess>(settings.fft_lens_effects);
			skip_hdr_pp_ = SyncLoadPostProcess("ToneMapping.ppml", "skip_tone_mapping");
		}

		ppaa_enabled_ = settings.ppaa ? 1 : 0;
		if (settings.ppaa)
		{
			smaa_edge_detection_pp_ = SyncLoadPostProcess("SMAA.ppml", "luma_edge_detection");
			smaa_blending_weight_pp_ = SyncLoadPostProcess("SMAA.ppml", "blending_weight_calculation");
		}

		gamma_enabled_ = settings.gamma;
		color_grading_enabled_ = settings.color_grading;
		if (settings.ppaa || settings.color_grading || settings.gamma)
		{
			for (size_t i = 0; i < 12; ++ i)
			{
				post_tone_mapping_pps_[i] = SyncLoadPostProcess("PostToneMapping.ppml", std::format("PostToneMapping{}", i));
			}

			post_tone_mapping_pp_ = post_tone_mapping_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
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

		DepthStencilViewPtr ds_dsv;
		if (hdr_pp_ || post_tone_mapping_pp_ || (settings.stereo_method != STM_None))
		{
			ds_tex_ = this->ScreenDepthStencilTexture();
			if (ds_tex_ && (screen_width == render_width) && (screen_height == render_height))
			{
				ds_srv_ = rf.MakeTextureSrv(ds_tex_);
				ds_dsv = rf.Make2DDsv(ds_tex_, 0, 1, 0);
			}
			else
			{
				ElementFormat fmt;
				if (caps.depth_texture_support)
				{
					if ((settings.depth_stencil_fmt != EF_Unknown)
						&& caps.TextureFormatSupport(settings.depth_stencil_fmt))
					{
						fmt = settings.depth_stencil_fmt;
					}
					else
					{
						BOOST_ASSERT(caps.TextureFormatSupport(EF_D16));

						fmt = EF_D16;
					}
					ds_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
					KLAYGE_TEXTURE_DEBUG_NAME(ds_tex_);
					ds_srv_ = rf.MakeTextureSrv(ds_tex_);
					ds_dsv = rf.Make2DDsv(ds_tex_, 0, 1, 0);
				}
				else
				{
					if ((settings.depth_stencil_fmt != EF_Unknown)
						&& caps.RenderTargetFormatSupport(settings.depth_stencil_fmt, 1, 0))
					{
						fmt = settings.depth_stencil_fmt;
					}
					else
					{
						BOOST_ASSERT(caps.RenderTargetFormatSupport(EF_D16, 1, 0));

						fmt = EF_D16;
					}
					ds_dsv = rf.Make2DDsv(render_width, render_height, fmt, 1, 0);
				}
			}
		}

		if ((settings.stereo_method != STM_None) || (settings.display_output_method != DOM_sRGB))
		{
			mono_frame_buffer_ = rf.MakeFrameBuffer();
			mono_frame_buffer_->Viewport()->Camera(cur_frame_buffer_->Viewport()->Camera());

			ElementFormat const backup_fmts[] = { EF_B10G11R11F, settings.color_fmt, EF_ABGR8, EF_ARGB8 };
			std::span<ElementFormat const> fmt_options = backup_fmts;
			if (settings.display_output_method != DOM_sRGB)
			{
				fmt_options = fmt_options.subspan(1);
			}
			auto fmt = caps.BestMatchTextureRenderTargetFormat(fmt_options, 1, 0);

			mono_tex_ = rf.MakeTexture2D(screen_width, screen_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(mono_tex_);
			mono_srv_ = rf.MakeTextureSrv(mono_tex_);
			mono_rtv_ = rf.Make2DRtv(mono_tex_, 0, 1, 0);
			mono_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, mono_rtv_);

			default_frame_buffers_[0] = default_frame_buffers_[1]
				= default_frame_buffers_[2] = mono_frame_buffer_;

			overlay_frame_buffer_ = rf.MakeFrameBuffer();
			overlay_frame_buffer_->Viewport()->Camera(cur_frame_buffer_->Viewport()->Camera());

			fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);

			overlay_tex_ = rf.MakeTexture2D(screen_width, screen_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(overlay_tex_);
			overlay_srv_ = rf.MakeTextureSrv(overlay_tex_);
			overlay_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(overlay_tex_, 0, 1, 0));

			DepthStencilViewPtr screen_size_ds_dsv;
			if (need_resize)
			{
				screen_size_ds_dsv = rf.Make2DDsv(screen_width, screen_height, ds_dsv->Format(), 1, 0);
			}
			else
			{
				screen_size_ds_dsv = ds_dsv;
			}
			overlay_frame_buffer_->Attach(screen_size_ds_dsv);
		}
		else
		{
			if (need_resize)
			{
				resize_frame_buffer_ = rf.MakeFrameBuffer();
				resize_frame_buffer_->Viewport()->Camera(cur_frame_buffer_->Viewport()->Camera());

				auto const fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
				BOOST_ASSERT(fmt != EF_Unknown);

				resize_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1,
					fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(resize_tex_);
				resize_srv_ = rf.MakeTextureSrv(resize_tex_);
				resize_rtv_ = rf.Make2DRtv(resize_tex_, 0, 1, 0);
				resize_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, resize_rtv_);

				ElementFormat ds_fmt;
				if ((settings.depth_stencil_fmt != EF_Unknown) && caps.RenderTargetFormatSupport(settings.depth_stencil_fmt, 1, 0))
				{
					ds_fmt = settings.depth_stencil_fmt;
				}
				else
				{
					BOOST_ASSERT(caps.RenderTargetFormatSupport(EF_D16, 1, 0));

					ds_fmt = EF_D16;
				}
				resize_frame_buffer_->Attach(rf.Make2DDsv(render_width, render_height, ds_fmt, 1, 0));

				default_frame_buffers_[0] = default_frame_buffers_[1]
					= default_frame_buffers_[2] = resize_frame_buffer_;
			}
		}

		if (smaa_edge_detection_pp_ || smaa_blending_weight_pp_)
		{
			auto fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_GR8, EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);
			smaa_edges_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(smaa_edges_tex_);
			smaa_edges_srv_ = rf.MakeTextureSrv(smaa_edges_tex_);
			smaa_edges_rtv_ = rf.Make2DRtv(smaa_edges_tex_, 0, 1, 0);

			fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);
			smaa_blend_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(smaa_blend_tex_);
			smaa_blend_srv_ = rf.MakeTextureSrv(smaa_blend_tex_);
			smaa_blend_rtv_ = rf.Make2DRtv(smaa_blend_tex_, 0, 1, 0);

			auto smaa_ds_dsv = rf.Make2DDsv(render_width, render_height, EF_D24S8, 1, 0);

			smaa_edge_detection_pp_->OutputPin(0, smaa_edges_rtv_);
			smaa_edge_detection_pp_->OutputFrameBuffer()->Attach(smaa_ds_dsv);

			smaa_blending_weight_pp_->InputPin(0, smaa_edges_srv_);
			smaa_blending_weight_pp_->OutputPin(0, smaa_blend_rtv_);
			smaa_blending_weight_pp_->OutputFrameBuffer()->Attach(smaa_ds_dsv);
		}
		
		if (post_tone_mapping_pp_)
		{
			post_tone_mapping_frame_buffer_ = rf.MakeFrameBuffer();
			post_tone_mapping_frame_buffer_->Viewport()->Camera(cur_frame_buffer_->Viewport()->Camera());

			auto const fmt = caps.BestMatchTextureRenderTargetFormat((settings.display_output_method == DOM_sRGB)
				? MakeSpan({EF_ABGR8_SRGB, EF_ARGB8_SRGB, EF_ABGR8, EF_ARGB8}) : MakeSpan({EF_B10G11R11F, EF_ABGR16F}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);

			post_tone_mapping_tex_ = rf.MakeTexture2D(render_width, render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
			KLAYGE_TEXTURE_DEBUG_NAME(post_tone_mapping_tex_);
			post_tone_mapping_srv_ = rf.MakeTextureSrv(post_tone_mapping_tex_);
			post_tone_mapping_rtv_ = rf.Make2DRtv(post_tone_mapping_tex_, 0, 1, 0);
			post_tone_mapping_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, post_tone_mapping_rtv_);
			post_tone_mapping_frame_buffer_->Attach(ds_dsv);

			default_frame_buffers_[0] = default_frame_buffers_[1] = post_tone_mapping_frame_buffer_;
		}

		if (hdr_pp_)
		{
			hdr_frame_buffer_ = rf.MakeFrameBuffer();
			hdr_frame_buffer_->Viewport()->Camera(cur_frame_buffer_->Viewport()->Camera());

			auto const fmt = caps.BestMatchTextureRenderTargetFormat(caps.fp_color_support ? MakeSpan({EF_B10G11R11F, EF_ABGR16F})
				: MakeSpan({EF_ABGR8_SRGB, EF_ARGB8_SRGB, EF_ABGR8, EF_ARGB8}), 1, 0);
			BOOST_ASSERT(fmt != EF_Unknown);
			hdr_tex_ = rf.MakeTexture2D(render_width, render_height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
			KLAYGE_TEXTURE_DEBUG_NAME(hdr_tex_);
			hdr_srv_ = rf.MakeTextureSrv(hdr_tex_);
			hdr_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(hdr_tex_, 0, 1, 0));
			hdr_frame_buffer_->Attach(ds_dsv);

			default_frame_buffers_[0] = hdr_frame_buffer_;
		}

		this->BindFrameBuffer(default_frame_buffers_[0]);
		this->Stereo(settings.stereo_method);
		this->StereoSeparation(settings.stereo_separation);
		this->DisplayOutput(settings.display_output_method);
		this->PaperWhiteNits(settings.paper_white);
		this->DisplayMaxLuminanceNits(settings.display_max_luminance);

#ifndef KLAYGE_SHIP
		PerfProfiler& profiler = context.PerfProfilerInstance();
		hdr_pp_perf_ = profiler.CreatePerfRegion(0, "HDR PP");
		smaa_pp_perf_ = profiler.CreatePerfRegion(0, "SMAA PP");
		post_tone_mapping_pp_perf_ = profiler.CreatePerfRegion(0, "Post tone mapping PP");
		resize_pp_perf_ = profiler.CreatePerfRegion(0, "Resize PP");
		hdr_display_pp_perf_ = profiler.CreatePerfRegion(0, "HDR display PP");
		stereoscopic_pp_perf_ = profiler.CreatePerfRegion(0, "Stereoscopic PP");
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
		hdr_display_pp_.reset();
		for (size_t i = 0; i < 2; ++ i)
		{
			resize_pps_[i].reset();
		}
		for (size_t i = 0; i < 12; ++ i)
		{
			post_tone_mapping_pps_[i].reset();
		}
		post_tone_mapping_pp_.reset();
		smaa_edge_detection_pp_.reset();
		smaa_blending_weight_pp_.reset();
		skip_hdr_pp_.reset();
		hdr_pp_.reset();

		screen_frame_buffer_.reset();
		overlay_frame_buffer_.reset();
		mono_frame_buffer_.reset();
		resize_frame_buffer_.reset();
		post_tone_mapping_frame_buffer_.reset();
		hdr_frame_buffer_.reset();

		overlay_tex_.reset();
		overlay_srv_.reset();
		mono_tex_.reset();
		mono_srv_.reset();
		mono_rtv_.reset();
		resize_tex_.reset();
		resize_srv_.reset();
		resize_rtv_.reset();
		hdr_tex_.reset();
		hdr_srv_.reset();
		ds_tex_.reset();
		ds_srv_.reset();
		post_tone_mapping_tex_.reset();
		post_tone_mapping_srv_.reset();
		post_tone_mapping_rtv_.reset();

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
	void RenderEngine::SetStateObject(RenderStateObjectPtr const & rs_obj)
	{
		if (cur_rs_obj_ != rs_obj)
		{
			if (force_line_mode_)
			{
				auto rs_desc = rs_obj->GetRasterizerStateDesc();
				auto const & dss_desc = rs_obj->GetDepthStencilStateDesc();
				auto const & bs_desc = rs_obj->GetBlendStateDesc();
				rs_desc.polygon_mode = PM_Line;
				cur_line_rs_obj_ = Context::Instance().RenderFactoryInstance().MakeRenderStateObject(rs_desc, dss_desc, bs_desc);
				cur_line_rs_obj_->Active();
			}
			else
			{
				rs_obj->Active();
			}
			cur_rs_obj_ = rs_obj;
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

		if ((cur_frame_buffer_ != new_fb) || (new_fb && new_fb->Dirty()))
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
		if (tech.HWResourceReady(effect))
		{
			this->DoRender(effect, tech, rl);
		}
	}

	void RenderEngine::Dispatch(RenderEffect const & effect, RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		if (tech.HWResourceReady(effect))
		{
			this->DoDispatch(effect, tech, tgx, tgy, tgz);
		}
	}

	void RenderEngine::DispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
		GraphicsBufferPtr const & buff_args, uint32_t offset)
	{
		if (tech.HWResourceReady(effect))
		{
			this->DoDispatchIndirect(effect, tech, buff_args, offset);
		}
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

	void RenderEngine::GetCustomAttrib([[maybe_unused]] std::string_view name, [[maybe_unused]] void* value) const
	{
	}

	void RenderEngine::SetCustomAttrib([[maybe_unused]] std::string_view name, [[maybe_unused]] void* value)
	{
	}

	void RenderEngine::Resize(uint32_t width, uint32_t height)
	{
		uint32_t const old_screen_width = default_frame_buffers_[3]->Width();
		uint32_t const old_screen_height = default_frame_buffers_[3]->Height();

		WindowPtr const & win = Context::Instance().AppInstance().MainWnd();
		float const eff_dpi_scale = win->EffectiveDPIScale();
		uint32_t const new_screen_width = static_cast<uint32_t>(width * eff_dpi_scale + 0.5f);
		uint32_t const new_screen_height = static_cast<uint32_t>(height * eff_dpi_scale + 0.5f);

		uint32_t const new_render_width = static_cast<uint32_t>(width * default_render_width_scale_ + 0.5f);
		uint32_t const new_render_height = static_cast<uint32_t>(height * default_render_height_scale_ + 0.5f);

		if ((old_screen_width != new_screen_width) || (old_screen_height != new_screen_height))
		{
			this->DoResize(new_screen_width, new_screen_height);

			RenderSettings const & settings = Context::Instance().Config().graphics_cfg;
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			DepthStencilViewPtr ds_dsv;
			if (hdr_pp_ || post_tone_mapping_pp_ || (stereo_method_ != STM_None))
			{
				ElementFormat fmt = ds_tex_->Format();
				ds_tex_ = this->ScreenDepthStencilTexture();
				if (ds_tex_ && (new_screen_width == new_render_width) && (new_screen_height == new_render_height))
				{
					ds_srv_ = rf.MakeTextureSrv(ds_tex_);
					ds_dsv = rf.Make2DDsv(ds_tex_, 0, 1, 0);
				}
				else
				{
					if (caps.depth_texture_support)
					{
						ds_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
						KLAYGE_TEXTURE_DEBUG_NAME(ds_tex_);
						ds_srv_ = rf.MakeTextureSrv(ds_tex_);
						ds_dsv = rf.Make2DDsv(ds_tex_, 0, 1, 0);
					}
					else
					{
						if ((settings.depth_stencil_fmt != EF_Unknown)
							&& caps.RenderTargetFormatSupport(settings.depth_stencil_fmt, 1, 0))
						{
							fmt = settings.depth_stencil_fmt;
						}
						else
						{
							BOOST_ASSERT(caps.RenderTargetFormatSupport(EF_D16, 1, 0));

							fmt = EF_D16;
						}
						ds_dsv = rf.Make2DDsv(new_render_width, new_render_height, fmt, 1, 0);
					}
				}
			}

			default_frame_buffers_[0] = default_frame_buffers_[1] = default_frame_buffers_[2] = default_frame_buffers_[3]
				= screen_frame_buffer_;

			if ((stereo_method_ != STM_None) || (display_output_method_ != DOM_sRGB))
			{
				ElementFormat fmt = mono_tex_->Format();
				mono_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(mono_tex_);
				mono_srv_ = rf.MakeTextureSrv(mono_tex_);
				mono_rtv_ = rf.Make2DRtv(mono_tex_, 0, 1, 0);
				mono_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, mono_rtv_);

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
						resize_frame_buffer_->Viewport()->Camera(cur_frame_buffer_->Viewport()->Camera());
					}

					ElementFormat fmt;
					if (resize_tex_)
					{
						fmt = resize_tex_->Format();
					}
					else
					{
						fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
						BOOST_ASSERT(fmt != EF_Unknown);
					}

					resize_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
					KLAYGE_TEXTURE_DEBUG_NAME(resize_tex_);
					resize_srv_ = rf.MakeTextureSrv(resize_tex_);
					resize_rtv_ = rf.Make2DRtv(resize_tex_, 0, 1, 0);
					resize_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, resize_rtv_);

					ElementFormat ds_fmt;
					if ((settings.depth_stencil_fmt != EF_Unknown) && caps.RenderTargetFormatSupport(settings.depth_stencil_fmt, 1, 0))
					{
						ds_fmt = settings.depth_stencil_fmt;
					}
					else
					{
						BOOST_ASSERT(caps.RenderTargetFormatSupport(EF_D16, 1, 0));

						ds_fmt = EF_D16;
					}
					resize_frame_buffer_->Attach(rf.Make2DDsv(new_render_width, new_render_height, ds_fmt, 1, 0));

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
			if (smaa_edge_detection_pp_ || smaa_blending_weight_pp_)
			{
				smaa_edges_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1,
					smaa_edges_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(smaa_edges_tex_);
				smaa_edges_srv_ = rf.MakeTextureSrv(smaa_edges_tex_);
				smaa_edges_rtv_ = rf.Make2DRtv(smaa_edges_tex_, 0, 1, 0);
				smaa_blend_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1,
					smaa_blend_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(smaa_blend_tex_);
				smaa_blend_srv_ = rf.MakeTextureSrv(smaa_blend_tex_);
				smaa_blend_rtv_ = rf.Make2DRtv(smaa_blend_tex_, 0, 1, 0);

				auto smaa_ds_dsv = rf.Make2DDsv(new_render_width, new_render_height, EF_D24S8, 1, 0);

				smaa_edge_detection_pp_->OutputPin(0, smaa_edges_rtv_);
				smaa_edge_detection_pp_->OutputFrameBuffer()->Attach(smaa_ds_dsv);

				smaa_blending_weight_pp_->InputPin(0, smaa_edges_srv_);
				smaa_blending_weight_pp_->OutputPin(0, smaa_blend_rtv_);
				smaa_blending_weight_pp_->OutputFrameBuffer()->Attach(smaa_ds_dsv);
			}
			if (post_tone_mapping_pp_)
			{
				ElementFormat fmt = post_tone_mapping_tex_->Format();
				post_tone_mapping_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 1, 1, fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write);
				KLAYGE_TEXTURE_DEBUG_NAME(post_tone_mapping_tex_);
				post_tone_mapping_srv_ = rf.MakeTextureSrv(post_tone_mapping_tex_);
				post_tone_mapping_rtv_ = rf.Make2DRtv(post_tone_mapping_tex_, 0, 1, 0);
				post_tone_mapping_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, post_tone_mapping_rtv_);
				post_tone_mapping_frame_buffer_->Attach(ds_dsv);

				default_frame_buffers_[0] = default_frame_buffers_[1] = post_tone_mapping_frame_buffer_;
			}
			if (hdr_pp_)
			{
				ElementFormat fmt = hdr_tex_->Format();
				hdr_tex_ = rf.MakeTexture2D(new_render_width, new_render_height, 4, 1, fmt, 1, 0,
					EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
				KLAYGE_TEXTURE_DEBUG_NAME(hdr_tex_);
				hdr_srv_ = rf.MakeTextureSrv(hdr_tex_);
				hdr_frame_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(hdr_tex_, 0, 1, 0));
				hdr_frame_buffer_->Attach(ds_dsv);

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
		app.OnResize(new_render_width, new_render_height);
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
				hdr_tex_->BuildMipSubLevels(TextureFilter::Linear);
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
		smaa_pp_perf_->Begin();
#endif
		if (ppaa_enabled_)
		{
			if (!skip)
			{
				CameraPtr const& camera = cur_frame_buffer_->Viewport()->Camera();
				float q = camera->FarPlane() / (camera->FarPlane() - camera->NearPlane());
				float2 near_q(camera->NearPlane() * q, q);
				smaa_edge_detection_pp_->SetParam(0, near_q);
				smaa_edge_detection_pp_->OutputFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Stencil,
					Color(0, 0, 0, 0), 1, 0);
				this->BindFrameBuffer(smaa_edge_detection_pp_->OutputFrameBuffer());
				smaa_edge_detection_pp_->Render();

				smaa_blending_weight_pp_->OutputFrameBuffer()->Clear(FrameBuffer::CBM_Color,
					Color(0, 0, 0, 0), 1, 0);
				this->BindFrameBuffer(smaa_blending_weight_pp_->OutputFrameBuffer());
				smaa_blending_weight_pp_->Render();
			}
		}
#ifndef KLAYGE_SHIP
		smaa_pp_perf_->End();
#endif

#ifndef KLAYGE_SHIP
		post_tone_mapping_pp_perf_->Begin();
#endif
		if (ppaa_enabled_ || gamma_enabled_ || color_grading_enabled_)
		{
			if (skip)
			{
				post_tone_mapping_pps_[0]->Apply();
			}
			else
			{
				post_tone_mapping_pp_->Apply();
			}
		}
		else
		{
			if (post_tone_mapping_pps_[0])
			{
				post_tone_mapping_pps_[0]->Apply();
			}
		}
#ifndef KLAYGE_SHIP
		post_tone_mapping_pp_perf_->End();
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
					this->DefaultFrameBuffer()->AttachedRtv(FrameBuffer::Attachment::Color0)->ClearColor(Color(0, 0, 0, 0));
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
			auto const & ds_dsv = this->DefaultFrameBuffer()->AttachedDsv();
			if (ds_dsv)
			{
				ds_dsv->ClearDepth(1.0f);
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
		if (post_tone_mapping_pp_)
		{
			pp_chain_dirty_ = true;
			ppaa_enabled_ = aa;
			post_tone_mapping_pp_ = post_tone_mapping_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	void RenderEngine::GammaEnabled(bool gamma)
	{
		if (post_tone_mapping_pp_)
		{
			pp_chain_dirty_ = true;
			gamma_enabled_ = gamma;
			post_tone_mapping_pp_ = post_tone_mapping_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	void RenderEngine::ColorGradingEnabled(bool cg)
	{
		if (post_tone_mapping_pp_)
		{
			pp_chain_dirty_ = true;
			color_grading_enabled_ = cg;
			post_tone_mapping_pp_ = post_tone_mapping_pps_[ppaa_enabled_ * 4 + gamma_enabled_ * 2 + color_grading_enabled_];
		}
	}

	uint32_t RenderEngine::NumRealizedCameraInstances() const
	{
		return (num_camera_instances_ == 0) ? cur_frame_buffer_->Viewport()->NumCameras() : num_camera_instances_;
	}

	void RenderEngine::Refresh()
	{
		auto& context = Context::Instance();
		if (context.AppInstance().MainWnd()->Active())
		{
			context.SceneManagerInstance().Update();

#ifndef KLAYGE_SHIP
			context.PerfProfilerInstance().CollectData();
#endif
		}
	}

	void RenderEngine::ConvertToDisplay()
	{
		if (display_output_method_ != DOM_sRGB)
		{
			BOOST_ASSERT(hdr_display_pp_);

			fb_stage_ = 3;

#ifndef KLAYGE_SHIP
			hdr_display_pp_perf_->Begin();
#endif

			this->BindFrameBuffer(screen_frame_buffer_);
			hdr_display_pp_->SetParam(0, static_cast<float>(paper_white_));
			hdr_display_pp_->SetParam(1, static_cast<float>(display_max_luminance_));
			hdr_display_pp_->Render();

#ifndef KLAYGE_SHIP
			hdr_display_pp_perf_->End();
#endif

			fb_stage_ = 0;
		}
		else if (stereo_method_ != STM_None)
		{
			fb_stage_ = 3;

#ifndef KLAYGE_SHIP
			stereoscopic_pp_perf_->Begin();
#endif

			this->BindFrameBuffer(screen_frame_buffer_);
			if (stereoscopic_pp_)
			{
				Camera const& camera = *screen_frame_buffer_->Viewport()->Camera();

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
					Viewport const& vp = *screen_frame_buffer_->Viewport();

					float w = 0.5f;
					float h = 1;
					float x_left = 0;
					float x_right = 0.5f;
					float y = 0;

					float aspect = static_cast<float>(vp.Width() / 2) / vp.Height();

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
				KFL_UNREACHABLE("Invalid stereo method");
			}

			stereoscopic_pp_ = SyncLoadPostProcess("Stereoscopic.ppml", pp_name);
		}

		pp_chain_dirty_ = true;
	}

	void RenderEngine::DisplayOutput(DisplayOutputMethod method)
	{
		display_output_method_ = method;

		if (display_output_method_ != DOM_sRGB)
		{
			std::string pp_name;
			switch (display_output_method_)
			{
			case DOM_HDR10:
				pp_name = "DisplayHDR10";
				break;

			default:
				KFL_UNREACHABLE("Invalid display output method");
			}
			hdr_display_pp_ = SyncLoadPostProcess("HDRDisplay.ppml", pp_name);

			hdr_enabled_ = true;
			gamma_enabled_ = false;
			color_grading_enabled_ = false;
		}
		else
		{
			hdr_display_pp_.reset();
		}

		pp_chain_dirty_ = true;
	}

	void RenderEngine::PaperWhiteNits(uint32_t nits)
	{
		paper_white_ = nits;
		this->UpdateHDRRescale();
	}

	void RenderEngine::DisplayMaxLuminanceNits(uint32_t nits)
	{
		display_max_luminance_ = nits;
		this->UpdateHDRRescale();
	}

	void RenderEngine::UpdateHDRRescale()
	{
		float constexpr N = 0.25f;
		hdr_rescale_ = log2(1 - N * paper_white_ / display_max_luminance_) / log2(1 - N);
	}

	void RenderEngine::AssemblePostProcessChain()
	{
		if (post_tone_mapping_pp_)
		{
			for (size_t i = 0; i < 12; ++ i)
			{
				post_tone_mapping_pps_[i]->OutputPin(0, RenderTargetViewPtr());
			}
		}
		if (hdr_pp_)
		{
			hdr_pp_->OutputPin(0, RenderTargetViewPtr());
			skip_hdr_pp_->OutputPin(0, RenderTargetViewPtr());
		}

		if (stereo_method_ != STM_None)
		{
			if (stereoscopic_pp_)
			{
				if (post_tone_mapping_pp_)
				{
					for (size_t i = 0; i < 12; ++ i)
					{
						post_tone_mapping_pps_[i]->OutputPin(0, mono_rtv_);
					}
				}
				if (hdr_pp_)
				{
					hdr_pp_->OutputPin(0, mono_rtv_);
					skip_hdr_pp_->OutputPin(0, mono_rtv_);
				}

				stereoscopic_pp_->InputPin(0, mono_srv_);
				stereoscopic_pp_->InputPin(1, ds_srv_);
				stereoscopic_pp_->InputPin(2, overlay_srv_);
			}
		}
		else if (display_output_method_ != DOM_sRGB)
		{
			// TODO: Make HDR output work with stereoscopic

			if (hdr_display_pp_)
			{
				if (post_tone_mapping_pp_)
				{
					for (size_t i = 0; i < 12; ++ i)
					{
						post_tone_mapping_pps_[i]->OutputPin(0, mono_rtv_);
					}
				}
				if (hdr_pp_)
				{
					hdr_pp_->OutputPin(0, mono_rtv_);
					skip_hdr_pp_->OutputPin(0, mono_rtv_);
				}

				hdr_display_pp_->InputPin(0, mono_srv_);
				hdr_display_pp_->InputPin(1, overlay_srv_);
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
				if (post_tone_mapping_pp_)
				{
					for (size_t i = 0; i < 12; ++ i)
					{
						post_tone_mapping_pps_[i]->OutputPin(0, resize_rtv_);
					}
				}
				if (hdr_pp_)
				{
					hdr_pp_->OutputPin(0, resize_rtv_);
					skip_hdr_pp_->OutputPin(0, resize_rtv_);
				}

				if (resize_pps_[0])
				{
					for (size_t i = 0; i < 2; ++ i)
					{
						resize_pps_[i]->InputPin(0, resize_srv_);
					}
				}
			}
		}

		if (post_tone_mapping_pp_)
		{
			if (hdr_pp_)
			{
				hdr_pp_->OutputPin(0, post_tone_mapping_rtv_);
				skip_hdr_pp_->OutputPin(0, post_tone_mapping_rtv_);
			}

			for (size_t i = 0; i < 12; ++ i)
			{
				post_tone_mapping_pps_[i]->InputPin(0, post_tone_mapping_srv_);
				post_tone_mapping_pps_[i]->InputPin(1, smaa_blend_srv_);
				post_tone_mapping_pps_[i]->InputPin(2, smaa_edges_srv_);
			}

			if (smaa_edge_detection_pp_)
			{
				smaa_edge_detection_pp_->InputPin(0, post_tone_mapping_srv_);
				smaa_edge_detection_pp_->InputPin(1, ds_srv_);
			}
		}

		if (hdr_pp_)
		{
			hdr_pp_->InputPin(0, hdr_srv_);
			skip_hdr_pp_->InputPin(0, hdr_srv_);
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
					auto rs_desc = cur_rs_obj_->GetRasterizerStateDesc();
					auto const & dss_desc = cur_rs_obj_->GetDepthStencilStateDesc();
					auto const & bs_desc = cur_rs_obj_->GetBlendStateDesc();
					rs_desc.polygon_mode = PM_Line;
					cur_line_rs_obj_ = Context::Instance().RenderFactoryInstance().MakeRenderStateObject(rs_desc, dss_desc, bs_desc);
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
		default_material_.reset();
		predefined_material_cb_.reset();
		predefined_mesh_cb_.reset();
		predefined_model_cb_.reset();
		predefined_camera_cb_.reset();

		mipmapper_.reset();

		cur_frame_buffer_.reset();
		screen_frame_buffer_.reset();
		ds_tex_.reset();
		ds_srv_.reset();
		hdr_frame_buffer_.reset();
		hdr_tex_.reset();
		hdr_srv_.reset();
		post_tone_mapping_frame_buffer_.reset();
		post_tone_mapping_tex_.reset();
		post_tone_mapping_srv_.reset();
		post_tone_mapping_rtv_.reset();
		resize_frame_buffer_.reset();
		resize_tex_.reset();
		resize_srv_.reset();
		resize_rtv_.reset();
		mono_frame_buffer_.reset();
		mono_tex_.reset();
		mono_srv_.reset();
		mono_rtv_.reset();
		for (int i = 0; i < 4; ++ i)
		{
			default_frame_buffers_[i].reset();
		}

		overlay_frame_buffer_.reset();
		overlay_tex_.reset();
		overlay_srv_.reset();

		smaa_edges_tex_.reset();
		smaa_edges_srv_.reset();
		smaa_edges_rtv_.reset();
		smaa_blend_tex_.reset();
		smaa_blend_srv_.reset();
		smaa_blend_rtv_.reset();

		so_buffers_.reset();

		cur_rs_obj_.reset();
		cur_line_rs_obj_.reset();

		pp_rl_.reset();
		vpp_rl_.reset();

		hdr_pp_.reset();
		skip_hdr_pp_.reset();
		smaa_edge_detection_pp_.reset();
		smaa_blending_weight_pp_.reset();
		post_tone_mapping_pp_.reset();
		resize_pps_[0].reset();
		resize_pps_[1].reset();
		hdr_display_pp_.reset();
		stereoscopic_pp_.reset();

		for (int i = 0; i < 12; ++ i)
		{
			post_tone_mapping_pps_[i].reset();
		}

		this->DoDestroy();
	}

	RenderMaterialPtr const& RenderEngine::DefaultMaterial() const
	{
		if (!default_material_)
		{
			std::lock_guard<std::mutex> lock(default_mtl_instance_mutex);
			if (!default_material_)
			{
				default_material_ = MakeSharedPtr<RenderMaterial>();
			}
		}
		return default_material_;
	}

	PredefinedMaterialCBuffer const& RenderEngine::PredefinedMaterialCBufferInstance() const
	{
		if (!predefined_material_cb_)
		{
			std::lock_guard<std::mutex> lock(mtl_cb_instance_mutex);
			if (!predefined_material_cb_)
			{
				predefined_material_cb_ = MakeUniquePtr<PredefinedMaterialCBuffer>();
			}
		}
		return *predefined_material_cb_;
	}

	PredefinedMeshCBuffer const& RenderEngine::PredefinedMeshCBufferInstance() const
	{
		if (!predefined_mesh_cb_)
		{
			std::lock_guard<std::mutex> lock(mesh_cb_instance_mutex);
			if (!predefined_mesh_cb_)
			{
				predefined_mesh_cb_ = MakeUniquePtr<PredefinedMeshCBuffer>();
			}
		}
		return *predefined_mesh_cb_;
	}

	PredefinedModelCBuffer const& RenderEngine::PredefinedModelCBufferInstance() const
	{
		if (!predefined_model_cb_)
		{
			std::lock_guard<std::mutex> lock(model_cb_instance_mutex);
			if (!predefined_model_cb_)
			{
				predefined_model_cb_ = MakeUniquePtr<PredefinedModelCBuffer>();
			}
		}
		return *predefined_model_cb_;
	}

	PredefinedCameraCBuffer const& RenderEngine::PredefinedCameraCBufferInstance() const
	{
		if (!predefined_camera_cb_)
		{
			std::lock_guard<std::mutex> lock(camera_cb_instance_mutex);
			if (!predefined_camera_cb_)
			{
				predefined_camera_cb_ = MakeUniquePtr<PredefinedCameraCBuffer>();
			}
		}
		return *predefined_camera_cb_;
	}

	Mipmapper const& RenderEngine::MipmapperInstance() const
	{
		if (!mipmapper_)
		{
			std::lock_guard<std::mutex> lock(mipmapper_instance_mutex);
			if (!mipmapper_)
			{
				mipmapper_ = MakeUniquePtr<Mipmapper>();
			}
		}
		return *mipmapper_;
	}
}
