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
#include <KlayGE/Sampler.hpp>
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
		d3dx_effect_ = MakeCOMPtr(effect);
	}

	std::string D3D9RenderEffect::DoNameBySemantic(std::string const & semantic)
	{
		return d3dx_effect_->GetParameterBySemantic(NULL, semantic.c_str());
	}

	RenderEffectParameterPtr D3D9RenderEffect::DoParameterByName(std::string const & name)
	{
		D3DXPARAMETER_DESC desc;
		d3dx_effect_->GetParameterDesc(name.c_str(), &desc);

		if (D3DXPC_SCALAR == desc.Class)
		{
			if (D3DXPT_FLOAT == desc.Type)
			{
				if (0 == desc.Elements)
				{
					return RenderEffectParameterPtr(new D3D9RenderEffectParameterFloat(*this, name));
				}
				else
				{
					return RenderEffectParameterPtr(new D3D9RenderEffectParameterFloatArray(*this, name));
				}
			}
			else
			{
				if (D3DXPT_INT == desc.Type)
				{
					if (0 == desc.Elements)
					{
						return RenderEffectParameterPtr(new D3D9RenderEffectParameterInt(*this, name));
					}
					else
					{
						return RenderEffectParameterPtr(new D3D9RenderEffectParameterIntArray(*this, name));
					}
				}
			}
		}

		if ((D3DXPC_VECTOR == desc.Class) && (D3DXPT_FLOAT == desc.Type) && (1 == desc.Rows) && (4 == desc.Columns))
		{
			if (0 == desc.Elements)
			{
				return RenderEffectParameterPtr(new D3D9RenderEffectParameterVector4(*this, name));
			}
			else
			{
				return RenderEffectParameterPtr(new D3D9RenderEffectParameterVector4Array(*this, name));
			}
		}

		if ((D3DXPC_MATRIX_ROWS == desc.Class) && (D3DXPT_FLOAT == desc.Type) && (4 == desc.Rows) && (4 == desc.Columns))
		{
			if (0 == desc.Elements)
			{
				return RenderEffectParameterPtr(new D3D9RenderEffectParameterMatrix4(*this, name));
			}
			else
			{
				return RenderEffectParameterPtr(new D3D9RenderEffectParameterMatrix4Array(*this, name));
			}
		}

		if ((D3DXPC_OBJECT == desc.Class)
				&& ((D3DXPT_SAMPLER == desc.Type) || (D3DXPT_SAMPLER1D == desc.Type) || (D3DXPT_SAMPLER2D == desc.Type)
					|| (D3DXPT_SAMPLER3D == desc.Type) || (D3DXPT_SAMPLERCUBE == desc.Type)))
		{
			boost::shared_ptr<D3D9RenderEffectParameterSampler> ret(new D3D9RenderEffectParameterSampler(*this, name));
			sampler_params_.push_back(boost::weak_ptr<D3D9RenderEffectParameterSampler>(ret));
			return ret;
		}

		BOOST_ASSERT(false);
		return RenderEffectParameterPtr();
	}

	bool D3D9RenderEffect::Validate(std::string const & technique)
	{
		return SUCCEEDED(d3dx_effect_->ValidateTechnique(technique.c_str()));
	}

	void D3D9RenderEffect::SetTechnique(std::string const & technique)
	{
		TIF(d3dx_effect_->SetTechnique(technique.c_str()));
	}

	uint32_t D3D9RenderEffect::DoBegin(uint32_t flags)
	{
		UINT passes;
		TIF(d3dx_effect_->Begin(&passes, flags));
		return passes;
	}

	void D3D9RenderEffect::DoEnd()
	{
		TIF(d3dx_effect_->End());
	}

	void D3D9RenderEffect::BeginPass(uint32_t passNum)
	{
		TIF(d3dx_effect_->BeginPass(passNum));
	}

	void D3D9RenderEffect::EndPass()
	{
		TIF(d3dx_effect_->EndPass());
	}

	void D3D9RenderEffect::DoOnLostDevice()
	{
		for (sampler_params_type::iterator iter = sampler_params_.begin(); iter != sampler_params_.end(); ++ iter)
		{
			boost::shared_ptr<D3D9RenderEffectParameterSampler> p = iter->lock();
			BOOST_ASSERT(p);
			BOOST_ASSERT(params_.find(p->Name()) != params_.end());

			SamplerPtr s;
			p->Value(s);
			static_cast<D3D9Texture&>(*s->GetTexture()).OnLostDevice();
			params_[p->Name()].second = true;
		}

		TIF(d3dx_effect_->OnLostDevice());
	}

	void D3D9RenderEffect::DoOnResetDevice()
	{
		TIF(d3dx_effect_->OnResetDevice());

		for (sampler_params_type::iterator iter = sampler_params_.begin(); iter != sampler_params_.end(); ++ iter)
		{
			boost::shared_ptr<D3D9RenderEffectParameterSampler> p = iter->lock();
			BOOST_ASSERT(p);
			BOOST_ASSERT(params_.find(p->Name()) != params_.end());

			SamplerPtr s;
			p->Value(s);
			static_cast<D3D9Texture&>(*s->GetTexture()).OnResetDevice();
			params_[p->Name()].second = true;
		}
	}

	
	void D3D9RenderEffectParameterFloat::DoFlush(float const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetFloat(name_.c_str(), value));
	}

	void D3D9RenderEffectParameterVector4::DoFlush(Vector4 const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetVector(name_.c_str(), reinterpret_cast<D3DXVECTOR4 const *>(&value)));
	}

	void D3D9RenderEffectParameterMatrix4::DoFlush(Matrix4 const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetMatrix(name_.c_str(), reinterpret_cast<D3DXMATRIX const *>(&value)));
	}

	void D3D9RenderEffectParameterInt::DoFlush(int const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetInt(name_.c_str(), value));
	}

	void D3D9RenderEffectParameterSampler::DoFlush(SamplerPtr const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);
	}

	void D3D9RenderEffectParameterSampler::DoOnLostDevice()
	{
		if (val_)
		{
			BOOST_ASSERT(dynamic_cast<D3D9Texture*>(val_->GetTexture().get()) != NULL);

			D3D9Texture& texture = static_cast<D3D9Texture&>(*val_->GetTexture());
			texture.OnLostDevice();
		}
	}

	void D3D9RenderEffectParameterSampler::DoOnResetDevice()
	{
		if (val_)
		{
			BOOST_ASSERT(dynamic_cast<D3D9Texture*>(val_->GetTexture().get()) != NULL);

			D3D9Texture& texture = static_cast<D3D9Texture&>(*val_->GetTexture());
			texture.OnResetDevice();

			boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
			TIF(d3dx_effect->SetTexture(name_.c_str(), texture.D3DBaseTexture().get()));
		}
	}

	void D3D9RenderEffectParameterFloatArray::DoFlush(std::vector<float> const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetFloatArray(name_.c_str(), &value[0], static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterVector4Array::DoFlush(std::vector<Vector4> const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetVectorArray(name_.c_str(), reinterpret_cast<D3DXVECTOR4 const *>(&value[0]),
			static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterMatrix4Array::DoFlush(std::vector<Matrix4> const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetMatrixArray(name_.c_str(), reinterpret_cast<D3DXMATRIX const *>(&value[0]),
			static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterIntArray::DoFlush(std::vector<int> const & value)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEffect*>(&effect_) != NULL);

		boost::shared_ptr<ID3DXEffect> d3dx_effect = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect();
		TIF(d3dx_effect->SetIntArray(name_.c_str(), &value[0], static_cast<UINT>(value.size())));
	}
}
