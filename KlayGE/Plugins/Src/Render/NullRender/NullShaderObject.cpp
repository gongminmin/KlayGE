/**
 * @file NullShaderObject.cpp
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

#include <KFL/CXX2a/format.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KFL/com_ptr.hpp>
#include <KFL/CustomizedStreamBuf.hpp>
#include <KFL/Hash.hpp>
#include <KFL/ResIdentifier.hpp>

#include <string>

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <KlayGE/SALWrapper.hpp>
#include <d3dcompiler.h>
#endif

#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
#include <glloader/glloader.h>
#include <DXBC2GLSL/DXBC2GLSL.hpp>
#endif

#ifndef D3DCOMPILE_SKIP_OPTIMIZATION
#define D3DCOMPILE_SKIP_OPTIMIZATION 0x00000004
#endif
#ifndef D3DCOMPILE_PREFER_FLOW_CONTROL
#define D3DCOMPILE_PREFER_FLOW_CONTROL 0x00000400
#endif
#ifndef D3DCOMPILE_ENABLE_STRICTNESS
#define D3DCOMPILE_ENABLE_STRICTNESS 0x00000800
#endif

#include <KlayGE/NullRender/NullRenderEngine.hpp>
#include <KlayGE/NullRender/NullShaderObject.hpp>

namespace KlayGE
{
	D3DShaderStageObject::D3DShaderStageObject(ShaderStage stage, bool as_d3d12)
		: ShaderStageObject(stage), as_d3d12_(as_d3d12)
	{
	}

	void D3DShaderStageObject::StreamIn(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res)
	{
		uint32_t native_shader_block_len;
		res.read(&native_shader_block_len, sizeof(native_shader_block_len));
		native_shader_block_len = LE2Native(native_shader_block_len);

		is_validate_ = false;
		std::string_view const shader_profile = this->GetShaderProfile(effect, shader_desc_ids[static_cast<uint32_t>(stage_)]);
		if (native_shader_block_len >= 25 + shader_profile.size())
		{
			uint8_t len;
			res.read(reinterpret_cast<char*>(&len), sizeof(len));
			std::string& profile = shader_profile_;
			profile.resize(len);
			res.read(&profile[0], len);
			if (profile == shader_profile)
			{
				is_validate_ = true;

				uint32_t blob_size;
				res.read(reinterpret_cast<char*>(&blob_size), sizeof(blob_size));
				shader_code_.resize(blob_size);

				res.read(reinterpret_cast<char*>(shader_code_.data()), blob_size);

				uint16_t cb_desc_size;
				res.read(reinterpret_cast<char*>(&cb_desc_size), sizeof(cb_desc_size));
				cb_desc_size = LE2Native(cb_desc_size);
				shader_desc_.cb_desc.resize(cb_desc_size);
				for (size_t i = 0; i < shader_desc_.cb_desc.size(); ++i)
				{
					res.read(reinterpret_cast<char*>(&len), sizeof(len));
					shader_desc_.cb_desc[i].name.resize(len);
					res.read(&shader_desc_.cb_desc[i].name[0], len);

					shader_desc_.cb_desc[i].name_hash = RT_HASH(shader_desc_.cb_desc[i].name.c_str());

					res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].size), sizeof(shader_desc_.cb_desc[i].size));
					shader_desc_.cb_desc[i].size = LE2Native(shader_desc_.cb_desc[i].size);

					uint16_t var_desc_size;
					res.read(reinterpret_cast<char*>(&var_desc_size), sizeof(var_desc_size));
					var_desc_size = LE2Native(var_desc_size);
					shader_desc_.cb_desc[i].var_desc.resize(var_desc_size);
					for (size_t j = 0; j < shader_desc_.cb_desc[i].var_desc.size(); ++j)
					{
						res.read(reinterpret_cast<char*>(&len), sizeof(len));
						shader_desc_.cb_desc[i].var_desc[j].name.resize(len);
						res.read(&shader_desc_.cb_desc[i].var_desc[j].name[0], len);

						res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].start_offset),
							sizeof(shader_desc_.cb_desc[i].var_desc[j].start_offset));
						shader_desc_.cb_desc[i].var_desc[j].start_offset = LE2Native(shader_desc_.cb_desc[i].var_desc[j].start_offset);
						res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].type),
							sizeof(shader_desc_.cb_desc[i].var_desc[j].type));
						res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].rows),
							sizeof(shader_desc_.cb_desc[i].var_desc[j].rows));
						res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].columns),
							sizeof(shader_desc_.cb_desc[i].var_desc[j].columns));
						res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].elements),
							sizeof(shader_desc_.cb_desc[i].var_desc[j].elements));
						shader_desc_.cb_desc[i].var_desc[j].elements = LE2Native(shader_desc_.cb_desc[i].var_desc[j].elements);
					}
				}

				res.read(reinterpret_cast<char*>(&shader_desc_.num_samplers), sizeof(shader_desc_.num_samplers));
				shader_desc_.num_samplers = LE2Native(shader_desc_.num_samplers);
				res.read(reinterpret_cast<char*>(&shader_desc_.num_srvs), sizeof(shader_desc_.num_srvs));
				shader_desc_.num_srvs = LE2Native(shader_desc_.num_srvs);
				res.read(reinterpret_cast<char*>(&shader_desc_.num_uavs), sizeof(shader_desc_.num_uavs));
				shader_desc_.num_uavs = LE2Native(shader_desc_.num_uavs);

				uint16_t res_desc_size;
				res.read(reinterpret_cast<char*>(&res_desc_size), sizeof(res_desc_size));
				res_desc_size = LE2Native(res_desc_size);
				shader_desc_.res_desc.resize(res_desc_size);
				for (size_t i = 0; i < shader_desc_.res_desc.size(); ++i)
				{
					res.read(reinterpret_cast<char*>(&len), sizeof(len));
					shader_desc_.res_desc[i].name.resize(len);
					res.read(&shader_desc_.res_desc[i].name[0], len);

					res.read(reinterpret_cast<char*>(&shader_desc_.res_desc[i].type), sizeof(shader_desc_.res_desc[i].type));

					res.read(reinterpret_cast<char*>(&shader_desc_.res_desc[i].dimension), sizeof(shader_desc_.res_desc[i].dimension));

					res.read(reinterpret_cast<char*>(&shader_desc_.res_desc[i].bind_point), sizeof(shader_desc_.res_desc[i].bind_point));
					shader_desc_.res_desc[i].bind_point = LE2Native(shader_desc_.res_desc[i].bind_point);
				}

				this->FillCBufferIndices(effect);

				this->StageSpecificStreamIn(res);
			}
		}
	}

	void D3DShaderStageObject::StreamOut(std::ostream& os)
	{
		std::vector<char> native_shader_block;
		VectorOutputStreamBuf native_shader_buff(native_shader_block);
		std::ostream oss(&native_shader_buff);

		{
			uint8_t len = static_cast<uint8_t>(shader_profile_.size());
			oss.write(reinterpret_cast<char const*>(&len), sizeof(len));
			oss.write(reinterpret_cast<char const*>(shader_profile_.data()), len);
		}

		if (!shader_code_.empty())
		{
			uint8_t len;

			uint32_t blob_size = Native2LE(static_cast<uint32_t>(shader_code_.size()));
			oss.write(reinterpret_cast<char const*>(&blob_size), sizeof(blob_size));
			oss.write(reinterpret_cast<char const*>(shader_code_.data()), shader_code_.size());

			uint16_t cb_desc_size = Native2LE(static_cast<uint16_t>(shader_desc_.cb_desc.size()));
			oss.write(reinterpret_cast<char const*>(&cb_desc_size), sizeof(cb_desc_size));
			for (size_t i = 0; i < shader_desc_.cb_desc.size(); ++i)
			{
				len = static_cast<uint8_t>(shader_desc_.cb_desc[i].name.size());
				oss.write(reinterpret_cast<char const*>(&len), sizeof(len));
				oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].name[0]), len);

				uint32_t size = Native2LE(shader_desc_.cb_desc[i].size);
				oss.write(reinterpret_cast<char const*>(&size), sizeof(size));

				uint16_t var_desc_size = Native2LE(static_cast<uint16_t>(shader_desc_.cb_desc[i].var_desc.size()));
				oss.write(reinterpret_cast<char const*>(&var_desc_size), sizeof(var_desc_size));
				for (size_t j = 0; j < shader_desc_.cb_desc[i].var_desc.size(); ++j)
				{
					len = static_cast<uint8_t>(shader_desc_.cb_desc[i].var_desc[j].name.size());
					oss.write(reinterpret_cast<char const*>(&len), sizeof(len));
					oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].var_desc[j].name[0]), len);

					uint32_t start_offset = Native2LE(shader_desc_.cb_desc[i].var_desc[j].start_offset);
					oss.write(reinterpret_cast<char const*>(&start_offset), sizeof(start_offset));
					oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].var_desc[j].type),
						sizeof(shader_desc_.cb_desc[i].var_desc[j].type));
					oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].var_desc[j].rows),
						sizeof(shader_desc_.cb_desc[i].var_desc[j].rows));
					oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].var_desc[j].columns),
						sizeof(shader_desc_.cb_desc[i].var_desc[j].columns));
					uint16_t elements = Native2LE(shader_desc_.cb_desc[i].var_desc[j].elements);
					oss.write(reinterpret_cast<char const*>(&elements), sizeof(elements));
				}
			}

			uint16_t num_samplers = Native2LE(shader_desc_.num_samplers);
			oss.write(reinterpret_cast<char const*>(&num_samplers), sizeof(num_samplers));
			uint16_t num_srvs = Native2LE(shader_desc_.num_srvs);
			oss.write(reinterpret_cast<char const*>(&num_srvs), sizeof(num_srvs));
			uint16_t num_uavs = Native2LE(shader_desc_.num_uavs);
			oss.write(reinterpret_cast<char const*>(&num_uavs), sizeof(num_uavs));

			uint16_t res_desc_size = Native2LE(static_cast<uint16_t>(shader_desc_.res_desc.size()));
			oss.write(reinterpret_cast<char const*>(&res_desc_size), sizeof(res_desc_size));
			for (size_t i = 0; i < shader_desc_.res_desc.size(); ++i)
			{
				len = static_cast<uint8_t>(shader_desc_.res_desc[i].name.size());
				oss.write(reinterpret_cast<char const*>(&len), sizeof(len));
				oss.write(reinterpret_cast<char const*>(&shader_desc_.res_desc[i].name[0]), len);

				oss.write(reinterpret_cast<char const*>(&shader_desc_.res_desc[i].type), sizeof(shader_desc_.res_desc[i].type));
				oss.write(reinterpret_cast<char const*>(&shader_desc_.res_desc[i].dimension), sizeof(shader_desc_.res_desc[i].dimension));
				uint16_t bind_point = Native2LE(shader_desc_.res_desc[i].bind_point);
				oss.write(reinterpret_cast<char const*>(&bind_point), sizeof(bind_point));
			}

			this->StageSpecificStreamOut(oss);
		}

		uint32_t len = static_cast<uint32_t>(native_shader_block.size());
		{
			uint32_t tmp = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
		if (len > 0)
		{
			os.write(reinterpret_cast<char const*>(&native_shader_block[0]), len * sizeof(native_shader_block[0]));
		}
	}

	void D3DShaderStageObject::CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
		std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		shader_code_.clear();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		uint32_t const shader_desc_id = shader_desc_ids[static_cast<uint32_t>(stage_)];

		auto const& sd = effect.GetShaderDesc(shader_desc_id);

		shader_profile_ = std::string(this->GetShaderProfile(effect, shader_desc_id));
		is_validate_ = !shader_profile_.empty();

		if (is_validate_)
		{
			std::vector<std::pair<char const*, char const*>> macros;
			if (as_d3d12_)
			{
				macros.emplace_back("KLAYGE_D3D12", "1");
			}
			else
			{
				macros.emplace_back("KLAYGE_D3D11", "1");
			}
			macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");

			uint32_t flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if !defined(KLAYGE_DEBUG)
			flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			shader_code_ =
				ShaderStageObject::CompileToDXBC(stage_, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile_.c_str(), flags);

			if (!shader_code_.empty())
			{
				com_ptr<ID3D11ShaderReflection> reflection;
				ShaderStageObject::ReflectDXBC(shader_code_, reflection.put_void());
				if (reflection != nullptr)
				{
					D3D11_SHADER_DESC desc;
					reflection->GetDesc(&desc);

					for (UINT c = 0; c < desc.ConstantBuffers; ++c)
					{
						ID3D11ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

						D3D11_SHADER_BUFFER_DESC d3d_cb_desc;
						reflection_cb->GetDesc(&d3d_cb_desc);
						if ((D3D_CT_CBUFFER == d3d_cb_desc.Type) || (D3D_CT_TBUFFER == d3d_cb_desc.Type))
						{
							D3DShaderDesc::ConstantBufferDesc cb_desc;
							cb_desc.name = d3d_cb_desc.Name;
							cb_desc.name_hash = RT_HASH(d3d_cb_desc.Name);
							cb_desc.size = d3d_cb_desc.Size;

							for (UINT v = 0; v < d3d_cb_desc.Variables; ++v)
							{
								ID3D11ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

								D3D11_SHADER_VARIABLE_DESC var_desc;
								reflection_var->GetDesc(&var_desc);

								D3D11_SHADER_TYPE_DESC type_desc;
								reflection_var->GetType()->GetDesc(&type_desc);

								D3DShaderDesc::ConstantBufferDesc::VariableDesc vd;
								vd.name = var_desc.Name;
								vd.start_offset = var_desc.StartOffset;
								vd.type = static_cast<uint8_t>(type_desc.Type);
								vd.rows = static_cast<uint8_t>(type_desc.Rows);
								vd.columns = static_cast<uint8_t>(type_desc.Columns);
								vd.elements = static_cast<uint16_t>(type_desc.Elements);
								cb_desc.var_desc.push_back(vd);
							}

							shader_desc_.cb_desc.push_back(cb_desc);
						}
					}

					this->FillCBufferIndices(effect);

					int max_sampler_bind_pt = -1;
					int max_srv_bind_pt = -1;
					int max_uav_bind_pt = -1;
					for (uint32_t i = 0; i < desc.BoundResources; ++i)
					{
						D3D11_SHADER_INPUT_BIND_DESC si_desc;
						reflection->GetResourceBindingDesc(i, &si_desc);

						switch (si_desc.Type)
						{
						case D3D_SIT_SAMPLER:
							max_sampler_bind_pt = std::max(max_sampler_bind_pt, static_cast<int>(si_desc.BindPoint));
							break;

						case D3D_SIT_TEXTURE:
						case D3D_SIT_STRUCTURED:
						case D3D_SIT_BYTEADDRESS:
							max_srv_bind_pt = std::max(max_srv_bind_pt, static_cast<int>(si_desc.BindPoint));
							break;

						case D3D_SIT_UAV_RWTYPED:
						case D3D_SIT_UAV_RWSTRUCTURED:
						case D3D_SIT_UAV_RWBYTEADDRESS:
						case D3D_SIT_UAV_APPEND_STRUCTURED:
						case D3D_SIT_UAV_CONSUME_STRUCTURED:
						case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
							max_uav_bind_pt = std::max(max_uav_bind_pt, static_cast<int>(si_desc.BindPoint));
							break;

						default:
							break;
						}
					}

					shader_desc_.num_samplers = static_cast<uint16_t>(max_sampler_bind_pt + 1);
					shader_desc_.num_srvs = static_cast<uint16_t>(max_srv_bind_pt + 1);
					shader_desc_.num_uavs = static_cast<uint16_t>(max_uav_bind_pt + 1);

					for (uint32_t i = 0; i < desc.BoundResources; ++i)
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
								D3DShaderDesc::BoundResourceDesc brd;
								brd.name = si_desc.Name;
								brd.type = static_cast<uint8_t>(si_desc.Type);
								brd.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
								shader_desc_.res_desc.push_back(brd);
							}
							break;

						default:
							break;
						}
					}

					this->StageSpecificReflection(reflection.get());
				}

				shader_code_ =
					ShaderStageObject::StripDXBC(shader_code_, D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO |
																   D3DCOMPILER_STRIP_TEST_BLOBS | D3DCOMPILER_STRIP_PRIVATE_DATA);
			}
		}

		if (shader_code_.empty())
		{
			shader_profile_.clear();
		}
#else
		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(pass);
		KFL_UNUSED(shader_desc_ids);
#endif
	}

	void D3DShaderStageObject::CreateHwShader(RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(shader_desc_ids);

		hw_res_ready_ = true;
	}

	void D3DShaderStageObject::FillCBufferIndices(RenderEffect const& effect)
	{
		if (!shader_desc_.cb_desc.empty())
		{
			cbuff_indices_.resize(shader_desc_.cb_desc.size());
		}
		for (size_t c = 0; c < shader_desc_.cb_desc.size(); ++c)
		{
			uint32_t i = 0;
			for (; i < effect.NumCBuffers(); ++i)
			{
				if (effect.CBufferByIndex(i)->NameHash() == shader_desc_.cb_desc[c].name_hash)
				{
					cbuff_indices_[c] = static_cast<uint8_t>(i);
					break;
				}
			}
			BOOST_ASSERT(i < effect.NumCBuffers());
		}
	}

	std::string_view D3DShaderStageObject::GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const
	{
		std::string_view shader_profile = effect.GetShaderDesc(shader_desc_id).profile;
		if (is_available_)
		{
			if (shader_profile == "auto")
			{
				auto const& re = checked_cast<NullRenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				shader_profile = re.DefaultShaderProfile(stage_);
			}
		}
		else
		{
			shader_profile = std::string_view();
		}
		return shader_profile;
	}


	D3D11VertexShaderStageObject::D3D11VertexShaderStageObject() : D3DShaderStageObject(ShaderStage::Vertex, false)
	{
		is_available_ = true;
	}

	void D3D11VertexShaderStageObject::StageSpecificStreamIn(ResIdentifier& res)
	{
		res.read(reinterpret_cast<char*>(&vs_signature_), sizeof(vs_signature_));
		vs_signature_ = LE2Native(vs_signature_);
	}

	void D3D11VertexShaderStageObject::StageSpecificStreamOut(std::ostream& os)
	{
		uint32_t const vs_signature = Native2LE(vs_signature_);
		os.write(reinterpret_cast<char const*>(&vs_signature), sizeof(vs_signature));
	}

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
	void D3D11VertexShaderStageObject::StageSpecificReflection(ID3D11ShaderReflection* reflection)
	{
		D3D11_SHADER_DESC desc;
		reflection->GetDesc(&desc);

		vs_signature_ = 0;
		D3D11_SIGNATURE_PARAMETER_DESC signature;
		for (uint32_t i = 0; i < desc.InputParameters; ++i)
		{
			reflection->GetInputParameterDesc(i, &signature);

			size_t seed = RT_HASH(signature.SemanticName);
			HashCombine(seed, signature.SemanticIndex);
			HashCombine(seed, signature.Register);
			HashCombine(seed, static_cast<uint32_t>(signature.SystemValueType));
			HashCombine(seed, static_cast<uint32_t>(signature.ComponentType));
			HashCombine(seed, signature.Mask);
			HashCombine(seed, signature.ReadWriteMask);
			HashCombine(seed, signature.Stream);
			HashCombine(seed, signature.MinPrecision);

			size_t sig = vs_signature_;
			HashCombine(sig, seed);
			vs_signature_ = static_cast<uint32_t>(sig);
		}
	}
#endif


	D3D12VertexShaderStageObject::D3D12VertexShaderStageObject() : D3DShaderStageObject(ShaderStage::Vertex, true)
	{
		is_available_ = true;
	}


	D3DPixelShaderStageObject::D3DPixelShaderStageObject(bool as_d3d12) : D3DShaderStageObject(ShaderStage::Pixel, as_d3d12)
	{
		is_available_ = true;
	}


	D3DGeometryShaderStageObject::D3DGeometryShaderStageObject(bool as_d3d12) : D3DShaderStageObject(ShaderStage::Geometry, as_d3d12)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.gs_support;
	}


	D3DComputeShaderStageObject::D3DComputeShaderStageObject(bool as_d3d12) : D3DShaderStageObject(ShaderStage::Compute, as_d3d12)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.cs_support;
	}

	void D3DComputeShaderStageObject::StageSpecificStreamIn(ResIdentifier& res)
	{
		res.read(reinterpret_cast<char*>(&block_size_x_), sizeof(block_size_x_));
		block_size_x_ = LE2Native(block_size_x_);

		res.read(reinterpret_cast<char*>(&block_size_y_), sizeof(block_size_y_));
		block_size_y_ = LE2Native(block_size_y_);

		res.read(reinterpret_cast<char*>(&block_size_z_), sizeof(block_size_z_));
		block_size_z_ = LE2Native(block_size_z_);
	}

	void D3DComputeShaderStageObject::StageSpecificStreamOut(std::ostream& os)
	{
		uint32_t block_size_x = Native2LE(block_size_x_);
		os.write(reinterpret_cast<char const*>(&block_size_x), sizeof(block_size_x));

		uint32_t block_size_y = Native2LE(block_size_y_);
		os.write(reinterpret_cast<char const*>(&block_size_y), sizeof(block_size_y));

		uint32_t block_size_z = Native2LE(block_size_z_);
		os.write(reinterpret_cast<char const*>(&block_size_z), sizeof(block_size_z));
	}

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
	void D3DComputeShaderStageObject::StageSpecificReflection(ID3D11ShaderReflection* reflection)
	{
		reflection->GetThreadGroupSize(&block_size_x_, &block_size_y_, &block_size_z_);
	}
#endif


	D3DHullShaderStageObject::D3DHullShaderStageObject(bool as_d3d12) : D3DShaderStageObject(ShaderStage::Hull, as_d3d12)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.hs_support;
	}


	D3DDomainShaderStageObject::D3DDomainShaderStageObject(bool as_d3d12) : D3DShaderStageObject(ShaderStage::Domain, as_d3d12)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.ds_support;
	}


	OGLShaderStageObject::OGLShaderStageObject(ShaderStage stage, GLenum gl_shader_type, bool as_gles)
		: ShaderStageObject(stage), gl_shader_type_(gl_shader_type), as_gles_(as_gles)
	{
	}

	void OGLShaderStageObject::StreamIn(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res)
	{
		uint32_t native_shader_block_len;
		res.read(&native_shader_block_len, sizeof(native_shader_block_len));
		native_shader_block_len = LE2Native(native_shader_block_len);

		auto const& sd = effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(stage_)]);

		shader_func_name_ = sd.func_name;

		is_validate_ = false;
		if (native_shader_block_len >= 24)
		{
			is_validate_ = true;

			uint32_t len32;
			res.read(reinterpret_cast<char*>(&len32), sizeof(len32));
			len32 = LE2Native(len32);
			glsl_src_.resize(len32, '\0');
			res.read(&glsl_src_[0], len32);

			uint16_t num16;
			res.read(reinterpret_cast<char*>(&num16), sizeof(num16));
			num16 = LE2Native(num16);
			pnames_.resize(num16);
			for (size_t i = 0; i < num16; ++i)
			{
				uint8_t len8;
				res.read(reinterpret_cast<char*>(&len8), sizeof(len8));

				pnames_[i].resize(len8);
				res.read(&pnames_[i][0], len8);
			}

			res.read(reinterpret_cast<char*>(&num16), sizeof(num16));
			num16 = LE2Native(num16);
			glsl_res_names_.resize(num16);
			for (size_t i = 0; i < num16; ++i)
			{
				uint8_t len8;
				res.read(reinterpret_cast<char*>(&len8), sizeof(len8));

				glsl_res_names_[i].resize(len8);
				res.read(&glsl_res_names_[i][0], len8);
			}

			res.read(reinterpret_cast<char*>(&num16), sizeof(num16));
			num16 = LE2Native(num16);
			for (size_t i = 0; i < num16; ++i)
			{
				uint8_t len8;
				res.read(reinterpret_cast<char*>(&len8), sizeof(len8));

				std::string tex_name;
				tex_name.resize(len8);
				res.read(&tex_name[0], len8);

				res.read(reinterpret_cast<char*>(&len8), sizeof(len8));

				std::string sampler_name;
				sampler_name.resize(len8);
				res.read(&sampler_name[0], len8);

				tex_sampler_pairs_.push_back({ tex_name, sampler_name });
			}

			this->StageSpecificStreamIn(res);
		}
	}

	void OGLShaderStageObject::StreamOut(std::ostream& os)
	{
		std::vector<char> native_shader_block;

		if (!glsl_src_.empty())
		{
			VectorOutputStreamBuf native_shader_buff(native_shader_block);
			std::ostream oss(&native_shader_buff);

			uint32_t len32 = Native2LE(static_cast<uint32_t>(glsl_src_.size()));
			oss.write(reinterpret_cast<char const*>(&len32), sizeof(len32));
			oss.write(&glsl_src_[0], glsl_src_.size());

			uint16_t num16 = Native2LE(static_cast<uint16_t>(pnames_.size()));
			oss.write(reinterpret_cast<char const*>(&num16), sizeof(num16));
			for (size_t i = 0; i < pnames_.size(); ++i)
			{
				uint8_t len8 = static_cast<uint8_t>(pnames_[i].size());
				oss.write(reinterpret_cast<char const*>(&len8), sizeof(len8));
				oss.write(&pnames_[i][0], pnames_[i].size());
			}

			num16 = Native2LE(static_cast<uint16_t>(glsl_res_names_.size()));
			oss.write(reinterpret_cast<char const*>(&num16), sizeof(num16));
			for (size_t i = 0; i < glsl_res_names_.size(); ++i)
			{
				uint8_t len8 = static_cast<uint8_t>(glsl_res_names_[i].size());
				oss.write(reinterpret_cast<char const*>(&len8), sizeof(len8));
				oss.write(&glsl_res_names_[i][0], glsl_res_names_[i].size());
			}

			num16 = Native2LE(static_cast<uint16_t>(tex_sampler_pairs_.size()));
			oss.write(reinterpret_cast<char const*>(&num16), sizeof(num16));
			for (size_t i = 0; i < num16; ++i)
			{
				uint8_t len8 = static_cast<uint8_t>(tex_sampler_pairs_[i].first.size());
				oss.write(reinterpret_cast<char const*>(&len8), sizeof(len8));
				oss.write(&tex_sampler_pairs_[i].first[0], len8);

				len8 = static_cast<uint8_t>(tex_sampler_pairs_[i].second.size());
				oss.write(reinterpret_cast<char const*>(&len8), sizeof(len8));
				oss.write(&tex_sampler_pairs_[i].second[0], len8);
			}

			this->StageSpecificStreamOut(oss);
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

	void OGLShaderStageObject::CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
		std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		ShaderDesc const& sd = effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(stage_)]);

		shader_func_name_ = sd.func_name;

		bool has_gs = false;
		if (!effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(ShaderStage::Geometry)]).func_name.empty())
		{
			has_gs = true;
		}
		bool has_ps = false;
		if (!effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(ShaderStage::Pixel)]).func_name.empty())
		{
			has_ps = true;
		}

		is_validate_ = true;
		switch (stage_)
		{
		case ShaderStage::Vertex:
		case ShaderStage::Pixel:
		case ShaderStage::Hull:
			break;

		case ShaderStage::Domain:
#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
			{
				auto& shader_obj = *pass.GetShaderObject(effect);
				auto* hs_stage = checked_cast<OGLHullShaderStageObject*>(shader_obj.Stage(ShaderStage::Hull).get());
				BOOST_ASSERT(hs_stage != nullptr);
				checked_cast<OGLDomainShaderStageObject*>(this)->DsParameters(hs_stage->DsPartitioning(), hs_stage->DsOutputPrimitive());
			}
#endif
			break;

		case ShaderStage::Geometry:
			if (as_gles_)
			{
				is_validate_ = false;
			}
			break;

		default:
			is_validate_ = false;
			break;
		}

		if (is_validate_)
		{
			auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			auto const& caps = re.DeviceCaps();

			std::string_view const shader_profile = this->GetShaderProfile(effect, shader_desc_ids[static_cast<uint32_t>(stage_)]);
			is_validate_ = !shader_profile.empty();

			if (is_validate_)
			{
				std::string err_msg;
				std::vector<std::pair<char const*, char const*>> macros;
				macros.emplace_back("KLAYGE_DXBC2GLSL", "1");
				if (as_gles_)
				{
					macros.emplace_back("KLAYGE_OPENGLES", "1");
					if (!caps.TextureFormatSupport(EF_BC5) || !caps.TextureFormatSupport(EF_BC5_SRGB))
					{
						macros.emplace_back("KLAYGE_BC5_AS_AG", "1");
					}
					else
					{
						macros.emplace_back("KLAYGE_BC5_AS_GA", "1");
					}
				}
				else
				{
					macros.emplace_back("KLAYGE_OPENGL", "1");
					if (!caps.TextureFormatSupport(EF_BC5) || !caps.TextureFormatSupport(EF_BC5_SRGB))
					{
						macros.emplace_back("KLAYGE_BC5_AS_AG", "1");
					}
				}
				if (!caps.TextureFormatSupport(EF_BC4) || !caps.TextureFormatSupport(EF_BC4_SRGB))
				{
					macros.emplace_back("KLAYGE_BC4_AS_G", "1");
				}
				if (as_gles_)
				{
					bool frag_depth_support;
					re.GetCustomAttrib("FRAG_DEPTH_SUPPORT", &frag_depth_support);
					macros.emplace_back("KLAYGE_FRAG_DEPTH", frag_depth_support ? "1" : "0");
				}
				else
				{
					macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");
				}

				uint32_t const flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
				std::vector<uint8_t> code = ShaderStageObject::CompileToDXBC(
					stage_, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile.data(), flags);
				if (code.empty())
				{
					is_validate_ = false;
				}
				else
				{
					try
					{
						int major_version, minor_version;
						re.GetCustomAttrib("MAJOR_VERSION", &major_version);
						re.GetCustomAttrib("MINOR_VERSION", &minor_version);

						GLSLVersion gsv;
						if (as_gles_)
						{
							switch (major_version)
							{
							case 3:
								switch (minor_version)
								{
								case 2:
									gsv = GSV_320_ES;
									break;

								case 1:
									gsv = GSV_310_ES;
									break;

								default:
								case 0:
									gsv = GSV_300_ES;
									break;
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid OpenGLES version");
							}
						}
						else
						{
							switch (major_version)
							{
							case 4:
								switch (minor_version)
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
									KFL_UNREACHABLE("Invalid OpenGL 4 sub-version");
								}
								break;

							default:
								KFL_UNREACHABLE("Invalid OpenGL version");
							}
						}

						DXBC2GLSL::DXBC2GLSL dxbc2glsl;
						uint32_t rules = DXBC2GLSL::DXBC2GLSL::DefaultRules(gsv);
						rules &= ~GSR_UniformBlockBinding;
						if (as_gles_)
						{
							rules &= ~GSR_MatrixType;
							rules &= ~GSR_UIntType;
							rules |= caps.max_simultaneous_rts > 1 ? static_cast<uint32_t>(GSR_DrawBuffers) : 0;
							if ((ShaderStage::Hull == stage_) || (ShaderStage::Domain == stage_))
							{
								rules |= static_cast<uint32_t>(GSR_EXTTessellationShader);
							}
						}
						dxbc2glsl.FeedDXBC(&code[0], has_gs, has_ps, static_cast<ShaderTessellatorPartitioning>(this->DsPartitioning()),
							static_cast<ShaderTessellatorOutputPrimitive>(this->DsOutputPrimitive()), gsv, rules);
						glsl_src_ = dxbc2glsl.GLSLString();
						pnames_.clear();
						glsl_res_names_.clear();

						for (uint32_t i = 0; i < dxbc2glsl.NumCBuffers(); ++i)
						{
							for (uint32_t j = 0; j < dxbc2glsl.NumVariables(i); ++j)
							{
								if (dxbc2glsl.VariableUsed(i, j))
								{
									pnames_.push_back(dxbc2glsl.VariableName(i, j));
									glsl_res_names_.push_back(dxbc2glsl.VariableName(i, j));
								}
							}
						}

						std::vector<char const*> tex_names;
						std::vector<char const*> sampler_names;
						for (uint32_t i = 0; i < dxbc2glsl.NumResources(); ++i)
						{
							if (dxbc2glsl.ResourceUsed(i))
							{
								char const* res_name = dxbc2glsl.ResourceName(i);

								if (SIT_TEXTURE == dxbc2glsl.ResourceType(i))
								{
									if (SSD_BUFFER == dxbc2glsl.ResourceDimension(i))
									{
										pnames_.push_back(res_name);
										glsl_res_names_.push_back(res_name);
									}
									else
									{
										tex_names.push_back(res_name);
									}
								}
								else if (SIT_SAMPLER == dxbc2glsl.ResourceType(i))
								{
									sampler_names.push_back(res_name);
								}
							}
						}

						for (size_t i = 0; i < tex_names.size(); ++i)
						{
							for (size_t j = 0; j < sampler_names.size(); ++j)
							{
								std::string const combined_sampler_name = std::string(tex_names[i]) + "_" + sampler_names[j];
								tex_sampler_pairs_.push_back({ tex_names[i], sampler_names[j] });

								pnames_.push_back(combined_sampler_name);
								glsl_res_names_.push_back(combined_sampler_name);
							}
						}

						this->StageSpecificAttachShader(dxbc2glsl);
					}
					catch (std::exception& ex)
					{
						is_validate_ = false;

						LogError() << "Error(s) in conversion: " << tech.Name() << "/" << pass.Name() << "/" << sd.func_name << std::endl;
						LogError() << ex.what() << std::endl;
						LogError() << "Please send this information and your shader to webmaster at klayge.org. We'll fix this ASAP."
							<< std::endl;
					}
				}
			}
		}
	}

	void OGLShaderStageObject::CreateHwShader(RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(shader_desc_ids);

		hw_res_ready_ = true;
	}

	std::string_view OGLShaderStageObject::GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const
	{
		std::string_view shader_profile = effect.GetShaderDesc(shader_desc_id).profile;
		if (is_available_)
		{
			if (shader_profile == "auto")
			{
				auto const& re = checked_cast<NullRenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				shader_profile = re.DefaultShaderProfile(stage_);
			}
		}
		else
		{
			shader_profile = std::string_view();
		}

		return shader_profile;
	}


	OGLVertexShaderStageObject::OGLVertexShaderStageObject(bool as_gles)
		: OGLShaderStageObject(ShaderStage::Vertex, GL_VERTEX_SHADER, as_gles)
	{
		is_available_ = true;
	}

	void OGLVertexShaderStageObject::StageSpecificStreamIn(ResIdentifier& res)
	{
		uint8_t num8;
		res.read(reinterpret_cast<char*>(&num8), sizeof(num8));
		usages_.resize(num8);
		for (size_t i = 0; i < num8; ++i)
		{
			uint8_t veu;
			res.read(reinterpret_cast<char*>(&veu), sizeof(veu));

			usages_[i] = static_cast<VertexElementUsage>(veu);
		}

		res.read(reinterpret_cast<char*>(&num8), sizeof(num8));
		if (num8 > 0)
		{
			usage_indices_.resize(num8);
			res.read(reinterpret_cast<char*>(&usage_indices_[0]), num8 * sizeof(usage_indices_[0]));
		}

		res.read(reinterpret_cast<char*>(&num8), sizeof(num8));
		glsl_attrib_names_.resize(num8);
		for (size_t i = 0; i < num8; ++i)
		{
			uint8_t len8;
			res.read(reinterpret_cast<char*>(&len8), sizeof(len8));

			glsl_attrib_names_[i].resize(len8);
			res.read(&glsl_attrib_names_[i][0], len8);
		}
	}

	void OGLVertexShaderStageObject::StageSpecificStreamOut(std::ostream& os)
	{
		uint8_t num8 = static_cast<uint8_t>(usages_.size());
		os.write(reinterpret_cast<char const*>(&num8), sizeof(num8));
		for (size_t i = 0; i < usages_.size(); ++i)
		{
			uint8_t veu = static_cast<uint8_t>(usages_[i]);
			os.write(reinterpret_cast<char const*>(&veu), sizeof(veu));
		}

		num8 = static_cast<uint8_t>(usage_indices_.size());
		os.write(reinterpret_cast<char const*>(&num8), sizeof(num8));
		if (!usage_indices_.empty())
		{
			os.write(reinterpret_cast<char const*>(&usage_indices_[0]), usage_indices_.size() * sizeof(usage_indices_[0]));
		}

		num8 = static_cast<uint8_t>(glsl_attrib_names_.size());
		os.write(reinterpret_cast<char const*>(&num8), sizeof(num8));
		for (size_t i = 0; i < glsl_attrib_names_.size(); ++i)
		{
			uint8_t len8 = static_cast<uint8_t>(glsl_attrib_names_[i].size());
			os.write(reinterpret_cast<char const*>(&len8), sizeof(len8));
			os.write(&glsl_attrib_names_[i][0], glsl_attrib_names_[i].size());
		}
	}

	void OGLVertexShaderStageObject::StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl)
	{
		for (uint32_t i = 0; i < dxbc2glsl.NumInputParams(); ++i)
		{
			if (dxbc2glsl.InputParam(i).mask != 0)
			{
				std::string semantic = dxbc2glsl.InputParam(i).semantic_name;
				uint32_t semantic_index = dxbc2glsl.InputParam(i).semantic_index;
				std::string glsl_param_name = semantic;
				size_t const semantic_hash = RT_HASH(semantic.c_str());

				if ((CT_HASH("SV_VertexID") != semantic_hash) && (CT_HASH("SV_InstanceID") != semantic_hash))
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
						glsl_param_name = std::format("TEXCOORD{}", semantic_index);
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
						KFL_UNREACHABLE("Invalid semantic");
					}

					usages_.push_back(usage);
					usage_indices_.push_back(usage_index);
					glsl_attrib_names_.push_back(glsl_param_name);
				}
			}
		}
	}


	OGLPixelShaderStageObject::OGLPixelShaderStageObject(bool as_gles)
		: OGLShaderStageObject(ShaderStage::Pixel, GL_FRAGMENT_SHADER, as_gles)
	{
		is_available_ = true;
	}


	OGLGeometryShaderStageObject::OGLGeometryShaderStageObject()
		: OGLShaderStageObject(ShaderStage::Geometry, GL_GEOMETRY_SHADER, false)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.gs_support;
	}

	void OGLGeometryShaderStageObject::StageSpecificStreamIn(ResIdentifier& res)
	{
		res.read(reinterpret_cast<char*>(&gs_input_type_), sizeof(gs_input_type_));
		gs_input_type_ = LE2Native(gs_input_type_);

		res.read(reinterpret_cast<char*>(&gs_output_type_), sizeof(gs_output_type_));
		gs_output_type_ = LE2Native(gs_output_type_);

		res.read(reinterpret_cast<char*>(&gs_max_output_vertex_), sizeof(gs_max_output_vertex_));
		gs_max_output_vertex_ = LE2Native(gs_max_output_vertex_);
	}

	void OGLGeometryShaderStageObject::StageSpecificStreamOut(std::ostream& os)
	{
		uint32_t git = Native2LE(gs_input_type_);
		os.write(reinterpret_cast<char const*>(&git), sizeof(git));

		uint32_t got = Native2LE(gs_output_type_);
		os.write(reinterpret_cast<char const*>(&got), sizeof(got));

		uint32_t gmov = Native2LE(gs_max_output_vertex_);
		os.write(reinterpret_cast<char const*>(&gmov), sizeof(gmov));
	}

	void OGLGeometryShaderStageObject::StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl)
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
			gs_input_type_ = GL_LINES_ADJACENCY;
			break;

		case SP_Triangle:
			gs_input_type_ = GL_TRIANGLES;
			break;

		case SP_TriangleAdj:
			gs_input_type_ = GL_TRIANGLES_ADJACENCY;
			break;

		default:
			KFL_UNREACHABLE("Invalid GS input type");
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
			KFL_UNREACHABLE("Invalid GS output topology");
		}

		gs_max_output_vertex_ = dxbc2glsl.MaxGSOutputVertex();
	}


	OGLESGeometryShaderStageObject::OGLESGeometryShaderStageObject()
		: OGLShaderStageObject(ShaderStage::Geometry, GL_GEOMETRY_SHADER, true)
	{
		is_available_ = false;
		is_validate_ = false;
	}


	OGLComputeShaderStageObject::OGLComputeShaderStageObject(bool as_gles)
		: OGLShaderStageObject(ShaderStage::Compute, GL_COMPUTE_SHADER, as_gles)
	{
		is_available_ = false;
		is_validate_ = false;
	}


	OGLHullShaderStageObject::OGLHullShaderStageObject(bool as_gles)
		: OGLShaderStageObject(ShaderStage::Hull, GL_TESS_CONTROL_SHADER, as_gles)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.hs_support;
	}

	void OGLHullShaderStageObject::StageSpecificAttachShader(DXBC2GLSL::DXBC2GLSL const& dxbc2glsl)
	{
		ds_partitioning_ = dxbc2glsl.DSPartitioning();
		ds_output_primitive_ = dxbc2glsl.DSOutputPrimitive();
	}


	OGLDomainShaderStageObject::OGLDomainShaderStageObject(bool as_gles)
		: OGLShaderStageObject(ShaderStage::Domain, GL_TESS_EVALUATION_SHADER, as_gles)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.ds_support;
	}

	void OGLDomainShaderStageObject::DsParameters(uint32_t partitioning, uint32_t output_primitive)
	{
		ds_partitioning_ = partitioning;
		ds_output_primitive_ = output_primitive;
	}


	NullShaderObject::NullShaderObject()
		: NullShaderObject(MakeSharedPtr<ShaderObjectTemplate>(), MakeSharedPtr<NullShaderObjectTemplate>())
	{
		auto const& re = checked_cast<NullRenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		std::string_view const platform_name = re.NativeShaderPlatformName();
		if (platform_name.find("d3d_11_") == 0)
		{
			null_so_template_->as_d3d11_ = true;
		}
		else if (platform_name.find("d3d_12_") == 0)
		{
			null_so_template_->as_d3d12_ = true;
		}
#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
		else if (platform_name.find("gl_") == 0)
		{
			null_so_template_->as_gl_ = true;
		}
		else if (platform_name.find("gles_") == 0)
		{
			null_so_template_->as_gles_ = true;
		}
#endif
	}
	
	NullShaderObject::NullShaderObject(
		std::shared_ptr<ShaderObjectTemplate> so_template, std::shared_ptr<NullShaderObjectTemplate> null_so_template)
		: ShaderObject(std::move(so_template)), null_so_template_(std::move(null_so_template))
	{
	}

	ShaderObjectPtr NullShaderObject::Clone(RenderEffect const & effect)
	{
		KFL_UNUSED(effect);
		return MakeSharedPtr<NullShaderObject>();
	}

	void NullShaderObject::Bind(RenderEffect const& effect)
	{
		KFL_UNUSED(effect);
	}

	void NullShaderObject::Unbind()
	{
	}

	void NullShaderObject::CreateHwResources(ShaderStage stage, RenderEffect const & effect)
	{
		if (null_so_template_->as_gl_ || null_so_template_->as_gles_)
		{
			this->OGLAppendTexSamplerBinds(stage, effect, checked_cast<OGLShaderStageObject*>(this->Stage(stage).get())->TexSamplerPairs());
		}
	}

	void NullShaderObject::DoLinkShaders(RenderEffect const & effect)
	{
		KFL_UNUSED(effect);
	}

	// OpenGL/OpenGLES

	void NullShaderObject::OGLAppendTexSamplerBinds(
		ShaderStage stage, RenderEffect const& effect, std::vector<std::pair<std::string, std::string>> const& tex_sampler_pairs)
	{
		uint32_t const mask = 1UL << static_cast<uint32_t>(stage);
		for (auto const& tex_sampler : tex_sampler_pairs)
		{
			std::string const combined_sampler_name = tex_sampler.first + "_" + tex_sampler.second;

			bool found = false;
			for (uint32_t k = 0; k < gl_tex_sampler_binds_.size(); ++k)
			{
				if (std::get<0>(gl_tex_sampler_binds_[k]) == combined_sampler_name)
				{
					std::get<3>(gl_tex_sampler_binds_[k]) |= mask;
					found = true;
					break;
				}
			}
			if (!found)
			{
				gl_tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name, effect.ParameterByName(tex_sampler.first),
					effect.ParameterByName(tex_sampler.second), mask));
			}
		}
	}
}
