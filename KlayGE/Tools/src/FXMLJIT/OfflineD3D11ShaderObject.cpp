/**
* @file OfflineD3D11ShaderObject.cpp
* @author Minmin Gong
*
* @section DESCRIPTION
*
* This source file is part of KlayGE
* For the latest info, see http://www.klayge.org
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published
* by the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* You may alternatively use this source under the terms of
* the KlayGE Proprietary License (KPL). You can obtained such a license
* from http://www.klayge.org/licensing/.
*/

#include <KlayGE/KlayGE.hpp>
#define INITGUID
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/ResIdentifier.hpp>

#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/functional/hash.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <KlayGE/SALWrapper.hpp>
#include <d3dcompiler.h>
#endif

#include "OfflineRenderEffect.hpp"
#include "OfflineD3D11ShaderObject.hpp"

namespace KlayGE
{
	namespace Offline
	{
		D3D11ShaderObject::D3D11ShaderObject(OfflineRenderDeviceCaps const & caps)
				: ShaderObject(caps), vs_signature_(0)
		{
			is_shader_validate_.fill(true);

			switch (caps.major_version)
			{
			case 12:
				switch (caps.minor_version)
				{
				case 1:
					feature_level_ = D3D_FEATURE_LEVEL_12_1;
					break;

				default:
				case 0:
					feature_level_ = D3D_FEATURE_LEVEL_12_0;
					break;
				}
				break;

			case 11:
				switch (caps.minor_version)
				{
				case 1:
					feature_level_ = D3D_FEATURE_LEVEL_11_1;
					break;

				default:
				case 0:
					feature_level_ = D3D_FEATURE_LEVEL_11_0;
					break;
				}
				break;

			case 10:
				switch (caps.minor_version)
				{
				case 1:
					feature_level_ = D3D_FEATURE_LEVEL_10_1;
					break;

				default:
				case 0:
					feature_level_ = D3D_FEATURE_LEVEL_10_0;
					break;
				}
				break;

			case 9:
				switch (caps.minor_version)
				{
				case 3:
					feature_level_ = D3D_FEATURE_LEVEL_9_3;
					break;

				case 2:
					feature_level_ = D3D_FEATURE_LEVEL_9_2;
					break;

				default:
				case 1:
					feature_level_ = D3D_FEATURE_LEVEL_9_1;
					break;
				}
				break;
			}

			switch (feature_level_)
			{
			case D3D_FEATURE_LEVEL_12_1:
			case D3D_FEATURE_LEVEL_12_0:
			case D3D_FEATURE_LEVEL_11_1:
			case D3D_FEATURE_LEVEL_11_0:
				vs_profile_ = "vs_5_0";
				ps_profile_ = "ps_5_0";
				gs_profile_ = "gs_5_0";
				cs_profile_ = "cs_5_0";
				hs_profile_ = "hs_5_0";
				ds_profile_ = "ds_5_0";
				break;

			case D3D_FEATURE_LEVEL_10_1:
				vs_profile_ = "vs_4_1";
				ps_profile_ = "ps_4_1";
				gs_profile_ = "gs_4_1";
				cs_profile_ = "cs_4_1";
				hs_profile_ = "";
				ds_profile_ = "";
				break;

			case D3D_FEATURE_LEVEL_10_0:
				vs_profile_ = "vs_4_0";
				ps_profile_ = "ps_4_0";
				gs_profile_ = "gs_4_0";
				cs_profile_ = "cs_4_0";
				hs_profile_ = "";
				ds_profile_ = "";
				break;

			case D3D_FEATURE_LEVEL_9_3:
				vs_profile_ = "vs_4_0_level_9_3";
				ps_profile_ = "ps_4_0_level_9_3";
				gs_profile_ = "";
				cs_profile_ = "";
				hs_profile_ = "";
				ds_profile_ = "";
				break;

			default:
				vs_profile_ = "vs_4_0_level_9_1";
				ps_profile_ = "ps_4_0_level_9_1";
				gs_profile_ = "";
				cs_profile_ = "";
				hs_profile_ = "";
				ds_profile_ = "";
				break;
			}
		}

