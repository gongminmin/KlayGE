// OGLShaderObject.cpp
// KlayGE OpenGL shader对象类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2006-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Geometry shader to GLSL compiler works
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
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <cstdio>
#include <string>
#include <algorithm>
#include <sstream>
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

#include <Cg/cg.h>

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactoryInternal.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "Cg.lib")
#endif

namespace
{
	using namespace KlayGE;

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

	char const * predefined_funcs = "\n									\
	float4 tex1DLevel(sampler1D s, float location, float lod)\n			\
	{\n																	\
		return tex1Dlod(s, float4(location, 0, 0, lod));\n				\
	}\n																	\
	\n																	\
	float4 tex2DLevel(sampler2D s, float2 location, float lod)\n		\
	{\n																	\
		return tex2Dlod(s, float4(location, 0, lod));\n					\
	}\n																	\
	\n																	\
	float4 tex3DLevel(sampler3D s, float3 location, float lod)\n		\
	{\n																	\
		return tex3Dlod(s, float4(location, lod));\n					\
	}\n																	\
	\n																	\
	float4 texCUBELevel(samplerCUBE s, float3 location, float lod)\n	\
	{\n																	\
		return texCUBElod(s, float4(location, lod));\n					\
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
	}\n";

	char const * predefined_attribs = "\n"\
		"attribute vec4 a_gl_Color;\n"\
		"attribute vec4 a_gl_SecondaryColor;\n"\
		"attribute vec3 a_gl_Normal;\n"\
		"attribute vec4 a_gl_Vertex;\n"\
		"attribute vec4 a_gl_MultiTexCoord0;\n"\
		"attribute vec4 a_gl_MultiTexCoord1;\n"\
		"attribute vec4 a_gl_MultiTexCoord2;\n"\
		"attribute vec4 a_gl_MultiTexCoord3;\n"\
		"attribute vec4 a_gl_MultiTexCoord4;\n"\
		"attribute vec4 a_gl_MultiTexCoord5;\n"\
		"attribute vec4 a_gl_MultiTexCoord6;\n"\
		"attribute vec4 a_gl_MultiTexCoord7;\n"\
		"attribute float a_gl_FogCoord;\n";

	char const * predefined_varyings = "\n"\
		"varying vec4 v_gl_FrontColor;\n"\
		"varying vec4 v_gl_BackColor;\n"\
		"varying vec4 v_gl_FrontSecondaryColor;\n"\
		"varying vec4 v_gl_BackSecondaryColor;\n"\
		"varying vec4 v_gl_TexCoord[8];\n"\
		"varying float v_gl_FogFragCoord;\n";

	char const * predefined_vs_out_varyings_with_gs = "\n"\
		"varying vec4 v_gl_FrontColorIn;\n"\
		"varying vec4 v_gl_BackColorIn;\n"\
		"varying vec4 v_gl_FrontSecondaryColorIn;\n"\
		"varying vec4 v_gl_BackSecondaryColorIn;\n"\
		"varying vec4 v_gl_TexCoordIn0;\n"\
		"varying vec4 v_gl_TexCoordIn1;\n"\
		"varying vec4 v_gl_TexCoordIn2;\n"\
		"varying vec4 v_gl_TexCoordIn3;\n"\
		"varying vec4 v_gl_TexCoordIn4;\n"\
		"varying vec4 v_gl_TexCoordIn5;\n"\
		"varying vec4 v_gl_TexCoordIn6;\n"\
		"varying vec4 v_gl_TexCoordIn7;\n"\
		"varying float v_gl_FogFragCoordIn;\n";

	char const * predefined_gs_in_varyings = "\n"\
		"varying in vec4 v_gl_FrontColorIn[%d];\n"\
		"varying in vec4 v_gl_BackColorIn[%d];\n"\
		"varying in vec4 v_gl_FrontSecondaryColorIn[%d];\n"\
		"varying in vec4 v_gl_BackSecondaryColorIn[%d];\n"\
		"varying in vec4 v_gl_TexCoordIn0[%d];\n"\
		"varying in vec4 v_gl_TexCoordIn1[%d];\n"\
		"varying in vec4 v_gl_TexCoordIn2[%d];\n"\
		"varying in vec4 v_gl_TexCoordIn3[%d];\n"\
		"varying in vec4 v_gl_TexCoordIn4[%d];\n"\
		"varying in vec4 v_gl_TexCoordIn5[%d];\n"\
		"varying in vec4 v_gl_TexCoordIn6[%d];\n"\
		"varying in vec4 v_gl_TexCoordIn7[%d];\n"\
		"varying in float v_gl_FogFragCoordIn[%d];\n";

	char const * predefined_gs_out_varyings = "\n"\
		"varying out vec4 v_gl_FrontColorOut;\n"\
		"varying out vec4 v_gl_BackColorOut;\n"\
		"varying out vec4 v_gl_FrontSecondaryColorOut;\n"\
		"varying out vec4 v_gl_BackSecondaryColorOut;\n"\
		"varying out vec4 v_gl_TexCoordOut[8];\n"\
		"varying out float v_gl_FogFragCoordOut;\n";

	char const * predefined_ps_in_varyings_with_gs = "\n"\
		"varying vec4 v_gl_FrontColorOut;\n"\
		"varying vec4 v_gl_BackColorOut;\n"\
		"varying vec4 v_gl_FrontSecondaryColorOut;\n"\
		"varying vec4 v_gl_BackSecondaryColorOut;\n"\
		"varying vec4 v_gl_TexCoordOut[8];\n"\
		"varying float v_gl_FogFragCoordOut;\n";

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

			glUniform1i(location_, v);
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

			glUniform1ui(location_, v);
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

			glUniform1i(location_, v);
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

			glUniform1f(location_, v);
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

			glUniform2uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
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

			glUniform3uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
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

			glUniform4uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
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

			glUniform2iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
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

			glUniform3iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
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

			glUniform4iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
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

			glUniform2fv(location_, 1, &v.x());
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

			glUniform3fv(location_, 1, &v.x());
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

			glUniform4fv(location_, 1, &v.x());
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

			glUniform4fv(location_, 4, &v[0]);
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
				glUniform1iv(location_, static_cast<int>(tmp.size()), &tmp[0]);
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
				glUniform1uiv(location_, static_cast<int>(v.size()), reinterpret_cast<GLuint*>(&v[0]));
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
				glUniform1iv(location_, static_cast<int>(v.size()), reinterpret_cast<GLint*>(&v[0]));
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
				glUniform1fv(location_, static_cast<int>(v.size()), &v[0]);
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
				glUniform2uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
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
				glUniform3uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
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
				glUniform4uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
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
				glUniform2iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
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
				glUniform3iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
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
				glUniform4iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
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
				glUniform2fv(location_, static_cast<long>(v.size()), &v[0][0]);
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
				glUniform3fv(location_, static_cast<long>(v.size()), &v[0][0]);
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
				glUniform4fv(location_, static_cast<long>(v.size()), &v[0][0]);
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
				glUniform4fv(location_, static_cast<long>(v.size()) * 4, &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >
	{
	public:
		SetOGLShaderParameter(std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> >& samplers,
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
				glActiveTexture(GL_TEXTURE0 + stage_);
				checked_pointer_cast<OGLSamplerStateObject>((*samplers_)[stage_].second)->Active(stage_, (*samplers_)[stage_].first);
				GLuint const tex_type = checked_pointer_cast<OGLTexture>((*samplers_)[stage_].first)->GLType();
				GLuint const gl_tex = checked_pointer_cast<OGLTexture>((*samplers_)[stage_].first)->GLTexture();
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
	OGLShaderObject::OGLShaderObject()
		: gs_input_type_(0), gs_output_type_(0)
	{
		is_shader_validate_.assign(true);
	}

	OGLShaderObject::~OGLShaderObject()
	{
		glDeleteProgram(glsl_program_);
	}

	std::string OGLShaderObject::GenShaderText(RenderEffect const & effect)
	{
		std::stringstream shader_ss;

		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		bool sample_helper = false;
		for (uint32_t i = 0; i < effect.NumShaders(); ++ i)
		{
			RenderShaderFunc const & effect_shader = effect.ShaderByIndex(i);
			ShaderType shader_type = effect_shader.Type();
			if (((ST_NumShaderTypes == shader_type) || (ST_VertexShader == shader_type) || (ST_PixelShader == shader_type) || (ST_GeometryShader == shader_type))
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
									shader_ss << "3D";
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
		BOOST_FOREACH(BOOST_TYPEOF(cbuffers)::const_reference cbuff, cbuffers)
		{
			BOOST_FOREACH(BOOST_TYPEOF(cbuff.second)::const_reference param_index, cbuff.second)
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
				ss << "3D";
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

	std::string OGLShaderObject::ConvertToGLSL(std::string const & glsl, ShaderType type, uint32_t gs_input_vertices, bool has_gs)
	{
		char predefined_gs_in_varyings_add_num[1024];
		sprintf(predefined_gs_in_varyings_add_num, predefined_gs_in_varyings,
			gs_input_vertices, gs_input_vertices, gs_input_vertices, gs_input_vertices,
			gs_input_vertices, gs_input_vertices, gs_input_vertices, gs_input_vertices,
			gs_input_vertices, gs_input_vertices, gs_input_vertices, gs_input_vertices,
			gs_input_vertices);

		std::stringstream ss;
		switch (type)
		{
		case ST_VertexShader:
			ss << predefined_attribs << std::endl;
			if (has_gs)
			{
				ss << predefined_vs_out_varyings_with_gs << std::endl;
			}
			else
			{
				ss << predefined_varyings << std::endl;
			}
			break;

		case ST_GeometryShader:
			ss << predefined_gs_in_varyings_add_num << std::endl;
			ss << predefined_gs_out_varyings << std::endl;
			break;

		case ST_PixelShader:
			if (has_gs)
			{
				ss << predefined_ps_in_varyings_with_gs << std::endl;
			}
			else
			{
				ss << predefined_varyings << std::endl;
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		boost::char_separator<char> sep("", " \t\n.,():;+-*/%&!|^[]{}'\"?");
		boost::tokenizer<boost::char_separator<char> > tok(glsl, sep);
		std::string this_token;
		for (BOOST_AUTO(beg, tok.begin()); beg != tok.end(); ++ beg)
		{
			this_token = *beg;

			switch (type)
			{
			case ST_VertexShader:
			case ST_GeometryShader:
				if (("gl_Color" == this_token) || ("gl_Normal" == this_token)
					|| ("gl_Vertex" == this_token) || ("gl_FogCoord" == this_token)
					|| (0 == this_token.find("gl_MultiTexCoord")))

				{
					ss << "a_" << this_token;
				}
				else
				{
					if (("gl_FogFragCoord" == this_token)
						|| ("gl_FrontColor" == this_token)
						|| ("gl_BackColor" == this_token)
						|| ("gl_FrontSecondaryColor" == this_token)
						|| ("gl_BackSecondaryColor" == this_token))
					{
						ss << "v_" << this_token;
						if ((ST_VertexShader == type) && has_gs)
						{
							ss << "In";
						}
						else
						{
							if (ST_GeometryShader == type)
							{
								ss << "Out";
							}
						}
					}
					else
					{
						if (("gl_FogFragCoordIn" == this_token)
							|| ("gl_FrontColorIn" == this_token)
							|| ("gl_BackColorIn" == this_token)
							|| ("gl_FrontSecondaryColorIn" == this_token)
							|| ("gl_BackSecondaryColorIn" == this_token))
						{
							ss << "v_" << this_token;
						}
						else
						{
							if ("gl_TexCoord" == this_token)
							{
								if ((ST_VertexShader == type) && has_gs)
								{
									std::string tmp_token[3];
									for (int t = 0; t < 3; ++ t)
									{
										++ beg;
										tmp_token[t] = *beg;
									}

									ss << "v_" << this_token << "In" << tmp_token[1];
								}
								else
								{
									ss << "v_" << this_token;
									if (ST_GeometryShader == type)
									{
										ss  << "Out";
									}
								}
							}
							else
							{
								if ("gl_TexCoordIn" == this_token)
								{
									std::string tmp_token[6];
									for (int t = 0; t < 6; ++ t)
									{
										++ beg;
										tmp_token[t] = *beg;
									}

									ss << "v_" << this_token << tmp_token[4]
										 << tmp_token[0] << tmp_token[1] << tmp_token[2];
								}
								else
								{
									ss << this_token;
								}
							}
						}
					}
				}
				break;

			case ST_PixelShader:
				if (("gl_TexCoord" == this_token) || ("gl_FogFragCoord" == this_token))
				{
					ss << "v_" << this_token;
					if (has_gs)
					{
						ss << "Out";
					}
				}
				else
				{
					if ("gl_Color" == this_token)
					{
						ss << "v_gl_FrontColor";
						if (has_gs)
						{
							ss << "Out";
						}
					}
					else
					{
						if ("gl_SecondaryColor" == this_token)
						{
							ss << "v_gl_FrontSecondaryColor";
							if (has_gs)
							{
								ss << "Out";
							}
						}
						else
						{
							ss << this_token;
						}
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

	void OGLShaderObject::SetShader(RenderEffect& effect, boost::shared_ptr<std::vector<uint32_t> > const & shader_desc_ids,
		uint32_t /*tech_index*/, uint32_t /*pass_index*/)
	{
		OGLRenderFactory& rf = *checked_cast<OGLRenderFactory*>(&Context::Instance().RenderFactoryInstance());
		RenderEngine& re = rf.RenderEngineInstance();

		shader_desc_ids_ = shader_desc_ids;
		shader_text_ = MakeSharedPtr<std::string>(this->GenShaderText(effect));

		std::vector<char const *> args;
		args.push_back("-DKLAYGE_OPENGL=1");
		if (!re.DeviceCaps().bc5_support)
		{
			args.push_back("-DKLAYGE_BC5_AS_AG");
		}
		else
		{
			args.push_back("-DKLAYGE_BC5_AS_GA");
		}
		args.push_back(NULL);

		boost::array<CGprogram, ST_NumShaderTypes> shaders;
		glsl_srcs_ = MakeSharedPtr<std::vector<std::string> >(ShaderObject::ST_NumShaderTypes);

		glsl_program_ = glCreateProgram();

		bool has_gs = false;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			shader_desc& sd = effect.GetShaderDesc((*shader_desc_ids)[type]);
			if ((ST_GeometryShader == type) && (!sd.profile.empty()))
			{
				has_gs = true;
			}
		}

		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			shaders[type] = 0;

			shader_desc& sd = effect.GetShaderDesc((*shader_desc_ids)[type]);
			if (!sd.profile.empty())
			{
				is_shader_validate_[type] = true;

				GLenum shader_type;
				CGprofile profile;
				switch (type)
				{
				case ST_VertexShader:
					shader_type = GL_VERTEX_SHADER;
					profile = CG_PROFILE_GLSLV;
					break;

				case ST_PixelShader:
					shader_type = GL_FRAGMENT_SHADER;
					profile = CG_PROFILE_GLSLF;
					break;

				case ST_GeometryShader:
					shader_type = GL_GEOMETRY_SHADER_EXT;
					profile = CG_PROFILE_GLSLG;
					break;

				default:
					shader_type = 0;
					profile = CG_PROFILE_UNKNOWN;
					is_shader_validate_[type] = false;
					break;
				}

				if (is_shader_validate_[type])
				{
					shaders[type] = cgCreateProgram(CGContextIniter::Instance().Context(),
							CG_SOURCE, shader_text_->c_str(), profile, sd.func_name.c_str(), &args[0]);

					CGerror error = cgGetError();
					if (error != CG_NO_ERROR)
					{
#ifdef KLAYGE_DEBUG
						if (CG_COMPILER_ERROR == error)
						{
							std::istringstream iss(*shader_text_);
							std::string s;
							int line = 1;
							while (iss)
							{
								std::getline(iss, s);
								std::cerr << line << " " << s << std::endl;
								++ line;
							}
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
				}

				if (is_shader_validate_[type])
				{
					uint32_t gs_input_vertices = 0;
					if (ST_GeometryShader == type)
					{
						switch (cgGetProgramInput(shaders[type]))
						{
						case CG_POINT:
							gs_input_type_ = GL_POINTS;
							gs_input_vertices = 1;
							break;

						case CG_LINE:
							gs_input_type_ = GL_LINES;
							gs_input_vertices = 2;
							break;

						case CG_LINE_ADJ:
							gs_input_type_ = GL_LINES_ADJACENCY_EXT;
							gs_input_vertices = 4;
							break;

						case CG_TRIANGLE:
							gs_input_type_ = GL_TRIANGLES;
							gs_input_vertices = 3;
							break;

						case CG_TRIANGLE_ADJ:
							gs_input_type_ = GL_TRIANGLES_ADJACENCY_EXT;
							gs_input_vertices = 6;
							break;

						default:
							BOOST_ASSERT(false);
							gs_input_type_ = 0;
							gs_input_vertices = 0;
							break;
						}

						switch (cgGetProgramOutput(shaders[type]))
						{
						case CG_POINT_OUT:
							gs_output_type_ = GL_POINTS;
							break;

						case CG_LINE_OUT:
							gs_output_type_ = GL_LINE_STRIP;
							break;

						case CG_TRIANGLE_OUT:
							gs_output_type_ = GL_TRIANGLE_STRIP;
							break;

						default:
							BOOST_ASSERT(false);
							gs_output_type_ = 0;
							break;
						}
					}

					(*glsl_srcs_)[type] = this->ConvertToGLSL(cgGetProgramString(shaders[type], CG_COMPILED_PROGRAM),
						static_cast<ShaderType>(type), gs_input_vertices, has_gs);
					char const * glsl = (*glsl_srcs_)[type].c_str();
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

					if (ST_GeometryShader == type)
					{
						glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_INPUT_TYPE_EXT, gs_input_type_);
						glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_OUTPUT_TYPE_EXT, gs_output_type_);

						int temp;
						glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
						glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
					}

					glDeleteShader(object);
				}
			}

			is_validate_ &= is_shader_validate_[type];
		}

		if (is_validate_)
		{
			if (glloader_GL_VERSION_4_1() || glloader_GL_ARB_get_program_binary())
			{
				glProgramParameteri(glsl_program_, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
			}

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

			if (is_validate_ && (glloader_GL_VERSION_4_1() || glloader_GL_ARB_get_program_binary()))
			{
				GLint formats = 0;
				glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
				glsl_bin_formats_ = MakeSharedPtr<std::vector<GLint> >(formats);
				glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &(*glsl_bin_formats_)[0]);

				GLint len = 0;
				glGetProgramiv(glsl_program_, GL_PROGRAM_BINARY_LENGTH, &len);
				glsl_bin_program_ = MakeSharedPtr<std::vector<uint8_t> >(len);
				glGetProgramBinary(glsl_program_, len, NULL, reinterpret_cast<GLenum*>(&(*glsl_bin_formats_)[0]), &(*glsl_bin_program_)[0]);
			}

			for (int type = 0; type < ST_NumShaderTypes; ++ type)
			{
				CGprogram sub_prog = shaders[type];
				if (shaders[type] != 0)
				{
					CGparameter cg_param = cgGetFirstParameter(sub_prog, CG_GLOBAL);
					while (cg_param)
					{
						if (cgIsParameterUsed(cg_param, sub_prog)
							&& (CG_PARAMETERCLASS_OBJECT != cgGetParameterClass(cg_param)))
						{
							char const * pname = cgGetParameterName(cg_param);

							char const * glsl_param_name = cgGetParameterResourceName(cg_param);
							std::string hacked_name = std::string("_") + pname;

							if ((cgGetError() != CG_NO_ERROR) || (NULL == glsl_param_name))
							{
								// Some times cgGetParameterResourceName doesn't work
								glsl_param_name = hacked_name.c_str();
							}

							GLint location = glGetUniformLocation(glsl_program_, glsl_param_name);
							if (location != -1)
							{
								RenderEffectParameterPtr const & p = effect.ParameterByName(pname);
								if (p)
								{
									param_binds_.push_back(this->GetBindFunc(location, p));
								}
								else
								{
									for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
									{
										if (tex_sampler_binds_[i].first == pname)
										{
											parameter_bind_t pb;
											pb.combined_sampler_name = tex_sampler_binds_[i].first;
											pb.location = location;
											pb.shader_type = type;
											pb.tex_sampler_bind_index = static_cast<int>(i);

											uint32_t index = static_cast<uint32_t>(samplers_.size());
											samplers_.resize(index + 1);

											pb.func = SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_, location,
												index, tex_sampler_binds_[i].second.first, tex_sampler_binds_[i].second.second);

											param_binds_.push_back(pb);

											break;
										}
									}
								}
							}
						}

						cg_param = cgGetNextParameter(cg_param);
					}

					if (0 == type)
					{
						cg_param = cgGetFirstParameter(sub_prog, CG_PROGRAM);
						while (cg_param)
						{
							if (cgIsParameterUsed(cg_param, sub_prog)
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

								attrib_locs_.insert(std::make_pair(std::make_pair(usage, usage_index),
									glGetAttribLocation(glsl_program_, glsl_param_name.c_str())));
							}

							cg_param = cgGetNextParameter(cg_param);
						}
					}

					cgDestroyProgram(shaders[type]);
				}
			}
		}
	}

	ShaderObjectPtr OGLShaderObject::Clone(RenderEffect& effect)
	{
		OGLShaderObjectPtr ret = MakeSharedPtr<OGLShaderObject>();

		ret->shader_desc_ids_ = shader_desc_ids_;
		ret->shader_text_ = shader_text_;
		ret->glsl_bin_formats_ = glsl_bin_formats_;
		ret->glsl_bin_program_ = glsl_bin_program_;
		ret->glsl_srcs_ = glsl_srcs_;
		ret->gs_input_type_ = gs_input_type_;
		ret->gs_output_type_ = gs_output_type_;

		ret->tex_sampler_binds_.resize(tex_sampler_binds_.size());
		for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			ret->tex_sampler_binds_[i].first = tex_sampler_binds_[i].first;
			ret->tex_sampler_binds_[i].second.first = effect.ParameterByName(*(tex_sampler_binds_[i].second.first->Name()));
			ret->tex_sampler_binds_[i].second.second = effect.ParameterByName(*(tex_sampler_binds_[i].second.second->Name()));
		}

		ret->glsl_program_ = glCreateProgram();

		bool use_bin_program = true;

		// Binary program can't work with GS in current NVIDIA's drivers
		if (is_shader_validate_[ST_GeometryShader])
		{
			shader_desc& sd = effect.GetShaderDesc((*shader_desc_ids_)[ST_GeometryShader]);
			if (!sd.func_name.empty())
			{
				use_bin_program = false;
			}
		}

		if (use_bin_program && (glloader_GL_VERSION_4_1() || glloader_GL_ARB_get_program_binary()) && glsl_bin_program_)
		{
			ret->is_validate_ = is_validate_;
			for (size_t type = 0; type < ST_NumShaderTypes; ++ type)
			{
				ret->is_shader_validate_[type] = is_shader_validate_[type];
			}

			if (is_validate_)
			{
				glProgramBinary(ret->glsl_program_, static_cast<GLenum>((*glsl_bin_formats_)[0]),
					&(*glsl_bin_program_)[0], static_cast<GLsizei>(glsl_bin_program_->size()));

				if (ret->is_shader_validate_[ST_GeometryShader])
				{
					shader_desc& sd = effect.GetShaderDesc((*ret->shader_desc_ids_)[ST_GeometryShader]);
					if (!sd.func_name.empty())
					{
						glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_INPUT_TYPE_EXT, ret->gs_input_type_);
						glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_OUTPUT_TYPE_EXT, ret->gs_output_type_);

						int temp;
						glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
						glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
					}
				}
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
					shader_desc& sd = effect.GetShaderDesc((*ret->shader_desc_ids_)[type]);
					if (!sd.func_name.empty())
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

						default:
							shader_type = 0;
							BOOST_ASSERT(false);
							break;
						}

						char const * glsl = (*glsl_srcs_)[type].c_str();
						GLuint object = glCreateShader(shader_type);
						if (0 == object)
						{
							ret->is_shader_validate_[type] = false;
						}

						glShaderSource(object, 1, &glsl, NULL);

						glCompileShader(object);

						GLint compiled = false;
						glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);
#ifdef KLAYGE_DEBUG
						if (!compiled)
						{
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
						ret->is_shader_validate_[type] &= compiled ? true : false;

						glAttachShader(ret->glsl_program_, object);

						if (ST_GeometryShader == type)
						{
							glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_INPUT_TYPE_EXT, ret->gs_input_type_);
							glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_OUTPUT_TYPE_EXT, ret->gs_output_type_);

							int temp;
							glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
							glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
						}

						glDeleteShader(object);
					}
				}

				ret->is_validate_ &= ret->is_shader_validate_[type];
			}

			if (ret->is_validate_)
			{
				glLinkProgram(ret->glsl_program_);

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
						std::cerr << &info[0] << std::endl;
					}
				}
#endif
				ret->is_validate_ &= linked ? true : false;
			}
		}

		if (ret->is_validate_)
		{
			ret->attrib_locs_ = attrib_locs_;

			BOOST_FOREACH(BOOST_TYPEOF(param_binds_)::reference pb, param_binds_)
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

							new_pb.func = SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(ret->samplers_,
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

	GLint OGLShaderObject::GetAttribLocation(VertexElementUsage usage, uint8_t usage_index)
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

	OGLShaderObject::parameter_bind_t OGLShaderObject::GetBindFunc(GLint location, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.location = location;

		switch (param->type())
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

	void OGLShaderObject::Bind()
	{
		glUseProgram(glsl_program_);

		BOOST_FOREACH(BOOST_TYPEOF(param_binds_)::reference pb, param_binds_)
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

	void OGLShaderObject::Unbind()
	{
		//glUseProgram(0);
	}
}
