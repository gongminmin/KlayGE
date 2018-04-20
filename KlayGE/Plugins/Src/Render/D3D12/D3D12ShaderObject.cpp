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
#include <KFL/COMPtr.hpp>
#include <KFL/ResIdentifier.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KFL/Hash.hpp>

#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <boost/assert.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <KlayGE/SALWrapper.hpp>
#include <d3dcompiler.h>
#endif

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12InterfaceLoader.hpp>
#include <KlayGE/D3D12/D3D12ShaderObject.hpp>

namespace
{
	using namespace KlayGE;

	class SetD3D12ShaderParameterTextureSRV
	{
	public:
		SetD3D12ShaderParameterTextureSRV(std::tuple<D3D12Resource*, uint32_t, uint32_t>& srvsrc,
				D3D12ShaderResourceViewSimulation*& srv, RenderEffectParameter* param)
			: srvsrc_(&srvsrc), srv_(&srv), param_(param)
		{
		}

		void operator()()
		{
			TextureSubresource tex_subres;
			param_->Value(tex_subres);
			if (tex_subres.tex)
			{
				*srvsrc_ = std::make_tuple(checked_cast<D3D12Texture*>(tex_subres.tex.get()),
					tex_subres.first_array_index * tex_subres.tex->NumMipMaps() + tex_subres.first_level,
					tex_subres.num_items * tex_subres.num_levels);
				*srv_ = checked_cast<D3D12Texture*>(tex_subres.tex.get())->RetriveD3DShaderResourceView(
					tex_subres.first_array_index, tex_subres.num_items,
					tex_subres.first_level, tex_subres.num_levels).get();
			}
			else
			{
				std::get<0>(*srvsrc_) = nullptr;
			}
		}

	private:
		std::tuple<D3D12Resource*, uint32_t, uint32_t>* srvsrc_;
		D3D12ShaderResourceViewSimulation** srv_;
		RenderEffectParameter* param_;
	};

	class SetD3D12ShaderParameterGraphicsBufferSRV
	{
	public:
		SetD3D12ShaderParameterGraphicsBufferSRV(std::tuple<D3D12Resource*, uint32_t, uint32_t>& srvsrc,
				D3D12ShaderResourceViewSimulation*& srv, RenderEffectParameter* param)
			: srvsrc_(&srvsrc), srv_(&srv), param_(param)
		{
		}

		void operator()()
		{
			GraphicsBufferPtr buf;
			param_->Value(buf);
			if (buf)
			{
				*srvsrc_ = std::make_tuple(checked_cast<D3D12GraphicsBuffer*>(buf.get()), 0, 1);
				*srv_ = checked_cast<D3D12GraphicsBuffer*>(buf.get())->D3DShaderResourceView().get();
			}
			else
			{
				std::get<0>(*srvsrc_) = nullptr;
			}
		}

	private:
		std::tuple<D3D12Resource*, uint32_t, uint32_t>* srvsrc_;
		D3D12ShaderResourceViewSimulation** srv_;
		RenderEffectParameter* param_;
	};

	class SetD3D12ShaderParameterTextureUAV
	{
	public:
		SetD3D12ShaderParameterTextureUAV(std::pair<D3D12Resource*, ID3D12Resource*>& uavsrc,
				D3D12UnorderedAccessViewSimulation*& uav, RenderEffectParameter* param)
			: uavsrc_(&uavsrc), uav_(&uav), param_(param)
		{
		}

		void operator()()
		{
			TextureSubresource tex_subres;
			param_->Value(tex_subres);
			if (tex_subres.tex)
			{
				uavsrc_->first = checked_cast<D3D12Texture*>(tex_subres.tex.get());
				uavsrc_->second = nullptr; // TODO
				*uav_ = checked_cast<D3D12Texture*>(tex_subres.tex.get())->RetriveD3DUnorderedAccessView(
					tex_subres.first_array_index, tex_subres.num_items, tex_subres.first_level).get();
			}
			else
			{
				uavsrc_->first = nullptr;
				uavsrc_->second = nullptr;
			}
		}

	private:
		std::pair<D3D12Resource*, ID3D12Resource*>* uavsrc_;
		D3D12UnorderedAccessViewSimulation** uav_;
		RenderEffectParameter* param_;
	};

	class SetD3D12ShaderParameterGraphicsBufferUAV
	{
	public:
		SetD3D12ShaderParameterGraphicsBufferUAV(std::pair<D3D12Resource*, ID3D12Resource*>& uavsrc,
				D3D12UnorderedAccessViewSimulation*& uav, RenderEffectParameter* const & param)
			: uavsrc_(&uavsrc), uav_(&uav), param_(param)
		{
		}

		void operator()()
		{
			GraphicsBufferPtr buf;
			param_->Value(buf);
			if (buf)
			{
				uavsrc_->first = checked_cast<D3D12GraphicsBuffer*>(buf.get());
				uavsrc_->second = checked_cast<D3D12GraphicsBuffer*>(buf.get())->D3DBufferCounterUpload().get();
				*uav_ = checked_cast<D3D12GraphicsBuffer*>(buf.get())->D3DUnorderedAccessView().get();
			}
			else
			{
				uavsrc_->first = nullptr;
				uavsrc_->second = nullptr;
			}
		}

