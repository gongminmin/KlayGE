// D3D9RenderEffect.cpp
// KlayGE D3D9渲染效果类 实现文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 修正了没有使用GetParameterByName的Bug (2004.3.1)
// 修改了SetTexture的参数 (2004.3.6)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#include <KlayGE/D3D9/D3D9RenderEffect.hpp>

namespace KlayGE
{
	D3D9RenderEffect::D3D9RenderEffect(const String& srcData, UINT flags)
	{
		D3D9RenderEngine& renderEngine(reinterpret_cast<D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()));

		ID3DXEffect* effect;
		D3DXCreateEffect(renderEngine.D3DDevice().Get(), srcData.c_str(), srcData.size(),
			NULL, NULL, flags, NULL, &effect, NULL);
		effect_ = COMPtr<ID3DXEffect>(effect);
	}

	void D3D9RenderEffect::Desc(UINT& parameters, UINT& techniques, UINT& functions)
	{
		D3DXEFFECT_DESC desc;
		TIF(effect_->GetDesc(&desc));

		parameters = desc.Parameters;
		techniques = desc.Techniques;
		functions = desc.Functions;
	}

	void D3D9RenderEffect::SetValue(const String& name, const void* data, UINT bytes)
	{
		TIF(effect_->SetValue(effect_->GetParameterByName(NULL, name.c_str()), data, bytes));
	}

	void* D3D9RenderEffect::GetValue(const String& name, UINT bytes) const
	{
		void* data(NULL);
		TIF(effect_->GetValue(effect_->GetParameterByName(NULL, name.c_str()), data, bytes));
		return data;
	}

	void D3D9RenderEffect::SetFloat(const String& name, float value)
	{
		TIF(effect_->SetFloat(effect_->GetParameterByName(NULL, name.c_str()), value));
	}

	float D3D9RenderEffect::GetFloat(const String& name) const
	{
		float value;
		TIF(effect_->GetFloat(effect_->GetParameterByName(NULL, name.c_str()), &value));
		return value;
	}

	void D3D9RenderEffect::SetVector(const String& name, const Vector4& value)
	{
		D3DXVECTOR4 vec(value.x(), value.y(), value.z(), value.w());
		TIF(effect_->SetVector(effect_->GetParameterByName(NULL, name.c_str()), &vec));
	}

	Vector4 D3D9RenderEffect::GetVector(const String& name) const
	{
		D3DXVECTOR4 vec;
		TIF(effect_->GetVector(effect_->GetParameterByName(NULL, name.c_str()), &vec));

		return MakeVector(vec.x, vec.y, vec.z, vec.w);
	}

	void D3D9RenderEffect::SetMatrix(const String& name, const Matrix4& value)
	{
		D3DXMATRIX mat(&(*value.Begin()));
		TIF(effect_->SetMatrix(effect_->GetParameterByName(NULL, name.c_str()), &mat));
	}

	Matrix4 D3D9RenderEffect::GetMatrix(const String& name) const
	{
		D3DXMATRIX mat;
		TIF(effect_->GetMatrix(effect_->GetParameterByName(NULL, name.c_str()), &mat));
		return Matrix4(mat);
	}

	void D3D9RenderEffect::SetInt(const String& name, int value)
	{
		TIF(effect_->SetInt(effect_->GetParameterByName(NULL, name.c_str()), value));
	}

	int D3D9RenderEffect::GetInt(const String& name) const
	{
		int value;
		TIF(effect_->GetInt(effect_->GetParameterByName(NULL, name.c_str()), &value));
		return value;
	}

	void D3D9RenderEffect::SetBool(const String& name, bool value)
	{
		TIF(effect_->SetBool(effect_->GetParameterByName(NULL, name.c_str()), value));
	}

	bool D3D9RenderEffect::GetBool(const String& name) const
	{
		BOOL value;
		TIF(effect_->GetBool(effect_->GetParameterByName(NULL, name.c_str()), &value));
		return value == TRUE;
	}

	void D3D9RenderEffect::SetString(const String& name, const String& value)
	{
		TIF(effect_->SetString(effect_->GetParameterByName(NULL, name.c_str()), value.c_str()));
	}

	String D3D9RenderEffect::GetString(const String& name) const
	{
		const char* value(NULL);
		TIF(effect_->GetString(effect_->GetParameterByName(NULL, name.c_str()), &value));
		return String(value);
	}

	void D3D9RenderEffect::SetTexture(const String& name, const TexturePtr& tex)
	{
		IDirect3DTexture9* texture(NULL);
		if (tex.Get() != NULL)
		{
			texture = static_cast<D3D9Texture*>(tex.Get())->D3DTexture().Get();
		}
		TIF(effect_->SetTexture(effect_->GetParameterByName(NULL, name.c_str()), texture));
	}

	void D3D9RenderEffect::SetVertexShader(const String& name, U32 vsHandle)
	{
		TIF(effect_->SetVertexShader(effect_->GetParameterByName(NULL, name.c_str()),
			reinterpret_cast<IDirect3DVertexShader9*>(vsHandle)));
	}

	void D3D9RenderEffect::SetPixelShader(const String& name, U32 psHandle)
	{
		TIF(effect_->SetPixelShader(effect_->GetParameterByName(NULL, name.c_str()),
			reinterpret_cast<IDirect3DPixelShader9*>(psHandle)));
	}

	void D3D9RenderEffect::Technique(const String& technique)
	{
		TIF(effect_->SetTechnique(effect_->GetTechniqueByName(technique.c_str())));
	}

	void D3D9RenderEffect::Technique(UINT technique)
	{
		TIF(effect_->SetTechnique(effect_->GetTechnique(technique)));
	}

	void D3D9RenderEffect::Validate()
	{
		TIF(effect_->ValidateTechnique(effect_->GetCurrentTechnique()));
	}

	UINT D3D9RenderEffect::Begin(UINT flags)
	{
		UINT passes;
		TIF(effect_->Begin(&passes, flags));
		return passes;
	}

	void D3D9RenderEffect::Pass(UINT passNum)
	{
		TIF(effect_->Pass(passNum));
	}

	void D3D9RenderEffect::End()
	{
		TIF(effect_->End());
	}
}
