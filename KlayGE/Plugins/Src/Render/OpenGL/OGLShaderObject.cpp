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
	\n																	\
	\n																	\
	float4 tex1DARRAYLevel(sampler1D s, float2 location, float lod)\n	\
	{\n																	\
		return tex1DARRAYlod(s, float4(location, 0, lod));\n			\
	}\n																	\
	\n																	\
	float4 tex2DARRAYLevel(sampler2D s, float3 location, float lod)\n	\
	{\n																	\
		return tex2DARRAYlod(s, float4(location, lod));\n			\
	}\n																	\
	\n																	\
	float4 tex3DARRAYLevel(sampler3D s, float3 location, float lod)\n	\
	{\n																	\
		return tex3DARRAYlod(s, float4(location, lod));\n				\
	}\n																	\
	\n																	\
	float4 texCUBEARRAYLevel(samplerCUBE s, float3 location, float lod)\n\
	{\n																	\
		return texCUBEARRAYlod(s, float4(location, lod));\n				\
	}\n																	\
	\n																	\
	\n																	\
	float4 tex1DARRAYBias(sampler1D s, float2 location, float lod)\n	\
	{\n																	\
		return tex1DARRAYbias(s, float4(location, 0, lod));\n			\
	}\n																	\
	\n																	\
	float4 tex2DARRAYBias(sampler2D s, float3 location, float lod)\n	\
	{\n																	\
		return tex2DARRAYbias(s, float4(location, lod));\n				\
	}\n																	\
	\n																	\
	float4 tex3DARRAYBias(sampler3D s, float3 location, float lod)\n	\
	{\n																	\
		return tex3DARRAYbias(s, float4(location, lod));\n				\
	}\n																	\
	\n																	\
	float4 texCUBEARRAYBias(samplerCUBE s, float3 location, float lod)\n\
	{\n																	\
		return texCUBEARRAYbias(s, float4(location, lod));\n			\
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
		"varying out vec4 v_gl_FrontColor;\n"\
		"varying out vec4 v_gl_BackColor;\n"\
		"varying out vec4 v_gl_FrontSecondaryColor;\n"\
		"varying out vec4 v_gl_BackSecondaryColor;\n"\
		"varying out vec4 v_gl_TexCoord[8];\n"\
		"varying out float v_gl_FogFragCoord;\n";

	char const * predefined_ps_out_varyings = "varying out vec4 v_gl_FragData_%d;\n";

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
			re.Uniform4fv(location_, 4, &v[0]);
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
				re.Uniform4fv(location_, static_cast<long>(v.size()) * 4, &v[0][0]);
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

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if ((*samplers_)[stage_].first)
			{
				re.ActiveTexture(GL_TEXTURE0 + stage_);
				checked_pointer_cast<OGLSamplerStateObject>((*samplers_)[stage_].second)->Active((*samplers_)[stage_].first);
				GLuint const tex_type = checked_pointer_cast<OGLTexture>((*samplers_)[stage_].first)->GLType();
				GLuint const gl_tex = checked_pointer_cast<OGLTexture>((*samplers_)[stage_].first)->GLTexture();
				glBindTexture(tex_type, gl_tex);

				re.Uniform1i(location_, stage_);
			}
			else
			{
				re.ActiveTexture(GL_TEXTURE0 + stage_);
				glBindTexture(GL_TEXTURE_2D, 0);

				re.Uniform1i(location_, stage_);
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
		has_discard_ = false;
		has_tessellation_ = false;
		is_shader_validate_.assign(true);

		glsl_program_ = glCreateProgram();

		glsl_srcs_ = MakeSharedPtr<boost::array<boost::shared_ptr<std::string>, ST_NumShaderTypes> >();

		pnames_ = MakeSharedPtr<boost::array<boost::shared_ptr<std::vector<std::string> >, ST_NumShaderTypes> >();
		glsl_res_names_ = MakeSharedPtr<boost::array<boost::shared_ptr<std::vector<std::string> >, ST_NumShaderTypes> >();

		vs_usages_ = MakeSharedPtr<std::vector<VertexElementUsage> >();
		vs_usage_indices_ = MakeSharedPtr<std::vector<uint8_t> >();
		glsl_vs_attrib_names_ = MakeSharedPtr<std::vector<std::string> >();
	}

	OGLShaderObject::~OGLShaderObject()
	{
		glDeleteProgram(glsl_program_);
	}

	std::string OGLShaderObject::GenShaderText(ShaderType type, RenderEffect const & effect)
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
				for (KLAYGE_AUTO(beg, tok.begin()); beg != tok.end();)
				{
					this_token = *beg;

					RenderEffectParameterPtr const & param = effect.ParameterByName(this_token);
					if (param &&
						((REDT_texture1D == param->type()) || (REDT_texture2D == param->type()) || (REDT_texture3D == param->type())
							|| (REDT_textureCUBE == param->type())
							|| (REDT_texture1DArray == param->type()) || (REDT_texture2DArray == param->type()) || (REDT_texture3DArray == param->type())
							|| (REDT_textureCUBEArray == param->type())))
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
								if (tex_sampler_binds_[j].get<0>() == combined_sampler_name)
								{
									tex_sampler_binds_[j].get<3>() |= 1UL << type;
									found = true;
									break;
								}
							}
							if (!found)
							{
								tex_sampler_binds_.push_back(boost::make_tuple(combined_sampler_name,
									param, effect.ParameterByName(sample_tokens[4]), 1UL << type));
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
							case REDT_texture1DArray:
							case REDT_texture2DArray:
							case REDT_texture3DArray:
							case REDT_textureCUBEArray:
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

								case REDT_texture1DArray:
									shader_ss << "1DARRAY";
									break;

								case REDT_texture2DArray:
									shader_ss << "2DARRAY";
									break;

								case REDT_texture3DArray:
									shader_ss << "3DARRAY";
									break;

								case REDT_textureCUBEArray:
									shader_ss << "CUBEARRAY";
									break;

								default:
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

		KLAYGE_AUTO(cbuffers, effect.CBuffers());
		typedef KLAYGE_DECLTYPE(cbuffers) CBuffersType;
		KLAYGE_FOREACH(CBuffersType::const_reference cbuff, cbuffers)
		{
			typedef KLAYGE_DECLTYPE(cbuff.second) CBuffersSecondType;
			KLAYGE_FOREACH(CBuffersSecondType::const_reference param_index, cbuff.second)
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
			RenderEffectParameterPtr const & param = tex_sampler_binds_[i].get<1>();
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

			default:
				break;

			/*case REDT_texture1DArray:
				ss << "1DARRAY";
				break;

			case REDT_texture2DArray:
				ss << "2DARRAY";
				break;

			case REDT_texture3DArray:
				ss << "3DARRAY";
				break;

			case REDT_textureCUBEArray:
				ss << "CUBEARRAY";
				break;

			default:
				BOOST_ASSERT(false);
				break;*/
			}
			ss << " " << tex_sampler_binds_[i].get<0>() << ";" << std::endl;
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

		std::string predefined_ps_out_varyings_add_num;
		int max_mrt = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().max_simultaneous_rts;
		for (int i = 0; i < max_mrt; ++ i)
		{
			char temp[256];
			sprintf(temp, predefined_ps_out_varyings, i);
			predefined_ps_out_varyings_add_num += temp;
		}
		predefined_ps_out_varyings_add_num += "\n";

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
			ss << predefined_varyings << std::endl;
			if (glloader_GL_VERSION_4_2())
			{
				ss << predefined_ps_out_varyings_add_num << std::endl;
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		boost::char_separator<char> sep("", " \t\n.,():;+-*/%&!|^[]{}'\"?");
		boost::tokenizer<boost::char_separator<char> > tok(glsl, sep);
		std::string this_token;
		for (KLAYGE_AUTO(beg, tok.begin()); beg != tok.end(); ++ beg)
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
				}
				else
				{
					if ("gl_Color" == this_token)
					{
						ss << "v_gl_FrontColor";
					}
					else
					{
						if ("gl_SecondaryColor" == this_token)
						{
							ss << "v_gl_FrontSecondaryColor";
						}
						else
						{
							if ("gl_FragColor" == this_token)
							{
								if (glloader_GL_VERSION_4_2())
								{
									ss << "v_gl_FragData_0";
								}
								else
								{
									ss << this_token;
								}
							}
							else
							{
								if ("gl_FragData" == this_token)
								{
									if (glloader_GL_VERSION_3_1())
									{
										std::string tmp_token[3];
										for (int t = 0; t < 3; ++ t)
										{
											++ beg;
											tmp_token[t] = *beg;
										}

										ss << "v_gl_FragData_" << tmp_token[1];
									}
									else
									{
										ss << this_token;
									}
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

	bool OGLShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & /*shader_desc_ids*/,
			std::vector<uint8_t> const & native_shader_block)
	{
		bool ret = false;

		if (native_shader_block.size() >= 24)
		{
			uint8_t const * nsbp = &native_shader_block[0];
			uint32_t fourcc;
			memcpy(&fourcc, nsbp, sizeof(fourcc));
			nsbp += sizeof(fourcc);
			LittleEndianToNative<sizeof(fourcc)>(&fourcc);
			if (MakeFourCC<'G', 'L', 'S', 'L'>::value == fourcc)
			{
				uint32_t ver;
				memcpy(&ver, nsbp, sizeof(ver));
				nsbp += sizeof(ver);
				LittleEndianToNative<sizeof(ver)>(&ver);
				if (1 == ver)
				{
					uint64_t timestamp;
					memcpy(&timestamp, nsbp, sizeof(timestamp));
					nsbp += sizeof(timestamp);
					LittleEndianToNative<sizeof(timestamp)>(&timestamp);
					if (timestamp >= effect.Timestamp())
					{
						uint64_t hash_val;
						memcpy(&hash_val, nsbp, sizeof(hash_val));
						nsbp += sizeof(timestamp);
						LittleEndianToNative<sizeof(hash_val)>(&hash_val);
						if (effect.PredefinedMacrosHash() == hash_val)
						{
							uint32_t len32;
							memcpy(&len32, nsbp, sizeof(len32));
							nsbp += sizeof(len32);
							LittleEndianToNative<sizeof(len32)>(&len32);
							(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(len32, '\0');
							memcpy(&(*(*glsl_srcs_)[type])[0], nsbp, len32);
							nsbp += len32;

							uint16_t num16;
							memcpy(&num16, nsbp, sizeof(num16));
							nsbp += sizeof(num16);
							LittleEndianToNative<sizeof(num16)>(&num16);
							(*pnames_)[type] = MakeSharedPtr<std::vector<std::string> >(num16);
							for (size_t i = 0; i < num16; ++ i)
							{
								uint8_t len8;
								memcpy(&len8, nsbp, sizeof(len8));
								nsbp += sizeof(len8);
											
								(*(*pnames_)[type])[i].resize(len8);
								memcpy(&(*(*pnames_)[type])[i][0], nsbp, len8);
								nsbp += len8;
							}

							memcpy(&num16, nsbp, sizeof(num16));
							nsbp += sizeof(num16);
							LittleEndianToNative<sizeof(num16)>(&num16);
							(*glsl_res_names_)[type] = MakeSharedPtr<std::vector<std::string> >(num16);
							for (size_t i = 0; i < num16; ++ i)
							{
								uint8_t len8;
								memcpy(&len8, nsbp, sizeof(len8));
								nsbp += sizeof(len8);

								(*(*glsl_res_names_)[type])[i].resize(len8);
								memcpy(&(*(*glsl_res_names_)[type])[i][0], nsbp, len8);
								nsbp += len8;
							}

							memcpy(&num16, nsbp, sizeof(num16));
							nsbp += sizeof(num16);
							LittleEndianToNative<sizeof(num16)>(&num16);
							for (size_t i = 0; i < num16; ++ i)
							{
								uint8_t len8;
								memcpy(&len8, nsbp, sizeof(len8));
								nsbp += sizeof(len8);

								std::string tex_name;
								tex_name.resize(len8);
								memcpy(&tex_name[0], nsbp, len8);
								nsbp += len8;

								memcpy(&len8, nsbp, sizeof(len8));
								nsbp += sizeof(len8);

								std::string sampler_name;
								sampler_name.resize(len8);
								memcpy(&sampler_name[0], nsbp, len8);
								nsbp += len8;

								std::string combined_sampler_name = tex_name + "__" + sampler_name;

								bool found = false;
								for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
								{
									if (tex_sampler_binds_[k].get<0>() == combined_sampler_name)
									{
										tex_sampler_binds_[k].get<3>() |= 1UL << type;
										found = true;
										break;
									}
								}
								if (!found)
								{
									tex_sampler_binds_.push_back(boost::make_tuple(combined_sampler_name,
										effect.ParameterByName(tex_name), effect.ParameterByName(sampler_name), 1UL << type));
								}
							}

							if (ST_VertexShader == type)
							{
								uint8_t num8;
								memcpy(&num8, nsbp, sizeof(num8));
								nsbp += sizeof(num8);
								vs_usages_->resize(num8);
								for (size_t i = 0; i < num8; ++ i)
								{
									uint8_t veu;
									memcpy(&veu, nsbp, sizeof(veu));
									nsbp += sizeof(veu);

									(*vs_usages_)[i] = static_cast<VertexElementUsage>(veu);
								}

								memcpy(&num8, nsbp, sizeof(num8));
								nsbp += sizeof(num8);
								vs_usage_indices_->resize(num8);
								memcpy(&(*vs_usage_indices_)[0], nsbp, num8 * sizeof((*vs_usage_indices_)[0]));
								nsbp += num8 * sizeof((*vs_usage_indices_)[0]);

								memcpy(&num8, nsbp, sizeof(num8));
								nsbp += sizeof(num8);
								glsl_vs_attrib_names_->resize(num8);
								for (size_t i = 0; i < num8; ++ i)
								{
									uint8_t len8;
									memcpy(&len8, nsbp, sizeof(len8));
									nsbp += sizeof(len8);

									(*glsl_vs_attrib_names_)[i].resize(len8);
									memcpy(&(*glsl_vs_attrib_names_)[i][0], nsbp, len8);
									nsbp += len8;
								}
							}
							else if (ST_GeometryShader == type)
							{
								memcpy(&gs_input_type_, nsbp, sizeof(gs_input_type_));
								nsbp += sizeof(gs_input_type_);
								LittleEndianToNative<sizeof(gs_input_type_)>(&gs_input_type_);

								memcpy(&gs_output_type_, nsbp, sizeof(gs_output_type_));
								nsbp += sizeof(gs_output_type_);
								LittleEndianToNative<sizeof(gs_output_type_)>(&gs_output_type_);
							}

							this->AttachGLSL(type);

							ret = true;
						}
					}
				}
			}
		}

		return ret;
	}

	void OGLShaderObject::ExtractNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint8_t>& native_shader_block)
	{
		native_shader_block.clear();

		if ((*glsl_srcs_)[type])
		{
			std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

			uint32_t fourcc = MakeFourCC<'G', 'L', 'S', 'L'>::value;
			NativeToLittleEndian<sizeof(fourcc)>(&fourcc);
			oss.write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));

			uint32_t ver = 1;
			NativeToLittleEndian<sizeof(ver)>(&ver);
			oss.write(reinterpret_cast<char const *>(&ver), sizeof(ver));

			uint64_t timestamp = effect.Timestamp();
			NativeToLittleEndian<sizeof(timestamp)>(&timestamp);
			oss.write(reinterpret_cast<char const *>(&timestamp), sizeof(timestamp));

			uint64_t hash_val = effect.PredefinedMacrosHash();
			NativeToLittleEndian<sizeof(hash_val)>(&hash_val);
			oss.write(reinterpret_cast<char const *>(&hash_val), sizeof(hash_val));

			uint32_t len32 = static_cast<uint32_t>((*glsl_srcs_)[type]->size());
			NativeToLittleEndian<sizeof(len32)>(&len32);
			oss.write(reinterpret_cast<char const *>(&len32), sizeof(len32));
			oss.write(&(*(*glsl_srcs_)[type])[0], (*glsl_srcs_)[type]->size());

			uint16_t num16 = static_cast<uint16_t>((*pnames_)[type]->size());
			NativeToLittleEndian<sizeof(num16)>(&num16);
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < (*pnames_)[type]->size(); ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>((*(*pnames_)[type])[i].size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&(*(*pnames_)[type])[i][0], (*(*pnames_)[type])[i].size());
			}

			num16 = static_cast<uint16_t>((*glsl_res_names_)[type]->size());
			NativeToLittleEndian<sizeof(num16)>(&num16);
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < (*glsl_res_names_)[type]->size(); ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>((*(*glsl_res_names_)[type])[i].size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&(*(*glsl_res_names_)[type])[i][0], (*(*glsl_res_names_)[type])[i].size());
			}

			std::vector<std::pair<std::string, std::string> > tex_sampler_pairs;
			for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
			{
				if (tex_sampler_binds_[i].get<3>() | (1UL << type))
				{
					tex_sampler_pairs.push_back(std::make_pair(*tex_sampler_binds_[i].get<1>()->Name(),
						*tex_sampler_binds_[i].get<2>()->Name()));
				}
			}

			num16 = static_cast<uint16_t>(tex_sampler_pairs.size());
			NativeToLittleEndian<sizeof(num16)>(&num16);
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
				oss.write(reinterpret_cast<char const *>(&(*vs_usage_indices_)[0]), vs_usage_indices_->size() * sizeof((*vs_usage_indices_)[0]));

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
				uint32_t git = gs_input_type_;
				NativeToLittleEndian<sizeof(git)>(&git);
				oss.write(reinterpret_cast<char const *>(&git), sizeof(git));

				uint32_t got = gs_output_type_;
				NativeToLittleEndian<sizeof(got)>(&got);
				oss.write(reinterpret_cast<char const *>(&got), sizeof(got));
			}

			std::string out_str = oss.str();
			native_shader_block.resize(out_str.size());
			memcpy(&native_shader_block[0], &out_str[0], out_str.size());
		}
	}

	void OGLShaderObject::AttachShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids)
	{
		shader_desc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		OGLRenderFactory& rf = *checked_cast<OGLRenderFactory*>(&Context::Instance().RenderFactoryInstance());
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
		std::string flipping_str;
		{
			std::stringstream ss;
			ss << "-DKLAYGE_FLIPPING=" << (re.RequiresFlipping() ? -1 : +1);
			flipping_str = ss.str();
		}
		std::string standard_derivatives_str;
		{
			std::stringstream ss;
			ss << "-DKLAYGE_DERIVATIVES=" << (caps.standard_derivatives_support ? 1 : 0);
			standard_derivatives_str = ss.str();
		}

		std::string shader_text = this->GenShaderText(type, effect);

		std::vector<char const *> args;
		args.push_back("-DKLAYGE_OPENGL=1");
		args.push_back(max_sm_str.c_str());
		args.push_back(max_tex_array_str.c_str());
		args.push_back(max_tex_depth_str.c_str());
		args.push_back(max_tex_units_str.c_str());
		args.push_back(nullptr);
		args.push_back(flipping_str.c_str());
		args.push_back(standard_derivatives_str.c_str());
		if (!re.DeviceCaps().texture_format_support(EF_BC5))
		{
			args.push_back("-DKLAYGE_BC5_AS_AG");
		}
		else
		{
			args.push_back("-DKLAYGE_BC5_AS_GA");
		}
		args.push_back(nullptr);

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
			break;

		default:
			is_shader_validate_[type] = false;
			break;
		}

		if (is_shader_validate_[type])
		{		
			CGprofile profile;
			switch (type)
			{
			case ST_VertexShader:
				profile = CG_PROFILE_GLSLV;
				break;

			case ST_PixelShader:
				profile = CG_PROFILE_GLSLF;
				break;

			case ST_GeometryShader:
				profile = CG_PROFILE_GLSLG;
				break;

			default:
				profile = CG_PROFILE_UNKNOWN;
				break;
			}

			std::string no_tex_lod_str;
			{
				std::stringstream ss;
				ss << "-DKLAYGE_NO_TEX_LOD=" << ((ST_VertexShader == type) ? 0 : ((glloader_GL_VERSION_2_1() || glloader_GL_ARB_shader_texture_lod()) ? 0 : 1));
				no_tex_lod_str = ss.str();
			}
			args[5] = no_tex_lod_str.c_str();

			CGprogram cg_shader = cgCreateProgram(CGContextIniter::Instance().Context(),
					CG_SOURCE, shader_text.c_str(), profile, sd.func_name.c_str(), &args[0]);

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
						LogError("%d %s", line, s.c_str());
						++ line;
					}
					LogError("Error when compiling %s:", sd.func_name.c_str());
					LogError(cgGetErrorString(error));

					char const* listing = cgGetLastListing(CGContextIniter::Instance().Context());
					if (listing)
					{
						LogError(listing);
					}
				}
