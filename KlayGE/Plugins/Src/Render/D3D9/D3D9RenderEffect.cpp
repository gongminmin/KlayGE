// D3D9RenderEffect.cpp
// KlayGE D3D9渲染效果类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 使用了新的effect系统 (2006.11.1)
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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Sampler.hpp>

#include <boost/assert.hpp>
#include <string>
#include <sstream>
#include <iostream>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderEffect.hpp>

namespace KlayGE
{
	RenderTechniquePtr D3D9RenderEffect::MakeRenderTechnique()
	{
		return RenderTechniquePtr(new D3D9RenderTechnique(*this));
	}


	void D3D9RenderTechnique::DoBegin()
	{
	}

	void D3D9RenderTechnique::DoEnd()
	{
	}

	RenderPassPtr D3D9RenderTechnique::MakeRenderPass()
	{
		return RenderPassPtr(new D3D9RenderPass(effect_));
	}


	void D3D9RenderPass::DoLoad()
	{
		is_validate_ = true;

		std::string s_text = this->shader_text();

		std::string vs_profile = shader_descs_[ST_VERTEX_SHADER].profile;
		std::string const & vs_name = shader_descs_[ST_VERTEX_SHADER].func_name;

		std::string ps_profile = shader_descs_[ST_PIXEL_SHADER].profile;
		std::string const & ps_name = shader_descs_[ST_PIXEL_SHADER].func_name;

		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		ID3DXConstantTable* constant_table[ST_NUM_SHADER_TYPES] = { NULL };

		if (!vs_name.empty())
		{
			if ("auto" == vs_profile)
			{
				vs_profile = D3DXGetVertexShaderProfile(d3d_device.get());
			}

			this->create_vertex_shader(constant_table[ST_VERTEX_SHADER], vs_profile, vs_name, s_text);
		}

		if (!ps_name.empty())
		{
			if ("auto" == ps_profile)
			{
				ps_profile = D3DXGetPixelShaderProfile(d3d_device.get());
			}

			this->create_pixel_shader(constant_table[ST_PIXEL_SHADER], ps_profile, ps_name, s_text);
		}

		for (int i = 0; i < ST_NUM_SHADER_TYPES; ++ i)
		{
			if (constant_table[i])
			{
				uint32_t bool_begin = 0xFFFFFFFF, bool_end = 0;
				uint32_t int_begin = 0xFFFFFFFF, int_end = 0;
				uint32_t float_begin = 0xFFFFFFFF, float_end = 0;

				D3DXCONSTANTTABLE_DESC ct_desc;
				constant_table[i]->GetDesc(&ct_desc);
				for (UINT c = 0; c < ct_desc.Constants; ++ c)
				{
					D3DXHANDLE handle = constant_table[i]->GetConstant(NULL, c);
					D3DXCONSTANT_DESC constant_desc;
					UINT count;
					constant_table[i]->GetConstantDesc(handle, &constant_desc, &count);

					switch (constant_desc.RegisterSet)
					{
					case D3DXRS_BOOL:
						bool_begin = std::min<uint32_t>(bool_begin, constant_desc.RegisterIndex);
						bool_end = std::max<uint32_t>(bool_end, constant_desc.RegisterIndex + constant_desc.RegisterCount);
						break;

					case D3DXRS_INT4:
						int_begin = std::min<uint32_t>(int_begin, constant_desc.RegisterIndex);
						int_end = std::max<uint32_t>(int_end, constant_desc.RegisterIndex + constant_desc.RegisterCount);
						break;

					case D3DXRS_FLOAT4:
						float_begin = std::min<uint32_t>(float_begin, constant_desc.RegisterIndex);
						float_end = std::max<uint32_t>(float_end, constant_desc.RegisterIndex + constant_desc.RegisterCount);
						break;

					default:
						break;
					}

					D3D9RenderParameterDesc p_desc;
					p_desc.param = effect_.ParameterByName(constant_desc.Name);
					p_desc.register_set = constant_desc.RegisterSet;
					p_desc.register_index = constant_desc.RegisterIndex;
					p_desc.register_count = constant_desc.RegisterCount;
					p_desc.rows = constant_desc.Rows;
					p_desc.columns = constant_desc.Columns;

					if ((D3DXRS_SAMPLER == constant_desc.RegisterSet)
						&& (0 == i))
					{
						p_desc.register_index += D3DVERTEXTEXTURESAMPLER0;
					}

					param_descs_[i].push_back(p_desc);
				}

				if (bool_end > bool_begin)
				{
					bool_registers_[i].resize((bool_end - bool_begin) * 4);
					bool_start_[i] = bool_begin;
				}
				if (int_end > int_begin)
				{
					int_registers_[i].resize((int_end - int_begin) * 4);
					int_start_[i] = int_begin;
				}
				if (float_end > float_begin)
				{
					float_registers_[i].resize((float_end - float_begin) * 4);
					float_start_[i] = float_begin;
				}

				constant_table[i]->Release();
			}
		}
	}

