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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KFL/Hash.hpp>
#include <KFL/ResIdentifier.hpp>

#include <sstream>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

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
#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
	NullShaderObject::OGLShaderObjectTemplate::OGLShaderObjectTemplate()
		: gs_input_type_(0), gs_output_type_(0), gs_max_output_vertex_(0),
			ds_partitioning_(STP_Undefined), ds_output_primitive_(STOP_Undefined)
	{
	}
#endif

	NullShaderObject::NullShaderObject()
	{
		auto const & re = *checked_cast<NullRenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.NativeShaderPlatformName().find("d3d_11_") == 0)
		{
			so_template_ = MakeSharedPtr<D3D11ShaderObjectTemplate>();
			so_template_->as_d3d11_ = true;
		}
		else if (re.NativeShaderPlatformName().find("d3d_12_") == 0)
		{
			so_template_ = MakeSharedPtr<D3D11ShaderObjectTemplate>();
			so_template_->as_d3d12_ = true;
		}
#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
		else if (re.NativeShaderPlatformName().find("gl_") == 0)
		{
			so_template_ = MakeSharedPtr<OGLShaderObjectTemplate>();
			so_template_->as_gl_ = true;
		}
		else if (re.NativeShaderPlatformName().find("gles_") == 0)
		{
			so_template_ = MakeSharedPtr<OGLShaderObjectTemplate>();
			so_template_->as_gles_ = true;
		}
