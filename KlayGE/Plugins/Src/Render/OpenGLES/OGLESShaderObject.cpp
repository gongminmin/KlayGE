// OGLESShaderObject.cpp
// KlayGE OpenGL ES shader对象类 实现文件
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
#include <fstream>
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

#ifndef KLAYGE_PLATFORM_ANDROID
#if USE_DXBC2GLSL
#include <DXBC2GLSL/DXBC2GLSL.hpp>
#ifdef KLAYGE_COMPILER_GCC
	#define __in
	#define __in_ecount(size)
	#define __out
	#define __out_ecount(size)
	#define __in_bcount(size)
	#define __in_opt
	#define __in_ecount_opt(size)
	#define __out_opt
	#define __in_xcount_opt(size) 
#endif
#include <D3DCompiler.h>
#else
#include <Cg/cg.h>
#endif
#endif

#include <KlayGE/OpenGLES/OGLESRenderFactory.hpp>
#include <KlayGE/OpenGLES/OGLESRenderFactoryInternal.hpp>
#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESRenderStateObject.hpp>
#include <KlayGE/OpenGLES/OGLESShaderObject.hpp>

#if !USE_DXBC2GLSL
#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "Cg.lib")
#endif
#endif

namespace
{
	using namespace KlayGE;

#ifndef KLAYGE_PLATFORM_ANDROID
#if USE_DXBC2GLSL
	class DXBC2GLSLIniter
	{
	public:
		~DXBC2GLSLIniter()
		{
			::FreeLibrary(mod_d3dcompiler_);
		}

		static DXBC2GLSLIniter& Instance()
		{
			static DXBC2GLSLIniter initer;
			return initer;
		}

		HRESULT D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
			D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
			LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs) const
		{
			return DynamicD3DCompile_(pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, pEntrypoint,
				pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);
		}

		GLSLVersion GLSLVer() const
		{
			return gsv_;
		}

	private:
		DXBC2GLSLIniter()
		{
			mod_d3dcompiler_ = ::LoadLibraryW(L"d3dcompiler_47.dll");
			DynamicD3DCompile_ = reinterpret_cast<D3DCompileFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DCompile"));

			if (glloader_GLES_VERSION_3_1())
			{
				gsv_ = GSV_310_ES;
			}
			else if (glloader_GLES_VERSION_3_0())
			{
				gsv_ = GSV_300_ES;
			}
			else
			{
				gsv_ = GSV_100_ES;
			}
		}

	private:
		typedef HRESULT(WINAPI *D3DCompileFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
			D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
			LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);

		HMODULE mod_d3dcompiler_;
		D3DCompileFunc DynamicD3DCompile_;
		GLSLVersion gsv_;
	};