		void D3D11ShaderObject::StreamOut(std::ostream& os, ShaderType type)
		{
			std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

			{
				uint8_t len = static_cast<uint8_t>(shader_code_[type].second.size());
				oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
				oss.write(reinterpret_cast<char const *>(&shader_code_[type].second[0]), len);
			}

			std::shared_ptr<std::vector<uint8_t>> code_blob = shader_code_[type].first;
			if (code_blob)
			{
				uint8_t len;

				uint32_t blob_size = Native2LE(static_cast<uint32_t>(code_blob->size()));
				oss.write(reinterpret_cast<char const *>(&blob_size), sizeof(blob_size));
				oss.write(reinterpret_cast<char const *>(&((*code_blob)[0])), code_blob->size());

				D3D11ShaderDesc const & sd = shader_desc_[type];

				uint16_t cb_desc_size = Native2LE(static_cast<uint16_t>(sd.cb_desc.size()));
				oss.write(reinterpret_cast<char const *>(&cb_desc_size), sizeof(cb_desc_size));
				for (size_t i = 0; i < sd.cb_desc.size(); ++ i)
				{
					len = static_cast<uint8_t>(sd.cb_desc[i].name.size());
					oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
					oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].name[0]), len);

					uint32_t size = Native2LE(sd.cb_desc[i].size);
					oss.write(reinterpret_cast<char const *>(&size), sizeof(size));

