// OGLRenderEngine.cpp
// KlayGE OpenGL渲染引擎类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2004-2008
// Homepage: http://www.klayge.org
//
// 3.7.0
// 实验性的linux支持 (2008.5.19)
//
// 3.5.0
// 支持新的对象模型 (2006.11.19)
//
// 3.0.0
// 去掉了固定流水线 (2005.8.18)
//
// 2.8.0
// 增加了RenderDeviceCaps (2005.7.17)
// 简化了StencilBuffer相关操作 (2005.7.20)
// 只支持vbo (2005.7.31)
// 只支持OpenGL 1.5及以上 (2005.8.12)
//
// 2.7.0
// 支持vertex_buffer_object (2005.6.19)
// 支持OpenGL 1.3多纹理 (2005.6.26)
// 去掉了TextureCoordSet (2005.6.26)
// TextureAddressingMode, TextureFiltering和TextureAnisotropy移到Texture中 (2005.6.27)
//
// 2.4.0
// 增加了PolygonMode (2005.3.20)
//
// 2.0.1
// 初次建立 (2003.10.11)
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

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLRenderWindow.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderLayout.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#if defined(KLAYGE_CPU_X86)
	#ifdef KLAYGE_DEBUG
		#pragma comment(lib, "glloader_x86_d.lib")
	#else
		#pragma comment(lib, "glloader_x86.lib")
	#endif
#elif defined(KLAYGE_CPU_X64)
	#ifdef KLAYGE_DEBUG
		#pragma comment(lib, "glloader_x64_d.lib")
	#else
		#pragma comment(lib, "glloader_x64.lib")
	#endif
