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

#include <KlayGE/RenderEngine.hpp>

namespace KlayGE
{
	class NullRenderEngine : public RenderEngine
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Render Engine");
			return name;
		}

		bool RequiresFlipping() const
		{
			return false;
		}

		void BeginFrame()
		{
		}
		void EndFrame()
		{
		}

		void BeginPass()
		{
		}
		void EndPass()
		{
		}

		void ForceFlush()
		{
		}

		void ScissorRect(uint32_t /*x*/, uint32_t /*y*/, uint32_t /*width*/, uint32_t /*height*/)
		{
		}

		float4 TexelToPixelOffset() const
		{
			return float4(0, 0, 0, 0);
		}

		bool FullScreen() const
		{
			return false;
		}

		void FullScreen(bool /*fs*/)
		{
		}

	private:
		void DoCreateRenderWindow(std::string const & /*name*/, RenderSettings const & /*settings*/)
		{
		}

		void DoBindFrameBuffer(FrameBufferPtr const & /*fb*/)
		{
		}

		void DoBindSOBuffers(RenderLayoutPtr const & /*rl*/)
		{
		}

		void DoRender(RenderTechnique const & /*tech*/, RenderLayout const & /*rl*/)
		{
		}

		void DoDispatch(RenderTechnique const & /*tech*/, uint32_t /*tgx*/, uint32_t /*tgy*/, uint32_t /*tgz*/)
		{
		}

		void DoResize(uint32_t /*width*/, uint32_t /*height*/)
		{
		}
	};

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderEngine()
		: numPrimitivesJustRendered_(0),
			numVerticesJustRendered_(0),
			cur_front_stencil_ref_(0),
			cur_back_stencil_ref_(0),
			cur_blend_factor_(1, 1, 1, 1),
			cur_sample_mask_(0xFFFFFFFF),
			motion_frames_(0),
			stereo_method_(STM_None), stereo_separation_(0),
			fb_stage_(0)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::~RenderEngine()
	{
	}

	// 返回空对象
	/////////////////////////////////////////////////////////////////////////////////
	RenderEnginePtr RenderEngine::NullObject()
	{
		static RenderEnginePtr obj = MakeSharedPtr<NullRenderEngine>();
		return obj;
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::CreateRenderWindow(std::string const & name, RenderSettings const & settings)
	{
		render_settings_ = settings;
		stereo_separation_ = settings.stereo_separation;
		this->DoCreateRenderWindow(name, render_settings_);
		this->CheckConfig();
		RenderDeviceCaps const & caps = this->DeviceCaps();

		screen_frame_buffer_ = cur_frame_buffer_;
		if (render_settings_.hdr)
		{
			hdr_pp_ = MakeSharedPtr<HDRPostProcess>(render_settings_.fft_lens_effects);
			skip_hdr_pp_ = LoadPostProcess(ResLoader::Instance().Open("Copy.ppml"), "copy");
		}
		hdr_enabled_ = render_settings_.hdr;
		if (render_settings_.ppaa)
		{
			ppaa_pp_ = LoadPostProcess(ResLoader::Instance().Open("FXAA.ppml"), "fxaa");
			ppaa_show_edge_pp_ = LoadPostProcess(ResLoader::Instance().Open("FXAA.ppml"), "fxaa_show_edge");
			skip_ppaa_pp_ = LoadPostProcess(ResLoader::Instance().Open("Copy.ppml"), "copy");
		}
		ppaa_enabled_ = render_settings_.ppaa ? 1 : 0;
		if (render_settings_.color_grading)
		{
			color_grading_pp_ = LoadPostProcess(ResLoader::Instance().Open("ColorGrading.ppml"), "color_grading");
			color_grading_pp_->SetParam(0, int2(render_settings_.gamma, render_settings_.color_grading));
			skip_color_grading_pp_ = LoadPostProcess(ResLoader::Instance().Open("Copy.ppml"), "copy");
		}
		else
		{
			if (render_settings_.gamma)
			{
				color_grading_pp_ = LoadPostProcess(ResLoader::Instance().Open("GammaCorrection.ppml"), "gamma_correction");
				color_grading_pp_->SetParam(0, 1 / 2.2f);
				skip_color_grading_pp_ = LoadPostProcess(ResLoader::Instance().Open("Copy.ppml"), "copy");
			}
		}

		for (int i = 0; i < 5; ++ i)
		{
			default_frame_buffers_[i] = screen_frame_buffer_;
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		uint32_t const width = screen_frame_buffer_->Width();
		uint32_t const height = screen_frame_buffer_->Height();

		RenderViewPtr ds_view;
		if (hdr_pp_ || ppaa_pp_ || color_grading_pp_ || (render_settings_.stereo_method != STM_None))
		{
			if (caps.texture_format_support(EF_D24S8) || caps.texture_format_support(EF_D16))
			{
				ElementFormat fmt;
				if (caps.texture_format_support(render_settings_.depth_stencil_fmt))
				{
					fmt = render_settings_.depth_stencil_fmt;
				}
				else
				{
					BOOST_ASSERT(caps.texture_format_support(EF_D16));

					fmt = EF_D16;
				}
				ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
			}
			else
			{
				ElementFormat fmt;
				if (caps.rendertarget_format_support(render_settings_.depth_stencil_fmt, 1, 0))
				{
					fmt = render_settings_.depth_stencil_fmt;
				}
				else
				{
					BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

					fmt = EF_D16;
				}
				ds_view = rf.Make2DDepthStencilRenderView(width, height, fmt, 1, 0);
			}
		}

		if (render_settings_.stereo_method != STM_None)
		{
			mono_frame_buffer_ = rf.MakeFrameBuffer();
			mono_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.rendertarget_format_support(render_settings_.color_fmt, 1, 0))
			{
				fmt = render_settings_.color_fmt;
			}
			else
			{
				if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
				{
					fmt = EF_ABGR8;
				}
				else
				{
					BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

					fmt = EF_ARGB8;
				}
			}

			mono_tex_ = rf.MakeTexture2D(width, height, 1, 1,
				fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			mono_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*mono_tex_, 0, 1, 0));
			mono_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			default_frame_buffers_[0] = default_frame_buffers_[1]
				= default_frame_buffers_[2] = default_frame_buffers_[3] = mono_frame_buffer_;
		}

		if (color_grading_pp_)
		{
			cg_frame_buffer_ = rf.MakeFrameBuffer();
			cg_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

				fmt = EF_ARGB8;
			}
			ElementFormat fmt_srgb = MakeSRGB(fmt);
			if (caps.rendertarget_format_support(fmt_srgb, 1, 0))
			{
				fmt = fmt_srgb;
			}

			cg_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			cg_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*cg_tex_, 0, 1, 0));
			cg_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			default_frame_buffers_[0] = default_frame_buffers_[1] = default_frame_buffers_[2] = cg_frame_buffer_;
		}

		if (ppaa_pp_)
		{
			ldr_frame_buffer_ = rf.MakeFrameBuffer();
			ldr_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.rendertarget_format_support(EF_ABGR8, 1, 0))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ARGB8, 1, 0));

				fmt = EF_ARGB8;
			}
			ElementFormat fmt_srgb = MakeSRGB(fmt);
			if (caps.rendertarget_format_support(fmt_srgb, 1, 0))
			{
				fmt = fmt_srgb;
			}

			ldr_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			ldr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ldr_tex_, 0, 1, 0));
			ldr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			default_frame_buffers_[0] = default_frame_buffers_[1] = ldr_frame_buffer_;
		}

		if (hdr_pp_)
		{
			hdr_frame_buffer_ = rf.MakeFrameBuffer();
			hdr_frame_buffer_->GetViewport()->camera = cur_frame_buffer_->GetViewport()->camera;

			ElementFormat fmt;
			if (caps.rendertarget_format_support(EF_B10G11R11F, 1, 0))
			{
				fmt = EF_B10G11R11F;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

				fmt = EF_ABGR16F;
			}
			hdr_tex_ = rf.MakeTexture2D(width, height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			hdr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0, 1, 0));
			hdr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

			default_frame_buffers_[0] = hdr_frame_buffer_;
		}

		this->BindFrameBuffer(default_frame_buffers_[0]);
		this->Stereo(settings.stereo_method);
	}

	void RenderEngine::CheckConfig()
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
			rs_obj->Active();
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
		if (!fb)
		{
			new_fb = this->DefaultFrameBuffer();
		}
		else
		{
			new_fb = fb;
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

	void RenderEngine::BindSOBuffers(RenderLayoutPtr const & rl)
	{
		so_buffers_ = rl;
		this->DoBindSOBuffers(rl);
	}

	// 渲染一个vb
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::Render(RenderTechnique const & tech, RenderLayout const & rl)
	{
		this->DoRender(tech, rl);
	}

	void RenderEngine::Dispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		this->DoDispatch(tech, tgx, tgy, tgz);
	}

	// 上次Render()所渲染的图元数
	/////////////////////////////////////////////////////////////////////////////////
	size_t RenderEngine::NumPrimitivesJustRendered()
	{
		size_t const ret = numPrimitivesJustRendered_;
		numPrimitivesJustRendered_ = 0;
		return ret;
	}

	// 上次Render()所渲染的顶点数
	/////////////////////////////////////////////////////////////////////////////////
	size_t RenderEngine::NumVerticesJustRendered()
	{
		size_t const ret = numVerticesJustRendered_;
		numVerticesJustRendered_ = 0;
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
		this->DoResize(width, height);
		
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

		RenderViewPtr ds_view;
		if (hdr_pp_ || ppaa_pp_ || color_grading_pp_ || (stereo_method_ != STM_None))
		{
			if (caps.texture_format_support(EF_D24S8) || caps.texture_format_support(EF_D16))
			{
				ElementFormat fmt = ds_tex_->Format();
				ds_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
				ds_view = rf.Make2DDepthStencilRenderView(*ds_tex_, 0, 1, 0);
			}
			else
			{
				ElementFormat fmt;
				if (caps.rendertarget_format_support(render_settings_.depth_stencil_fmt, 1, 0))
				{
					fmt = render_settings_.depth_stencil_fmt;
				}
				else
				{
					BOOST_ASSERT(caps.rendertarget_format_support(EF_D16, 1, 0));

					fmt = EF_D16;
				}
				ds_view = rf.Make2DDepthStencilRenderView(width, height, fmt, 1, 0);
			}
		}

		if (stereo_method_ != STM_None)
		{
			ElementFormat fmt = mono_tex_->Format();
			mono_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			mono_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*mono_tex_, 0, 1, 0));
			mono_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}
		if (color_grading_pp_)
		{
			ElementFormat fmt = cg_tex_->Format();
			cg_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			cg_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*cg_tex_, 0, 1, 0));
			cg_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}
		if (ppaa_pp_)
		{
			ElementFormat fmt = ldr_tex_->Format();
			ldr_tex_ = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			ldr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*ldr_tex_, 0, 1, 0));
			ldr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}
		if (hdr_pp_)
		{
			ElementFormat fmt = hdr_tex_->Format();
			hdr_tex_ = rf.MakeTexture2D(width, height, 4, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips, nullptr);
			hdr_frame_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*hdr_tex_, 0, 1, 0));
			hdr_frame_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
		}

		this->AssemblePostProcessChain();
	}

	void RenderEngine::PostProcess(bool skip)
	{
		fb_stage_ = 1;

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

		fb_stage_ = 2;

		if (ppaa_enabled_)
		{
			if (skip)
			{
				skip_ppaa_pp_->Apply();
			}
			else
			{
				if (ppaa_enabled_ > 1)
				{
					ppaa_show_edge_pp_->Apply();
				}
				else
				{
					ppaa_pp_->Apply();
				}
			}
		}
		else
		{
			if (skip_ppaa_pp_)
			{
				skip_ppaa_pp_->Apply();
			}
		}

		fb_stage_ = 3;

		if (color_grading_pp_)
		{
			color_grading_pp_->Apply();
		}
		
		fb_stage_ = 4;

		if (stereo_method_ != STM_None)
		{
			this->Stereoscopic();
		}

		this->DefaultFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepth(1.0f);

		fb_stage_ = 0;
	}

	void RenderEngine::HDREnabled(bool hdr)
	{
		if (hdr_pp_)
		{
			hdr_enabled_ = hdr;
		}
	}

	void RenderEngine::PPAAEnabled(int aa)
	{
		if (ppaa_pp_)
		{
			ppaa_enabled_ = aa;
		}
	}

	void RenderEngine::ColorGradingEnabled(bool cg)
	{
		if (color_grading_pp_)
		{
			color_grading_pp_->SetParam(0, int2(render_settings_.gamma, cg));
		}
	}

	void RenderEngine::Refresh()
	{
		FrameBuffer& fb = *this->ScreenFrameBuffer();
		if (fb.Active())
		{
			Context::Instance().SceneManagerInstance().Update();
			fb.SwapBuffers();
		}
	}

	void RenderEngine::Stereoscopic()
	{
		BOOST_ASSERT(stereo_method_ != STM_None);

		this->BindFrameBuffer(screen_frame_buffer_);
		if (stereoscopic_pp_)
		{
			stereoscopic_pp_->SetParam(0, stereo_separation_);
			stereoscopic_pp_->SetParam(1, screen_frame_buffer_->GetViewport()->camera->NearPlane());
		}
		if (stereo_method_ != STM_LCDShutter)
		{
			stereoscopic_pp_->Render();
		}
		else
		{
			stereoscopic_pp_->Apply();
			this->StereoscopicForLCDShutter();
			this->BindFrameBuffer(screen_frame_buffer_);
		}
	}

	void RenderEngine::StereoscopicForLCDShutter()
	{
	}

	void RenderEngine::Stereo(StereoMethod method)
	{
		stereo_method_ = method;
		if (stereo_method_ != STM_None)
		{
			ResIdentifierPtr stereo_ppml = ResLoader::Instance().Open("Stereoscopic.ppml");
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

			default:
				BOOST_ASSERT(false);
				break;
			}

			stereoscopic_pp_ = LoadPostProcess(stereo_ppml, pp_name);

			if (STM_LCDShutter == stereo_method_)
			{
				stereo_lr_tex_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(mono_tex_->Width(0) * 2,
					mono_tex_->Height(0), 1, 1, mono_tex_->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			}
		}

		this->AssemblePostProcessChain();
	}

	void RenderEngine::AssemblePostProcessChain()
	{
		if (stereo_method_ != STM_None)
		{
			if (stereoscopic_pp_)
			{
				if (color_grading_pp_)
				{
					color_grading_pp_->OutputPin(0, mono_tex_);
					skip_color_grading_pp_->OutputPin(0, mono_tex_);
				}
				if (ppaa_pp_)
				{
					ppaa_pp_->OutputPin(0, mono_tex_);
					ppaa_show_edge_pp_->OutputPin(0, mono_tex_);
					skip_ppaa_pp_->OutputPin(0, mono_tex_);
				}
				if (hdr_pp_)
				{
					hdr_pp_->OutputPin(0, mono_tex_);
					skip_hdr_pp_->OutputPin(0, mono_tex_);
				}

				stereoscopic_pp_->InputPin(0, mono_tex_);
				stereoscopic_pp_->InputPin(1, ds_tex_);
				if (STM_LCDShutter == stereo_method_)
				{
					stereoscopic_pp_->OutputPin(0, stereo_lr_tex_);
				}
				else
				{
					stereoscopic_pp_->OutputPin(0, TexturePtr());
				}
			}
		}
		else
		{
			if (color_grading_pp_)
			{
				color_grading_pp_->OutputPin(0, TexturePtr());
				skip_color_grading_pp_->OutputPin(0, TexturePtr());
			}
			if (ppaa_pp_)
			{
				ppaa_pp_->OutputPin(0, TexturePtr());
				ppaa_show_edge_pp_->OutputPin(0, TexturePtr());
				skip_ppaa_pp_->OutputPin(0, TexturePtr());
			}
			if (hdr_pp_)
			{
				hdr_pp_->OutputPin(0, TexturePtr());
				skip_hdr_pp_->OutputPin(0, TexturePtr());
			}
		}

		if (color_grading_pp_)
		{
			if (ppaa_pp_)
			{
				ppaa_pp_->OutputPin(0, cg_tex_);
				ppaa_show_edge_pp_->OutputPin(0, cg_tex_);
				skip_ppaa_pp_->OutputPin(0, cg_tex_);
			}
			if (hdr_pp_)
			{
				hdr_pp_->OutputPin(0, cg_tex_);
				skip_hdr_pp_->OutputPin(0, cg_tex_);
			}

			color_grading_pp_->InputPin(0, cg_tex_);
		}

		if (ppaa_pp_)
		{
			if (hdr_pp_)
			{
				hdr_pp_->OutputPin(0, ldr_tex_);
				skip_hdr_pp_->OutputPin(0, ldr_tex_);
			}

			ppaa_pp_->InputPin(0, ldr_tex_);
			ppaa_show_edge_pp_->InputPin(0, ldr_tex_);
			skip_ppaa_pp_->InputPin(0, ldr_tex_);
		}

		if (hdr_pp_)
		{
			hdr_pp_->InputPin(0, hdr_tex_);
			skip_hdr_pp_->InputPin(0, hdr_tex_);
		}
	}
}
