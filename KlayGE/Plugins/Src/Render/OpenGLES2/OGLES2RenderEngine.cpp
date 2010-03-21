// OGLES2RenderEngine.cpp
// KlayGE OpenGL ES 2渲染引擎类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <glloader/glloader.h>
#ifdef Bool
#undef Bool		// for boost::foreach
#endif

#include <algorithm>
#include <sstream>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/OpenGLES2/OGLES2Mapping.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderWindow.hpp>
#include <KlayGE/OpenGLES2/OGLES2FrameBuffer.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderView.hpp>
#include <KlayGE/OpenGLES2/OGLES2Texture.hpp>
#include <KlayGE/OpenGLES2/OGLES2GraphicsBuffer.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderLayout.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderEngine.hpp>
#include <KlayGE/OpenGLES2/OGLES2RenderStateObject.hpp>
#include <KlayGE/OpenGLES2/OGLES2ShaderObject.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#if defined(KLAYGE_CPU_X86)
	#ifdef KLAYGE_DEBUG
		#pragma comment(lib, "glloader_es_x86_d.lib")
	#else
		#pragma comment(lib, "glloader_es_x86.lib")
	#endif
#elif defined(KLAYGE_CPU_X64)
	#ifdef KLAYGE_DEBUG
		#pragma comment(lib, "glloader_es_x64_d.lib")
	#else
		#pragma comment(lib, "glloader_es_x64.lib")
	#endif
