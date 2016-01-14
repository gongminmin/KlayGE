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
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/ResIdentifier.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/functional/hash.hpp>

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
				ID3D11ShaderResourceView*& srv, RenderEffectParameterPtr const & param)
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
		RenderEffectParameterPtr param_;
	};

	class SetD3D11ShaderParameterGraphicsBufferSRV
	{
	public:
		SetD3D11ShaderParameterGraphicsBufferSRV(std::tuple<void*, uint32_t, uint32_t>& srvsrc,
				ID3D11ShaderResourceView*& srv, RenderEffectParameterPtr const & param)
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
		RenderEffectParameterPtr param_;
	};

	class SetD3D11ShaderParameterTextureUAV
	{
	public:
		SetD3D11ShaderParameterTextureUAV(void*& uavsrc, ID3D11UnorderedAccessView*& uav, RenderEffectParameterPtr const & param)
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
		RenderEffectParameterPtr param_;
	};

	class SetD3D11ShaderParameterGraphicsBufferUAV
	{
	public:
		SetD3D11ShaderParameterGraphicsBufferUAV(void*& uavsrc, ID3D11UnorderedAccessView*& uav, RenderEffectParameterPtr const & param)
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
		RenderEffectParameterPtr param_;
	};
}

namespace KlayGE
{
	D3D11ShaderObject::D3D11ShaderObject()
	{
		has_discard_ = true;
		has_tessellation_ = false;
		is_shader_validate_.fill(true);
	}

	std::string D3D11ShaderObject::GetShaderProfile(ShaderType type, RenderEffect const & effect, uint32_t shader_desc_id)
	{
		ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_id);
		D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderDeviceCaps const & caps = render_eng.DeviceCaps();
		std::string shader_profile = sd.profile;
		size_t const shader_profile_hash = RT_HASH(shader_profile.c_str());
		switch (type)
		{
		case ST_VertexShader:
			if (CT_HASH("auto") == shader_profile_hash)
			{
				shader_profile = render_eng.VertexShaderProfile();
			}
			break;

		case ST_PixelShader:
			if (CT_HASH("auto") == shader_profile_hash)
			{
				shader_profile = render_eng.PixelShaderProfile();
			}
			break;

		case ST_GeometryShader:
			if (caps.gs_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = render_eng.GeometryShaderProfile();
				}
			}
			break;

