// OGLESRenderEngine.cpp
// KlayGE OpenGL ES 2��Ⱦ������ ʵ���ļ�
// Ver 3.12.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
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
#include <KFL/Hash.hpp>

#include <glloader/glloader.h>

#include <algorithm>
#include <cstring>
#include <ostream>
#include <string>

#include <boost/assert.hpp>

#include <KlayGE/OpenGLES/OGLESUtil.hpp>
#include <KlayGE/OpenGLES/OGLESRenderWindow.hpp>
#include <KlayGE/OpenGLES/OGLESFrameBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderView.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderLayout.hpp>
#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESRenderStateObject.hpp>
#include <KlayGE/OpenGLES/OGLESShaderObject.hpp>

namespace
{
	char const * DebugSourceString(GLenum value)
	{
		char const * ret;
		switch (value)
		{
		case GL_DEBUG_SOURCE_API:
			ret = "GL";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			ret = "shader compiler";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			ret = "window system";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			ret = "3rd party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			ret = "application";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			ret = "other";
			break;

		default:
			KFL_UNREACHABLE("Invalid debug source");
		}

		return ret;
	}

	char const * DebugTypeString(GLenum value)
	{
		char const * ret;
		switch (value)
		{
		case GL_DEBUG_TYPE_ERROR:
			ret = "error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			ret = "deprecated behavior";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			ret = "undefined behavior";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			ret = "performance";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			ret = "portability";
			break;
		case GL_DEBUG_TYPE_MARKER:
			ret = "marker";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			ret = "push group";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			ret = "pop group";
			break;
		case GL_DEBUG_TYPE_OTHER:
			ret = "other";
			break;

		default:
			KFL_UNREACHABLE("Invalid debug type");
		}

		return ret;
	}

	char const * DebugSeverityString(GLenum value)
	{
		char const * ret;
		switch (value)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			ret = "high";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			ret = "medium";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			ret = "low";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			ret = "notification";
			break;

		default:
			KFL_UNREACHABLE("Invalid debug severity");
		}

		return ret;
	}

	void GLLOADER_APIENTRY DebugOutputProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
			GLchar const * message, void const * user_param)
	{
		KFL_UNUSED(length);
		KFL_UNUSED(user_param);

		auto& os = (GL_DEBUG_TYPE_ERROR == type) ? KlayGE::LogError() : KlayGE::LogInfo();
		os << "OpenGL debug output: source: " << DebugSourceString(source) << "; "
			<< "type: " << DebugTypeString(type) << "; "
			<< "id: " << id << "; "
			<< "severity: " << DebugSeverityString(severity) << "; "
			<< "message: " << message << std::endl;
	}
}

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	OGLESRenderEngine::OGLESRenderEngine()
		: fbo_blit_src_(0), fbo_blit_dst_(0),
			clear_depth_(1), clear_stencil_(0), cur_program_(0),
			vp_x_(0), vp_y_(0), vp_width_(0), vp_height_(0),
			cur_fbo_(0)
	{
		native_shader_fourcc_ = MakeFourCC<'E', 'S', 'S', 'L'>::value;
		native_shader_version_ = 3;

		clear_clr_.fill(0);
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	OGLESRenderEngine::~OGLESRenderEngine()
	{
		this->Destroy();
	}

	// ������Ⱦϵͳ������
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & OGLESRenderEngine::Name() const
	{
		static const std::wstring name(L"OpenGL ES Render Engine");
		return name;
	}

	// ������Ⱦ����
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		FrameBufferPtr win = MakeSharedPtr<OGLESRenderWindow>(name, settings);

		if (glloader_GLES_VERSION_3_2())
		{
			native_shader_platform_name_ = "gles_3_2";
		}
		else if (glloader_GLES_VERSION_3_1())
		{
			native_shader_platform_name_ = "gles_3_1";
		}
		else //if (glloader_GLES_VERSION_3_0())
		{
			native_shader_platform_name_ = "gles_3_0";
		}

		this->FillRenderDeviceCaps();
		this->InitRenderStates();

#ifdef KLAYGE_PLATFORM_DARWIN
		Context::Instance().AppInstance().MainWnd()->BindListeners();
#endif

#ifdef KLAYGE_DEBUG
		bool const debug_context = true;
#else
		bool const debug_context = settings.debug_context;
#endif
		if (debug_context)
		{
			if (glloader_GLES_VERSION_3_2())
			{
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				glDebugMessageCallback(&DebugOutputProc, nullptr);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
			}
			else if (glloader_GLES_KHR_debug())
			{
				glEnable(GL_DEBUG_OUTPUT_KHR);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR);
				glDebugMessageCallbackKHR(&DebugOutputProc, nullptr);
				glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH_KHR, 0, nullptr, GL_TRUE);
				glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM_KHR, 0, nullptr, GL_TRUE);
				glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW_KHR, 0, nullptr, GL_FALSE);
				glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION_KHR, 0, nullptr, GL_FALSE);
			}
		}

