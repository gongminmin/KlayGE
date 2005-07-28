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

#include <boost/assert.hpp>

#include <KlayGE/D3D9/D3D9RenderEffect.hpp>

namespace KlayGE
{
	D3D9RenderEffect::D3D9RenderEffect(std::string const & srcData)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()) != NULL);

		D3D9RenderEngine& renderEngine(static_cast<D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		ID3DXEffect* effect;
		D3DXCreateEffect(renderEngine.D3DDevice().get(), srcData.c_str(),
			static_cast<UINT>(srcData.size()), NULL, NULL,
			0, NULL, &effect, NULL);
		effect_ = MakeCOMPtr(effect);
	}

	void D3D9RenderEffect::Desc(uint32_t& parameters, uint32_t& techniques, uint32_t& functions)
	{
		D3DXEFFECT_DESC desc;
		TIF(effect_->GetDesc(&desc));

		parameters = desc.Parameters;
		techniques = desc.Techniques;
		functions = desc.Functions;
	}

	std::string D3D9RenderEffect::DoNameBySemantic(std::string const & semantic)
	{
		return effect_->GetParameterBySemantic(NULL, semantic.c_str());
	}

	RenderEffectParameterPtr D3D9RenderEffect::DoParameterByName(std::string const & name)
	{
		return D3D9RenderEffectParameterPtr(new D3D9RenderEffectParameter(*this, name));
	}

	bool D3D9RenderEffect::Validate(std::string const & technique)
	{
		return SUCCEEDED(effect_->ValidateTechnique(technique.c_str()));
	}

	void D3D9RenderEffect::SetTechnique(std::string const & technique)
	{
		TIF(effect_->SetTechnique(technique.c_str()));
	}

	uint32_t D3D9RenderEffect::DoBegin(uint32_t flags)
	{
		UINT passes;
		TIF(effect_->Begin(&passes, flags));
		return passes;
	}

	void D3D9RenderEffect::DoEnd()
	{
		TIF(effect_->End());
	}

	void D3D9RenderEffect::BeginPass(uint32_t passNum)
	{
		TIF(effect_->BeginPass(passNum));
	}

	void D3D9RenderEffect::EndPass()
	{
		TIF(effect_->EndPass());
	}

	void D3D9RenderEffect::DoOnLostDevice()
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			static_cast<D3D9RenderEffectParameter*>(iter->second.first.get())->OnLostDevice();
			iter->second.second = true;
		}

		TIF(effect_->OnLostDevice());
	}

	void D3D9RenderEffect::DoOnResetDevice()
	{
		TIF(effect_->OnResetDevice());

		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			static_cast<D3D9RenderEffectParameter*>(iter->second.first.get())->OnResetDevice();
			iter->second.second = true;
		}
	}


	D3D9RenderEffectParameter::D3D9RenderEffectParameter(RenderEffect& effect, std::string const & name)
									: RenderEffectParameter(effect, name)
	{
	}

	bool D3D9RenderEffectParameter::DoTestType(RenderEffectParameterType type)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();

		D3DXPARAMETER_DESC desc;
		d3dx_effect->GetParameterDesc(name_.c_str(), &desc);

		switch (type)
		{
		case REPT_float:
			return (D3DXPC_SCALAR == desc.Class) && (D3DXPT_FLOAT == desc.Type);

		case REPT_Vector4:
			return (D3DXPC_VECTOR == desc.Class) && (D3DXPT_FLOAT == desc.Type) && (1 == desc.Rows) && (4 == desc.Columns);

		case REPT_Matrix4:
			return (D3DXPC_MATRIX_ROWS == desc.Class) && (D3DXPT_FLOAT == desc.Type) && (4 == desc.Rows) && (4 == desc.Columns);

		case REPT_int:
			return (D3DXPC_SCALAR == desc.Class) && (D3DXPT_INT == desc.Type);

		case REPT_Texture:
			return (D3DXPC_OBJECT == desc.Class)
				&& ((D3DXPT_TEXTURE == desc.Type) || (D3DXPT_TEXTURE1D == desc.Type) || (D3DXPT_TEXTURE2D == desc.Type)
					|| (D3DXPT_TEXTURE3D == desc.Type) || (D3DXPT_TEXTURECUBE == desc.Type));

		case REPT_float_array:
			return (D3DXPT_FLOAT == desc.Type) && (desc.Elements != 1);

		case REPT_Vector4_array:
			return (D3DXPC_VECTOR == desc.Class) && (D3DXPT_FLOAT == desc.Type) && (desc.Elements != 1);

		case REPT_Matrix4_array:
			return (D3DXPC_MATRIX_ROWS == desc.Class) && (D3DXPT_FLOAT == desc.Type) && (4 == desc.Rows) && (4 == desc.Columns) && (desc.Elements != 1);

		case REPT_int_array:
			return (D3DXPC_SCALAR == desc.Class) && (D3DXPT_INT == desc.Type) && (desc.Elements != 1);

		default:
			BOOST_ASSERT(false);
			return false;
		}
	}
	
	void D3D9RenderEffectParameter::DoFloat(float value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetFloat(name_.c_str(), value));
	}

	void D3D9RenderEffectParameter::DoVector4(Vector4 const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetVector(name_.c_str(), reinterpret_cast<D3DXVECTOR4 const *>(&value)));
	}

	void D3D9RenderEffectParameter::DoMatrix4(Matrix4 const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetMatrix(name_.c_str(), reinterpret_cast<D3DXMATRIX const *>(&value)));
	}

	void D3D9RenderEffectParameter::DoInt(int value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetInt(name_.c_str(), value));
	}

	void D3D9RenderEffectParameter::DoTexture(TexturePtr const & tex)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		IDirect3DBaseTexture9* texture(NULL);
		if (tex)
		{
			BOOST_ASSERT(dynamic_cast<D3D9Texture*>(tex.get()) != NULL);

			D3D9Texture const & d3d9Tex = static_cast<D3D9Texture const &>(*tex);
			texture = d3d9Tex.D3DBaseTexture().get();
		}

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetTexture(name_.c_str(), texture));
	}

	void D3D9RenderEffectParameter::DoSetFloatArray(float const * value, size_t count)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetFloatArray(name_.c_str(), value, static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::DoSetVector4Array(Vector4 const * value, size_t count)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetVectorArray(name_.c_str(), reinterpret_cast<D3DXVECTOR4 const *>(value),
			static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::DoSetMatrix4Array(Matrix4 const * matrices, size_t count)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetMatrixArray(name_.c_str(),
			reinterpret_cast<D3DXMATRIX const *>(matrices), static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::DoSetIntArray(int const * value, size_t count)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetIntArray(name_.c_str(), value, static_cast<UINT>(count)));
	}

	void D3D9RenderEffectParameter::DoOnLostDevice()
	{
		if (REPT_Texture == type_)
		{
			BOOST_ASSERT(dynamic_cast<D3D9Texture*>(boost::get<TexturePtr>(val_).get()) != NULL);

			D3D9Texture& texture = static_cast<D3D9Texture&>(*boost::get<TexturePtr>(val_));
			texture.OnLostDevice();
		}
	}

	void D3D9RenderEffectParameter::DoOnResetDevice()
	{
		if (REPT_Texture == type_)
		{
			if (boost::get<TexturePtr>(val_))
			{
				BOOST_ASSERT(dynamic_cast<D3D9Texture*>(boost::get<TexturePtr>(val_).get()) != NULL);

				D3D9Texture& texture = static_cast<D3D9Texture&>(*boost::get<TexturePtr>(val_));
				texture.OnResetDevice();

				boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
				TIF(d3dx_effect->SetTexture(name_.c_str(), texture.D3DBaseTexture().get()));
			}
		}
	}
}