#endif

				is_shader_validate_[type] = false;
			}
			else
			{
				uint32_t gs_input_vertices = 0;
				if (ST_GeometryShader == type)
				{
					switch (cgGetProgramInput(cg_shader))
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

					switch (cgGetProgramOutput(cg_shader))
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

				(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(this->ConvertToGLSL(cgGetProgramString(cg_shader, CG_COMPILED_PROGRAM),
					static_cast<ShaderType>(type), gs_input_vertices, has_gs));
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

						if ((cgGetError() != CG_NO_ERROR) || (nullptr == glsl_param_name))
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
		}

		if (is_shader_validate_[type])
		{
			this->AttachGLSL(type);
		}
	}

	void OGLShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/, ShaderObjectPtr const & shared_so)
	{
		OGLShaderObjectPtr so = checked_pointer_cast<OGLShaderObject>(shared_so);

		is_shader_validate_[type] = so->is_shader_validate_[type];

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
			}
			else if (ST_PixelShader == type)
			{
				has_discard_ = so->has_discard_;
			}

			for (uint32_t j = 0; j < so->tex_sampler_binds_.size(); ++ j)
			{
				if (so->tex_sampler_binds_[j].get<3>() | (1UL << type))
				{
					std::string const & combined_sampler_name = so->tex_sampler_binds_[j].get<0>();
					bool found = false;
					for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
					{
						if (tex_sampler_binds_[k].get<0>() == combined_sampler_name)
						{
							tex_sampler_binds_[k].get<3>() |= 1UL << type;
							found = true;
							break;
						}
					}
					if (!found)
					{
						tex_sampler_binds_.push_back(boost::make_tuple(combined_sampler_name,
							so->tex_sampler_binds_[j].get<1>(), so->tex_sampler_binds_[j].get<2>(), 1UL << type));
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
			if (!(*glsl_srcs_)[type])
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

			if (is_validate_ && (glloader_GL_VERSION_4_1() || glloader_GL_ARB_get_program_binary()))
			{
				GLint formats = 0;
				glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
				glsl_bin_formats_ = MakeSharedPtr<std::vector<GLint> >(formats);
				if (formats > 0)
				{
					glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &(*glsl_bin_formats_)[0]);

					GLint len = 0;
					glGetProgramiv(glsl_program_, GL_PROGRAM_BINARY_LENGTH, &len);
					glsl_bin_program_ = MakeSharedPtr<std::vector<uint8_t> >(len);
					glGetProgramBinary(glsl_program_, len, nullptr, reinterpret_cast<GLenum*>(&(*glsl_bin_formats_)[0]), &(*glsl_bin_program_)[0]);
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
									if (tex_sampler_binds_[i].get<0>() == (*(*pnames_)[type])[pi])
									{
										parameter_bind_t pb;
										pb.combined_sampler_name = tex_sampler_binds_[i].get<0>();
										pb.location = location;
										pb.shader_type = type;
										pb.tex_sampler_bind_index = static_cast<int>(i);

										uint32_t index = static_cast<uint32_t>(samplers_.size());
										samplers_.resize(index + 1);

										pb.func = SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_, location,
											index, tex_sampler_binds_[i].get<1>(), tex_sampler_binds_[i].get<2>());

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

	ShaderObjectPtr OGLShaderObject::Clone(RenderEffect const & effect)
	{
		OGLShaderObjectPtr ret = MakeSharedPtr<OGLShaderObject>();

		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;
		ret->glsl_bin_formats_ = glsl_bin_formats_;
		ret->glsl_bin_program_ = glsl_bin_program_;
		ret->glsl_srcs_ = glsl_srcs_;
		ret->pnames_ = pnames_;
		ret->glsl_res_names_ = glsl_res_names_;
		ret->vs_usages_ = vs_usages_;
		ret->vs_usage_indices_ = vs_usage_indices_;
		ret->glsl_vs_attrib_names_ = glsl_vs_attrib_names_;
		ret->gs_input_type_ = gs_input_type_;
		ret->gs_output_type_ = gs_output_type_;

		ret->tex_sampler_binds_.resize(tex_sampler_binds_.size());
		for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			ret->tex_sampler_binds_[i].get<0>() = tex_sampler_binds_[i].get<0>();
			ret->tex_sampler_binds_[i].get<1>() = effect.ParameterByName(*(tex_sampler_binds_[i].get<1>()->Name()));
			ret->tex_sampler_binds_[i].get<2>() = effect.ParameterByName(*(tex_sampler_binds_[i].get<2>()->Name()));
			ret->tex_sampler_binds_[i].get<3>() = tex_sampler_binds_[i].get<3>();
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
				if (ret->is_shader_validate_[ST_GeometryShader])
				{
					if ((*glsl_srcs_)[ST_GeometryShader] && !(*glsl_srcs_)[ST_GeometryShader]->empty())
					{
						glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_INPUT_TYPE_EXT, ret->gs_input_type_);
						glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_OUTPUT_TYPE_EXT, ret->gs_output_type_);

						// TODO: read maxvertexcount from shader
						glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_VERTICES_OUT_EXT, 32);
					}
				}

				glProgramParameteri(ret->glsl_program_, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);

				glProgramBinary(ret->glsl_program_, static_cast<GLenum>((*glsl_bin_formats_)[0]),
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
			ret->attrib_locs_ = attrib_locs_;

			typedef KLAYGE_DECLTYPE(param_binds_) ParamBindsType;
			KLAYGE_FOREACH(ParamBindsType::reference pb, param_binds_)
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
						if (ret->tex_sampler_binds_[j].get<0>() == pname)
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
								ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index].get<1>(),
								ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index].get<2>());

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
		KLAYGE_AUTO(iter, attrib_locs_.find(std::make_pair(usage, usage_index)));
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

		glShaderSource(object, 1, &glsl, nullptr);

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
				LogError(&info[0]);
			}
		}
#endif
		is_shader_validate_[type] &= compiled ? true : false;

		glAttachShader(glsl_program_, object);

		if (ST_GeometryShader == type)
		{
			glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_INPUT_TYPE_EXT, gs_input_type_);
			glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_OUTPUT_TYPE_EXT, gs_output_type_);

			// TODO: read maxvertexcount from shader
			glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_VERTICES_OUT_EXT, 32);
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
		is_validate_ &= linked ? true : false;
	}

	void OGLShaderObject::Bind()
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.UseProgram(glsl_program_);

		typedef KLAYGE_DECLTYPE(param_binds_) ParamBindsType;
		KLAYGE_FOREACH(ParamBindsType::reference pb, param_binds_)
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
				LogError(&info[0]);
			}
		}
#endif
	}

	void OGLShaderObject::Unbind()
	{
		//glUseProgram(0);
	}
}
