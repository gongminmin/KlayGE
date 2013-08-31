// OGLESRenderEngine.cpp
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
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <glloader/glloader.h>
#ifdef Bool
#undef Bool		// for boost::foreach
#endif

#include <algorithm>
#include <sstream>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESRenderWindow.hpp>
#include <KlayGE/OpenGLES/OGLESFrameBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderView.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderLayout.hpp>
#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESRenderStateObject.hpp>
#include <KlayGE/OpenGLES/OGLESShaderObject.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#if defined(KLAYGE_CPU_X86)
	#ifdef KLAYGE_DEBUG
		#pragma comment(lib, "glloader_es_vc_x86_d.lib")
	#else
		#pragma comment(lib, "glloader_es_vc_x86.lib")
	#endif
#elif defined(KLAYGE_CPU_X64)
	#ifdef KLAYGE_DEBUG
		#pragma comment(lib, "glloader_es_vc_x64_d.lib")
	#else
		#pragma comment(lib, "glloader_es_vc_x64.lib")
	#endif
#endif
#endif

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLESRenderEngine::OGLESRenderEngine()
		: fbo_blit_src_(0), fbo_blit_dst_(0),
			clear_depth_(1), clear_stencil_(0), cur_program_(0),
			vp_x_(0), vp_y_(0), vp_width_(0), vp_height_(0),
			cur_fbo_(0)
	{
		clear_clr_.fill(0);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLESRenderEngine::~OGLESRenderEngine()
	{
		if (fbo_blit_src_ != 0)
		{
			glDeleteFramebuffers(1, &fbo_blit_src_);
		}
		if (fbo_blit_dst_ != 0)
		{
			glDeleteFramebuffers(1, &fbo_blit_dst_);
		}
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & OGLESRenderEngine::Name() const
	{
		static const std::wstring name(L"OpenGL ES Render Engine");
		return name;
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		motion_frames_ = settings.motion_frames;

		FrameBufferPtr win = MakeSharedPtr<OGLESRenderWindow>(name, settings);

		this->FillRenderDeviceCaps();
		this->InitRenderStates();

		win->Attach(FrameBuffer::ATT_Color0,
			MakeSharedPtr<OGLESScreenColorRenderView>(win->Width(), win->Height(), settings.color_fmt));
		if (NumDepthBits(settings.depth_stencil_fmt) > 0)
		{
			win->Attach(FrameBuffer::ATT_DepthStencil,
				MakeSharedPtr<OGLESScreenDepthStencilRenderView>(win->Width(), win->Height(), settings.depth_stencil_fmt));
		}

		this->BindFrameBuffer(win);

		glGenFramebuffers(1, &fbo_blit_src_);
		glGenFramebuffers(1, &fbo_blit_dst_);
	}

	void OGLESRenderEngine::CheckConfig(RenderSettings& settings)
	{
#if defined KLAYGE_PLATFORM_ANDROID
		settings.hdr = false;
		settings.ppaa = false;
		settings.gamma = false;
		settings.color_grading = false;
#else
		UNREF_PARAM(settings);
#endif
	}

	void OGLESRenderEngine::InitRenderStates()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRasterizerStateObject(RasterizerStateDesc());
		cur_dss_obj_ = rf.MakeDepthStencilStateObject(DepthStencilStateDesc());
		cur_bs_obj_ = rf.MakeBlendStateObject(BlendStateDesc());
		checked_pointer_cast<OGLESRasterizerStateObject>(cur_rs_obj_)->ForceDefaultState();
		checked_pointer_cast<OGLESDepthStencilStateObject>(cur_dss_obj_)->ForceDefaultState();
		checked_pointer_cast<OGLESBlendStateObject>(cur_bs_obj_)->ForceDefaultState();

		glEnable(GL_POLYGON_OFFSET_FILL);
		if (glloader_GLES_VERSION_3_0())
		{
			glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
		}

		active_tex_unit_ = GL_TEXTURE0;
		glActiveTexture(active_tex_unit_);

		binded_buffer_.clear();
	}

	void OGLESRenderEngine::ActiveTexture(GLenum tex_unit)
	{
		if (tex_unit != active_tex_unit_)
		{
			glActiveTexture(tex_unit);
			active_tex_unit_ = tex_unit;
		}
	}

	void OGLESRenderEngine::BindBuffer(GLenum target, GLuint buffer, bool force)
	{
		KLAYGE_AUTO(iter, binded_buffer_.find(target));
		if (force || (iter == binded_buffer_.end()) || (iter->second != buffer))
		{
			glBindBuffer(target, buffer);
			binded_buffer_[target] = buffer;
		}
	}

	void OGLESRenderEngine::DeleteBuffers(GLsizei n, GLuint const * buffers)
	{
		for (GLsizei i = 0; i < n; ++ i)
		{
			for (KLAYGE_AUTO(iter, binded_buffer_.begin()); iter != binded_buffer_.end();)
			{
				if (iter->second == buffers[i])
				{
					binded_buffer_.erase(iter ++);
				}
				else
				{
					++ iter;
				}
			}
		}
		glDeleteBuffers(n, buffers);
	}

	void OGLESRenderEngine::ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
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

	void OGLESRenderEngine::ClearDepth(GLfloat depth)
	{
		if (depth != clear_depth_)
		{
			glClearDepthf(depth);
			clear_depth_ = depth;
		}
	}

	void OGLESRenderEngine::ClearStencil(GLuint stencil)
	{
		if (stencil != clear_stencil_)
		{
			glClearStencil(stencil);
			clear_stencil_ = stencil;
		}
	}

	void OGLESRenderEngine::UseProgram(GLuint program)
	{
		if (program != cur_program_)
		{
			glUseProgram(program);
			cur_program_ = program;
		}
	}

	void OGLESRenderEngine::Uniform1i(GLint location, GLint value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformi_cache_.find(cur_program_));
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.insert(std::make_pair(cur_program_, std::map<GLint, int4>())).first;
		}
		KLAYGE_AUTO(iter_v, iter_p->second.find(location));
		if (iter_v == iter_p->second.end())
		{
			dirty = true;
			iter_p->second.insert(std::make_pair(location, int4(value, 0, 0, 0)));
		}
		else
		{
			if (iter_v->second.x() != value)
			{
				dirty = true;
				iter_v->second.x() = value;
			}
		}

		if (dirty)
		{
			glUniform1i(location, value);
		}
	}
		
	void OGLESRenderEngine::Uniform1ui(GLint location, GLuint value)
	{
		this->Uniform1i(location, value);
	}

	void OGLESRenderEngine::Uniform1f(GLint location, float value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformf_cache_.find(cur_program_));
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.insert(std::make_pair(cur_program_, std::map<GLint, float4>())).first;
		}
		KLAYGE_AUTO(iter_v, iter_p->second.find(location));
		if (iter_v == iter_p->second.end())
		{
			dirty = true;
			iter_p->second.insert(std::make_pair(location, float4(value, 0, 0, 0)));
		}
		else
		{
			if (iter_v->second.x() != value)
			{
				dirty = true;
				iter_v->second.x() = value;
			}
		}

		if (dirty)
		{
			glUniform1f(location, value);
		}
	}

	void OGLESRenderEngine::Uniform1iv(GLint location, GLsizei count, GLint const * value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformi_cache_.find(cur_program_));
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.insert(std::make_pair(cur_program_, std::map<GLint, int4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			KLAYGE_AUTO(iter_v, iter_p->second.find(location + i));
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.insert(std::make_pair(location, int4(value[i], 0, 0, 0)));
			}
			else
			{
				if (iter_v->second.x() != value[i])
				{
					dirty = true;
					iter_v->second.x() = value[i];
				}
			}			
		}

		if (dirty)
		{
			glUniform1iv(location, count, value);
		}
	}

	void OGLESRenderEngine::Uniform1uiv(GLint location, GLsizei count, GLuint const * value)
	{
		this->Uniform1iv(location, count, reinterpret_cast<GLint const *>(value));
	}

	void OGLESRenderEngine::Uniform1fv(GLint location, GLsizei count, GLfloat const * value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformf_cache_.find(cur_program_));
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.insert(std::make_pair(cur_program_, std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			KLAYGE_AUTO(iter_v, iter_p->second.find(location + i));
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.insert(std::make_pair(location, float4(value[i], 0, 0, 0)));
			}
			else
			{
				if (iter_v->second.x() != value[i])
				{
					dirty = true;
					iter_v->second.x() = value[i];
				}
			}			
		}

		if (dirty)
		{
			glUniform1fv(location, count, value);
		}
	}

	void OGLESRenderEngine::Uniform2iv(GLint location, GLsizei count, GLint const * value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformi_cache_.find(cur_program_));
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.insert(std::make_pair(cur_program_, std::map<GLint, int4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			KLAYGE_AUTO(iter_v, iter_p->second.find(location + i));
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.insert(std::make_pair(location, int4(value[i * 2 + 0], value[i * 2 + 1], 0, 0)));
			}
			else
			{
				if ((iter_v->second.x() != value[i * 2 + 0]) || (iter_v->second.y() != value[i * 2 + 1]))
				{
					dirty = true;
					iter_v->second.x() = value[i * 2 + 0];
					iter_v->second.y() = value[i * 2 + 1];
				}
			}			
		}

		if (dirty)
		{
			glUniform2iv(location, count, value);
		}
	}

	void OGLESRenderEngine::Uniform2uiv(GLint location, GLsizei count, GLuint const * value)
	{
		this->Uniform2iv(location, count, reinterpret_cast<GLint const *>(value));
	}

	void OGLESRenderEngine::Uniform2fv(GLint location, GLsizei count, GLfloat const * value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformf_cache_.find(cur_program_));
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.insert(std::make_pair(cur_program_, std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			KLAYGE_AUTO(iter_v, iter_p->second.find(location + i));
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.insert(std::make_pair(location, float4(value[i * 2 + 0], value[i * 2 + 1], 0, 0)));
			}
			else
			{
				if ((iter_v->second.x() != value[i * 2 + 0]) || (iter_v->second.y() != value[i * 2 + 1]))
				{
					dirty = true;
					iter_v->second.x() = value[i * 2 + 0];
					iter_v->second.y() = value[i * 2 + 1];
				}
			}			
		}

		if (dirty)
		{
			glUniform2fv(location, count, value);
		}
	}

	void OGLESRenderEngine::Uniform3iv(GLint location, GLsizei count, GLint const * value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformi_cache_.find(cur_program_));
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.insert(std::make_pair(cur_program_, std::map<GLint, int4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			KLAYGE_AUTO(iter_v, iter_p->second.find(location + i));
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.insert(std::make_pair(location, int4(value[i * 3 + 0], value[i * 3 + 1], value[i * 3 + 2], 0)));
			}
			else
			{
				if ((iter_v->second.x() != value[i * 3 + 0]) || (iter_v->second.y() != value[i * 3 + 1])
					|| (iter_v->second.z() != value[i * 3 + 2]))
				{
					dirty = true;
					iter_v->second.x() = value[i * 3 + 0];
					iter_v->second.y() = value[i * 3 + 1];
					iter_v->second.z() = value[i * 3 + 2];
				}
			}			
		}

		if (dirty)
		{
			glUniform3iv(location, count, value);
		}
	}

	void OGLESRenderEngine::Uniform3uiv(GLint location, GLsizei count, GLuint const * value)
	{
		this->Uniform3iv(location, count, reinterpret_cast<GLint const *>(value));
	}

	void OGLESRenderEngine::Uniform3fv(GLint location, GLsizei count, GLfloat const * value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformf_cache_.find(cur_program_));
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.insert(std::make_pair(cur_program_, std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			KLAYGE_AUTO(iter_v, iter_p->second.find(location + i));
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.insert(std::make_pair(location, float4(value[i * 3 + 0], value[i * 3 + 1], value[i * 3 + 2], 0)));
			}
			else
			{
				if ((iter_v->second.x() != value[i * 3 + 0]) || (iter_v->second.y() != value[i * 3 + 1])
					|| (iter_v->second.z() != value[i * 3 + 2]))
				{
					dirty = true;
					iter_v->second.x() = value[i * 3 + 0];
					iter_v->second.y() = value[i * 3 + 1];
					iter_v->second.z() = value[i * 3 + 2];
				}
			}			
		}

		if (dirty)
		{
			glUniform3fv(location, count, value);
		}
	}

	void OGLESRenderEngine::Uniform4iv(GLint location, GLsizei count, GLint const * value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformi_cache_.find(cur_program_));
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.insert(std::make_pair(cur_program_, std::map<GLint, int4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			KLAYGE_AUTO(iter_v, iter_p->second.find(location + i));
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.insert(std::make_pair(location,
					int4(value[i * 4 + 0], value[i * 4 + 1], value[i * 4 + 2], value[i * 4 + 3])));
			}
			else
			{
				if ((iter_v->second.x() != value[i * 4 + 0]) || (iter_v->second.y() != value[i * 4 + 1])
					|| (iter_v->second.z() != value[i * 4 + 2]) || (iter_v->second.z() != value[i * 4 + 3]))
				{
					dirty = true;
					iter_v->second.x() = value[i * 4 + 0];
					iter_v->second.y() = value[i * 4 + 1];
					iter_v->second.z() = value[i * 4 + 2];
					iter_v->second.w() = value[i * 4 + 3];
				}
			}			
		}

		if (dirty)
		{
			glUniform4iv(location, count, value);
		}
	}

	void OGLESRenderEngine::Uniform4uiv(GLint location, GLsizei count, GLuint const * value)
	{
		this->Uniform4iv(location, count, reinterpret_cast<GLint const *>(value));
	}

	void OGLESRenderEngine::Uniform4fv(GLint location, GLsizei count, GLfloat const * value)
	{
		bool dirty = false;
		KLAYGE_AUTO(iter_p, uniformf_cache_.find(cur_program_));
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.insert(std::make_pair(cur_program_, std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			KLAYGE_AUTO(iter_v, iter_p->second.find(location + i));
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.insert(std::make_pair(location,
					float4(value[i * 4 + 0], value[i * 4 + 1], value[i * 4 + 2], value[i * 4 + 3])));
			}
			else
			{
				if ((iter_v->second.x() != value[i * 4 + 0]) || (iter_v->second.y() != value[i * 4 + 1])
					|| (iter_v->second.z() != value[i * 4 + 2]) || (iter_v->second.z() != value[i * 4 + 3]))
				{
					dirty = true;
					iter_v->second.x() = value[i * 4 + 0];
					iter_v->second.y() = value[i * 4 + 1];
					iter_v->second.z() = value[i * 4 + 2];
					iter_v->second.w() = value[i * 4 + 3];
				}
			}			
		}

		if (dirty)
		{
			glUniform4fv(location, count, value);
		}
	}

	void OGLESRenderEngine::BindFramebuffer(GLuint fbo, bool force)
	{
		if (force || (cur_fbo_ != fbo))
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			cur_fbo_ = fbo;
		}
	}

	void OGLESRenderEngine::DeleteFramebuffers(GLsizei n, GLuint const * framebuffers)
	{
		for (GLsizei i = 0; i < n; ++ i)
		{
			if (cur_fbo_ == framebuffers[i])
			{
				cur_fbo_ = 0;
			}
		}
		glDeleteFramebuffers(n, framebuffers);
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
	{
		BOOST_ASSERT(fb);

		Viewport const & vp = *fb->GetViewport();
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
	void OGLESRenderEngine::DoBindSOBuffers(RenderLayoutPtr const & rl)
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
				so_buffs_.push_back(checked_pointer_cast<OGLESGraphicsBuffer>(so_rl_->GetVertexStream(i))->GLvbo());

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
	
	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::DoRender(RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t const num_instance = rl.NumInstances();
		BOOST_ASSERT(num_instance != 0);

		OGLESShaderObjectPtr cur_shader = checked_pointer_cast<OGLESShaderObject>(tech.Pass(0)->GetShaderObject());
		checked_cast<OGLESRenderLayout const *>(&rl)->Active(cur_shader);

		size_t const vertexCount = rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();
		GLenum mode;
		uint32_t primCount;
		OGLESMapping::Mapping(mode, primCount, rl);

		numPrimitivesJustRendered_ += num_instance * primCount;
		numVerticesJustRendered_ += num_instance * vertexCount;

		GLenum index_type = GL_UNSIGNED_SHORT;
		uint8_t* index_offset = nullptr;
		if (rl.UseIndices())
		{
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
		size_t const inst_format_size = rl.InstanceStreamFormat().size();

		if (glloader_GLES_VERSION_3_0() && rl.InstanceStream())
		{
			OGLESGraphicsBuffer& stream(*checked_pointer_cast<OGLESGraphicsBuffer>(rl.InstanceStream()));

			uint32_t const instance_size = rl.InstanceSize();
			BOOST_ASSERT(num_instance * instance_size <= stream.Size());

			uint8_t* elem_offset = nullptr;
			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				vertex_element const & vs_elem = rl.InstanceStreamFormat()[i];

				GLint attr = cur_shader->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					GLint const num_components = static_cast<GLint>(NumComponents(vs_elem.format));
					GLenum type;
					GLboolean normalized;
					OGLESMapping::MappingVertexFormat(type, normalized, vs_elem.format);
					normalized = (((VEU_Diffuse == vs_elem.usage) || (VEU_Specular == vs_elem.usage)) && !IsFloatFormat(vs_elem.format)) ? GL_TRUE : normalized;
					GLvoid* offset = static_cast<GLvoid*>(elem_offset + rl.StartInstanceLocation() * instance_size);

					stream.Active(false);
					glVertexAttribPointer(attr, num_components, type, normalized, instance_size, offset);
					glEnableVertexAttribArray(attr);

					glVertexAttribDivisor(attr, 1);
				}

				elem_offset += vs_elem.element_size();
			}

			if (so_rl_)
			{
				glBeginTransformFeedback(so_primitive_mode_);
			}

			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();

					if (so_rl_)
					{
						OGLESShaderObjectPtr shader = checked_pointer_cast<OGLESShaderObject>(pass->GetShaderObject());
						glTransformFeedbackVaryings(shader->GLSLProgram(), static_cast<GLsizei>(so_vars_ptrs_.size()), &so_vars_ptrs_[0], GL_SEPARATE_ATTRIBS);
						for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
						{
							glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, j, so_buffs_[j]);
						}
					}

					glDrawElementsInstanced(mode, static_cast<GLsizei>(rl.NumIndices()),
						index_type, index_offset, num_instance);
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
						OGLESShaderObjectPtr shader = checked_pointer_cast<OGLESShaderObject>(pass->GetShaderObject());
						glTransformFeedbackVaryings(shader->GLSLProgram(), static_cast<GLsizei>(so_vars_ptrs_.size()), &so_vars_ptrs_[0], GL_SEPARATE_ATTRIBS);							for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
						{
							glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, j, so_buffs_[j]);
						}
					}

					glDrawArraysInstanced(mode, rl.StartVertexLocation(), static_cast<GLsizei>(rl.NumVertices()), num_instance);
					pass->Unbind();
				}
			}

			if (so_rl_)
			{
				glEndTransformFeedback();
			}

			for (size_t i = 0; i < inst_format_size; ++ i)
			{
				vertex_element const & vs_elem = rl.InstanceStreamFormat()[i];
				GLint attr = cur_shader->GetAttribLocation(vs_elem.usage, vs_elem.usage_index);
				if (attr != -1)
				{
					glDisableVertexAttribArray(attr);
					glVertexAttribDivisor(attr, 0);
				}
			}
		}
		else
		{
			for (uint32_t instance = rl.StartInstanceLocation(); instance < rl.StartInstanceLocation() + num_instance; ++ instance)
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
							OGLESMapping::MappingVertexFormat(type, normalized, vs_elem.format);
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
									GLubyte const * byte_addr = static_cast<GLubyte const *>(addr);
									if (normalized)
									{
										glVertexAttrib4f(attr, byte_addr[0] / 255.0f, byte_addr[1] / 255.0f, byte_addr[2] / 255.0f, byte_addr[3] / 255.0f);
									}
									else
									{
										glVertexAttrib4f(attr, byte_addr[0], byte_addr[1], byte_addr[2], byte_addr[3]);
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
					glBeginTransformFeedback(so_primitive_mode_);
				}

				if (rl.UseIndices())
				{
					for (uint32_t i = 0; i < num_passes; ++ i)
					{
						RenderPassPtr const & pass = tech.Pass(i);

						pass->Bind();

						if (so_rl_)
						{
							OGLESShaderObjectPtr shader = checked_pointer_cast<OGLESShaderObject>(pass->GetShaderObject());
							glTransformFeedbackVaryings(shader->GLSLProgram(), static_cast<GLsizei>(so_vars_ptrs_.size()), &so_vars_ptrs_[0], GL_SEPARATE_ATTRIBS);
							for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
							{
								glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, j, so_buffs_[j]);
							}
						}

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

						if (so_rl_)
						{
							OGLESShaderObjectPtr shader = checked_pointer_cast<OGLESShaderObject>(pass->GetShaderObject());
							glTransformFeedbackVaryings(shader->GLSLProgram(), static_cast<GLsizei>(so_vars_ptrs_.size()), &so_vars_ptrs_[0], GL_SEPARATE_ATTRIBS);
							for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
							{
								glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, j, so_buffs_[j]);
							}
						}

						glDrawArrays(mode, rl.StartVertexLocation(), static_cast<GLsizei>(rl.NumVertices()));
						pass->Unbind();
					}
				}

				if (so_rl_)
				{
					glEndTransformFeedback();
				}
			}
		}

		checked_cast<OGLESRenderLayout const *>(&rl)->Deactive(cur_shader);
	}

	void OGLESRenderEngine::DoDispatch(RenderTechnique const & /*tech*/, uint32_t /*tgx*/, uint32_t /*tgy*/, uint32_t /*tgz*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESRenderEngine::ForceFlush()
	{
		glFlush();
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glScissor(x, y, width, height);
	}

	void OGLESRenderEngine::GetCustomAttrib(std::string const & name, void* value)
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

	void OGLESRenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_pointer_cast<OGLESRenderWindow>(screen_frame_buffer_)->Resize(width, height);
	}

	bool OGLESRenderEngine::FullScreen() const
	{
		return checked_pointer_cast<OGLESRenderWindow>(screen_frame_buffer_)->FullScreen();
	}

	void OGLESRenderEngine::FullScreen(bool fs)
	{
		checked_pointer_cast<OGLESRenderWindow>(screen_frame_buffer_)->FullScreen(fs);
	}

	void OGLESRenderEngine::AdjustProjectionMatrix(float4x4& proj_mat)
	{
		proj_mat *= MathLib::scaling(1.0f, 1.0f, 2.0f) * MathLib::translation(0.0f, 0.0f, -1.0f);
	}

	bool OGLESRenderEngine::VertexFormatSupport(ElementFormat elem_fmt)
	{
		return vertex_format_.find(elem_fmt) != vertex_format_.end();
	}

	bool OGLESRenderEngine::TextureFormatSupport(ElementFormat elem_fmt)
	{
		return texture_format_.find(elem_fmt) != texture_format_.end();
	}

	bool OGLESRenderEngine::RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t /*sample_quality*/)
	{
		return (rendertarget_format_.find(elem_fmt) != rendertarget_format_.end()) && (sample_count <= 1);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::FillRenderDeviceCaps()
	{
		std::string vendor(reinterpret_cast<char const *>(glGetString(GL_VENDOR)));
		if (vendor.find("Imagination", 0) != std::string::npos)
		{
			hack_for_pvr_ = true;
		}
		else
		{
			hack_for_pvr_ = false;
		}
		if (vendor.find("ARM", 0) != std::string::npos)
		{
			hack_for_mali_ = true;
		}
		else
		{
			hack_for_mali_ = false;
		}

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
			if (caps_.max_vertex_texture_units != 0)
			{
				caps_.max_shader_model = 3;
			}
			else
			{
				caps_.max_shader_model = 2;
			}
		}

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &temp);
		caps_.max_texture_height = caps_.max_texture_width = temp;
		if (hack_for_pvr_ || hack_for_mali_)
		{
			caps_.max_texture_depth = 1;
		}
		else
		{
			if (glloader_GLES_VERSION_3_0() || glloader_GLES_OES_texture_3D())
			{
				glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_OES, &temp);
				caps_.max_texture_depth = temp;
			}
			else
			{
				caps_.max_texture_depth = 1;
			}
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

		if (glloader_GLES_VERSION_3_0())
		{
			glGetIntegerv(GL_MAX_DRAW_BUFFERS, &temp);
			caps_.max_simultaneous_rts = static_cast<uint8_t>(temp);
		}
		else
		{
			caps_.max_simultaneous_rts = 1;
		}
		caps_.max_simultaneous_uavs = 0;

		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &temp);
		caps_.max_vertex_streams = static_cast<uint8_t>(temp);

		caps_.is_tbdr = false;

		caps_.hw_instancing_support = true;
		caps_.instance_id_support = false;
		caps_.stream_output_support = false;
		caps_.alpha_to_coverage_support = true;
		if (glloader_GLES_VERSION_3_0())
		{
			caps_.primitive_restart_support = true;
		}
		else
		{
			caps_.primitive_restart_support = false;
		}
		caps_.multithread_rendering_support = false;
		caps_.multithread_res_creating_support = false;
		caps_.mrt_independent_bit_depths_support = false;
		if (hack_for_pvr_ || hack_for_mali_)
		{
			caps_.standard_derivatives_support = false;
		}
		else
		{
			if (glloader_GLES_VERSION_3_0() || glloader_GLES_OES_standard_derivatives())
			{
				glGetIntegerv(GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES, &temp);
				caps_.standard_derivatives_support = (temp != 0);
			}
			else
			{
				caps_.standard_derivatives_support = false;
			}
		}
		caps_.logic_op_support = false;

		caps_.gs_support = false;
		caps_.cs_support = false;
		caps_.hs_support = false;
		caps_.ds_support = false;
		caps_.tess_method = TM_No;

		
		vertex_format_.insert(EF_A8);
		vertex_format_.insert(EF_R8);
		vertex_format_.insert(EF_GR8);
		vertex_format_.insert(EF_BGR8);
		if (glloader_GLES_EXT_texture_format_BGRA8888())
		{
			vertex_format_.insert(EF_ARGB8);
		}
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
		if (glloader_GLES_OES_vertex_type_10_10_10_2())
		{
			vertex_format_.insert(EF_A2BGR10);
		}
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
		vertex_format_.insert(EF_R32F);
		vertex_format_.insert(EF_GR32F);
		vertex_format_.insert(EF_BGR32F);
		vertex_format_.insert(EF_ABGR32F);
		vertex_format_.insert(EF_R16F);
		vertex_format_.insert(EF_GR16F);
		vertex_format_.insert(EF_BGR16F);
		vertex_format_.insert(EF_ABGR16F);

		texture_format_.insert(EF_A8);
		texture_format_.insert(EF_ARGB4);
		texture_format_.insert(EF_R8);
		texture_format_.insert(EF_SIGNED_R8);
		if (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_texture_rg())
		{
			texture_format_.insert(EF_GR8);
		}
		texture_format_.insert(EF_ABGR8);
		if (glloader_GLES_EXT_texture_format_BGRA8888())
		{
			texture_format_.insert(EF_ARGB8);
		}
		if (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_texture_type_2_10_10_10_REV())
		{
			texture_format_.insert(EF_A2BGR10);
		}
		if (glloader_GLES_VERSION_3_0())
		{
			texture_format_.insert(EF_R8UI);
			texture_format_.insert(EF_R8I);
			texture_format_.insert(EF_GR8UI);
			texture_format_.insert(EF_GR8I);
			texture_format_.insert(EF_BGR8UI);
			texture_format_.insert(EF_BGR8I);
			texture_format_.insert(EF_R16UI);
			texture_format_.insert(EF_R16I);
			texture_format_.insert(EF_GR16UI);
			texture_format_.insert(EF_GR16I);
			texture_format_.insert(EF_BGR16UI);
			texture_format_.insert(EF_BGR16I);
			texture_format_.insert(EF_ABGR16UI);
			texture_format_.insert(EF_ABGR16I);
			texture_format_.insert(EF_R32UI);
			texture_format_.insert(EF_R32I);
			texture_format_.insert(EF_GR32UI);
			texture_format_.insert(EF_GR32I);
			texture_format_.insert(EF_BGR32UI);
			texture_format_.insert(EF_BGR32I);
			texture_format_.insert(EF_ABGR32UI);
			texture_format_.insert(EF_ABGR32I);
		}
		if ((glloader_GLES_VERSION_3_0() || glloader_GLES_OES_texture_half_float()) && !hack_for_pvr_)
		{
			texture_format_.insert(EF_R16F);
			texture_format_.insert(EF_GR16F);
			texture_format_.insert(EF_BGR16F);
			texture_format_.insert(EF_ABGR16F);
		}
		if (glloader_GLES_VERSION_3_0())
		{
			texture_format_.insert(EF_B10G11R11F);
		}
		if (glloader_GLES_OES_texture_float())
		{
			texture_format_.insert(EF_R32F);
			texture_format_.insert(EF_GR32F);
			texture_format_.insert(EF_BGR32F);
			texture_format_.insert(EF_ABGR32F);
		}
		if (glloader_GLES_EXT_texture_compression_dxt1() || glloader_GLES_EXT_texture_compression_s3tc())
		{
			texture_format_.insert(EF_BC1);
		}
		if (glloader_GLES_EXT_texture_compression_s3tc())
		{
			texture_format_.insert(EF_BC2);
			texture_format_.insert(EF_BC3);
		}
		if (glloader_GLES_EXT_texture_compression_latc() && !(hack_for_pvr_ || hack_for_mali_))
		{
			texture_format_.insert(EF_BC4);
			texture_format_.insert(EF_BC5);
			texture_format_.insert(EF_SIGNED_BC4);
			texture_format_.insert(EF_SIGNED_BC5);
		}
		if ((glloader_GLES_VERSION_3_0() || glloader_GLES_OES_depth_texture()))
		{
			texture_format_.insert(EF_D16);
		}
		if ((glloader_GLES_VERSION_3_0() || glloader_GLES_OES_packed_depth_stencil()))
		{
			texture_format_.insert(EF_D24S8);
		}
		if (glloader_GLES_VERSION_3_0())
		{
			texture_format_.insert(EF_D32F);
		}
		if (glloader_GLES_VERSION_3_0())
		{
			texture_format_.insert(EF_ABGR8_SRGB);
		}

		if (glloader_GLES_EXT_texture_format_BGRA8888())
		{
			rendertarget_format_.insert(EF_ARGB8);
		}

		if (glloader_GLES_VERSION_3_0)
		{
			GLint max_samples;
			glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
			max_samples_ = static_cast<uint32_t>(max_samples);
		}
		else if (glloader_GLES_EXT_multisampled_render_to_texture())
		{
			GLint max_samples;
			glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_samples);
			max_samples_ = static_cast<uint32_t>(max_samples);
		}
		else
		{
			max_samples_ = 1;
		}

		if (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_texture_rg())
		{
			rendertarget_format_.insert(EF_R8);
			rendertarget_format_.insert(EF_GR8);
		}
		rendertarget_format_.insert(EF_ABGR8);
		rendertarget_format_.insert(EF_SIGNED_ABGR8);
		if (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_texture_type_2_10_10_10_REV())
		{
			rendertarget_format_.insert(EF_A2BGR10);
		}
		if (glloader_GLES_VERSION_3_0())
		{
			rendertarget_format_.insert(EF_R16UI);
			rendertarget_format_.insert(EF_R16I);
			rendertarget_format_.insert(EF_GR16UI);
			rendertarget_format_.insert(EF_GR16I);
			rendertarget_format_.insert(EF_ABGR16UI);
			rendertarget_format_.insert(EF_ABGR16I);
			rendertarget_format_.insert(EF_R32UI);
			rendertarget_format_.insert(EF_R32I);
			rendertarget_format_.insert(EF_GR32UI);
			rendertarget_format_.insert(EF_GR32I);
			rendertarget_format_.insert(EF_ABGR32UI);
			rendertarget_format_.insert(EF_ABGR32I);
		}
		if (glloader_GLES_EXT_color_buffer_half_float() || glloader_GLES_EXT_color_buffer_float())
		{
			rendertarget_format_.insert(EF_R16F);
			rendertarget_format_.insert(EF_GR16F);
			rendertarget_format_.insert(EF_ABGR16F);
		}
		if (glloader_GLES_EXT_color_buffer_float())
		{
			rendertarget_format_.insert(EF_R32F);
			rendertarget_format_.insert(EF_GR32F);
			rendertarget_format_.insert(EF_ABGR32F);
		}
		rendertarget_format_.insert(EF_D16);
		if (glloader_GLES_VERSION_3_0() || glloader_GLES_OES_packed_depth_stencil())
		{
			rendertarget_format_.insert(EF_D24S8);
		}
		if (glloader_GLES_VERSION_3_0())
		{
			rendertarget_format_.insert(EF_D32F);
		}
		if (glloader_GLES_VERSION_3_0())
		{
			rendertarget_format_.insert(EF_ABGR8_SRGB);
		}

		caps_.vertex_format_support = bind<bool>(&OGLESRenderEngine::VertexFormatSupport, this,
			placeholders::_1);
		caps_.texture_format_support = bind<bool>(&OGLESRenderEngine::TextureFormatSupport, this,
			placeholders::_1);
		caps_.rendertarget_format_support = bind<bool>(&OGLESRenderEngine::RenderTargetFormatSupport, this,
			placeholders::_1, placeholders::_2, placeholders::_3);
	}
}
