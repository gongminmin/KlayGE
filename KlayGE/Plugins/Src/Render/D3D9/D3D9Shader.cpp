// D3D9Shader.cpp
// KlayGE D3D9渲染器类 实现文件
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
#include <KlayGE/Engine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/alloc.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>

#include <vector>

#include <KlayGE/D3D9/D3D9Shader.hpp>

namespace KlayGE
{
	D3D9ShaderParameter::D3D9ShaderParameter(const COMPtr<ID3DXConstantTable>& constantTable, D3DXHANDLE param)
		: constantTable_(constantTable),
			param_(param),
			d3dDevice_(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice())
	{
	}

	void D3D9ShaderParameter::SetFloat(float f)
	{
		constantTable_->SetFloat(d3dDevice_.Get(), param_, f);
	}

	void D3D9ShaderParameter::SetFloatArray(const float* f, size_t count)
	{
		constantTable_->SetFloatArray(d3dDevice_.Get(), param_, f, static_cast<UINT>(count));
	}

	void D3D9ShaderParameter::SetMatrix(const Matrix4& mat)
	{
		constantTable_->SetMatrix(d3dDevice_.Get(), param_,
			reinterpret_cast<const D3DXMATRIX*>(&mat));
	}

	void D3D9ShaderParameter::SetMatrixArray(const Matrix4* mat, size_t count)
	{
		constantTable_->SetMatrixArray(d3dDevice_.Get(), param_,
			reinterpret_cast<const D3DXMATRIX*>(mat), static_cast<UINT>(count));
	}

	void D3D9ShaderParameter::SetVector(const Vector4& vec)
	{
		constantTable_->SetVector(d3dDevice_.Get(), param_,
			reinterpret_cast<const D3DXVECTOR4*>(&vec));
	}

	void D3D9ShaderParameter::SetVectorArray(const Vector4* vec, size_t count)
	{
		constantTable_->SetVectorArray(d3dDevice_.Get(), param_,
			reinterpret_cast<const D3DXVECTOR4*>(vec), static_cast<UINT>(count));
	}


	D3D9VertexShader::D3D9VertexShader(const String& src, const String& functionName, const String& profile)
		: d3dDevice_(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice())
	{
		ID3DXBuffer* shaderBuffer;
		ID3DXConstantTable* constantTable;
		::D3DXCompileShader(src.c_str(), static_cast<UINT>(src.size()), NULL, NULL, functionName.c_str(),
			profile.c_str(), 0, &shaderBuffer, NULL, &constantTable);
		COMPtr<ID3DXBuffer> code(shaderBuffer);
		constantTable_ = COMPtr<ID3DXConstantTable>(constantTable);

		IDirect3DVertexShader9* vertexShader;
		TIF(d3dDevice_->CreateVertexShader(reinterpret_cast<DWORD*>(code->GetBufferPointer()),
										&vertexShader));
		vertexShader_ = COMPtr<IDirect3DVertexShader9>(vertexShader);
	}

	void D3D9VertexShader::Active()
	{
		d3dDevice_->SetVertexShader(vertexShader_.Get());
	}

	ShaderParameterPtr D3D9VertexShader::GetNamedParameter(const String& name)
	{
		D3DXHANDLE param(constantTable_->GetConstantByName(NULL, name.c_str()));
		return ShaderParameterPtr(new D3D9ShaderParameter(constantTable_, param));
	}

	COMPtr<IDirect3DVertexShader9> D3D9VertexShader::D3DVertexShader() const
	{
		return vertexShader_;
	}

	
	D3D9PixelShader::D3D9PixelShader(const String& src, const String& functionName, const String& profile)
		: d3dDevice_(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice())
	{
		ID3DXBuffer* shaderBuffer;
		ID3DXConstantTable* constantTable;
		::D3DXCompileShader(src.c_str(), static_cast<UINT>(src.size()), NULL, NULL, functionName.c_str(),
			profile.c_str(), 0, &shaderBuffer, NULL, &constantTable);
		COMPtr<ID3DXBuffer> code(shaderBuffer);
		constantTable_ = COMPtr<ID3DXConstantTable>(constantTable);

		IDirect3DPixelShader9* pixelShader;
		TIF(d3dDevice_->CreatePixelShader(reinterpret_cast<DWORD*>(code->GetBufferPointer()),
										&pixelShader));
		pixelShader_ = COMPtr<IDirect3DPixelShader9>(pixelShader);
	}

	void D3D9PixelShader::Active()
	{
		d3dDevice_->SetPixelShader(pixelShader_.Get());
	}

	ShaderParameterPtr D3D9PixelShader::GetNamedParameter(const String& name)
	{
		return ShaderParameterPtr(new D3D9ShaderParameter(constantTable_,
			constantTable_->GetConstantByName(NULL, name.c_str())));
	}

	COMPtr<IDirect3DPixelShader9> D3D9PixelShader::D3DPixelShader() const
	{
		return pixelShader_;
	}
}
