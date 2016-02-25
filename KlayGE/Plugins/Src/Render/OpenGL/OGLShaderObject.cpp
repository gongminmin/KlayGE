// OGLShaderObject.cpp
// KlayGE OpenGL shader对象类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2006-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Geometry shader to GLSL compiler works (2010.8.16)
// Reuse generated GLSL between passes (2010.9.30)
//
// 3.9.0
// Cg载入后编译成GLSL使用 (2009.4.26)
//
// 3.7.0
// 改为直接传入RenderEffect (2008.7.4)
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/ResIdentifier.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KFL/Matrix.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <cstdio>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>

#include <glloader/glloader.h>

#include <DXBC2GLSL/DXBC2GLSL.hpp>

#ifndef D3DCOMPILE_SKIP_OPTIMIZATION
#define D3DCOMPILE_SKIP_OPTIMIZATION 0x00000004
#endif
#ifndef D3DCOMPILE_PREFER_FLOW_CONTROL
#define D3DCOMPILE_PREFER_FLOW_CONTROL 0x00000400
#endif

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactoryInternal.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

namespace
{
	using namespace KlayGE;

	template <typename SrcType>
	class SetOGLShaderParameter
	{
	};

	template <>
	class SetOGLShaderParameter<bool>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			bool v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1i(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<uint32_t>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint32_t v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1ui(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int32_t>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int32_t v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1i(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1f(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<uint2>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint2 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform2uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<uint3>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint3 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform3uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<uint4>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint4 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform4uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int2>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int2 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform2iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int3>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int3 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform3iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int4>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int4 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform4iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float2>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float2 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform2fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float3>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float3 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform3fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float4>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float4 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform4fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float4x4>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.UniformMatrix4fv(location_, 1, true, &v[0]);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<bool*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<bool> v;
			param_->Value(v);

			if (!v.empty())
			{
				std::vector<int> tmp(v.begin(), v.end());
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform1iv(location_, static_cast<int>(tmp.size()), &tmp[0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<uint32_t*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint32_t> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform1uiv(location_, static_cast<int>(v.size()), reinterpret_cast<GLuint*>(&v[0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int32_t*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int32_t> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform1iv(location_, static_cast<int>(v.size()), reinterpret_cast<GLint*>(&v[0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform1fv(location_, static_cast<int>(v.size()), &v[0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<uint2*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint2> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform2uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<uint3*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint3> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform3uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<uint4*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint4> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform4uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int2*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int2> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform2iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int3*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int3> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform3iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<int4*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int4> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform4iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float2*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float2> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform2fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float3*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float3> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform3fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float4*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform4fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<float4x4*>
	{
	public:
		SetOGLShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4x4> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.UniformMatrix4fv(location_, static_cast<GLsizei>(v.size()), false, &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>>
	{
	public:
		SetOGLShaderParameter(std::vector<std::pair<TexturePtr, SamplerStateObjectPtr>>& samplers,
					std::vector<GLuint>& gl_bind_targets, std::vector<GLuint>& gl_bind_textures,
					GLint location, GLuint stage,
					RenderEffectParameterPtr const & tex_param, RenderEffectParameterPtr const & sampler_param)
			: samplers_(&samplers), gl_bind_targets_(&gl_bind_targets), gl_bind_textures_(&gl_bind_textures),
				location_(location), stage_(stage), tex_param_(tex_param), sampler_param_(sampler_param)
		{
		}

		void operator()()
		{
			tex_param_->Value((*samplers_)[stage_].first);
			sampler_param_->Value((*samplers_)[stage_].second);

			if ((*samplers_)[stage_].first)
			{
				checked_pointer_cast<OGLSamplerStateObject>((*samplers_)[stage_].second)->Active((*samplers_)[stage_].first);
				(*gl_bind_targets_)[stage_] = checked_pointer_cast<OGLTexture>((*samplers_)[stage_].first)->GLType();
				(*gl_bind_textures_)[stage_] = checked_pointer_cast<OGLTexture>((*samplers_)[stage_].first)->GLTexture();
			}
			else
			{
				(*gl_bind_targets_)[stage_] = GL_TEXTURE_2D;
				(*gl_bind_textures_)[stage_] = 0;
			}

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1i(location_, stage_);
		}

	private:
		std::vector<std::pair<TexturePtr, SamplerStateObjectPtr>>* samplers_;
		std::vector<GLuint>* gl_bind_targets_;
		std::vector<GLuint>* gl_bind_textures_;
		GLint location_;
		GLuint stage_;
		RenderEffectParameterPtr tex_param_;
		RenderEffectParameterPtr sampler_param_;
	};
}

namespace KlayGE
{
	OGLShaderObject::OGLShaderObject()
		: gs_input_type_(0), gs_output_type_(0), gs_max_output_vertex_(0),
			ds_partitioning_(STP_Undefined), ds_output_primitive_(STOP_Undefined)
	{
		has_discard_ = false;
		has_tessellation_ = false;
		is_shader_validate_.fill(true);

		glsl_program_ = glCreateProgram();

		shader_func_names_ = MakeSharedPtr<std::array<std::string, ST_NumShaderTypes>>();
		glsl_srcs_ = MakeSharedPtr<std::array<std::shared_ptr<std::string>, ST_NumShaderTypes>>();

		pnames_ = MakeSharedPtr<std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes>>();
		glsl_res_names_ = MakeSharedPtr<std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes>>();

		vs_usages_ = MakeSharedPtr<std::vector<VertexElementUsage>>();
		vs_usage_indices_ = MakeSharedPtr<std::vector<uint8_t>>();
		glsl_vs_attrib_names_ = MakeSharedPtr<std::vector<std::string>>();
	}

	OGLShaderObject::~OGLShaderObject()
	{
		glDeleteProgram(glsl_program_);
	}

	bool OGLShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
			std::vector<uint8_t> const & native_shader_block)
	{
		bool ret = false;

		(*shader_func_names_)[type] = effect.GetShaderDesc(shader_desc_ids[type]).func_name;

		is_shader_validate_[type] = false;
		if (native_shader_block.size() >= 24)
		{
			uint8_t const * nsbp = &native_shader_block[0];
			
			is_shader_validate_[type] = true;

			uint32_t len32;
			std::memcpy(&len32, nsbp, sizeof(len32));
			nsbp += sizeof(len32);
			len32 = LE2Native(len32);
			(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(len32, '\0');
			std::memcpy(&(*(*glsl_srcs_)[type])[0], nsbp, len32);
			nsbp += len32;

			uint16_t num16;
			std::memcpy(&num16, nsbp, sizeof(num16));
			nsbp += sizeof(num16);
			num16 = LE2Native(num16);
			(*pnames_)[type] = MakeSharedPtr<std::vector<std::string>>(num16);
			for (size_t i = 0; i < num16; ++ i)
			{
				uint8_t len8;
				std::memcpy(&len8, nsbp, sizeof(len8));
				nsbp += sizeof(len8);
											
				(*(*pnames_)[type])[i].resize(len8);
				std::memcpy(&(*(*pnames_)[type])[i][0], nsbp, len8);
				nsbp += len8;
			}

			std::memcpy(&num16, nsbp, sizeof(num16));
			nsbp += sizeof(num16);
			num16 = LE2Native(num16);
			(*glsl_res_names_)[type] = MakeSharedPtr<std::vector<std::string>>(num16);
			for (size_t i = 0; i < num16; ++ i)
			{
				uint8_t len8;
				std::memcpy(&len8, nsbp, sizeof(len8));
				nsbp += sizeof(len8);

				(*(*glsl_res_names_)[type])[i].resize(len8);
				std::memcpy(&(*(*glsl_res_names_)[type])[i][0], nsbp, len8);
				nsbp += len8;
			}

			std::memcpy(&num16, nsbp, sizeof(num16));
			nsbp += sizeof(num16);
			num16 = LE2Native(num16);
			for (size_t i = 0; i < num16; ++ i)
			{
				uint8_t len8;
				std::memcpy(&len8, nsbp, sizeof(len8));
				nsbp += sizeof(len8);

				std::string tex_name;
				tex_name.resize(len8);
				std::memcpy(&tex_name[0], nsbp, len8);
				nsbp += len8;

				std::memcpy(&len8, nsbp, sizeof(len8));
				nsbp += sizeof(len8);

				std::string sampler_name;
				sampler_name.resize(len8);
				std::memcpy(&sampler_name[0], nsbp, len8);
				nsbp += len8;

				std::string combined_sampler_name = tex_name + "_" + sampler_name;

				bool found = false;
				for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
				{
					if (std::get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
					{
						std::get<3>(tex_sampler_binds_[k]) |= 1UL << type;
						found = true;
						break;
					}
				}
				if (!found)
				{
					tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
						effect.ParameterByName(tex_name), effect.ParameterByName(sampler_name), 1UL << type));
				}
			}

			if (ST_VertexShader == type)
			{
				uint8_t num8;
				std::memcpy(&num8, nsbp, sizeof(num8));
				nsbp += sizeof(num8);
				vs_usages_->resize(num8);
				for (size_t i = 0; i < num8; ++ i)
				{
					uint8_t veu;
					std::memcpy(&veu, nsbp, sizeof(veu));
					nsbp += sizeof(veu);

					(*vs_usages_)[i] = static_cast<VertexElementUsage>(veu);
				}

				std::memcpy(&num8, nsbp, sizeof(num8));
				nsbp += sizeof(num8);
				if (num8 > 0)
				{
					vs_usage_indices_->resize(num8);
					std::memcpy(&(*vs_usage_indices_)[0], nsbp, num8 * sizeof((*vs_usage_indices_)[0]));
					nsbp += num8 * sizeof((*vs_usage_indices_)[0]);
				}

				std::memcpy(&num8, nsbp, sizeof(num8));
				nsbp += sizeof(num8);
				glsl_vs_attrib_names_->resize(num8);
				for (size_t i = 0; i < num8; ++ i)
				{
					uint8_t len8;
					std::memcpy(&len8, nsbp, sizeof(len8));
					nsbp += sizeof(len8);

					(*glsl_vs_attrib_names_)[i].resize(len8);
					std::memcpy(&(*glsl_vs_attrib_names_)[i][0], nsbp, len8);
					nsbp += len8;
				}
			}
			else if (ST_GeometryShader == type)
			{
				std::memcpy(&gs_input_type_, nsbp, sizeof(gs_input_type_));
				nsbp += sizeof(gs_input_type_);
				gs_input_type_ = LE2Native(gs_input_type_);

				std::memcpy(&gs_output_type_, nsbp, sizeof(gs_output_type_));
				nsbp += sizeof(gs_output_type_);
				gs_output_type_ = LE2Native(gs_output_type_);

				std::memcpy(&gs_max_output_vertex_, nsbp, sizeof(gs_max_output_vertex_));
				nsbp += sizeof(gs_max_output_vertex_);
				gs_max_output_vertex_ = LE2Native(gs_max_output_vertex_);
			}

			this->AttachGLSL(type);

			ret = is_shader_validate_[type];
		}

		return ret;
	}

	bool OGLShaderObject::StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
		std::vector<uint32_t> const & shader_desc_ids)
	{
		uint32_t len;
		res->read(&len, sizeof(len));
		len = LE2Native(len);
		std::vector<uint8_t> native_shader_block(len);
		if (len > 0)
		{
			res->read(&native_shader_block[0], len * sizeof(native_shader_block[0]));
		}

		return this->AttachNativeShader(type, effect, shader_desc_ids, native_shader_block);
	}

	void OGLShaderObject::StreamOut(std::ostream& os, ShaderType type)
	{
		std::vector<uint8_t> native_shader_block;

		if ((*glsl_srcs_)[type])
		{
			std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

			uint32_t len32 = Native2LE(static_cast<uint32_t>((*glsl_srcs_)[type]->size()));
			oss.write(reinterpret_cast<char const *>(&len32), sizeof(len32));
			oss.write(&(*(*glsl_srcs_)[type])[0], (*glsl_srcs_)[type]->size());

			uint16_t num16 = Native2LE(static_cast<uint16_t>((*pnames_)[type]->size()));
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < (*pnames_)[type]->size(); ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>((*(*pnames_)[type])[i].size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&(*(*pnames_)[type])[i][0], (*(*pnames_)[type])[i].size());
			}

			num16 = Native2LE(static_cast<uint16_t>((*glsl_res_names_)[type]->size()));
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < (*glsl_res_names_)[type]->size(); ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>((*(*glsl_res_names_)[type])[i].size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&(*(*glsl_res_names_)[type])[i][0], (*(*glsl_res_names_)[type])[i].size());
			}

			std::vector<std::pair<std::string, std::string>> tex_sampler_pairs;
			for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
			{
				if (std::get<3>(tex_sampler_binds_[i]) | (1UL << type))
				{
					tex_sampler_pairs.emplace_back(std::get<1>(tex_sampler_binds_[i])->Name(),
						std::get<2>(tex_sampler_binds_[i])->Name());
				}
			}

			num16 = Native2LE(static_cast<uint16_t>(tex_sampler_pairs.size()));
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < num16; ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>(tex_sampler_pairs[i].first.size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&tex_sampler_pairs[i].first[0], len8);

				len8 = static_cast<uint8_t>(tex_sampler_pairs[i].second.size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&tex_sampler_pairs[i].second[0], len8);
			}

			if (ST_VertexShader == type)
			{
				uint8_t num8 = static_cast<uint8_t>(vs_usages_->size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				for (size_t i = 0; i < vs_usages_->size(); ++ i)
				{
					uint8_t veu = static_cast<uint8_t>((*vs_usages_)[i]);
					oss.write(reinterpret_cast<char const *>(&veu), sizeof(veu));
				}

				num8 = static_cast<uint8_t>(vs_usage_indices_->size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				if (!vs_usage_indices_->empty())
				{
					oss.write(reinterpret_cast<char const *>(&(*vs_usage_indices_)[0]), vs_usage_indices_->size() * sizeof((*vs_usage_indices_)[0]));
				}

				num8 = static_cast<uint8_t>(glsl_vs_attrib_names_->size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				for (size_t i = 0; i < glsl_vs_attrib_names_->size(); ++ i)
				{
					uint8_t len8 = static_cast<uint8_t>((*glsl_vs_attrib_names_)[i].size());
					oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
					oss.write(&(*glsl_vs_attrib_names_)[i][0], (*glsl_vs_attrib_names_)[i].size());
				}
			}
			else if (ST_GeometryShader == type)
			{
				uint32_t git = Native2LE(gs_input_type_);
				oss.write(reinterpret_cast<char const *>(&git), sizeof(git));

				uint32_t got = Native2LE(gs_output_type_);
				oss.write(reinterpret_cast<char const *>(&got), sizeof(got));

				uint32_t gmov = Native2LE(gs_max_output_vertex_);
				oss.write(reinterpret_cast<char const *>(&gmov), sizeof(gmov));
			}

			std::string out_str = oss.str();
			native_shader_block.resize(out_str.size());
			std::memcpy(&native_shader_block[0], &out_str[0], out_str.size());
		}

		uint32_t len = static_cast<uint32_t>(native_shader_block.size());
		{
			uint32_t tmp = Native2LE(len);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
		if (len > 0)
		{
			os.write(reinterpret_cast<char const *>(&native_shader_block[0]), len * sizeof(native_shader_block[0]));
		}
	}

	void OGLShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids)
	{
		ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		(*shader_func_names_)[type] = sd.func_name;

		bool has_gs = false;
		if (!effect.GetShaderDesc(shader_desc_ids[ST_GeometryShader]).func_name.empty())
		{
			has_gs = true;
		}

		is_shader_validate_[type] = true;
		switch (type)
		{
		case ST_VertexShader:
		case ST_PixelShader:
		case ST_GeometryShader:
		case ST_HullShader:
		case ST_DomainShader:
			break;

		default:
			is_shader_validate_[type] = false;
			break;
		}

		if (is_shader_validate_[type])
		{
			OGLRenderEngine const & re = *checked_cast<OGLRenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			RenderDeviceCaps const & caps = re.DeviceCaps();

			is_shader_validate_[type] = true;

			char const * shader_profile = sd.profile.c_str();
			size_t const shader_profile_hash = RT_HASH(shader_profile);
			switch (type)
			{
			case ST_VertexShader:
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = "vs_5_0";
				}
				break;

			case ST_PixelShader:
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = "ps_5_0";
				}
				break;

			case ST_GeometryShader:
				if (caps.gs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "gs_5_0";
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_ComputeShader:
				if (caps.cs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "cs_5_0";
					}
					if ((CT_HASH("cs_5_0") == shader_profile_hash) && (caps.max_shader_model < ShaderModel(5, 0)))
					{
						is_shader_validate_[type] = false;
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_HullShader:
				if (caps.hs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "hs_5_0";
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_DomainShader:
				if (caps.ds_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "ds_5_0";
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			default:
				is_shader_validate_[type] = false;
				break;
			}

			std::vector<uint8_t> code;
			if (is_shader_validate_[type])
			{
				std::string err_msg;
				std::vector<std::pair<char const *, char const *>> macros;
				macros.emplace_back("KLAYGE_DXBC2GLSL", "1");
				macros.emplace_back("KLAYGE_OPENGL", "1");
				if (!caps.texture_format_support(EF_BC5) || !caps.texture_format_support(EF_BC5_SRGB))
				{
					macros.emplace_back("KLAYGE_BC5_AS_AG", "1");
				}
				if (!caps.texture_format_support(EF_BC4) || !caps.texture_format_support(EF_BC4_SRGB))
				{
					macros.emplace_back("KLAYGE_BC4_AS_G", "1");
				}
				macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");

				uint32_t const flags = D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
				code = this->CompileToDXBC(type, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile, flags);
				if (code.empty())
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					try
					{
						GLSLVersion gsv;
						if (glloader_GL_VERSION_4_5())
						{
							gsv = GSV_450;
						}
						else if (glloader_GL_VERSION_4_4())
						{
							gsv = GSV_440;
						}
						else if (glloader_GL_VERSION_4_3())
						{
							gsv = GSV_430;
						}
						else if (glloader_GL_VERSION_4_2())
						{
							gsv = GSV_420;
						}
						else if (glloader_GL_VERSION_4_1())
						{
							gsv = GSV_410;
						}
						else if (glloader_GL_VERSION_4_0())
						{
							gsv = GSV_400;
						}
						else if (glloader_GL_VERSION_3_3())
						{
							gsv = GSV_330;
						}
						else if (glloader_GL_VERSION_3_2())
						{
							gsv = GSV_150;
						}
						else if (glloader_GL_VERSION_3_1())
						{
							gsv = GSV_140;
						}
						else if (glloader_GL_VERSION_3_0())
						{
							gsv = GSV_130;
						}
						else
						{
							gsv = GSV_120;
						}

						DXBC2GLSL::DXBC2GLSL dxbc2glsl;
						uint32_t rules = DXBC2GLSL::DXBC2GLSL::DefaultRules(gsv);
						rules &= ~GSR_UniformBlockBinding;
						dxbc2glsl.FeedDXBC(&code[0],
							has_gs, static_cast<ShaderTessellatorPartitioning>(ds_partitioning_),
							static_cast<ShaderTessellatorOutputPrimitive>(ds_output_primitive_),
							gsv, rules);
						(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(dxbc2glsl.GLSLString());
						(*pnames_)[type] = MakeSharedPtr<std::vector<std::string>>();
						(*glsl_res_names_)[type] = MakeSharedPtr<std::vector<std::string>>();

						for (uint32_t i = 0; i < dxbc2glsl.NumCBuffers(); ++ i)
						{
							for (uint32_t j = 0; j < dxbc2glsl.NumVariables(i); ++ j)
							{
								if (dxbc2glsl.VariableUsed(i, j))
								{
									(*pnames_)[type]->push_back(dxbc2glsl.VariableName(i, j));
									(*glsl_res_names_)[type]->push_back(dxbc2glsl.VariableName(i, j));
								}
							}
						}

						std::vector<char const *> tex_names;
						std::vector<char const *> sampler_names;
						for (uint32_t i = 0; i < dxbc2glsl.NumResources(); ++ i)
						{
							if (dxbc2glsl.ResourceUsed(i))
							{
								if (SIT_TEXTURE == dxbc2glsl.ResourceType(i))
								{
									tex_names.push_back(dxbc2glsl.ResourceName(i));
								}
								else if (SIT_SAMPLER == dxbc2glsl.ResourceType(i))
								{
									sampler_names.push_back(dxbc2glsl.ResourceName(i));
								}
							}
						}

						for (size_t i = 0; i < tex_names.size(); ++ i)
						{
							RenderEffectParameterPtr const & param = effect.ParameterByName(tex_names[i]);
							for (size_t j = 0; j < sampler_names.size(); ++ j)
							{
								std::string combined_sampler_name = std::string(tex_names[i]) + "_" + sampler_names[j];
								bool found = false;
								for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
								{
									if (std::get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
									{
										std::get<3>(tex_sampler_binds_[k]) |= 1UL << type;
										found = true;
										break;
									}
								}
								if (!found)
								{
									tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
										param, effect.ParameterByName(sampler_names[j]), 1UL << type));
								}

								(*pnames_)[type]->push_back(combined_sampler_name);
								(*glsl_res_names_)[type]->push_back(combined_sampler_name);
							}
						}

						if (ST_VertexShader == type)
						{
							for (uint32_t i = 0; i < dxbc2glsl.NumInputParams(); ++ i)
							{
								if (dxbc2glsl.InputParam(i).mask != 0)
								{
									std::string semantic = dxbc2glsl.InputParam(i).semantic_name;
									uint32_t semantic_index = dxbc2glsl.InputParam(i).semantic_index;
									std::string glsl_param_name = semantic;
									size_t const semantic_hash = RT_HASH(semantic.c_str());

									if ((CT_HASH("SV_VertexID") != semantic_hash)
										&& (CT_HASH("SV_InstanceID") != semantic_hash))
									{
										VertexElementUsage usage = VEU_Position;
										uint8_t usage_index = 0;
										if (CT_HASH("POSITION") == semantic_hash)
										{
											usage = VEU_Position;
											glsl_param_name = "POSITION0";
										}
										else if (CT_HASH("NORMAL") == semantic_hash)
										{
											usage = VEU_Normal;
											glsl_param_name = "NORMAL0";
										}
										else if (CT_HASH("COLOR") == semantic_hash)
										{
											if (0 == semantic_index)
											{
												usage = VEU_Diffuse;
												glsl_param_name = "COLOR0";
											}
											else
											{
												usage = VEU_Specular;
												glsl_param_name = "COLOR1";
											}
										}
										else if (CT_HASH("BLENDWEIGHT") == semantic_hash)
										{
											usage = VEU_BlendWeight;
											glsl_param_name = "BLENDWEIGHT0";
										}
										else if (CT_HASH("BLENDINDICES") == semantic_hash)
										{
											usage = VEU_BlendIndex;
											glsl_param_name = "BLENDINDICES0";
										}
										else if (0 == semantic.find("TEXCOORD"))
										{
											usage = VEU_TextureCoord;
											usage_index = static_cast<uint8_t>(semantic_index);
											glsl_param_name = "TEXCOORD" + boost::lexical_cast<std::string>(semantic_index);
										}
										else if (CT_HASH("TANGENT") == semantic_hash)
										{
											usage = VEU_Tangent;
											glsl_param_name = "TANGENT0";
										}
										else if (CT_HASH("BINORMAL") == semantic_hash)
										{
											usage = VEU_Binormal;
											glsl_param_name = "BINORMAL0";
										}
										else
										{
											BOOST_ASSERT(false);
											usage = VEU_Position;
											glsl_param_name = "POSITION0";
										}

										vs_usages_->push_back(usage);
										vs_usage_indices_->push_back(usage_index);
										glsl_vs_attrib_names_->push_back(glsl_param_name);
									}
								}
							}
						}
						else if (ST_GeometryShader == type)
						{
							switch (dxbc2glsl.GSInputPrimitive())
							{
							case SP_Point:
								gs_input_type_ = GL_POINTS;
								break;

							case SP_Line:
								gs_input_type_ = GL_LINES;
								break;

							case SP_LineAdj:
								gs_input_type_ = GL_LINES_ADJACENCY_EXT;
								break;

							case SP_Triangle:
								gs_input_type_ = GL_TRIANGLES;
								break;

							case SP_TriangleAdj:
								gs_input_type_ = GL_TRIANGLES_ADJACENCY_EXT;
								break;

							default:
								BOOST_ASSERT(false);
								gs_input_type_ = 0;
								break;
							}

							switch (dxbc2glsl.GSOutputTopology(0))
							{
							case SPT_PointList:
								gs_output_type_ = GL_POINTS;
								break;

							case SPT_LineStrip:
								gs_output_type_ = GL_LINE_STRIP;
								break;

							case SPT_TriangleStrip:
								gs_output_type_ = GL_TRIANGLE_STRIP;
								break;

							default:
								BOOST_ASSERT(false);
								gs_output_type_ = 0;
								break;
							}

							gs_max_output_vertex_ = dxbc2glsl.MaxGSOutputVertex();
						}
						else if (ST_HullShader == type)
						{
							ds_partitioning_ = dxbc2glsl.DSPartitioning();
							ds_output_primitive_ = dxbc2glsl.DSOutputPrimitive();
						}
					}
					catch (std::exception& ex)
					{
						is_shader_validate_[type] = false;

						LogError("Error(s) in conversion: %s/%s/%s", tech.Name().c_str(), pass.Name().c_str(), sd.func_name.c_str());
						LogError(ex.what());
						LogError("Please send this information and your shader to webmaster at klayge.org. We'll fix this ASAP.");
					}
				}
			}
		}

		if (is_shader_validate_[type])
		{
			this->AttachGLSL(type);
		}
	}

	void OGLShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/,
			RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & shared_so)
	{
		OGLShaderObjectPtr so = checked_pointer_cast<OGLShaderObject>(shared_so);

		is_shader_validate_[type] = so->is_shader_validate_[type];
		(*shader_func_names_)[type] = (*so->shader_func_names_)[type];

		if (is_shader_validate_[type])
		{
			(*glsl_srcs_)[type] = (*so->glsl_srcs_)[type];

			(*pnames_)[type] = (*so->pnames_)[type];
			(*glsl_res_names_)[type] = (*so->glsl_res_names_)[type];
			if (ST_VertexShader == type)
			{
				*vs_usages_ = *so->vs_usages_;
				*vs_usage_indices_ = *so->vs_usage_indices_;
				*glsl_vs_attrib_names_ = *so->glsl_vs_attrib_names_;
			}
			else if (ST_GeometryShader == type)
			{
				gs_input_type_ = so->gs_input_type_;
				gs_output_type_ = so->gs_output_type_;
				gs_max_output_vertex_ = so->gs_max_output_vertex_;
			}
			else if (ST_PixelShader == type)
			{
				has_discard_ = so->has_discard_;
			}
			else if (ST_HullShader == type)
			{
				ds_partitioning_ = so->ds_partitioning_;
				ds_output_primitive_ = so->ds_output_primitive_;
			}

			for (uint32_t j = 0; j < so->tex_sampler_binds_.size(); ++ j)
			{
				if (std::get<3>(so->tex_sampler_binds_[j]) | (1UL << type))
				{
					std::string const & combined_sampler_name = std::get<0>(so->tex_sampler_binds_[j]);
					bool found = false;
					for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
					{
						if (std::get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
						{
							std::get<3>(tex_sampler_binds_[k]) |= 1UL << type;
							found = true;
							break;
						}
					}
					if (!found)
					{
						tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
							std::get<1>(so->tex_sampler_binds_[j]), std::get<2>(so->tex_sampler_binds_[j]), 1UL << type));
					}
				}
			}

			this->AttachGLSL(type);
		}
	}

	void OGLShaderObject::LinkShaders(RenderEffect const & effect)
	{
		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			if (!(*shader_func_names_)[type].empty())
			{
				is_validate_ &= is_shader_validate_[type];
			}
		}

		if (is_validate_)
		{
			if (glloader_GL_VERSION_4_1() || glloader_GL_ARB_get_program_binary())
			{
				glProgramParameteri(glsl_program_, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
			}

			this->LinkGLSL();
			this->AttachUBOs(effect);

			if (is_validate_ && (glloader_GL_VERSION_4_1() || glloader_GL_ARB_get_program_binary()))
			{
				GLint num = 0;
				glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num);
				if (num > 0)
				{
					GLint len = 0;
					glGetProgramiv(glsl_program_, GL_PROGRAM_BINARY_LENGTH, &len);
					glsl_bin_program_ = MakeSharedPtr<std::vector<uint8_t>>(len);
					glGetProgramBinary(glsl_program_, len, nullptr, &glsl_bin_format_, &(*glsl_bin_program_)[0]);
				}
			}

			for (int type = 0; type < ST_NumShaderTypes; ++ type)
			{
				if ((*pnames_)[type])
				{
					for (size_t pi = 0; pi < (*pnames_)[type]->size(); ++ pi)
					{
						GLint location = glGetUniformLocation(glsl_program_, (*(*glsl_res_names_)[type])[pi].c_str());
						if (location != -1)
						{
							RenderEffectParameterPtr const & p = effect.ParameterByName((*(*pnames_)[type])[pi]);
							if (p)
							{
								param_binds_.push_back(this->GetBindFunc(location, p));
							}
							else
							{
								for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
								{
									if (std::get<0>(tex_sampler_binds_[i]) == (*(*pnames_)[type])[pi])
									{
										parameter_bind_t pb;
										pb.combined_sampler_name = std::get<0>(tex_sampler_binds_[i]);
										pb.location = location;
										pb.shader_type = type;
										pb.tex_sampler_bind_index = static_cast<int>(i);

										uint32_t index = static_cast<uint32_t>(samplers_.size());
										samplers_.resize(index + 1);
										gl_bind_targets_.resize(index + 1);
										gl_bind_textures_.resize(index + 1);

										pb.func = SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>>(samplers_,
											gl_bind_targets_, gl_bind_textures_,
											location, index, std::get<1>(tex_sampler_binds_[i]), std::get<2>(tex_sampler_binds_[i]));

										param_binds_.push_back(pb);

										break;
									}
								}
							}
						}
					}
				}

				if (ST_VertexShader == type)
				{
					for (size_t pi = 0; pi < glsl_vs_attrib_names_->size(); ++ pi)
					{
						attrib_locs_.emplace(std::make_pair((*vs_usages_)[pi], (*vs_usage_indices_)[pi]),
								glGetAttribLocation(glsl_program_, (*glsl_vs_attrib_names_)[pi].c_str()));
					}
				}
			}
		}
	}

	ShaderObjectPtr OGLShaderObject::Clone(RenderEffect const & effect)
	{
		OGLShaderObjectPtr ret = MakeSharedPtr<OGLShaderObject>();

		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;
		ret->glsl_bin_format_ = glsl_bin_format_;
		ret->glsl_bin_program_ = glsl_bin_program_;
		ret->shader_func_names_ = shader_func_names_;
		ret->glsl_srcs_ = glsl_srcs_;
		ret->pnames_ = pnames_;
		ret->glsl_res_names_ = glsl_res_names_;
		ret->vs_usages_ = vs_usages_;
		ret->vs_usage_indices_ = vs_usage_indices_;
		ret->glsl_vs_attrib_names_ = glsl_vs_attrib_names_;
		ret->gs_input_type_ = gs_input_type_;
		ret->gs_output_type_ = gs_output_type_;
		ret->gs_max_output_vertex_ = gs_max_output_vertex_;
		ret->ds_partitioning_ = ds_partitioning_;
		ret->ds_output_primitive_ = ds_output_primitive_;

		ret->tex_sampler_binds_.resize(tex_sampler_binds_.size());
		for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			std::get<0>(ret->tex_sampler_binds_[i]) = std::get<0>(tex_sampler_binds_[i]);
			std::get<1>(ret->tex_sampler_binds_[i]) = effect.ParameterByName(std::get<1>(tex_sampler_binds_[i])->Name());
			std::get<2>(ret->tex_sampler_binds_[i]) = effect.ParameterByName(std::get<2>(tex_sampler_binds_[i])->Name());
			std::get<3>(ret->tex_sampler_binds_[i]) = std::get<3>(tex_sampler_binds_[i]);
		}

		if ((glloader_GL_VERSION_4_1() || glloader_GL_ARB_get_program_binary()) && glsl_bin_program_)
		{
			ret->is_validate_ = is_validate_;
			for (size_t type = 0; type < ST_NumShaderTypes; ++ type)
			{
				ret->is_shader_validate_[type] = is_shader_validate_[type];
			}

			if (ret->is_validate_)
			{
				glProgramParameteri(ret->glsl_program_, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);

				glProgramBinary(ret->glsl_program_, glsl_bin_format_,
					&(*glsl_bin_program_)[0], static_cast<GLsizei>(glsl_bin_program_->size()));

				GLint linked = false;
				glGetProgramiv(ret->glsl_program_, GL_LINK_STATUS, &linked);
#ifdef KLAYGE_DEBUG
				if (!linked)
				{
					GLint len = 0;
					glGetProgramiv(ret->glsl_program_, GL_INFO_LOG_LENGTH, &len);
					if (len > 0)
					{
						std::vector<char> info(len + 1, 0);
						glGetProgramInfoLog(ret->glsl_program_, len, &len, &info[0]);
						LogError(&info[0]);
					}
				}
#endif
				ret->is_validate_ &= linked ? true : false;
			}
		}
		else
		{
			ret->is_validate_ = true;
			for (size_t type = 0; type < ST_NumShaderTypes; ++ type)
			{
				ret->is_shader_validate_[type] = is_shader_validate_[type];

				if (is_shader_validate_[type])
				{
					if ((*glsl_srcs_)[type] && !(*glsl_srcs_)[type]->empty())
					{
						ret->AttachGLSL(static_cast<uint32_t>(type));
					}
				}

				ret->is_validate_ &= ret->is_shader_validate_[type];
			}

			if (ret->is_validate_)
			{
				ret->LinkGLSL();
			}
		}

		if (ret->is_validate_)
		{
			ret->AttachUBOs(effect);

			ret->attrib_locs_ = attrib_locs_;

			for (auto const & pb : param_binds_)
			{
				if (pb.param)
				{
					RenderEffectParameterPtr const & p = effect.ParameterByName(pb.param->Name());
					ret->param_binds_.push_back(ret->GetBindFunc(pb.location, p));
				}
				else
				{
					std::string const & pname = pb.combined_sampler_name;
					for (size_t j = 0; j < ret->tex_sampler_binds_.size(); ++ j)
					{
						if (std::get<0>(ret->tex_sampler_binds_[j]) == pname)
						{
							parameter_bind_t new_pb;
							new_pb.combined_sampler_name = pname;
							new_pb.location = pb.location;
							new_pb.shader_type = pb.shader_type;
							new_pb.tex_sampler_bind_index = pb.tex_sampler_bind_index;

							uint32_t index = static_cast<uint32_t>(ret->samplers_.size());
							ret->samplers_.resize(index + 1);
							ret->gl_bind_targets_.resize(index + 1);
							ret->gl_bind_textures_.resize(index + 1);

							new_pb.func = SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>>(ret->samplers_,
								ret->gl_bind_targets_, ret->gl_bind_textures_,
								new_pb.location, index,
								std::get<1>(ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index]),
								std::get<2>(ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index]));

							ret->param_binds_.push_back(new_pb);

							break;
						}
					}
				}
			}
		}

		return ret;
	}

	GLint OGLShaderObject::GetAttribLocation(VertexElementUsage usage, uint8_t usage_index)
	{
		auto iter = attrib_locs_.find(std::make_pair(usage, usage_index));
		if (iter != attrib_locs_.end())
		{
			return iter->second;
		}
		else
		{
			return -1;
		}
	}

	OGLShaderObject::parameter_bind_t OGLShaderObject::GetBindFunc(GLint location, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.location = location;

		switch (param->Type())
		{
		case REDT_bool:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<bool*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<bool>(location, param);
			}
			break;

		case REDT_uint:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<uint32_t*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<uint32_t>(location, param);
			}
			break;

		case REDT_int:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<int32_t*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<int32_t>(location, param);
			}
			break;

		case REDT_float:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<float*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<float>(location, param);
			}
			break;

		case REDT_uint2:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<uint2*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<uint2>(location, param);
			}
			break;

		case REDT_uint3:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<uint3*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<uint3>(location, param);
			}
			break;

		case REDT_uint4:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<uint4*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<uint4>(location, param);
			}
			break;

		case REDT_int2:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<int2*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<int2>(location, param);
			}
			break;

		case REDT_int3:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<int3*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<int3>(location, param);
			}
			break;

		case REDT_int4:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<int4*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<int4>(location, param);
			}
			break;

		case REDT_float2:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<float2*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<float2>(location, param);
			}
			break;

		case REDT_float3:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<float3*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<float3>(location, param);
			}
			break;

