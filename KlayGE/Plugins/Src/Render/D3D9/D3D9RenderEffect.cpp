// D3D9RenderEffect.cpp
// KlayGE D3D9渲染效果类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 增加了D3D9RenderEffectInclude (2006.7.12)
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
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/ResLoader.hpp>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 4189)
#endif
#include <boost/algorithm/string/split.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <functional>
#include <string>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderEffect.hpp>

namespace KlayGE
{
	D3D9RenderEffect::D3D9RenderEffect(std::string const & srcData)
	{
		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		D3D9RenderEffectInclude include;

		ID3DXEffect* effect;
		D3DXCreateEffect(renderEngine.D3DDevice().get(), srcData.c_str(),
			static_cast<UINT>(srcData.size()), NULL, &include,
			0, NULL, &effect, NULL);
		d3dx_effect_ = MakeCOMPtr(effect);

		D3DXEFFECT_DESC desc;
		d3dx_effect_->GetDesc(&desc);

		for (uint32_t i = 0; i < desc.Parameters; ++ i)
		{
			RenderEffectParameterPtr reparam;

			D3DXPARAMETER_DESC param_desc;
			d3dx_effect_->GetParameterDesc(d3dx_effect_->GetParameter(NULL, i), &param_desc);

			std::string const name = param_desc.Name;
			std::string semantic;
			if (param_desc.Semantic)
			{
				semantic = param_desc.Semantic;
			}

			if (D3DXPC_SCALAR == param_desc.Class)
			{
				if (D3DXPT_FLOAT == param_desc.Type)
				{
					if (0 == param_desc.Elements)
					{
						reparam.reset(new D3D9RenderEffectParameterFloat(*this, name, semantic));
					}
					else
					{
						reparam.reset(new D3D9RenderEffectParameterFloatArray(*this, name, semantic));
					}
				}
				else
				{
					if (D3DXPT_INT == param_desc.Type)
					{
						if (0 == param_desc.Elements)
						{
							reparam.reset(new D3D9RenderEffectParameterInt(*this, name, semantic));
						}
						else
						{
							reparam.reset(new D3D9RenderEffectParameterIntArray(*this, name, semantic));
						}
					}
					else
					{
						if (D3DXPT_BOOL == param_desc.Type)
						{
							if (0 == param_desc.Elements)
							{
								reparam.reset(new D3D9RenderEffectParameterBool(*this, name, semantic));
							}
							else
							{
								reparam.reset(new D3D9RenderEffectParameterBoolArray(*this, name, semantic));
							}
						}
					}
				}
			}
			else
			{
				if ((D3DXPC_VECTOR == param_desc.Class) && (D3DXPT_FLOAT == param_desc.Type) && (1 == param_desc.Rows))
				{
					if (0 == param_desc.Elements)
					{
						switch (param_desc.Columns)
						{
						case 2:
							reparam.reset(new D3D9RenderEffectParameterFloat2(*this, name, semantic));
							break;

						case 3:
							reparam.reset(new D3D9RenderEffectParameterFloat3(*this, name, semantic));
							break;
						
						case 4:
							reparam.reset(new D3D9RenderEffectParameterFloat4(*this, name, semantic));
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
					}
					else
					{
						reparam.reset(new D3D9RenderEffectParameterFloat4Array(*this, name, semantic));
					}
				}
				else
				{
					if ((D3DXPC_MATRIX_ROWS == param_desc.Class) && (D3DXPT_FLOAT == param_desc.Type))
					{
						if (0 == param_desc.Elements)
						{
							reparam.reset(new D3D9RenderEffectParameterFloat4x4(*this, name, semantic));
						}
						else
						{
							reparam.reset(new D3D9RenderEffectParameterFloat4x4Array(*this, name, semantic));
						}
					}
					else
					{
						if ((D3DXPC_OBJECT == param_desc.Class)
								&& ((D3DXPT_SAMPLER == param_desc.Type) || (D3DXPT_SAMPLER1D == param_desc.Type) || (D3DXPT_SAMPLER2D == param_desc.Type)
									|| (D3DXPT_SAMPLER3D == param_desc.Type) || (D3DXPT_SAMPLERCUBE == param_desc.Type)))
						{
							reparam.reset(new D3D9RenderEffectParameterSampler(*this, name, semantic));
						}
						else
						{
							BOOST_ASSERT(false);
						}
					}
				}
			}

			params_.push_back(reparam);
		}

		for (uint32_t i = 0; i < desc.Techniques; ++ i)
		{
			techniques_.push_back(this->MakeRenderTechnique(i));
		}
	}

	void D3D9RenderEffect::DoOnLostDevice()
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			RenderEffectParameterPtr param = *iter;
			if (dynamic_cast<D3D9RenderEffectParameterSampler*>(param.get()) != NULL)
			{
				SamplerPtr s;
				param->Value(s);
				if (s && s->GetTexture())
				{
					checked_pointer_cast<D3D9Texture>(s->GetTexture())->OnLostDevice();
				}

				param->Dirty();
			}
		}