					uint16_t var_desc_size = Native2LE(static_cast<uint16_t>(sd.cb_desc[i].var_desc.size()));
					oss.write(reinterpret_cast<char const *>(&var_desc_size), sizeof(var_desc_size));
					for (size_t j = 0; j < sd.cb_desc[i].var_desc.size(); ++ j)
					{
						len = static_cast<uint8_t>(sd.cb_desc[i].var_desc[j].name.size());
						oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
						oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].name[0]), len);

						uint32_t start_offset = Native2LE(sd.cb_desc[i].var_desc[j].start_offset);
						oss.write(reinterpret_cast<char const *>(&start_offset), sizeof(start_offset));
						oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].type), sizeof(sd.cb_desc[i].var_desc[j].type));
						oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].rows), sizeof(sd.cb_desc[i].var_desc[j].rows));
						oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].columns), sizeof(sd.cb_desc[i].var_desc[j].columns));
						uint16_t elements = Native2LE(sd.cb_desc[i].var_desc[j].elements);
						oss.write(reinterpret_cast<char const *>(&elements), sizeof(elements));
					}
				}

				uint16_t num_samplers = Native2LE(sd.num_samplers);
				oss.write(reinterpret_cast<char const *>(&num_samplers), sizeof(num_samplers));
				uint16_t num_srvs = Native2LE(sd.num_srvs);
				oss.write(reinterpret_cast<char const *>(&num_srvs), sizeof(num_srvs));
				uint16_t num_uavs = Native2LE(sd.num_uavs);
				oss.write(reinterpret_cast<char const *>(&num_uavs), sizeof(num_uavs));

				uint16_t res_desc_size = Native2LE(static_cast<uint16_t>(sd.res_desc.size()));
				oss.write(reinterpret_cast<char const *>(&res_desc_size), sizeof(res_desc_size));
				for (size_t i = 0; i < sd.res_desc.size(); ++ i)
				{
					len = static_cast<uint8_t>(sd.res_desc[i].name.size());
					oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
					oss.write(reinterpret_cast<char const *>(&sd.res_desc[i].name[0]), len);

					oss.write(reinterpret_cast<char const *>(&sd.res_desc[i].type), sizeof(sd.res_desc[i].type));
					oss.write(reinterpret_cast<char const *>(&sd.res_desc[i].dimension), sizeof(sd.res_desc[i].dimension));
					uint16_t bind_point = Native2LE(sd.res_desc[i].bind_point);
					oss.write(reinterpret_cast<char const *>(&bind_point), sizeof(bind_point));
				}

				if (ST_VertexShader == type)
				{
					uint32_t vs_signature = Native2LE(vs_signature_);
					oss.write(reinterpret_cast<char const *>(&vs_signature), sizeof(vs_signature));
				}
				else if (ST_ComputeShader == type)
				{
					uint32_t cs_block_size_x = Native2LE(cs_block_size_x_);
					oss.write(reinterpret_cast<char const *>(&cs_block_size_x), sizeof(cs_block_size_x));

					uint32_t cs_block_size_y = Native2LE(cs_block_size_y_);
					oss.write(reinterpret_cast<char const *>(&cs_block_size_y), sizeof(cs_block_size_y));

					uint32_t cs_block_size_z = Native2LE(cs_block_size_z_);
					oss.write(reinterpret_cast<char const *>(&cs_block_size_z), sizeof(cs_block_size_z));
				}
			}

			std::string native_shader_block = oss.str();
			uint32_t len = static_cast<uint32_t>(native_shader_block.size());
			{
				uint32_t tmp = Native2LE(len);
				os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
			}
			if (len > 0)
			{
				os.write(reinterpret_cast<char const *>(&native_shader_block[0]), len * sizeof(native_shader_block[0]));
			}
		}

		std::shared_ptr<std::vector<uint8_t>> D3D11ShaderObject::CompiteToBytecode(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			D3D_FEATURE_LEVEL const feature_level = feature_level_;
			OfflineRenderDeviceCaps const & caps = caps_;

			ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

			is_shader_validate_[type] = true;

			char const * shader_profile = sd.profile.c_str();
			size_t const shader_profile_hash = RT_HASH(shader_profile);
			switch (type)
			{
			case ST_VertexShader:
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = vs_profile_;
				}
				break;

			case ST_PixelShader:
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = ps_profile_;
				}
				break;

			case ST_GeometryShader:
				if (caps.gs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = gs_profile_;
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_ComputeShader:
				if (caps.cs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = cs_profile_;
					}
					if ((CT_HASH("cs_5_0") == shader_profile_hash) && (caps.max_shader_model < ShaderModel(5, 0)))
					{
						is_shader_validate_[type] = false;
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_HullShader:
				if (caps.hs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = hs_profile_;
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_DomainShader:
				if (caps.ds_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = ds_profile_;
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			default:
				is_shader_validate_[type] = false;
				break;
			}
			shader_code_[type].second = shader_profile;

			std::shared_ptr<std::vector<uint8_t>> code = MakeSharedPtr<std::vector<uint8_t>>();
			if (is_shader_validate_[type])
			{
				std::vector<std::pair<char const *, char const *>> macros;
				macros.emplace_back("KLAYGE_D3D11", "1");
				if (feature_level <= D3D_FEATURE_LEVEL_9_3)
				{
					macros.emplace_back("KLAYGE_BC5_AS_AG", "1");
					macros.emplace_back("KLAYGE_BC4_AS_G", "1");
				}
				macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");

				uint32_t flags = 0;
#if !defined(KLAYGE_DEBUG)
				flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
				*code = this->CompileToDXBC(type, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile, flags);

				if (!code->empty())
				{
					ID3D11ShaderReflection* reflection;
					this->ReflectDXBC(*code, reinterpret_cast<void**>(&reflection));
					if (reflection != nullptr)
					{
						D3D11_SHADER_DESC desc;
						reflection->GetDesc(&desc);

						for (UINT c = 0; c < desc.ConstantBuffers; ++ c)
						{
							ID3D11ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

							D3D11_SHADER_BUFFER_DESC d3d_cb_desc;
							reflection_cb->GetDesc(&d3d_cb_desc);
							if ((D3D_CT_CBUFFER == d3d_cb_desc.Type) || (D3D_CT_TBUFFER == d3d_cb_desc.Type))
							{
								D3D11ShaderDesc::ConstantBufferDesc cb_desc;
								cb_desc.name = d3d_cb_desc.Name;
								cb_desc.name_hash = RT_HASH(d3d_cb_desc.Name);
								cb_desc.size = d3d_cb_desc.Size;

								for (UINT v = 0; v < d3d_cb_desc.Variables; ++ v)
								{
									ID3D11ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

									D3D11_SHADER_VARIABLE_DESC var_desc;
									reflection_var->GetDesc(&var_desc);

									D3D11_SHADER_TYPE_DESC type_desc;
									reflection_var->GetType()->GetDesc(&type_desc);

									D3D11ShaderDesc::ConstantBufferDesc::VariableDesc vd;
									vd.name = var_desc.Name;
									vd.start_offset = var_desc.StartOffset;
									vd.type = static_cast<uint8_t>(type_desc.Type);
									vd.rows = static_cast<uint8_t>(type_desc.Rows);
									vd.columns = static_cast<uint8_t>(type_desc.Columns);
									vd.elements = static_cast<uint16_t>(type_desc.Elements);
									cb_desc.var_desc.push_back(vd);
								}

								shader_desc_[type].cb_desc.push_back(cb_desc);
							}
						}

						int num_samplers = -1;
						int num_srvs = -1;
						int num_uavs = -1;
						for (uint32_t i = 0; i < desc.BoundResources; ++ i)
						{
							D3D11_SHADER_INPUT_BIND_DESC si_desc;
							reflection->GetResourceBindingDesc(i, &si_desc);

							switch (si_desc.Type)
							{
							case D3D_SIT_SAMPLER:
								num_samplers = std::max(num_samplers, static_cast<int>(si_desc.BindPoint));
								break;

							case D3D_SIT_TEXTURE:
							case D3D_SIT_STRUCTURED:
							case D3D_SIT_BYTEADDRESS:
								num_srvs = std::max(num_srvs, static_cast<int>(si_desc.BindPoint));
								break;

							case D3D_SIT_UAV_RWTYPED:
							case D3D_SIT_UAV_RWSTRUCTURED:
							case D3D_SIT_UAV_RWBYTEADDRESS:
							case D3D_SIT_UAV_APPEND_STRUCTURED:
							case D3D_SIT_UAV_CONSUME_STRUCTURED:
							case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
								num_uavs = std::max(num_uavs, static_cast<int>(si_desc.BindPoint));
								break;

							default:
								break;
							}
						}

						shader_desc_[type].num_samplers = static_cast<uint16_t>(num_samplers + 1);
						shader_desc_[type].num_srvs = static_cast<uint16_t>(num_srvs + 1);
						shader_desc_[type].num_uavs = static_cast<uint16_t>(num_uavs + 1);

						for (uint32_t i = 0; i < desc.BoundResources; ++ i)
						{
							D3D11_SHADER_INPUT_BIND_DESC si_desc;
							reflection->GetResourceBindingDesc(i, &si_desc);

							switch (si_desc.Type)
							{
							case D3D_SIT_TEXTURE:
							case D3D_SIT_SAMPLER:
							case D3D_SIT_STRUCTURED:
							case D3D_SIT_BYTEADDRESS:
							case D3D_SIT_UAV_RWTYPED:
							case D3D_SIT_UAV_RWSTRUCTURED:
							case D3D_SIT_UAV_RWBYTEADDRESS:
							case D3D_SIT_UAV_APPEND_STRUCTURED:
							case D3D_SIT_UAV_CONSUME_STRUCTURED:
							case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
								if (effect.ParameterByName(si_desc.Name))
								{
									D3D11ShaderDesc::BoundResourceDesc brd;
									brd.name = si_desc.Name;
									brd.type = static_cast<uint8_t>(si_desc.Type);
									brd.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
									shader_desc_[type].res_desc.push_back(brd);
								}
								break;

							default:
								break;
							}
						}

						if (ST_VertexShader == type)
						{
							vs_signature_ = 0;
							D3D11_SIGNATURE_PARAMETER_DESC signature;
							for (uint32_t i = 0; i < desc.InputParameters; ++ i)
							{
								reflection->GetInputParameterDesc(i, &signature);

								size_t seed = boost::hash_range(signature.SemanticName, signature.SemanticName + strlen(signature.SemanticName));
								boost::hash_combine(seed, signature.SemanticIndex);
								boost::hash_combine(seed, signature.Register);
								boost::hash_combine(seed, static_cast<uint32_t>(signature.SystemValueType));
								boost::hash_combine(seed, static_cast<uint32_t>(signature.ComponentType));
								boost::hash_combine(seed, signature.Mask);
								boost::hash_combine(seed, signature.ReadWriteMask);
								boost::hash_combine(seed, signature.Stream);
								boost::hash_combine(seed, signature.MinPrecision);

								size_t sig = vs_signature_;
								boost::hash_combine(sig, seed);
								vs_signature_ = static_cast<uint32_t>(sig);
							}
						}
						else if (ST_ComputeShader == type)
						{
							reflection->GetThreadGroupSize(&cs_block_size_x_, &cs_block_size_y_, &cs_block_size_z_);
						}

						reflection->Release();
					}

					*code = this->StripDXBC(*code, D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO
						| D3DCOMPILER_STRIP_TEST_BLOBS | D3DCOMPILER_STRIP_PRIVATE_DATA);
				}
			}

			if (code->empty())
			{
				code.reset();
			}

			return code;
	#else
			KFL_UNUSED(type);
			KFL_UNUSED(effect);
			KFL_UNUSED(tech);
			KFL_UNUSED(pass);
			KFL_UNUSED(shader_desc_ids);

			return std::shared_ptr<std::vector<uint8_t>>();
	#endif
		}

		void D3D11ShaderObject::AttachShaderBytecode(ShaderType type, RenderEffect const & effect,
			std::vector<uint32_t> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob)
		{
			if (code_blob)
			{
				OfflineRenderDeviceCaps const & caps = caps_;

				ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

				uint8_t shader_major_ver = ("auto" == sd.profile) ? 0 : static_cast<uint8_t>(sd.profile[3] - '0');
				uint8_t shader_minor_ver = ("auto" == sd.profile) ? 0 : static_cast<uint8_t>(sd.profile[5] - '0');
				if (ShaderModel(shader_major_ver, shader_minor_ver) > caps.max_shader_model)
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					switch (type)
					{
					case ST_VertexShader:
						if (!sd.so_decl.empty())
						{
							if (!caps.gs_support)
							{
								is_shader_validate_[type] = false;
							}
						}

						shader_code_[type].first = code_blob;
						break;

					case ST_PixelShader:
						shader_code_[type].first = code_blob;
						break;

					case ST_GeometryShader:
						if (caps.gs_support)
						{
							shader_code_[type].first = code_blob;
						}
						else
						{
							is_shader_validate_[type] = false;
						}
						break;

					case ST_ComputeShader:
						if (caps.cs_support)
						{
							shader_code_[type].first = code_blob;
						}
						else
						{
							is_shader_validate_[type] = false;
						}
						break;

					case ST_HullShader:
						if (caps.hs_support)
						{
							shader_code_[type].first = code_blob;
						}
						else
						{
							is_shader_validate_[type] = false;
						}
						break;

					case ST_DomainShader:
						if (caps.ds_support)
						{
							if (!sd.so_decl.empty())
							{
								if (!caps.gs_support)
								{
									is_shader_validate_[type] = false;
								}
							}

							shader_code_[type].first = code_blob;
						}
						else
						{
							is_shader_validate_[type] = false;
						}
						break;

					default:
						is_shader_validate_[type] = false;
						break;
					}
				}

				// Shader reflection
				cbuff_indices_[type].resize(shader_desc_[type].cb_desc.size());
				for (size_t c = 0; c < shader_desc_[type].cb_desc.size(); ++ c)
				{
					bool found = false;
					for (uint32_t i = 0; i < effect.NumCBuffers(); ++ i)
					{
						if (effect.CBufferByIndex(i)->NameHash() == shader_desc_[type].cb_desc[c].name_hash)
						{
							cbuff_indices_[type][c] = static_cast<uint8_t>(i);
							found = true;
							break;
						}
					}
					BOOST_ASSERT(found);
					KFL_UNUSED(found);
				}
			}
			else
			{
				is_shader_validate_[type] = false;
			}
		}

		void D3D11ShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids)
		{
			std::shared_ptr<std::vector<uint8_t>> code_blob = this->CompiteToBytecode(type, effect, tech, pass, shader_desc_ids);
			this->AttachShaderBytecode(type, effect, shader_desc_ids, code_blob);
		}

		void D3D11ShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/,
				RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & shared_so)
		{
			if (shared_so)
			{
				D3D11ShaderObject const & so = *checked_cast<D3D11ShaderObject*>(shared_so.get());

				is_shader_validate_[type] = so.is_shader_validate_[type];
				shader_code_[type] = so.shader_code_[type];
				shader_desc_[type] = so.shader_desc_[type];
				switch (type)
				{
				case ST_VertexShader:
					vs_signature_ = so.vs_signature_;
					break;

				case ST_PixelShader:
					break;

				case ST_GeometryShader:
					break;

				case ST_ComputeShader:
					cs_block_size_x_ = so.cs_block_size_x_;
					cs_block_size_y_ = so.cs_block_size_y_;
					cs_block_size_z_ = so.cs_block_size_z_;
					break;

				case ST_HullShader:
					break;

				case ST_DomainShader:
					break;

				default:
					is_shader_validate_[type] = false;
					break;
				}

				cbuff_indices_[type] = so.cbuff_indices_[type];
			}
		}

		void D3D11ShaderObject::LinkShaders(RenderEffect const & effect)
		{
			std::vector<uint32_t> all_cbuff_indices;
			is_validate_ = true;
			for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
			{
				is_validate_ &= is_shader_validate_[type];

				all_cbuff_indices.insert(all_cbuff_indices.end(),
					cbuff_indices_[type].begin(), cbuff_indices_[type].end());
				for (size_t i = 0; i < cbuff_indices_[type].size(); ++ i)
				{
					RenderEffectConstantBufferPtr const & cbuff = effect.CBufferByIndex(cbuff_indices_[type][i]);
					cbuff->Resize(shader_desc_[type].cb_desc[i].size);
					BOOST_ASSERT(cbuff->NumParameters() == shader_desc_[type].cb_desc[i].var_desc.size());
					for (uint32_t j = 0; j < cbuff->NumParameters(); ++ j)
					{
						RenderEffectParameterPtr const & param = effect.ParameterByIndex(cbuff->ParameterIndex(j));
						uint32_t stride;
						if (shader_desc_[type].cb_desc[i].var_desc[j].elements > 0)
						{
							if (param->Type() != REDT_float4x4)
							{
								stride = 16;
							}
							else
							{
								stride = 64;
							}
						}
						else
						{
							if (param->Type() != REDT_float4x4)
							{
								stride = 4;
							}
							else
							{
								stride = 16;
							}
						}
						param->BindToCBuffer(cbuff, shader_desc_[type].cb_desc[i].var_desc[j].start_offset, stride);
					}
				}
			}
		}
	}
}

#endif
