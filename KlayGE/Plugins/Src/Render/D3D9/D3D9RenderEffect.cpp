// D3D9RenderEffect.cpp
// KlayGE D3D9渲染效果类 实现文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
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
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#include <cassert>

#include <KlayGE/D3D9/D3D9RenderEffect.hpp>

namespace KlayGE
{
	D3D9RenderEffect::D3D9RenderEffect(std::string const & srcData, uint32_t flags)
	{
		D3D9RenderEngine& renderEngine(static_cast<D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		ID3DXEffect* effect;
		D3DXCreateEffect(renderEngine.D3DDevice().get(), srcData.c_str(),
			static_cast<UINT>(srcData.size()), NULL, NULL,
			flags, NULL, &effect, NULL);
		effect_ = MakeCOMPtr(effect);
	}

	D3D9RenderEffect::D3D9RenderEffect(D3D9RenderEffect const & rhs)
	{
		D3D9RenderEngine& renderEngine(static_cast<D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		ID3DXEffect* effect;
		rhs.effect_->CloneEffect(renderEngine.D3DDevice().get(), &effect);
		effect_ = MakeCOMPtr(effect);
	}

	RenderEffectPtr D3D9RenderEffect::Clone() const
	{
		return RenderEffectPtr(new D3D9RenderEffect(*this));
	}

	void D3D9RenderEffect::Desc(uint32_t& parameters, uint32_t& techniques, uint32_t& functions)
	{
		D3DXEFFECT_DESC desc;
		TIF(effect_->GetDesc(&desc));

		parameters = desc.Parameters;
		techniques = desc.Techniques;
		functions = desc.Functions;
	}

	RenderEffectParameterPtr D3D9RenderEffect::Parameter(uint32_t index)
	{
		D3D9RenderEffectParameterPtr ret(new D3D9RenderEffectParameter(effect_,
			effect_->GetParameter(NULL, index)));
		params_.insert(ret);

		return ret;
	}

	RenderEffectParameterPtr D3D9RenderEffect::ParameterByName(std::string const & name)
	{
		D3D9RenderEffectParameterPtr ret(new D3D9RenderEffectParameter(effect_,
			effect_->GetParameterByName(NULL, name.c_str())));
		params_.insert(ret);

		return ret;
	}

	RenderEffectParameterPtr D3D9RenderEffect::ParameterBySemantic(std::string const & semantic)
	{
		D3D9RenderEffectParameterPtr ret(new D3D9RenderEffectParameter(effect_,
			effect_->GetParameterBySemantic(NULL, semantic.c_str())));
		params_.insert(ret);

		return ret;
	}

	void D3D9RenderEffect::SetTechnique(std::string const & technique)
	{
		D3DXHANDLE handle(effect_->GetTechniqueByName(technique.c_str()));
		if (this->Validate(handle))
		{
			effect_->SetTechnique(handle);
		}
	}

	void D3D9RenderEffect::SetTechnique(uint32_t technique)
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

	uint32_t D3D9RenderEffect::Begin(uint32_t flags)
	{
		UINT passes;
		TIF(effect_->Begin(&passes, flags));
		return passes;
	}

	void D3D9RenderEffect::BeginPass(uint32_t passNum)
	{
		TIF(effect_->BeginPass(passNum));
	}

	void D3D9RenderEffect::EndPass()
	{
		TIF(effect_->EndPass());
	}

	void D3D9RenderEffect::End()
	{
		TIF(effect_->End());
	}

	void D3D9RenderEffect::OnLostDevice()
	{
		for (std::set<D3D9RenderEffectParameterPtr>::iterator iter = params_.begin();
			iter != params_.end(); ++ iter)
		{
			(*iter)->OnLostDevice();
		}

		TIF(effect_->OnLostDevice());
	}

	void D3D9RenderEffect::OnResetDevice()
	{
		TIF(effect_->OnResetDevice());

		for (std::set<D3D9RenderEffectParameterPtr>::iterator iter = params_.begin();
			iter != params_.end(); ++ iter)
		{
			(*iter)->OnResetDevice();
		}
	}


	D3D9RenderEffectParameter::D3D9RenderEffectParameter(boost::shared_ptr<ID3DXEffect> const & effect, D3DXHANDLE parameter)
		: effect_(effect), parameter_(parameter)
	{
	}
	
	RenderEffectParameter& D3D9RenderEffectParameter::operator=(float value)
	{
		texture_.reset();

		TIF(effect_->SetFloat(parameter_, value));
		return *this;
	}

	RenderEffectParameter& D3D9RenderEffectParameter::operator=(Vector4 const & value)
	{
		texture_.reset();

		TIF(effect_->SetVector(parameter_, reinterpret_cast<D3DXVECTOR4 const *>(&value)));
		return *this;
	}

	RenderEffectParameter& D3D9RenderEffectParameter::operator=(Matrix4 const & value)
	{
		texture_.reset();

		TIF(effect_->SetMatrix(parameter_, reinterpret_cast<D3DXMATRIX const *>(&value)));
		return *this;
	}

	RenderEffectParameter& D3D9RenderEffectParameter::operator=(int value)
	{
		texture_.reset();

		TIF(effect_->SetInt(parameter_, value));
		return *this;
	}

	RenderEffectParameter& D3D9RenderEffectParameter::operator=(TexturePtr const & tex)
	{
		texture_ = tex;

		IDirect3DTexture9* texture(NULL);
		if (tex)
		{
			texture = static_cast<D3D9Texture*>(tex.get())->D3DTexture().get();
		}
		TIF(effect_->SetTexture(parameter_, texture));

		return *this;
	}

	D3D9RenderEffectParameter::operator float() const
	{
		float value;
		TIF(effect_->GetFloat(parameter_, &value));
		return value;
	}

	D3D9RenderEffectParameter::operator Vector4() const
	{
		D3DXVECTOR4 vec;
		TIF(effect_->GetVector(parameter_, &vec));

		return Vector4(vec.x, vec.y, vec.z, vec.w);
	}

	D3D9RenderEffectParameter::operator Matrix4() const
	{
		D3DXMATRIX mat;
		TIF(effect_->GetMatrix(parameter_, &mat));
		return Matrix4(mat);
	}

	D3D9RenderEffectParameter::operator int() const
	{
		int value;
		TIF(effect_->GetInt(parameter_, &value));
		return value;
	}

	void D3D9RenderEffectParameter::SetFloatArray(float const * value, size_t count)
	{
		TIF(effect_->SetFloatArray(parameter_, value, static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::GetFloatArray(float* value, size_t count)
	{
		TIF(effect_->GetFloatArray(parameter_, value, static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::SetVectorArray(Vector4 const * value, size_t count)
	{
		TIF(effect_->SetVectorArray(parameter_, reinterpret_cast<D3DXVECTOR4 const *>(value),
			static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::GetVectorArray(Vector4* value, size_t count)
	{
		TIF(effect_->GetVectorArray(parameter_, reinterpret_cast<D3DXVECTOR4*>(value),
			static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::SetMatrixArray(Matrix4 const * matrices, size_t count)
	{
		TIF(effect_->SetMatrixArray(parameter_,
			reinterpret_cast<D3DXMATRIX const *>(matrices), static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::GetMatrixArray(Matrix4* matrices, size_t count)
	{
		TIF(effect_->GetMatrixArray(parameter_,
			reinterpret_cast<D3DXMATRIX*>(matrices), static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::SetIntArray(int const * value, size_t count)
	{
		TIF(effect_->SetIntArray(parameter_, value, static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::GetIntArray(int* value, size_t count)
	{
		TIF(effect_->GetIntArray(parameter_, value, static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::OnLostDevice()
	{
		if (texture_)
		{
			D3D9Texture* texture = static_cast<D3D9Texture*>(texture_.get());
			texture->OnLostDevice();
		}
	}

	void D3D9RenderEffectParameter::OnResetDevice()
	{
		if (texture_)
		{
			D3D9Texture* texture = static_cast<D3D9Texture*>(texture_.get());
			texture->OnResetDevice();
			TIF(effect_->SetTexture(parameter_, texture->D3DTexture().get()));
		}
	}
}