		case REDT_float4:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<float4*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<float4>(location, param);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize())
			{
				ret.func = SetOGLShaderParameter<float4x4*>(location, param);
			}
			else
			{
				ret.func = SetOGLShaderParameter<float4x4>(location, param);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void OGLShaderObject::AttachGLSL(uint32_t type)
	{
		GLenum shader_type;
		switch (type)
		{
		case ST_VertexShader:
			shader_type = GL_VERTEX_SHADER;
			break;

		case ST_PixelShader:
			shader_type = GL_FRAGMENT_SHADER;
			break;

		case ST_GeometryShader:
			shader_type = GL_GEOMETRY_SHADER_EXT;
			break;

		case ST_HullShader:
			shader_type = GL_TESS_CONTROL_SHADER;
			break;

		case ST_DomainShader:
			shader_type = GL_TESS_EVALUATION_SHADER;
			break;

		default:
			shader_type = 0;
			break;
		}

		char const * glsl = (*glsl_srcs_)[type]->c_str();
		GLuint object = glCreateShader(shader_type);
		if (0 == object)
		{
			is_shader_validate_[type] = false;
		}

		glShaderSource(object, 1, &glsl, nullptr);

		glCompileShader(object);

		GLint compiled = false;
		glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			LogError("Error when compiling GLSL %s:", (*shader_func_names_)[type].c_str());

			GLint len = 0;
			glGetShaderiv(object, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				std::vector<char> info(len + 1, 0);
				glGetShaderInfoLog(object, len, &len, &info[0]);
				this->PrintGLSLError(static_cast<ShaderType>(type), &info[0]);
			}
		}

		is_shader_validate_[type] &= compiled ? true : false;

		glAttachShader(glsl_program_, object);

		if (ST_GeometryShader == type)
		{
			if (!glloader_GL_VERSION_3_2())
			{
				if (glloader_GL_ARB_geometry_shader4())
				{
					glProgramParameteriARB(glsl_program_, GL_GEOMETRY_INPUT_TYPE_ARB, gs_input_type_);
					glProgramParameteriARB(glsl_program_, GL_GEOMETRY_OUTPUT_TYPE_ARB, gs_output_type_);
					glProgramParameteriARB(glsl_program_, GL_GEOMETRY_VERTICES_OUT_ARB, gs_max_output_vertex_);
				}
				else if (glloader_GL_EXT_geometry_shader4())
				{
					glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_INPUT_TYPE_EXT, gs_input_type_);
					glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_OUTPUT_TYPE_EXT, gs_output_type_);
					glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_VERTICES_OUT_EXT, gs_max_output_vertex_);
				}
			}
		}

		glDeleteShader(object);
	}

	void OGLShaderObject::LinkGLSL()
	{
		glLinkProgram(glsl_program_);

		GLint linked = false;
		glGetProgramiv(glsl_program_, GL_LINK_STATUS, &linked);
#ifdef KLAYGE_DEBUG
		if (!linked)
		{
			std::string shader_names;
			for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
			{
				if (!(*shader_func_names_)[type].empty())
				{
					shader_names += (*shader_func_names_)[type] + '/';
				}
			}
			if (!shader_names.empty())
			{
				shader_names.resize(shader_names.size() - 1);
			}

			LogError("Error when linking GLSLs %s:", shader_names.c_str());

			GLint len = 0;
			glGetProgramiv(glsl_program_, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				std::vector<char> info(len + 1, 0);
				glGetProgramInfoLog(glsl_program_, len, &len, &info[0]);

				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				if (re.HackForNV())
				{
					std::istringstream err_iss(&info[0]);
					std::string type1, type2;
					err_iss >> type1 >> type2;
					if ("Vertex" == type1)
					{
						this->PrintGLSLError(ST_VertexShader, &info[0]);
					}
					else if ("Geometry" == type1)
					{
						this->PrintGLSLError(ST_GeometryShader, &info[0]);
					}
					else if ("Fragment" == type1)
					{
						this->PrintGLSLError(ST_PixelShader, &info[0]);
					}
					else if ("Tessellation" == type1)
					{
						if ("control" == type2)
						{
							this->PrintGLSLError(ST_HullShader, &info[0]);
						}
						else
						{
							this->PrintGLSLError(ST_DomainShader, &info[0]);
						}
					}
					else
					{
						LogError(&info[0]);
					}
				}
				else
				{
					LogError(&info[0]);
				}
			}
		}
#endif
		is_validate_ &= linked ? true : false;
	}

	void OGLShaderObject::AttachUBOs(RenderEffect const & effect)
	{
		if (glloader_GL_VERSION_3_1() || glloader_GL_ARB_uniform_buffer_object())
		{
			GLint active_ubos;
			glGetProgramiv(glsl_program_, GL_ACTIVE_UNIFORM_BLOCKS, &active_ubos);
			all_cbuffs_.resize(active_ubos);
			gl_bind_cbuffs_.resize(active_ubos);
			for (int i = 0; i < active_ubos; ++ i)
			{
				GLint length = 0;
				glGetActiveUniformBlockiv(glsl_program_, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &length);

				std::vector<GLchar> ubo_name(length, '\0');
				glGetActiveUniformBlockName(glsl_program_, i, length, nullptr, &ubo_name[0]);

				RenderEffectConstantBufferPtr const & cbuff = effect.CBufferByName(&ubo_name[0]);
				BOOST_ASSERT(cbuff);
				all_cbuffs_[i] = cbuff;

				glUniformBlockBinding(glsl_program_, glGetUniformBlockIndex(glsl_program_, &ubo_name[0]), i);

				GLint ubo_size = 0;
				glGetActiveUniformBlockiv(glsl_program_, i, GL_UNIFORM_BLOCK_DATA_SIZE, &ubo_size);
				cbuff->Resize(ubo_size);
				gl_bind_cbuffs_[i] = checked_cast<OGLGraphicsBuffer*>(cbuff->HWBuff().get())->GLvbo();

				GLint uniforms = 0;
				glGetActiveUniformBlockiv(glsl_program_, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniforms);

				std::vector<GLuint> uniform_indices(uniforms);
				glGetActiveUniformBlockiv(glsl_program_, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
					reinterpret_cast<GLint*>(&uniform_indices[0]));

				std::vector<GLint> uniform_name_lens(uniforms);
				glGetActiveUniformsiv(glsl_program_, uniforms, &uniform_indices[0],
					GL_UNIFORM_NAME_LENGTH, &uniform_name_lens[0]);

				std::vector<GLint> uniform_array_strides(uniforms);
				glGetActiveUniformsiv(glsl_program_, uniforms, &uniform_indices[0],
					GL_UNIFORM_ARRAY_STRIDE, &uniform_array_strides[0]);

				std::vector<GLint> uniform_matrix_strides(uniforms);
				glGetActiveUniformsiv(glsl_program_, uniforms, &uniform_indices[0],
					GL_UNIFORM_MATRIX_STRIDE, &uniform_matrix_strides[0]);

				std::vector<GLint> uniform_offsets(uniforms);
				glGetActiveUniformsiv(glsl_program_, uniforms, &uniform_indices[0],
					GL_UNIFORM_OFFSET, &uniform_offsets[0]);

				for (GLint j = 0; j < uniforms; ++ j)
				{
					std::vector<GLchar> uniform_name(uniform_name_lens[j], '\0');
					GLint size;
					GLenum type;
					glGetActiveUniform(glsl_program_, uniform_indices[j], uniform_name_lens[j],
						nullptr, &size, &type, &uniform_name[0]);

					auto iter = std::find(uniform_name.begin(), uniform_name.end(), '[');
					if (iter != uniform_name.end())
					{
						*iter = '\0';
					}

					RenderEffectParameterPtr const & param = effect.ParameterByName(&uniform_name[0]);
					GLint stride;
					if (param->ArraySize())
					{
						stride = uniform_array_strides[j];
					}
					else
					{
						if (param->Type() != REDT_float4x4)
						{
							stride = 4;
						}
						else
						{
							stride = uniform_matrix_strides[j];
						}
					}
					param->BindToCBuffer(cbuff, uniform_offsets[j], stride);
				}
			}
		}
	}

	void OGLShaderObject::Bind()
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.UseProgram(glsl_program_);

		for (auto const & pb : param_binds_)
		{
			pb.func();
		}

		for (size_t i = 0; i < all_cbuffs_.size(); ++ i)
		{
			all_cbuffs_[i]->Update();
		}

		if (!gl_bind_cbuffs_.empty())
		{
			re.BindBuffersBase(GL_UNIFORM_BUFFER, 0, static_cast<GLsizei>(all_cbuffs_.size()), &gl_bind_cbuffs_[0]);
		}

		if (!gl_bind_textures_.empty())
		{
			re.BindTextures(0, static_cast<GLsizei>(gl_bind_textures_.size()), &gl_bind_targets_[0], &gl_bind_textures_[0]);
		}

