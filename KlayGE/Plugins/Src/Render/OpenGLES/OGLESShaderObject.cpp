// OGLESShaderObject.cpp
// KlayGE OpenGL ES2 shader对象类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Reuse generated GLSL between passes (2010.9.30)
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/ResLoader.hpp>

#include <cstdio>
#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/assert.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/assign.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 6328)
#endif
#include <boost/tokenizer.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <glloader/glloader.h>

#ifndef KLAYGE_PLATFORM_ANDROID
#include <Cg/cg.h>
#endif

#include <KlayGE/OpenGLES/OGLESRenderFactory.hpp>
#include <KlayGE/OpenGLES/OGLESRenderFactoryInternal.hpp>
#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESRenderStateObject.hpp>
#include <KlayGE/OpenGLES/OGLESShaderObject.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "Cg.lib")
#endif

namespace
{
	using namespace KlayGE;

#ifndef KLAYGE_PLATFORM_ANDROID
	class CGContextIniter
	{
	public:
		static CGContextIniter& Instance()
		{
			static CGContextIniter initer;
			return initer;
		}

		CGcontext Context()
		{
			return context_;
		}

	private:
		CGContextIniter()
		{
			context_ = cgCreateContext();
		}

	private:
		CGcontext context_;
	};
#endif

	char const * predefined_funcs = "\n									\
	float4 tex1DLevel(sampler1D s, float location, float lod)\n			\
	{\n																	\
	#if KLAYGE_NO_TEX_LOD\n												\
		return tex1D(s, location);\n									\
	#else\n																\
		return tex1Dlod(s, float4(location, 0, 0, lod));\n				\
	#endif\n															\
	}\n																	\
	\n																	\
	float4 tex2DLevel(sampler2D s, float2 location, float lod)\n		\
	{\n																	\
	#if KLAYGE_NO_TEX_LOD\n												\
		return tex2D(s, location);\n									\
	#else\n																\
		return tex2Dlod(s, float4(location, 0, lod));\n					\
	#endif\n															\
	}\n																	\
	\n																	\
	float4 tex3DLevel(sampler3D s, float3 location, float lod)\n		\
	{\n																	\
	#if KLAYGE_NO_TEX_LOD\n												\
		return tex3D(s, location);\n									\
	#else\n																\
		return tex3Dlod(s, float4(location, lod));\n					\
	#endif\n															\
	}\n																	\
	\n																	\
	float4 texCUBELevel(samplerCUBE s, float3 location, float lod)\n	\
	{\n																	\
	#if KLAYGE_NO_TEX_LOD\n												\
		return texCUBE(s, location);\n									\
	#else\n																\
		return texCUBElod(s, float4(location, lod));\n					\
	#endif\n															\
	}\n																	\
	\n																	\
	\n																	\
	float4 tex1DBias(sampler1D s, float location, float lod)\n			\
	{\n																	\
		return tex1Dbias(s, float4(location, 0, 0, lod));\n				\
	}\n																	\
	\n																	\
	float4 tex2DBias(sampler2D s, float2 location, float lod)\n			\
	{\n																	\
		return tex2Dbias(s, float4(location, 0, lod));\n				\
	}\n																	\
	\n																	\
	float4 tex3DBias(sampler3D s, float3 location, float lod)\n			\
	{\n																	\
		return tex3Dbias(s, float4(location, lod));\n					\
	}\n																	\
	\n																	\
	float4 texCUBEBias(samplerCUBE s, float3 location, float lod)\n		\
	{\n																	\
		return texCUBEbias(s, float4(location, lod));\n					\
	}\n																	\
	";

	char const * predefined_attribs = "\n		\
	attribute vec4 a_gl_Color;\n				\
	attribute vec3 a_gl_Normal;\n				\
	attribute vec4 a_gl_Vertex;\n				\
	attribute vec4 a_gl_MultiTexCoord0;\n		\
	attribute vec4 a_gl_MultiTexCoord1;\n		\
	attribute vec4 a_gl_MultiTexCoord2;\n		\
	attribute vec4 a_gl_MultiTexCoord3;\n		\
	attribute vec4 a_gl_MultiTexCoord4;\n		\
	attribute vec4 a_gl_MultiTexCoord5;\n		\
	attribute vec4 a_gl_MultiTexCoord6;\n		\
	attribute vec4 a_gl_MultiTexCoord7;\n		\
	attribute float a_gl_FogCoord;\n			\
	";

	char const * predefined_varyings = "\n	\
	varying vec4 v_gl_FrontColor;\n			\
    varying vec4 v_gl_TexCoord[8];\n		\
    varying float v_gl_FogFragCoord;\n		\
	";

	template <typename SrcType>
	class SetOGLES2ShaderParameter
	{
	};

	template <>
	class SetOGLES2ShaderParameter<bool>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			bool v;
			param_->Value(v);