#else
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
	float4 tex2DGrad(sampler2D s, float2 location, float2 dpdx, float2 dpdy)\n		\
	{\n																	\
	#if KLAYGE_DERIVATIVES\n											\
		return tex2D(s, location, dpdx, dpdy);\n						\
	#else\n																\
		return tex2D(s, location);\n									\
	#endif\n															\
	}\n																	\
	\n																	\
	float4 tex3DGrad(sampler3D s, float3 location, float3 dpdx, float3 dpdy)\n		\
	{\n																	\
	#if KLAYGE_DERIVATIVES\n											\
		return tex3D(s, location, dpdx, dpdy);\n						\
	#else\n																\
		return tex3D(s, location);\n									\
	#endif\n															\
	}\n																	\
	\n																	\
	float4 texCUBEGrad(samplerCUBE s, float3 location, float3 dpdx, float3 dpdy)\n	\
	{\n																	\
	#if KLAYGE_DERIVATIVES\n											\
		return texCUBE(s, location, dpdx, dpdy);\n						\
	#else\n																\
		return texCUBE(s, location);\n									\
	#endif\n															\
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
#endif
#endif

	template <typename SrcType>
	class SetOGLESShaderParameter
	{
	};

	template <>
	class SetOGLESShaderParameter<bool>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			bool v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1i(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<uint32_t>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint32_t v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1ui(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<int32_t>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int32_t v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1i(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1f(location_, v);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<uint2>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint2 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform2uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<uint3>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint3 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform3uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<uint4>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			uint4 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform4uiv(location_, 1, reinterpret_cast<GLuint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<int2>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int2 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform2iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<int3>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int3 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform3iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<int4>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			int4 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform4iv(location_, 1, reinterpret_cast<GLint*>(&v.x()));
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float2>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float2 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform2fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float3>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float3 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform3fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float4>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float4 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform4fv(location_, 1, &v.x());
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float4x4>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform4fv(location_, 4, &v[0]);
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<bool*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
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
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform1iv(location_, static_cast<int>(tmp.size()), &tmp[0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<uint32_t*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint32_t> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform1uiv(location_, static_cast<int>(v.size()), reinterpret_cast<GLuint*>(&v[0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<int32_t*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int32_t> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform1iv(location_, static_cast<int>(v.size()), reinterpret_cast<GLint*>(&v[0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform1fv(location_, static_cast<int>(v.size()), &v[0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<uint2*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint2> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform2uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<uint3*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint3> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform3uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<uint4*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<uint4> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform4uiv(location_, static_cast<long>(v.size()), reinterpret_cast<GLuint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<int2*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int2> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform2iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<int3*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int3> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform3iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<int4*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int4> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform4iv(location_, static_cast<long>(v.size()), reinterpret_cast<GLint*>(&v[0][0]));
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float2*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float2> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform2fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float3*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float3> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform3fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float4*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform4fv(location_, static_cast<long>(v.size()), &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<float4x4*>
	{
	public:
		SetOGLESShaderParameter(GLint location, RenderEffectParameterPtr const & param)
			: location_(location), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4x4> v;
			param_->Value(v);

			if (!v.empty())
			{
				OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.Uniform4fv(location_, static_cast<GLsizei>(v.size()) * 4, &v[0][0]);
			}
		}

	private:
		GLint location_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetOGLESShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >
	{
	public:
		SetOGLESShaderParameter(std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> >& samplers,
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
				checked_pointer_cast<OGLESSamplerStateObject>((*samplers_)[stage_].second)->Active((*samplers_)[stage_].first);
				(*gl_bind_targets_)[stage_] = checked_pointer_cast<OGLESTexture>((*samplers_)[stage_].first)->GLType();
				(*gl_bind_textures_)[stage_] = checked_pointer_cast<OGLESTexture>((*samplers_)[stage_].first)->GLTexture();
			}
			else
			{
				(*gl_bind_targets_)[stage_] = GL_TEXTURE_2D;
				(*gl_bind_textures_)[stage_] = 0;
			}

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1i(location_, stage_);
		}

	private:
		std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> >* samplers_;
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
	OGLESShaderObject::OGLESShaderObject()
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

	OGLESShaderObject::~OGLESShaderObject()
	{
		glDeleteProgram(glsl_program_);
	}

#ifndef KLAYGE_PLATFORM_ANDROID
#if !USE_DXBC2GLSL
	std::string OGLESShaderObject::GenCgShaderText(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass)
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
				for (KLAYGE_AUTO(beg, tok.begin()); beg != tok.end();)
				{
					this_token = *beg;
					size_t const this_token_hash = RT_HASH(this_token.c_str());

					RenderEffectParameterPtr const & param = effect.ParameterByName(this_token);
					if (param &&
						((REDT_texture1D == param->Type()) || (REDT_texture2D == param->Type()) || (REDT_texture3D == param->Type())
							|| (REDT_textureCUBE == param->Type())))
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

							if (!sample_helper && (("SampleLevel" == sample_tokens[2]) || ("SampleBias" == sample_tokens[2]) || ("SampleGrad" == sample_tokens[2])))
							{
								sample_helper = true;
							}

							switch (param->Type())
							{
							case REDT_texture1D:
							case REDT_texture2D:
							case REDT_texture3D:
							case REDT_textureCUBE:
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
								else if ("SampleBias" == sample_tokens[2])
								{
									shader_ss << "Bias";
								}
								else if ("SampleGrad" == sample_tokens[2])
								{
									shader_ss << "Grad";
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

		for (uint32_t i = 0; i < effect.NumCBuffers(); ++ i)
		{
			RenderEffectConstantBufferPtr const & cbuff = effect.CBufferByIndex(i);
			for (uint32_t j = 0; j < cbuff->NumParameters(); ++ j)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(cbuff->ParameterIndex(j));

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
			ss << " " << get<0>(tex_sampler_binds_[i]) << ";" << std::endl;
		}

		ss << shader_ss.str();

		return ss.str();
	}
#endif

#if USE_DXBC2GLSL
	std::string OGLESShaderObject::GenHLSLShaderText(ShaderType type, RenderEffect const & effect,
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

		for (uint32_t i = 0; i < effect.NumCBuffers(); ++ i)
		{
			RenderEffectConstantBufferPtr const & cbuff = effect.CBufferByIndex(i);
			ss << "cbuffer " << *cbuff->Name() << std::endl;
			ss << "{" << std::endl;

			for (uint32_t j = 0; j < cbuff->NumParameters(); ++ j)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(cbuff->ParameterIndex(j));
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
					ss << "Texture";
					if (caps.max_texture_depth <= 1)
					{
						ss << "2D";
					}
					else
					{
						ss << "3D";
					}
					ss << "<" << elem_type << "> " << *param.Name() << ";" << std::endl;
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
	std::string OGLESShaderObject::ConvertToESSL(std::string const & glsl, ShaderType type)
	{
		std::stringstream ss;

		{
			bool has_directive = false;
			std::stringstream iss(glsl);
			std::string s;
			while (iss)
			{
				std::getline(iss, s);

				if ((0 == s.find("#version")) || (0 == s.find("#extension")))
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
					if ((CT_HASH("gl_Color") == this_token_hash) || (CT_HASH("gl_Normal") == this_token_hash)
						|| (CT_HASH("gl_Vertex") == this_token_hash) || (CT_HASH("gl_FogCoord") == this_token_hash)
						|| (0 == this_token.find("gl_MultiTexCoord")))

					{
						ss << "a_" << this_token;
					}
					else if ((CT_HASH("gl_TexCoord") == this_token_hash) || (CT_HASH("gl_FogFragCoord") == this_token_hash))
					{
						ss << "v_" << this_token;
					}
					else if ((CT_HASH("gl_FrontColor") == this_token_hash) || (CT_HASH("gl_BackColor") == this_token_hash))
					{
						ss << "v_gl_FrontColor";
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
#endif

	bool OGLESShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
			std::vector<uint8_t> const & native_shader_block)
	{
		bool ret = false;

		is_shader_validate_[type] = false;
		if (native_shader_block.size() >= 24)
		{
			uint8_t const * nsbp = &native_shader_block[0];
			uint32_t fourcc;
			std::memcpy(&fourcc, nsbp, sizeof(fourcc));
			nsbp += sizeof(fourcc);
			fourcc = LE2Native(fourcc);
			if (MakeFourCC<'E', 'S', 'S', 'L'>::value == fourcc)
			{
				uint32_t ver;
				std::memcpy(&ver, nsbp, sizeof(ver));
				nsbp += sizeof(ver);
				ver = LE2Native(ver);
				if (3 == ver)
				{
					uint32_t len32;
					std::memcpy(&len32, nsbp, sizeof(len32));
					nsbp += sizeof(len32);
					len32 = LE2Native(len32);
					(*shader_func_names_)[type] = effect.GetShaderDesc(shader_desc_ids[type]).func_name;
					(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(len32, '\0');
					std::memcpy(&(*(*glsl_srcs_)[type])[0], nsbp, len32);
					nsbp += len32;

					uint16_t num16;
					std::memcpy(&num16, nsbp, sizeof(num16));
					nsbp += sizeof(num16);
					num16 = LE2Native(num16);
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
					num16 = LE2Native(num16);
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

					this->AttachGLSL(type);

					ret = is_shader_validate_[type];
				}
			}
		}

		return ret;
	}

	bool OGLESShaderObject::StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
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

	void OGLESShaderObject::StreamOut(std::ostream& os, ShaderType type)
	{
		std::vector<uint8_t> native_shader_block;

		if ((*glsl_srcs_)[type])
		{
			std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

			uint32_t fourcc = Native2LE(MakeFourCC<'E', 'S', 'S', 'L'>::value);
			oss.write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));

			uint32_t ver = Native2LE(3);
			oss.write(reinterpret_cast<char const *>(&ver), sizeof(ver));

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

			std::vector<std::pair<std::string, std::string> > tex_sampler_pairs;
			for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
			{
				if (get<3>(tex_sampler_binds_[i]) | (1UL << type))
				{
					tex_sampler_pairs.push_back(std::make_pair(*get<1>(tex_sampler_binds_[i])->Name(),
						*get<2>(tex_sampler_binds_[i])->Name()));
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

	void OGLESShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids)
	{
		ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		(*shader_func_names_)[type] = sd.func_name;

		is_shader_validate_[type] = true;
		switch (type)
		{
		case ST_VertexShader:
		case ST_PixelShader:
			break;

		default:
			is_shader_validate_[type] = false;
			break;
		}

		if (is_shader_validate_[type])
		{
#ifndef KLAYGE_PLATFORM_ANDROID
#if USE_DXBC2GLSL
			OGLESRenderEngine const & re = *checked_cast<OGLESRenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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
			std::string no_tex_lod_str;
			{
				std::stringstream ss;
				ss << static_cast<int>(caps.max_pixel_texture_units);
				ss << (ST_VertexShader == type) ? 0 : 1;//(glloader_GLES_EXT_shader_texture_lod() ? 0 : 1);
				no_tex_lod_str = ss.str();
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
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_OPENGLES", "1" };
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
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_NO_TEX_LOD", no_tex_lod_str.c_str() };
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
				else
				{
					D3D_SHADER_MACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_GA", "1" };
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

				DXBC2GLSLIniter::Instance().D3DCompile(hlsl_shader_text.c_str(),
					static_cast<UINT>(hlsl_shader_text.size()), nullptr, &macros[0],
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
					try
					{
						GLSLVersion gsv = DXBC2GLSLIniter::Instance().GLSLVer();
						DXBC2GLSL::DXBC2GLSL dxbc2glsl;
						uint32_t rules = DXBC2GLSL::DXBC2GLSL::DefaultRules(gsv);
						rules &= ~GSR_UseUBO;
						rules &= ~GSR_MatrixType;
						if (!glloader_GLES_VERSION_3_0())
						{
							rules &= ~GSR_VersionDecl;
						}
						dxbc2glsl.FeedDXBC(code->GetBufferPointer(), false, gsv, rules);
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
			}
#else
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
				ss << "-DKLAYGE_NO_TEX_LOD=" << ((ST_VertexShader == type) ? 0 : 1);//(glloader_GLES_EXT_shader_texture_lod() ? 0 : 1));
				no_tex_lod_str = ss.str();
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
			args.push_back("-DKLAYGE_OPENGLES=1");
			args.push_back(max_sm_str.c_str());
			args.push_back(max_tex_array_str.c_str());
			args.push_back(max_tex_depth_str.c_str());
			args.push_back(max_tex_units_str.c_str());
			args.push_back(no_tex_lod_str.c_str());
			args.push_back(flipping_str.c_str());
			args.push_back(standard_derivatives_str.c_str());
			if (!caps.texture_format_support(EF_BC5)
				|| !caps.texture_format_support(EF_BC5_SRGB))
			{
				args.push_back("-DKLAYGE_BC5_AS_AG");
			}
			else
			{
				args.push_back("-DKLAYGE_BC5_AS_GA");
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
				(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(this->ConvertToESSL(cgGetProgramString(cg_shader, CG_COMPILED_PROGRAM),
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
#endif
#endif
		}

		if (is_shader_validate_[type])
		{
			this->AttachGLSL(type);
		}
	}

	void OGLESShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/,
			RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & shared_so)
	{
		OGLESShaderObjectPtr so = checked_pointer_cast<OGLESShaderObject>(shared_so);

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

	void OGLESShaderObject::LinkShaders(RenderEffect const & effect)
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
									if (get<0>(tex_sampler_binds_[i]) == (*(*pnames_)[type])[pi])
									{
										parameter_bind_t pb;
										pb.combined_sampler_name = get<0>(tex_sampler_binds_[i]);
										pb.location = location;
										pb.shader_type = type;
										pb.tex_sampler_bind_index = static_cast<int>(i);

										uint32_t index = static_cast<uint32_t>(samplers_.size());
										samplers_.resize(index + 1);
										gl_bind_targets_.resize(index + 1);
										gl_bind_textures_.resize(index + 1);

										pb.func = SetOGLESShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_,
											gl_bind_targets_, gl_bind_textures_,
											location, index, get<1>(tex_sampler_binds_[i]), get<2>(tex_sampler_binds_[i]));

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

	ShaderObjectPtr OGLESShaderObject::Clone(RenderEffect const & effect)
	{
		OGLESShaderObjectPtr ret = MakeSharedPtr<OGLESShaderObject>();

		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;
		ret->shader_func_names_ = shader_func_names_;
		ret->glsl_srcs_ = glsl_srcs_;
		ret->pnames_ = pnames_;
		ret->glsl_res_names_ = glsl_res_names_;
		ret->vs_usages_ = vs_usages_;
		ret->vs_usage_indices_ = vs_usage_indices_;
		ret->glsl_vs_attrib_names_ = glsl_vs_attrib_names_;

		ret->tex_sampler_binds_.resize(tex_sampler_binds_.size());
		for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			get<0>(ret->tex_sampler_binds_[i]) = get<0>(tex_sampler_binds_[i]);
			get<1>(ret->tex_sampler_binds_[i]) = effect.ParameterByName(*(get<1>(tex_sampler_binds_[i])->Name()));
			get<2>(ret->tex_sampler_binds_[i]) = effect.ParameterByName(*(get<2>(tex_sampler_binds_[i])->Name()));
			get<3>(ret->tex_sampler_binds_[i]) = get<3>(tex_sampler_binds_[i]);
		}

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
							ret->gl_bind_targets_.resize(index + 1);
							ret->gl_bind_textures_.resize(index + 1);

							new_pb.func = SetOGLESShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr> >(ret->samplers_,
								ret->gl_bind_targets_, ret->gl_bind_textures_,
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

	GLint OGLESShaderObject::GetAttribLocation(VertexElementUsage usage, uint8_t usage_index)
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

	OGLESShaderObject::parameter_bind_t OGLESShaderObject::GetBindFunc(GLint location, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.location = location;

		switch (param->Type())
		{
		case REDT_bool:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<bool*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<bool>(location, param);
			}
			break;

		case REDT_uint:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<uint32_t*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<uint32_t>(location, param);
			}
			break;

		case REDT_int:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<int32_t*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<int32_t>(location, param);
			}
			break;

		case REDT_float:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<float*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<float>(location, param);
			}
			break;

		case REDT_uint2:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<uint2*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<uint2>(location, param);
			}
			break;

		case REDT_uint3:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<uint3*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<uint3>(location, param);
			}
			break;

		case REDT_uint4:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<uint4*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<uint4>(location, param);
			}
			break;

		case REDT_int2:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<int2*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<int2>(location, param);
			}
			break;

		case REDT_int3:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<int3*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<int3>(location, param);
			}
			break;

		case REDT_int4:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<int4*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<int4>(location, param);
			}
			break;

		case REDT_float2:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<float2*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<float2>(location, param);
			}
			break;

		case REDT_float3:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<float3*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<float3>(location, param);
			}
			break;

		case REDT_float4:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<float4*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<float4>(location, param);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize())
			{
				ret.func = SetOGLESShaderParameter<float4x4*>(location, param);
			}
			else
			{
				ret.func = SetOGLESShaderParameter<float4x4>(location, param);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
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

		glShaderSource(object, 1, &glsl, nullptr);

		glCompileShader(object);

		GLint compiled = false;
		glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			LogError("Error when compiling ESSL %s:", (*shader_func_names_)[type].c_str());

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
#ifndef KLAYGE_PLATFORM_ANDROID
		glDeleteShader(object);
#endif
	}

	void OGLESShaderObject::LinkGLSL()
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
				LogError(&info[0]);
			}
		}
#endif
		is_validate_ &= linked ? true : false;
	}

	void OGLESShaderObject::Bind()
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.UseProgram(glsl_program_);

		typedef KLAYGE_DECLTYPE(param_binds_) ParamBindsType;
		KLAYGE_FOREACH(ParamBindsType::reference pb, param_binds_)
		{
			pb.func();
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

	void OGLESShaderObject::Unbind()
	{
		//glUseProgram(0);
	}

	void OGLESShaderObject::PrintGLSLError(ShaderType type, char const * info)
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		std::string const & glsl = *(*glsl_srcs_)[type];

		if (re.HackForMali())
		{
			std::istringstream err_iss(&info[0]);
			std::string err_str;
			while (err_iss)
			{
				std::getline(err_iss, err_str);
				if (!err_str.empty())
				{
					std::string::size_type pos = err_str.find("1:");
					if (pos != std::string::npos)
					{
						pos += 2;
						std::string::size_type pos2 = err_str.find(':', pos);
						std::string part_err_str = err_str.substr(pos, pos2 - pos);
						int err_line = boost::lexical_cast<int>(part_err_str);

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

	void OGLESShaderObject::PrintGLSLErrorAtLine(std::string const & glsl, int err_line)
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