#endif

		has_discard_ = true;
		has_tessellation_ = false;
		is_shader_validate_.fill(false);
	}
	
	NullShaderObject::NullShaderObject(std::shared_ptr<NullShaderObjectTemplate> const & so_template)
		: so_template_(so_template)
	{
		has_discard_ = true;
		has_tessellation_ = false;
		is_shader_validate_.fill(false);
	}

	bool NullShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::vector<uint8_t> const & native_shader_block)
	{
		KFL_UNUSED(type);
		KFL_UNUSED(effect);
		KFL_UNUSED(shader_desc_ids);
		KFL_UNUSED(native_shader_block);
		return true;
	}

	bool NullShaderObject::StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		KFL_UNUSED(type);
		KFL_UNUSED(effect);
		KFL_UNUSED(shader_desc_ids);

		uint32_t len;
		res->read(&len, sizeof(len));
		len = LE2Native(len);
		if (len > 0)
		{
			res->seekg(len, std::ios_base::cur);
		}

		return true;
	}

	void NullShaderObject::StreamOut(std::ostream& os, ShaderType type)
	{
		if (so_template_->as_d3d11_ || so_template_->as_d3d12_)
		{
			this->D3D11StreamOut(os, type);
		}
		else if (so_template_->as_gl_ || so_template_->as_gles_)
		{
			this->OGLStreamOut(os, type);
		}
	}

	void NullShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		if (so_template_->as_d3d11_ || so_template_->as_d3d12_)
		{
			this->D3D11AttachShader(type, effect, tech, pass, shader_desc_ids);
		}
		else if (so_template_->as_gl_ || so_template_->as_gles_)
		{
			this->OGLAttachShader(type, effect, tech, pass, shader_desc_ids);
		}
	}

	void NullShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so)
	{
		if (so_template_->as_d3d11_ || so_template_->as_d3d12_)
		{
			this->D3D11AttachShader(type, effect, tech, pass, shared_so);
		}
		else if (so_template_->as_gl_ || so_template_->as_gles_)
		{
			this->OGLAttachShader(type, effect, tech, pass, shared_so);
		}
	}

	void NullShaderObject::LinkShaders(RenderEffect const & effect)
	{
		if (so_template_->as_d3d11_ || so_template_->as_d3d12_)
		{
			this->D3D11LinkShaders(effect);
		}
		else if (so_template_->as_gl_ || so_template_->as_gles_)
		{
			this->OGLLinkShaders(effect);
		}
	}

	ShaderObjectPtr NullShaderObject::Clone(RenderEffect const & effect)
	{
		KFL_UNUSED(effect);
		return MakeSharedPtr<NullShaderObject>();
	}

	void NullShaderObject::Bind()
	{
	}

	void NullShaderObject::Unbind()
	{
	}
	
	// D3D11/D3D12

	void NullShaderObject::D3D11StreamOut(std::ostream& os, ShaderType type)
	{
		// D3D11ShaderObject::StreamOut
		// D3D12ShaderObject::StreamOut

		auto d3d_so_template = checked_cast<D3D11ShaderObjectTemplate*>(so_template_.get());

		std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

		{
			uint8_t len = static_cast<uint8_t>(d3d_so_template->shader_code_[type].second.size());
			oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
			oss.write(reinterpret_cast<char const *>(&d3d_so_template->shader_code_[type].second[0]), len);
		}

		std::shared_ptr<std::vector<uint8_t>> code_blob = d3d_so_template->shader_code_[type].first;
		if (code_blob)
		{
			uint8_t len;

			uint32_t blob_size = Native2LE(static_cast<uint32_t>(code_blob->size()));
			oss.write(reinterpret_cast<char const *>(&blob_size), sizeof(blob_size));
			oss.write(reinterpret_cast<char const *>(&((*code_blob)[0])), code_blob->size());

			auto const & sd = *d3d_so_template->shader_desc_[type];

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
				uint32_t vs_signature = Native2LE(d3d_so_template->vs_signature_);
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

	std::shared_ptr<std::vector<uint8_t>> NullShaderObject::D3D11CompiteToBytecode(ShaderType type,
		RenderEffect const & effect, RenderTechnique const & tech, RenderPass const & pass,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		// D3D11ShaderObject::CompiteToBytecode
		// D3D12ShaderObject::CompiteToBytecode

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		auto d3d_so_template = checked_cast<D3D11ShaderObjectTemplate*>(so_template_.get());

		auto const & re = *checked_cast<NullRenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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
		d3d_so_template->shader_code_[type].second = shader_profile;

		std::shared_ptr<std::vector<uint8_t>> code = MakeSharedPtr<std::vector<uint8_t>>();
		if (is_shader_validate_[type])
		{
			std::vector<std::pair<char const *, char const *>> macros;
			if (so_template_->as_d3d11_)
			{
				macros.emplace_back("KLAYGE_D3D11", "1");
			}
			else
			{
				BOOST_ASSERT(so_template_->as_d3d12_);
				macros.emplace_back("KLAYGE_D3D12", "1");
			}
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
					if (!d3d_so_template->shader_desc_[type])
					{
						d3d_so_template->shader_desc_[type] = MakeSharedPtr<D3D11ShaderObjectTemplate::D3D11ShaderDesc>();
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

							d3d_so_template->shader_desc_[type]->cb_desc.push_back(cb_desc);
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

					d3d_so_template->shader_desc_[type]->num_samplers = static_cast<uint16_t>(max_sampler_bind_pt + 1);
					d3d_so_template->shader_desc_[type]->num_srvs = static_cast<uint16_t>(max_srv_bind_pt + 1);
					d3d_so_template->shader_desc_[type]->num_uavs = static_cast<uint16_t>(max_uav_bind_pt + 1);

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
								d3d_so_template->shader_desc_[type]->res_desc.push_back(brd);
							}
							break;

						default:
							break;
						}
					}

					if (ST_VertexShader == type)
					{
						d3d_so_template->vs_signature_ = 0;
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

							size_t sig = d3d_so_template->vs_signature_;
							HashCombine(sig, seed);
							d3d_so_template->vs_signature_ = static_cast<uint32_t>(sig);
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

	void NullShaderObject::D3D11AttachShaderBytecode(ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::shared_ptr<std::vector<uint8_t>> const & code_blob)
	{
		// Simplified D3D11ShaderObject::AttachShaderBytecode
		// Simplified D3D12ShaderObject::AttachShaderBytecode

		auto d3d_so_template = checked_cast<D3D11ShaderObjectTemplate*>(so_template_.get());

		if (code_blob)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			NullRenderEngine const & re = *checked_cast<NullRenderEngine const *>(&rf.RenderEngineInstance());
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
				d3d_so_template->shader_code_[type].first = code_blob;

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
					break;

				case ST_PixelShader:
					break;

				case ST_GeometryShader:
					if (!caps.gs_support)
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
					if (!caps.hs_support)
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
			if (!d3d_so_template->shader_desc_[type]->cb_desc.empty())
			{
				d3d_so_template->cbuff_indices_[type] = MakeSharedPtr<std::vector<uint8_t>>(d3d_so_template->shader_desc_[type]->cb_desc.size());
			}
			for (size_t c = 0; c < d3d_so_template->shader_desc_[type]->cb_desc.size(); ++ c)
			{
				uint32_t i = 0;
				for (; i < effect.NumCBuffers(); ++ i)
				{
					if (effect.CBufferByIndex(i)->NameHash() == d3d_so_template->shader_desc_[type]->cb_desc[c].name_hash)
					{
						(*d3d_so_template->cbuff_indices_[type])[c] = static_cast<uint8_t>(i);
						break;
					}
				}
				BOOST_ASSERT(i < effect.NumCBuffers());
			}
		}
		else
		{
			is_shader_validate_[type] = false;
		}
	}

	void NullShaderObject::D3D11AttachShader(ShaderType type, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		// D3D11ShaderObject::AttachShader
		// D3D12ShaderObject::AttachShader

		std::shared_ptr<std::vector<uint8_t>> code_blob = this->D3D11CompiteToBytecode(type, effect, tech, pass, shader_desc_ids);
		this->D3D11AttachShaderBytecode(type, effect, shader_desc_ids, code_blob);
	}

	void NullShaderObject::D3D11AttachShader(ShaderType type, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so)
	{
		// Simplified D3D11ShaderObject::AttachShader
		// Simplified D3D12ShaderObject::AttachShader

		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(pass);

		auto d3d_so_template = checked_cast<D3D11ShaderObjectTemplate*>(so_template_.get());

		if (shared_so)
		{
			NullShaderObject const & so = *checked_cast<NullShaderObject*>(shared_so.get());
			auto d3d_so_so_template = checked_cast<D3D11ShaderObjectTemplate*>(so.so_template_.get());

			is_shader_validate_[type] = so.is_shader_validate_[type];
			d3d_so_template->shader_code_[type] = d3d_so_so_template->shader_code_[type];
			d3d_so_template->shader_desc_[type] = d3d_so_so_template->shader_desc_[type];
			switch (type)
			{
			case ST_VertexShader:
				d3d_so_template->vs_signature_ = d3d_so_so_template->vs_signature_;
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

			d3d_so_template->cbuff_indices_[type] = d3d_so_so_template->cbuff_indices_[type];
		}
	}

	void NullShaderObject::D3D11LinkShaders(RenderEffect const & effect)
	{
		KFL_UNUSED(effect);

		// Simplified D3D11ShaderObject::LinkShaders
		// Simplified D3D12ShaderObject::LinkShaders

		is_validate_ = true;
		for (size_t type = 0; type < ST_NumShaderTypes; ++type)
		{
			is_validate_ &= is_shader_validate_[type];
		}
	}

	// OpenGL/OpenGLES

	void NullShaderObject::OGLStreamOut(std::ostream& os, ShaderType type)
	{
		// OGLShaderObject::StreamOut
		// OGLESShaderObject::StreamOut

#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
		auto ogl_so_template = checked_cast<OGLShaderObjectTemplate*>(so_template_.get());

		std::vector<uint8_t> native_shader_block;

		if (ogl_so_template->glsl_srcs_[type])
		{
			std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

			uint32_t len32 = Native2LE(static_cast<uint32_t>(ogl_so_template->glsl_srcs_[type]->size()));
			oss.write(reinterpret_cast<char const *>(&len32), sizeof(len32));
			oss.write(&(*ogl_so_template->glsl_srcs_[type])[0], ogl_so_template->glsl_srcs_[type]->size());

			uint16_t num16 = Native2LE(static_cast<uint16_t>(ogl_so_template->pnames_[type]->size()));
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < ogl_so_template->pnames_[type]->size(); ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>((*ogl_so_template->pnames_[type])[i].size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&(*ogl_so_template->pnames_[type])[i][0], (*ogl_so_template->pnames_[type])[i].size());
			}

			num16 = Native2LE(static_cast<uint16_t>(ogl_so_template->glsl_res_names_[type]->size()));
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < ogl_so_template->glsl_res_names_[type]->size(); ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>((*ogl_so_template->glsl_res_names_[type])[i].size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&(*ogl_so_template->glsl_res_names_[type])[i][0], (*ogl_so_template->glsl_res_names_[type])[i].size());
			}

			std::vector<std::pair<std::string, std::string>> tex_sampler_pairs;
			for (size_t i = 0; i < gl_tex_sampler_binds_.size(); ++ i)
			{
				if (std::get<3>(gl_tex_sampler_binds_[i]) | (1UL << type))
				{
					tex_sampler_pairs.emplace_back(std::get<1>(gl_tex_sampler_binds_[i])->Name(),
						std::get<2>(gl_tex_sampler_binds_[i])->Name());
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
				uint8_t num8 = static_cast<uint8_t>(ogl_so_template->vs_usages_.size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				for (size_t i = 0; i < ogl_so_template->vs_usages_.size(); ++ i)
				{
					uint8_t veu = static_cast<uint8_t>(ogl_so_template->vs_usages_[i]);
					oss.write(reinterpret_cast<char const *>(&veu), sizeof(veu));
				}

				num8 = static_cast<uint8_t>(ogl_so_template->vs_usage_indices_.size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				if (!ogl_so_template->vs_usage_indices_.empty())
				{
					oss.write(reinterpret_cast<char const *>(&ogl_so_template->vs_usage_indices_[0]),
						ogl_so_template->vs_usage_indices_.size() * sizeof(ogl_so_template->vs_usage_indices_[0]));
				}

				num8 = static_cast<uint8_t>(ogl_so_template->glsl_vs_attrib_names_.size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				for (size_t i = 0; i < ogl_so_template->glsl_vs_attrib_names_.size(); ++ i)
				{
					uint8_t len8 = static_cast<uint8_t>(ogl_so_template->glsl_vs_attrib_names_[i].size());
					oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
					oss.write(&ogl_so_template->glsl_vs_attrib_names_[i][0], ogl_so_template->glsl_vs_attrib_names_[i].size());
				}
			}
			else if (ST_GeometryShader == type)
			{
				uint32_t git = Native2LE(ogl_so_template->gs_input_type_);
				oss.write(reinterpret_cast<char const *>(&git), sizeof(git));

				uint32_t got = Native2LE(ogl_so_template->gs_output_type_);
				oss.write(reinterpret_cast<char const *>(&got), sizeof(got));

				uint32_t gmov = Native2LE(ogl_so_template->gs_max_output_vertex_);
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
#else
		KFL_UNUSED(os);
		KFL_UNUSED(type);
#endif
	}

	void NullShaderObject::OGLAttachShader(ShaderType type, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		// OGLShaderObject::AttachShader
		// OGLESShaderObject::AttachShader

#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
		auto ogl_so_template = checked_cast<OGLShaderObjectTemplate*>(so_template_.get());

		ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		ogl_so_template->shader_func_names_[type] = sd.func_name;

		bool has_gs = false;
		if (so_template_->as_gl_)
		{
			if (!effect.GetShaderDesc(shader_desc_ids[ST_GeometryShader]).func_name.empty())
			{
				has_gs = true;
			}
		}
		bool has_ps = false;
		if (!effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty())
		{
			has_ps = true;
		}

		is_shader_validate_[type] = true;
		switch (type)
		{
		case ST_VertexShader:
		case ST_PixelShader:
		case ST_HullShader:
		case ST_DomainShader:
			break;

		case ST_GeometryShader:
			if (so_template_->as_gl_)
			{
				break;
			}
			else
			{
				BOOST_ASSERT(so_template_->as_gles_);
				is_shader_validate_[type] = false;
				break;
			}

		default:
			is_shader_validate_[type] = false;
			break;
		}

		if (is_shader_validate_[type])
		{
			auto const & re = *checked_cast<NullRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto const & caps = re.DeviceCaps();

			int major_version, minor_version;
			re.GetCustomAttrib("MAJOR_VERSION", &major_version);
			re.GetCustomAttrib("MINOR_VERSION", &minor_version);

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
				if (so_template_->as_gl_)
				{
					macros.emplace_back("KLAYGE_OPENGL", "1");
				}
				else
				{
					BOOST_ASSERT(so_template_->as_gles_);
					macros.emplace_back("KLAYGE_OPENGLES", "1");
				}
				if (so_template_->as_gl_)
				{
					if (!caps.texture_format_support(EF_BC5) || !caps.texture_format_support(EF_BC5_SRGB))
					{
						macros.emplace_back("KLAYGE_BC5_AS_AG", "1");
					}
				}
				else
				{
					BOOST_ASSERT(so_template_->as_gles_);
					if (!caps.texture_format_support(EF_BC5) || !caps.texture_format_support(EF_BC5_SRGB))
					{
						macros.emplace_back("KLAYGE_BC5_AS_AG", "1");
					}
					else
					{
						macros.emplace_back("KLAYGE_BC5_AS_GA", "1");
					}
				}
				if (!caps.texture_format_support(EF_BC4) || !caps.texture_format_support(EF_BC4_SRGB))
				{
					macros.emplace_back("KLAYGE_BC4_AS_G", "1");
				}
				if (so_template_->as_gl_)
				{
					macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");
				}
				else
				{
					bool frag_depth_support;
					re.GetCustomAttrib("FRAG_DEPTH_SUPPORT", &frag_depth_support);
					macros.emplace_back("KLAYGE_FRAG_DEPTH", frag_depth_support ? "1" : "0");
				}

				uint32_t const flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
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
						if (so_template_->as_gl_)
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
						else
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

						DXBC2GLSL::DXBC2GLSL dxbc2glsl;
						uint32_t rules = DXBC2GLSL::DXBC2GLSL::DefaultRules(gsv);
						rules &= ~GSR_UniformBlockBinding;
						if (so_template_->as_gles_)
						{
							rules &= ~GSR_MatrixType;
							rules &= ~GSR_UIntType;
							rules |= caps.max_simultaneous_rts > 1 ? static_cast<uint32_t>(GSR_DrawBuffers) : 0;
							if ((ST_HullShader == type) || (ST_DomainShader == type))
							{
								rules |= static_cast<uint32_t>(GSR_EXTTessellationShader);
							}
						}
						dxbc2glsl.FeedDXBC(&code[0],
							has_gs, has_ps, static_cast<ShaderTessellatorPartitioning>(ogl_so_template->ds_partitioning_),
							static_cast<ShaderTessellatorOutputPrimitive>(ogl_so_template->ds_output_primitive_),
							gsv, rules);
						ogl_so_template->glsl_srcs_[type] = MakeSharedPtr<std::string>(dxbc2glsl.GLSLString());
						ogl_so_template->pnames_[type] = MakeSharedPtr<std::vector<std::string>>();
						ogl_so_template->glsl_res_names_[type] = MakeSharedPtr<std::vector<std::string>>();

						for (uint32_t i = 0; i < dxbc2glsl.NumCBuffers(); ++ i)
						{
							for (uint32_t j = 0; j < dxbc2glsl.NumVariables(i); ++ j)
							{
								if (dxbc2glsl.VariableUsed(i, j))
								{
									ogl_so_template->pnames_[type]->push_back(dxbc2glsl.VariableName(i, j));
									ogl_so_template->glsl_res_names_[type]->push_back(dxbc2glsl.VariableName(i, j));
								}
							}
						}

						std::vector<char const *> tex_names;
						std::vector<char const *> sampler_names;
						for (uint32_t i = 0; i < dxbc2glsl.NumResources(); ++ i)
						{
							if (dxbc2glsl.ResourceUsed(i))
							{
								char const * res_name = dxbc2glsl.ResourceName(i);

								if (SIT_TEXTURE == dxbc2glsl.ResourceType(i))
								{
									if (SSD_BUFFER == dxbc2glsl.ResourceDimension(i))
									{
										ogl_so_template->pnames_[type]->push_back(res_name);
										ogl_so_template->glsl_res_names_[type]->push_back(res_name);
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

						for (size_t i = 0; i < tex_names.size(); ++ i)
						{
							auto param = effect.ParameterByName(tex_names[i]);
							for (size_t j = 0; j < sampler_names.size(); ++ j)
							{
								std::string combined_sampler_name = std::string(tex_names[i]) + "_" + sampler_names[j];
								bool found = false;
								for (uint32_t k = 0; k < gl_tex_sampler_binds_.size(); ++ k)
								{
									if (std::get<0>(gl_tex_sampler_binds_[k]) == combined_sampler_name)
									{
										std::get<3>(gl_tex_sampler_binds_[k]) |= 1UL << type;
										found = true;
										break;
									}
								}
								if (!found)
								{
									gl_tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
										param, effect.ParameterByName(sampler_names[j]), 1UL << type));
								}

								ogl_so_template->pnames_[type]->push_back(combined_sampler_name);
								ogl_so_template->glsl_res_names_[type]->push_back(combined_sampler_name);
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
											KFL_UNREACHABLE("Invalid semantic");
										}

										ogl_so_template->vs_usages_.push_back(usage);
										ogl_so_template->vs_usage_indices_.push_back(usage_index);
										ogl_so_template->glsl_vs_attrib_names_.push_back(glsl_param_name);
									}
								}
							}
						}
						else if (ST_GeometryShader == type)
						{
							switch (dxbc2glsl.GSInputPrimitive())
							{
							case SP_Point:
								ogl_so_template->gs_input_type_ = GL_POINTS;
								break;

							case SP_Line:
								ogl_so_template->gs_input_type_ = GL_LINES;
								break;

							case SP_LineAdj:
								ogl_so_template->gs_input_type_ = GL_LINES_ADJACENCY;
								break;

							case SP_Triangle:
								ogl_so_template->gs_input_type_ = GL_TRIANGLES;
								break;

							case SP_TriangleAdj:
								ogl_so_template->gs_input_type_ = GL_TRIANGLES_ADJACENCY;
								break;

							default:
								KFL_UNREACHABLE("Invalid GS input type");
							}

							switch (dxbc2glsl.GSOutputTopology(0))
							{
							case SPT_PointList:
								ogl_so_template->gs_output_type_ = GL_POINTS;
								break;

							case SPT_LineStrip:
								ogl_so_template->gs_output_type_ = GL_LINE_STRIP;
								break;

							case SPT_TriangleStrip:
								ogl_so_template->gs_output_type_ = GL_TRIANGLE_STRIP;
								break;

							default:
								KFL_UNREACHABLE("Invalid GS output topology");
							}

							ogl_so_template->gs_max_output_vertex_ = dxbc2glsl.MaxGSOutputVertex();
						}
						else if (ST_HullShader == type)
						{
							ogl_so_template->ds_partitioning_ = dxbc2glsl.DSPartitioning();
							ogl_so_template->ds_output_primitive_ = dxbc2glsl.DSOutputPrimitive();
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
#else
		KFL_UNUSED(type);
		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(pass);
		KFL_UNUSED(shader_desc_ids);
#endif
	}

	void NullShaderObject::OGLAttachShader(ShaderType type, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass, ShaderObjectPtr const & shared_so)
	{
		// Simplified OGLShaderObject::AttachShader
		// Simplified OGLESShaderObject::AttachShader

		KFL_UNUSED(effect);
		KFL_UNUSED(tech);
		KFL_UNUSED(pass);

#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
		auto so = checked_cast<NullShaderObject*>(shared_so.get());

		auto ogl_so_template = checked_cast<OGLShaderObjectTemplate*>(so_template_.get());
		auto ogl_so_so_template = checked_cast<OGLShaderObjectTemplate*>(so->so_template_.get());

		is_shader_validate_[type] = so->is_shader_validate_[type];
		ogl_so_template->shader_func_names_[type] = ogl_so_so_template->shader_func_names_[type];

		if (is_shader_validate_[type])
		{
			ogl_so_template->glsl_srcs_[type] = ogl_so_so_template->glsl_srcs_[type];

			ogl_so_template->pnames_[type] = ogl_so_so_template->pnames_[type];
			ogl_so_template->glsl_res_names_[type] = ogl_so_so_template->glsl_res_names_[type];
			if (ST_VertexShader == type)
			{
				ogl_so_template->vs_usages_ = ogl_so_so_template->vs_usages_;
				ogl_so_template->vs_usage_indices_ = ogl_so_so_template->vs_usage_indices_;
				ogl_so_template->glsl_vs_attrib_names_ = ogl_so_so_template->glsl_vs_attrib_names_;
			}
			else if (ST_GeometryShader == type)
			{
				ogl_so_template->gs_input_type_ = ogl_so_so_template->gs_input_type_;
				ogl_so_template->gs_output_type_ = ogl_so_so_template->gs_output_type_;
				ogl_so_template->gs_max_output_vertex_ = ogl_so_so_template->gs_max_output_vertex_;
			}
			else if (ST_HullShader == type)
			{
				ogl_so_template->ds_partitioning_ = ogl_so_so_template->ds_partitioning_;
				ogl_so_template->ds_output_primitive_ = ogl_so_so_template->ds_output_primitive_;
			}

			for (uint32_t j = 0; j < so->gl_tex_sampler_binds_.size(); ++ j)
			{
				if (std::get<3>(so->gl_tex_sampler_binds_[j]) | (1UL << type))
				{
					std::string const & combined_sampler_name = std::get<0>(so->gl_tex_sampler_binds_[j]);
					bool found = false;
					for (uint32_t k = 0; k < gl_tex_sampler_binds_.size(); ++ k)
					{
						if (std::get<0>(gl_tex_sampler_binds_[k]) == combined_sampler_name)
						{
							std::get<3>(gl_tex_sampler_binds_[k]) |= 1UL << type;
							found = true;
							break;
						}
					}
					if (!found)
					{
						gl_tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
							std::get<1>(so->gl_tex_sampler_binds_[j]), std::get<2>(so->gl_tex_sampler_binds_[j]), 1UL << type));
					}
				}
			}
		}
#else
		KFL_UNUSED(type);
		KFL_UNUSED(shared_so);
#endif
	}

	void NullShaderObject::OGLLinkShaders(RenderEffect const & effect)
	{
		// Simplified OGLShaderObject::LinkShaders
		// Simplified OGLESShaderObject::LinkShaders

		KFL_UNUSED(effect);

#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
		auto ogl_so_template = checked_cast<OGLShaderObjectTemplate*>(so_template_.get());

		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++type)
		{
			if (!ogl_so_template->shader_func_names_[type].empty())
			{
				is_validate_ &= is_shader_validate_[type];
			}
		}
#endif
	}
}