#endif
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLES2RenderEngine::OGLES2RenderEngine()
		: clear_depth_(1), clear_stencil_(0),
			vp_x_(0), vp_y_(0), vp_width_(0), vp_height_(0),
			cur_fbo_(0)
	{
		clear_clr_.assign(0);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLES2RenderEngine::~OGLES2RenderEngine()
	{
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & OGLES2RenderEngine::Name() const
	{
		static const std::wstring name(L"OpenGL ES 2 Render Engine");
		return name;
	}

	// 开始渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::StartRendering()
	{
#if defined KLAYGE_PLATFORM_WINDOWS
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
				gotMsg = ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ? true : false;
			}
			else
			{
				gotMsg = ::GetMessage(&msg, NULL, 0, 0) ? true : false;
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
#elif defined KLAYGE_PLATFORM_LINUX
		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		::Display* x_display = main_wnd->XDisplay();
		XEvent event;
		for (;;)
		{
			do
			{
				XNextEvent(x_display, &event);
				main_wnd->MsgProc(event);
			} while(XPending(x_display));

			FrameBuffer& fb = *this->CurFrameBuffer();
			if (fb.Active())
			{
				Context::Instance().SceneManagerInstance().Update();
				fb.SwapBuffers();
			}
		}
#endif
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		motion_frames_ = settings.motion_frames;

		FrameBufferPtr win = MakeSharedPtr<OGLES2RenderWindow>(name, settings);

		this->FillRenderDeviceCaps();
		this->InitRenderStates();

		win->Attach(FrameBuffer::ATT_Color0,
			MakeSharedPtr<OGLES2ScreenColorRenderView>(win->Width(), win->Height(), win->Format()));
		if (win->DepthBits() > 0)
		{
			win->Attach(FrameBuffer::ATT_DepthStencil,
				MakeSharedPtr<OGLES2ScreenDepthStencilRenderView>(win->Width(), win->Height(), settings.depth_stencil_fmt));
		}

		this->BindFrameBuffer(win);
	}

	void OGLES2RenderEngine::InitRenderStates()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRasterizerStateObject(RasterizerStateDesc());
		cur_dss_obj_ = rf.MakeDepthStencilStateObject(DepthStencilStateDesc());
		cur_bs_obj_ = rf.MakeBlendStateObject(BlendStateDesc());
		checked_pointer_cast<OGLES2RasterizerStateObject>(cur_rs_obj_)->ForceDefaultState();
		checked_pointer_cast<OGLES2DepthStencilStateObject>(cur_dss_obj_)->ForceDefaultState();
		checked_pointer_cast<OGLES2BlendStateObject>(cur_bs_obj_)->ForceDefaultState();

		glEnable(GL_POLYGON_OFFSET_FILL);
	}

	void OGLES2RenderEngine::TexParameter(GLuint tex, GLenum target, GLenum pname, GLint param)
	{
		glBindTexture(target, tex);

		GLint tmp;
		glGetTexParameteriv(target, pname, &tmp);
		if (tmp != param)
		{
			glTexParameteri(target, pname, param);
		}
	}

	void OGLES2RenderEngine::TexParameterf(GLuint tex, GLenum target, GLenum pname, GLfloat param)
	{
		glBindTexture(target, tex);

		GLfloat tmp;
		glGetTexParameterfv(target, pname, &tmp);
		if (tmp != param)
		{
			glTexParameterf(target, pname, param);
		}
	}

	void OGLES2RenderEngine::ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
	{
		if ((clear_clr_[0] != r) || (clear_clr_[1] != g) || (clear_clr_[2] != b) || (clear_clr_[3] != a))
		{
			glClearColor(r, g, b, a);
			clear_clr_[0] = r;
			clear_clr_[1] = g;
			clear_clr_[2] = b;
			clear_clr_[3] = a;
		}
	}

	void OGLES2RenderEngine::ClearDepth(GLfloat depth)
	{
		if (depth != clear_depth_)
		{
			glClearDepthf(depth);
			clear_depth_ = depth;
		}
	}

	void OGLES2RenderEngine::ClearStencil(GLuint stencil)
	{
		if (stencil != clear_stencil_)
		{
			glClearStencil(stencil);
			clear_stencil_ = stencil;
		}
	}

	void OGLES2RenderEngine::BindFramebuffer(GLuint fbo, bool force)
	{
		if (force || (cur_fbo_ != fbo))
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			cur_fbo_ = fbo;
		}
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
	{
		BOOST_ASSERT(fb);

		Viewport const & vp = fb->GetViewport();
		if ((vp_x_ != vp.left) || (vp_y_ != vp.top) || (vp_width_ != vp.width) || (vp_height_ != vp.height))
		{
			glViewport(vp.left, vp.top, vp.width, vp.height);

			vp_x_ = vp.left;
			vp_y_ = vp.top;
			vp_width_ = vp.width;
			vp_height_ = vp.height;
		}
	}

	// 设置当前Stream output目标
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::DoBindSOBuffers(RenderLayoutPtr const & /*rl*/)
	{
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::BeginFrame()
	{
	}

	// 开始一个Pass
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::BeginPass()
	{
	}
	
	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::DoRender(RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t const num_instance = rl.NumInstance();
		BOOST_ASSERT(num_instance != 0);

		checked_cast<OGLES2RenderLayout const *>(&rl)->Active(tech.Pass(0)->GetShaderObject());

		size_t const vertexCount = rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();
		GLenum mode;
		uint32_t primCount;
		OGLES2Mapping::Mapping(mode, primCount, rl);

		numPrimitivesJustRendered_ += num_instance * primCount;
		numVerticesJustRendered_ += num_instance * vertexCount;

		GLenum index_type = GL_UNSIGNED_SHORT;
		if (rl.UseIndices())
		{
			OGLES2GraphicsBuffer& stream(*checked_pointer_cast<OGLES2GraphicsBuffer>(rl.GetIndexStream()));
			stream.Active();

			if (EF_R16UI == rl.IndexStreamFormat())
			{
				index_type = GL_UNSIGNED_SHORT;
			}
			else
			{
				index_type = GL_UNSIGNED_INT;
			}
		}

		uint32_t const num_passes = tech.NumPasses();

		for (uint32_t instance = 0; instance < num_instance; ++ instance)
		{
			BOOST_ASSERT(!rl.InstanceStream());

			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					glDrawElements(mode, static_cast<GLsizei>(rl.NumIndices()),
						index_type, 0);
					pass->Unbind();
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					glDrawArrays(mode, 0, static_cast<GLsizei>(rl.NumVertices()));
					pass->Unbind();
				}
			}
		}

		checked_cast<OGLES2RenderLayout const *>(&rl)->Deactive(tech.Pass(0)->GetShaderObject());
	}

	void OGLES2RenderEngine::DoDispatch(RenderTechnique const & /*tech*/, uint32_t /*tgx*/, uint32_t /*tgy*/, uint32_t /*tgz*/)
	{
		BOOST_ASSERT(false);
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::EndFrame()
	{
	}

	// 结束一个Pass
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::EndPass()
	{
	}

	// 设置模板位数
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t OGLES2RenderEngine::StencilBufferBitDepth()
	{
		return 8;
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glScissor(x, y, width, height);
	}

	void OGLES2RenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_pointer_cast<OGLES2RenderWindow>(screen_frame_buffer_)->Resize(width, height);
	}

	bool OGLES2RenderEngine::FullScreen() const
	{
		return checked_pointer_cast<OGLES2RenderWindow>(screen_frame_buffer_)->FullScreen();
	}

	void OGLES2RenderEngine::FullScreen(bool fs)
	{
		checked_pointer_cast<OGLES2RenderWindow>(screen_frame_buffer_)->FullScreen(fs);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::FillRenderDeviceCaps()
	{
		GLint temp;

		if (glloader_GLES_VERSION_2_0())
		{
			glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &temp);
			caps_.max_vertex_texture_units = static_cast<uint8_t>(temp);
		}
		else
		{
			caps_.max_vertex_texture_units = 0;
		}

		if (glloader_GLES_VERSION_2_0())
		{
			caps_.max_shader_model = 2;
		}

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &temp);
		caps_.max_texture_height = caps_.max_texture_width = temp;
		if (glloader_GLES_OES_texture_3D())
		{
			glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_OES, &temp);
			caps_.max_texture_depth = temp;
		}
		else
		{
			caps_.max_texture_depth = 1;
		}

		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &temp);
		caps_.max_texture_cube_size = temp;

		caps_.max_texture_array_length = 1;

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &temp);
		caps_.max_pixel_texture_units = static_cast<uint8_t>(temp);

		caps_.max_geometry_texture_units = 0;

		if (glloader_GLES_EXT_texture_filter_anisotropic())
		{
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &temp);
			caps_.max_texture_anisotropy = static_cast<uint8_t>(temp);
		}
		else
		{
			caps_.max_texture_anisotropy = 1;
		}

		caps_.max_simultaneous_rts = 1;

		glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &temp);
		caps_.max_vertices = temp;
		glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &temp);
		caps_.max_indices = temp;

		caps_.hw_instancing_support = true;
		caps_.stream_output_support = false;
		caps_.alpha_to_coverage_support = true;
		caps_.depth_texture_support = true;
		caps_.primitive_restart_support = false;
		caps_.multithread_rendering_support = false;
		if (glloader_GLES_EXT_texture_format_BGRA8888())
		{
			caps_.argb8_support = true;
		}
		else
		{
			caps_.argb8_support = false;
		}
		if (glloader_GLES_EXT_texture_compression_dxt1())
		{
			caps_.bc1_support = true;
		}
		else
		{
			caps_.bc1_support = false;
		}
		caps_.bc2_support = false;
		caps_.bc3_support = false;
		caps_.bc4_support = false;
		caps_.bc5_support = false;
		caps_.bc6_support = false;
		caps_.bc7_support = false;

		caps_.gs_support = false;
		caps_.cs_support = false;
		caps_.hs_support = false;
		caps_.ds_support = false;
	}
}
