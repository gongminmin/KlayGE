// OGLShader.hpp
// KlayGE OpenGL渲染器类 使用Cg 头文件
// Ver 2.1.0
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.0
// 初次建立 (2004.4.18)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9SHADER_HPP
#define _D3D9SHADER_HPP

#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Shader.hpp>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

namespace KlayGE
{
	class OGLShaderParameter : public ShaderParameter
	{
	public:
		OGLShaderParameter(CGparameter param);

		void SetFloat(float f);
		void SetFloatArray(const float* f, size_t count);
		void SetMatrix(const Matrix4& mat);
		void SetMatrixArray(const Matrix4* mat, size_t count);
		void SetVector(const Vector4& vec);
		void SetVectorArray(const Vector4* vec, size_t count);

	private:
		CGparameter param_;
	};

	class OGLVertexShader : public VertexShader
	{
	public:
		OGLVertexShader(const String& src, const String& functionName, const String& profile);
		~OGLVertexShader();

		void Active();
		ShaderParameterPtr GetNamedParameter(const String& name);

		CGprogram OGLVertexProgram() const;

	private:
		CGprogram vertexProgram_;
	};

	class OGLPixelShader : public PixelShader
	{
	public:
		OGLPixelShader(const String& src, const String& functionName, const String& profile);
		~OGLPixelShader();

		void Active();
		ShaderParameterPtr GetNamedParameter(const String& name);

		CGprogram OGLPixelProgram() const;

	private:
		CGprogram pixelProgram_;
	};
}

#endif		// _D3D9SHADER_HPP
