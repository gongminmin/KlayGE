// D3D9RenderEffect.cpp
// KlayGE D3D9渲染效果类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 支持"x.y"形式的参数名 (2005.8.17)
// 优化了Sampler设置 (2005.9.7)
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
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Sampler.hpp>

#include <boost/assert.hpp>
#pragma warning(disable : 4189)
#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <functional>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderEffect.hpp>

namespace KlayGE
{
	D3D9RenderEffect::D3D9RenderEffect(std::string const & srcData)
	{
		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		ID3DXEffect* effect;
		D3DXCreateEffect(renderEngine.D3DDevice().get(), srcData.c_str(),
			static_cast<UINT>(srcData.size()), NULL, NULL,
			0, NULL, &effect, NULL);
		d3dx_effect_ = MakeCOMPtr(effect);

		D3DXEFFECT_DESC desc;
		d3dx_effect_->GetDesc(&desc);
		for (uint32_t i = 0; i < desc.Techniques; ++ i)
		{
			techniques_.push_back(this->MakeRenderTechnique(i));
		}
	}

	std::string D3D9RenderEffect::DoNameBySemantic(std::string const & semantic)
	{
		return d3dx_effect_->GetParameterBySemantic(NULL, semantic.c_str());
	}

	RenderEffectParameterPtr D3D9RenderEffect::DoParameterByName(std::string const & name)
	{
		D3DXPARAMETER_DESC desc;
		std::vector<std::string> path;
		boost::algorithm::split(path, name, boost::bind(std::equal_to<char>(), '.', _1));
		D3DXHANDLE handle = NULL;
		for (std::vector<std::string>::iterator iter = path.begin(); iter != path.end(); ++ iter)
		{
			handle = d3dx_effect_->GetParameterByName(handle, iter->c_str());
			BOOST_ASSERT(handle != NULL);
		}
		d3dx_effect_->GetParameterDesc(handle, &desc);

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
				else
				{
					if (D3DXPT_BOOL == desc.Type)
					{
						if (0 == desc.Elements)
						{
							return RenderEffectParameterPtr(new D3D9RenderEffectParameterBool(*this, name));
						}
						else
						{
							return RenderEffectParameterPtr(new D3D9RenderEffectParameterBoolArray(*this, name));
						}
					}
				}
			}
		}

		if ((D3DXPC_VECTOR == desc.Class) && (D3DXPT_FLOAT == desc.Type) && (1 == desc.Rows))
		{
			if (0 == desc.Elements)
			{
				switch (desc.Columns)
				{
				case 2:
					return RenderEffectParameterPtr(new D3D9RenderEffectParameterVector2(*this, name));

				case 3:
					return RenderEffectParameterPtr(new D3D9RenderEffectParameterVector3(*this, name));
				
				case 4:
					return RenderEffectParameterPtr(new D3D9RenderEffectParameterVector4(*this, name));

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				return RenderEffectParameterPtr(new D3D9RenderEffectParameterVector4Array(*this, name));
			}
		}

		if ((D3DXPC_MATRIX_ROWS == desc.Class) && (D3DXPT_FLOAT == desc.Type))
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
			return RenderEffectParameterPtr(new D3D9RenderEffectParameterSampler(*this, name));
		}

		BOOST_ASSERT(false);
		return RenderEffectParameterPtr();
	}

	void D3D9RenderEffect::DoOnLostDevice()
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			RenderEffectParameterPtr param = iter->second.first;
			if (dynamic_cast<D3D9RenderEffectParameterSampler*>(param.get()) != NULL)
			{
				SamplerPtr s;
				param->Value(s);
				if (s && s->GetTexture())
				{
					checked_cast<D3D9Texture*>(s->GetTexture().get())->OnLostDevice();
				}

				iter->second.second = true;
			}
		}

		TIF(d3dx_effect_->OnLostDevice());
	}

	void D3D9RenderEffect::DoOnResetDevice()
	{
		TIF(d3dx_effect_->OnResetDevice());

		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			RenderEffectParameterPtr param = iter->second.first;
			if (dynamic_cast<D3D9RenderEffectParameterSampler*>(param.get()) != NULL)
			{
				SamplerPtr s;
				param->Value(s);
				if (s && s->GetTexture())
				{
					checked_cast<D3D9Texture*>(s->GetTexture().get())->OnResetDevice();
				}

				iter->second.second = true;
			}
		}
	}

	RenderTechniquePtr D3D9RenderEffect::MakeRenderTechnique(uint32_t n)
	{
		D3DXHANDLE tech = d3dx_effect_->GetTechnique(n);
		BOOST_ASSERT(tech != NULL);

		D3DXTECHNIQUE_DESC desc;
		d3dx_effect_->GetTechniqueDesc(tech, &desc);
		RenderTechniquePtr ret(new D3D9RenderTechnique(*this, desc.Name, tech));
		return ret;
	}


	D3D9RenderTechnique::D3D9RenderTechnique(RenderEffect& effect, std::string const & name, D3DXHANDLE tech)
		: RenderTechnique(effect, name),
			tech_(tech)
	{
		D3DXTECHNIQUE_DESC desc;
		static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->GetTechniqueDesc(tech_, &desc);
		for (uint32_t i = 0; i < desc.Passes; ++ i)
		{
			passes_.push_back(this->MakeRenderPass(i));
		}
	}

	bool D3D9RenderTechnique::Validate()
	{
		return SUCCEEDED(static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->ValidateTechnique(tech_));
	}

	uint32_t D3D9RenderTechnique::DoBegin(uint32_t flags)
	{
		TIF(static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->SetTechnique(tech_));

		UINT passes;
		TIF(static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->Begin(&passes, D3DXFX_DONOTSAVESAMPLERSTATE | flags));
		return passes;
	}

	void D3D9RenderTechnique::DoEnd()
	{
		TIF(static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->End());
	}

	RenderPassPtr D3D9RenderTechnique::MakeRenderPass(uint32_t n)
	{
		D3DXHANDLE pass = static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->GetPass(tech_, n);
		BOOST_ASSERT(pass != NULL);

		RenderPassPtr ret(new D3D9RenderPass(effect_, n, pass));
		return ret;
	}


	D3D9RenderPass::D3D9RenderPass(RenderEffect& effect, uint32_t index, D3DXHANDLE pass)
		: RenderPass(effect, index),
			pass_(pass)
	{
		D3DXPASS_DESC desc;
		static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->GetPassDesc(pass, &desc);

		if (desc.pVertexShaderFunction != NULL)
		{
			ID3DXConstantTable* constant_table;
			D3DXGetShaderConstantTable(desc.pVertexShaderFunction, &constant_table);
			constant_table_[0] = MakeCOMPtr(constant_table);
		}
		if (desc.pPixelShaderFunction != NULL)
		{
			ID3DXConstantTable* constant_table;
			D3DXGetShaderConstantTable(desc.pPixelShaderFunction, &constant_table);
			constant_table_[1] = MakeCOMPtr(constant_table);
		}

		for (int i = 0; i < 2; ++ i)
		{
			if (constant_table_[i])
			{
				D3DXCONSTANTTABLE_DESC ct_desc;
				constant_table_[i]->GetDesc(&ct_desc);
				for (UINT c = 0; c < ct_desc.Constants; ++ c)
				{
					D3DXHANDLE handle = constant_table_[i]->GetConstant(NULL, c);
					D3DXCONSTANT_DESC constant_desc;
					UINT count;
					constant_table_[i]->GetConstantDesc(handle, &constant_desc, &count);
					if ((D3DXPT_SAMPLER == constant_desc.Type)
						|| (D3DXPT_SAMPLER1D == constant_desc.Type)
						|| (D3DXPT_SAMPLER2D == constant_desc.Type)
						|| (D3DXPT_SAMPLER3D == constant_desc.Type)
						|| (D3DXPT_SAMPLERCUBE == constant_desc.Type))
					{
						RenderEffectParameterPtr param = effect_.ParameterByName(constant_desc.Name);
						if (param)
						{
							UINT sampler_index = constant_table_[i]->GetSamplerIndex(handle);
							if (0 == i)
							{
								sampler_index += D3DVERTEXTEXTURESAMPLER0;
							}
							samplers_[i].insert(std::make_pair(param, sampler_index));
						}
					}
				}
			}
		}
	}

	void D3D9RenderPass::Begin()
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		for (int i = 0; i < 2; ++ i)
		{
			for (MapVector<RenderEffectParameterPtr, uint32_t>::iterator iter = samplers_[i].begin();
				iter != samplers_[i].end(); ++ iter)
			{
				SamplerPtr sampler;
				iter->first->Value(sampler);

				render_eng.SetSampler(iter->second, sampler);
			}
		}

		TIF(static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->BeginPass(index_));
	}

	void D3D9RenderPass::End()
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		TIF(static_cast<D3D9RenderEffect&>(effect_).D3DXEffect()->EndPass());

		for (int i = 0; i < 2; ++ i)
		{
			for (MapVector<RenderEffectParameterPtr, uint32_t>::iterator iter = samplers_[i].begin();
				iter != samplers_[i].end(); ++ iter)
			{
				render_eng.DisableSampler(iter->second);
			}
		}
	}


	void D3D9RenderEffectParameterBool::DoFlush(bool const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		BOOL tmp = value;
		TIF(d3dx_effect->SetValue(name_.c_str(), &tmp, sizeof(tmp)));
	}

	void D3D9RenderEffectParameterInt::DoFlush(int const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat::DoFlush(float const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterVector2::DoFlush(float2 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterVector3::DoFlush(float3 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterVector4::DoFlush(float4 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterMatrix4::DoFlush(float4x4 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterSampler::DoFlush(SamplerPtr const & value)
	{
	}

	void D3D9RenderEffectParameterSampler::DoOnLostDevice()
	{
		if (val_)
		{
			D3D9Texture& texture = *checked_cast<D3D9Texture*>(val_->GetTexture().get());
			texture.OnLostDevice();
		}
	}

	void D3D9RenderEffectParameterSampler::DoOnResetDevice()
	{
		if (val_)
		{
			D3D9Texture& texture = *checked_cast<D3D9Texture*>(val_->GetTexture().get());
			texture.OnResetDevice();
		}
	}

	void D3D9RenderEffectParameterBoolArray::DoFlush(std::vector<bool> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		std::vector<BOOL> tmp(value.begin(), value.end());
		TIF(d3dx_effect->SetBoolArray(name_.c_str(), &tmp[0], static_cast<UINT>(tmp.size())));
	}

	void D3D9RenderEffectParameterIntArray::DoFlush(std::vector<int> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetIntArray(name_.c_str(), &value[0], static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterFloatArray::DoFlush(std::vector<float> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetFloatArray(name_.c_str(), &value[0], static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterVector4Array::DoFlush(std::vector<float4> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetVectorArray(name_.c_str(), reinterpret_cast<D3DXVECTOR4 const *>(&value[0]),
			static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterMatrix4Array::DoFlush(std::vector<float4x4> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetMatrixArray(name_.c_str(), reinterpret_cast<D3DXMATRIX const *>(&value[0]),
			static_cast<UINT>(value.size())));
	}
}
