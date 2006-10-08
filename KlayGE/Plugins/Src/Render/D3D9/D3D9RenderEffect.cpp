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
	D3D9RenderEffect::D3D9RenderEffect(ResIdentifierPtr const & source)
	{
		source->seekg(0, std::ios_base::end);
		std::vector<char> data(source->tellg());
		source->seekg(0);
		source->read(&data[0], static_cast<std::streamsize>(data.size()));

		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		D3D9RenderEffectInclude include;

		ID3DXEffect* effect;
		D3DXCreateEffect(renderEngine.D3DDevice().get(), &data[0],
			static_cast<UINT>(data.size()), NULL, &include,
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
				switch (param_desc.Type)
				{
				case D3DXPT_FLOAT:
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
					break;

				case D3DXPT_INT:
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
					break;

				case D3DXPT_BOOL:
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
					break;

				default:
					BOOST_ASSERT(false);
					break;
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
		return RenderTechniquePtr(new D3D9RenderTechnique(*this, desc.Name, tech));
	}


	D3D9RenderTechnique::D3D9RenderTechnique(RenderEffect& effect, std::string const & name, D3DXHANDLE tech)
		: RenderTechnique(effect, name),
			tech_(tech)
	{
		D3DXTECHNIQUE_DESC desc;
		this->D3DXEffect()->GetTechniqueDesc(tech_, &desc);
		for (uint32_t i = 0; i < desc.Passes; ++ i)
		{
			passes_.push_back(this->MakeRenderPass(i));
		}
	}

	bool D3D9RenderTechnique::Validate()
	{
		return SUCCEEDED(this->D3DXEffect()->ValidateTechnique(tech_));
	}

	void D3D9RenderTechnique::DoBegin(uint32_t flags)
	{
		TIF(this->D3DXEffect()->SetTechnique(tech_));

		UINT passes;
		TIF(this->D3DXEffect()->Begin(&passes, D3DXFX_DONOTSAVESHADERSTATE | D3DXFX_DONOTSAVESAMPLERSTATE | flags));
	}

	void D3D9RenderTechnique::DoEnd()
	{
		TIF(this->D3DXEffect()->End());
	}

	RenderPassPtr D3D9RenderTechnique::MakeRenderPass(uint32_t n)
	{
		D3DXHANDLE pass = this->D3DXEffect()->GetPass(tech_, n);
		BOOST_ASSERT(pass != NULL);

		return RenderPassPtr(new D3D9RenderPass(effect_, n, pass));
	}

	ID3DXEffectPtr const & D3D9RenderTechnique::D3DXEffect() const
	{
		return checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
	}


	D3D9RenderPass::D3D9RenderPass(RenderEffect& effect, uint32_t index, D3DXHANDLE pass)
		: RenderPass(effect, index),
			pass_(pass)
	{
		D3DXPASS_DESC desc;
		this->D3DXEffect()->GetPassDesc(pass, &desc);

		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		if (desc.pVertexShaderFunction != NULL)
		{
			ID3DXConstantTable* constant_table;
			D3DXGetShaderConstantTable(desc.pVertexShaderFunction, &constant_table);
			constant_table_[0] = MakeCOMPtr(constant_table);

			IDirect3DVertexShader9* shader;
			d3d_device->CreateVertexShader(desc.pVertexShaderFunction, &shader);
			vertex_shader_ = MakeCOMPtr(shader);
		}
		if (desc.pPixelShaderFunction != NULL)
		{
			ID3DXConstantTable* constant_table;
			D3DXGetShaderConstantTable(desc.pPixelShaderFunction, &constant_table);
			constant_table_[1] = MakeCOMPtr(constant_table);

			IDirect3DPixelShader9* shader;
			d3d_device->CreatePixelShader(desc.pPixelShaderFunction, &shader);
			pixel_shader_ = MakeCOMPtr(shader);
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

					RenderEffectParameterPtr param = effect_.ParameterByName(constant_desc.Name);
					D3D9RenderEffectParameterDesc desc;

					if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterSampler>(param))
					{
						UINT sampler_index = constant_table_[i]->GetSamplerIndex(handle);
						if (0 == i)
						{
							sampler_index += D3DVERTEXTEXTURESAMPLER0;
						}

						desc.RegisterInfo(0 == i, sampler_index, 1);
					}
					else
					{
						desc.RegisterInfo(0 == i, constant_desc.RegisterIndex, constant_desc.RegisterCount);
					}

					parameters_[i].push_back(std::make_pair(param, desc));
				}
			}
		}
	}

	void D3D9RenderPass::Begin()
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		d3d_device->SetVertexShader(vertex_shader_.get());
		d3d_device->SetPixelShader(pixel_shader_.get());

		for (int i = 0; i < 2; ++ i)
		{
			for (std::vector<std::pair<RenderEffectParameterPtr, D3D9RenderEffectParameterDesc> >::iterator iter = parameters_[i].begin();
				iter != parameters_[i].end(); ++ iter)
			{
				RenderEffectParameterPtr param = iter->first;

				if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterBool>(param))
				{
					checked_pointer_cast<D3D9RenderEffectParameterBool>(param)->Flush(iter->second);
				}
				else
				{
					if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterInt>(param))
					{
						checked_pointer_cast<D3D9RenderEffectParameterInt>(param)->Flush(iter->second);
					}
					else
					{
						if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterFloat>(param))
						{
							checked_pointer_cast<D3D9RenderEffectParameterFloat>(param)->Flush(iter->second);
						}
						else
						{
							if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterFloat2>(param))
							{
								checked_pointer_cast<D3D9RenderEffectParameterFloat2>(param)->Flush(iter->second);
							}
							else
							{
								if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterFloat3>(param))
								{
									checked_pointer_cast<D3D9RenderEffectParameterFloat3>(param)->Flush(iter->second);
								}
								else
								{
									if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterFloat4>(param))
									{
										checked_pointer_cast<D3D9RenderEffectParameterFloat4>(param)->Flush(iter->second);
									}
									else
									{
										if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterFloat4x4>(param))
										{
											checked_pointer_cast<D3D9RenderEffectParameterFloat4x4>(param)->Flush(iter->second);
										}
										else
										{
											if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterSampler>(param))
											{
												checked_pointer_cast<D3D9RenderEffectParameterSampler>(param)->Flush(iter->second);
											}
											else
											{
												if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterBoolArray>(param))
												{
													checked_pointer_cast<D3D9RenderEffectParameterBoolArray>(param)->Flush(iter->second);
												}
												else
												{
													if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterIntArray>(param))
													{
														checked_pointer_cast<D3D9RenderEffectParameterIntArray>(param)->Flush(iter->second);
													}
													else
													{
														if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterFloatArray>(param))
														{
															checked_pointer_cast<D3D9RenderEffectParameterFloatArray>(param)->Flush(iter->second);
														}
														else
														{
															if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterFloat4Array>(param))
															{
																checked_pointer_cast<D3D9RenderEffectParameterFloat4Array>(param)->Flush(iter->second);
															}
															else
															{
																if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterFloat4x4Array>(param))
																{
																	checked_pointer_cast<D3D9RenderEffectParameterFloat4x4Array>(param)->Flush(iter->second);
																}
																else
																{
																	BOOST_ASSERT(false);
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		TIF(this->D3DXEffect()->BeginPass(index_));
	}

	void D3D9RenderPass::End()
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		TIF(this->D3DXEffect()->EndPass());

		for (int i = 0; i < 2; ++ i)
		{
			for (std::vector<std::pair<RenderEffectParameterPtr, D3D9RenderEffectParameterDesc> >::iterator iter = parameters_[i].begin();
				iter != parameters_[i].end(); ++ iter)
			{
				if (boost::dynamic_pointer_cast<D3D9RenderEffectParameterSampler>(iter->first))
				{
					render_eng.DisableSampler(iter->second.RegisterStart());
				}
			}
		}
	}

	ID3DXEffectPtr const & D3D9RenderPass::D3DXEffect() const
	{
		return checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
	}


	void D3D9RenderEffectParameterDesc::RegisterInfo(bool is_vertex_shader, uint32_t register_start, uint32_t register_count)
	{
		is_vertex_shader_ = is_vertex_shader;

		register_start_ = register_start;
		register_count_ = register_count;
	}


	void D3D9RenderEffectParameterBool::DoFlush(bool const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		BOOL tmp = value;
		TIF(d3dx_effect->SetValue(name_.c_str(), &tmp, sizeof(tmp)));
	}

	void D3D9RenderEffectParameterBool::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		BOOL tmp[4] = { val_, FALSE, FALSE, FALSE };
		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantB(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantB(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterInt::DoFlush(int const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterInt::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		int tmp[4] = { val_, 0, 0, 0 };
		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantI(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantI(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterFloat::DoFlush(float const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		float tmp[4] = { val_, 0, 0, 0 };
		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantF(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantF(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterFloat2::DoFlush(float2 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat2::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		float tmp[4] = { val_.x(), val_.y(), 0, 0 };
		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantF(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantF(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterFloat3::DoFlush(float3 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat3::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		float tmp[4] = { val_.x(), val_.y(), val_.z(), 0 };
		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantF(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantF(desc.RegisterStart(), tmp, desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterFloat4::DoFlush(float4 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat4::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantF(desc.RegisterStart(), &val_[0], desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantF(desc.RegisterStart(), &val_[0], desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterFloat4x4::DoFlush(float4x4 const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetValue(name_.c_str(), &value, sizeof(value)));
	}

	void D3D9RenderEffectParameterFloat4x4::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantF(desc.RegisterStart(), &val_[0], desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantF(desc.RegisterStart(), &val_[0], desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterSampler::DoFlush(SamplerPtr const & /*value*/)
	{
	}

	void D3D9RenderEffectParameterSampler::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		render_eng.SetSampler(desc.RegisterStart(), val_);
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

	void D3D9RenderEffectParameterBoolArray::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		std::vector<BOOL> tmp(val_.begin(), val_.end());
		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantB(desc.RegisterStart(), &tmp[0], desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantB(desc.RegisterStart(), &tmp[0], desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterIntArray::DoFlush(std::vector<int> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetIntArray(name_.c_str(), &value[0], static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterIntArray::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantI(desc.RegisterStart(), &val_[0], desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantI(desc.RegisterStart(), &val_[0], desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterFloatArray::DoFlush(std::vector<float> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetFloatArray(name_.c_str(), &value[0], static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterFloatArray::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantF(desc.RegisterStart(), &val_[0], desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantF(desc.RegisterStart(), &val_[0], desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterFloat4Array::DoFlush(std::vector<float4> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetVectorArray(name_.c_str(), reinterpret_cast<D3DXVECTOR4 const *>(&value[0]),
			static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterFloat4Array::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantF(desc.RegisterStart(), &val_[0][0], desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantF(desc.RegisterStart(), &val_[0][0], desc.RegisterCount());
		}
	}

	void D3D9RenderEffectParameterFloat4x4Array::DoFlush(std::vector<float4x4> const & value)
	{
		ID3DXEffectPtr d3dx_effect = checked_cast<D3D9RenderEffect*>(&effect_)->D3DXEffect();
		TIF(d3dx_effect->SetMatrixArray(name_.c_str(), reinterpret_cast<D3DXMATRIX const *>(&value[0]),
			static_cast<UINT>(value.size())));
	}

	void D3D9RenderEffectParameterFloat4x4Array::Flush(D3D9RenderEffectParameterDesc const & desc)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		if (desc.IsVertexShader())
		{
			d3d_device->SetVertexShaderConstantF(desc.RegisterStart(), &val_[0][0], desc.RegisterCount());
		}
		else
		{
			d3d_device->SetPixelShaderConstantF(desc.RegisterStart(), &val_[0][0], desc.RegisterCount());
		}
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
