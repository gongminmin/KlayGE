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
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <D3DCompiler.h>
#endif

#include "OfflineRenderEffect.hpp"
#include "OfflineD3D11ShaderObject.hpp"

DEFINE_GUID(IID_ID3D11ShaderReflection_47,
	0x8d536ca1, 0x0cca, 0x4956, 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84);

struct D3D11_SIGNATURE_PARAMETER_DESC_47
{
	LPCSTR						SemanticName;
	UINT						SemanticIndex;
	UINT						Register;
	D3D_NAME					SystemValueType;
	D3D_REGISTER_COMPONENT_TYPE	ComponentType;
	BYTE						Mask;
	BYTE						ReadWriteMask;
	UINT						Stream;
	UINT						MinPrecision;
};

#if D3D_COMPILER_VERSION < 44
#define D3DCOMPILER_STRIP_PRIVATE_DATA 8
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
class D3DCompilerInit
{
public:
	D3DCompilerInit()
	{
		mod_d3dcompiler_ = ::LoadLibraryEx(TEXT("d3dcompiler_47.dll"), nullptr, 0);
		if (mod_d3dcompiler_)
		{
			DynamicD3DCompile_ = reinterpret_cast<D3DCompileFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DCompile"));
			DynamicD3DReflect_ = reinterpret_cast<D3DReflectFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DReflect"));
			DynamicD3DStripShader_ = reinterpret_cast<D3DStripShaderFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DStripShader"));
		}
		else
		{
			::MessageBoxW(nullptr, L"Can't load d3dcompiler_47.dll", L"Error", MB_OK);
		}
	}

	~D3DCompilerInit()
	{
		if (mod_d3dcompiler_)
		{
			::FreeLibrary(mod_d3dcompiler_);
		}
	}

	HRESULT D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
		D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
		LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs) const
	{
		return DynamicD3DCompile_(pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, pEntrypoint,
			pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);
	}

	HRESULT D3DReflect(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector) const
	{
		return DynamicD3DReflect_(pSrcData, SrcDataSize, pInterface, ppReflector);
	}

	HRESULT D3DStripShader(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags, ID3DBlob** ppStrippedBlob) const
	{
		return DynamicD3DStripShader_(pShaderBytecode, BytecodeLength, uStripFlags, ppStrippedBlob);
	}

private:
	typedef HRESULT(WINAPI *D3DCompileFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
		D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
		LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
	typedef HRESULT(WINAPI *D3DReflectFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector);
	typedef HRESULT(WINAPI *D3DStripShaderFunc)(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags, ID3DBlob** ppStrippedBlob);

private:
	HMODULE mod_d3dcompiler_;

	D3DCompileFunc DynamicD3DCompile_;
	D3DReflectFunc DynamicD3DReflect_;
	D3DStripShaderFunc DynamicD3DStripShader_;
};

