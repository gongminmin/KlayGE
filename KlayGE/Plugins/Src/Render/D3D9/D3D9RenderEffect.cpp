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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#include <cassert>

#include <KlayGE/D3D9/D3D9RenderEffect.hpp>

namespace KlayGE
{
	D3D9RenderEffect::D3D9RenderEffect(const std::string& srcData, UINT flags)
	{
		D3D9RenderEngine& renderEngine(static_cast<D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		ID3DXEffect* effect;
		D3DXCreateEffect(renderEngine.D3DDevice().Get(), srcData.c_str(),
			static_cast<::UINT>(srcData.size()), NULL, NULL,
			flags, NULL, &effect, NULL);
		effect_ = COMPtr<ID3DXEffect>(effect);
	}

	D3D9RenderEffect::D3D9RenderEffect(const D3D9RenderEffect& rhs)
	{
		D3D9RenderEngine& renderEngine(static_cast<D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		ID3DXEffect* effect;
		rhs.effect_->CloneEffect(renderEngine.D3DDevice().Get(), &effect);
		effect_ = COMPtr<ID3DXEffect>(effect);
	}

	RenderEffectPtr D3D9RenderEffect::Clone() const
	{
		return RenderEffectPtr(new D3D9RenderEffect(*this));
	}

	void D3D9RenderEffect::Desc(UINT& parameters, UINT& techniques, UINT& functions)
	{
		D3DXEFFECT_DESC desc;
		TIF(effect_->GetDesc(&desc));

		parameters = desc.Parameters;
		techniques = desc.Techniques;
		functions = desc.Functions;
	}

	RenderEffectParameterPtr D3D9RenderEffect::Parameter(UINT index)
	{
		return RenderEffectParameterPtr(new D3D9RenderEffectParameter(effect_,
			effect_->GetParameter(NULL, index)));
	}

	RenderEffectParameterPtr D3D9RenderEffect::ParameterByName(const std::string& name)
	{
		return RenderEffectParameterPtr(new D3D9RenderEffectParameter(effect_,
			effect_->GetParameterByName(NULL, name.c_str())));
	}

	RenderEffectParameterPtr D3D9RenderEffect::ParameterBySemantic(const std::string& semantic)
	{
		return RenderEffectParameterPtr(new D3D9RenderEffectParameter(effect_,
			effect_->GetParameterBySemantic(NULL, semantic.c_str())));
	}

	void D3D9RenderEffect::SetTechnique(const std::string& technique)
	{
		D3DXHANDLE handle(effect_->GetTechniqueByName(technique.c_str()));
		if (this->Validate(handle))
		{
			effect_->SetTechnique(handle);
		}
	}

	void D3D9RenderEffect::SetTechnique(UINT technique)
	{
		D3DXHANDLE handle(effect_->GetTechnique(technique));
		if (this->Validate(handle))
		{
			effect_->SetTechnique(handle);
		}
	}

	bool D3D9RenderEffect::Validate(D3DXHANDLE handle)
	{
		return SUCCEEDED(effect_->ValidateTechnique(handle));
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


	D3D9RenderEffectParameter::D3D9RenderEffectParameter(COMPtr<ID3DXEffect> effect, D3DXHANDLE parameter)
		: effect_(effect), parameter_(parameter)
	{
	}
	
	void D3D9RenderEffectParameter::SetFloat(float value)
	{
		TIF(effect_->SetFloat(parameter_, value));
	}

	float D3D9RenderEffectParameter::GetFloat() const
	{
		float value;
		TIF(effect_->GetFloat(parameter_, &value));
		return value;
	}

	void D3D9RenderEffectParameter::SetVector(const Vector4& value)
	{
		D3DXVECTOR4 vec(value.x(), value.y(), value.z(), value.w());
		TIF(effect_->SetVector(parameter_, &vec));
	}

	Vector4 D3D9RenderEffectParameter::GetVector() const
	{
		D3DXVECTOR4 vec;
		TIF(effect_->GetVector(parameter_, &vec));

		return Vector4(vec.x, vec.y, vec.z, vec.w);
	}

	void D3D9RenderEffectParameter::SetMatrix(const Matrix4& value)
	{
		D3DXMATRIX mat(&(*value.begin()));
		TIF(effect_->SetMatrix(parameter_, &mat));
	}

	Matrix4 D3D9RenderEffectParameter::GetMatrix() const
	{
		D3DXMATRIX mat;
		TIF(effect_->GetMatrix(parameter_, &mat));
		return Matrix4(mat);
	}

	void D3D9RenderEffectParameter::SetMatrixArray(const Matrix4* matrices, size_t count)
	{
		TIF(effect_->SetMatrixArray(parameter_,
			reinterpret_cast<const D3DXMATRIX*>(matrices), static_cast<::UINT>(count)));
	}

	void D3D9RenderEffectParameter::GetMatrixArray(Matrix4* matrices, size_t count)
	{
		TIF(effect_->GetMatrixArray(parameter_,
			reinterpret_cast<D3DXMATRIX*>(matrices), static_cast<::UINT>(count)));
	}	

	void D3D9RenderEffectParameter::SetInt(int value)
	{
		TIF(effect_->SetInt(parameter_, value));
	}

	int D3D9RenderEffectParameter::GetInt() const
	{
		int value;
		TIF(effect_->GetInt(parameter_, &value));
		return value;
	}

	void D3D9RenderEffectParameter::SetBool(bool value)
	{
		TIF(effect_->SetBool(parameter_, value));
	}

	bool D3D9RenderEffectParameter::GetBool() const
	{
		BOOL value;
		TIF(effect_->GetBool(parameter_, &value));
		return value == TRUE;
	}

	void D3D9RenderEffectParameter::SetString(const std::string& value)
	{
		TIF(effect_->SetString(parameter_, value.c_str()));
	}

	std::string D3D9RenderEffectParameter::GetString() const
	{
		const char* value(NULL);
		TIF(effect_->GetString(parameter_, &value));
		return std::string(value);
	}

	void D3D9RenderEffectParameter::SetTexture(const TexturePtr& tex)
	{
		IDirect3DTexture9* texture(NULL);
		if (tex.Get() != NULL)
		{
			texture = static_cast<D3D9Texture*>(tex.Get())->D3DTexture().Get();
		}
		TIF(effect_->SetTexture(parameter_, texture));
	}
}
