// OGLShader.cpp
// KlayGE OGL渲染器类 使用Cg 实现文件
// Ver 2.1.0
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.0
// 初次建立 (2004.4.18)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>

#include <vector>

#include <KlayGE/OpenGL/OGLShader.hpp>

namespace
{
	class CgIniter
	{
	public:
		CgIniter()
			{ context_ = cgCreateContext(); }

		~CgIniter()
			{ cgDestroyContext(context_); }

		CGcontext GetContext()
			{ return context_; }

	private:
		CGcontext context_;
	};

	CGcontext GetCGcontext()
	{
		static CgIniter cgIniter;
		return cgIniter.GetContext();
	}
}

namespace KlayGE
{
	OGLShaderParameter::OGLShaderParameter(CGparameter param)
		: param_(param)
	{
	}

	void OGLShaderParameter::SetFloat(float f)
	{
		cgGLSetParameter1f(param_, f);
	}

	void OGLShaderParameter::SetFloatArray(const float* f, size_t count)
	{
		cgGLSetParameterArray1f(param_, 0, count, f);
	}

	void OGLShaderParameter::SetMatrix(const Matrix4& mat)
	{
		cgGLSetMatrixParameterfr(param_, &mat(0, 0));
	}

	void OGLShaderParameter::SetMatrixArray(const Matrix4* mat, size_t count)
	{
		cgGLSetMatrixParameterArrayfr(param_, 0, count, &((*mat)(0, 0)));
	}

	void OGLShaderParameter::SetVector(const Vector4& vec)
	{
		cgGLSetParameter4f(param_, vec.x(), vec.y(), vec.z(), vec.w());
	}

	void OGLShaderParameter::SetVectorArray(const Vector4* vec, size_t count)
	{
		cgGLSetParameterArray4f(param_, 0, count, &((*vec)[0]));
	}


	OGLVertexShader::OGLVertexShader(const std::string& src, const std::string& functionName, const std::string& profile)
	{
		vertexProgram_ = cgCreateProgram(GetCGcontext(),
							CG_SOURCE, src.c_str(), CG_PROFILE_ARBVP1,
							functionName.c_str(), NULL);
		cgGLLoadProgram(vertexProgram_);
	}

	OGLVertexShader::~OGLVertexShader()
	{
		cgDestroyProgram(vertexProgram_);
	}

	void OGLVertexShader::Active()
	{
		cgGLEnableProfile(CG_PROFILE_ARBVP1);
		cgGLBindProgram(vertexProgram_);
	}

	ShaderParameterPtr OGLVertexShader::GetNamedParameter(const std::string& name)
	{
		return ShaderParameterPtr(new OGLShaderParameter(cgGetNamedParameter(vertexProgram_, name.c_str())));
	}

	CGprogram OGLVertexShader::OGLVertexProgram() const
	{
		return vertexProgram_;
	}

	
	OGLPixelShader::OGLPixelShader(const std::string& src, const std::string& functionName, const std::string& profile)
	{
		pixelProgram_ = cgCreateProgram(GetCGcontext(),
							CG_SOURCE, src.c_str(), CG_PROFILE_ARBFP1,
							functionName.c_str(), NULL);
		cgGLLoadProgram(pixelProgram_);
	}

	OGLPixelShader::~OGLPixelShader()
	{
		cgDestroyProgram(pixelProgram_);
	}

	void OGLPixelShader::Active()
	{
		cgGLEnableProfile(CG_PROFILE_ARBFP1);
		cgGLBindProgram(pixelProgram_);
	}

	ShaderParameterPtr OGLPixelShader::GetNamedParameter(const std::string& name)
	{
		return ShaderParameterPtr(new OGLShaderParameter(cgGetNamedParameter(pixelProgram_, name.c_str())));
	}

	CGprogram OGLPixelShader::OGLPixelProgram() const
	{
		return pixelProgram_;
	}
}
