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
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>

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

		void StartRendering()
		{
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

		uint16_t StencilBufferBitDepth()
		{
			return 0;
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
			stereo_mode_(false), stereo_separation_(0), stereo_active_eye_(0)
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
		this->DoCreateRenderWindow(name, settings);
		screen_frame_buffer_ = cur_frame_buffer_;
		this->StereoMode(settings.stereo_mode);
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
		if (fb != cur_frame_buffer_)
		{
			if (cur_frame_buffer_)
			{
				cur_frame_buffer_->OnUnbind();
			}

			if (!fb)
			{
				if (stereo_mode_)
				{
					cur_frame_buffer_ = stereo_frame_buffers_[stereo_active_eye_];
				}
				else
				{
					cur_frame_buffer_ = screen_frame_buffer_;
				}
			}
			else
			{
				cur_frame_buffer_ = fb;
			}

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
		if (stereo_mode_)
		{
			return stereo_frame_buffers_[stereo_active_eye_];
		}
		else
		{
			return screen_frame_buffer_;
		}
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

	void RenderEngine::Resize(uint32_t width, uint32_t height)
	{
		this->DoResize(width, height);

		if (stereo_mode_)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			for (int i = 0; i < 2; ++ i)
			{
				stereo_colors_[i] = rf.MakeTexture2D(width, height, 1, 1, stereo_colors_[i]->Format(),
					stereo_colors_[i]->SampleCount(), stereo_colors_[i]->SampleQuality(),
					EAH_GPU_Read | EAH_GPU_Write, NULL);
				stereo_frame_buffers_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*stereo_colors_[i], 0, 0));

				RenderViewPtr ds_view = rf.Make2DDepthStencilRenderView(screen_frame_buffer_->Width(), screen_frame_buffer_->Height(),
					stereo_frame_buffers_[i]->Attached(FrameBuffer::ATT_DepthStencil)->Format(),
					stereo_colors_[i]->SampleCount(), stereo_colors_[i]->SampleQuality());
				stereo_frame_buffers_[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
			}
		}
	}

	void RenderEngine::Stereoscopic()
	{
		if (stereo_mode_)
		{
			this->BindFrameBuffer(screen_frame_buffer_);

			float4 texel_to_pixel = this->TexelToPixelOffset();
			texel_to_pixel.x() /= screen_frame_buffer_->Width() / 2.0f;
			texel_to_pixel.y() /= screen_frame_buffer_->Height() / 2.0f;
			*texel_to_pixel_offset_ep_ = texel_to_pixel;

			*left_tex_ep_ = stereo_colors_[0];
			*right_tex_ep_ = stereo_colors_[1];

			this->Render(*stereoscopic_tech_, *stereoscopic_rl_);
		}
	}

	void RenderEngine::CreateStereoscopicVB()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		stereoscopic_rl_ = rf.MakeRenderLayout();
		stereoscopic_rl_->TopologyType(RenderLayout::TT_TriangleStrip);

		float2 pos[] =
		{
			float2(-1, +1),
			float2(+1, +1),
			float2(-1, -1),
			float2(+1, -1)
		};
		ElementInitData init_data;
		init_data.row_pitch = sizeof(pos);
		init_data.data = &pos[0];
		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		stereoscopic_rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

		stereoscopic_effect_ = rf.LoadEffect("Stereoscopic.fxml");
		stereoscopic_tech_ = stereoscopic_effect_->TechniqueByName("RedCyan");
		texel_to_pixel_offset_ep_ = stereoscopic_effect_->ParameterByName("texel_to_pixel_offset");
		left_tex_ep_ = stereoscopic_effect_->ParameterByName("left_tex");
		right_tex_ep_ = stereoscopic_effect_->ParameterByName("right_tex");
		*(stereoscopic_effect_->ParameterByName("flipping")) = static_cast<int32_t>(stereo_frame_buffers_[0]->RequiresFlipping() ? -1 : +1);;
	}

	void RenderEngine::StereoMode(bool stereo)
	{
		stereo_mode_ = stereo;
		stereo_active_eye_ = 0;
		if (stereo_mode_)
		{
			screen_frame_buffer_->GetViewport().camera->StereoMode(stereo_mode_);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			for (int i = 0; i < 2; ++ i)
			{
				stereo_frame_buffers_[i] = rf.MakeFrameBuffer();
				stereo_frame_buffers_[i]->GetViewport().camera = screen_frame_buffer_->GetViewport().camera;

				stereo_colors_[i] = rf.MakeTexture2D(screen_frame_buffer_->Width(), screen_frame_buffer_->Height(),
					1, 1, screen_frame_buffer_->Format(), render_settings_.sample_count, render_settings_.sample_quality,
					EAH_GPU_Read | EAH_GPU_Write, NULL);
				stereo_frame_buffers_[i]->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*stereo_colors_[i], 0, 0));

				RenderViewPtr ds_view = rf.Make2DDepthStencilRenderView(screen_frame_buffer_->Width(), screen_frame_buffer_->Height(),
					render_settings_.depth_stencil_fmt, render_settings_.sample_count, render_settings_.sample_quality);
				stereo_frame_buffers_[i]->Attach(FrameBuffer::ATT_DepthStencil, ds_view);
			}

			cur_frame_buffer_ = stereo_frame_buffers_[stereo_active_eye_];

			this->CreateStereoscopicVB();
		}
		else
		{
			cur_frame_buffer_ = screen_frame_buffer_;
		}
	}
}