#endif
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLRenderEngine::OGLRenderEngine()
		: fbo_blit_src_(0), fbo_blit_dst_(0),
			clear_depth_(1), clear_stencil_(0),
			vp_x_(0), vp_y_(0), vp_width_(0), vp_height_(0),
			cur_fbo_(0), restart_index_(0)
	{
		clear_clr_.assign(0);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLRenderEngine::~OGLRenderEngine()
	{
		if (fbo_blit_src_ != 0)
		{
			glDeleteFramebuffersEXT(1, &fbo_blit_src_);
		}
		if (fbo_blit_dst_ != 0)
		{
			glDeleteFramebuffersEXT(1, &fbo_blit_dst_);
		}
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & OGLRenderEngine::Name() const
	{
		static const std::wstring name(L"OpenGL Render Engine");
		return name;
	}

	// 开始渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::StartRendering()
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
	void OGLRenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		motion_frames_ = settings.motion_frames;

		FrameBufferPtr win = MakeSharedPtr<OGLRenderWindow>(name, settings);

		this->FillRenderDeviceCaps();
		this->InitRenderStates();

		win->Attach(FrameBuffer::ATT_Color0,
			MakeSharedPtr<OGLScreenColorRenderView>(win->Width(), win->Height(), win->Format()));
		if (win->DepthBits() > 0)
		{
			win->Attach(FrameBuffer::ATT_DepthStencil,
				MakeSharedPtr<OGLScreenDepthStencilRenderView>(win->Width(), win->Height(), settings.depth_stencil_fmt));
		}

		this->BindFrameBuffer(win);

		glGenFramebuffersEXT(1, &fbo_blit_src_);
		glGenFramebuffersEXT(1, &fbo_blit_dst_);
	}

	void OGLRenderEngine::InitRenderStates()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRasterizerStateObject(RasterizerStateDesc());
		cur_dss_obj_ = rf.MakeDepthStencilStateObject(DepthStencilStateDesc());
		cur_bs_obj_ = rf.MakeBlendStateObject(BlendStateDesc());
		checked_pointer_cast<OGLRasterizerStateObject>(cur_rs_obj_)->ForceDefaultState();
		checked_pointer_cast<OGLDepthStencilStateObject>(cur_dss_obj_)->ForceDefaultState();
		checked_pointer_cast<OGLBlendStateObject>(cur_bs_obj_)->ForceDefaultState();

		glEnable(GL_POLYGON_OFFSET_FILL);
		glEnable(GL_POLYGON_OFFSET_POINT);
		glEnable(GL_POLYGON_OFFSET_LINE);
		if (glloader_GL_VERSION_3_1())
		{
			glEnable(GL_PRIMITIVE_RESTART);
		}

		mip_map_lod_bias_.resize(std::max(std::max(caps_.max_vertex_texture_units, caps_.max_pixel_texture_units), caps_.max_geometry_texture_units), 0);
		for (uint32_t stage = 0; stage < mip_map_lod_bias_.size(); ++ stage)
		{
			float bias = mip_map_lod_bias_[stage];
			GLenum tex_unit = GL_TEXTURE0 + stage;
			if (glloader_GL_EXT_direct_state_access())
			{
				glMultiTexEnvfEXT(tex_unit, GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
			}
			else
			{
				this->ActiveTexture(tex_unit);
				glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
			}
		}

		active_tex_unit_ = GL_TEXTURE0;
		glActiveTexture(active_tex_unit_);

		binded_buffer_.clear();
	}

	void OGLRenderEngine::MipMapLodBias(uint32_t stage, float bias)
	{
		if (mip_map_lod_bias_[stage] != bias)
		{
			GLenum tex_unit = GL_TEXTURE0 + stage;
			if (glloader_GL_EXT_direct_state_access())
			{
				glMultiTexEnvfEXT(tex_unit, GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
			}
			else
			{
				this->ActiveTexture(tex_unit);
				glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
			}

			mip_map_lod_bias_[stage] = bias;
		}
	}

	void OGLRenderEngine::ActiveTexture(GLenum tex_unit)
	{
		if (tex_unit != active_tex_unit_)
		{
			glActiveTexture(tex_unit);
			active_tex_unit_ = tex_unit;
		}
	}

	void OGLRenderEngine::BindBuffer(GLenum target, GLuint buffer)
	{
		BOOST_AUTO(iter, binded_buffer_.find(target));
		if ((iter == binded_buffer_.end()) || (iter->second != buffer))
		{
			glBindBuffer(target, buffer);
			binded_buffer_[target] = buffer;
		}
	}

	void OGLRenderEngine::ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
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

	void OGLRenderEngine::ClearDepth(GLfloat depth)
	{
		if (depth != clear_depth_)
		{
			glClearDepth(depth);
			clear_depth_ = depth;
		}
	}

	void OGLRenderEngine::ClearStencil(GLuint stencil)
	{
		if (stencil != clear_stencil_)
		{
			glClearStencil(stencil);
			clear_stencil_ = stencil;
		}
	}

	void OGLRenderEngine::BindFramebuffer(GLuint fbo, bool force)
	{
		if (force || (cur_fbo_ != fbo))
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
			cur_fbo_ = fbo;
		}
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
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
	void OGLRenderEngine::DoBindSOBuffers(RenderLayoutPtr const & rl)
	{
		so_rl_ = rl;

		if (so_rl_)
		{
			switch (rl->TopologyType())
			{
			case RenderLayout::TT_PointList:
				so_primitive_mode_ = GL_POINTS;
				break;

			case RenderLayout::TT_LineList:
				so_primitive_mode_ = GL_LINES;
				break;

			case RenderLayout::TT_TriangleList:
				so_primitive_mode_ = GL_TRIANGLES;
				break;

			default:
				BOOST_ASSERT(false);
				so_primitive_mode_ = GL_POINTS;
				break;
			}

			so_vars_.resize(0);
			for (uint32_t i = 0; i < so_rl_->NumVertexStreams(); ++ i)
			{
				so_buffs_.push_back(checked_pointer_cast<OGLGraphicsBuffer>(so_rl_->GetVertexStream(i))->OGLvbo());

				vertex_element const & ve = so_rl_->VertexStreamFormat(i)[0];
				switch (ve.usage)
				{
				case VEU_Position:
					so_vars_.push_back("gl_Position");
					break;

				case VEU_Normal:
					so_vars_.push_back("gl_Normal");
					break;

				case VEU_Diffuse:
					so_vars_.push_back("gl_FrontColor");
					break;

				case VEU_Specular:
					so_vars_.push_back("gl_FrontSecondaryColor");
					break;

				case VEU_BlendWeight:
					so_vars_.push_back("_BLENDWEIGHT");
					break;
					
				case VEU_BlendIndex:
					so_vars_.push_back("_BLENDINDEX");
					break;

				case VEU_TextureCoord:
					{
						std::stringstream ss;
						ss << "glTexCoord[" << ve.usage_index << "]";
						so_vars_.push_back(ss.str());
					}
					break;

				case VEU_Tangent:
					so_vars_.push_back("_TANGENT");
					break;
					
				case VEU_Binormal:
					so_vars_.push_back("_BINORMAL");
					break;
				}
			}

			so_vars_ptrs_.resize(so_vars_.size());
			for (size_t i = 0; i < so_rl_->NumVertexStreams(); ++ i)
			{
				so_vars_ptrs_[i] = so_vars_[i].c_str();
			}
		}
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::BeginFrame()
	{
	}

	// 开始一个Pass
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::BeginPass()
	{
	}
	
	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::DoRender(RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t const num_instance = rl.NumInstance();
		BOOST_ASSERT(num_instance != 0);

		OGLShaderObjectPtr cur_shader = checked_pointer_cast<OGLShaderObject>(tech.Pass(0)->GetShaderObject());
		checked_cast<OGLRenderLayout const *>(&rl)->Active(cur_shader);

		size_t const vertexCount = rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();
		GLenum mode;
		uint32_t primCount;
		OGLMapping::Mapping(mode, primCount, rl);

		numPrimitivesJustRendered_ += num_instance * primCount;
		numVerticesJustRendered_ += num_instance * vertexCount;

		GLenum index_type = GL_UNSIGNED_SHORT;
		if (rl.UseIndices())
		{
			if (EF_R16UI == rl.IndexStreamFormat())
			{
				index_type = GL_UNSIGNED_SHORT;

				if (restart_index_ != 0xFFFF)
				{
					if (glloader_GL_VERSION_3_1())
					{
						glPrimitiveRestartIndex(0xFFFF);
					}
					restart_index_ = 0xFFFF;
				}
			}
			else
			{
				index_type = GL_UNSIGNED_INT;

				if (restart_index_ != 0xFFFFFFFF)
				{
					if (glloader_GL_VERSION_3_1())
					{
						glPrimitiveRestartIndex(0xFFFFFFFF);
					}
					restart_index_ = 0xFFFFFFFF;
				}
			}
		}

		uint32_t const num_passes = tech.NumPasses();
		size_t const inst_format_size = rl.InstanceStreamFormat().size();

		if (glloader_GL_VERSION_3_3() && rl.InstanceStream())
		{
			OGLGraphicsBuffer& stream(*checked_pointer_cast<OGLGraphicsBuffer>(rl.InstanceStream()));

			uint32_t const instance_size = rl.InstanceSize();
			BOOST_ASSERT(num_instance * instance_size <= stream.Size());

			uint8_t* elem_offset = NULL;
			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				vertex_element const & vs_elem = rl.InstanceStreamFormat()[i];

				GLint attr = cur_shader->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum type;
					GLboolean normalized;
					OGLMapping::MappingVertexFormat(type, normalized, vs_elem.format);
					normalized = (((VEU_Diffuse == vs_elem.usage) || (VEU_Specular == vs_elem.usage)) && !IsFloatFormat(vs_elem.format)) ? GL_TRUE : normalized;
					GLvoid* offset = static_cast<GLvoid*>(elem_offset);

					stream.Active();
					glVertexAttribPointer(attr, num_components, type, normalized, instance_size, offset);
					glEnableVertexAttribArray(attr);

					glVertexAttribDivisor(attr, 1);
				}

				elem_offset += vs_elem.element_size();
			}

			if (so_rl_)
			{
				glBeginTransformFeedbackEXT(so_primitive_mode_);
			}

			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();

					if (so_rl_)
					{
						OGLShaderObjectPtr shader = checked_pointer_cast<OGLShaderObject>(pass->GetShaderObject());
						glTransformFeedbackVaryingsEXT(shader->GLSLProgram(), static_cast<GLsizei>(so_vars_ptrs_.size()), &so_vars_ptrs_[0], GL_SEPARATE_ATTRIBS_EXT);
						for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
						{
							glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, j, so_buffs_[j]);
						}
					}

					glDrawElementsInstanced(mode, static_cast<GLsizei>(rl.NumIndices()),
						index_type, 0, num_instance);
					pass->Unbind();
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();

					if (so_rl_)
					{
						OGLShaderObjectPtr shader = checked_pointer_cast<OGLShaderObject>(pass->GetShaderObject());
						glTransformFeedbackVaryingsEXT(shader->GLSLProgram(), static_cast<GLsizei>(so_vars_ptrs_.size()), &so_vars_ptrs_[0], GL_SEPARATE_ATTRIBS_EXT);
						for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
						{
							glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, j, so_buffs_[j]);
						}
					}

					glDrawArraysInstanced(mode, 0, static_cast<GLsizei>(rl.NumVertices()), num_instance);
					pass->Unbind();
				}
			}

			if (so_rl_)
			{
				glEndTransformFeedbackEXT();
			}

			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				vertex_element const & vs_elem = rl.InstanceStreamFormat()[i];
				GLint attr = cur_shader->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				glDisableVertexAttribArray(attr);
				glVertexAttribDivisor(attr, 0);
			}
		}
		else
		{
			for (uint32_t instance = 0; instance < num_instance; ++ instance)
			{
				if (rl.InstanceStream())
				{
					GraphicsBuffer& stream = *rl.InstanceStream();

					uint32_t const instance_size = rl.InstanceSize();
					BOOST_ASSERT(num_instance * instance_size <= stream.Size());
					GraphicsBuffer::Mapper mapper(stream, BA_Read_Only);
					uint8_t const * buffer = mapper.Pointer<uint8_t>();

					uint32_t elem_offset = 0;
					for (size_t i = 0; i < inst_format_size; ++ i)
					{
						BOOST_ASSERT(elem_offset < instance_size);

						vertex_element const & vs_elem = rl.InstanceStreamFormat()[i];

						GLint attr = cur_shader->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
						if (attr != -1)
						{
							void const * addr = &buffer[instance * instance_size + elem_offset];
							GLfloat const * float_addr = static_cast<GLfloat const *>(addr);
							GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
							GLenum type;
							GLboolean normalized;
							OGLMapping::MappingVertexFormat(type, normalized, vs_elem.format);
							normalized = (((VEU_Diffuse == vs_elem.usage) || (VEU_Specular == vs_elem.usage)) && !IsFloatFormat(vs_elem.format)) ? GL_TRUE : normalized;

							switch (num_components)
							{
							case 1:
								BOOST_ASSERT(IsFloatFormat(vs_elem.format));
								glVertexAttrib1fv(attr, float_addr);
								break;

							case 2:
								BOOST_ASSERT(IsFloatFormat(vs_elem.format));
								glVertexAttrib2fv(attr, float_addr);
								break;

							case 3:
								BOOST_ASSERT(IsFloatFormat(vs_elem.format));
								glVertexAttrib3fv(attr, float_addr);
								break;

							case 4:
								if (IsFloatFormat(vs_elem.format))
								{
									glVertexAttrib4fv(attr, float_addr);
								}
								else
								{
									if (normalized)
									{
										glVertexAttrib4Nubv(attr, static_cast<GLubyte const *>(addr));
									}
									else
									{
										glVertexAttrib4ubv(attr, static_cast<GLubyte const *>(addr));
									}
								}
								break;

							default:
								BOOST_ASSERT(false);
								break;
							}
						}

						elem_offset += vs_elem.element_size();
					}
				}

				if (so_rl_)
				{
					glBeginTransformFeedbackEXT(so_primitive_mode_);
				}

				if (rl.UseIndices())
				{
					for (uint32_t i = 0; i < num_passes; ++ i)
					{
						RenderPassPtr const & pass = tech.Pass(i);

						pass->Bind();

						if (so_rl_)
						{
							OGLShaderObjectPtr shader = checked_pointer_cast<OGLShaderObject>(pass->GetShaderObject());
							glTransformFeedbackVaryingsEXT(shader->GLSLProgram(), static_cast<GLsizei>(so_vars_ptrs_.size()), &so_vars_ptrs_[0], GL_SEPARATE_ATTRIBS_EXT);
							for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
							{
								glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, j, so_buffs_[j]);
							}
						}

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

						if (so_rl_)
						{
							OGLShaderObjectPtr shader = checked_pointer_cast<OGLShaderObject>(pass->GetShaderObject());
							glTransformFeedbackVaryingsEXT(shader->GLSLProgram(), static_cast<GLsizei>(so_vars_ptrs_.size()), &so_vars_ptrs_[0], GL_SEPARATE_ATTRIBS_EXT);
							for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
							{
								glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, j, so_buffs_[j]);
							}
						}

						glDrawArrays(mode, 0, static_cast<GLsizei>(rl.NumVertices()));
						pass->Unbind();
					}
				}

				if (so_rl_)
				{
					glEndTransformFeedbackEXT();
				}
			}
		}

		checked_cast<OGLRenderLayout const *>(&rl)->Deactive(cur_shader);
	}

	void OGLRenderEngine::DoDispatch(RenderTechnique const & /*tech*/, uint32_t /*tgx*/, uint32_t /*tgy*/, uint32_t /*tgz*/)
	{
		BOOST_ASSERT(false);
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::EndFrame()
	{
	}

	// 结束一个Pass
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::EndPass()
	{
	}

	void OGLRenderEngine::ForceFlush()
	{
		glFlush();
	}

	// 设置模板位数
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t OGLRenderEngine::StencilBufferBitDepth()
	{
		return 8;
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glScissor(x, y, width, height);
	}

	void OGLRenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_pointer_cast<OGLRenderWindow>(screen_frame_buffer_)->Resize(width, height);
	}

	bool OGLRenderEngine::FullScreen() const
	{
		return checked_pointer_cast<OGLRenderWindow>(screen_frame_buffer_)->FullScreen();
	}

	void OGLRenderEngine::FullScreen(bool fs)
	{
		checked_pointer_cast<OGLRenderWindow>(screen_frame_buffer_)->FullScreen(fs);
	}

	void OGLRenderEngine::AdjustPerspectiveMatrix(float4x4& pers_mat)
	{
		pers_mat *= MathLib::scaling(1.0f, 1.0f, 2.0f) * MathLib::translation(0.0f, 0.0f, -1.0f);
	}

	bool OGLRenderEngine::VertexFormatSupport(ElementFormat elem_fmt)
	{
		return vertex_format_.find(elem_fmt) != vertex_format_.end();
	}

	bool OGLRenderEngine::TextureFormatSupport(ElementFormat elem_fmt)
	{
		return texture_format_.find(elem_fmt) != texture_format_.end();
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderEngine::FillRenderDeviceCaps()
	{
		GLint temp;

		if (glloader_GL_VERSION_2_0() || glloader_GL_ARB_vertex_shader())
		{
			glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &temp);
			caps_.max_vertex_texture_units = static_cast<uint8_t>(temp);
		}
		else
		{
			caps_.max_vertex_texture_units = 0;
		}

		if (glloader_GL_VERSION_4_0() || glloader_GL_ARB_gpu_shader5())
		{
			//caps_.max_shader_model = 5;
			caps_.max_shader_model = 4;
		}
		else
		{
			if (glloader_GL_VERSION_2_0()
				|| (glloader_GL_ARB_vertex_shader() && glloader_GL_ARB_fragment_shader()))
			{
				if (caps_.max_vertex_texture_units != 0)
				{
					if (glloader_GL_EXT_gpu_shader4())
					{
						caps_.max_shader_model = 4;
					}
					else
					{
						caps_.max_shader_model = 3;
					}
				}
				else
				{
					caps_.max_shader_model = 2;
				}
			}
			else
			{
				if (glloader_GL_ARB_vertex_program() && glloader_GL_ARB_fragment_program())
				{
					caps_.max_shader_model = 1;
				}
				else
				{
					caps_.max_shader_model = 0;
				}
			}
		}

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &temp);
		caps_.max_texture_height = caps_.max_texture_width = temp;
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &temp);
		caps_.max_texture_depth = temp;

		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &temp);
		caps_.max_texture_cube_size = temp;

		/*if (glloader_GL_VERSION_3_0() || glloader_GL_EXT_texture_array())
		{
			glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, &temp);
			caps_.max_texture_array_length = temp;
		}
		else
		{
			caps_.max_texture_array_length = 1;
		}*/
		caps_.max_texture_array_length = 1;

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &temp);
		caps_.max_pixel_texture_units = static_cast<uint8_t>(temp);

		if (glloader_GL_ARB_geometry_shader4() || glloader_GL_EXT_geometry_shader4())
		{
			glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB, &temp);
			caps_.max_geometry_texture_units = static_cast<uint8_t>(temp);
		}
		else
		{
			caps_.max_geometry_texture_units = 0;
		}

		if (glloader_GL_EXT_texture_filter_anisotropic())
		{
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &temp);
			caps_.max_texture_anisotropy = static_cast<uint8_t>(temp);
		}
		else
		{
			caps_.max_texture_anisotropy = 1;
		}

		if (glloader_GL_VERSION_2_0() || glloader_GL_ARB_draw_buffers())
		{
			glGetIntegerv(GL_MAX_DRAW_BUFFERS, &temp);
			caps_.max_simultaneous_rts = static_cast<uint8_t>(temp);
		}
		else
		{
			caps_.max_simultaneous_rts = 1;
		}

		glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &temp);
		caps_.max_vertices = temp;
		glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &temp);
		caps_.max_indices = temp;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &temp);
		caps_.max_vertex_streams = temp;

		caps_.hw_instancing_support = true;
		caps_.stream_output_support = false;
		caps_.alpha_to_coverage_support = true;
		if (glloader_GL_VERSION_3_1() || glloader_GL_NV_primitive_restart())
		{
			caps_.primitive_restart_support = true;
		}
		else
		{
			caps_.primitive_restart_support = false;
		}
		caps_.multithread_rendering_support = false;
		caps_.multithread_res_creating_support = false;

		if (glloader_GL_ARB_geometry_shader4() || glloader_GL_EXT_geometry_shader4()
			|| glloader_GL_NV_geometry_shader4())
		{
			caps_.gs_support = true;
		}
		else
		{
			caps_.gs_support = false;
		}
			
		caps_.cs_support = false;
		if (glloader_GL_VERSION_4_0() || glloader_GL_ARB_tessellation_shader())
		{
			//caps_.hs_support = true;
			//caps_.ds_support = true;
			// Cg compiler don't support Cg->GLSL hull/domain shader
			caps_.hs_support = false;
			caps_.ds_support = false;
		}
		else
		{
			caps_.hs_support = false;
			caps_.ds_support = false;
		}

		std::string vendor(reinterpret_cast<char const *>(glGetString(GL_VENDOR)));
		if ((vendor.find("ATI", 0) != std::string::npos) || (vendor.find("AMD", 0) != std::string::npos))
		{
			hack_for_ati_ = true;
		}
		else
		{
			hack_for_ati_ = false;
		}

		ElementFormat const all_elem_fmts[] = 
		{
			EF_Unknown,
			EF_A8,

			EF_ARGB4,

			EF_R8,
			EF_SIGNED_R8,
			EF_GR8,
			EF_SIGNED_GR8,
			EF_BGR8,
			EF_SIGNED_BGR8,
			EF_ARGB8,
			EF_ABGR8,
			EF_SIGNED_ABGR8,
			EF_A2BGR10,
			EF_SIGNED_A2BGR10,

			EF_R8UI,
			EF_R8I,
			EF_GR8UI,
			EF_GR8I,
			EF_BGR8UI,
			EF_BGR8I,
			EF_ABGR8UI,
			EF_ABGR8I,
			EF_A2BGR10UI,
			EF_A2BGR10I,

			EF_R16,
			EF_SIGNED_R16,
			EF_GR16,
			EF_SIGNED_GR16,
			EF_BGR16,
			EF_SIGNED_BGR16,
			EF_ABGR16,
			EF_SIGNED_ABGR16,
			EF_R32,
			EF_SIGNED_R32,
			EF_GR32,
			EF_SIGNED_GR32,
			EF_BGR32,
			EF_SIGNED_BGR32,
			EF_ABGR32,
			EF_SIGNED_ABGR32,

			EF_R16UI,
			EF_R16I,
			EF_GR16UI,
			EF_GR16I,
			EF_BGR16UI,
			EF_BGR16I,
			EF_ABGR16UI,
			EF_ABGR16I,
			EF_R32UI,
			EF_R32I,
			EF_GR32UI,
			EF_GR32I,
			EF_BGR32UI,
			EF_BGR32I,
			EF_ABGR32UI,
			EF_ABGR32I,

			EF_R16F,
			EF_GR16F,
			EF_B10G11R11F,
			EF_BGR16F,
			EF_ABGR16F,
			EF_R32F,
			EF_GR32F,
			EF_BGR32F,
			EF_ABGR32F,

			EF_BC1,
			EF_SIGNED_BC1,
			EF_BC2,
			EF_SIGNED_BC2,
			EF_BC3,
			EF_SIGNED_BC3,
			EF_BC4,
			EF_SIGNED_BC4,
			EF_BC5,
			EF_SIGNED_BC5,
			EF_BC6,
			EF_SIGNED_BC6,
			EF_BC7,

			EF_D16,
			EF_D24S8,
			EF_D32F,

			EF_ARGB8_SRGB,
			EF_ABGR8_SRGB,
			EF_BC1_SRGB,
			EF_BC2_SRGB,
			EF_BC3_SRGB,
			EF_BC4_SRGB,
			EF_BC5_SRGB,
			EF_BC7_SRGB
		};
		for (int i = 0; i < sizeof(all_elem_fmts) / sizeof(all_elem_fmts[0]); ++ i)
		{
			ElementFormat const elem_fmt = all_elem_fmts[i];
			switch (elem_fmt)
			{
			case EF_A8:
			case EF_R8:
			case EF_GR8:
			case EF_BGR8:
			case EF_ARGB8:
			case EF_ABGR8:
			case EF_R8UI:
			case EF_GR8UI:
			case EF_BGR8UI:
			case EF_ABGR8UI:
			case EF_SIGNED_R8:
			case EF_SIGNED_GR8:
			case EF_SIGNED_BGR8:
			case EF_SIGNED_ABGR8:
			case EF_R8I:
			case EF_GR8I:
			case EF_BGR8I:
			case EF_ABGR8I:
			case EF_A2BGR10:
			case EF_R16:
			case EF_GR16:
			case EF_BGR16:
			case EF_ABGR16:
			case EF_R16UI:
			case EF_GR16UI:
			case EF_BGR16UI:
			case EF_ABGR16UI:
			case EF_SIGNED_R16:
			case EF_SIGNED_GR16:
			case EF_SIGNED_BGR16:
			case EF_SIGNED_ABGR16:
			case EF_R16I:
			case EF_GR16I:
			case EF_BGR16I:
			case EF_ABGR16I:
			case EF_R32UI:
			case EF_GR32UI:
			case EF_BGR32UI:
			case EF_ABGR32UI:
			case EF_R32I:
			case EF_GR32I:
			case EF_BGR32I:
			case EF_ABGR32I:
			case EF_R32F:
			case EF_GR32F:
			case EF_BGR32F:
			case EF_ABGR32F:
				vertex_format_.insert(elem_fmt);
				break;

			case EF_SIGNED_A2BGR10:
				if (glloader_GL_VERSION_3_3() || glloader_GL_ARB_vertex_type_2_10_10_10_rev())
				{
					vertex_format_.insert(elem_fmt);
				}
				break;

			case EF_R16F:
			case EF_GR16F:
			case EF_BGR16F:
			case EF_ABGR16F:
				if (glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())
				{
					vertex_format_.insert(elem_fmt);
				}
				break;

			case EF_B10G11R11F:
				if (glloader_GL_VERSION_3_0() || glloader_GL_EXT_packed_float())
				{
					vertex_format_.insert(elem_fmt);
				}
				break;

			default:
				break;
			}

			switch (elem_fmt)
			{
			case EF_A8:
			case EF_ARGB4:
			case EF_R8:
			case EF_SIGNED_R8:
				texture_format_.insert(elem_fmt);
				break;

			case EF_GR8:
			case EF_SIGNED_GR8:
				if (glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_BGR8:
				texture_format_.insert(elem_fmt);
				break;

			case EF_SIGNED_BGR8:
				if (glloader_GL_NV_texture_shader())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_ARGB8:
			case EF_ABGR8:
				texture_format_.insert(elem_fmt);
				break;

			case EF_SIGNED_ABGR8:
				if (glloader_GL_NV_texture_shader())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_A2BGR10:
			case EF_SIGNED_A2BGR10:
				texture_format_.insert(elem_fmt);
				break;

			case EF_R8UI:
			case EF_R8I:
			case EF_GR8UI:
			case EF_GR8I:
			case EF_BGR8UI:
			case EF_BGR8I:
			case EF_ABGR8UI:
			case EF_ABGR8I:
				if (glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_R16:
			case EF_SIGNED_R16:
				texture_format_.insert(elem_fmt);
				break;

			case EF_GR16:
			case EF_SIGNED_GR16:
				if (glloader_GL_VERSION_3_0() || glloader_GL_ARB_texture_rg())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_BGR16:
			case EF_SIGNED_BGR16:
			case EF_ABGR16:
			case EF_SIGNED_ABGR16:
				texture_format_.insert(elem_fmt);
				break;

			case EF_R16UI:
			case EF_R16I:
			case EF_GR16UI:
			case EF_GR16I:
			case EF_BGR16UI:
			case EF_BGR16I:
			case EF_ABGR16UI:
			case EF_ABGR16I:
			case EF_R32UI:
			case EF_R32I:
			case EF_GR32UI:
			case EF_GR32I:
			case EF_BGR32UI:
			case EF_BGR32I:
			case EF_ABGR32UI:
			case EF_ABGR32I:
				if (glloader_GL_VERSION_3_0() || glloader_GL_EXT_texture_integer())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_R16F:
			case EF_GR16F:
				texture_format_.insert(elem_fmt);
				break;

			case EF_B10G11R11F:
				if (glloader_GL_VERSION_3_0() || glloader_GL_EXT_packed_float())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_BGR16F:
			case EF_ABGR16F:
			case EF_R32F:
			case EF_GR32F:
			case EF_BGR32F:
			case EF_ABGR32F:
				texture_format_.insert(elem_fmt);
				break;

			case EF_BC1:
				if (glloader_GL_EXT_texture_compression_dxt1())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_BC2:
			case EF_BC3:
				if (glloader_GL_EXT_texture_compression_s3tc())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_BC4:
			case EF_BC5:
			case EF_SIGNED_BC4:
			case EF_SIGNED_BC5:
				if (glloader_GL_EXT_texture_compression_latc())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_BC6:
			case EF_BC7:
				if (glloader_GL_ARB_texture_compression_bptc())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_D16:
				texture_format_.insert(elem_fmt);
				break;

			case EF_D24S8:
				if (glloader_GL_EXT_packed_depth_stencil())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			case EF_D32F:
				texture_format_.insert(elem_fmt);
				break;

			case EF_ARGB8_SRGB:
			case EF_BC1_SRGB:
			case EF_BC2_SRGB:
			case EF_BC3_SRGB:
			case EF_BC4_SRGB:
			case EF_BC5_SRGB:
				if (glloader_GL_EXT_texture_sRGB())
				{
					texture_format_.insert(elem_fmt);
				}
				break;

			default:
				break;
			}
		}

		caps_.vertex_format_support = boost::bind<bool>(&OGLRenderEngine::VertexFormatSupport, this, _1);
		caps_.texture_format_support = boost::bind<bool>(&OGLRenderEngine::TextureFormatSupport, this, _1);
	}
}