	private:
		std::pair<D3D12Resource*, ID3D12Resource*>* uavsrc_;
		D3D12UnorderedAccessViewSimulation** uav_;
		RenderEffectParameter* param_;
	};
}

namespace KlayGE
{
	D3D12ShaderObject::D3D12ShaderObject()
		: D3D12ShaderObject(MakeSharedPtr<D3D12ShaderObjectTemplate>())
	{
	}

	D3D12ShaderObject::D3D12ShaderObject(std::shared_ptr<D3D12ShaderObjectTemplate> const & so_template)
		: so_template_(so_template)
	{
		has_discard_ = true;
		has_tessellation_ = false;
		is_shader_validate_.fill(true);
	}

	std::string_view D3D12ShaderObject::GetShaderProfile(ShaderType type, RenderEffect const & effect, uint32_t shader_desc_id)
	{
		auto const & sd = effect.GetShaderDesc(shader_desc_id);
		auto const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto const & caps = re.DeviceCaps();
		std::string_view shader_profile = sd.profile;
		size_t const shader_profile_hash = HashRange(shader_profile.begin(), shader_profile.end());
		switch (type)
		{
		case ST_VertexShader:
			if (CT_HASH("auto") == shader_profile_hash)
			{
				shader_profile = re.VertexShaderProfile();
			}
			break;

		case ST_PixelShader:
			if (CT_HASH("auto") == shader_profile_hash)
			{
				shader_profile = re.PixelShaderProfile();
			}
			break;

		case ST_GeometryShader:
			if (caps.gs_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = re.GeometryShaderProfile();
				}
			}
			break;

		case ST_ComputeShader:
			if (caps.cs_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = re.ComputeShaderProfile();
				}
			}
			break;