		case ST_ComputeShader:
			if (caps.cs_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = render_eng.ComputeShaderProfile();
				}
			}
			break;

		case ST_HullShader:
			if (caps.hs_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = render_eng.HullShaderProfile();
				}
			}
			break;

		case ST_DomainShader:
			if (caps.ds_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = render_eng.DomainShaderProfile();
				}
			}
			break;

		default:
			break;
		}

		return shader_profile;
	}

	bool D3D11ShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
		std::vector<uint8_t> const & native_shader_block)
	{
		bool ret = false;

		is_shader_validate_[type] = false;
		std::string shader_profile = this->GetShaderProfile(type, effect, shader_desc_ids[type]);
		if (native_shader_block.size() >= 25 + shader_profile.size())
		{
			uint8_t const * nsbp = &native_shader_block[0];

			uint8_t len = *nsbp;
			++ nsbp;
			std::string& profile = shader_code_[type].second;
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

				shader_desc_[type] = MakeSharedPtr<D3D11ShaderDesc>();
				D3D11ShaderDesc& sd = *shader_desc_[type];

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
					std::memcpy(&vs_signature_, nsbp, sizeof(vs_signature_));
					nsbp += sizeof(vs_signature_);
					vs_signature_ = LE2Native(vs_signature_);
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
		std::vector<uint32_t> const & shader_desc_ids)
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

			D3D11ShaderDesc const & sd = *shader_desc_[type];

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
		D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		D3D_FEATURE_LEVEL feature_level = render_eng.DeviceFeatureLevel();
		RenderDeviceCaps const & caps = render_eng.DeviceCaps();

		ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		is_shader_validate_[type] = true;

		char const * shader_profile = sd.profile.c_str();
		size_t const shader_profile_hash = RT_HASH(shader_profile);
		switch (type)
		{
		case ST_VertexShader:
			if (CT_HASH("auto") == shader_profile_hash)
			{
				shader_profile = render_eng.VertexShaderProfile();
			}
			break;

		case ST_PixelShader:
			if (CT_HASH("auto") == shader_profile_hash)
			{
				shader_profile = render_eng.PixelShaderProfile();
			}
			break;

		case ST_GeometryShader:
			if (caps.gs_support)
			{
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = render_eng.GeometryShaderProfile();
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
					shader_profile = render_eng.ComputeShaderProfile();
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
					shader_profile = render_eng.HullShaderProfile();
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
					shader_profile = render_eng.DomainShaderProfile();
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
					if (!shader_desc_[type])
					{
						shader_desc_[type] = MakeSharedPtr<D3D11ShaderDesc>();
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

							shader_desc_[type]->cb_desc.push_back(cb_desc);
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

					shader_desc_[type]->num_samplers = static_cast<uint16_t>(num_samplers + 1);
					shader_desc_[type]->num_srvs = static_cast<uint16_t>(num_srvs + 1);
					shader_desc_[type]->num_uavs = static_cast<uint16_t>(num_uavs + 1);

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
								shader_desc_[type]->res_desc.push_back(brd);
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
							vertex_shader_ = MakeCOMPtr(vs);

							if (!sd.so_decl.empty())
							{
								if (caps.gs_support)
								{
									std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(sd.so_decl.size());
									for (size_t i = 0; i < sd.so_decl.size(); ++ i)
									{
										d3d11_decl[i] = D3D11Mapping::Mapping(sd.so_decl[i], static_cast<uint8_t>(i));
									}

									UINT rasterized_stream = 0;
									if ((caps.max_shader_model >= ShaderModel(5, 0)) && (effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty()))
									{
										rasterized_stream = D3D11_SO_NO_RASTERIZED_STREAM;
									}

									ID3D11GeometryShader* gs;
									if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(&((*code_blob)[0]), code_blob->size(),
										&d3d11_decl[0], static_cast<UINT>(d3d11_decl.size()), 0, 0, rasterized_stream, nullptr, &gs)))
									{
										is_shader_validate_[type] = false;
									}
									else
									{
										geometry_shader_ = MakeCOMPtr(gs);
									}
								}
								else
								{
									is_shader_validate_[type] = false;
								}
							}

							shader_code_[type].first = code_blob;
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
							pixel_shader_ = MakeCOMPtr(ps);
							shader_code_[type].first = code_blob;
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
								geometry_shader_ = MakeCOMPtr(gs);
								shader_code_[type].first = code_blob;
							}
						}
						else
						{
							std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(sd.so_decl.size());
							for (size_t i = 0; i < sd.so_decl.size(); ++ i)
							{
								d3d11_decl[i] = D3D11Mapping::Mapping(sd.so_decl[i], static_cast<uint8_t>(i));
							}

							UINT rasterized_stream = 0;
							if ((caps.max_shader_model >= ShaderModel(5, 0)) && (effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty()))
							{
								rasterized_stream = D3D11_SO_NO_RASTERIZED_STREAM;
							}

							ID3D11GeometryShader* gs;
							if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(&((*code_blob)[0]), code_blob->size(),
								&d3d11_decl[0], static_cast<UINT>(d3d11_decl.size()), 0, 0, rasterized_stream, nullptr, &gs)))
							{
								is_shader_validate_[type] = false;
							}
							else
							{
								geometry_shader_ = MakeCOMPtr(gs);
								shader_code_[type].first = code_blob;
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
							compute_shader_ = MakeCOMPtr(cs);
							shader_code_[type].first = code_blob;
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
							hull_shader_ = MakeCOMPtr(hs);
							shader_code_[type].first = code_blob;
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
							domain_shader_ = MakeCOMPtr(ds);

							if (!sd.so_decl.empty())
							{
								if (caps.gs_support)
								{
									std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(sd.so_decl.size());
									for (size_t i = 0; i < sd.so_decl.size(); ++ i)
									{
										d3d11_decl[i] = D3D11Mapping::Mapping(sd.so_decl[i], static_cast<uint8_t>(i));
									}

									UINT rasterized_stream = 0;
									if ((caps.max_shader_model >= ShaderModel(5, 0)) && (effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty()))
									{
										rasterized_stream = D3D11_SO_NO_RASTERIZED_STREAM;
									}

									ID3D11GeometryShader* gs;
									if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(&((*code_blob)[0]), code_blob->size(),
										&d3d11_decl[0], static_cast<UINT>(d3d11_decl.size()), 0, 0, rasterized_stream, nullptr, &gs)))
									{
										is_shader_validate_[type] = false;
									}
									else
									{
										geometry_shader_ = MakeCOMPtr(gs);
									}
								}
								else
								{
									is_shader_validate_[type] = false;
								}
							}

							shader_code_[type].first = code_blob;
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
			if (!shader_desc_[type]->cb_desc.empty())
			{
				cbuff_indices_[type] = MakeSharedPtr<std::vector<uint8_t>>(shader_desc_[type]->cb_desc.size());
			}
			d3d11_cbuffs_[type].resize(shader_desc_[type]->cb_desc.size());
			for (size_t c = 0; c < shader_desc_[type]->cb_desc.size(); ++ c)
			{
				uint32_t i = 0;
				for (; i < effect.NumCBuffers(); ++ i)
				{
					if (effect.CBufferByIndex(i)->NameHash() == shader_desc_[type]->cb_desc[c].name_hash)
					{
						(*cbuff_indices_[type])[c] = static_cast<uint8_t>(i);
						break;
					}
				}
				BOOST_ASSERT(i < effect.NumCBuffers());
			}

			samplers_[type].resize(shader_desc_[type]->num_samplers);
			srvsrcs_[type].resize(shader_desc_[type]->num_srvs, std::make_tuple(static_cast<void*>(nullptr), 0, 0));
			srvs_[type].resize(shader_desc_[type]->num_srvs);
			uavsrcs_[type].resize(shader_desc_[type]->num_uavs, nullptr);
			uavs_[type].resize(shader_desc_[type]->num_uavs);

			for (size_t i = 0; i < shader_desc_[type]->res_desc.size(); ++ i)
			{
				RenderEffectParameterPtr const & p = effect.ParameterByName(shader_desc_[type]->res_desc[i].name);
				BOOST_ASSERT(p);

				D3D11ShaderParameterHandle p_handle;
				p_handle.shader_type = static_cast<uint8_t>(type);
				p_handle.cbuff = 0;
				p_handle.offset = shader_desc_[type]->res_desc[i].bind_point;
				p_handle.elements = 1;
				p_handle.rows = 0;
				p_handle.columns = 1;
				if (D3D_SIT_SAMPLER == shader_desc_[type]->res_desc[i].type)
				{
					p_handle.param_type = D3D_SVT_SAMPLER;

					SamplerStateObjectPtr sampler;
					p->Value(sampler);
					if (sampler)
					{
						samplers_[p_handle.shader_type][p_handle.offset]
							= checked_cast<D3D11SamplerStateObject*>(sampler.get())->D3DSamplerState();
					}
				}
				else
				{
					if (D3D_SRV_DIMENSION_BUFFER == shader_desc_[type]->res_desc[i].dimension)
					{
						p_handle.param_type = D3D_SVT_BUFFER;
					}
					else
					{
						p_handle.param_type = D3D_SVT_TEXTURE;
					}

					param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
				}
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
				vertex_shader_ = so.vertex_shader_;
				vs_signature_ = so.vs_signature_;
				geometry_shader_ = so.geometry_shader_;
				break;

			case ST_PixelShader:
				pixel_shader_ = so.pixel_shader_;
				break;

			case ST_GeometryShader:
				geometry_shader_ = so.geometry_shader_;
				break;

			case ST_ComputeShader:
				compute_shader_ = so.compute_shader_;
				cs_block_size_x_ = so.cs_block_size_x_;
				cs_block_size_y_ = so.cs_block_size_y_;
				cs_block_size_z_ = so.cs_block_size_z_;
				break;

			case ST_HullShader:
				hull_shader_ = so.hull_shader_;
				if (hull_shader_)
				{
					has_tessellation_ = true;
				}
				break;

			case ST_DomainShader:
				domain_shader_ = so.domain_shader_;
				geometry_shader_ = so.geometry_shader_;
				if (domain_shader_)
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
			uavsrcs_[type].resize(so.uavs_[type].size(), nullptr);
			uavs_[type].resize(so.uavs_[type].size());

			cbuff_indices_[type] = so.cbuff_indices_[type];
			d3d11_cbuffs_[type].resize(so.d3d11_cbuffs_[type].size());

			param_binds_[type].reserve(so.param_binds_[type].size());
			for (auto const & pb : so.param_binds_[type])
			{
				param_binds_[type].push_back(this->GetBindFunc(pb.p_handle, pb.param));
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

			if (cbuff_indices_[type] && !cbuff_indices_[type]->empty())
			{
				all_cbuff_indices.insert(all_cbuff_indices.end(),
					cbuff_indices_[type]->begin(), cbuff_indices_[type]->end());
				for (size_t i = 0; i < cbuff_indices_[type]->size(); ++ i)
				{
					RenderEffectConstantBufferPtr const & cbuff = effect.CBufferByIndex((*cbuff_indices_[type])[i]);
					cbuff->Resize(shader_desc_[type]->cb_desc[i].size);
					BOOST_ASSERT(cbuff->NumParameters() == shader_desc_[type]->cb_desc[i].var_desc.size());
					for (uint32_t j = 0; j < cbuff->NumParameters(); ++ j)
					{
						RenderEffectParameterPtr const & param = effect.ParameterByIndex(cbuff->ParameterIndex(j));
						uint32_t stride;
						if (shader_desc_[type]->cb_desc[i].var_desc[j].elements > 0)
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
						param->BindToCBuffer(cbuff, shader_desc_[type]->cb_desc[i].var_desc[j].start_offset, stride);
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
		D3D11ShaderObjectPtr ret = MakeSharedPtr<D3D11ShaderObject>();
		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->vertex_shader_ = vertex_shader_;
		ret->pixel_shader_ = pixel_shader_;
		ret->geometry_shader_ = geometry_shader_;
		ret->compute_shader_ = compute_shader_;
		ret->hull_shader_ = hull_shader_;
		ret->domain_shader_ = domain_shader_;
		ret->vs_signature_ = vs_signature_;
		ret->cs_block_size_x_ = cs_block_size_x_;
		ret->cs_block_size_y_ = cs_block_size_y_;
		ret->cs_block_size_z_ = cs_block_size_z_;

		std::vector<uint32_t> all_cbuff_indices;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->shader_code_[i] = shader_code_[i];
			ret->shader_desc_[i] = shader_desc_[i];

			ret->samplers_[i] = samplers_[i];
			ret->srvsrcs_[i].resize(srvsrcs_[i].size(), std::make_tuple(static_cast<void*>(nullptr), 0, 0));
			ret->srvs_[i].resize(srvs_[i].size());
			ret->uavsrcs_[i].resize(uavsrcs_[i].size(), nullptr);
			ret->uavs_[i].resize(uavs_[i].size());

			ret->cbuff_indices_[i] = cbuff_indices_[i];
			if (cbuff_indices_[i] && !cbuff_indices_[i]->empty())
			{
				ret->d3d11_cbuffs_[i].resize(d3d11_cbuffs_[i].size());
				all_cbuff_indices.insert(all_cbuff_indices.end(), cbuff_indices_[i]->begin(), cbuff_indices_[i]->end());
				for (size_t j = 0; j < cbuff_indices_[i]->size(); ++ j)
				{
					RenderEffectConstantBufferPtr const & cbuff = effect.CBufferByIndex((*cbuff_indices_[i])[j]);
					ret->d3d11_cbuffs_[i][j] = checked_cast<D3D11GraphicsBuffer*>(cbuff->HWBuff().get())->D3DBuffer();
				}
			}

			ret->param_binds_[i].reserve(param_binds_[i].size());
			for (auto const & pb : param_binds_[i])
			{
				ret->param_binds_[i].push_back(ret->GetBindFunc(pb.p_handle, effect.ParameterByName(pb.param->Name())));
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

	D3D11ShaderObject::parameter_bind_t D3D11ShaderObject::GetBindFunc(D3D11ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		std::memcpy(&ret.p_handle, &p_handle, sizeof(p_handle));

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
		case REDT_texture3D:
		case REDT_textureCUBE:
		case REDT_texture1DArray:
		case REDT_texture2DArray:
		case REDT_texture3DArray:
		case REDT_textureCUBEArray:
			ret.func = SetD3D11ShaderParameterTextureSRV(srvsrcs_[p_handle.shader_type][p_handle.offset], srvs_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
		case REDT_byte_address_buffer:
			ret.func = SetD3D11ShaderParameterGraphicsBufferSRV(srvsrcs_[p_handle.shader_type][p_handle.offset], srvs_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_rw_texture1D:
		case REDT_rw_texture2D:
		case REDT_rw_texture3D:
		case REDT_rw_texture1DArray:
		case REDT_rw_texture2DArray:
			ret.func = SetD3D11ShaderParameterTextureUAV(uavsrcs_[p_handle.shader_type][p_handle.offset], uavs_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_rw_byte_address_buffer:
			ret.func = SetD3D11ShaderParameterGraphicsBufferUAV(uavsrcs_[p_handle.shader_type][p_handle.offset], uavs_[p_handle.shader_type][p_handle.offset], param);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void D3D11ShaderObject::Bind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext* d3d_imm_ctx = re.D3DDeviceImmContext();

		re.VSSetShader(vertex_shader_.get());
		re.GSSetShader(geometry_shader_.get());
		re.PSSetShader(pixel_shader_.get());
		re.CSSetShader(compute_shader_.get());
		re.HSSetShader(hull_shader_.get());
		re.DSSetShader(domain_shader_.get());

		for (size_t st = 0; st < ST_NumShaderTypes; ++ st)
		{
			for (auto const & pb : param_binds_[st])
			{
				pb.func();
			}
		}

		for (size_t i = 0; i < all_cbuffs_.size(); ++ i)
		{
			all_cbuffs_[i]->Update();
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

		if (!uavs_[ST_ComputeShader].empty())
		{
			std::vector<ID3D11UnorderedAccessView*> uavs_ptr(uavs_[ST_ComputeShader].size());
			for (uint32_t i = 0; i < uavs_[ST_ComputeShader].size(); ++ i)
			{
				if (uavsrcs_[ST_ComputeShader][i] != nullptr)
				{
					re.DetachSRV(uavsrcs_[ST_ComputeShader][i], 0, 1);
				}

				uavs_ptr[i] = uavs_[ST_ComputeShader][i];
			}

			d3d_imm_ctx->CSSetUnorderedAccessViews(0, static_cast<UINT>(uavs_[ST_ComputeShader].size()), &uavs_ptr[0],
				reinterpret_cast<UINT*>(&uavs_ptr[0]));
		}
	}

	void D3D11ShaderObject::Unbind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext* d3d_imm_ctx = re.D3DDeviceImmContext();

		if (!uavs_[ST_ComputeShader].empty())
		{
			std::vector<ID3D11UnorderedAccessView*> uavs(uavs_[ST_ComputeShader].size(), nullptr);
			d3d_imm_ctx->CSSetUnorderedAccessViews(0, static_cast<UINT>(uavs_[ST_ComputeShader].size()), &uavs[0],
				reinterpret_cast<UINT*>(&uavs[0]));
		}
	}
}