D3DCompilerInit d3dcompiler;
#endif

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
			case 11:
				switch (caps.minor_version)
				{
				case 1:
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
					feature_level_ = D3D_FEATURE_LEVEL_11_1;
#else
					feature_level_ = D3D_FEATURE_LEVEL_11_0;
#endif
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
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
			case D3D_FEATURE_LEVEL_11_1:
#endif
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

		std::string D3D11ShaderObject::GenShaderText(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass) const
		{
			OfflineRenderDeviceCaps const & caps = caps_;

			std::stringstream ss;

			for (uint32_t i = 0; i < effect.NumMacros(); ++ i)
			{
				std::pair<std::string, std::string> const & name_value = effect.MacroByIndex(i);
				ss << "#define " << name_value.first << " " << name_value.second << std::endl;
			}
			ss << std::endl;

			for (uint32_t i = 0; i < tech.NumMacros(); ++ i)
			{
				std::pair<std::string, std::string> const & name_value = tech.MacroByIndex(i);
				ss << "#define " << name_value.first << " " << name_value.second << std::endl;
			}
			ss << std::endl;

			for (uint32_t i = 0; i < pass.NumMacros(); ++ i)
			{
				std::pair<std::string, std::string> const & name_value = pass.MacroByIndex(i);
				ss << "#define " << name_value.first << " " << name_value.second << std::endl;
			}
			ss << std::endl;

			for (uint32_t i = 0; i < effect.NumCBuffers(); ++ i)
			{
				RenderEffectConstantBufferPtr const & cbuff = effect.CBufferByIndex(i);
				ss << "cbuffer " << *cbuff->Name() << std::endl;
				ss << "{" << std::endl;

				for (uint32_t j = 0; j < cbuff->NumParameters(); ++ j)
				{
					RenderEffectParameter& param = *effect.ParameterByIndex(cbuff->ParameterIndex(j));
					switch (param.Type())
					{
					case REDT_texture1D:
					case REDT_texture2D:
					case REDT_texture3D:
					case REDT_textureCUBE:
					case REDT_texture1DArray:
					case REDT_texture2DArray:
					case REDT_texture3DArray:
					case REDT_textureCUBEArray:
					case REDT_sampler:
					case REDT_buffer:
					case REDT_structured_buffer:
					case REDT_byte_address_buffer:
					case REDT_rw_buffer:
					case REDT_rw_structured_buffer:
					case REDT_rw_texture1D:
					case REDT_rw_texture2D:
					case REDT_rw_texture3D:
					case REDT_rw_texture1DArray:
					case REDT_rw_texture2DArray:
					case REDT_rw_byte_address_buffer:
					case REDT_append_structured_buffer:
					case REDT_consume_structured_buffer:
						break;

					default:
						ss << effect.TypeName(param.Type()) << " " << *param.Name();
						if (param.ArraySize())
						{
							ss << "[" << *param.ArraySize() << "]";
						}
						ss << ";" << std::endl;
						break;
					}
				}

				ss << "};" << std::endl;
			}

			for (uint32_t i = 0; i < effect.NumParameters(); ++ i)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(i);

				switch (param.Type())
				{
				case REDT_texture1D:
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "Texture1D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_texture2D:
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "Texture2D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_texture3D:
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "Texture3D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_textureCUBE:
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "TextureCube<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_texture1DArray:
					if (caps.max_shader_model >= ShaderModel(4, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "Texture1DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_texture2DArray:
					if (caps.max_shader_model >= ShaderModel(4, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "Texture2DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_textureCUBEArray:
					if (caps.max_shader_model >= ShaderModel(4, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "TextureCubeArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_buffer:
					if (caps.max_shader_model >= ShaderModel(4, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "Buffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_sampler:
					ss << "sampler " << *param.Name() << ";" << std::endl;
					break;

				case REDT_structured_buffer:
					if (caps.max_shader_model >= ShaderModel(4, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "StructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_byte_address_buffer:
					if (caps.max_shader_model >= ShaderModel(4, 0))
					{
						ss << "ByteAddressBuffer " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_rw_buffer:
					if (caps.max_shader_model >= ShaderModel(5, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "RWBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_rw_structured_buffer:
					if (caps.max_shader_model >= ShaderModel(4, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "RWStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_rw_texture1D:
					if (caps.max_shader_model >= ShaderModel(5, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "RWTexture1D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_rw_texture2D:
					if (caps.max_shader_model >= ShaderModel(5, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "RWTexture2D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_rw_texture3D:
					if (caps.max_shader_model >= ShaderModel(5, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "RWTexture3D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;
				case REDT_rw_texture1DArray:
					if (caps.max_shader_model >= ShaderModel(5, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "RWTexture1DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_rw_texture2DArray:
					if (caps.max_shader_model >= ShaderModel(5, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "RWTexture2DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_rw_byte_address_buffer:
					if (caps.max_shader_model >= ShaderModel(4, 0))
					{
						ss << "RWByteAddressBuffer " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_append_structured_buffer:
					if (caps.max_shader_model >= ShaderModel(5, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "AppendStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				case REDT_consume_structured_buffer:
					if (caps.max_shader_model >= ShaderModel(5, 0))
					{
						std::string elem_type;
						param.Var()->Value(elem_type);
						ss << "ConsumeStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
					}
					break;

				default:
					break;
				}
			}

			for (uint32_t i = 0; i < effect.NumShaders(); ++ i)
			{
				RenderShaderFunc const & effect_shader = effect.ShaderByIndex(i);
				ShaderType shader_type = effect_shader.Type();
				if ((ST_NumShaderTypes == shader_type) || (type == shader_type))
				{
					if (caps.max_shader_model >= effect_shader.Version())
					{
						ss << effect_shader.str() << std::endl;
					}
				}
			}

			return ss.str();
		}

		void D3D11ShaderObject::StreamOut(std::ostream& os, ShaderType type)
		{
			std::vector<uint8_t> native_shader_block;

			std::shared_ptr<std::vector<uint8_t>> code_blob = shader_code_[type].first;
			if (code_blob)
			{
				std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

				uint8_t len = static_cast<uint8_t>(shader_code_[type].second.size());
				oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
				oss.write(reinterpret_cast<char const *>(&shader_code_[type].second[0]), len);

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

		std::shared_ptr<std::vector<uint8_t>> D3D11ShaderObject::CompiteToBytecode(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids)
		{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			D3D_FEATURE_LEVEL const feature_level = feature_level_;
			OfflineRenderDeviceCaps const & caps = caps_;

			std::string max_sm_str = boost::lexical_cast<std::string>(caps.max_shader_model.FullVersion());
			std::string max_tex_array_str = boost::lexical_cast<std::string>(caps.max_texture_array_length);
			std::string max_tex_depth_str = boost::lexical_cast<std::string>(caps.max_texture_depth);
			std::string max_tex_units_str = boost::lexical_cast<std::string>(static_cast<int>(caps.max_pixel_texture_units));
			std::string flipping_str = boost::lexical_cast<std::string>(caps.requires_flipping ? -1 : +1);
			std::string standard_derivatives_str = boost::lexical_cast<std::string>(caps.standard_derivatives_support ? 1 : 0);
			std::string no_tex_lod_str = boost::lexical_cast<std::string>((ST_PixelShader == type) ? (caps.shader_texture_lod_support ? 0 : 1) : 1);

			ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

			std::string shader_text = this->GenShaderText(static_cast<ShaderType>(type), effect, tech, pass);

			is_shader_validate_[type] = true;

			std::string shader_profile = sd.profile;
			size_t const shader_profile_hash = RT_HASH(shader_profile.c_str());
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

			ID3DBlob* code = nullptr;
			if (is_shader_validate_[type])
			{
				ID3DBlob* err_msg;
				std::vector<D3D_SHADER_MACRO> macros;
				{
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_D3D11", "1" };
					macros.push_back(macro_d3d11);
				}
				{
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_SHADER_MODEL", max_sm_str.c_str() };
					macros.push_back(macro_d3d11);
				}
				{
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_ARRAY_LEN", max_tex_array_str.c_str() };
					macros.push_back(macro_d3d11);
				}
				{
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_DEPTH", max_tex_depth_str.c_str() };
					macros.push_back(macro_d3d11);
				}
				{
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_UNITS", max_tex_units_str.c_str() };
					macros.push_back(macro_d3d11);
				}
				{
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_FLIPPING", flipping_str.c_str() };
					macros.push_back(macro_d3d11);
				}
				{
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_DERIVATIVES", standard_derivatives_str.c_str() };
					macros.push_back(macro_d3d11);
				}
				{
					D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_NO_TEX_LOD", no_tex_lod_str.c_str() };
					macros.push_back(macro_d3d11);
				}
				if (feature_level <= D3D_FEATURE_LEVEL_9_3)
				{
					D3D_SHADER_MACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_AG", "1" };
					macros.push_back(macro_bc5_as_bc3);

					D3D_SHADER_MACRO macro_bc4_as_bc1 = { "KLAYGE_BC4_AS_G", "1" };
					macros.push_back(macro_bc4_as_bc1);
				}
				if (!caps.fp_color_support)
				{
					D3D_SHADER_MACRO macro_no_fp_tex = { "KLAYGE_NO_FP_COLOR", "1" };
					macros.push_back(macro_no_fp_tex);
				}
				if (caps.pack_to_rgba_required)
				{
					D3D_SHADER_MACRO macro_pack_to_rgba = { "KLAYGE_PACK_TO_RGBA", "1" };
					macros.push_back(macro_pack_to_rgba);
				}
				{
					D3D_SHADER_MACRO macro_frag_depth = { "KLAYGE_FRAG_DEPTH", "1" };
					macros.push_back(macro_frag_depth);
				}
				{
					D3D_SHADER_MACRO macro_shader_type = { "", "1" };
					switch (type)
					{
					case ST_VertexShader:
						macro_shader_type.Name = "KLAYGE_VERTEX_SHADER";
						break;

					case ST_PixelShader:
						macro_shader_type.Name = "KLAYGE_PIXEL_SHADER";
						break;

					case ST_GeometryShader:
						macro_shader_type.Name = "KLAYGE_GEOMETRY_SHADER";
						break;

					case ST_ComputeShader:
						macro_shader_type.Name = "KLAYGE_COMPUTE_SHADER";
						break;

					case ST_HullShader:
						macro_shader_type.Name = "KLAYGE_HULL_SHADER";
						break;

					case ST_DomainShader:
						macro_shader_type.Name = "KLAYGE_DOMAIN_SHADER";
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
					macros.push_back(macro_shader_type);
				}
				{
					D3D_SHADER_MACRO macro_end = { nullptr, nullptr };
					macros.push_back(macro_end);
				}
				uint32_t flags = 0;
#if !defined(KLAYGE_DEBUG)
				flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
				d3dcompiler.D3DCompile(shader_text.c_str(), static_cast<UINT>(shader_text.size()), nullptr, &macros[0],
					nullptr, sd.func_name.c_str(), shader_profile.c_str(),
					flags, 0, &code, &err_msg);
				if (err_msg != nullptr)
				{
					LogError("Error when compiling %s:", sd.func_name.c_str());

					std::map<int, std::vector<std::string>> err_lines;
					{
						std::istringstream err_iss(static_cast<char*>(err_msg->GetBufferPointer()));
						std::string err_str;
						while (err_iss)
						{
							std::getline(err_iss, err_str);

							int err_line = -1;
							std::string::size_type pos = err_str.find("): error X");
							if (pos == std::string::npos)
							{
								pos = err_str.find("): warning X");
							}
							if (pos != std::string::npos)
							{
								std::string part_err_str = err_str.substr(0, pos);
								pos = part_err_str.rfind("(");
								part_err_str = part_err_str.substr(pos + 1);
								std::istringstream(part_err_str) >> err_line;
							}

							std::vector<std::string>& msgs = err_lines[err_line];
							bool found = false;
							for (auto const & msg : msgs)
							{
								if (msg == err_str)
								{
									found = true;
									break;
								}
							}

							if (!found)
							{
								// To make the error message unrecognized by Visual Studio
								if ((0 == err_str.find("error X")) || (0 == err_str.find("warning X")))
								{
									err_str = "(0): " + err_str;
								}

								msgs.push_back(err_str);
							}
						}
					}

					for (auto iter = err_lines.begin(); iter != err_lines.end(); ++ iter)
					{
						if (iter->first >= 0)
						{
							std::istringstream iss(shader_text);
							std::string s;
							int line = 1;

							LogInfo("...");
							while (iss && ((iter->first - line) >= 3))
							{
								std::getline(iss, s);
								++ line;
							}
							while (iss && (abs(line - iter->first) < 3))
							{
								std::getline(iss, s);
							
								while (!s.empty() && (('\r' == s[s.size() - 1]) || ('\n' == s[s.size() - 1])))
								{
									s.resize(s.size() - 1);
								}

								LogInfo("%d %s", line, s.c_str());

								++ line;
							}
							LogInfo("...");
						}

						for (auto const & msg : iter->second)
						{
							LogError(msg.c_str());
						}
					}

					err_msg->Release();
				}

				if (code)
				{
					ID3D11ShaderReflection* reflection;
					d3dcompiler.D3DReflect(code->GetBufferPointer(), code->GetBufferSize(),
						IID_ID3D11ShaderReflection_47, reinterpret_cast<void**>(&reflection));
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
								{
									RenderEffectParameterPtr const & p = effect.ParameterByName(si_desc.Name);
									if (p)
									{
										D3D11ShaderDesc::BoundResourceDesc brd;
										brd.name = si_desc.Name;
										brd.type = static_cast<uint8_t>(si_desc.Type);
										brd.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
										shader_desc_[type].res_desc.push_back(brd);
									}
								}
								break;

							default:
								break;
							}
						}

						if (ST_VertexShader == type)
						{
							vs_signature_ = 0;
							D3D11_SIGNATURE_PARAMETER_DESC_47 signature;
							for (uint32_t i = 0; i < desc.InputParameters; ++ i)
							{
								reflection->GetInputParameterDesc(i, reinterpret_cast<D3D11_SIGNATURE_PARAMETER_DESC*>(&signature));

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

					ID3DBlob* strip_code = nullptr;
					d3dcompiler.D3DStripShader(code->GetBufferPointer(), code->GetBufferSize(),
						D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO
						| D3DCOMPILER_STRIP_TEST_BLOBS | D3DCOMPILER_STRIP_PRIVATE_DATA,
						&strip_code);
					if (strip_code)
					{
						code->Release();
						code = strip_code;
					}
				}
			}

			std::shared_ptr<std::vector<uint8_t>> ret;
			if (code)
			{
				ret = MakeSharedPtr<std::vector<uint8_t>>(code->GetBufferSize());
				std::memcpy(&((*ret)[0]), code->GetBufferPointer(), code->GetBufferSize());
				code->Release();
			}

			return ret;
	#else
			UNREF_PARAM(type);
			UNREF_PARAM(effect);
			UNREF_PARAM(tech);
			UNREF_PARAM(pass);
			UNREF_PARAM(shader_desc_ids);

			return shared_ptr<std::vector<uint8_t>>();
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
					UNREF_PARAM(found);
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