#ifdef KLAYGE_DEBUG
		glValidateProgram(glsl_program_);

		GLint validated = false;
		glGetProgramiv(glsl_program_, GL_VALIDATE_STATUS, &validated);
		if (!validated)
		{
			GLint len = 0;
			glGetProgramiv(glsl_program_, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				std::vector<char> info(len + 1, 0);
				glGetProgramInfoLog(glsl_program_, len, &len, &info[0]);
				LogError(&info[0]);
			}
		}
#endif
	}

	void OGLShaderObject::Unbind()
	{
		//glUseProgram(0);
	}

	void OGLShaderObject::PrintGLSLError(ShaderType type, char const * info)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		std::string const & glsl = *(*glsl_srcs_)[type];

		if (re.HackForIntel())
		{
			std::istringstream err_iss(info);
			std::string err_str;
			while (err_iss)
			{
				std::getline(err_iss, err_str);
				if (!err_str.empty())
				{
					std::string::size_type pos = err_str.find(": 0:");
					if (pos != std::string::npos)
					{
						pos += 4;
						std::string::size_type pos2 = err_str.find(':', pos + 1);
						std::string part_err_str = err_str.substr(pos, pos2 - pos);
						this->PrintGLSLErrorAtLine(glsl, boost::lexical_cast<int>(part_err_str));
					}

					LogError(err_str.c_str());
					LogError("\n");
				}
			}
		}
		else if (re.HackForNV())
		{
			std::istringstream err_iss(info);
			std::string err_str;
			while (err_iss)
			{
				std::getline(err_iss, err_str);
				if (!err_str.empty())
				{
					std::string::size_type pos = err_str.find(") : error");
					if (pos != std::string::npos)
					{
						std::string::size_type pos2 = err_str.find('(') + 1;
						std::string part_err_str = err_str.substr(pos2, pos - pos2);
						this->PrintGLSLErrorAtLine(glsl, boost::lexical_cast<int>(part_err_str));
					}

					LogError(err_str.c_str());
					LogError("\n");
				}
			}
		}
		else
		{
			std::istringstream iss(glsl);
			std::string s;
			int line = 1;
			while (iss)
			{
				std::getline(iss, s);
				LogError("%d %s", line, s.c_str());
				++ line;
			}

			LogError(&info[0]);
			LogError("\n");
		}
	}

	void OGLShaderObject::PrintGLSLErrorAtLine(std::string const & glsl, int err_line)
	{
		std::istringstream iss(glsl);
		std::string s;
		int line = 1;
		LogError("...");
		while (iss)
		{
			std::getline(iss, s);
			if ((line - err_line > -3) && (line - err_line < 3))
			{
				LogError("%d %s", line, s.c_str());
			}
			++ line;
		}
		LogError("...");
	}
}