#if defined(KLAYGE_PLATFORM_IOS)
		win->Attach(FrameBuffer::ATT_Color0,
			MakeSharedPtr<OGLESEAGLRenderView>(settings.color_fmt));
		if (NumDepthBits(settings.depth_stencil_fmt) > 0)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			win->Attach(FrameBuffer::ATT_DepthStencil,
				rf.Make2DDsv(win->Width(), win->Height(), settings.depth_stencil_fmt, 1, 0));
		}
#else
		win->Attach(FrameBuffer::Attachment::Color0,
			MakeSharedPtr<OGLESScreenRenderTargetView>(win->Width(), win->Height(), settings.color_fmt));
		if (NumDepthBits(settings.depth_stencil_fmt) > 0)
		{
			win->Attach(MakeSharedPtr<OGLESScreenDepthStencilView>(win->Width(), win->Height(), settings.depth_stencil_fmt));
		}
#endif

		this->BindFrameBuffer(win);

		glGenFramebuffers(1, &fbo_blit_src_);
		glGenFramebuffers(1, &fbo_blit_dst_);
	}

	void OGLESRenderEngine::CheckConfig(RenderSettings& settings)
	{
#if defined KLAYGE_PLATFORM_ANDROID || defined(KLAYGE_PLATFORM_IOS)
		settings.hdr = false;
		settings.ppaa = false;
		settings.gamma = false;
		settings.color_grading = false;
#else
		KFL_UNUSED(settings);
#endif
	}

	void OGLESRenderEngine::InitRenderStates()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRenderStateObject(RasterizerStateDesc(), DepthStencilStateDesc(), BlendStateDesc());
		checked_pointer_cast<OGLESRenderStateObject>(cur_rs_obj_)->ForceDefaultState();

		glEnable(GL_POLYGON_OFFSET_FILL);
		if (caps_.primitive_restart_support)
		{
			glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
		}

		active_tex_unit_ = GL_TEXTURE0;
		glActiveTexture(active_tex_unit_);

		binded_textures_.clear();
		binded_samplers_.clear();
		binded_buffers_.clear();
	}

	void OGLESRenderEngine::ActiveTexture(GLenum tex_unit)
	{
		if (tex_unit != active_tex_unit_)
		{
			glActiveTexture(tex_unit);
			active_tex_unit_ = tex_unit;
		}
	}

	void OGLESRenderEngine::BindTexture(GLuint index, GLuint target, GLuint texture, bool force)
	{
		this->BindTextures(index, 1, &target, &texture, force);
	}

	void OGLESRenderEngine::BindTextures(GLuint first, GLsizei count, GLuint const * targets, GLuint const * textures, bool force)
	{
		if (first + count > binded_textures_.size())
		{
			binded_textures_.resize(first + count, std::make_pair(0, 0xFFFFFFFF));
		}

		bool dirty = force;
		if (!dirty)
		{
			uint32_t start_dirty = first;
			uint32_t end_dirty = first + count;
			while ((start_dirty != end_dirty) && (binded_textures_[start_dirty].first == targets[start_dirty])
				&& (binded_textures_[start_dirty].second == textures[start_dirty]))
			{
				++ start_dirty;
			}
			while ((start_dirty != end_dirty) && (binded_textures_[end_dirty - 1].first == targets[end_dirty - 1])
				&& (binded_textures_[end_dirty - 1].second == textures[end_dirty - 1]))
			{
				-- end_dirty;
			}

			first = start_dirty;
			count = end_dirty - start_dirty;
			dirty = (count > 0);
		}

		if (dirty)
		{
			for (uint32_t i = first; i < first + count; ++ i)
			{
				this->ActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(targets[i], textures[i]);

				binded_textures_[i] = std::make_pair(targets[i], textures[i]);
			}
		}
	}

	void OGLESRenderEngine::BindSampler(GLuint index, GLuint sampler, bool force)
	{
		this->BindSamplers(index, 1, &sampler, force);
	}

	void OGLESRenderEngine::BindSamplers(GLuint first, GLsizei count, GLuint const * samplers, bool force)
	{
		if (first + count > binded_samplers_.size())
		{
			binded_samplers_.resize(first + count, 0xFFFFFFFF);
		}

		bool dirty = force;
		if (!dirty)
		{
			uint32_t start_dirty = first;
			uint32_t end_dirty = first + count;
			while ((start_dirty != end_dirty) && (binded_samplers_[start_dirty] == samplers[start_dirty]))
			{
				++ start_dirty;
			}
			while ((start_dirty != end_dirty) && (binded_samplers_[end_dirty - 1] == samplers[end_dirty - 1]))
			{
				-- end_dirty;
			}

			first = start_dirty;
			count = end_dirty - start_dirty;
			dirty = (count > 0);
		}

		if (dirty)
		{
			for (uint32_t i = first; i < first + count; ++ i)
			{
				glBindSampler(i, samplers[i]);
			}

			memcpy(&binded_samplers_[first], &samplers[first], count * sizeof(samplers[0]));
		}
	}

	void OGLESRenderEngine::BindBuffer(GLenum target, GLuint buffer, bool force)
	{
		auto iter = binded_buffers_.find(target);
		if (force || (iter == binded_buffers_.end()) || (iter->second != buffer))
		{
			glBindBuffer(target, buffer);
			binded_buffers_[target] = buffer;
		}
	}

	void OGLESRenderEngine::BindBuffersBase(GLenum target, GLuint first, GLsizei count, GLuint const * buffers, bool force)
	{
		auto& binded = binded_buffers_with_binding_points_[target];
		if (first + count > binded.size())
		{
			binded.resize(first + count, 0xFFFFFFFF);
		}

		bool dirty = force;
		if (!dirty)
		{
			dirty = (memcmp(&binded[first], buffers, count * sizeof(buffers[0])) != 0);
		}

		if (dirty)
		{
			for (uint32_t i = first; i < first + count; ++ i)
			{
				glBindBufferBase(target, i, buffers[i - first]);
			}
			auto iter = binded_buffers_.find(target);
			if (iter != binded_buffers_.end())
			{
				glBindBuffer(target, iter->second);
			}

			memcpy(&binded[first], buffers, count * sizeof(buffers[0]));
		}
	}

	void OGLESRenderEngine::DeleteTextures(GLsizei n, GLuint const * textures)
	{
		for (GLsizei i = 0; i < n; ++ i)
		{
			for (auto iter = binded_textures_.begin(); iter != binded_textures_.end();)
			{
				if (iter->second == textures[i])
				{
					iter = binded_textures_.erase(iter);
				}
				else
				{
					++ iter;
				}
			}
		}
		glDeleteTextures(n, textures);
	}

	void OGLESRenderEngine::DeleteSamplers(GLsizei n, GLuint const * samplers)
	{
		for (GLsizei i = 0; i < n; ++ i)
		{
			for (auto iter = binded_samplers_.begin(); iter != binded_samplers_.end();)
			{
				if (*iter == samplers[i])
				{
					iter = binded_samplers_.erase(iter);
				}
				else
				{
					++ iter;
				}
			}
		}
		glDeleteSamplers(n, samplers);
	}

	void OGLESRenderEngine::DeleteBuffers(GLsizei n, GLuint const * buffers)
	{
		for (GLsizei i = 0; i < n; ++ i)
		{
			for (auto iter = binded_buffers_.begin(); iter != binded_buffers_.end();)
			{
				if (iter->second == buffers[i])
				{
					binded_buffers_.erase(iter ++);
				}
				else
				{
					++ iter;
				}
			}

			for (auto iter_target = binded_buffers_with_binding_points_.begin();
				iter_target != binded_buffers_with_binding_points_.end();
				++ iter_target)
			{
				for (auto iter_buff = iter_target->second.begin(); iter_buff != iter_target->second.end();)
				{
					if (*iter_buff == buffers[i])
					{
						iter_buff = iter_target->second.erase(iter_buff);
					}
					else
					{
						++ iter_buff;
					}
				}
			}
		}
		glDeleteBuffers(n, buffers);
	}

	void OGLESRenderEngine::OverrideBindBufferCache(GLenum target, GLuint buffer)
	{
		auto iter = binded_buffers_.find(target);
		if (iter != binded_buffers_.end())
		{
			iter->second = buffer;
		}
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
		auto iter_p = uniformi_cache_.find(cur_program_);
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.emplace(cur_program_, (std::map<GLint, int4>())).first;
		}
		auto iter_v = iter_p->second.find(location);
		if (iter_v == iter_p->second.end())
		{
			dirty = true;
			iter_p->second.emplace(location, int4(value, 0, 0, 0));
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
		auto iter_p = uniformf_cache_.find(cur_program_);
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.emplace(cur_program_, (std::map<GLint, float4>())).first;
		}
		auto iter_v = iter_p->second.find(location);
		if (iter_v == iter_p->second.end())
		{
			dirty = true;
			iter_p->second.emplace(location, float4(value, 0, 0, 0));
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
		auto iter_p = uniformi_cache_.find(cur_program_);
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.emplace(cur_program_, (std::map<GLint, int4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location, int4(value[i], 0, 0, 0));
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
		auto iter_p = uniformf_cache_.find(cur_program_);
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.emplace(cur_program_, (std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location, float4(value[i], 0, 0, 0));
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
		auto iter_p = uniformi_cache_.find(cur_program_);
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.emplace(cur_program_, (std::map<GLint, int4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location, int4(value[i * 2 + 0], value[i * 2 + 1], 0, 0));
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
		auto iter_p = uniformf_cache_.find(cur_program_);
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.emplace(cur_program_, (std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location, float4(value[i * 2 + 0], value[i * 2 + 1], 0, 0));
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
		auto iter_p = uniformi_cache_.find(cur_program_);
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.emplace(cur_program_, (std::map<GLint, int4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location, int4(value[i * 3 + 0], value[i * 3 + 1], value[i * 3 + 2], 0));
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
		auto iter_p = uniformf_cache_.find(cur_program_);
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.emplace(cur_program_, (std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location, float4(value[i * 3 + 0], value[i * 3 + 1], value[i * 3 + 2], 0));
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
		auto iter_p = uniformi_cache_.find(cur_program_);
		if (iter_p == uniformi_cache_.end())
		{
			dirty = true;
			iter_p = uniformi_cache_.emplace(cur_program_, (std::map<GLint, int4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location,
					int4(value[i * 4 + 0], value[i * 4 + 1], value[i * 4 + 2], value[i * 4 + 3]));
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
		auto iter_p = uniformf_cache_.find(cur_program_);
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.emplace(cur_program_, (std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location,
					float4(value[i * 4 + 0], value[i * 4 + 1], value[i * 4 + 2], value[i * 4 + 3]));
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

	void OGLESRenderEngine::UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, GLfloat const * value)
	{
		bool dirty = false;
		auto iter_p = uniformf_cache_.find(cur_program_);
		if (iter_p == uniformf_cache_.end())
		{
			dirty = true;
			iter_p = uniformf_cache_.emplace(cur_program_, (std::map<GLint, float4>())).first;
		}
		for (GLsizei i = 0; i < count * 4; ++ i)
		{
			auto iter_v = iter_p->second.find(location + i);
			if (iter_v == iter_p->second.end())
			{
				dirty = true;
				iter_p->second.emplace(location,
					float4(value[i * 4 + 0], value[i * 4 + 1], value[i * 4 + 2], value[i * 4 + 3]));
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
			glUniformMatrix4fv(location, count, transpose, value);
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

	// ���õ�ǰ��ȾĿ��
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
	{
		BOOST_ASSERT(fb);

		Viewport const & vp = *fb->Viewport();
		if ((vp_x_ != vp.Left()) || (vp_y_ != vp.Top()) || (vp_width_ != vp.Width()) || (vp_height_ != vp.Height()))
		{
			glViewport(vp.Left(), vp.Top(), vp.Width(), vp.Height());

			vp_x_ = vp.Left();
			vp_y_ = vp.Top();
			vp_width_ = vp.Width();
			vp_height_ = vp.Height();
		}
	}

	// ���õ�ǰStream outputĿ��
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
				KFL_UNREACHABLE("Invalid topology type");
			}

			so_buffs_.resize(so_rl_->NumVertexStreams());
			for (uint32_t i = 0; i < so_rl_->NumVertexStreams(); ++ i)
			{
				so_buffs_[i] = checked_pointer_cast<OGLESGraphicsBuffer>(so_rl_->GetVertexStream(i))->GLvbo();
			}
		}
		else
		{
			so_buffs_.clear();
		}
	}

	// ��Ⱦ
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t const num_instances = rl.NumInstances() * this->NumRealizedCameraInstances();
		BOOST_ASSERT(num_instances != 0);

		OGLESShaderObjectPtr cur_shader = checked_pointer_cast<OGLESShaderObject>(tech.Pass(0).GetShaderObject(effect));
		checked_cast<OGLESRenderLayout const&>(rl).Active(cur_shader);

		uint32_t const vertex_count = rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();
		GLenum mode;
		uint32_t prim_count;
		OGLESMapping::Mapping(mode, prim_count, rl);

		num_primitives_just_rendered_ += num_instances * prim_count;
		num_vertices_just_rendered_ += num_instances * vertex_count;

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
		GraphicsBufferPtr const & buff_args = rl.GetIndirectArgs();
		if (glloader_GLES_VERSION_3_1() && buff_args)
		{
			this->BindBuffer(GL_DRAW_INDIRECT_BUFFER, checked_pointer_cast<OGLESGraphicsBuffer>(buff_args)->GLvbo());
			GLvoid* args_offset = reinterpret_cast<GLvoid*>(static_cast<GLintptr>(rl.IndirectArgsOffset()));
			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);

					if (so_rl_)
					{
						OGLESShaderObjectPtr shader = checked_pointer_cast<OGLESShaderObject>(pass.GetShaderObject(effect));
						for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
						{
							glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, j, so_buffs_[j]);
						}

						glBeginTransformFeedback(so_primitive_mode_);
					}

					glDrawElementsIndirect(mode, index_type, args_offset);

					if (so_rl_)
					{
						glEndTransformFeedback();
					}

					pass.Unbind(effect);
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);

					if (so_rl_)
					{
						OGLESShaderObjectPtr shader = checked_pointer_cast<OGLESShaderObject>(pass.GetShaderObject(effect));
						for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
						{
							glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, j, so_buffs_[j]);
						}

						glBeginTransformFeedback(so_primitive_mode_);
					}

					glDrawArraysIndirect(mode, args_offset);

					if (so_rl_)
					{
						glEndTransformFeedback();
					}

					pass.Unbind(effect);
				}
			}

			num_draws_just_called_ += num_passes;
		}
		else
		{
			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);

					if (so_rl_)
					{
						OGLESShaderObjectPtr shader = checked_pointer_cast<OGLESShaderObject>(pass.GetShaderObject(effect));
						for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
						{
							glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, j, so_buffs_[j]);
						}

						glBeginTransformFeedback(so_primitive_mode_);
					}

					glDrawElementsInstanced(mode, static_cast<GLsizei>(rl.NumIndices()), index_type, index_offset, num_instances);

					if (so_rl_)
					{
						glEndTransformFeedback();
					}

					pass.Unbind(effect);
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);

					if (so_rl_)
					{
						OGLESShaderObjectPtr shader = checked_pointer_cast<OGLESShaderObject>(pass.GetShaderObject(effect));
						for (uint32_t j = 0; j < so_buffs_.size(); ++ j)
						{
							glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, j, so_buffs_[j]);
						}

						glBeginTransformFeedback(so_primitive_mode_);
					}

					glDrawArraysInstanced(mode, rl.StartVertexLocation(), static_cast<GLsizei>(rl.NumVertices()), num_instances);

					if (so_rl_)
					{
						glEndTransformFeedback();
					}

					pass.Unbind(effect);
				}
			}

			num_draws_just_called_ += num_passes;
		}
	}

	void OGLESRenderEngine::DoDispatch(RenderEffect const & effect, RenderTechnique const & tech,
		uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(tgx);
		KFL_UNUSED(tgy);
		KFL_UNUSED(tgz);

		KFL_UNREACHABLE("Not implemented");
	}

	void OGLESRenderEngine::DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
		GraphicsBufferPtr const & buff_args, uint32_t offset)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(buff_args);
		KFL_UNUSED(offset);

		KFL_UNREACHABLE("Not implemented");
	}

	void OGLESRenderEngine::ForceFlush()
	{
		glFlush();
	}

	TexturePtr const & OGLESRenderEngine::ScreenDepthStencilTexture() const
	{
		static TexturePtr ret;
		return ret;
	}

	// ���ü�������
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glScissor(x, y, width, height);
	}

	void OGLESRenderEngine::GetCustomAttrib(std::string_view name, void* value) const
	{
		size_t const name_hash = HashValue(std::move(name));
		if (CT_HASH("VENDOR") == name_hash)
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
		else if (CT_HASH("RENDERER") == name_hash)
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
		else if (CT_HASH("VERSION") == name_hash)
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
		else if (CT_HASH("SHADING_LANGUAGE_VERSION") == name_hash)
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
		else if (CT_HASH("NUM_FEATURES") == name_hash)
		{
			*static_cast<int*>(value) = glloader_num_features();
		}
		else if (0 == name.find("FEATURE_NAME_"))
		{
			int const n = std::stoi(std::string(name.substr(13)));
			*static_cast<std::string*>(value) = glloader_get_feature_name(n);
		}
	}

	void OGLESRenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_pointer_cast<OGLESRenderWindow>(screen_frame_buffer_)->Resize(width, height);
	}

	void OGLESRenderEngine::DoDestroy()
	{
		if (fbo_blit_src_ != 0)
		{
			glDeleteFramebuffers(1, &fbo_blit_src_);
		}
		if (fbo_blit_dst_ != 0)
		{
			glDeleteFramebuffers(1, &fbo_blit_dst_);
		}

		so_rl_.reset();

		glloader_uninit();
	}

	void OGLESRenderEngine::DoSuspend()
	{
		// TODO
	}

	void OGLESRenderEngine::DoResume()
	{
		// TODO
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

	// ����豸����
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderEngine::FillRenderDeviceCaps()
	{
		std::string vendor(reinterpret_cast<char const *>(glGetString(GL_VENDOR)));
		std::string renderer(reinterpret_cast<char const *>(glGetString(GL_RENDERER)));
		if (vendor.find("NVIDIA", 0) != std::string::npos)
		{
			hack_for_tegra_ = true;
		}
		else
		{
			hack_for_tegra_ = false;
		}
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
		if (vendor.find("Qualcomm", 0) != std::string::npos)
		{
			hack_for_adreno_ = true;
		}
		else
		{
			hack_for_adreno_ = false;
		}
		if (vendor.find("Google", 0) != std::string::npos)
		{
			if (renderer.find("ANGLE", 0) != std::string::npos)
			{
				hack_for_angle_ = true;
				hack_for_android_emulator_ = false;
			}
			else
			{
				hack_for_angle_ = false;
				hack_for_android_emulator_ = true;
			}
		}
		else
		{
			hack_for_angle_ = false;
			hack_for_android_emulator_ = false;
		}

		GLint temp;

		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &temp);
		caps_.max_vertex_texture_units = static_cast<uint8_t>(temp);

		caps_.max_shader_model = ShaderModel(4, 0);

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &temp);
		caps_.max_texture_height = caps_.max_texture_width = temp;
		if (this->HackForPVR() || this->HackForMali() || this->HackForAdreno())
		{
			caps_.max_texture_depth = 1;
		}
		else
		{
			glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_OES, &temp);
			caps_.max_texture_depth = temp;
		}

		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &temp);
		caps_.max_texture_cube_size = temp;

		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &temp);
		caps_.max_texture_array_length = temp;

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

		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &temp);
		caps_.max_simultaneous_rts = static_cast<uint8_t>(temp);
		caps_.max_simultaneous_uavs = 0;

		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &temp);
		caps_.max_vertex_streams = static_cast<uint8_t>(temp);

		caps_.is_tbdr = false;

		if (!this->HackForAdreno())
		{
			caps_.primitive_restart_support = true;
		}
		else
		{
			caps_.primitive_restart_support = false;
		}
		caps_.multithread_rendering_support = false;
		caps_.multithread_res_creating_support = false;
		caps_.arbitrary_multithread_rendering_support = false;
		caps_.mrt_independent_bit_depths_support = false;
		caps_.logic_op_support = false;
		caps_.independent_blend_support = false;
		if (glloader_GLES_VERSION_3_1())
		{
			caps_.draw_indirect_support = true;
		}
		else
		{
			caps_.draw_indirect_support = false;
		}
		caps_.no_overwrite_support = false;
		if (this->HackForAndroidEmulator())
		{
			caps_.full_npot_texture_support = false;
		}
		else
		{
			caps_.full_npot_texture_support = true;
		}
		if ((caps_.max_texture_array_length > 1)
			&& (glloader_GLES_VERSION_3_2() || glloader_GLES_OES_geometry_shader()
			|| glloader_GLES_EXT_geometry_shader() || glloader_GLES_ANDROID_extension_pack_es31a()))
		{
			caps_.render_to_texture_array_support = true;
		}
		else
		{
			caps_.render_to_texture_array_support = false;
		}
		caps_.explicit_multi_sample_support = false;	// TODO
		if (glloader_GLES_VERSION_3_2() || glloader_GLES_OES_texture_buffer() || glloader_GLES_EXT_texture_buffer())
		{
			caps_.load_from_buffer_support = true;
		}
		else
		{
			caps_.load_from_buffer_support = false;
		}
		caps_.uavs_at_every_stage_support = false;	// TODO
		caps_.rovs_support = false;	// TODO
		caps_.flexible_srvs_support = false; // TODO
		caps_.vp_rt_index_at_every_stage_support = false; // TODO

		caps_.gs_support = glloader_GLES_VERSION_3_2() || glloader_GLES_OES_geometry_shader()
			|| glloader_GLES_EXT_geometry_shader() || glloader_GLES_ANDROID_extension_pack_es31a();
		caps_.cs_support = false;
		if (glloader_GLES_VERSION_3_2() || glloader_GLES_OES_tessellation_shader()
			|| glloader_GLES_EXT_tessellation_shader() || glloader_GLES_ANDROID_extension_pack_es31a())
		{
			caps_.hs_support = true;
			caps_.ds_support = true;
			caps_.tess_method = TM_Hardware;
		}
		else
		{
			caps_.hs_support = false;
			caps_.ds_support = false;
			caps_.tess_method = TM_No;
		}

		{
			std::vector<ElementFormat> vertex_formats =
			{
				EF_A8,
				EF_R8,
				EF_GR8,
				EF_BGR8,
				EF_ABGR8,
				EF_R8UI,
				EF_GR8UI,
				EF_BGR8UI,
				EF_ABGR8UI,
				EF_SIGNED_R8,
				EF_SIGNED_GR8,
				EF_SIGNED_BGR8,
				EF_SIGNED_ABGR8,
				EF_R8I,
				EF_GR8I,
				EF_BGR8I,
				EF_ABGR8I,
				EF_R16,
				EF_GR16,
				EF_BGR16,
				EF_ABGR16,
				EF_R16UI,
				EF_GR16UI,
				EF_BGR16UI,
				EF_ABGR16UI,
				EF_SIGNED_R16,
				EF_SIGNED_GR16,
				EF_SIGNED_BGR16,
				EF_SIGNED_ABGR16,
				EF_R16I,
				EF_GR16I,
				EF_BGR16I,
				EF_ABGR16I,
				EF_R32UI,
				EF_GR32UI,
				EF_BGR32UI,
				EF_ABGR32UI,
				EF_R32I,
				EF_GR32I,
				EF_BGR32I,
				EF_ABGR32I,
				EF_R32F,
				EF_GR32F,
				EF_BGR32F,
				EF_ABGR32F,
				EF_R16F,
				EF_GR16F,
				EF_BGR16F,
				EF_ABGR16F
			};
			if (glloader_GLES_EXT_texture_format_BGRA8888())
			{
				vertex_formats.push_back(EF_ARGB8);
			}
			if (glloader_GLES_OES_vertex_type_10_10_10_2())
			{
				vertex_formats.push_back(EF_A2BGR10);
			}

			caps_.AssignVertexFormats(std::move(vertex_formats));
		}
		{
			std::vector<ElementFormat> texture_formats =
			{
				EF_A8,
				EF_ARGB4,
				EF_R8,
				EF_SIGNED_R8,
				EF_GR8,
				EF_ABGR8,
				EF_A2BGR10,
				EF_R8UI,
				EF_R8I,
				EF_GR8UI,
				EF_GR8I,
				EF_BGR8UI,
				EF_BGR8I,
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
				EF_B10G11R11F,
				EF_D16,
				EF_D24S8,
				EF_D32F,
				EF_ABGR8_SRGB,
				EF_ETC1,
				EF_ETC2_R11,
				EF_SIGNED_ETC2_R11,
				EF_ETC2_GR11,
				EF_SIGNED_ETC2_GR11,
				EF_ETC2_BGR8,
				EF_ETC2_BGR8_SRGB,
				EF_ETC2_A1BGR8,
				EF_ETC2_A1BGR8_SRGB,
				EF_ETC2_ABGR8,
				EF_ETC2_ABGR8_SRGB
			};
			if (glloader_GLES_EXT_texture_format_BGRA8888())
			{
				texture_formats.push_back(EF_ARGB8);
			}
			if (!this->HackForPVR() && !this->HackForAndroidEmulator())
			{
				texture_formats.insert(texture_formats.end(),
					{
						EF_R16F,
						EF_GR16F,
						EF_BGR16F,
						EF_ABGR16F
					});
			}
			if (glloader_GLES_OES_texture_float())
			{
				texture_formats.insert(texture_formats.end(),
					{
						EF_R32F,
						EF_GR32F,
						EF_BGR32F,
						EF_ABGR32F
					});
			}
			if (glloader_GLES_EXT_texture_compression_dxt1() || glloader_GLES_EXT_texture_compression_s3tc())
			{
				texture_formats.push_back(EF_BC1);
			}
			if (glloader_GLES_EXT_texture_compression_s3tc())
			{
				texture_formats.insert(texture_formats.end(),
					{
						EF_BC2,
						EF_BC3
					});
			}
			if (glloader_GLES_EXT_texture_compression_latc() && !(this->HackForPVR() || this->HackForMali() || this->HackForAdreno()))
			{
				texture_formats.insert(texture_formats.end(),
					{
						EF_BC4,
						EF_BC5,
						EF_SIGNED_BC4,
						EF_SIGNED_BC5
					});
			}

			caps_.AssignTextureFormats(std::move(texture_formats));
		}
		{
			GLint max_samples;
			glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

			std::map<ElementFormat, std::vector<uint32_t>> render_target_formats;
			auto add_render_target_format = [&render_target_formats, &max_samples](std::span<ElementFormat const> fmts)
			{
				for (auto fmt : fmts)
				{
					for (int i = 1; i <= max_samples; i *= 2)
					{
						render_target_formats[fmt].push_back(RenderDeviceCaps::EncodeSampleCountQuality(i, 1));
					}
				}
			};

			add_render_target_format(
				MakeSpan({
					EF_R8,
					EF_GR8,
					EF_ABGR8,
					EF_SIGNED_ABGR8,
					EF_A2BGR10,
					EF_R16UI,
					EF_R16I,
					EF_GR16UI,
					EF_GR16I,
					EF_ABGR16UI,
					EF_ABGR16I,
					EF_R32UI,
					EF_R32I,
					EF_GR32UI,
					EF_GR32I,
					EF_ABGR32UI,
					EF_ABGR32I,
					EF_D16,
					EF_D24S8,
					EF_D32F,
					EF_ABGR8_SRGB
				}));
			if (glloader_GLES_EXT_texture_format_BGRA8888())
			{
				add_render_target_format(MakeSpan<1>(EF_ARGB8));
			}
			if (glloader_GLES_VERSION_3_2() || glloader_GLES_EXT_color_buffer_half_float() || glloader_GLES_EXT_color_buffer_float())
			{
				add_render_target_format(
					MakeSpan({
						EF_R16F,
						EF_GR16F
					}));
			}
			if (glloader_GLES_VERSION_3_2() || glloader_GLES_EXT_color_buffer_half_float() || glloader_GLES_EXT_color_buffer_float()
				|| this->HackForTegra())
			{
				add_render_target_format(MakeSpan<1>(EF_ABGR16F));
			}
			if (glloader_GLES_VERSION_3_2() || glloader_GLES_APPLE_color_buffer_packed_float() || glloader_GLES_NV_packed_float())
			{
				add_render_target_format(MakeSpan<1>(EF_B10G11R11F));
			}
			if (glloader_GLES_VERSION_3_2() || glloader_GLES_EXT_color_buffer_float())
			{
				add_render_target_format(
					MakeSpan({
						EF_R32F,
						EF_GR32F,
						EF_ABGR32F
					}));
			}

			caps_.AssignRenderTargetFormats(std::move(render_target_formats));
		}
	}
}
