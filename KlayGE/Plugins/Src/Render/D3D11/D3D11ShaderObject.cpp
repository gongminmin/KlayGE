// D3D11ShaderObject.cpp
// KlayGE D3D11 shader对象类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 加速Shader编译 (2009.7.31)
//
// 3.8.0
// 支持Gemoetry Shader (2009.2.5)
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

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

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>

namespace
{
	using namespace KlayGE;

	class SetD3D11ShaderParameterTextureSRV
	{
	public:
		SetD3D11ShaderParameterTextureSRV(std::tuple<void*, uint32_t, uint32_t>& srvsrc,
				ID3D11ShaderResourceView*& srv, RenderEffectParameter* param)
			: srvsrc_(&srvsrc), srv_(&srv), param_(param)
		{
		}

		void operator()()
		{
			TextureSubresource tex_subres;
			param_->Value(tex_subres);
			if (tex_subres.tex)
			{
				*srvsrc_ = std::make_tuple(tex_subres.tex.get(),
					tex_subres.first_array_index * tex_subres.tex->NumMipMaps() + tex_subres.first_level,
					tex_subres.num_items * tex_subres.num_levels);
				*srv_ = checked_cast<D3D11Texture*>(tex_subres.tex.get())->RetriveD3DShaderResourceView(
					tex_subres.first_array_index, tex_subres.num_items,
					tex_subres.first_level, tex_subres.num_levels).get();
			}
			else
			{
				std::get<0>(*srvsrc_) = nullptr;
			}
		}

	private:
		std::tuple<void*, uint32_t, uint32_t>* srvsrc_;
		ID3D11ShaderResourceView** srv_;
		RenderEffectParameter* param_;
	};

	class SetD3D11ShaderParameterGraphicsBufferSRV
	{
	public:
		SetD3D11ShaderParameterGraphicsBufferSRV(std::tuple<void*, uint32_t, uint32_t>& srvsrc,
				ID3D11ShaderResourceView*& srv, RenderEffectParameter* param)
			: srvsrc_(&srvsrc), srv_(&srv), param_(param)
		{
		}

		void operator()()
		{
			GraphicsBufferPtr buf;
			param_->Value(buf);
			if (buf)
			{
				*srvsrc_ = std::make_tuple(buf.get(), 0, 1);
				*srv_ = checked_cast<D3D11GraphicsBuffer*>(buf.get())->D3DShaderResourceView().get();
			}
			else
			{
				std::get<0>(*srvsrc_) = nullptr;
			}
		}

	private:
		std::tuple<void*, uint32_t, uint32_t>* srvsrc_;
		ID3D11ShaderResourceView** srv_;
		RenderEffectParameter* param_;
	};

	class SetD3D11ShaderParameterTextureUAV
	{
	public:
		SetD3D11ShaderParameterTextureUAV(void*& uavsrc, ID3D11UnorderedAccessView*& uav, RenderEffectParameter* param)
			: uavsrc_(&uavsrc), uav_(&uav), param_(param)
		{
		}

		void operator()()
		{
			TextureSubresource tex_subres;
			param_->Value(tex_subres);
			if (tex_subres.tex)
			{
				*uavsrc_ = tex_subres.tex.get();
				*uav_ = checked_cast<D3D11Texture*>(tex_subres.tex.get())->RetriveD3DUnorderedAccessView(
					tex_subres.first_array_index, tex_subres.num_items, tex_subres.first_level).get();
			}
			else
			{
				*uavsrc_ = nullptr;
			}
		}

	private:
		void** uavsrc_;
		ID3D11UnorderedAccessView** uav_;
		RenderEffectParameter* param_;
	};

	class SetD3D11ShaderParameterGraphicsBufferUAV
	{
	public:
		SetD3D11ShaderParameterGraphicsBufferUAV(void*& uavsrc, ID3D11UnorderedAccessView*& uav, RenderEffectParameter* param)
			: uavsrc_(&uavsrc), uav_(&uav), param_(param)
		{
		}

		void operator()()
		{
			GraphicsBufferPtr buf;
			param_->Value(buf);
			if (buf)
			{
				*uavsrc_ = buf.get();
				*uav_ = checked_cast<D3D11GraphicsBuffer*>(buf.get())->D3DUnorderedAccessView().get();
			}
			else
			{
				*uavsrc_ = nullptr;
			}
		}