		case ST_HullShader:
			if (caps.hs_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = re.HullShaderProfile();
				}
			}
			break;

		case ST_DomainShader:
			if (caps.ds_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = re.DomainShaderProfile();
				}
			}
			break;

		default:
			break;
		}

		return shader_profile;
	}

	bool D3D12ShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::vector<uint8_t> const & native_shader_block)
	{
		bool ret = false;

		is_shader_validate_[type] = false;
		std::string_view shader_profile = this->GetShaderProfile(type, effect, shader_desc_ids[type]);
		if (native_shader_block.size() >= 25 + shader_profile.size())
		{
			uint8_t const * nsbp = &native_shader_block[0];

			uint8_t len = *nsbp;
			++ nsbp;
			std::string profile = so_template_->shader_code_[type].second;
			profile.resize(len);
			std::memcpy(&profile[0], nsbp, len);
			nsbp += len;
			if (profile == shader_profile)
			{
				is_shader_validate_[type] = true;

				uint32_t blob_size;
				std::memcpy(&blob_size, nsbp, sizeof(blob_size));
				nsbp += sizeof(blob_size);
				std::shared_ptr<std::vector<uint8_t>> code_blob = MakeSharedPtr<std::vector<uint8_t>>(blob_size);

				std::memcpy(&((*code_blob)[0]), nsbp, blob_size);
				nsbp += blob_size;

				so_template_->shader_desc_[type] = MakeSharedPtr<D3D12ShaderObjectTemplate::D3D12ShaderDesc>();
				auto& sd = *so_template_->shader_desc_[type];

				uint16_t cb_desc_size;
				std::memcpy(&cb_desc_size, nsbp, sizeof(cb_desc_size));
				nsbp += sizeof(cb_desc_size);
				cb_desc_size = LE2Native(cb_desc_size);
				sd.cb_desc.resize(cb_desc_size);
				for (size_t i = 0; i < sd.cb_desc.size(); ++ i)
				{
					len = *nsbp;
					++ nsbp;
					sd.cb_desc[i].name.resize(len);
					std::memcpy(&sd.cb_desc[i].name[0], nsbp, len);
					nsbp += len;

					sd.cb_desc[i].name_hash = RT_HASH(sd.cb_desc[i].name.c_str());

					std::memcpy(&sd.cb_desc[i].size, nsbp, sizeof(sd.cb_desc[i].size));
					nsbp += sizeof(sd.cb_desc[i].size);
					sd.cb_desc[i].size = LE2Native(sd.cb_desc[i].size);

					uint16_t var_desc_size;
					std::memcpy(&var_desc_size, nsbp, sizeof(var_desc_size));
					nsbp += sizeof(var_desc_size);
					var_desc_size = LE2Native(var_desc_size);
					sd.cb_desc[i].var_desc.resize(var_desc_size);
					for (size_t j = 0; j < sd.cb_desc[i].var_desc.size(); ++ j)
					{
						len = *nsbp;
						++ nsbp;
						sd.cb_desc[i].var_desc[j].name.resize(len);
						std::memcpy(&sd.cb_desc[i].var_desc[j].name[0], nsbp, len);
						nsbp += len;

						std::memcpy(&sd.cb_desc[i].var_desc[j].start_offset, nsbp, sizeof(sd.cb_desc[i].var_desc[j].start_offset));
						nsbp += sizeof(sd.cb_desc[i].var_desc[j].start_offset);
						sd.cb_desc[i].var_desc[j].start_offset = LE2Native(sd.cb_desc[i].var_desc[j].start_offset);
						std::memcpy(&sd.cb_desc[i].var_desc[j].type, nsbp, sizeof(sd.cb_desc[i].var_desc[j].type));
						nsbp += sizeof(sd.cb_desc[i].var_desc[j].type);
						std::memcpy(&sd.cb_desc[i].var_desc[j].rows, nsbp, sizeof(sd.cb_desc[i].var_desc[j].rows));
						nsbp += sizeof(sd.cb_desc[i].var_desc[j].rows);
						std::memcpy(&sd.cb_desc[i].var_desc[j].columns, nsbp, sizeof(sd.cb_desc[i].var_desc[j].columns));
						nsbp += sizeof(sd.cb_desc[i].var_desc[j].columns);
						std::memcpy(&sd.cb_desc[i].var_desc[j].elements, nsbp, sizeof(sd.cb_desc[i].var_desc[j].elements));
						nsbp += sizeof(sd.cb_desc[i].var_desc[j].elements);
						sd.cb_desc[i].var_desc[j].elements = LE2Native(sd.cb_desc[i].var_desc[j].elements);
					}
				}

				std::memcpy(&sd.num_samplers, nsbp, sizeof(sd.num_samplers));
				nsbp += sizeof(sd.num_samplers);
				sd.num_samplers = LE2Native(sd.num_samplers);
				std::memcpy(&sd.num_srvs, nsbp, sizeof(sd.num_srvs));
				nsbp += sizeof(sd.num_srvs);
				sd.num_srvs = LE2Native(sd.num_srvs);
				std::memcpy(&sd.num_uavs, nsbp, sizeof(sd.num_uavs));
				nsbp += sizeof(sd.num_uavs);
				sd.num_uavs = LE2Native(sd.num_uavs);

				uint16_t res_desc_size;
				std::memcpy(&res_desc_size, nsbp, sizeof(res_desc_size));
				nsbp += sizeof(res_desc_size);
				res_desc_size = LE2Native(res_desc_size);
				sd.res_desc.resize(res_desc_size);
				for (size_t i = 0; i < sd.res_desc.size(); ++ i)
				{
					len = *nsbp;
					++ nsbp;
					sd.res_desc[i].name.resize(len);
					std::memcpy(&sd.res_desc[i].name[0], nsbp, len);
					nsbp += len;

					std::memcpy(&sd.res_desc[i].type, nsbp, sizeof(sd.res_desc[i].type));
					nsbp += sizeof(sd.res_desc[i].type);

					std::memcpy(&sd.res_desc[i].dimension, nsbp, sizeof(sd.res_desc[i].dimension));
					nsbp += sizeof(sd.res_desc[i].dimension);

					std::memcpy(&sd.res_desc[i].bind_point, nsbp, sizeof(sd.res_desc[i].bind_point));
					nsbp += sizeof(sd.res_desc[i].bind_point);
					sd.res_desc[i].bind_point = LE2Native(sd.res_desc[i].bind_point);
				}

				if (ST_VertexShader == type)
				{
					std::memcpy(&so_template_->vs_signature_, nsbp, sizeof(so_template_->vs_signature_));
					nsbp += sizeof(so_template_->vs_signature_);
					so_template_->vs_signature_ = LE2Native(so_template_->vs_signature_);
				}
				else if (ST_ComputeShader == type)
				{
					std::memcpy(&cs_block_size_x_, nsbp, sizeof(cs_block_size_x_));
					nsbp += sizeof(cs_block_size_x_);
					cs_block_size_x_ = LE2Native(cs_block_size_x_);

					std::memcpy(&cs_block_size_y_, nsbp, sizeof(cs_block_size_y_));
					nsbp += sizeof(cs_block_size_y_);
					cs_block_size_y_ = LE2Native(cs_block_size_y_);

					std::memcpy(&cs_block_size_z_, nsbp, sizeof(cs_block_size_z_));
					nsbp += sizeof(cs_block_size_z_);
					cs_block_size_z_ = LE2Native(cs_block_size_z_);
				}

				this->AttachShaderBytecode(type, effect, shader_desc_ids, code_blob);

				ret = is_shader_validate_[type];
			}
		}

		return ret;
	}

	bool D3D12ShaderObject::StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		uint32_t len;
		res->read(&len, sizeof(len));
		len = LE2Native(len);
		std::vector<uint8_t> native_shader_block(len);
		if (len > 0)
		{
			res->read(&native_shader_block[0], len * sizeof(native_shader_block[0]));
		}

		return this->AttachNativeShader(type, effect, shader_desc_ids, native_shader_block);
	}

	void D3D12ShaderObject::StreamOut(std::ostream& os, ShaderType type)
	{
		std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

		{
			uint8_t len = static_cast<uint8_t>(so_template_->shader_code_[type].second.size());
			oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
			oss.write(reinterpret_cast<char const *>(&so_template_->shader_code_[type].second[0]), len);
		}

		std::shared_ptr<std::vector<uint8_t>> code_blob = so_template_->shader_code_[type].first;
		if (code_blob)
		{
			uint8_t len;

			uint32_t blob_size = Native2LE(static_cast<uint32_t>(code_blob->size()));
			oss.write(reinterpret_cast<char const *>(&blob_size), sizeof(blob_size));
			oss.write(reinterpret_cast<char const *>(&((*code_blob)[0])), code_blob->size());

			auto const & sd = *so_template_->shader_desc_[type];

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
					oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].columns),
						sizeof(sd.cb_desc[i].var_desc[j].columns));
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
				uint32_t vs_signature = Native2LE(so_template_->vs_signature_);
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

	std::shared_ptr<std::vector<uint8_t>> D3D12ShaderObject::CompiteToBytecode(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		auto const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto const & caps = re.DeviceCaps();

		ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		is_shader_validate_[type] = true;

		char const * shader_profile = sd.profile.c_str();
		size_t const shader_profile_hash = RT_HASH(shader_profile);
		switch (type)
		{
		case ST_VertexShader:
			if (CT_HASH("auto") == shader_profile_hash)
			{
				shader_profile = re.VertexShaderProfile();
			}
			break;

		case ST_PixelShader:
			if (CT_HASH("auto") == shader_profile_hash)
			{
				shader_profile = re.PixelShaderProfile();
			}
			break;

		case ST_GeometryShader:
			if (caps.gs_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = re.GeometryShaderProfile();
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
					shader_profile = re.ComputeShaderProfile();
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
					shader_profile = re.HullShaderProfile();
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
					shader_profile = re.DomainShaderProfile();
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
		so_template_->shader_code_[type].second = shader_profile;

		std::shared_ptr<std::vector<uint8_t>> code = MakeSharedPtr<std::vector<uint8_t>>();
		if (is_shader_validate_[type])
		{
			std::vector<std::pair<char const *, char const *>> macros;
			macros.emplace_back("KLAYGE_D3D12", "1");
			macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");

			uint32_t flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if !defined(KLAYGE_DEBUG)
			flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			*code = this->CompileToDXBC(type, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile, flags);

			if (!code->empty())
			{
				ID3D12ShaderReflection* reflection;
				this->ReflectDXBC(*code, reinterpret_cast<void**>(&reflection));
				if (reflection != nullptr)
				{
					if (!so_template_->shader_desc_[type])
					{
						so_template_->shader_desc_[type] = MakeSharedPtr<D3D12ShaderObjectTemplate::D3D12ShaderDesc>();
					}

					D3D12_SHADER_DESC desc;
					reflection->GetDesc(&desc);

					for (UINT c = 0; c < desc.ConstantBuffers; ++ c)
					{
						ID3D12ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

						D3D12_SHADER_BUFFER_DESC d3d_cb_desc;
						reflection_cb->GetDesc(&d3d_cb_desc);
						if ((D3D_CT_CBUFFER == d3d_cb_desc.Type) || (D3D_CT_TBUFFER == d3d_cb_desc.Type))
						{
							D3D12ShaderObjectTemplate::D3D12ShaderDesc::ConstantBufferDesc cb_desc;
							cb_desc.name = d3d_cb_desc.Name;
							cb_desc.name_hash = RT_HASH(d3d_cb_desc.Name);
							cb_desc.size = d3d_cb_desc.Size;

							for (UINT v = 0; v < d3d_cb_desc.Variables; ++ v)
							{
								ID3D12ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

								D3D12_SHADER_VARIABLE_DESC var_desc;
								reflection_var->GetDesc(&var_desc);

								D3D12_SHADER_TYPE_DESC type_desc;
								reflection_var->GetType()->GetDesc(&type_desc);

								D3D12ShaderObjectTemplate::D3D12ShaderDesc::ConstantBufferDesc::VariableDesc vd;
								vd.name = var_desc.Name;
								vd.start_offset = var_desc.StartOffset;
								vd.type = static_cast<uint8_t>(type_desc.Type);
								vd.rows = static_cast<uint8_t>(type_desc.Rows);
								vd.columns = static_cast<uint8_t>(type_desc.Columns);
								vd.elements = static_cast<uint16_t>(type_desc.Elements);
								cb_desc.var_desc.push_back(vd);
							}

							so_template_->shader_desc_[type]->cb_desc.push_back(cb_desc);
						}
					}

					int max_sampler_bind_pt = -1;
					int max_srv_bind_pt = -1;
					int max_uav_bind_pt = -1;
					for (uint32_t i = 0; i < desc.BoundResources; ++ i)
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

					so_template_->shader_desc_[type]->num_samplers = static_cast<uint16_t>(max_sampler_bind_pt + 1);
					so_template_->shader_desc_[type]->num_srvs = static_cast<uint16_t>(max_srv_bind_pt + 1);
					so_template_->shader_desc_[type]->num_uavs = static_cast<uint16_t>(max_uav_bind_pt + 1);

					for (uint32_t i = 0; i < desc.BoundResources; ++ i)
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
								D3D12ShaderObjectTemplate::D3D12ShaderDesc::BoundResourceDesc brd;
								brd.name = si_desc.Name;
								brd.type = static_cast<uint8_t>(si_desc.Type);
								brd.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
								so_template_->shader_desc_[type]->res_desc.push_back(brd);
							}
							break;

						default:
							break;
						}
					}

					if (ST_VertexShader == type)
					{
						so_template_->vs_signature_ = 0;
						D3D12_SIGNATURE_PARAMETER_DESC signature;
						for (uint32_t i = 0; i < desc.InputParameters; ++ i)
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

							size_t sig = so_template_->vs_signature_;
							HashCombine(sig, seed);
							so_template_->vs_signature_ = static_cast<uint32_t>(sig);
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
			so_template_->shader_code_[type].second.clear();
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

	void D3D12ShaderObject::AttachShaderBytecode(ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob)
	{
		if (code_blob)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&rf.RenderEngineInstance());
			RenderDeviceCaps const & caps = re.DeviceCaps();

			ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

			uint8_t shader_major_ver = ("auto" == sd.profile) ? 0 : static_cast<uint8_t>(sd.profile[3] - '0');
			uint8_t shader_minor_ver = ("auto" == sd.profile) ? 0 : static_cast<uint8_t>(sd.profile[5] - '0');
			if (ShaderModel(shader_major_ver, shader_minor_ver) > caps.max_shader_model)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				so_template_->shader_code_[type].first = code_blob;

				switch (type)
				{
				case ST_VertexShader:
					if (caps.gs_support && !sd.so_decl.empty())
					{
						so_template_->so_decl_.resize(sd.so_decl.size());
						for (size_t i = 0; i < sd.so_decl.size(); ++ i)
						{
							so_template_->so_decl_[i] = D3D12Mapping::Mapping(sd.so_decl[i]);
						}

						so_template_->rasterized_stream_ = 0;
						if (effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty())
						{
							so_template_->rasterized_stream_ = D3D12_SO_NO_RASTERIZED_STREAM;
						}

						so_template_->vs_so_ = true;
					}
					else
					{
						so_template_->vs_so_ = false;
					}
					break;

				case ST_PixelShader:
					break;

				case ST_GeometryShader:
					if (caps.gs_support)
					{
						so_template_->so_decl_.resize(sd.so_decl.size());
						for (size_t i = 0; i < sd.so_decl.size(); ++ i)
						{
							so_template_->so_decl_[i] = D3D12Mapping::Mapping(sd.so_decl[i]);
						}

						so_template_->rasterized_stream_ = 0;
						if (effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty())
						{
							so_template_->rasterized_stream_ = D3D12_SO_NO_RASTERIZED_STREAM;
						}
					}
					else
					{
						is_shader_validate_[type] = false;
					}
					break;

				case ST_ComputeShader:
					if (!caps.cs_support)
					{
						is_shader_validate_[type] = false;
					}
					break;

				case ST_HullShader:
					if (caps.hs_support)
					{
						has_tessellation_ = true;
					}
					else
					{
						is_shader_validate_[type] = false;
					}
					break;

				case ST_DomainShader:
					so_template_->ds_so_ = false;
					if (caps.ds_support)
					{
						if (!sd.so_decl.empty())
						{
							if (caps.gs_support)
							{
								so_template_->so_decl_.resize(sd.so_decl.size());
								for (size_t i = 0; i < sd.so_decl.size(); ++ i)
								{
									so_template_->so_decl_[i] = D3D12Mapping::Mapping(sd.so_decl[i]);
								}

								so_template_->rasterized_stream_ = 0;
								if (effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty())
								{
									so_template_->rasterized_stream_ = D3D12_SO_NO_RASTERIZED_STREAM;
								}

								so_template_->ds_so_ = true;
							}
							else
							{
								is_shader_validate_[type] = false;
							}
						}

						has_tessellation_ = true;
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
			if (!so_template_->shader_desc_[type]->cb_desc.empty())
			{
				so_template_->cbuff_indices_[type] = MakeSharedPtr<std::vector<uint8_t>>(so_template_->shader_desc_[type]->cb_desc.size());
			}
			d3d_cbuffs_[type].resize(so_template_->shader_desc_[type]->cb_desc.size());
			for (size_t c = 0; c < so_template_->shader_desc_[type]->cb_desc.size(); ++ c)
			{
				uint32_t i = 0;
				for (; i < effect.NumCBuffers(); ++ i)
				{
					if (effect.CBufferByIndex(i)->NameHash() == so_template_->shader_desc_[type]->cb_desc[c].name_hash)
					{
						(*so_template_->cbuff_indices_[type])[c] = static_cast<uint8_t>(i);
						break;
					}
				}
				BOOST_ASSERT(i < effect.NumCBuffers());
			}

			samplers_[type].resize(so_template_->shader_desc_[type]->num_samplers);
			srvsrcs_[type].resize(so_template_->shader_desc_[type]->num_srvs,
				std::make_tuple(static_cast<D3D12Resource*>(nullptr), 0, 0));
			srvs_[type].resize(so_template_->shader_desc_[type]->num_srvs);
			uavsrcs_[type].resize(so_template_->shader_desc_[type]->num_uavs,
				std::make_pair(static_cast<D3D12Resource*>(nullptr), static_cast<ID3D12Resource*>(nullptr)));
			uavs_[type].resize(so_template_->shader_desc_[type]->num_uavs);

			for (size_t i = 0; i < so_template_->shader_desc_[type]->res_desc.size(); ++ i)
			{
				RenderEffectParameter* p = effect.ParameterByName(so_template_->shader_desc_[type]->res_desc[i].name);
				BOOST_ASSERT(p);

				uint32_t offset = so_template_->shader_desc_[type]->res_desc[i].bind_point;
				if (D3D_SIT_SAMPLER == so_template_->shader_desc_[type]->res_desc[i].type)
				{
					SamplerStateObjectPtr sampler;
					p->Value(sampler);
					if (sampler)
					{
						samplers_[type][offset] = checked_cast<D3D12SamplerStateObject*>(sampler.get())->D3DDesc();
					}
				}
				else
				{
					param_binds_[type].push_back(this->GetBindFunc(type, offset, p));
				}
			}
		}
		else
		{
			is_shader_validate_[type] = false;
		}
	}

	void D3D12ShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		std::shared_ptr<std::vector<uint8_t>> code_blob = this->CompiteToBytecode(type, effect, tech, pass, shader_desc_ids);
		this->AttachShaderBytecode(type, effect, shader_desc_ids, code_blob);
	}

	void D3D12ShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/,
			RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & shared_so)
	{
		if (shared_so)
		{
			D3D12ShaderObject const & so = *checked_cast<D3D12ShaderObject*>(shared_so.get());

			is_shader_validate_[type] = so.is_shader_validate_[type];
			if (is_shader_validate_[type])
			{
				so_template_->shader_code_[type] = so.so_template_->shader_code_[type];
				so_template_->shader_desc_[type] = so.so_template_->shader_desc_[type];
				switch (type)
				{
				case ST_VertexShader:
					so_template_->vs_signature_ = so.so_template_->vs_signature_;
					if (so.so_template_->vs_so_)
					{
						so_template_->shader_code_[ST_GeometryShader] = so.so_template_->shader_code_[ST_GeometryShader];
						so_template_->vs_so_ = so.so_template_->vs_so_;
						so_template_->so_decl_ = so.so_template_->so_decl_;
						so_template_->rasterized_stream_ = so.so_template_->rasterized_stream_;
					}
					break;

				case ST_PixelShader:
					break;

				case ST_GeometryShader:
					so_template_->so_decl_ = so.so_template_->so_decl_;
					so_template_->rasterized_stream_ = so.so_template_->rasterized_stream_;
					break;

				case ST_ComputeShader:
					cs_block_size_x_ = so.cs_block_size_x_;
					cs_block_size_y_ = so.cs_block_size_y_;
					cs_block_size_z_ = so.cs_block_size_z_;
					break;

				case ST_HullShader:
					if (!so_template_->shader_code_[ST_HullShader].first->empty())
					{
						has_tessellation_ = true;
					}
					break;

				case ST_DomainShader:
					if (!so_template_->shader_code_[ST_DomainShader].first->empty())
					{
						has_tessellation_ = true;

						if (so.so_template_->ds_so_)
						{
							so_template_->shader_code_[ST_GeometryShader] = so.so_template_->shader_code_[ST_GeometryShader];
							so_template_->ds_so_ = so.so_template_->ds_so_;
							so_template_->so_decl_ = so.so_template_->so_decl_;
							so_template_->rasterized_stream_ = so.so_template_->rasterized_stream_;
						}
					}
					break;

				default:
					is_shader_validate_[type] = false;
					break;
				}

				samplers_[type] = so.samplers_[type];
				srvsrcs_[type].resize(so.srvs_[type].size(),
					std::make_tuple(static_cast<D3D12Resource*>(nullptr), 0, 0));
				srvs_[type].resize(so.srvs_[type].size());
				uavsrcs_[type].resize(so.uavs_[type].size(),
					std::make_pair(static_cast<D3D12Resource*>(nullptr), static_cast<ID3D12Resource*>(nullptr)));
				uavs_[type].resize(so.uavs_[type].size());

				so_template_->cbuff_indices_[type] = so.so_template_->cbuff_indices_[type];
				d3d_cbuffs_[type].resize(so.d3d_cbuffs_[type].size());

				param_binds_[type].reserve(so.param_binds_[type].size());
				for (auto const & pb : so.param_binds_[type])
				{
					param_binds_[type].push_back(this->GetBindFunc(type, pb.offset, pb.param));
				}
			}
		}
	}

	void D3D12ShaderObject::LinkShaders(RenderEffect const & effect)
	{
		std::vector<uint32_t> all_cbuff_indices;
		is_validate_ = true;
		for (size_t type = 0; type < ST_NumShaderTypes; ++ type)
		{
			is_validate_ &= is_shader_validate_[type];

			if (so_template_->cbuff_indices_[type] && !so_template_->cbuff_indices_[type]->empty())
			{
				all_cbuff_indices.insert(all_cbuff_indices.end(),
					so_template_->cbuff_indices_[type]->begin(), so_template_->cbuff_indices_[type]->end());
				for (size_t i = 0; i < so_template_->cbuff_indices_[type]->size(); ++ i)
				{
					auto cbuff = effect.CBufferByIndex((*so_template_->cbuff_indices_[type])[i]);
					cbuff->Resize(so_template_->shader_desc_[type]->cb_desc[i].size);
					BOOST_ASSERT(cbuff->NumParameters() == so_template_->shader_desc_[type]->cb_desc[i].var_desc.size());
					for (uint32_t j = 0; j < cbuff->NumParameters(); ++ j)
					{
						RenderEffectParameter* param = effect.ParameterByIndex(cbuff->ParameterIndex(j));
						uint32_t stride;
						if (so_template_->shader_desc_[type]->cb_desc[i].var_desc[j].elements > 0)
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
						param->BindToCBuffer(*cbuff, so_template_->shader_desc_[type]->cb_desc[i].var_desc[j].start_offset, stride);
					}

					d3d_cbuffs_[type][i] = cbuff->HWBuff().get();
				}
			}
		}

		std::sort(all_cbuff_indices.begin(), all_cbuff_indices.end());
		all_cbuff_indices.erase(std::unique(all_cbuff_indices.begin(), all_cbuff_indices.end()),
			all_cbuff_indices.end());
		all_cbuffs_.resize(all_cbuff_indices.size());
		for (size_t i = 0; i < all_cbuff_indices.size(); ++ i)
		{
			all_cbuffs_[i] = effect.CBufferByIndex(all_cbuff_indices[i]);
		}

		this->CreateRootSignature();

		num_handles_ = 0;
		for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			num_handles_ += static_cast<uint32_t>(srvs_[i].size() + uavs_[i].size());
		}
	}

	void D3D12ShaderObject::CreateRootSignature()
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		std::array<uint32_t, ShaderObject::ST_NumShaderTypes * 4> num;
		uint32_t num_sampler = 0;
		for (uint32_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			num[i * 4 + 0] = static_cast<uint32_t>(d3d_cbuffs_[i].size());
			num[i * 4 + 1] = static_cast<uint32_t>(srvs_[i].size());
			num[i * 4 + 2] = static_cast<uint32_t>(uavs_[i].size());
			num[i * 4 + 3] = static_cast<uint32_t>(samplers_[i].size());

			num_sampler += num[i * 4 + 3];
		}

		so_template_->root_signature_ = re.CreateRootSignature(num, !!so_template_->shader_code_[ST_VertexShader].first,
			!so_template_->so_decl_.empty());

		if (num_sampler > 0)
		{
			D3D12_DESCRIPTOR_HEAP_DESC sampler_heap_desc;
			sampler_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			sampler_heap_desc.NumDescriptors = static_cast<UINT>(num_sampler);
			sampler_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			sampler_heap_desc.NodeMask = 0;
			ID3D12DescriptorHeap* s_heap;
			TIFHR(device->CreateDescriptorHeap(&sampler_heap_desc, IID_ID3D12DescriptorHeap, reinterpret_cast<void**>(&s_heap)));
			so_template_->sampler_heap_ = MakeCOMPtr(s_heap);

			UINT const sampler_desc_size = re.SamplerDescSize();
			D3D12_CPU_DESCRIPTOR_HANDLE cpu_sampler_handle = so_template_->sampler_heap_->GetCPUDescriptorHandleForHeapStart();
			for (uint32_t i = 0; i < ST_NumShaderTypes; ++ i)
			{
				if (!samplers_[i].empty())
				{
					for (uint32_t j = 0; j < samplers_[i].size(); ++ j)
					{
						device->CreateSampler(&samplers_[i][j], cpu_sampler_handle);
						cpu_sampler_handle.ptr += sampler_desc_size;
					}
				}
			}
		}
	}
	
	ShaderObjectPtr D3D12ShaderObject::Clone(RenderEffect const & effect)
	{
		D3D12ShaderObjectPtr ret = MakeSharedPtr<D3D12ShaderObject>();
		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->cs_block_size_x_ = cs_block_size_x_;
		ret->cs_block_size_y_ = cs_block_size_y_;
		ret->cs_block_size_z_ = cs_block_size_z_;
		ret->so_template_ = so_template_;

		std::vector<uint32_t> all_cbuff_indices;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->samplers_[i] = samplers_[i];
			ret->srvsrcs_[i].resize(srvsrcs_[i].size(),
				std::make_tuple(static_cast<D3D12Resource*>(nullptr), 0, 0));
			ret->srvs_[i].resize(srvs_[i].size());
			ret->uavsrcs_[i].resize(uavsrcs_[i].size(),
				std::make_pair(static_cast<D3D12Resource*>(nullptr), static_cast<ID3D12Resource*>(nullptr)));
			ret->uavs_[i].resize(uavs_[i].size());

			if (so_template_->cbuff_indices_[i] && !so_template_->cbuff_indices_[i]->empty())
			{
				ret->d3d_cbuffs_[i].resize(d3d_cbuffs_[i].size());
				all_cbuff_indices.insert(all_cbuff_indices.end(),
					so_template_->cbuff_indices_[i]->begin(), so_template_->cbuff_indices_[i]->end());
				for (size_t j = 0; j < so_template_->cbuff_indices_[i]->size(); ++ j)
				{
					auto cbuff = effect.CBufferByIndex((*so_template_->cbuff_indices_[i])[j]);
					ret->d3d_cbuffs_[i][j] = cbuff->HWBuff().get();
				}
			}

			ret->param_binds_[i].reserve(param_binds_[i].size());
			for (auto const & pb : param_binds_[i])
			{
				ret->param_binds_[i].push_back(ret->GetBindFunc(static_cast<ShaderType>(i), pb.offset,
					effect.ParameterByName(pb.param->Name())));
			}
		}

		std::sort(all_cbuff_indices.begin(), all_cbuff_indices.end());
		all_cbuff_indices.erase(std::unique(all_cbuff_indices.begin(), all_cbuff_indices.end()),
			all_cbuff_indices.end());
		ret->all_cbuffs_.resize(all_cbuff_indices.size());
		for (size_t i = 0; i < all_cbuff_indices.size(); ++ i)
		{
			ret->all_cbuffs_[i] = effect.CBufferByIndex(all_cbuff_indices[i]);
		}

		ret->num_handles_ = num_handles_;

		return ret;
	}

	D3D12ShaderObject::ParameterBind D3D12ShaderObject::GetBindFunc(ShaderType type, uint32_t offset, RenderEffectParameter* param)
	{
		ParameterBind ret;
		ret.param = param;
		ret.offset = offset;

		switch (param->Type())
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
			ret.func = SetD3D12ShaderParameterTextureSRV(srvsrcs_[type][offset], srvs_[type][offset], param);
			break;

		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
		case REDT_byte_address_buffer:
			ret.func = SetD3D12ShaderParameterGraphicsBufferSRV(srvsrcs_[type][offset], srvs_[type][offset], param);
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
			ret.func = SetD3D12ShaderParameterTextureUAV(uavsrcs_[type][offset], uavs_[type][offset], param);
			break;

		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_rw_byte_address_buffer:
		case REDT_rasterizer_ordered_buffer:
		case REDT_rasterizer_ordered_structured_buffer:
		case REDT_rasterizer_ordered_byte_address_buffer:
			ret.func = SetD3D12ShaderParameterGraphicsBufferUAV(uavsrcs_[type][offset], uavs_[type][offset], param);
			break;

		default:
			KFL_UNREACHABLE("Can't be called");
		}

		return ret;
	}

	void D3D12ShaderObject::Bind()
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		for (size_t st = 0; st < ST_NumShaderTypes; ++ st)
		{
			for (auto const & pb : param_binds_[st])
			{
				pb.func();
			}

			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			D3D12_RESOURCE_STATES state_after
				= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			for (auto const & srvsrc : srvsrcs_[st])
			{
				for (uint32_t subres = 0; subres < std::get<2>(srvsrc); ++ subres)
				{
					auto res = std::get<0>(srvsrc);
					if (res != nullptr)
					{
						if (res->UpdateResourceBarrier(std::get<1>(srvsrc) + subres, barrier, state_after))
						{
							barriers.push_back(barrier);
						}
					}
				}
			}

			state_after = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			for (auto const & uavsrc : uavsrcs_[st])
			{
				if (uavsrc.first != nullptr)
				{
#ifdef KLAYGE_DEBUG
					for (auto const & srvsrc : srvsrcs_[st])
					{
						BOOST_ASSERT(std::get<0>(srvsrc) != uavsrc.first);
#if defined(KLAYGE_COMPILER_CLANGC2)
						KFL_UNUSED(srvsrc);
#endif
					}
#endif

					if (uavsrc.first->UpdateResourceBarrier(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, barrier, state_after))
					{
						barriers.push_back(barrier);
					}
				}
			}
		}
		if (!barriers.empty())
		{
			auto& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.D3DRenderCmdList()->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
		}

		for (auto cb : all_cbuffs_)
		{
			cb->Update();
		}
	}

	void D3D12ShaderObject::Unbind()
	{
	}

	void D3D12ShaderObject::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc)
	{
		pso_desc.pRootSignature = so_template_->root_signature_.get();

		{
			auto blob = so_template_->shader_code_[ShaderObject::ST_VertexShader].first.get();
			if (blob)
			{
				pso_desc.VS.pShaderBytecode = blob->data();
				pso_desc.VS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.VS.pShaderBytecode = nullptr;
				pso_desc.VS.BytecodeLength = 0;
			}
		}
		{
			auto blob = so_template_->shader_code_[ShaderObject::ST_PixelShader].first.get();
			if (blob)
			{
				pso_desc.PS.pShaderBytecode = blob->data();
				pso_desc.PS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.PS.pShaderBytecode = nullptr;
				pso_desc.PS.BytecodeLength = 0;
			}
		}
		{
			auto blob = so_template_->shader_code_[ShaderObject::ST_DomainShader].first.get();
			if (blob)
			{
				pso_desc.DS.pShaderBytecode = blob->data();
				pso_desc.DS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.DS.pShaderBytecode = nullptr;
				pso_desc.DS.BytecodeLength = 0;
			}
		}
		{
			auto blob = so_template_->shader_code_[ShaderObject::ST_HullShader].first.get();
			if (blob)
			{
				pso_desc.HS.pShaderBytecode = blob->data();
				pso_desc.HS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.HS.pShaderBytecode = nullptr;
				pso_desc.HS.BytecodeLength = 0;
			}
		}
		{
			auto blob = so_template_->shader_code_[ShaderObject::ST_GeometryShader].first.get();
			if (blob)
			{
				pso_desc.GS.pShaderBytecode = blob->data();
				pso_desc.GS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.GS.pShaderBytecode = nullptr;
				pso_desc.GS.BytecodeLength = 0;
			}
		}

		auto const & so_decls = so_template_->so_decl_;
		pso_desc.StreamOutput.pSODeclaration = so_decls.data();
		pso_desc.StreamOutput.NumEntries = static_cast<UINT>(so_decls.size());
		pso_desc.StreamOutput.pBufferStrides = nullptr;
		pso_desc.StreamOutput.NumStrides = 0;
		pso_desc.StreamOutput.RasterizedStream = so_template_->rasterized_stream_;
	}

	void D3D12ShaderObject::UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc)
	{
		pso_desc.pRootSignature = so_template_->root_signature_.get();

		auto blob = so_template_->shader_code_[ShaderObject::ST_ComputeShader].first.get();
		if (blob)
		{
			pso_desc.CS.pShaderBytecode = blob->data();
			pso_desc.CS.BytecodeLength = static_cast<UINT>(blob->size());
		}
		else
		{
			pso_desc.CS.pShaderBytecode = nullptr;
			pso_desc.CS.BytecodeLength = 0;
		}
	}
}