			glUniform1i(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<uint32_t>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint32_t v;
			param_->Value(v);

			glUniform1i(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<int32_t>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int32_t v;
			param_->Value(v);

			glUniform1i(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float v;
			param_->Value(v);

			glUniform1f(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<uint2>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint2 v;
			param_->Value(v);

			glUniform2iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<uint3>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint3 v;
			param_->Value(v);

			glUniform3iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<uint4>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint4 v;
			param_->Value(v);

			glUniform4iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<int2>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int2 v;
			param_->Value(v);

			glUniform2iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<int3>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int3 v;
			param_->Value(v);

			glUniform3iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<int4>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int4 v;
			param_->Value(v);

			glUniform4iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float2>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float2 v;
			param_->Value(v);

			glUniform2fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float3>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float3 v;
			param_->Value(v);

			glUniform3fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float4>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float4 v;
			param_->Value(v);

			glUniform4fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float4x4>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			glUniform4fv(location_, 4, &v[0]);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<bool*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
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
				glUniform1iv(location_, static_cast<int>(tmp.size()), &tmp[0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<uint32_t*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint32_t> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform1iv(location_, static_cast<int>(v.size()), reinterpret_cast<GLint*>(&v[0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<int32_t*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int32_t> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform1iv(location_, static_cast<int>(v.size()), reinterpret_cast<GLint*>(&v[0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform1fv(location_, static_cast<int>(v.size()), &v[0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<uint2*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint2> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform2iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<uint3*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint3> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform3iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<uint4*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint4> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform4iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<int2*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int2> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform2iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<int3*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int3> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform3iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<int4*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int4> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform4iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float2*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float2> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform2fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float3*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float3> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform3fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float4*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform4fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<float4x4*>
	{
	public:
		SetOGLES2ShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4x4> v;
			param_->Value(v);

			if (!v.empty())
			{
				glUniform4fv(location_, static_cast<long>(v.size()) * 4, &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLES2ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >
	{
	public:
		SetOGLES2ShaderParameter(std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> >& samplers,
					GLint location, GLuint stage,
					RenderEffectParameterPtr const & tex_param, RenderEffectParameterPtr const & sampler_param)
			: samplers_(&samplers), location_(location), stage_(stage), tex_param_(tex_param), sampler_param_(sampler_param)
		{
		}

		void operator()()
		{
			tex_param_->Value((*samplers_)[stage_].first);
			sampler_param_->Value((*samplers_)[stage_].second);

			if ((*samplers_)[stage_].first)
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				re.ActiveTexture(GL_TEXTURE0 + stage_);
				checked_pointer_cast<OGLESSamplerStateObject>((*samplers_)[stage_].second)->Active(stage_, (*samplers_)[stage_].first);
				GLuint const tex_type = checked_pointer_cast<OGLESTexture>((*samplers_)[stage_].first)->GLType();
				GLuint const gl_tex = checked_pointer_cast<OGLESTexture>((*samplers_)[stage_].first)->GLTexture();
				glBindTexture(tex_type, gl_tex);

				glUniform1i(location_, stage_);
			}
		}

	private:
		std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> >* samplers_;
		GLint location_;
		GLuint stage_;
		RenderEffectParameterPtr tex_param_;
		RenderEffectParameterPtr sampler_param_;
	};
}

namespace KlayGE
{
	OGLESShaderObject::OGLESShaderObject()
	{
		has_discard_ = false;
		has_tessellation_ = false;
		is_shader_validate_.assign(true);
	}

	OGLESShaderObject::~OGLESShaderObject()
	{
		glDeleteProgram(glsl_program_);
	}

	std::string OGLESShaderObject::GenShaderText(RenderEffect const & effect)
	{
		std::stringstream shader_ss;

		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		bool sample_helper = false;
		for (uint32_t i = 0; i < effect.NumShaders(); ++ i)
		{
			RenderShaderFunc const & effect_shader = effect.ShaderByIndex(i);
			ShaderType shader_type = effect_shader.Type();
			if (((ST_NumShaderTypes == shader_type) || (ST_VertexShader == shader_type) || (ST_PixelShader == shader_type))
				 && (caps.max_shader_model >= effect_shader.Version()))
			{
				std::string const & s = effect_shader.str();
				boost::char_separator<char> sep("", " \t\n.,():;+-*/%&!|^[]{}'\"?");
				boost::tokenizer<boost::char_separator<char> > tok(s, sep);
				std::string this_token;
				for (BOOST_AUTO(beg, tok.begin()); beg != tok.end();)
				{
					this_token = *beg;

					RenderEffectParameterPtr const & param = effect.ParameterByName(this_token);
					if (param &&
						((REDT_texture1D == param->type()) || (REDT_texture2D == param->type()) || (REDT_texture3D == param->type())
							|| (REDT_textureCUBE == param->type())))
					{
						std::vector<std::string> sample_tokens;
						sample_tokens.push_back(this_token);
						++ beg;
						if ("." == *beg)
						{
							while (*beg != ",")
							{
								if ((*beg != " ") && (*beg != "\t") && (*beg != "\n"))
								{
									sample_tokens.push_back(*beg);
								}
								++ beg;
							}

							std::string combined_sampler_name = sample_tokens[0] + "__" + sample_tokens[4];
							bool found = false;
							for (uint32_t j = 0; j < tex_sampler_binds_.size(); ++ j)
							{
								if (tex_sampler_binds_[j].first == combined_sampler_name)
								{
									found = true;
									break;
								}
							}
							if (!found)
							{
								tex_sampler_binds_.push_back(std::make_pair(combined_sampler_name,
									std::make_pair(param, effect.ParameterByName(sample_tokens[4]))));
							}

							if ((!sample_helper) && (("SampleLevel" == sample_tokens[2]) || ("SampleBias" == sample_tokens[2])))
							{
								sample_helper = true;
							}

							switch (param->type())
							{
							case REDT_texture1D:
							case REDT_texture2D:
							case REDT_texture3D:
							case REDT_textureCUBE:
								shader_ss << "tex";

								switch (param->type())
								{
								case REDT_texture1D:
									shader_ss << "1D";
									break;

								case REDT_texture2D:
									shader_ss << "2D";
									break;

								case REDT_texture3D:
									if (caps.max_texture_depth <= 1)
									{
										shader_ss << "2D";
									}
									else
									{
										shader_ss << "3D";
									}
									break;

								case REDT_textureCUBE:
									shader_ss << "CUBE";
									break;
								}

								if ("SampleLevel" == sample_tokens[2])
								{
									shader_ss << "Level";
								}
								else
								{
									if ("SampleBias" == sample_tokens[2])
									{
										shader_ss << "Bias";
									}
								}

								break;
							}
							shader_ss << "(" << combined_sampler_name << ",";

							++ beg;
						}
						else
						{
							shader_ss << this_token;
						}
					}
					else
					{
						if ("SV_Position" == this_token)
						{
							shader_ss << "POSITION";
						}
						else
						{
							if ("SV_Depth" == this_token)
							{
								shader_ss << "DEPTH";
							}
							else
							{
								if (0 == this_token.find("SV_Target"))
								{
									shader_ss << "COLOR" << this_token.substr(9);
								}
								else
								{
									if ("[" == this_token)
									{
										++ beg;
										if (("branch" == *beg)
											|| ("flatten" == *beg)
											|| ("forcecase" == *beg)
											|| ("call" == *beg)
											|| ("unroll" == *beg)
											|| ("loop" == *beg))
										{
											std::string attr = *beg;
											++ beg;
											if (*beg != "]")
											{
												shader_ss << "[" << attr << *beg;
											}
										}
										else
										{
											shader_ss << "[" << *beg;
										}
									}
									else
									{
										shader_ss << this_token;
									}
								}
							}
						}

						++ beg;
					}
				}
				shader_ss << std::endl;
			}
		}

		std::stringstream ss;
		if (sample_helper)
		{
			ss << predefined_funcs << std::endl;
		}

		for (uint32_t i = 0; i < effect.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = effect.MacroByIndex(i);
			ss << "#define " << name_value.first << " " << name_value.second << std::endl;
		}
		ss << std::endl;

		BOOST_AUTO(cbuffers, effect.CBuffers());
		typedef BOOST_TYPEOF(cbuffers) CBuffersType;
		BOOST_FOREACH(CBuffersType::const_reference cbuff, cbuffers)
		{
			typedef BOOST_TYPEOF(cbuff.second) CBuffersSecondType;
			BOOST_FOREACH(CBuffersSecondType::const_reference param_index, cbuff.second)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(param_index);

				switch (param.type())
				{
				case REDT_buffer:
				case REDT_structured_buffer:
				case REDT_byte_address_buffer:
				case REDT_rw_buffer:
				case REDT_rw_structured_buffer:
				case REDT_rw_texture1D:
				case REDT_rw_texture2D:
				case REDT_rw_texture3D:
				case REDT_rw_texture1DArray:
				case REDT_rw_texture2DArray:
				case REDT_rw_byte_address_buffer:
				case REDT_append_structured_buffer:
				case REDT_consume_structured_buffer:
					ss << std::endl;
					break;

				default:
					ss << effect.TypeName(param.type()) << " " << *param.Name();
					if (param.ArraySize())
					{
						ss << "[" << *param.ArraySize() << "]";
					}
					ss << ";" << std::endl;
					break;
				}
			}
		}

		for (uint32_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			RenderEffectParameterPtr const & param = tex_sampler_binds_[i].second.first;
			ss << "sampler";
			switch (param->type())
			{
			case REDT_texture1D:
				ss << "1D";
				break;

			case REDT_texture2D:
				ss << "2D";
				break;

			case REDT_texture3D:
				if (caps.max_texture_depth <= 1)
				{
					ss << "2D";
				}
				else
				{
					ss << "3D";
				}
				break;

			case REDT_textureCUBE:
				ss << "CUBE";
				break;
			}
			ss << " " << tex_sampler_binds_[i].first << ";" << std::endl;
		}

		ss << shader_ss.str();

		return ss.str();
	}

	std::string OGLESShaderObject::ConvertToELSL(std::string const & glsl, ShaderType type)
	{
		std::stringstream ss;
		switch (type)
		{
		case ST_VertexShader:
			ss << predefined_attribs << std::endl;
			break;

		case ST_PixelShader:
			ss << "precision highp float;" << std::endl;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		ss << predefined_varyings << std::endl;

		boost::char_separator<char> sep("", " \t\n.,():;+-*/%&!|^[]{}'\"?");
		boost::tokenizer<boost::char_separator<char> > tok(glsl, sep);
		std::string this_token;
		for (BOOST_AUTO(beg, tok.begin()); beg != tok.end(); ++ beg)
		{
			this_token = *beg;

			switch (type)
			{
			case ST_VertexShader:
				if (("gl_Color" == this_token) || ("gl_Normal" == this_token)
					|| ("gl_Vertex" == this_token) || ("gl_FogCoord" == this_token)
					|| (0 == this_token.find("gl_MultiTexCoord")))

				{
					ss << "a_" << this_token;
				}
				else
				{
					if (("gl_TexCoord" == this_token) || ("gl_FogFragCoord" == this_token))
					{
						ss << "v_" << this_token;
					}
					else
					{
						if (("gl_FrontColor" == this_token) || ("gl_BackColor" == this_token))
						{
							ss << "v_gl_FrontColor";
						}
						else
						{
							ss << this_token;
						}
					}
				}
				break;

			case ST_PixelShader:
				if (("gl_TexCoord" == this_token) || ("gl_FogFragCoord" == this_token))
				{
					ss << "v_" << this_token;
				}
				else
				{
					if ("gl_Color" == this_token)
					{
						ss << "v_gl_FrontColor";
					}
					else
					{
						if ("discard" == this_token)
						{
							has_discard_ = true;
						}

						ss << this_token;
					}
				}
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		return ss.str();
	}

	void OGLESShaderObject::SetShader(RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
		std::vector<ShaderObjectPtr> const & shared_so)
	{
		std::string shader_text = this->GenShaderText(effect);

		glsl_srcs_ = MakeSharedPtr<boost::array<boost::shared_ptr<std::string>, ST_NumShaderTypes> >();

		pnames_ = MakeSharedPtr<boost::array<boost::shared_ptr<std::vector<std::string> >, ST_NumShaderTypes> >();
		glsl_res_names_ = MakeSharedPtr<boost::array<boost::shared_ptr<std::vector<std::string> >, ST_NumShaderTypes> >();

		vs_usages_ = MakeSharedPtr<std::vector<VertexElementUsage> >();
		vs_usage_indices_ = MakeSharedPtr<std::vector<uint8_t> >();
		glsl_vs_attrib_names_ = MakeSharedPtr<std::vector<std::string> >();

		glsl_program_ = glCreateProgram();

		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			shader_desc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);
			if (!sd.profile.empty())
			{
				is_shader_validate_[type] = true;

				GLenum shader_type;
				switch (type)
				{
				case ST_VertexShader:
					shader_type = GL_VERTEX_SHADER;
					break;

				case ST_PixelShader:
					shader_type = GL_FRAGMENT_SHADER;
					break;

				default:
					shader_type = 0;
					is_shader_validate_[type] = false;
					break;
				}

				if (shared_so[type])
				{
					OGLES2ShaderObjectPtr so = checked_pointer_cast<OGLESShaderObject>(shared_so[type]);

					is_shader_validate_[type] = so->is_shader_validate_[type];

					if (is_shader_validate_[type])
					{
						(*glsl_srcs_)[type] = (*so->glsl_srcs_)[type];

						(*pnames_)[type] = (*so->pnames_)[type];
						(*glsl_res_names_)[type] = (*so->glsl_res_names_)[type];
						if (0 == type)
						{
							*vs_usages_ = *so->vs_usages_;
							*vs_usage_indices_ = *so->vs_usage_indices_;
							*glsl_vs_attrib_names_ = *so->glsl_vs_attrib_names_;
						}

						if (ST_PixelShader == type)
						{
							has_discard_ = so->has_discard_;
						}
					}
				}
				else
				{
					if (is_shader_validate_[type])
					{
						std::string const & effect_name = ResLoader::Instance().Locate(effect.ResName());
						std::string fxml_bin_name = effect_name.substr(0, effect_name.rfind("."));
						std::stringstream oss;
						oss << fxml_bin_name << "_" << sd.func_name << "_" << type << ".fxml_bin";
						fxml_bin_name = oss.str();

						bool recompile_cg;
						ResIdentifierPtr bin_res = ResLoader::Instance().Open(fxml_bin_name);
						if (bin_res)
						{
							if (effect.Timestamp() < bin_res->Timestamp())
							{
								uint32_t fourcc;
								bin_res->read(&fourcc, sizeof(fourcc));
								if (fourcc != MakeFourCC<'F', 'X', 'M', 'L'>::value)
								{
									recompile_cg = true;
								}
								else
								{
									uint32_t ver;
									bin_res->read(&ver, sizeof(ver));
									if (ver != 1)
									{
										recompile_cg = true;
									}
									else
									{
										uint32_t len;
										bin_res->read(&len, sizeof(len));
										(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(len, '\0');
										bin_res->read(&(*(*glsl_srcs_)[type])[0], len);

										uint32_t num;
										bin_res->read(&num, sizeof(num));
										(*pnames_)[type] = MakeSharedPtr<std::vector<std::string> >(num);
										for (size_t i = 0; i < num; ++ i)
										{
											bin_res->read(&len, sizeof(len));
											
											(*(*pnames_)[type])[i].resize(len);
											bin_res->read(&(*(*pnames_)[type])[i][0], len);
										}

										bin_res->read(&num, sizeof(num));
										(*glsl_res_names_)[type] = MakeSharedPtr<std::vector<std::string> >(num);
										for (size_t i = 0; i < num; ++ i)
										{
											bin_res->read(&len, sizeof(len));
											
											(*(*glsl_res_names_)[type])[i].resize(len);
											bin_res->read(&(*(*glsl_res_names_)[type])[i][0], len);
										}

										bin_res->read(&num, sizeof(num));
										vs_usages_->resize(num);
										bin_res->read(&(*vs_usages_)[0], num * sizeof((*vs_usages_)[0]));

										bin_res->read(&num, sizeof(num));
										vs_usage_indices_->resize(num);
										bin_res->read(&(*vs_usage_indices_)[0], num * sizeof((*vs_usage_indices_)[0]));

										bin_res->read(&num, sizeof(num));
										glsl_vs_attrib_names_->resize(num);
										for (size_t i = 0; i < num; ++ i)
										{
											bin_res->read(&len, sizeof(len));
											
											(*glsl_vs_attrib_names_)[i].resize(len);
											bin_res->read(&(*glsl_vs_attrib_names_)[i][0], len);
										}

										recompile_cg = false;
									}
								}
							}
							else
							{
								recompile_cg = true;
							}
							
							bin_res.reset();
						}
						else
						{
							recompile_cg = true;
						}

						if (recompile_cg)
						{
							this->CompileToNative(shader_text, type, sd.func_name);

#ifndef KLAYGE_PLATFORM_ANDROID
							if (is_shader_validate_[type])
							{
								std::ofstream ofs(fxml_bin_name.c_str(), std::ios_base::out | std::ios_base::binary);
								uint32_t fourcc = MakeFourCC<'F', 'X', 'M', 'L'>::value;
								ofs.write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));
								uint32_t ver = 1;
								ofs.write(reinterpret_cast<char const *>(&ver), sizeof(ver));
								
								uint32_t len = (*glsl_srcs_)[type]->size();
								ofs.write(reinterpret_cast<char const *>(&len), sizeof(len));
								ofs.write(&(*(*glsl_srcs_)[type])[0], len);

								uint32_t num = (*pnames_)[type]->size();
								ofs.write(reinterpret_cast<char const *>(&num), sizeof(num));
								for (size_t i = 0; i < num; ++ i)
								{
									len = (*(*pnames_)[type])[i].size();
									ofs.write(reinterpret_cast<char const *>(&len), sizeof(len));
									ofs.write(&(*(*pnames_)[type])[i][0], len);
								}

								num = (*glsl_res_names_)[type]->size();
								ofs.write(reinterpret_cast<char const *>(&num), sizeof(num));
								for (size_t i = 0; i < num; ++ i)
								{
									len = (*(*glsl_res_names_)[type])[i].size();
									ofs.write(reinterpret_cast<char const *>(&len), sizeof(len));
									ofs.write(&(*(*glsl_res_names_)[type])[i][0], len);
								}

								num = vs_usages_->size();
								ofs.write(reinterpret_cast<char const *>(&num), sizeof(num));
								ofs.write(reinterpret_cast<char const *>(&(*vs_usages_)[0]), num * sizeof((*vs_usages_)[0]));

								num = vs_usage_indices_->size();
								ofs.write(reinterpret_cast<char const *>(&num), sizeof(num));
								ofs.write(reinterpret_cast<char const *>(&(*vs_usage_indices_)[0]), num * sizeof((*vs_usage_indices_)[0]));

								num = glsl_vs_attrib_names_->size();
								ofs.write(reinterpret_cast<char const *>(&num), sizeof(num));
								for (size_t i = 0; i < num; ++ i)
								{
									len = (*glsl_vs_attrib_names_)[i].size();
									ofs.write(reinterpret_cast<char const *>(&len), sizeof(len));
									ofs.write(&(*glsl_vs_attrib_names_)[i][0], len);
								}
							}
#endif
						}
					}
				}

				if (is_shader_validate_[type])
				{
					this->AttachGLSL(type);
				}
			}

			is_validate_ &= is_shader_validate_[type];
		}

		if (is_validate_)
		{
			this->LinkGLSL();

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
									if (tex_sampler_binds_[i].first == (*(*pnames_)[type])[pi])
									{
										parameter_bind_t pb;
										pb.combined_sampler_name = tex_sampler_binds_[i].first;
										pb.location = location;
										pb.shader_type = type;
										pb.tex_sampler_bind_index = static_cast<int>(i);

										uint32_t index = static_cast<uint32_t>(samplers_.size());
										samplers_.resize(index + 1);

										pb.func = SetOGLES2ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_, location,
											index, tex_sampler_binds_[i].second.first, tex_sampler_binds_[i].second.second);

										param_binds_.push_back(pb);

										break;
									}
								}
							}
						}
					}
				}
				
				if (0 == type)
				{
					for (size_t pi = 0; pi < glsl_vs_attrib_names_->size(); ++ pi)
					{
						attrib_locs_.insert(std::make_pair(std::make_pair((*vs_usages_)[pi], (*vs_usage_indices_)[pi]),
								glGetAttribLocation(glsl_program_, (*glsl_vs_attrib_names_)[pi].c_str())));
					}
				}
			}
		}
	}

	ShaderObjectPtr OGLESShaderObject::Clone(RenderEffect const & effect)
	{
		OGLES2ShaderObjectPtr ret = MakeSharedPtr<OGLESShaderObject>();

		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;
		ret->glsl_srcs_ = glsl_srcs_;
		ret->pnames_ = pnames_;
		ret->glsl_res_names_ = glsl_res_names_;
		ret->vs_usages_ = vs_usages_;
		ret->vs_usage_indices_ = vs_usage_indices_;
		ret->glsl_vs_attrib_names_ = glsl_vs_attrib_names_;

		ret->tex_sampler_binds_.resize(tex_sampler_binds_.size());
		for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			ret->tex_sampler_binds_[i].first = tex_sampler_binds_[i].first;
			ret->tex_sampler_binds_[i].second.first = effect.ParameterByName(*(tex_sampler_binds_[i].second.first->Name()));
			ret->tex_sampler_binds_[i].second.second = effect.ParameterByName(*(tex_sampler_binds_[i].second.second->Name()));
		}

		ret->glsl_program_ = glCreateProgram();

		ret->is_validate_ = true;
		for (size_t type = 0; type < ST_NumShaderTypes; ++ type)
		{
			ret->is_shader_validate_[type] = is_shader_validate_[type];

			if (is_shader_validate_[type])
			{
				if ((*glsl_srcs_)[type] && !(*glsl_srcs_)[type]->empty())
				{
					ret->AttachGLSL(type);
				}
			}

			ret->is_validate_ &= ret->is_shader_validate_[type];
		}

		if (ret->is_validate_)
		{
			ret->LinkGLSL();

			ret->attrib_locs_ = attrib_locs_;

			typedef BOOST_TYPEOF(param_binds_) ParamBindsType;
			BOOST_FOREACH(ParamBindsType::reference pb, param_binds_)
			{
				if (pb.param)
				{
					RenderEffectParameterPtr const & p = effect.ParameterByName(*pb.param->Name());
					ret->param_binds_.push_back(ret->GetBindFunc(pb.location, p));
				}
				else
				{
					std::string const & pname = pb.combined_sampler_name;
					for (size_t j = 0; j < ret->tex_sampler_binds_.size(); ++ j)
					{
						if (ret->tex_sampler_binds_[j].first == pname)
						{
							parameter_bind_t new_pb;
							new_pb.combined_sampler_name = pname;
							new_pb.location = pb.location;
							new_pb.shader_type = pb.shader_type;
							new_pb.tex_sampler_bind_index = pb.tex_sampler_bind_index;

							uint32_t index = static_cast<uint32_t>(ret->samplers_.size());
							ret->samplers_.resize(index + 1);

							new_pb.func = SetOGLES2ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(ret->samplers_,
								new_pb.location, index,
								ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index].second.first,
								ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index].second.second);

							ret->param_binds_.push_back(new_pb);

							break;
						}
					}
				}
			}
		}

		return ret;
	}

	GLint OGLESShaderObject::GetAttribLocation(VertexElementUsage usage, uint8_t usage_index)
	{
		BOOST_AUTO(iter, attrib_locs_.find(std::make_pair(usage, usage_index)));
		if (iter != attrib_locs_.end())
		{
			return iter->second;
		}
		else
		{
			return -1;
		}
	}

	OGLESShaderObject::parameter_bind_t OGLESShaderObject::GetBindFunc(GLint location, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.location = location;

		switch (param->type())
		{
		case REDT_bool:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<bool*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<bool>(location, param);
			}
			break;

		case REDT_uint:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<uint32_t*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<uint32_t>(location, param);
			}
			break;

		case REDT_int:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<int32_t*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<int32_t>(location, param);
			}
			break;

		case REDT_float:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<float*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<float>(location, param);
			}
			break;

		case REDT_uint2:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<uint2*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<uint2>(location, param);
			}
			break;

		case REDT_uint3:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<uint3*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<uint3>(location, param);
			}
			break;

		case REDT_uint4:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<uint4*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<uint4>(location, param);
			}
			break;

