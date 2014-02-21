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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 6328)
#endif
#include <boost/tokenizer.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <glloader/glloader.h>

#if USE_DXBC2GLSL
#include <DXBC2GLSL/DXBC2GLSL.hpp>
#include <D3DCompiler.h>
#else
#include <Cg/cg.h>
#endif

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderFactoryInternal.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

#if !USE_DXBC2GLSL
#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "Cg.lib")
#endif
#endif

namespace
{
	using namespace KlayGE;

#if !USE_DXBC2GLSL
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
#endif

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
#if USE_DXBC2GLSL
			re.UniformMatrix4fv(location_, 1, false, &v[0]);
#else
			re.Uniform4fv(location_, 4, &v[0]);
#endif
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
#if USE_DXBC2GLSL
				re.UniformMatrix4fv(location_, static_cast<GLsizei>(v.size()), false, &v[0][0]);
#else
				re.Uniform4fv(location_, static_cast<GLsizei>(v.size()) * 4, &v[0][0]);
#endif
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
		: gs_input_type_(0), gs_output_type_(0), gs_max_output_vertex_(0)
	{
		has_discard_ = false;
		has_tessellation_ = false;
		is_shader_validate_.fill(true);

		glsl_program_ = glCreateProgram();

		shader_func_names_ = MakeSharedPtr<array<std::string, ST_NumShaderTypes> >();
		glsl_srcs_ = MakeSharedPtr<array<shared_ptr<std::string>, ST_NumShaderTypes> >();

		pnames_ = MakeSharedPtr<array<shared_ptr<std::vector<std::string> >, ST_NumShaderTypes> >();
		glsl_res_names_ = MakeSharedPtr<array<shared_ptr<std::vector<std::string> >, ST_NumShaderTypes> >();

		vs_usages_ = MakeSharedPtr<std::vector<VertexElementUsage> >();
		vs_usage_indices_ = MakeSharedPtr<std::vector<uint8_t> >();
		glsl_vs_attrib_names_ = MakeSharedPtr<std::vector<std::string> >();
	}

	OGLShaderObject::~OGLShaderObject()
	{
		glDeleteProgram(glsl_program_);
	}

#if !USE_DXBC2GLSL
	std::string OGLShaderObject::GenCgShaderText(ShaderType type, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass)
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
					size_t const this_token_hash = RT_HASH(this_token.c_str());

					RenderEffectParameterPtr const & param = effect.ParameterByName(this_token);
					if (param &&
						((REDT_texture1D == param->Type()) || (REDT_texture2D == param->Type()) || (REDT_texture3D == param->Type())
							|| (REDT_textureCUBE == param->Type())
							|| (REDT_texture1DArray == param->Type()) || (REDT_texture2DArray == param->Type()) || (REDT_texture3DArray == param->Type())
							|| (REDT_textureCUBEArray == param->Type())))
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

							std::string combined_sampler_name = sample_tokens[0] + "_" + sample_tokens[4];
							bool found = false;
							for (uint32_t j = 0; j < tex_sampler_binds_.size(); ++ j)
							{
								if (get<0>(tex_sampler_binds_[j]) == combined_sampler_name)
								{
									get<3>(tex_sampler_binds_[j]) |= 1UL << type;
									found = true;
									break;
								}
							}
							if (!found)
							{
								tex_sampler_binds_.push_back(KlayGE::make_tuple(combined_sampler_name,
									param, effect.ParameterByName(sample_tokens[4]), 1UL << type));
							}

							if (!sample_helper && (("SampleLevel" == sample_tokens[2]) || ("SampleBias" == sample_tokens[2])))
							{
								sample_helper = true;
							}

							switch (param->Type())
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