	void D3D9RenderPass::compile_shader(ID3DXBuffer*& code, ID3DXConstantTable*& constant_table,
						std::string const & profile, std::string const & name, std::string const & text)
	{
		ID3DXBuffer* pErrMsg;
		D3DXCompileShader(text.c_str(), static_cast<UINT>(text.size()), NULL, NULL, name.c_str(), profile.c_str(),
			0, &code, &pErrMsg, &constant_table);
		if (pErrMsg != NULL)
		{
#ifdef KLAYGE_DEBUG
			std::cerr << text << std::endl;
			std::cerr << static_cast<char*>(pErrMsg->GetBufferPointer()) << std::endl;
#endif
			pErrMsg->Release();
		}
	}

	void D3D9RenderPass::create_vertex_shader(ID3DXConstantTable*& ct, std::string const & profile, std::string const & name, std::string const & text)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		ID3DXBuffer* code;
		compile_shader(code, ct, profile, name, text);
		if (NULL == code)
		{
			is_validate_ = false;
		}
		else
		{
			IDirect3DVertexShader9* vs;
			if (FAILED(d3d_device->CreateVertexShader(static_cast<DWORD*>(code->GetBufferPointer()), &vs)))
			{
				is_validate_ = false;
			}
			vertex_shader_ = MakeCOMPtr(vs);
			code->Release();
		}
	}

	void D3D9RenderPass::create_pixel_shader(ID3DXConstantTable*& ct, std::string const & profile, std::string const & name, std::string const & text)
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		ID3DXBuffer* code;
		compile_shader(code, ct, profile, name, text);
		if (NULL == code)
		{
			is_validate_ = false;
		}
		else
		{
			IDirect3DPixelShader9* ps;
			if (FAILED(d3d_device->CreatePixelShader(static_cast<DWORD*>(code->GetBufferPointer()), &ps)))
			{
				is_validate_ = false;
			}
			pixel_shader_ = MakeCOMPtr(ps);
			code->Release();
		}
	}

	std::string D3D9RenderPass::shader_text() const
	{
		std::stringstream ss;
		for (uint32_t i = 0; i < effect_.NumParameters(); ++ i)
		{
			RenderEffectParameter& param = *effect_.ParameterByIndex(i);

			ss << type_define::instance().type_name(param.type()) << " " << param.Name();
			if (param.ArraySize() != 0)
			{
				ss << "[" << param.ArraySize() << "]";
			}

			ss << ";" << std::endl;
		}

		for (uint32_t i = 0; i < effect_.NumShaders(); ++ i)
		{
			ss << effect_.ShaderByIndex(i).str() << std::endl;
		}

		return ss.str();
	}

	void D3D9RenderPass::DoBegin()
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		d3d_device->SetVertexShader(vertex_shader_.get());
		d3d_device->SetPixelShader(pixel_shader_.get());

		for (int i = 0; i < ST_NUM_SHADER_TYPES; ++ i)
		{
			bool bool_dirty = false;
			bool int_dirty = false;
			bool float_dirty = false;

			for (size_t c = 0; c < param_descs_[i].size(); ++ c)
			{
				D3D9RenderParameterDesc const & desc = param_descs_[i][c];

				RenderEffectParameterPtr param = desc.param;
				if (param->IsDirty())
				{
					switch (desc.register_set)
					{
					case D3DXRS_BOOL:
						if (param->ArraySize() != 0)
						{
							std::vector<bool> tmp;
							param->Value(tmp);
							for (size_t j = 0; j < tmp.size(); ++ j)
							{
								bool_registers_[i][(desc.register_index + j) * 4] = tmp[j];
							}
						}
						else
						{
							bool tmp;
							param->Value(tmp);
							bool_registers_[i][desc.register_index * 4] = tmp;
						}
						bool_dirty = true;
						break;

					case D3DXRS_INT4:
						if (param->ArraySize() != 0)
						{
							std::vector<int> tmp;
							param->Value(tmp);
							for (size_t j = 0; j < tmp.size(); ++ j)
							{
								bool_registers_[i][(desc.register_index + j) * 4] = tmp[j];
							}
						}
						else
						{
							param->Value(int_registers_[i][desc.register_index * 4]);
						}
						int_dirty = true;
						break;

					case D3DXRS_FLOAT4:
						switch (param->type())
						{
						case type_define::TC_int:
							if (param->ArraySize() != 0)
							{
								std::vector<int> tmp;
								param->Value(tmp);
								for (size_t j = 0; j < tmp.size(); ++ j)
								{
									float_registers_[i][(desc.register_index + j) * 4] = static_cast<float>(tmp[j]);
								}
							}
							else
							{
								int v;
								param->Value(v);
								float_registers_[i][desc.register_index * 4] = static_cast<float>(v);
							}
							float_dirty = true;
							break;

						case type_define::TC_float:
							if (param->ArraySize() != 0)
							{
								std::vector<float> tmp;
								param->Value(tmp);
								for (size_t j = 0; j < tmp.size(); ++ j)
								{
									float_registers_[i][(desc.register_index + j) * 4] = tmp[j];
								}
							}
							else
							{
								param->Value(float_registers_[i][desc.register_index * 4]);
							}
							float_dirty = true;
							break;

						case type_define::TC_float2:
							{
								float2 tmp;
								param->Value(tmp);
								memcpy(&float_registers_[i][desc.register_index * 4], &tmp[0], sizeof(tmp));
							}
							float_dirty = true;
							break;

						case type_define::TC_float3:
							{
								float3 tmp;
								param->Value(tmp);
								memcpy(&float_registers_[i][desc.register_index * 4], &tmp[0], sizeof(tmp));
							}
							float_dirty = true;
							break;

						case type_define::TC_float4:
							if (param->ArraySize() != 0)
							{
								std::vector<float4> tmp;
								param->Value(tmp);
								memcpy(&float_registers_[i][desc.register_index * 4], &tmp[0],
									std::min(desc.register_count, static_cast<uint32_t>(tmp.size())) * sizeof(float4));
							}
							else
							{
								float4 tmp;
								param->Value(tmp);
								memcpy(&float_registers_[i][desc.register_index * 4], &tmp[0], sizeof(tmp));
							}
							float_dirty = true;
							break;

						case type_define::TC_float4x4:
							if (param->ArraySize() != 0)
							{
								uint32_t start = desc.register_index;
								std::vector<float4x4> tmp;
								param->Value(tmp);
								for (std::vector<float4x4>::iterator iter = tmp.begin();
									iter != tmp.end(); ++ iter)
								{
									*iter = MathLib::transpose(*iter);
									memcpy(&float_registers_[i][start * 4], &(*iter)[0], desc.rows * sizeof(float4));
									start += desc.rows;
								}
							}
							else
							{
								float4x4 tmp;
								param->Value(tmp);
								tmp = MathLib::transpose(tmp);
								memcpy(&float_registers_[i][desc.register_index * 4], &tmp[0], desc.register_count * sizeof(float4));
							}
							float_dirty = true;
							break;

						default:
							BOOST_ASSERT(false);
							break;
						}
					}

					//param->Dirty(false);
				}
			}

			if (bool_dirty && !bool_registers_[i].empty())
			{
				if (0 == i)
				{
					d3d_device->SetVertexShaderConstantB(bool_start_[i], &bool_registers_[i][0],
						static_cast<UINT>(bool_registers_[i].size()) / 4);
				}
				else
				{
					d3d_device->SetPixelShaderConstantB(bool_start_[i], &bool_registers_[i][0],
						static_cast<UINT>(bool_registers_[i].size()) / 4);
				}
			}
			if (int_dirty && !int_registers_[i].empty())
			{
				if (0 == i)
				{
					d3d_device->SetVertexShaderConstantI(int_start_[i], &int_registers_[i][0],
						static_cast<UINT>(int_registers_[i].size()) / 4);
				}
				else
				{
					d3d_device->SetPixelShaderConstantI(int_start_[i], &int_registers_[i][0],
						static_cast<UINT>(int_registers_[i].size()) / 4);
				}
			}
			if (float_dirty && !float_registers_[i].empty())
			{
				if (0 == i)
				{
					d3d_device->SetVertexShaderConstantF(float_start_[i], &float_registers_[i][0],
						static_cast<UINT>(float_registers_[i].size()) / 4);
				}
				else
				{
					d3d_device->SetPixelShaderConstantF(float_start_[i], &float_registers_[i][0],
						static_cast<UINT>(float_registers_[i].size()) / 4);
				}
			}
		}

		for (int i = 0; i < ST_NUM_SHADER_TYPES; ++ i)
		{
			for (size_t c = 0; c < param_descs_[i].size(); ++ c)
			{
				D3D9RenderParameterDesc const & desc = param_descs_[i][c];
				if (D3DXRS_SAMPLER == desc.register_set)
				{
					SamplerPtr s;
					desc.param->Value(s);
					render_eng.SetSampler(desc.register_index, s);
				}
			}
		}
	}

	void D3D9RenderPass::DoEnd()
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));

		for (int i = 0; i < ST_NUM_SHADER_TYPES; ++ i)
		{
			for (size_t c = 0; c < param_descs_[i].size(); ++ c)
			{
				D3D9RenderParameterDesc const & desc = param_descs_[i][c];
				if (D3DXRS_SAMPLER == desc.register_set)
				{
					render_eng.DisableSampler(desc.register_index);
				}
			}
		}
	}
}
