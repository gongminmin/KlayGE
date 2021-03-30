/**
 * @file D3D12ShaderObject.cpp
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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/ResIdentifier.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KFL/CustomizedStreamBuf.hpp>
#include <KFL/Hash.hpp>

#include <string>
#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>

#if KLAYGE_IS_DEV_PLATFORM
#include <KlayGE/SALWrapper.hpp>
#include <d3dcompiler.h>
#endif
#if defined(KLAYGE_COMPILER_GCC)
#undef __out
#endif

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12InterfaceLoader.hpp>
#include <KlayGE/D3D12/D3D12ShaderObject.hpp>

namespace
{
	using namespace KlayGE;

	class D3D12ShaderParameterSrvUpdater final
	{
	public:
		D3D12ShaderParameterSrvUpdater(std::tuple<D3D12Resource*, uint32_t, uint32_t>& srvsrc, D3D12_CPU_DESCRIPTOR_HANDLE& srv_handle,
			RenderEffectParameter const& param)
			: srvsrc_(srvsrc), srv_handle_(srv_handle), param_(param)
		{
		}

		void operator()()
		{
			D3D12ShaderResourceViewSimulation* srv_sim;
			ShaderResourceViewPtr srv;
			param_.Value(srv);
			if (srv)
			{
				if (srv->TextureResource())
				{
					srvsrc_ = std::make_tuple(checked_cast<D3D12Texture*>(srv->TextureResource().get()),
						srv->FirstArrayIndex() * srv->TextureResource()->NumMipMaps() + srv->FirstLevel(),
						srv->ArraySize() * srv->NumLevels());
				}
				else
				{
					BOOST_ASSERT(srv->BufferResource());
					srvsrc_ = std::make_tuple(checked_cast<D3D12GraphicsBuffer*>(srv->BufferResource().get()), 0, 1);
				}

				srv_sim = checked_cast<D3D12ShaderResourceView&>(*srv).RetrieveD3DShaderResourceView().get();
			}
			else
			{
				std::get<0>(srvsrc_) = nullptr;
				srv_sim = nullptr;
			}

			auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			srv_handle_ = srv_sim ? srv_sim->Handle() : re.NullSrvHandle();
		}

	private:
		std::tuple<D3D12Resource*, uint32_t, uint32_t>& srvsrc_;
		D3D12_CPU_DESCRIPTOR_HANDLE& srv_handle_;
		RenderEffectParameter const& param_;
	};

	class D3D12ShaderParameterUavUpdater final
	{
	public:
		D3D12ShaderParameterUavUpdater(
			std::tuple<D3D12Resource*, uint32_t, uint32_t>& uavsrc, D3D12_CPU_DESCRIPTOR_HANDLE& uav_handle, RenderEffectParameter const& param)
			: uavsrc_(uavsrc), uav_handle_(uav_handle), param_(param)
		{
		}

		void operator()()
		{
			D3D12UnorderedAccessViewSimulation* uav_sim;
			UnorderedAccessViewPtr uav;
			param_.Value(uav);
			if (uav)
			{
				if (uav->TextureResource())
				{
					auto* tex = checked_cast<D3D12Texture*>(uav->TextureResource().get());
					uavsrc_ = std::make_tuple(tex, uav->FirstArrayIndex() * uav->TextureResource()->NumMipMaps() + uav->Level(), 1);
				}
				else
				{
					BOOST_ASSERT(uav->BufferResource());
					uavsrc_ = std::make_tuple(checked_cast<D3D12GraphicsBuffer*>(uav->BufferResource().get()), 0, 1);
				}

				uav_sim = checked_cast<D3D12UnorderedAccessView&>(*uav).RetrieveD3DUnorderedAccessView();
			}
			else
			{
				std::get<0>(uavsrc_) = nullptr;
				uav_sim = nullptr;
			}

			auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			uav_handle_ = uav_sim ? uav_sim->Handle() : re.NullUavHandle();
		}

	private:
		std::tuple<D3D12Resource*, uint32_t, uint32_t>& uavsrc_;
		D3D12_CPU_DESCRIPTOR_HANDLE& uav_handle_;
		RenderEffectParameter const& param_;
	};
}

namespace KlayGE
{
	D3D12ShaderStageObject::D3D12ShaderStageObject(ShaderStage stage) : ShaderStageObject(stage)
	{
	}

	void D3D12ShaderStageObject::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		KFL_UNUSED(pso_desc);
		KFL_UNREACHABLE("Couldn't update graphics pipeline state for this shader stage.");
	}

	void D3D12ShaderStageObject::UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		KFL_UNUSED(pso_desc);
		KFL_UNREACHABLE("Couldn't update compute pipeline state for this shader stage.");
	}

	void D3D12ShaderStageObject::StreamIn(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids, ResIdentifier& res)
	{
		uint32_t native_shader_block_len;
		res.read(&native_shader_block_len, sizeof(native_shader_block_len));
		native_shader_block_len = LE2Native(native_shader_block_len);

		is_validate_ = false;
		std::string_view shader_profile = this->GetShaderProfile(effect, shader_desc_ids[static_cast<uint32_t>(stage_)]);
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

	void D3D12ShaderStageObject::StreamOut(std::ostream& os)
	{
		std::vector<char> native_shader_block;
		VectorOutputStreamBuf native_shader_buff(native_shader_block);
		std::ostream oss(&native_shader_buff);

		{
			uint8_t len = static_cast<uint8_t>(shader_profile_.size());
			oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
			oss.write(reinterpret_cast<char const *>(shader_profile_.data()), len);
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
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
		if (len > 0)
		{
			os.write(reinterpret_cast<char const *>(&native_shader_block[0]), len * sizeof(native_shader_block[0]));
		}
	}

	void D3D12ShaderStageObject::CompileShader(RenderEffect const& effect, RenderTechnique const& tech, RenderPass const& pass,
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
			std::vector<std::pair<char const *, char const *>> macros;
			macros.emplace_back("KLAYGE_D3D12", "1");
			macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");

			uint32_t flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if !defined(KLAYGE_DEBUG)
			flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			shader_code_ =
				ShaderStageObject::CompileToDXBC(stage_, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile_.c_str(), flags);

			if (!shader_code_.empty())
			{
				com_ptr<ID3D12ShaderReflection> reflection;
				ShaderStageObject::ReflectDXBC(shader_code_, reflection.put_void());
				if (reflection != nullptr)
				{
					D3D12_SHADER_DESC desc;
					reflection->GetDesc(&desc);

					for (UINT c = 0; c < desc.ConstantBuffers; ++c)
					{
						ID3D12ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

						D3D12_SHADER_BUFFER_DESC d3d_cb_desc;
						reflection_cb->GetDesc(&d3d_cb_desc);
						if ((D3D_CT_CBUFFER == d3d_cb_desc.Type) || (D3D_CT_TBUFFER == d3d_cb_desc.Type))
						{
							auto& cb_desc = shader_desc_.cb_desc.emplace_back();
							cb_desc.name = d3d_cb_desc.Name;
							cb_desc.name_hash = RT_HASH(d3d_cb_desc.Name);
							cb_desc.size = d3d_cb_desc.Size;

							for (UINT v = 0; v < d3d_cb_desc.Variables; ++v)
							{
								ID3D12ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

								D3D12_SHADER_VARIABLE_DESC var_desc;
								reflection_var->GetDesc(&var_desc);

								D3D12_SHADER_TYPE_DESC type_desc;
								reflection_var->GetType()->GetDesc(&type_desc);

								auto& vd = cb_desc.var_desc.emplace_back();
								vd.name = var_desc.Name;
								vd.start_offset = var_desc.StartOffset;
								vd.type = static_cast<uint8_t>(type_desc.Type);
								vd.rows = static_cast<uint8_t>(type_desc.Rows);
								vd.columns = static_cast<uint8_t>(type_desc.Columns);
								vd.elements = static_cast<uint16_t>(type_desc.Elements);
							}
						}
					}

					this->FillCBufferIndices(effect);

					int max_sampler_bind_pt = -1;
					int max_srv_bind_pt = -1;
					int max_uav_bind_pt = -1;
					for (uint32_t i = 0; i < desc.BoundResources; ++i)
					{
						D3D12_SHADER_INPUT_BIND_DESC si_desc;
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
						D3D12_SHADER_INPUT_BIND_DESC si_desc;
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
								auto& brd = shader_desc_.res_desc.emplace_back();
								brd.name = si_desc.Name;
								brd.type = static_cast<uint8_t>(si_desc.Type);
								brd.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
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

	void D3D12ShaderStageObject::CreateHwShader(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		if (!shader_code_.empty())
		{
			auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			auto const& caps = re.DeviceCaps();
			auto const & sd = effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(stage_)]);

			uint8_t shader_major_ver = ("auto" == sd.profile) ? 0 : static_cast<uint8_t>(sd.profile[3] - '0');
			uint8_t shader_minor_ver = ("auto" == sd.profile) ? 0 : static_cast<uint8_t>(sd.profile[5] - '0');
			if (ShaderModel(shader_major_ver, shader_minor_ver) > caps.max_shader_model)
			{
				is_validate_ = false;
			}
			else
			{
				is_validate_ = true;
				this->StageSpecificCreateHwShader(effect, shader_desc_ids);
			}
		}
		else
		{
			is_validate_ = false;
		}

		hw_res_ready_ = true;
	}

	std::span<uint8_t const> D3D12ShaderStageObject::ShaderCodeBlob() const
	{
		return MakeSpan(shader_code_);
	}

	void D3D12ShaderStageObject::FillCBufferIndices(RenderEffect const& effect)
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

	std::string_view D3D12ShaderStageObject::GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const
	{
		std::string_view shader_profile = effect.GetShaderDesc(shader_desc_id).profile;
		if (is_available_)
		{
			if (shader_profile == "auto")
			{
				auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				shader_profile = re.DefaultShaderProfile(stage_);
			}
		}
		else
		{
			shader_profile = std::string_view();
		}
		return shader_profile;
	}


	D3D12VertexShaderStageObject::D3D12VertexShaderStageObject() : D3D12ShaderStageObject(ShaderStage::Vertex)
	{
		is_available_ = true;
	}

	void D3D12VertexShaderStageObject::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		pso_desc.VS.pShaderBytecode = shader_code_.data();
		pso_desc.VS.BytecodeLength = static_cast<UINT>(shader_code_.size());

		if (pso_desc.StreamOutput.pSODeclaration == nullptr)
		{
			pso_desc.StreamOutput.pSODeclaration = so_decl_.data();
			pso_desc.StreamOutput.NumEntries = static_cast<UINT>(so_decl_.size());
			pso_desc.StreamOutput.pBufferStrides = nullptr;
			pso_desc.StreamOutput.NumStrides = 0;
			pso_desc.StreamOutput.RasterizedStream = rasterized_stream_;
		}
	}

	void D3D12VertexShaderStageObject::StageSpecificCreateHwShader(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		auto const& sd = effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(stage_)]);
		if (caps.gs_support && !sd.so_decl.empty())
		{
			so_decl_.resize(sd.so_decl.size());
			for (size_t i = 0; i < sd.so_decl.size(); ++i)
			{
				so_decl_[i] = D3D12Mapping::Mapping(sd.so_decl[i]);
			}

			rasterized_stream_ = 0;
			if (effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(ShaderStage::Pixel)]).func_name.empty())
			{
				rasterized_stream_ = D3D12_SO_NO_RASTERIZED_STREAM;
			}
		}
	}


	D3D12PixelShaderStageObject::D3D12PixelShaderStageObject() : D3D12ShaderStageObject(ShaderStage::Pixel)
	{
		is_available_ = true;
	}

	void D3D12PixelShaderStageObject::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		pso_desc.PS.pShaderBytecode = shader_code_.data();
		pso_desc.PS.BytecodeLength = static_cast<UINT>(shader_code_.size());
	}


	D3D12GeometryShaderStageObject::D3D12GeometryShaderStageObject() : D3D12ShaderStageObject(ShaderStage::Geometry)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.gs_support;
	}

	void D3D12GeometryShaderStageObject::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		pso_desc.GS.pShaderBytecode = shader_code_.data();
		pso_desc.GS.BytecodeLength = static_cast<UINT>(shader_code_.size());

		if (pso_desc.StreamOutput.pSODeclaration == nullptr)
		{
			pso_desc.StreamOutput.pSODeclaration = so_decl_.data();
			pso_desc.StreamOutput.NumEntries = static_cast<UINT>(so_decl_.size());
			pso_desc.StreamOutput.pBufferStrides = nullptr;
			pso_desc.StreamOutput.NumStrides = 0;
			pso_desc.StreamOutput.RasterizedStream = rasterized_stream_;
		}
	}

	void D3D12GeometryShaderStageObject::StageSpecificCreateHwShader(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		if (is_available_)
		{
			ShaderDesc const& sd = effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(stage_)]);
			so_decl_.resize(sd.so_decl.size());
			for (size_t i = 0; i < sd.so_decl.size(); ++i)
			{
				so_decl_[i] = D3D12Mapping::Mapping(sd.so_decl[i]);
			}

			rasterized_stream_ = 0;
			if (effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(ShaderStage::Pixel)]).func_name.empty())
			{
				rasterized_stream_ = D3D12_SO_NO_RASTERIZED_STREAM;
			}
		}
		else
		{
			is_validate_ = false;
		}
	}


	D3D12ComputeShaderStageObject::D3D12ComputeShaderStageObject() : D3D12ShaderStageObject(ShaderStage::Compute)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.cs_support;
	}

	void D3D12ComputeShaderStageObject::UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		pso_desc.CS.pShaderBytecode = shader_code_.data();
		pso_desc.CS.BytecodeLength = static_cast<UINT>(shader_code_.size());
	}

	void D3D12ComputeShaderStageObject::StageSpecificCreateHwShader(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(shader_desc_ids);

		if (!is_available_)
		{
			is_validate_ = false;
		}
	}

	void D3D12ComputeShaderStageObject::StageSpecificStreamIn(ResIdentifier& res)
	{
		res.read(reinterpret_cast<char*>(&block_size_x_), sizeof(block_size_x_));
		block_size_x_ = LE2Native(block_size_x_);

		res.read(reinterpret_cast<char*>(&block_size_y_), sizeof(block_size_y_));
		block_size_y_ = LE2Native(block_size_y_);

		res.read(reinterpret_cast<char*>(&block_size_z_), sizeof(block_size_z_));
		block_size_z_ = LE2Native(block_size_z_);
	}

	void D3D12ComputeShaderStageObject::StageSpecificStreamOut(std::ostream& os)
	{
		uint32_t block_size_x = Native2LE(block_size_x_);
		os.write(reinterpret_cast<char const*>(&block_size_x), sizeof(block_size_x));

		uint32_t block_size_y = Native2LE(block_size_y_);
		os.write(reinterpret_cast<char const*>(&block_size_y), sizeof(block_size_y));

		uint32_t block_size_z = Native2LE(block_size_z_);
		os.write(reinterpret_cast<char const*>(&block_size_z), sizeof(block_size_z));
	}

#if KLAYGE_IS_DEV_PLATFORM
	void D3D12ComputeShaderStageObject::StageSpecificReflection(ID3D12ShaderReflection* reflection)
	{
		reflection->GetThreadGroupSize(&block_size_x_, &block_size_y_, &block_size_z_);
	}
#endif


	D3D12HullShaderStageObject::D3D12HullShaderStageObject() : D3D12ShaderStageObject(ShaderStage::Hull)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.hs_support;
	}

	void D3D12HullShaderStageObject::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		pso_desc.HS.pShaderBytecode = shader_code_.data();
		pso_desc.HS.BytecodeLength = static_cast<UINT>(shader_code_.size());
	}

	void D3D12HullShaderStageObject::StageSpecificCreateHwShader(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		KFL_UNUSED(effect);
		KFL_UNUSED(shader_desc_ids);

		if (!is_available_)
		{
			is_validate_ = false;
		}
	}


	D3D12DomainShaderStageObject::D3D12DomainShaderStageObject() : D3D12ShaderStageObject(ShaderStage::Domain)
	{
		auto const& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& caps = re.DeviceCaps();
		is_available_ = caps.ds_support;
	}

	void D3D12DomainShaderStageObject::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		pso_desc.DS.pShaderBytecode = shader_code_.data();
		pso_desc.DS.BytecodeLength = static_cast<UINT>(shader_code_.size());

		if (pso_desc.StreamOutput.pSODeclaration == nullptr)
		{
			pso_desc.StreamOutput.pSODeclaration = so_decl_.data();
			pso_desc.StreamOutput.NumEntries = static_cast<UINT>(so_decl_.size());
			pso_desc.StreamOutput.pBufferStrides = nullptr;
			pso_desc.StreamOutput.NumStrides = 0;
			pso_desc.StreamOutput.RasterizedStream = rasterized_stream_;
		}
	}

	void D3D12DomainShaderStageObject::StageSpecificCreateHwShader(
		RenderEffect const& effect, std::array<uint32_t, NumShaderStages> const& shader_desc_ids)
	{
		if (is_available_)
		{
			auto const& sd = effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(stage_)]);
			if (!sd.so_decl.empty())
			{
				auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				auto const& caps = re.DeviceCaps();
				if (caps.gs_support)
				{
					so_decl_.resize(sd.so_decl.size());
					for (size_t i = 0; i < sd.so_decl.size(); ++i)
					{
						so_decl_[i] = D3D12Mapping::Mapping(sd.so_decl[i]);
					}

					rasterized_stream_ = 0;
					if (effect.GetShaderDesc(shader_desc_ids[static_cast<uint32_t>(ShaderStage::Pixel)]).func_name.empty())
					{
						rasterized_stream_ = D3D12_SO_NO_RASTERIZED_STREAM;
					}
				}
				else
				{
					is_validate_ = false;
				}
			}
		}
		else
		{
			is_validate_ = false;
		}
	}


	D3D12ShaderObject::D3D12ShaderObject() : D3D12ShaderObject(MakeSharedPtr<Immutable>(), MakeSharedPtr<D3D12Immutable>())
	{
	}

	D3D12ShaderObject::D3D12ShaderObject(std::shared_ptr<Immutable> immutable, std::shared_ptr<D3D12Immutable> d3d_immutable) noexcept
		: ShaderObject(std::move(immutable)), d3d_immutable_(std::move(d3d_immutable))
	{
	}

	D3D12ShaderObject::~D3D12ShaderObject()
	{
		if (srv_uav_desc_block_ && Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocDynamicCbvSrvUavDescBlock(std::move(srv_uav_desc_block_));
		}
	}

	void D3D12ShaderObject::DoLinkShaders(RenderEffect& effect)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		std::array<uint32_t, NumShaderStages * 4> num;
		uint32_t num_sampler = 0;
		for (uint32_t stage = 0; stage < NumShaderStages; ++stage)
		{
			auto const* shader_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(static_cast<ShaderStage>(stage)).get());
			if (shader_stage)
			{
				auto const& shader_desc = shader_stage->GetD3D12ShaderDesc();
				num[stage * 4 + 0] = static_cast<uint32_t>(shader_desc.cb_desc.size());

				if (!shader_stage->ShaderCodeBlob().empty())
				{
					d3d_immutable_->num_srvs_[stage] = shader_desc.num_srvs;
					d3d_immutable_->num_uavs_[stage] = shader_desc.num_uavs;
					d3d_immutable_->num_samplers_[stage] = shader_desc.num_samplers;

					num_sampler += shader_desc.num_samplers;
				}
			}
			else
			{
				num[stage * 4 + 0] = 0;

				d3d_immutable_->num_srvs_[stage] = 0;
				d3d_immutable_->num_uavs_[stage] = 0;
				d3d_immutable_->num_samplers_[stage] = 0;
			}

			num[stage * 4 + 1] = d3d_immutable_->num_srvs_[stage];
			num[stage * 4 + 2] = d3d_immutable_->num_uavs_[stage];
			num[stage * 4 + 3] = d3d_immutable_->num_samplers_[stage];
		}

		bool has_stream_output = false;
		if (this->Stage(ShaderStage::Geometry) &&
			checked_cast<D3D12ShaderStageObject&>(*this->Stage(ShaderStage::Geometry)).HasStreamOutput())
		{
			has_stream_output = true;
		}
		else if (this->Stage(ShaderStage::Domain) &&
				 checked_cast<D3D12ShaderStageObject&>(*this->Stage(ShaderStage::Domain)).HasStreamOutput())
		{
			has_stream_output = true;
		}
		else if (this->Stage(ShaderStage::Vertex) &&
				 checked_cast<D3D12ShaderStageObject&>(*this->Stage(ShaderStage::Vertex)).HasStreamOutput())
		{
			has_stream_output = true;
		}

		d3d_immutable_->root_signature_ = re.CreateRootSignature(num, !!this->Stage(ShaderStage::Vertex), has_stream_output);

		uint32_t srv_starts[NumShaderStages + 1];
		uint32_t uav_starts[NumShaderStages + 1];
		srv_starts[0] = uav_starts[0] = 0;
		for (uint32_t stage = 1; stage <= NumShaderStages; ++stage)
		{
			srv_starts[stage] = srv_starts[stage - 1] + d3d_immutable_->num_srvs_[stage - 1];
			uav_starts[stage] = uav_starts[stage - 1] + d3d_immutable_->num_uavs_[stage - 1];
		}

		d3d_immutable_->num_total_srvs_ = srv_starts[NumShaderStages];
		srv_uav_handles_.resize(srv_starts[NumShaderStages] + uav_starts[NumShaderStages]);
		srv_uav_srcs_.resize(srv_uav_handles_.size(), std::make_tuple(static_cast<D3D12Resource*>(nullptr), 0, 0));

		ID3D12Device* device = re.D3DDevice();

		uint32_t const sampler_desc_size = re.SamplerDescSize();
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_sampler_handle{};
		if (num_sampler > 0)
		{
			d3d_immutable_->sampler_desc_block_ = re.AllocSamplerDescBlock(num_sampler);
			cpu_sampler_handle = d3d_immutable_->sampler_desc_block_.CpuHandle();
		}

		std::vector<uint32_t> all_cbuff_indices;
		for (uint32_t stage = 0; stage < NumShaderStages; ++stage)
		{
			auto const* shader_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(static_cast<ShaderStage>(stage)).get());
			if (shader_stage)
			{
				auto const& shader_desc = shader_stage->GetD3D12ShaderDesc();
				if (!shader_stage->ShaderCodeBlob().empty())
				{
					for (size_t i = 0; i < shader_desc.res_desc.size(); ++i)
					{
						RenderEffectParameter* p = effect.ParameterByName(shader_desc.res_desc[i].name);
						BOOST_ASSERT(p);

						uint32_t offset = shader_desc.res_desc[i].bind_point;
						if (D3D_SIT_SAMPLER == shader_desc.res_desc[i].type)
						{
							SamplerStateObjectPtr sampler;
							p->Value(sampler);
							if (sampler)
							{
								auto const& sampler_desc = checked_cast<D3D12SamplerStateObject&>(*sampler).D3DDesc();
								device->CreateSampler(&sampler_desc, cpu_sampler_handle);
								cpu_sampler_handle.ptr += sampler_desc_size;
							}
						}
						else
						{
							param_binds_[stage].push_back(this->GetBindFunc(srv_starts[stage], uav_starts[stage], offset, *p));
						}
					}
				}

				if (!shader_stage->CBufferIndices().empty())
				{
					auto const& cbuff_indices = shader_stage->CBufferIndices();

					all_cbuff_indices.insert(all_cbuff_indices.end(), cbuff_indices.begin(), cbuff_indices.end());
					for (size_t i = 0; i < cbuff_indices.size(); ++i)
					{
						auto cbuff = effect.CBufferByIndex(cbuff_indices[i]);
						cbuff->Resize(shader_desc.cb_desc[i].size);
						BOOST_ASSERT(cbuff->NumParameters() == shader_desc.cb_desc[i].var_desc.size());
						for (uint32_t j = 0; j < cbuff->NumParameters(); ++j)
						{
							RenderEffectParameter* param = effect.ParameterByIndex(cbuff->ParameterIndex(j));
							uint32_t stride;
							if (param->Type() == REDT_struct)
							{
								stride = 1;
							}
							else if (shader_desc.cb_desc[i].var_desc[j].elements > 0)
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
							param->BindToCBuffer(effect, cbuff_indices[i], shader_desc.cb_desc[i].var_desc[j].start_offset, stride);
						}
					}
				}
			}
		}
	}

	ShaderObjectPtr D3D12ShaderObject::Clone(RenderEffect& dst_effect)
	{
		D3D12ShaderObjectPtr ret = MakeSharedPtr<D3D12ShaderObject>(immutable_, d3d_immutable_);

		ret->hw_res_ready_ = hw_res_ready_;

		ret->srv_uav_handles_.resize(srv_uav_handles_.size());
		ret->srv_uav_srcs_.resize(srv_uav_srcs_.size(), std::make_tuple(static_cast<D3D12Resource*>(nullptr), 0, 0));

		uint32_t srv_starts[NumShaderStages + 1];
		uint32_t uav_starts[NumShaderStages + 1];
		srv_starts[0] = uav_starts[0] = 0;
		for (uint32_t stage = 1; stage <= NumShaderStages; ++stage)
		{
			srv_starts[stage] = srv_starts[stage - 1] + d3d_immutable_->num_srvs_[stage - 1];
			uav_starts[stage] = uav_starts[stage - 1] + d3d_immutable_->num_uavs_[stage - 1];
		}

		std::vector<uint32_t> all_cbuff_indices;
		for (size_t i = 0; i < NumShaderStages; ++ i)
		{
			ret->param_binds_[i].reserve(param_binds_[i].size());
			for (auto const & pb : param_binds_[i])
			{
				ret->param_binds_[i].push_back(
					ret->GetBindFunc(srv_starts[i], uav_starts[i], pb.offset, *dst_effect.ParameterByName(pb.param->Name())));
			}
		}

		return ret;
	}

	D3D12ShaderObject::ParameterBind D3D12ShaderObject::GetBindFunc(
		uint32_t srv_stage_base, uint32_t uav_stage_base, uint32_t offset, RenderEffectParameter const& param)
	{
		ParameterBind ret;
		ret.param = &param;
		ret.offset = offset;

		uint32_t const srv_offset = srv_stage_base + offset;
		uint32_t const uav_offset = d3d_immutable_->num_total_srvs_ + uav_stage_base + offset;

		switch (param.Type())
		{
		case REDT_bool:
		case REDT_uint:
		case REDT_int:
		case REDT_float:
		case REDT_uint2:
		case REDT_uint3:
		case REDT_uint4:
		case REDT_int2:
		case REDT_int3:
		case REDT_int4:
		case REDT_float2:
		case REDT_float3:
		case REDT_float4:
		case REDT_float4x4:
		case REDT_sampler:
			break;

		case REDT_texture1D:
		case REDT_texture2D:
		case REDT_texture2DMS:
		case REDT_texture3D:
		case REDT_textureCUBE:
		case REDT_texture1DArray:
		case REDT_texture2DArray:
		case REDT_texture2DMSArray:
		case REDT_texture3DArray:
		case REDT_textureCUBEArray:
		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
		case REDT_byte_address_buffer:
			ret.update = D3D12ShaderParameterSrvUpdater(srv_uav_srcs_[srv_offset], srv_uav_handles_[srv_offset], param);
			break;

		case REDT_rw_texture1D:
		case REDT_rw_texture2D:
		case REDT_rw_texture3D:
		case REDT_rw_texture1DArray:
		case REDT_rw_texture2DArray:
		case REDT_rasterizer_ordered_texture1D:
		case REDT_rasterizer_ordered_texture1DArray:
		case REDT_rasterizer_ordered_texture2D:
		case REDT_rasterizer_ordered_texture2DArray:
		case REDT_rasterizer_ordered_texture3D:
		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_rw_byte_address_buffer:
		case REDT_rasterizer_ordered_buffer:
		case REDT_rasterizer_ordered_structured_buffer:
		case REDT_rasterizer_ordered_byte_address_buffer:
			ret.update = D3D12ShaderParameterUavUpdater(srv_uav_srcs_[uav_offset], srv_uav_handles_[uav_offset], param);
			break;

		default:
			KFL_UNREACHABLE("Can't be called");
		}

		return ret;
	}

	void D3D12ShaderObject::Bind(RenderEffect const& effect)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* cmd_list = re.D3DRenderCmdList();

		for (size_t stage = 0; stage < NumShaderStages; ++stage)
		{
			for (auto const & pb : param_binds_[stage])
			{
				pb.update();
			}
		}

		D3D12_RESOURCE_STATES const srv_state_after = D3D12_RESOURCE_STATE_GENERIC_READ;
		D3D12_RESOURCE_STATES const uav_state_after = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		for (uint32_t i = 0; i < srv_uav_srcs_.size(); ++i)
		{
			auto const state_after = i < d3d_immutable_->num_total_srvs_ ? srv_state_after : uav_state_after;
			auto res = std::get<0>(srv_uav_srcs_[i]);
			if (res != nullptr)
			{
				for (uint32_t subres = 0; subres < std::get<2>(srv_uav_srcs_[i]); ++subres)
				{
					res->UpdateResourceBarrier(cmd_list, std::get<1>(srv_uav_srcs_[i]) + subres, state_after);
				}
			}
		}

		re.FlushResourceBarriers(cmd_list);

		for (size_t stage = 0; stage < NumShaderStages; ++stage)
		{
			auto const* shader_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(static_cast<ShaderStage>(stage)).get());
			if (shader_stage)
			{
				auto const& cbuff_indices = shader_stage->CBufferIndices();
				for (auto cb_index : cbuff_indices)
				{
					auto* cb = effect.CBufferByIndex(cb_index);
					cb->Update();
				}
			}
		}

		if (!srv_uav_handles_.empty())
		{
			re.RenewDynamicCbvSrvUavDescBlock(srv_uav_desc_block_, static_cast<uint32_t>(srv_uav_handles_.size()));

			auto const cpu_cbv_srv_uav_handle = srv_uav_desc_block_.CpuHandle();
			uint32_t num_srvs_uavs = static_cast<uint32_t>(srv_uav_handles_.size());
			re.D3DDevice()->CopyDescriptors(1, &cpu_cbv_srv_uav_handle, &num_srvs_uavs, num_srvs_uavs, srv_uav_handles_.data(), nullptr,
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}

	void D3D12ShaderObject::Unbind()
	{
	}

	uint32_t D3D12ShaderObject::NumCBuffers(ShaderStage stage) const noexcept
	{
		auto const* shader_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(static_cast<ShaderStage>(stage)).get());
		if (shader_stage)
		{
			return static_cast<uint32_t>(shader_stage->CBufferIndices().size());
		}
		return 0;
	}

	D3D12_GPU_VIRTUAL_ADDRESS D3D12ShaderObject::CBufferGpuVAddr(
		RenderEffect const& effect, ShaderStage stage, uint32_t index) const noexcept
	{
		auto const* shader_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(static_cast<ShaderStage>(stage)).get());
		if (shader_stage)
		{
			return checked_cast<D3D12GraphicsBuffer&>(*effect.CBufferByIndex(shader_stage->CBufferIndices()[index])->HWBuff()).GpuVirtualAddress();
		}
		return 0;
	}

	void D3D12ShaderObject::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		pso_desc.pRootSignature = d3d_immutable_->root_signature_.get();

		pso_desc.StreamOutput.pSODeclaration = nullptr;
		pso_desc.StreamOutput.NumEntries = 0;
		pso_desc.StreamOutput.pBufferStrides = nullptr;
		pso_desc.StreamOutput.NumStrides = 0;
		pso_desc.StreamOutput.RasterizedStream = 0;

		{
			auto const* ps_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(ShaderStage::Pixel).get());
			if (ps_stage)
			{
				ps_stage->UpdatePsoDesc(pso_desc);
			}
			else
			{
				pso_desc.PS.pShaderBytecode = nullptr;
				pso_desc.PS.BytecodeLength = 0;
			}
		}
		{
			auto const* gs_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(ShaderStage::Geometry).get());
			if (gs_stage)
			{
				gs_stage->UpdatePsoDesc(pso_desc);
			}
			else
			{
				pso_desc.GS.pShaderBytecode = nullptr;
				pso_desc.GS.BytecodeLength = 0;
			}
		}
		{
			auto const* ds_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(ShaderStage::Domain).get());
			if (ds_stage)
			{
				ds_stage->UpdatePsoDesc(pso_desc);
			}
			else
			{
				pso_desc.DS.pShaderBytecode = nullptr;
				pso_desc.DS.BytecodeLength = 0;
			}
		}
		{
			auto const* hs_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(ShaderStage::Hull).get());
			if (hs_stage)
			{
				hs_stage->UpdatePsoDesc(pso_desc);
			}
			else
			{
				pso_desc.HS.pShaderBytecode = nullptr;
				pso_desc.HS.BytecodeLength = 0;
			}
		}
		{
			auto const* vs_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(ShaderStage::Vertex).get());
			if (vs_stage)
			{
				vs_stage->UpdatePsoDesc(pso_desc);
			}
			else
			{
				pso_desc.VS.pShaderBytecode = nullptr;
				pso_desc.VS.BytecodeLength = 0;
			}
		}
	}

	void D3D12ShaderObject::UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const noexcept
	{
		pso_desc.pRootSignature = d3d_immutable_->root_signature_.get();

		auto const* cs_stage = checked_cast<D3D12ShaderStageObject*>(this->Stage(ShaderStage::Compute).get());
		if (cs_stage)
		{
			cs_stage->UpdatePsoDesc(pso_desc);
		}
		else
		{
			pso_desc.CS.pShaderBytecode = nullptr;
			pso_desc.CS.BytecodeLength = 0;
		}
	}

	D3D12ShaderObject::D3D12Immutable::~D3D12Immutable()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeallocSamplerDescBlock(std::move(sampler_desc_block_));
		}
	}
}