	private:
		void** uavsrc_;
		ID3D11UnorderedAccessView** uav_;
		RenderEffectParameter* param_;
	};
}

namespace KlayGE
{
	D3D11ShaderObject::D3D11ShaderObject()
		: D3D11ShaderObject(MakeSharedPtr<D3D11ShaderObjectTemplate>())
	{
	}

	D3D11ShaderObject::D3D11ShaderObject(std::shared_ptr<D3D11ShaderObjectTemplate> const & so_template)
		: so_template_(so_template)
	{
		has_discard_ = true;
		has_tessellation_ = false;
		is_shader_validate_.fill(true);
	}

	std::string_view D3D11ShaderObject::GetShaderProfile(ShaderType type, RenderEffect const & effect, uint32_t shader_desc_id)
	{
		auto const & sd = effect.GetShaderDesc(shader_desc_id);
		auto const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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

	bool D3D11ShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect,
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
			std::string& profile = so_template_->shader_code_[type].second;
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

				so_template_->shader_desc_[type] = MakeSharedPtr<D3D11ShaderObjectTemplate::D3D11ShaderDesc>();
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

	bool D3D11ShaderObject::StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
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

	void D3D11ShaderObject::StreamOut(std::ostream& os, ShaderType type)
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

	std::shared_ptr<std::vector<uint8_t>> D3D11ShaderObject::CompiteToBytecode(ShaderType type,
		RenderEffect const & effect, RenderTechnique const & tech, RenderPass const & pass,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		auto const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto const & caps = re.DeviceCaps();

		auto const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

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
			macros.emplace_back("KLAYGE_D3D11", "1");
			macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");
			if (caps.uav_format_support(EF_ABGR16F))
			{
				macros.emplace_back("KLAYGE_TYPED_UAV_SUPPORT", "1");
			}

			uint32_t flags = D3DCOMPILE_ENABLE_STRICTNESS;
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
					if (!so_template_->shader_desc_[type])
					{
						so_template_->shader_desc_[type] = MakeSharedPtr<D3D11ShaderObjectTemplate::D3D11ShaderDesc>();
					}

					D3D11_SHADER_DESC desc;
					reflection->GetDesc(&desc);

					for (UINT c = 0; c < desc.ConstantBuffers; ++ c)
					{
						ID3D11ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

						D3D11_SHADER_BUFFER_DESC d3d_cb_desc;
						reflection_cb->GetDesc(&d3d_cb_desc);
						if ((D3D_CT_CBUFFER == d3d_cb_desc.Type) || (D3D_CT_TBUFFER == d3d_cb_desc.Type))
						{
							D3D11ShaderObjectTemplate::D3D11ShaderDesc::ConstantBufferDesc cb_desc;
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

								D3D11ShaderObjectTemplate::D3D11ShaderDesc::ConstantBufferDesc::VariableDesc vd;
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

					so_template_->shader_desc_[type]->num_samplers = static_cast<uint16_t>(max_sampler_bind_pt + 1);
					so_template_->shader_desc_[type]->num_srvs = static_cast<uint16_t>(max_srv_bind_pt + 1);
					so_template_->shader_desc_[type]->num_uavs = static_cast<uint16_t>(max_uav_bind_pt + 1);

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
								D3D11ShaderObjectTemplate::D3D11ShaderDesc::BoundResourceDesc brd;
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
						D3D11_SIGNATURE_PARAMETER_DESC signature;
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

	void D3D11ShaderObject::CreateGeometryShaderWithStreamOutput(ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob,
		std::vector<ShaderDesc::StreamOutputDecl> const & so_decl)
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto const & d3d11_re = *checked_cast<D3D11RenderEngine const *>(&rf.RenderEngineInstance());
		auto d3d_device = d3d11_re.D3DDevice();
		auto const & caps = d3d11_re.DeviceCaps();

		std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(so_decl.size());
		for (size_t i = 0; i < so_decl.size(); ++ i)
		{
			d3d11_decl[i] = D3D11Mapping::Mapping(so_decl[i]);
		}

		UINT rasterized_stream = 0;
		if ((caps.max_shader_model >= ShaderModel(5, 0))
			&& (effect.GetShaderDesc(shader_desc_ids[ShaderObject::ST_PixelShader]).func_name.empty()))
		{
			rasterized_stream = D3D11_SO_NO_RASTERIZED_STREAM;
		}

		ID3D11GeometryShader* gs;
		if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(&((*code_blob)[0]), code_blob->size(),
			&d3d11_decl[0], static_cast<UINT>(d3d11_decl.size()), nullptr, 0, rasterized_stream, nullptr,
			&gs)))
		{
			is_shader_validate_[type] = false;
		}
		else
		{
			so_template_->geometry_shader_ = MakeCOMPtr(gs);
		}
	}

	void D3D11ShaderObject::AttachShaderBytecode(ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob)
	{
		if (code_blob)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&rf.RenderEngineInstance());
			ID3D11Device* d3d_device = re.D3DDevice();
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
				switch (type)
				{
				case ST_VertexShader:
					{
						ID3D11VertexShader* vs;
						if (FAILED(d3d_device->CreateVertexShader(&((*code_blob)[0]), code_blob->size(), nullptr, &vs)))
						{
							is_shader_validate_[type] = false;
						}
						else
						{
							so_template_->vertex_shader_ = MakeCOMPtr(vs);

							if (!sd.so_decl.empty())
							{
								if (caps.gs_support)
								{
									this->CreateGeometryShaderWithStreamOutput(type, effect, shader_desc_ids, code_blob,
										sd.so_decl);
								}
								else
								{
									is_shader_validate_[type] = false;
								}
							}

							so_template_->shader_code_[type].first = code_blob;
						}
					}
					break;

				case ST_PixelShader:
					{
						ID3D11PixelShader* ps;
						if (FAILED(d3d_device->CreatePixelShader(&((*code_blob)[0]), code_blob->size(), nullptr, &ps)))
						{
							is_shader_validate_[type] = false;
						}
						else
						{
							so_template_->pixel_shader_ = MakeCOMPtr(ps);
							so_template_->shader_code_[type].first = code_blob;
						}
					}
					break;

				case ST_GeometryShader:
					if (caps.gs_support)
					{
						if (sd.so_decl.empty())
						{
							ID3D11GeometryShader* gs;
							if (FAILED(d3d_device->CreateGeometryShader(&((*code_blob)[0]), code_blob->size(), nullptr, &gs)))
							{
								is_shader_validate_[type] = false;
							}
							else
							{
								so_template_->geometry_shader_ = MakeCOMPtr(gs);
								so_template_->shader_code_[type].first = code_blob;
							}
						}
						else
						{
							this->CreateGeometryShaderWithStreamOutput(type, effect, shader_desc_ids, code_blob,
								sd.so_decl);

							if (is_shader_validate_[type])
							{
								so_template_->shader_code_[type].first = code_blob;
							}
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
						ID3D11ComputeShader* cs;
						if (FAILED(d3d_device->CreateComputeShader(&((*code_blob)[0]), code_blob->size(), nullptr, &cs)))
						{
							is_shader_validate_[type] = false;
						}
						else
						{
							so_template_->compute_shader_ = MakeCOMPtr(cs);
							so_template_->shader_code_[type].first = code_blob;
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
						ID3D11HullShader* hs;
						if (FAILED(d3d_device->CreateHullShader(&((*code_blob)[0]), code_blob->size(), nullptr, &hs)))
						{
							is_shader_validate_[type] = false;
						}
						else
						{
							so_template_->hull_shader_ = MakeCOMPtr(hs);
							so_template_->shader_code_[type].first = code_blob;
							has_tessellation_ = true;
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
						ID3D11DomainShader* ds;
						if (FAILED(d3d_device->CreateDomainShader(&((*code_blob)[0]), code_blob->size(), nullptr, &ds)))
						{
							is_shader_validate_[type] = false;
						}
						else
						{
							so_template_->domain_shader_ = MakeCOMPtr(ds);

							if (!sd.so_decl.empty())
							{
								if (caps.gs_support)
								{
									this->CreateGeometryShaderWithStreamOutput(type, effect, shader_desc_ids, code_blob,
										sd.so_decl);
								}
								else
								{
									is_shader_validate_[type] = false;
								}
							}

							so_template_->shader_code_[type].first = code_blob;
							has_tessellation_ = true;
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
			}

			// Shader reflection
			if (!so_template_->shader_desc_[type]->cb_desc.empty())
			{
				so_template_->cbuff_indices_[type] = MakeSharedPtr<std::vector<uint8_t>>(so_template_->shader_desc_[type]->cb_desc.size());
			}
			d3d11_cbuffs_[type].resize(so_template_->shader_desc_[type]->cb_desc.size());
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
			srvsrcs_[type].resize(so_template_->shader_desc_[type]->num_srvs, std::make_tuple(static_cast<void*>(nullptr), 0, 0));
			srvs_[type].resize(so_template_->shader_desc_[type]->num_srvs);
			uavsrcs_.resize(so_template_->shader_desc_[type]->num_uavs, nullptr);
			uavs_.resize(so_template_->shader_desc_[type]->num_uavs);

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
						samplers_[type][offset] = checked_cast<D3D11SamplerStateObject*>(sampler.get())->D3DSamplerState();
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

	void D3D11ShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
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
			so_template_->shader_code_[type] = so.so_template_->shader_code_[type];
			so_template_->shader_desc_[type] = so.so_template_->shader_desc_[type];
			switch (type)
			{
			case ST_VertexShader:
				so_template_->vertex_shader_ = so.so_template_->vertex_shader_;
				so_template_->vs_signature_ = so.so_template_->vs_signature_;
				so_template_->geometry_shader_ = so.so_template_->geometry_shader_;
				break;

			case ST_PixelShader:
				so_template_->pixel_shader_ = so.so_template_->pixel_shader_;
				break;

			case ST_GeometryShader:
				so_template_->geometry_shader_ = so.so_template_->geometry_shader_;
				break;

			case ST_ComputeShader:
				so_template_->compute_shader_ = so.so_template_->compute_shader_;
				cs_block_size_x_ = so.cs_block_size_x_;
				cs_block_size_y_ = so.cs_block_size_y_;
				cs_block_size_z_ = so.cs_block_size_z_;
				break;

			case ST_HullShader:
				so_template_->hull_shader_ = so.so_template_->hull_shader_;
				if (so_template_->hull_shader_)
				{
					has_tessellation_ = true;
				}
				break;

			case ST_DomainShader:
				so_template_->domain_shader_ = so.so_template_->domain_shader_;
				so_template_->geometry_shader_ = so.so_template_->geometry_shader_;
				if (so_template_->domain_shader_)
				{
					has_tessellation_ = true;
				}
				break;

			default:
				is_shader_validate_[type] = false;
				break;
			}

			samplers_[type] = so.samplers_[type];
			srvsrcs_[type].resize(so.srvs_[type].size(), std::make_tuple(static_cast<void*>(nullptr), 0, 0));
			srvs_[type].resize(so.srvs_[type].size());
			if (so.so_template_->shader_desc_[type]->num_uavs > 0)
			{
				uavsrcs_.resize(so.uavs_.size(), nullptr);
				uavs_.resize(so.uavs_.size());
			}

			so_template_->cbuff_indices_[type] = so.so_template_->cbuff_indices_[type];
			d3d11_cbuffs_[type].resize(so.d3d11_cbuffs_[type].size());

			param_binds_[type].reserve(so.param_binds_[type].size());
			for (auto const & pb : so.param_binds_[type])
			{
				param_binds_[type].push_back(this->GetBindFunc(type, pb.offset, pb.param));
			}
		}
	}

	void D3D11ShaderObject::LinkShaders(RenderEffect const & effect)
	{
		std::vector<uint32_t> all_cbuff_indices;
		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
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

					d3d11_cbuffs_[type][i] = checked_cast<D3D11GraphicsBuffer*>(cbuff->HWBuff().get())->D3DBuffer();
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
	}

	ShaderObjectPtr D3D11ShaderObject::Clone(RenderEffect const & effect)
	{
		D3D11ShaderObjectPtr ret = MakeSharedPtr<D3D11ShaderObject>(so_template_);

		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->cs_block_size_x_ = cs_block_size_x_;
		ret->cs_block_size_y_ = cs_block_size_y_;
		ret->cs_block_size_z_ = cs_block_size_z_;
		ret->uavsrcs_.resize(uavsrcs_.size(), nullptr);
		ret->uavs_.resize(uavs_.size());

		std::vector<uint32_t> all_cbuff_indices;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->samplers_[i] = samplers_[i];
			ret->srvsrcs_[i].resize(srvsrcs_[i].size(), std::make_tuple(static_cast<void*>(nullptr), 0, 0));
			ret->srvs_[i].resize(srvs_[i].size());

			if (so_template_->cbuff_indices_[i] && !so_template_->cbuff_indices_[i]->empty())
			{
				ret->d3d11_cbuffs_[i].resize(d3d11_cbuffs_[i].size());
				all_cbuff_indices.insert(all_cbuff_indices.end(),
					so_template_->cbuff_indices_[i]->begin(), so_template_->cbuff_indices_[i]->end());
				for (size_t j = 0; j < so_template_->cbuff_indices_[i]->size(); ++ j)
				{
					auto cbuff = effect.CBufferByIndex((*so_template_->cbuff_indices_[i])[j]);
					ret->d3d11_cbuffs_[i][j] = checked_cast<D3D11GraphicsBuffer*>(cbuff->HWBuff().get())->D3DBuffer();
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

		return ret;
	}

	D3D11ShaderObject::ParameterBind D3D11ShaderObject::GetBindFunc(ShaderType type, uint32_t offset, RenderEffectParameter* param)
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
			ret.func = SetD3D11ShaderParameterTextureSRV(srvsrcs_[type][offset], srvs_[type][offset], param);
			break;

		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
		case REDT_byte_address_buffer:
			ret.func = SetD3D11ShaderParameterGraphicsBufferSRV(srvsrcs_[type][offset], srvs_[type][offset], param);
			break;

		case REDT_rw_texture1D:
		case REDT_rw_texture2D:
		case REDT_rw_texture3D:
		case REDT_rw_texture1DArray:
		case REDT_rw_texture2DArray:
			ret.func = SetD3D11ShaderParameterTextureUAV(uavsrcs_[offset], uavs_[offset], param);
			break;

		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_rw_byte_address_buffer:
			ret.func = SetD3D11ShaderParameterGraphicsBufferUAV(uavsrcs_[offset], uavs_[offset], param);
			break;

		default:
			KFL_UNREACHABLE("Invalid type");
		}

		return ret;
	}

	void D3D11ShaderObject::Bind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		re.VSSetShader(so_template_->vertex_shader_.get());
		re.GSSetShader(so_template_->geometry_shader_.get());
		re.PSSetShader(so_template_->pixel_shader_.get());
		re.CSSetShader(so_template_->compute_shader_.get());
		re.HSSetShader(so_template_->hull_shader_.get());
		re.DSSetShader(so_template_->domain_shader_.get());

		for (auto const & pbs : param_binds_)
		{
			for (auto const & pb : pbs)
			{
				pb.func();
			}
		}

		for (auto cb : all_cbuffs_)
		{
			cb->Update();
		}

		for (size_t st = 0; st < ST_NumShaderTypes; ++ st)
		{
			if (!srvs_[st].empty())
			{
				re.SetShaderResources(static_cast<ShaderObject::ShaderType>(st), srvsrcs_[st], srvs_[st]);
			}

			if (!samplers_[st].empty())
			{
				re.SetSamplers(static_cast<ShaderObject::ShaderType>(st), samplers_[st]);
			}

			if (!d3d11_cbuffs_[st].empty())
			{
				re.SetConstantBuffers(static_cast<ShaderObject::ShaderType>(st), d3d11_cbuffs_[st]);
			}
		}

		if (so_template_->compute_shader_ && !uavs_.empty())
		{
			for (uint32_t i = 0; i < uavs_.size(); ++ i)
			{
				if (uavsrcs_[i] != nullptr)
				{
					re.DetachSRV(uavsrcs_[i], 0, 1);
				}
			}

			std::vector<UINT> uav_init_counts(uavs_.size(), 0);
			re.CSSetUnorderedAccessViews(0, static_cast<UINT>(uavs_.size()), &uavs_[0], &uav_init_counts[0]);
		}
	}

	void D3D11ShaderObject::Unbind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		if (so_template_->compute_shader_ && !uavs_.empty())
		{
			std::vector<ID3D11UnorderedAccessView*> uavs(uavs_.size(), nullptr);
			std::vector<UINT> uav_init_counts(uavs_.size(), 0);
			re.CSSetUnorderedAccessViews(0, static_cast<UINT>(uavs.size()), &uavs[0], &uav_init_counts[0]);
		}
	}
}