		case REDT_int2:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<int2*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<int2>(location, param);
			}
			break;

		case REDT_int3:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<int3*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<int3>(location, param);
			}
			break;

		case REDT_int4:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<int4*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<int4>(location, param);
			}
			break;

		case REDT_float2:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<float2*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<float2>(location, param);
			}
			break;

		case REDT_float3:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<float3*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<float3>(location, param);
			}
			break;

		case REDT_float4:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<float4*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<float4>(location, param);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize())
			{
				ret.func = SetOGLES2ShaderParameter<float4x4*>(location, param);
			}
			else
			{
				ret.func = SetOGLES2ShaderParameter<float4x4>(location, param);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void OGLESShaderObject::CompileToNative(std::string const & shader_text, int type, std::string const & func_name)
	{
#ifndef KLAYGE_PLATFORM_ANDROID
		OGLESRenderFactory& rf = *checked_cast<OGLESRenderFactory*>(&Context::Instance().RenderFactoryInstance());
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		std::string max_sm_str;
		{
			std::stringstream ss;
			ss << "-DKLAYGE_SHADER_MODEL=" << static_cast<int>(caps.max_shader_model);
			max_sm_str = ss.str();
		}
		std::string max_tex_array_str;
		{
			std::stringstream ss;
			ss << "-DKLAYGE_MAX_TEX_ARRAY_LEN=" << caps.max_texture_array_length;
			max_tex_array_str = ss.str();
		}
		std::string max_tex_depth_str;
		{
			std::stringstream ss;
			ss << "-DKLAYGE_MAX_TEX_DEPTH=" << caps.max_texture_depth;
			max_tex_depth_str = ss.str();
		}
		std::string max_tex_units_str;
		{
			std::stringstream ss;
			ss << "-DKLAYGE_MAX_TEX_UNITS=" << static_cast<int>(caps.max_pixel_texture_units);
			max_tex_units_str = ss.str();
		}
		std::string no_tex_lod_str;
		{
			std::stringstream ss;
			ss << "-DKLAYGE_NO_TEX_LOD=" << ((ST_VertexShader == type) ? 0 : (glloader_GLES_EXT_shader_texture_lod() ? 0 : 1));
			no_tex_lod_str = ss.str();
		}		
		std::string flipping_str;
		{
			std::stringstream ss;
			ss << "-DKLAYGE_FLIPPING=" << (re.RequiresFlipping() ? -1 : +1);
			flipping_str = ss.str();
		}

		std::vector<char const *> args;
		args.push_back("-DKLAYGE_OPENGLES=1");
		args.push_back(max_sm_str.c_str());
		args.push_back(max_tex_array_str.c_str());
		args.push_back(max_tex_depth_str.c_str());
		args.push_back(max_tex_units_str.c_str());
		args.push_back(no_tex_lod_str.c_str());
		args.push_back(flipping_str.c_str());
		if (!re.DeviceCaps().texture_format_support(EF_BC5))
		{
			args.push_back("-DKLAYGE_BC5_AS_AG");
		}
		else
		{
			args.push_back("-DKLAYGE_BC5_AS_GA");
		}
		args.push_back(NULL);

		CGprofile profile;
		switch (type)
		{
		case ST_VertexShader:
			profile = CG_PROFILE_GLSLV;
			break;

		case ST_PixelShader:
			profile = CG_PROFILE_GLSLF;
			break;

		default:
			profile = CG_PROFILE_UNKNOWN;
			break;
		}

		CGprogram cg_shader = cgCreateProgram(CGContextIniter::Instance().Context(),
				CG_SOURCE, shader_text.c_str(), profile, func_name.c_str(), &args[0]);

		CGerror error = cgGetError();
		if (error != CG_NO_ERROR)
		{
#ifdef KLAYGE_DEBUG
			if (CG_COMPILER_ERROR == error)
			{
				std::istringstream iss(shader_text);
				std::string s;
				int line = 1;
				while (iss)
				{
					std::getline(iss, s);
					std::cerr << line << " " << s << std::endl;
					++ line;
				}
				std::cerr << "Error when compiling " << func_name << ":" << std::endl;
				std::cerr << cgGetErrorString(error) << std::endl;

				char const* listing = cgGetLastListing(CGContextIniter::Instance().Context());
				if (listing)
				{
					std::cerr << listing << std::endl;
				}
			}
#endif

			is_shader_validate_[type] = false;
		}
		else
		{
			(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(this->ConvertToELSL(cgGetProgramString(cg_shader, CG_COMPILED_PROGRAM),
				static_cast<ShaderType>(type)));
			(*pnames_)[type] = MakeSharedPtr<std::vector<std::string> >();
			(*glsl_res_names_)[type] = MakeSharedPtr<std::vector<std::string> >();

			CGparameter cg_param = cgGetFirstParameter(cg_shader, CG_GLOBAL);
			while (cg_param)
			{
				if (cgIsParameterUsed(cg_param, cg_shader)
					&& (CG_PARAMETERCLASS_OBJECT != cgGetParameterClass(cg_param)))
				{
					char const * pname = cgGetParameterName(cg_param);
					(*pnames_)[type]->push_back(pname);

					char const * glsl_param_name = cgGetParameterResourceName(cg_param);
					std::string hacked_name = std::string("_") + pname;

					if ((cgGetError() != CG_NO_ERROR) || (NULL == glsl_param_name))
					{
						// Some times cgGetParameterResourceName doesn't work
						glsl_param_name = hacked_name.c_str();
					}

					(*glsl_res_names_)[type]->push_back(glsl_param_name);
				}

				cg_param = cgGetNextParameter(cg_param);
			}

			if (0 == type)
			{
				cg_param = cgGetFirstParameter(cg_shader, CG_PROGRAM);
				while (cg_param)
				{
					if (cgIsParameterUsed(cg_param, cg_shader)
						&& (CG_PARAMETERCLASS_OBJECT != cgGetParameterClass(cg_param))
						&& ((CG_IN == cgGetParameterDirection(cg_param)) || (CG_INOUT == cgGetParameterDirection(cg_param))))
					{
						std::string semantic = cgGetParameterSemantic(cg_param);
						std::string glsl_param_name = semantic;//cgGetParameterResourceName(cg_param);

						VertexElementUsage usage = VEU_Position;
						uint8_t usage_index = 0;
						if ("POSITION" == semantic)
						{
							usage = VEU_Position;
							glsl_param_name = "a_gl_Vertex";
						}
						else if ("NORMAL" == semantic)
						{
							usage = VEU_Normal;
							glsl_param_name = "a_gl_Normal";
						}
						else if (("COLOR0" == semantic) || ("COLOR" == semantic))
						{
							usage = VEU_Diffuse;
							glsl_param_name = "a_gl_Color";
						}
						else if ("COLOR1" == semantic)
						{
							usage = VEU_Specular;
							glsl_param_name = "a_gl_SecondaryColor";
						}
						else if ("BLENDWEIGHT" == semantic)
						{
							usage = VEU_BlendWeight;
							glsl_param_name = "BLENDWEIGHT";
						}
						else if ("BLENDINDICES" == semantic)
						{
							usage = VEU_BlendIndex;
							glsl_param_name = "BLENDINDICES";
						}
						else if (0 == semantic.find("TEXCOORD"))
						{
							usage = VEU_TextureCoord;
							usage_index = static_cast<uint8_t>(boost::lexical_cast<int>(semantic.substr(8)));
							glsl_param_name = "a_gl_MultiTexCoord" + semantic.substr(8);
						}
						else if ("TANGENT" == semantic)
						{
							usage = VEU_Tangent;
							glsl_param_name = "TANGENT";
						}
						else
						{
							BOOST_ASSERT("BINORMAL" == semantic);

							usage = VEU_Binormal;
							glsl_param_name = "BINORMAL";
						}

						vs_usages_->push_back(usage);
						vs_usage_indices_->push_back(usage_index);
						glsl_vs_attrib_names_->push_back(glsl_param_name);
					}

					cg_param = cgGetNextParameter(cg_param);
				}
			}

			cgDestroyProgram(cg_shader);
		}
#endif
	}

	void OGLESShaderObject::AttachGLSL(uint32_t type)
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
		//printf("%s\n", glsl);

		glShaderSource(object, 1, &glsl, NULL);

		glCompileShader(object);

		GLint compiled = false;
		glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);
#ifdef KLAYGE_DEBUG
		if (!compiled)
		{
			printf("%s\n", glsl);

			GLint len = 0;
			glGetShaderiv(object, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				std::vector<char> info(len + 1, 0);
				glGetShaderInfoLog(object, len, &len, &info[0]);
				std::cerr << &info[0] << std::endl;
			}
		}
#endif
		is_shader_validate_[type] &= compiled ? true : false;

		glAttachShader(glsl_program_, object);
		glDeleteShader(object);
	}

	void OGLESShaderObject::LinkGLSL()
	{
		glLinkProgram(glsl_program_);

		GLint linked = false;
		glGetProgramiv(glsl_program_, GL_LINK_STATUS, &linked);
#ifdef KLAYGE_DEBUG
		if (!linked)
		{
			GLint len = 0;
			glGetProgramiv(glsl_program_, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				std::vector<char> info(len + 1, 0);
				glGetProgramInfoLog(glsl_program_, len, &len, &info[0]);
				std::cerr << &info[0] << std::endl;
			}
		}
#endif
		is_validate_ &= linked ? true : false;
	}

	void OGLESShaderObject::Bind()
	{
		glUseProgram(glsl_program_);

		typedef BOOST_TYPEOF(param_binds_) ParamBindsType;
		BOOST_FOREACH(ParamBindsType::reference pb, param_binds_)
		{
			pb.func();
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
				std::cerr << &info[0] << std::endl;
			}
		}
#endif
	}

	void OGLESShaderObject::Unbind()
	{
		//glUseProgram(0);
	}
}
