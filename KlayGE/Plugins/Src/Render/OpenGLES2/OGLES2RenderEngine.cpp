// OGLES2RenderEngine.cpp
// KlayGE OpenGL ES 2渲染引擎类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
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

	void OGLES2RenderEngine::CheckConfig()
	{
		if (!glloader_GLES_OES_texture_half_float() && !glloader_GLES_OES_texture_float())
		{
			render_settings_.hdr = false;
		}
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

		active_tex_unit_ = GL_TEXTURE0;
		glActiveTexture(active_tex_unit_);

		binded_buffer_.clear();
	}

	void OGLES2RenderEngine::ActiveTexture(GLenum tex_unit)
	{
		if (tex_unit != active_tex_unit_)
		{
			glActiveTexture(tex_unit);
			active_tex_unit_ = tex_unit;
		}
	}

	void OGLES2RenderEngine::BindBuffer(GLenum target, GLuint buffer)
	{
		BOOST_AUTO(iter, binded_buffer_.find(target));
		if ((iter == binded_buffer_.end()) || (iter->second != buffer))
		{
			glBindBuffer(target, buffer);
			binded_buffer_[target] = buffer;
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
		uint32_t const num_instance = rl.NumInstances();
		BOOST_ASSERT(num_instance != 0);

		checked_cast<OGLES2RenderLayout const *>(&rl)->Active(tech.Pass(0)->GetShaderObject());

		size_t const vertexCount = rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();
		GLenum mode;
		uint32_t primCount;
		OGLES2Mapping::Mapping(mode, primCount, rl);

		numPrimitivesJustRendered_ += num_instance * primCount;
		numVerticesJustRendered_ += num_instance * vertexCount;

		GLenum index_type = GL_UNSIGNED_SHORT;
		uint8_t* index_offset = NULL;
		if (rl.UseIndices())
		{
			OGLES2GraphicsBuffer& stream(*checked_pointer_cast<OGLES2GraphicsBuffer>(rl.GetIndexStream()));
			stream.Active();

			if (EF_R16UI == rl.IndexStreamFormat())
			{
				index_type = GL_UNSIGNED_SHORT;
				index_offset += rl.StartIndexLocation() * 2;
			}
			else
			{
				index_type = GL_UNSIGNED_INT;
				index_offset += rl.StartIndexLocation() * 4;
			}
		}

		uint32_t const num_passes = tech.NumPasses();

			for (uint32_t instance = rl.StartInstanceLocation(); instance < rl.StartInstanceLocation() + num_instance; ++ instance)
		{
			BOOST_ASSERT(!rl.InstanceStream());

			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					glDrawElements(mode, static_cast<GLsizei>(rl.NumIndices()),
						index_type, index_offset);
					pass->Unbind();
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					glDrawArrays(mode, rl.StartVertexLocation(), static_cast<GLsizei>(rl.NumVertices()));
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

	void OGLES2RenderEngine::ForceFlush()
	{
		glFlush();
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glScissor(x, y, width, height);
	}

	void OGLES2RenderEngine::GetCustomAttrib(std::string const & name, void* value)
	{
		if ("VENDOR" == name)
		{
			char const * str = reinterpret_cast<char const *>(glGetString(GL_VENDOR));
			if (str)
			{
				*static_cast<std::string*>(value) = str;
			}
			else
			{
				static_cast<std::string*>(value)->clear();
			}
		}
		if ("RENDERER" == name)
		{
			char const * str = reinterpret_cast<char const *>(glGetString(GL_RENDERER));
			if (str)
			{
				*static_cast<std::string*>(value) = str;
			}
			else
			{
				static_cast<std::string*>(value)->clear();
			}
		}
		if ("VERSION" == name)
		{
			char const * str = reinterpret_cast<char const *>(glGetString(GL_VERSION));
			if (str)
			{
				*static_cast<std::string*>(value) = str;
			}
			else
			{
				static_cast<std::string*>(value)->clear();
			}
		}
		if ("SHADING_LANGUAGE_VERSION" == name)
		{
			char const * str = reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
			if (str)
			{
				*static_cast<std::string*>(value) = str;
			}
			else
			{
				static_cast<std::string*>(value)->clear();
			}
		}
		if ("NUM_FEATURES" == name)
		{
			*static_cast<int*>(value) = glloader_num_features();
		}
		if (0 == name.find("FEATURE_NAME_"))
		{
			std::istringstream iss(name.substr(13));
			int n;
			iss >> n;
			*static_cast<std::string*>(value) = glloader_get_feature_name(n);
		}
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

	void OGLES2RenderEngine::AdjustPerspectiveMatrix(float4x4& pers_mat)
	{
		pers_mat *= MathLib::scaling(1.0f, 1.0f, 2.0f) * MathLib::translation(0.0f, 0.0f, -1.0f);
	}

	bool OGLES2RenderEngine::VertexFormatSupport(ElementFormat elem_fmt)
	{
		return vertex_format_.find(elem_fmt) != vertex_format_.end();
	}

	bool OGLES2RenderEngine::TextureFormatSupport(ElementFormat elem_fmt)
	{
		return texture_format_.find(elem_fmt) != texture_format_.end();
	}

	bool OGLES2RenderEngine::RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t /*sample_quality*/)
	{
		return (rendertarget_format_.find(elem_fmt) != rendertarget_format_.end()) && (sample_count <= 1);
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
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &temp);
		caps_.max_vertex_streams = temp;

		caps_.hw_instancing_support = true;
		caps_.instance_id_support = false;
		caps_.stream_output_support = false;
		caps_.alpha_to_coverage_support = true;
		caps_.primitive_restart_support = false;
		caps_.multithread_rendering_support = false;
		caps_.multithread_res_creating_support = false;
		caps_.mrt_independent_bit_depths_support = false;
		caps_.gs_support = false;
		caps_.cs_support = false;
		caps_.hs_support = false;
		caps_.ds_support = false;
		caps_.tess_method = TM_No;

		
		vertex_format_.insert(EF_A8);
		vertex_format_.insert(EF_R8);
		vertex_format_.insert(EF_GR8);
		vertex_format_.insert(EF_BGR8);
		vertex_format_.insert(EF_ABGR8);
		vertex_format_.insert(EF_R8UI);
		vertex_format_.insert(EF_GR8UI);
		vertex_format_.insert(EF_BGR8UI);
		vertex_format_.insert(EF_ABGR8UI);
		vertex_format_.insert(EF_SIGNED_R8);
		vertex_format_.insert(EF_SIGNED_GR8);
		vertex_format_.insert(EF_SIGNED_BGR8);
		vertex_format_.insert(EF_SIGNED_ABGR8);
		vertex_format_.insert(EF_R8I);
		vertex_format_.insert(EF_GR8I);
		vertex_format_.insert(EF_BGR8I);
		vertex_format_.insert(EF_ABGR8I);
		vertex_format_.insert(EF_A2BGR10);
		vertex_format_.insert(EF_R16);
		vertex_format_.insert(EF_GR16);
		vertex_format_.insert(EF_BGR16);
		vertex_format_.insert(EF_ABGR16);
		vertex_format_.insert(EF_R16UI);
		vertex_format_.insert(EF_GR16UI);
		vertex_format_.insert(EF_BGR16UI);
		vertex_format_.insert(EF_ABGR16UI);
		vertex_format_.insert(EF_SIGNED_R16);
		vertex_format_.insert(EF_SIGNED_GR16);
		vertex_format_.insert(EF_SIGNED_BGR16);
		vertex_format_.insert(EF_SIGNED_ABGR16);
		vertex_format_.insert(EF_R16I);
		vertex_format_.insert(EF_GR16I);
		vertex_format_.insert(EF_BGR16I);
		vertex_format_.insert(EF_ABGR16I);
		vertex_format_.insert(EF_R32UI);
		vertex_format_.insert(EF_GR32UI);
		vertex_format_.insert(EF_BGR32UI);
		vertex_format_.insert(EF_ABGR32UI);
		vertex_format_.insert(EF_R32I);
		vertex_format_.insert(EF_GR32I);
		vertex_format_.insert(EF_BGR32I);
		vertex_format_.insert(EF_ABGR32I);
		vertex_format_.insert(EF_R16F);
		vertex_format_.insert(EF_GR16F);
		vertex_format_.insert(EF_BGR16F);
		vertex_format_.insert(EF_ABGR16F);
		vertex_format_.insert(EF_R32F);
		vertex_format_.insert(EF_GR32F);
		vertex_format_.insert(EF_BGR32F);
		vertex_format_.insert(EF_ABGR32F);
		if (glloader_GLES_EXT_texture_format_BGRA8888())
		{
			vertex_format_.insert(EF_ARGB8);
		}

		texture_format_.insert(EF_A8);
		texture_format_.insert(EF_ARGB4);
		texture_format_.insert(EF_R8);
		texture_format_.insert(EF_SIGNED_R8);
		texture_format_.insert(EF_ABGR8);
		texture_format_.insert(EF_R16);
		texture_format_.insert(EF_SIGNED_R16);
		texture_format_.insert(EF_BGR16);
		texture_format_.insert(EF_SIGNED_BGR16);
		texture_format_.insert(EF_ABGR16);
		texture_format_.insert(EF_SIGNED_ABGR16);
		if (glloader_GLES_EXT_texture_format_BGRA8888())
		{
			texture_format_.insert(EF_ARGB8);
		}
		if (glloader_GLES_OES_texture_half_float())
		{
			texture_format_.insert(EF_R16F);
			texture_format_.insert(EF_GR16F);
			texture_format_.insert(EF_BGR16F);
			texture_format_.insert(EF_ABGR16F);
		}
		if (glloader_GLES_OES_texture_float())
		{
			texture_format_.insert(EF_R32F);
			texture_format_.insert(EF_GR32F);
			texture_format_.insert(EF_BGR32F);
			texture_format_.insert(EF_ABGR32F);
		}
		if (glloader_GLES_EXT_texture_compression_dxt1())
		{
			texture_format_.insert(EF_BC1);
		}
		texture_format_.insert(EF_D16);
		texture_format_.insert(EF_D24S8);
		texture_format_.insert(EF_D32F);

		rendertarget_format_.insert(EF_ARGB8);
		rendertarget_format_.insert(EF_ABGR8);
		rendertarget_format_.insert(EF_SIGNED_ABGR8);
		rendertarget_format_.insert(EF_A2BGR10);
		rendertarget_format_.insert(EF_SIGNED_A2BGR10);
		rendertarget_format_.insert(EF_ABGR16);
		rendertarget_format_.insert(EF_SIGNED_ABGR16);
		if (glloader_GLES_OES_texture_half_float())
		{
			rendertarget_format_.insert(EF_ABGR16F);
		}
		if (glloader_GLES_OES_texture_float())
		{
			rendertarget_format_.insert(EF_ABGR32F);
		}
		rendertarget_format_.insert(EF_D16);
		rendertarget_format_.insert(EF_D24S8);
		rendertarget_format_.insert(EF_D32F);

		caps_.vertex_format_support = boost::bind<bool>(&OGLES2RenderEngine::VertexFormatSupport, this, _1);
		caps_.texture_format_support = boost::bind<bool>(&OGLES2RenderEngine::TextureFormatSupport, this, _1);
		caps_.rendertarget_format_support = boost::bind<bool>(&OGLES2RenderEngine::RenderTargetFormatSupport, this, _1, _2, _3);
	}
}
