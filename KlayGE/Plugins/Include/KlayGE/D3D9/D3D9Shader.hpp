// D3D9Shader.hpp
// KlayGE D3D9渲染器类 头文件
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

#define NOMINMAX

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

namespace KlayGE
{
	class D3D9ShaderParameter : public ShaderParameter
	{
	public:
		D3D9ShaderParameter(const COMPtr<ID3DXConstantTable>& constantTable, D3DXHANDLE param);

		void SetFloat(float f);
		void SetFloatArray(const float* f, size_t count);
		void SetMatrix(const Matrix4& mat);
		void SetMatrixArray(const Matrix4* mat, size_t count);
		void SetVector(const Vector4& vec);
		void SetVectorArray(const Vector4* vec, size_t count);

	private:
		COMPtr<ID3DXConstantTable> constantTable_;
		D3DXHANDLE param_;

		COMPtr<IDirect3DDevice9> d3dDevice_;
	};

	class D3D9VertexShader : public VertexShader
	{
	public:
		D3D9VertexShader(const std::string& src, const std::string& functionName, const std::string& profile);

		void Active();
		ShaderParameterPtr GetNamedParameter(const std::string& name);

		COMPtr<IDirect3DVertexShader9> D3DVertexShader() const;

	private:
		COMPtr<IDirect3DVertexShader9> vertexShader_;
		COMPtr<ID3DXConstantTable> constantTable_;

		COMPtr<IDirect3DDevice9> d3dDevice_;
	};

	class D3D9PixelShader : public PixelShader
	{
	public:
		D3D9PixelShader(const std::string& src, const std::string& functionName, const std::string& profile);

		void Active();
		ShaderParameterPtr GetNamedParameter(const std::string& name);

		COMPtr<IDirect3DPixelShader9> D3DPixelShader() const;

	private:
		COMPtr<IDirect3DPixelShader9> pixelShader_;
		COMPtr<ID3DXConstantTable> constantTable_;

		COMPtr<IDirect3DDevice9> d3dDevice_;
	};
}

#endif		// _D3D9SHADER_HPP
