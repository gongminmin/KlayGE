// Shader.hpp
// KlayGE 渲染器类 实现文件
// Ver 2.1.0
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.0
// 初次建立 (2004.4.18)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SHADER_HPP
#define _SHADER_HPP

namespace KlayGE
{
	class ShaderParameter
	{
	public:
		virtual ~ShaderParameter()
			{ }

		static ShaderParameterPtr NullObject();

		virtual void SetFloat(float f) = 0;
		virtual void SetFloatArray(const float* f, size_t count) = 0;
		virtual void SetMatrix(const Matrix4& mat) = 0;
		virtual void SetMatrixArray(const Matrix4* mat, size_t count) = 0;
		virtual void SetVector(const Vector4& vec) = 0;
		virtual void SetVectorArray(const Vector4* vec, size_t count) = 0;
	};

	class VertexShader
	{
	public:
		virtual ~VertexShader()
			{ }

		static VertexShaderPtr NullObject();

		virtual void Active() = 0;
		virtual ShaderParameterPtr GetNamedParameter(const std::string& name) = 0;
	};

	class PixelShader
	{
	public:
		virtual ~PixelShader()
			{ }

		static PixelShaderPtr NullObject();

		virtual void Active() = 0;
		virtual ShaderParameterPtr GetNamedParameter(const std::string& name) = 0;
	};

	VertexShaderPtr LoadVertexShader(const std::string& shaderFileName,
		const std::string& functionName, const std::string& profile,
		bool fromPack = false);
	PixelShaderPtr LoadPixelShader(const std::string& shaderFileName,
		const std::string& functionName, const std::string& profile,
		bool fromPack = false);
}

#endif		// _SHADER_HPP
