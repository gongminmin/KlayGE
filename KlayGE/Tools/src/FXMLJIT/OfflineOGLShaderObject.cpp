/**
* @file OfflineOGLShaderObject.cpp
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
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/ResIdentifier.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ResLoader.hpp>

#include <cstdio>
#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>

#include <glloader/glloader.h>

#include <DXBC2GLSL/DXBC2GLSL.hpp>

#ifndef D3DCOMPILE_SKIP_OPTIMIZATION
#define D3DCOMPILE_SKIP_OPTIMIZATION 0x00000004
#endif
#ifndef D3DCOMPILE_PREFER_FLOW_CONTROL
#define D3DCOMPILE_PREFER_FLOW_CONTROL 0x00000400
#endif

#include "OfflineRenderEffect.hpp"
#include "OfflineOGLShaderObject.hpp"

namespace KlayGE
{
	namespace Offline
	{
		OGLShaderObject::OGLShaderObject(OfflineRenderDeviceCaps const & caps)
			: ShaderObject(caps),
				gs_input_type_(0), gs_output_type_(0), gs_max_output_vertex_(0),
				ds_partitioning_(STP_Undefined), ds_output_primitive_(STOP_Undefined)
		{
			is_shader_validate_.fill(true);

			shader_func_names_ = MakeSharedPtr<std::array<std::string, ST_NumShaderTypes>>();
			glsl_srcs_ = MakeSharedPtr<std::array<std::shared_ptr<std::string>, ST_NumShaderTypes>>();

			pnames_ = MakeSharedPtr<std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes>>();
			glsl_res_names_ = MakeSharedPtr<std::array<std::shared_ptr<std::vector<std::string>>, ST_NumShaderTypes>>();

			vs_usages_ = MakeSharedPtr<std::vector<VertexElementUsage>>();
			vs_usage_indices_ = MakeSharedPtr<std::vector<uint8_t>>();
			glsl_vs_attrib_names_ = MakeSharedPtr<std::vector<std::string>>();
		}

		void OGLShaderObject::StreamOut(std::ostream& os, ShaderType type)
		{
			std::vector<uint8_t> native_shader_block;

			if ((*glsl_srcs_)[type])
			{
				std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

				uint32_t len32 = Native2LE(static_cast<uint32_t>((*glsl_srcs_)[type]->size()));
				oss.write(reinterpret_cast<char const *>(&len32), sizeof(len32));
				oss.write(&(*(*glsl_srcs_)[type])[0], (*glsl_srcs_)[type]->size());

				uint16_t num16 = Native2LE(static_cast<uint16_t>((*pnames_)[type]->size()));
				oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
				for (size_t i = 0; i < (*pnames_)[type]->size(); ++ i)
				{
					uint8_t len8 = static_cast<uint8_t>((*(*pnames_)[type])[i].size());
					oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
					oss.write(&(*(*pnames_)[type])[i][0], (*(*pnames_)[type])[i].size());
				}

				num16 = Native2LE(static_cast<uint16_t>((*glsl_res_names_)[type]->size()));
				oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
				for (size_t i = 0; i < (*glsl_res_names_)[type]->size(); ++ i)
				{
					uint8_t len8 = static_cast<uint8_t>((*(*glsl_res_names_)[type])[i].size());
					oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
					oss.write(&(*(*glsl_res_names_)[type])[i][0], (*(*glsl_res_names_)[type])[i].size());
				}

				std::vector<std::pair<std::string, std::string>> tex_sampler_pairs;
				for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
				{
					if (std::get<3>(tex_sampler_binds_[i]) | (1UL << type))
					{
						tex_sampler_pairs.emplace_back(*std::get<1>(tex_sampler_binds_[i])->Name(),
							*std::get<2>(tex_sampler_binds_[i])->Name());
					}
				}

				num16 = Native2LE(static_cast<uint16_t>(tex_sampler_pairs.size()));
				oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
				for (size_t i = 0; i < num16; ++ i)
				{
					uint8_t len8 = static_cast<uint8_t>(tex_sampler_pairs[i].first.size());
					oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
					oss.write(&tex_sampler_pairs[i].first[0], len8);

					len8 = static_cast<uint8_t>(tex_sampler_pairs[i].second.size());
					oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
					oss.write(&tex_sampler_pairs[i].second[0], len8);
				}

				if (ST_VertexShader == type)
				{
					uint8_t num8 = static_cast<uint8_t>(vs_usages_->size());
					oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
					for (size_t i = 0; i < vs_usages_->size(); ++ i)
					{
						uint8_t veu = static_cast<uint8_t>((*vs_usages_)[i]);
						oss.write(reinterpret_cast<char const *>(&veu), sizeof(veu));
					}

					num8 = static_cast<uint8_t>(vs_usage_indices_->size());
					oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
					if (!vs_usage_indices_->empty())
					{
						oss.write(reinterpret_cast<char const *>(&(*vs_usage_indices_)[0]), vs_usage_indices_->size() * sizeof((*vs_usage_indices_)[0]));
					}

					num8 = static_cast<uint8_t>(glsl_vs_attrib_names_->size());
					oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
					for (size_t i = 0; i < glsl_vs_attrib_names_->size(); ++ i)
					{
						uint8_t len8 = static_cast<uint8_t>((*glsl_vs_attrib_names_)[i].size());
						oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
						oss.write(&(*glsl_vs_attrib_names_)[i][0], (*glsl_vs_attrib_names_)[i].size());
					}
				}
				else if (ST_GeometryShader == type)
				{
					uint32_t git = Native2LE(gs_input_type_);
					oss.write(reinterpret_cast<char const *>(&git), sizeof(git));

					uint32_t got = Native2LE(gs_output_type_);
					oss.write(reinterpret_cast<char const *>(&got), sizeof(got));

					uint32_t gmov = Native2LE(gs_max_output_vertex_);
					oss.write(reinterpret_cast<char const *>(&gmov), sizeof(gmov));
				}

				std::string out_str = oss.str();
				native_shader_block.resize(out_str.size());
				std::memcpy(&native_shader_block[0], &out_str[0], out_str.size());
			}

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

		void OGLShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids)
		{
			ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

			(*shader_func_names_)[type] = sd.func_name;

			bool has_gs = false;
			if (!effect.GetShaderDesc(shader_desc_ids[ST_GeometryShader]).func_name.empty())
			{
				has_gs = true;
			}

			is_shader_validate_[type] = true;
			switch (type)
			{
			case ST_VertexShader:
			case ST_PixelShader:
			case ST_GeometryShader:
			case ST_HullShader:
			case ST_DomainShader:
				break;

			default:
				is_shader_validate_[type] = false;
				break;
			}

			if (is_shader_validate_[type])
			{
				OfflineRenderDeviceCaps const & caps = caps_;

				is_shader_validate_[type] = true;

				char const * shader_profile = sd.profile.c_str();
				size_t const shader_profile_hash = RT_HASH(shader_profile);
				switch (type)
				{
				case ST_VertexShader:
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "vs_5_0";
					}
					break;

				case ST_PixelShader:
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "ps_5_0";
					}
					break;

				case ST_GeometryShader:
					if (caps.gs_support)
					{
						if (CT_HASH("auto") == shader_profile_hash)
						{
							shader_profile = "gs_5_0";
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
							shader_profile = "cs_5_0";
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
							shader_profile = "hs_5_0";
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
							shader_profile = "ds_5_0";
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

				std::vector<uint8_t> code;
				if (is_shader_validate_[type])
				{
					std::string err_msg;
					std::vector<std::pair<char const *, char const *>> macros;
					macros.emplace_back("KLAYGE_DXBC2GLSL", "1");
					macros.emplace_back("KLAYGE_OPENGL", "1");
					if (!caps.bc5_support)
					{
						macros.emplace_back("KLAYGE_BC5_AS_AG", "1");
					}
					if (!caps.bc4_support)
					{
						macros.emplace_back("KLAYGE_BC4_AS_G", "1");
					}
					macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");

					uint32_t const flags = D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
					code = this->CompileToDXBC(type, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile, flags);
					if (code.empty())
					{
						is_shader_validate_[type] = false;
					}
					else
					{
						try
						{
							GLSLVersion gsv;
							switch (caps.major_version)
							{
							case 4:
								switch (caps.minor_version)
								{
								case 5:
									gsv = GSV_450;
									break;

								case 4:
									gsv = GSV_440;
									break;

								case 3:
									gsv = GSV_430;
									break;

								case 2:
									gsv = GSV_420;
									break;

								case 1:
									gsv = GSV_410;
									break;

								default:
								case 0:
									gsv = GSV_400;
									break;
								}
								break;

							case 3:
								switch (caps.minor_version)
								{
								case 3:
									gsv = GSV_330;
									break;

								case 2:
									gsv = GSV_150;
									break;

								case 1:
									gsv = GSV_140;
									break;

								default:
								case 0:
									gsv = GSV_130;
									break;
								}
								break;

							case 2:
							default:
								switch (caps.minor_version)
								{
								case 1:
									gsv = GSV_120;
									break;

								default:
								case 0:
									gsv = GSV_110;
									break;
								}
								break;
							}

							DXBC2GLSL::DXBC2GLSL dxbc2glsl;
							uint32_t rules = DXBC2GLSL::DXBC2GLSL::DefaultRules(gsv);
							rules &= ~GSR_UniformBlockBinding;
							dxbc2glsl.FeedDXBC(&code[0],
								has_gs, static_cast<ShaderTessellatorPartitioning>(ds_partitioning_),
								static_cast<ShaderTessellatorOutputPrimitive>(ds_output_primitive_),
								gsv, rules);
							(*glsl_srcs_)[type] = MakeSharedPtr<std::string>(dxbc2glsl.GLSLString());
							(*pnames_)[type] = MakeSharedPtr<std::vector<std::string>>();
							(*glsl_res_names_)[type] = MakeSharedPtr<std::vector<std::string>>();

							for (uint32_t i = 0; i < dxbc2glsl.NumCBuffers(); ++ i)
							{
								for (uint32_t j = 0; j < dxbc2glsl.NumVariables(i); ++ j)
								{
									if (dxbc2glsl.VariableUsed(i, j))
									{
										(*pnames_)[type]->push_back(dxbc2glsl.VariableName(i, j));
										(*glsl_res_names_)[type]->push_back(dxbc2glsl.VariableName(i, j));
									}
								}
							}

							std::vector<char const *> tex_names;
							std::vector<char const *> sampler_names;
							for (uint32_t i = 0; i < dxbc2glsl.NumResources(); ++ i)
							{
								if (dxbc2glsl.ResourceUsed(i))
								{
									if (SIT_TEXTURE == dxbc2glsl.ResourceType(i))
									{
										tex_names.push_back(dxbc2glsl.ResourceName(i));
									}
									else if (SIT_SAMPLER == dxbc2glsl.ResourceType(i))
									{
										sampler_names.push_back(dxbc2glsl.ResourceName(i));
									}
								}
							}

							for (size_t i = 0; i < tex_names.size(); ++ i)
							{
								RenderEffectParameterPtr const & param = effect.ParameterByName(tex_names[i]);
								for (size_t j = 0; j < sampler_names.size(); ++ j)
								{
									std::string combined_sampler_name = std::string(tex_names[i]) + "_" + sampler_names[j];
									bool found = false;
									for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
									{
										if (std::get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
										{
											std::get<3>(tex_sampler_binds_[k]) |= 1UL << type;
											found = true;
											break;
										}
									}
									if (!found)
									{
										tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
											param, effect.ParameterByName(sampler_names[j]), 1UL << type));
									}

									(*pnames_)[type]->push_back(combined_sampler_name);
									(*glsl_res_names_)[type]->push_back(combined_sampler_name);
								}
							}

							if (ST_VertexShader == type)
							{
								for (uint32_t i = 0; i < dxbc2glsl.NumInputParams(); ++ i)
								{
									if (dxbc2glsl.InputParam(i).mask != 0)
									{
										std::string semantic = dxbc2glsl.InputParam(i).semantic_name;
										uint32_t semantic_index = dxbc2glsl.InputParam(i).semantic_index;
										std::string glsl_param_name = semantic;
										size_t const semantic_hash = RT_HASH(semantic.c_str());

										if ((CT_HASH("SV_VertexID") != semantic_hash)
											&& (CT_HASH("SV_InstanceID") != semantic_hash))
										{
											VertexElementUsage usage = VEU_Position;
											uint8_t usage_index = 0;
											if (CT_HASH("POSITION") == semantic_hash)
											{
												usage = VEU_Position;
												glsl_param_name = "POSITION0";
											}
											else if (CT_HASH("NORMAL") == semantic_hash)
											{
												usage = VEU_Normal;
												glsl_param_name = "NORMAL0";
											}
											else if (CT_HASH("COLOR") == semantic_hash)
											{
												if (0 == semantic_index)
												{
													usage = VEU_Diffuse;
													glsl_param_name = "COLOR0";
												}
												else
												{
													usage = VEU_Specular;
													glsl_param_name = "COLOR1";
												}
											}
											else if (CT_HASH("BLENDWEIGHT") == semantic_hash)
											{
												usage = VEU_BlendWeight;
												glsl_param_name = "BLENDWEIGHT0";
											}
											else if (CT_HASH("BLENDINDICES") == semantic_hash)
											{
												usage = VEU_BlendIndex;
												glsl_param_name = "BLENDINDICES0";
											}
											else if (0 == semantic.find("TEXCOORD"))
											{
												usage = VEU_TextureCoord;
												usage_index = static_cast<uint8_t>(semantic_index);
												glsl_param_name = "TEXCOORD" + boost::lexical_cast<std::string>(semantic_index);
											}
											else if (CT_HASH("TANGENT") == semantic_hash)
											{
												usage = VEU_Tangent;
												glsl_param_name = "TANGENT0";
											}
											else if (CT_HASH("BINORMAL") == semantic_hash)
											{
												usage = VEU_Binormal;
												glsl_param_name = "BINORMAL0";
											}
											else
											{
												BOOST_ASSERT(false);
												usage = VEU_Position;
												glsl_param_name = "POSITION0";
											}

											vs_usages_->push_back(usage);
											vs_usage_indices_->push_back(usage_index);
											glsl_vs_attrib_names_->push_back(glsl_param_name);
										}
									}
								}
							}
							else if (ST_GeometryShader == type)
							{
								switch (dxbc2glsl.GSInputPrimitive())
								{
								case SP_Point:
									gs_input_type_ = GL_POINTS;
									break;

								case SP_Line:
									gs_input_type_ = GL_LINES;
									break;

								case SP_LineAdj:
									gs_input_type_ = GL_LINES_ADJACENCY_EXT;
									break;

								case SP_Triangle:
									gs_input_type_ = GL_TRIANGLES;
									break;

								case SP_TriangleAdj:
									gs_input_type_ = GL_TRIANGLES_ADJACENCY_EXT;
									break;

								default:
									BOOST_ASSERT(false);
									gs_input_type_ = 0;
									break;
								}

								switch (dxbc2glsl.GSOutputTopology(0))
								{
								case SPT_PointList:
									gs_output_type_ = GL_POINTS;
									break;

								case SPT_LineStrip:
									gs_output_type_ = GL_LINE_STRIP;
									break;

								case SPT_TriangleStrip:
									gs_output_type_ = GL_TRIANGLE_STRIP;
									break;

								default:
									BOOST_ASSERT(false);
									gs_output_type_ = 0;
									break;
								}

								gs_max_output_vertex_ = dxbc2glsl.MaxGSOutputVertex();
							}
							else if (ST_HullShader == type)
							{
								ds_partitioning_ = dxbc2glsl.DSPartitioning();
								ds_output_primitive_ = dxbc2glsl.DSOutputPrimitive();
							}
						}
						catch (std::exception& ex)
						{
							is_shader_validate_[type] = false;

							LogError("Error(s) in conversion: %s/%s/%s", tech.Name().c_str(), pass.Name().c_str(), sd.func_name.c_str());
							LogError(ex.what());
							LogError("Please send this information and your shader to webmaster at klayge.org. We'll fix this ASAP.");
						}
					}
				}
			}
		}

		void OGLShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/,
				RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & shared_so)
		{
			OGLShaderObjectPtr so = checked_pointer_cast<OGLShaderObject>(shared_so);

			is_shader_validate_[type] = so->is_shader_validate_[type];
			(*shader_func_names_)[type] = (*so->shader_func_names_)[type];

			if (is_shader_validate_[type])
			{
				(*glsl_srcs_)[type] = (*so->glsl_srcs_)[type];

				(*pnames_)[type] = (*so->pnames_)[type];
				(*glsl_res_names_)[type] = (*so->glsl_res_names_)[type];
				if (ST_VertexShader == type)
				{
					*vs_usages_ = *so->vs_usages_;
					*vs_usage_indices_ = *so->vs_usage_indices_;
					*glsl_vs_attrib_names_ = *so->glsl_vs_attrib_names_;
				}
				else if (ST_GeometryShader == type)
				{
					gs_input_type_ = so->gs_input_type_;
					gs_output_type_ = so->gs_output_type_;
					gs_max_output_vertex_ = so->gs_max_output_vertex_;
				}
				else if (ST_HullShader == type)
				{
					ds_partitioning_ = so->ds_partitioning_;
					ds_output_primitive_ = so->ds_output_primitive_;
				}

				for (uint32_t j = 0; j < so->tex_sampler_binds_.size(); ++ j)
				{
					if (std::get<3>(so->tex_sampler_binds_[j]) | (1UL << type))
					{
						std::string const & combined_sampler_name = std::get<0>(so->tex_sampler_binds_[j]);
						bool found = false;
						for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
						{
							if (std::get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
							{
								std::get<3>(tex_sampler_binds_[k]) |= 1UL << type;
								found = true;
								break;
							}
						}
						if (!found)
						{
							tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
								std::get<1>(so->tex_sampler_binds_[j]), std::get<2>(so->tex_sampler_binds_[j]), 1UL << type));
						}
					}
				}
			}
		}

		void OGLShaderObject::LinkShaders(RenderEffect const & effect)
		{
			KFL_UNUSED(effect);

			is_validate_ = true;
			for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
			{
				if (!(*shader_func_names_)[type].empty())
				{
					is_validate_ &= is_shader_validate_[type];
				}
			}
		}
	}
}