								switch (param->Type())
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
						if (CT_HASH("SV_Position") == this_token_hash)
						{
							shader_ss << "POSITION";
						}
						else
						{
							if (CT_HASH("SV_Depth") == this_token_hash)
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
									if (CT_HASH("[") == this_token_hash)
									{
										++ beg;
										size_t const token_hash = RT_HASH((*beg).c_str());
										if ((CT_HASH("branch") == token_hash)
											|| (CT_HASH("flatten") == token_hash)
											|| (CT_HASH("forcecase") == token_hash)
											|| (CT_HASH("call") == token_hash)
											|| (CT_HASH("unroll") == token_hash)
											|| (CT_HASH("loop") == token_hash))
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

		for (uint32_t i = 0; i < tech.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = tech.MacroByIndex(i);
			ss << "#define " << name_value.first << " " << name_value.second << std::endl;
		}
		ss << std::endl;

		for (uint32_t i = 0; i < pass.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = pass.MacroByIndex(i);
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

				switch (param.Type())
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
					ss << effect.TypeName(param.Type()) << " " << *param.Name();
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
			RenderEffectParameterPtr const & param = get<1>(tex_sampler_binds_[i]);
			ss << "sampler";
			switch (param->Type())
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
			ss << " " << get<0>(tex_sampler_binds_[i]) << ";" << std::endl;
		}

		ss << shader_ss.str();

		return ss.str();
	}
#endif

#if USE_DXBC2GLSL
	std::string OGLShaderObject::GenHLSLShaderText(ShaderType type, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass) const
	{
		std::stringstream ss;

		for (uint32_t i = 0; i < effect.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = effect.MacroByIndex(i);
			ss << "#define " << name_value.first << " " << name_value.second << std::endl;
		}
		ss << std::endl;

		for (uint32_t i = 0; i < tech.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = tech.MacroByIndex(i);
			ss << "#define " << name_value.first << " " << name_value.second << std::endl;
		}
		ss << std::endl;

		for (uint32_t i = 0; i < pass.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = pass.MacroByIndex(i);
			ss << "#define " << name_value.first << " " << name_value.second << std::endl;
		}
		ss << std::endl;

		KLAYGE_AUTO(cbuffers, effect.CBuffers());
		typedef KLAYGE_DECLTYPE(cbuffers) CBuffersType;
		KLAYGE_FOREACH(CBuffersType::const_reference cbuff, cbuffers)
		{
			ss << "cbuffer " << cbuff.first << std::endl;
			ss << "{" << std::endl;

			typedef KLAYGE_DECLTYPE(cbuff.second) CBuffersSecondType;
			KLAYGE_FOREACH(CBuffersSecondType::const_reference param_index, cbuff.second)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(param_index);
				switch (param.Type())
				{
				case REDT_texture1D:
				case REDT_texture2D:
				case REDT_texture3D:
				case REDT_textureCUBE:
				case REDT_texture1DArray:
				case REDT_texture2DArray:
				case REDT_texture3DArray:
				case REDT_textureCUBEArray:
				case REDT_sampler:
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
					break;

				default:
					ss << effect.TypeName(param.Type()) << " " << *param.Name();
					if (param.ArraySize())
					{
						ss << "[" << *param.ArraySize() << "]";
					}
					ss << ";" << std::endl;
					break;
				}
			}

			ss << "};" << std::endl;
		}

		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		for (uint32_t i = 0; i < effect.NumParameters(); ++ i)
		{
			RenderEffectParameter& param = *effect.ParameterByIndex(i);

			switch (param.Type())
			{
			case REDT_texture1D:
			{
				std::string elem_type;
				param.Var()->Value(elem_type);
				ss << "Texture1D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
			}
				break;

			case REDT_texture2D:
			{
				std::string elem_type;
				param.Var()->Value(elem_type);
				ss << "Texture2D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
			}
				break;

			case REDT_texture3D:
			{
				std::string elem_type;
				param.Var()->Value(elem_type);
				ss << "Texture3D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
			}
				break;

			case REDT_textureCUBE:
			{
				std::string elem_type;
				param.Var()->Value(elem_type);
				ss << "TextureCube<" << elem_type << "> " << *param.Name() << ";" << std::endl;
			}
				break;

			case REDT_texture1DArray:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "Texture1DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_texture2DArray:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "Texture2DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_textureCUBEArray:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "TextureCubeArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_buffer:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "Buffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_sampler:
				ss << "sampler " << *param.Name() << ";" << std::endl;
				break;

			case REDT_structured_buffer:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "StructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_byte_address_buffer:
				if (caps.max_shader_model >= 4)
				{
					ss << "ByteAddressBuffer " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_buffer:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "RWBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_structured_buffer:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "RWStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_texture1D:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "RWTexture1D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_texture2D:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "RWTexture2D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_texture3D:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "RWTexture3D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;
			case REDT_rw_texture1DArray:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "RWTexture1DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_texture2DArray:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "RWTexture2DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_byte_address_buffer:
				if (caps.max_shader_model >= 4)
				{
					ss << "RWByteAddressBuffer " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_append_structured_buffer:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "AppendStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_consume_structured_buffer:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ss << "ConsumeStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			default:
				break;
			}
		}

		for (uint32_t i = 0; i < effect.NumShaders(); ++ i)
		{
			RenderShaderFunc const & effect_shader = effect.ShaderByIndex(i);
			ShaderType shader_type = effect_shader.Type();
			if ((ST_NumShaderTypes == shader_type) || (type == shader_type))
			{
				if (caps.max_shader_model >= effect_shader.Version())
				{
					ss << effect_shader.str() << std::endl;
				}
			}
		}

		return ss.str();
	}
#endif

#if !USE_DXBC2GLSL
	std::string OGLShaderObject::ConvertToGLSL(std::string const & glsl, ShaderType type, uint32_t gs_input_vertices, bool has_gs)
	{
		std::stringstream ss;
		
		{
			bool has_directive = false;
			std::stringstream iss(glsl);
			std::string s;
			while (iss)
			{
				std::getline(iss, s);

				if (0 == s.find("#version"))
				{
					has_directive = true;
					ss << "#version 120" << std::endl;
				}
				else if (0 == s.find("#extension"))
				{
					has_directive = true;
					ss << s << std::endl;
				}
				else if (!s.empty() && (s.find("//") != 0) && (s.find("/*") != 0))
				{
					break;
				}
			}
			if (has_directive)
			{
				ss << std::endl;
			}
		}

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
		for (KLAYGE_AUTO(beg, tok.begin()); beg != tok.end(); ++ beg)
		{
			std::string const & this_token = *beg;
			size_t const this_token_hash = RT_HASH(this_token.c_str());

			if (CT_HASH("#version") == this_token_hash)
			{
				++ beg; // Skip space
				++ beg; // Skip version
				++ beg; // Skip \n
			}
			else if (CT_HASH("#extension") == this_token_hash)
			{
				++ beg; // Skip space
				++ beg; // Skip extension name
				++ beg; // Skip space
				++ beg; // Skip ":"
				++ beg; // Skip "true"
				++ beg; // Skip \n
			}
			else
			{
				switch (type)
				{
				case ST_VertexShader:
				case ST_GeometryShader:
					if ((CT_HASH("gl_Color") == this_token_hash) || (CT_HASH("gl_Normal") == this_token_hash)
						|| (CT_HASH("gl_Vertex") == this_token_hash) || (CT_HASH("gl_FogCoord") == this_token_hash)
						|| (0 == this_token.find("gl_MultiTexCoord")))

					{
						ss << "a_" << this_token;
					}
					else if ((CT_HASH("gl_FogFragCoord") == this_token_hash)
						|| (CT_HASH("gl_FrontColor") == this_token_hash)
						|| (CT_HASH("gl_BackColor") == this_token_hash)
						|| (CT_HASH("gl_FrontSecondaryColor") == this_token_hash)
						|| (CT_HASH("gl_BackSecondaryColor") == this_token_hash))
					{
						ss << "v_" << this_token;
						if ((ST_VertexShader == type) && has_gs)
						{
							ss << "In";
						}
					}
					else if ((CT_HASH("gl_FogFragCoordIn") == this_token_hash)
						|| (CT_HASH("gl_FrontColorIn") == this_token_hash)
						|| (CT_HASH("gl_BackColorIn") == this_token_hash)
						|| (CT_HASH("gl_FrontSecondaryColorIn") == this_token_hash)
						|| (CT_HASH("gl_BackSecondaryColorIn") == this_token_hash))
					{
						ss << "v_" << this_token;
					}
					else if (CT_HASH("gl_TexCoord") == this_token_hash)
					{
						if ((ST_VertexShader == type) && has_gs)
						{
							ss << "v_" << this_token << "In";

							std::string tmp_token[3];
							for (int t = 0; t < 3; ++ t)
							{
								++ beg;
								tmp_token[t] = *beg;
							}

							ss << tmp_token[1];
						}
						else
						{
							ss << "v_" << this_token;
						}
					}
					else if (CT_HASH("gl_TexCoordIn") == this_token_hash)
					{
						ss << "v_" << this_token;

						std::string tmp_token[6];
						for (int t = 0; t < 6; ++ t)
						{
							++ beg;
							tmp_token[t] = *beg;
						}

						ss << tmp_token[4] << tmp_token[0] << tmp_token[1] << tmp_token[2];
					}
					else
					{
						ss << this_token;
					}
					break;

				case ST_PixelShader:
					if ((CT_HASH("gl_TexCoord") == this_token_hash) || (CT_HASH("gl_FogFragCoord") == this_token_hash))
					{
						ss << "v_" << this_token;
					}
					else if (CT_HASH("gl_Color") == this_token_hash)
					{
						ss << "v_gl_FrontColor";
					}
					else if (CT_HASH("gl_SecondaryColor") == this_token_hash)
					{
						ss << "v_gl_FrontSecondaryColor";
					}
					else if (CT_HASH("gl_FragColor") == this_token_hash)
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
					else if (CT_HASH("gl_FragData") == this_token_hash)
					{
						if (glloader_GL_VERSION_4_2())
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
						if (CT_HASH("discard") == this_token_hash)
						{
							has_discard_ = true;
						}

						ss << this_token;
					}
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}

		return ss.str();
	}
#endif

	bool OGLShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
			std::vector<uint8_t> const & native_shader_block)
	{
		bool ret = false;

		if (native_shader_block.size() >= 24)
		{
			uint8_t const * nsbp = &native_shader_block[0];
			uint32_t fourcc;
			std::memcpy(&fourcc, nsbp, sizeof(fourcc));
			nsbp += sizeof(fourcc);
			LittleEndianToNative<sizeof(fourcc)>(&fourcc);
			if (MakeFourCC<'G', 'L', 'S', 'L'>::value == fourcc)
			{
				uint32_t ver;
				std::memcpy(&ver, nsbp, sizeof(ver));
				nsbp += sizeof(ver);
				LittleEndianToNative<sizeof(ver)>(&ver);
				if (3 == ver)
				{
					uint32_t len32;
					std::memcpy(&len32, nsbp, sizeof(len32));
					nsbp += sizeof(len32);
					LittleEndianToNative<sizeof(len32)>(&len32);
					(*shader_func_names_)[type] = effect.GetShaderDesc(shader_desc_ids[type]).func_name;
					(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(len32, '\0');
					std::memcpy(&(*(*glsl_srcs_)[type])[0], nsbp, len32);
					nsbp += len32;

					uint16_t num16;
					std::memcpy(&num16, nsbp, sizeof(num16));
					nsbp += sizeof(num16);
					LittleEndianToNative<sizeof(num16)>(&num16);
					(*pnames_)[type] = MakeSharedPtr<std::vector<std::string> >(num16);
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
					LittleEndianToNative<sizeof(num16)>(&num16);
					(*glsl_res_names_)[type] = MakeSharedPtr<std::vector<std::string> >(num16);
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
					LittleEndianToNative<sizeof(num16)>(&num16);
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
							if (get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
							{
								get<3>(tex_sampler_binds_[k]) |= 1UL << type;
								found = true;
								break;
							}
						}
						if (!found)
						{
							tex_sampler_binds_.push_back(KlayGE::make_tuple(combined_sampler_name,
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
						LittleEndianToNative<sizeof(gs_input_type_)>(&gs_input_type_);

						std::memcpy(&gs_output_type_, nsbp, sizeof(gs_output_type_));
						nsbp += sizeof(gs_output_type_);
						LittleEndianToNative<sizeof(gs_output_type_)>(&gs_output_type_);

						std::memcpy(&gs_max_output_vertex_, nsbp, sizeof(gs_max_output_vertex_));
						nsbp += sizeof(gs_max_output_vertex_);
						LittleEndianToNative<sizeof(gs_max_output_vertex_)>(&gs_max_output_vertex_);
					}

					this->AttachGLSL(type);

					ret = true;
				}
			}
		}

		return ret;
	}

	bool OGLShaderObject::StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
		std::vector<uint32_t> const & shader_desc_ids, uint32_t tech_index, uint32_t pass_index)
	{
		uint32_t len;
		res->read(&len, sizeof(len));
		LittleEndianToNative<sizeof(len)>(&len);
		std::vector<uint8_t> native_shader_block(len);
		if (len > 0)
		{
			res->read(&native_shader_block[0], len * sizeof(native_shader_block[0]));
		}

		bool this_native_accepted = this->AttachNativeShader(type, effect, shader_desc_ids, native_shader_block);
		if (!this_native_accepted)
		{
			RenderTechniquePtr const & tech = effect.TechniqueByIndex(tech_index);
			RenderPassPtr const & pass = tech->Pass(pass_index);
			this->AttachShader(type, effect, *tech, *pass, shader_desc_ids);
			if ((*glsl_srcs_)[type])
			{
				this_native_accepted = true;
			}
		}

		return this_native_accepted;
	}

	void OGLShaderObject::StreamOut(std::ostream& os, ShaderType type)
	{
		std::vector<uint8_t> native_shader_block;

		if ((*glsl_srcs_)[type])
		{
			std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

			uint32_t fourcc = MakeFourCC<'G', 'L', 'S', 'L'>::value;
			NativeToLittleEndian<sizeof(fourcc)>(&fourcc);
			oss.write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));

			uint32_t ver = 3;
			NativeToLittleEndian<sizeof(ver)>(&ver);
			oss.write(reinterpret_cast<char const *>(&ver), sizeof(ver));

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
				if (get<3>(tex_sampler_binds_[i]) | (1UL << type))
				{
					tex_sampler_pairs.push_back(std::make_pair(*get<1>(tex_sampler_binds_[i])->Name(),
						*get<2>(tex_sampler_binds_[i])->Name()));
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
				uint32_t git = gs_input_type_;
				NativeToLittleEndian<sizeof(git)>(&git);
				oss.write(reinterpret_cast<char const *>(&git), sizeof(git));

				uint32_t got = gs_output_type_;
				NativeToLittleEndian<sizeof(got)>(&got);
				oss.write(reinterpret_cast<char const *>(&got), sizeof(got));

				uint32_t gmov = gs_max_output_vertex_;
				NativeToLittleEndian<sizeof(gmov)>(&gmov);
				oss.write(reinterpret_cast<char const *>(&gmov), sizeof(gmov));
			}

			std::string out_str = oss.str();
			native_shader_block.resize(out_str.size());
			std::memcpy(&native_shader_block[0], &out_str[0], out_str.size());
		}

		uint32_t len = static_cast<uint32_t>(native_shader_block.size());
		{
			uint32_t tmp = len;
			NativeToLittleEndian<sizeof(tmp)>(&tmp);
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

#if USE_DXBC2GLSL
		OGLRenderEngine const & re = *checked_cast<OGLRenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderDeviceCaps const & caps = re.DeviceCaps();

		std::string max_sm_str;
		{
			std::stringstream ss;
			ss << static_cast<int>(caps.max_shader_model);
			max_sm_str = ss.str();
		}
		std::string max_tex_array_str;
		{
			std::stringstream ss;
			ss << caps.max_texture_array_length;
			max_tex_array_str = ss.str();
		}
		std::string max_tex_depth_str;
		{
			std::stringstream ss;
			ss << caps.max_texture_depth;
			max_tex_depth_str = ss.str();
		}
		std::string max_tex_units_str;
		{
			std::stringstream ss;
			ss << static_cast<int>(caps.max_pixel_texture_units);
			max_tex_units_str = ss.str();
		}
		std::string flipping_str;
		{
			std::stringstream ss;
			ss << (re.RequiresFlipping() ? -1 : +1);
			flipping_str = ss.str();
		}
		std::string standard_derivatives_str;
		{
			std::stringstream ss;
			ss << (caps.standard_derivatives_support ? 1 : 0);
			standard_derivatives_str = ss.str();
		}

		std::string hlsl_shader_text = this->GenHLSLShaderText(type, effect, tech, pass);

		is_shader_validate_[type] = true;

		std::string shader_profile = sd.profile;
		size_t const shader_profile_hash = RT_HASH(shader_profile.c_str());
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
			if (caps.max_shader_model < 4)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = "gs_5_0";
				}
			}
			break;

		case ST_ComputeShader:
			if (caps.max_shader_model < 4)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = "cs_5_0";
				}
				if ((CT_HASH("cs_5_0") == shader_profile_hash) && (caps.max_shader_model < 5))
				{
					is_shader_validate_[type] = false;
				}
			}
			break;

		case ST_HullShader:
			if (caps.max_shader_model < 5)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = "hs_5_0";
				}
			}
			break;

		case ST_DomainShader:
			if (caps.max_shader_model < 5)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = "ds_5_0";
				}
			}
			break;

		default:
			is_shader_validate_[type] = false;
			break;
		}

		ID3DBlob* code = nullptr;
		if (is_shader_validate_[type])
		{
			ID3DBlob* err_msg;
			std::vector<D3D_SHADER_MACRO> macros;
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_DXBC2GLSL", "1" };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_OPENGL", "1" };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_SHADER_MODEL", max_sm_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_ARRAY_LEN", max_tex_array_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_DEPTH", max_tex_depth_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_UNITS", max_tex_units_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_NO_TEX_LOD", "0" };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_FLIPPING", flipping_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_DERIVATIVES", standard_derivatives_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			if (!caps.texture_format_support(EF_BC5)
				|| !caps.texture_format_support(EF_BC5_SRGB))
			{
				D3D_SHADER_MACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_AG", "1" };
				macros.push_back(macro_bc5_as_bc3);
			}
			if (!caps.texture_format_support(EF_BC4)
				|| !caps.texture_format_support(EF_BC4_SRGB))
			{
				D3D_SHADER_MACRO macro_bc4_as_bc1 = { "KLAYGE_BC4_AS_G", "1" };
				macros.push_back(macro_bc4_as_bc1);
			}
			if (!caps.fp_color_support)
			{
				D3D_SHADER_MACRO macro_no_fp_tex = { "KLAYGE_NO_FP_COLOR", "1" };
				macros.push_back(macro_no_fp_tex);
			}
			if (caps.pack_to_rgba_required)
			{
				D3D_SHADER_MACRO macro_pack_to_rgba = { "KLAYGE_PACK_TO_RGBA", "1" };
				macros.push_back(macro_pack_to_rgba);
			}
			{
				D3D_SHADER_MACRO macro_shader_type = { "", "1" };
				switch (type)
				{
				case ST_VertexShader:
					macro_shader_type.Name = "KLAYGE_VERTEX_SHADER";
					break;

				case ST_PixelShader:
					macro_shader_type.Name = "KLAYGE_PIXEL_SHADER";
					break;

				case ST_GeometryShader:
					macro_shader_type.Name = "KLAYGE_GEOMETRY_SHADER";
					break;

				case ST_ComputeShader:
					macro_shader_type.Name = "KLAYGE_COMPUTE_SHADER";
					break;

				case ST_HullShader:
					macro_shader_type.Name = "KLAYGE_HULL_SHADER";
					break;

				case ST_DomainShader:
					macro_shader_type.Name = "KLAYGE_DOMAIN_SHADER";
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
				macros.push_back(macro_shader_type);
			}
			{
				D3D_SHADER_MACRO macro_end = { nullptr, nullptr };
				macros.push_back(macro_end);
			}
			uint32_t flags = D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;

			typedef HRESULT(WINAPI *D3DCompileFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
				D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
				LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);

			HMODULE mod_d3dcompiler = ::LoadLibraryW(L"d3dcompiler_47.dll");
			D3DCompileFunc DynamicD3DCompile = reinterpret_cast<D3DCompileFunc>(::GetProcAddress(mod_d3dcompiler, "D3DCompile"));

			DynamicD3DCompile(hlsl_shader_text.c_str(), static_cast<UINT>(hlsl_shader_text.size()), nullptr, &macros[0],
				nullptr, sd.func_name.c_str(), shader_profile.c_str(),
				flags, 0, &code, &err_msg);
			if (err_msg != nullptr)
			{
				LogError("Error when compiling %s:", sd.func_name.c_str());

				std::map<int, std::vector<std::string> > err_lines;
				{
					std::istringstream err_iss(static_cast<char*>(err_msg->GetBufferPointer()));
					std::string err_str;
					while (err_iss)
					{
						std::getline(err_iss, err_str);

						int err_line = -1;
						std::string::size_type pos = err_str.find("): error X");
						if (pos == std::string::npos)
						{
							pos = err_str.find("): warning X");
						}
						if (pos != std::string::npos)
						{
							std::string part_err_str = err_str.substr(0, pos);
							pos = part_err_str.rfind("(");
							part_err_str = part_err_str.substr(pos + 1);
							std::istringstream(part_err_str) >> err_line;
						}

						std::vector<std::string>& msgs = err_lines[err_line];
						bool found = false;
						typedef KLAYGE_DECLTYPE(msgs) ErrMsgsType;
						KLAYGE_FOREACH(ErrMsgsType::const_reference msg, msgs)
						{
							if (msg == err_str)
							{
								found = true;
								break;
							}
						}

						if (!found)
						{
							msgs.push_back(err_str);
						}
					}
				}

				for (KLAYGE_AUTO(iter, err_lines.begin()); iter != err_lines.end(); ++ iter)
				{
					if (iter->first >= 0)
					{
						std::istringstream iss(hlsl_shader_text);
						std::string s;
						int line = 1;

						LogInfo("...");
						while (iss && ((iter->first - line) >= 3))
						{
							std::getline(iss, s);
							++ line;
						}
						while (iss && (abs(line - iter->first) < 3))
						{
							std::getline(iss, s);

							while (!s.empty() && (('\r' == s[s.size() - 1]) || ('\n' == s[s.size() - 1])))
							{
								s.resize(s.size() - 1);
							}

							LogInfo("%d %s", line, s.c_str());

							++ line;
						}
						LogInfo("...");
					}

					typedef KLAYGE_DECLTYPE(iter->second) ErrMsgsType;
					KLAYGE_FOREACH(ErrMsgsType::const_reference msg, iter->second)
					{
						LogError(msg.c_str());
					}
				}

				err_msg->Release();
			}

			if (code)
			{
				bool has_gs = false;
				if (!effect.GetShaderDesc(shader_desc_ids[ST_GeometryShader]).func_name.empty())
				{
					has_gs = true;
				}

				try
				{
					GLSLVersion gsv = GSV_120;
					if (glloader_GL_VERSION_4_4())
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

					DXBC2GLSL::DXBC2GLSL dxbc2glsl;
					uint32_t rules = DXBC2GLSL::DXBC2GLSL::DefaultRules(gsv);
					rules &= ~GSR_UseUBO;
					dxbc2glsl.FeedDXBC(code->GetBufferPointer(), has_gs, gsv, rules);
					(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(dxbc2glsl.GLSLString());
					(*pnames_)[type] = MakeSharedPtr<std::vector<std::string> >();
					(*glsl_res_names_)[type] = MakeSharedPtr<std::vector<std::string> >();

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
								if (get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
								{
									get<3>(tex_sampler_binds_[k]) |= 1UL << type;
									found = true;
									break;
								}
							}
							if (!found)
							{
								tex_sampler_binds_.push_back(KlayGE::make_tuple(combined_sampler_name,
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
				}
				catch (std::exception& ex)
				{
					is_shader_validate_[type] = false;

					LogError("Error(s) in conversion: %s/%s/%s", tech.Name().c_str(), pass.Name().c_str(), sd.func_name.c_str());
					LogError(ex.what());
					LogError("Please send this information and your shader to webmaster at klayge.org. We'll fix this ASAP.");
				}

				code->Release();
			}
			else
			{
				is_shader_validate_[type] = false;
			}

			::FreeLibrary(mod_d3dcompiler);
		}
#else
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

		std::string shader_text = this->GenCgShaderText(type, effect, tech, pass);

		std::vector<char const *> args;
		args.push_back("-DKLAYGE_CG=1");
		args.push_back("-DKLAYGE_OPENGL=1");
		args.push_back(max_sm_str.c_str());
		args.push_back(max_tex_array_str.c_str());
		args.push_back(max_tex_depth_str.c_str());
		args.push_back(max_tex_units_str.c_str());
		args.push_back("-DKLAYGE_NO_TEX_LOD=0");
		args.push_back(flipping_str.c_str());
		args.push_back(standard_derivatives_str.c_str());
		if (!caps.texture_format_support(EF_BC5)
			|| !caps.texture_format_support(EF_BC5_SRGB))
		{
			args.push_back("-DKLAYGE_BC5_AS_AG");
		}
		if (!caps.texture_format_support(EF_BC4)
			|| !caps.texture_format_support(EF_BC4_SRGB))
		{
			args.push_back("-DKLAYGE_BC4_AS_G");
		}
		if (!caps.fp_color_support)
		{
			args.push_back("-DKLAYGE_NO_FP_COLOR");
		}
		if (caps.pack_to_rgba_required)
		{
			args.push_back("-DKLAYGE_PACK_TO_RGBA");
		}
		switch (type)
		{
		case ST_VertexShader:
			args.push_back("-DKLAYGE_VERTEX_SHADER");
			break;

		case ST_PixelShader:
			args.push_back("-DKLAYGE_PIXEL_SHADER");
			break;

		case ST_GeometryShader:
			args.push_back("-DKLAYGE_GEOMETRY_SHADER");
			break;

		case ST_ComputeShader:
			args.push_back("-DKLAYGE_COMPUTE_SHADER");
			break;

		case ST_HullShader:
			args.push_back("-DKLAYGE_HULL_SHADER");
			break;

		case ST_DomainShader:
			args.push_back("-DKLAYGE_DOMAIN_SHADER");
			break;

		default:
			BOOST_ASSERT(false);
			break;
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

			CGprogram cg_shader = cgCreateProgram(CGContextIniter::Instance().Context(),
				CG_SOURCE, shader_text.c_str(), profile, sd.func_name.c_str(), &args[0]);

			CGerror error = cgGetError();
			if (error != CG_NO_ERROR)
			{
				if (CG_COMPILER_ERROR == error)
				{
					LogError("Error when compiling %s:", sd.func_name.c_str());
					char const* listing = cgGetLastListing(CGContextIniter::Instance().Context());
					if (listing)
					{
						std::string err_str = listing;
						std::string::size_type pos = err_str.find(") : error ");
						if (pos == std::string::npos)
						{
							pos = err_str.find(") : warning ");
						}
						if (pos != std::string::npos)
						{
							std::string part_err_str = err_str.substr(0, pos);
							pos = part_err_str.rfind("(");
							part_err_str = part_err_str.substr(pos + 1);
							int err_line = boost::lexical_cast<int>(part_err_str);

							std::istringstream iss(shader_text);
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

						LogError(cgGetErrorString(error));
						LogError(listing);
						LogError("\n");
					}
				}

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

					// TODO: read maxvertexcount from shader
					gs_max_output_vertex_ = 32;
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

				if (ST_VertexShader == type)
				{
					glsl_vs_attrib_names_->clear();

					cg_param = cgGetFirstParameter(cg_shader, CG_PROGRAM);
					while (cg_param)
					{
						if (cgIsParameterUsed(cg_param, cg_shader)
							&& (CG_PARAMETERCLASS_OBJECT != cgGetParameterClass(cg_param))
							&& ((CG_IN == cgGetParameterDirection(cg_param)) || (CG_INOUT == cgGetParameterDirection(cg_param))))
						{
							std::string semantic = cgGetParameterSemantic(cg_param);
							std::string glsl_param_name = semantic;//cgGetParameterResourceName(cg_param);
							size_t const semantic_hash = RT_HASH(semantic.c_str());

							VertexElementUsage usage = VEU_Position;
							uint8_t usage_index = 0;
							if (CT_HASH("POSITION") == semantic_hash)
							{
								usage = VEU_Position;
								glsl_param_name = "a_gl_Vertex";
							}
							else if (CT_HASH("NORMAL") == semantic_hash)
							{
								usage = VEU_Normal;
								glsl_param_name = "a_gl_Normal";
							}
							else if ((CT_HASH("COLOR0") == semantic_hash) || (CT_HASH("COLOR") == semantic_hash))
							{
								usage = VEU_Diffuse;
								glsl_param_name = "a_gl_Color";
							}
							else if (CT_HASH("COLOR1") == semantic_hash)
							{
								usage = VEU_Specular;
								glsl_param_name = "a_gl_SecondaryColor";
							}
							else if (CT_HASH("BLENDWEIGHT") == semantic_hash)
							{
								usage = VEU_BlendWeight;
								glsl_param_name = "BLENDWEIGHT";
							}
							else if (CT_HASH("BLENDINDICES") == semantic_hash)
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
							else if (CT_HASH("TANGENT") == semantic_hash)
							{
								usage = VEU_Tangent;
								glsl_param_name = "TANGENT";
							}
							else if (CT_HASH("BINORMAL") == semantic_hash)
							{
								usage = VEU_Binormal;
								glsl_param_name = "BINORMAL";
							}
							else
							{
								BOOST_ASSERT(false);
								usage = VEU_Position;
								glsl_param_name = "a_gl_Vertex";
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
#endif

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

			for (uint32_t j = 0; j < so->tex_sampler_binds_.size(); ++ j)
			{
				if (get<3>(so->tex_sampler_binds_[j]) | (1UL << type))
				{
					std::string const & combined_sampler_name = get<0>(so->tex_sampler_binds_[j]);
					bool found = false;
					for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
					{
						if (get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
						{
							get<3>(tex_sampler_binds_[k]) |= 1UL << type;
							found = true;
							break;
						}
					}
					if (!found)
					{
						tex_sampler_binds_.push_back(KlayGE::make_tuple(combined_sampler_name,
							get<1>(so->tex_sampler_binds_[j]), get<2>(so->tex_sampler_binds_[j]), 1UL << type));
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
									if (get<0>(tex_sampler_binds_[i]) == (*(*pnames_)[type])[pi])
									{
										parameter_bind_t pb;
										pb.combined_sampler_name = get<0>(tex_sampler_binds_[i]);
										pb.location = location;
										pb.shader_type = type;
										pb.tex_sampler_bind_index = static_cast<int>(i);

										uint32_t index = static_cast<uint32_t>(samplers_.size());
										samplers_.resize(index + 1);

										pb.func = SetOGLShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_, location,
											index, get<1>(tex_sampler_binds_[i]), get<2>(tex_sampler_binds_[i]));

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

		ret->tex_sampler_binds_.resize(tex_sampler_binds_.size());
		for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			get<0>(ret->tex_sampler_binds_[i]) = get<0>(tex_sampler_binds_[i]);
			get<1>(ret->tex_sampler_binds_[i]) = effect.ParameterByName(*(get<1>(tex_sampler_binds_[i])->Name()));
			get<2>(ret->tex_sampler_binds_[i]) = effect.ParameterByName(*(get<2>(tex_sampler_binds_[i])->Name()));
			get<3>(ret->tex_sampler_binds_[i]) = get<3>(tex_sampler_binds_[i]);
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
						glProgramParameteriEXT(ret->glsl_program_, GL_GEOMETRY_VERTICES_OUT_EXT, ret->gs_max_output_vertex_);
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
						if (get<0>(ret->tex_sampler_binds_[j]) == pname)
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
								get<1>(ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index]),
								get<2>(ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index]));

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

		if ((ST_GeometryShader == type) && (glloader_GL_VERSION_3_2() || glloader_GL_EXT_geometry_shader4()))
		{
			glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_INPUT_TYPE_EXT, gs_input_type_);
			glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_OUTPUT_TYPE_EXT, gs_output_type_);
			glProgramParameteriEXT(glsl_program_, GL_GEOMETRY_VERTICES_OUT_EXT, gs_max_output_vertex_);
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
					std::string type;
					err_iss >> type;
					if ("Vertex" == type)
					{
						this->PrintGLSLError(ST_VertexShader, &info[0]);
					}
					else if ("Geometry" == type)
					{
						this->PrintGLSLError(ST_VertexShader, &info[0]);
					}
					else if ("Fragment" == type)
					{
						this->PrintGLSLError(ST_VertexShader, &info[0]);
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