		TIF(d3dx_effect_->OnLostDevice());
	}

	void D3D9RenderEffect::DoOnResetDevice()
	{
		TIF(d3dx_effect_->OnResetDevice());

		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			RenderEffectParameterPtr param = *iter;
			if (dynamic_cast<D3D9RenderEffectParameterSampler*>(param.get()) != NULL)
			{
				SamplerPtr s;
				param->Value(s);
				if (s && s->GetTexture())
				{
					checked_pointer_cast<D3D9Texture>(s->GetTexture())->OnResetDevice();
				}

				param->Dirty();
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
		checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->GetTechniqueDesc(tech_, &desc);
		for (uint32_t i = 0; i < desc.Passes; ++ i)
		{
			passes_.push_back(this->MakeRenderPass(i));
		}
	}

	bool D3D9RenderTechnique::Validate()
	{
		return SUCCEEDED(checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->ValidateTechnique(tech_));
	}

	uint32_t D3D9RenderTechnique::DoBegin(uint32_t flags)
	{
		TIF(checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->SetTechnique(tech_));

		UINT passes;
		TIF(checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->Begin(&passes, D3DXFX_DONOTSAVESAMPLERSTATE | flags));
		return passes;
	}

	void D3D9RenderTechnique::DoEnd()
	{
		TIF(checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->End());
	}

	RenderPassPtr D3D9RenderTechnique::MakeRenderPass(uint32_t n)
	{
		D3DXHANDLE pass = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->GetPass(tech_, n);
		BOOST_ASSERT(pass != NULL);

		RenderPassPtr ret(new D3D9RenderPass(effect_, n, pass));
		return ret;
	}


	D3D9RenderPass::D3D9RenderPass(RenderEffect& effect, uint32_t index, D3DXHANDLE pass)
		: RenderPass(effect, index),
			pass_(pass)
	{
		D3DXPASS_DESC desc;
		checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->GetPassDesc(pass, &desc);

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

		TIF(checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->BeginPass(index_));
	}

	void D3D9RenderPass::End()
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		TIF(checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect()->EndPass());

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

	void D3D9RenderEffectParameterFloat2::DoFlush(float2 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat3::DoFlush(float3 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat4::DoFlush(float4 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat4x4::DoFlush(float4x4 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterSampler::DoFlush(SamplerPtr const & /*value*/)
	{
	}

	void D3D9RenderEffectParameterSampler::DoOnLostDevice()
	{
		if (val_)
		{
			D3D9Texture& texture = *checked_pointer_cast<D3D9Texture>(val_->GetTexture());
			texture.OnLostDevice();
		}
	}

	void D3D9RenderEffectParameterSampler::DoOnResetDevice()
	{
		if (val_)
		{
			D3D9Texture& texture = *checked_pointer_cast<D3D9Texture>(val_->GetTexture());
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

	void D3D9RenderEffectParameterFloat4Array::DoFlush(std::vector<float4> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetVectorArray(name_.c_str(), reinterpret_cast<D3DXVECTOR4 const *>(&value[0]),
			static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterFloat4x4Array::DoFlush(std::vector<float4x4> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetMatrixArray(name_.c_str(), reinterpret_cast<D3DXMATRIX const *>(&value[0]),
			static_cast<UINT>(value.size())));
	}

	HRESULT D3D9RenderEffectInclude::Open(D3DXINCLUDE_TYPE /*IncludeType*/, LPCSTR pFileName,
		LPCVOID /*pParentData*/, LPCVOID* ppData, UINT* pBytes)
	{
		ResLoader& loader = ResLoader::Instance();

		if (!loader.Locate(pFileName).empty())
		{
			ResIdentifierPtr res = loader.Load(pFileName);

			res->seekg(0, std::ios_base::end);
			std::streamsize size = res->tellg();
			res->seekg(0);

			char* pData = new char[size];
			res->read(pData, size);

			*ppData = pData;
			*pBytes = size;

			return S_OK;
		}
		else
		{
			*ppData = NULL;
			*pBytes = 0;

			return E_FAIL;
		}
	}

	HRESULT D3D9RenderEffectInclude::Close(LPCVOID pData)
	{
		char const * pData2 = static_cast<char const *>(pData);
		delete[] pData2;

		return S_OK;
	}
}
