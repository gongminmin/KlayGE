/**
* @file OfflineOGLESShaderObject.cpp
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

#if !(defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS))
#include <DXBC2GLSL/DXBC2GLSL.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#define CALL_D3DCOMPILER_DIRECTLY
#endif

#ifdef CALL_D3DCOMPILER_DIRECTLY

#include <KlayGE/SALWrapper.hpp>
#include <D3DCompiler.h>
#else
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
typedef char const * LPCSTR;
typedef long HRESULT;

#define D3DCOMPILE_DEBUG                            0x00000001
#define D3DCOMPILE_SKIP_VALIDATION                  0x00000002
#define D3DCOMPILE_SKIP_OPTIMIZATION                0x00000004
#define D3DCOMPILE_PACK_MATRIX_ROW_MAJOR            0x00000008
#define D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR         0x00000010
#define D3DCOMPILE_PARTIAL_PRECISION                0x00000020
#define D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT         0x00000040
#define D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT         0x00000080
#define D3DCOMPILE_NO_PRESHADER                     0x00000100
#define D3DCOMPILE_AVOID_FLOW_CONTROL               0x00000200
#define D3DCOMPILE_PREFER_FLOW_CONTROL              0x00000400
#define D3DCOMPILE_ENABLE_STRICTNESS                0x00000800
#define D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY   0x00001000
#define D3DCOMPILE_IEEE_STRICTNESS                  0x00002000
#define D3DCOMPILE_OPTIMIZATION_LEVEL0              0x00004000
#define D3DCOMPILE_OPTIMIZATION_LEVEL1              0x00000000
#define D3DCOMPILE_OPTIMIZATION_LEVEL2              0x0000c000
#define D3DCOMPILE_OPTIMIZATION_LEVEL3              0x00008000
#define D3DCOMPILE_WARNINGS_ARE_ERRORS              0x00040000

typedef struct _D3D_SHADER_MACRO
{
	LPCSTR Name;
	LPCSTR Definition;
} D3D_SHADER_MACRO, *LPD3D_SHADER_MACRO;
#endif
#endif

#include "OfflineRenderEffect.hpp"
#include "OfflineOGLESShaderObject.hpp"

namespace
{
	using namespace KlayGE;
	using namespace KlayGE::Offline;

#if !(defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS))
	class DXBC2GLSLIniter
	{
	public:
		~DXBC2GLSLIniter()
		{
#ifdef CALL_D3DCOMPILER_DIRECTLY
			::FreeLibrary(mod_d3dcompiler_);
#endif
		}

		static DXBC2GLSLIniter& Instance(OfflineRenderDeviceCaps const & caps)
		{
			static DXBC2GLSLIniter initer(caps);
			return initer;
		}

		HRESULT D3DCompile(std::string const & src_data,
			D3D_SHADER_MACRO const * defines, std::string const & entry_point,
			std::string const & target, uint32_t flags1, uint32_t flags2,
			std::vector<uint8_t>& code, std::string& error_msgs) const
		{
#ifdef CALL_D3DCOMPILER_DIRECTLY
			ID3DBlob* code_blob = nullptr;
			ID3DBlob* error_msgs_blob = nullptr;
			HRESULT hr = DynamicD3DCompile_(src_data.c_str(), static_cast<UINT>(src_data.size()),
				nullptr, defines, nullptr, entry_point.c_str(),
				target.c_str(), flags1, flags2, &code_blob, &error_msgs_blob);
			if (code_blob)
			{
				uint8_t const * p = static_cast<uint8_t const *>(code_blob->GetBufferPointer());
				code.assign(p, p + code_blob->GetBufferSize());
				code_blob->Release();
			}
			else
			{
				code.clear();
			}
			if (error_msgs_blob)
			{
				char const * p = static_cast<char const *>(error_msgs_blob->GetBufferPointer());
				error_msgs.assign(p, p + error_msgs_blob->GetBufferSize());
				error_msgs_blob->Release();
			}
			else
			{
				error_msgs.clear();
			}
			return hr;
#else
			std::string mark = boost::lexical_cast<std::string>(static_cast<void const *>(src_data.c_str()));
			std::string compile_input_file = entry_point + mark + "Input.tmp";
			std::string compile_output_file = entry_point + mark + "Output.tmp";

			uint32_t buffer_size;
			
			std::ofstream ofs(compile_input_file.c_str(), std::ios_base::binary);

			buffer_size = static_cast<uint32_t>(src_data.size());
			ofs.write(reinterpret_cast<char const *>(&buffer_size), sizeof(buffer_size));
			ofs.write(src_data.c_str(), buffer_size);
		
			uint32_t idx = 0;
			while ((defines[idx].Definition != nullptr) && (defines[idx].Name != nullptr))
			{
				++ idx;
			}

			ofs.write(reinterpret_cast<char const *>(&idx), sizeof(idx));

			idx = 0;
			while ((defines[idx].Definition != nullptr) && (defines[idx].Name != nullptr))
			{
				ofs << defines[idx].Name << std::endl;
				ofs << defines[idx].Definition << std::endl;
				++ idx;
			}

			ofs.close();
			
			std::stringstream ss;
			std::string d3dcompiler_wrapper_name = "D3DCompilerWrapper";
#ifdef KLAYGE_DEBUG
			d3dcompiler_wrapper_name += "_d";
#endif
#ifdef KLAYGE_PLATFORM_WINDOWS
			ss << d3dcompiler_wrapper_name << ".exe";
#else
			static bool first = true;
			if (first)
			{
				ss << WINE_PATH << "wineserver -p";
				system(ss.str().c_str());
				// We should hold on a persistant wineserver, or XCode will lost connection after wineserver instance close and wine may not be able to find '.exe.so' file
				first = false;
				ss.str(std::string());
			}
			d3dcompiler_wrapper_name += ".exe.so";
			std::string wrapper_path = ResLoader::Instance().Locate(d3dcompiler_wrapper_name);
			ss << WINE_PATH << "wine " << wrapper_path;
#endif
			ss << " compile";
			ss << " " << compile_input_file;
			ss << " " << entry_point << " " << target;
			ss << " " << flags1 << " " << flags2;
			ss << " " << compile_output_file;
			if (system(ss.str().c_str()) != 0)
			{
				return -1;
			}

			std::ifstream ifs(compile_output_file.c_str(), std::ios_base::binary);

			uint32_t hr;
			ifs.read(reinterpret_cast<char*>(&hr), sizeof(hr));

			ifs.read(reinterpret_cast<char*>(&buffer_size), sizeof(buffer_size));
			if (buffer_size > 0)
			{
				code.resize(buffer_size);
				ifs.read(reinterpret_cast<char*>(&code[0]), buffer_size);
			}
			else
			{
				code.clear();
			}
			
			ifs.read(reinterpret_cast<char*>(&buffer_size), sizeof(buffer_size));
			if (buffer_size > 0)
			{
				error_msgs.resize(buffer_size);
				ifs.read(&error_msgs[0], buffer_size);
			}
			else
			{
				error_msgs.clear();
			}

			ifs.close();

			remove(compile_input_file.c_str());
			remove(compile_output_file.c_str());
			
			return hr;
#endif
		}

		GLSLVersion GLSLVer() const
		{
			return gsv_;
		}

	private:
		explicit DXBC2GLSLIniter(OfflineRenderDeviceCaps const & caps)
		{
#ifdef CALL_D3DCOMPILER_DIRECTLY
			mod_d3dcompiler_ = ::LoadLibraryEx(TEXT("d3dcompiler_47.dll"), nullptr, 0);
#ifdef KLAYGE_COMPILER_MSVC
			__assume(mod_d3dcompiler_ != nullptr);
#endif
			DynamicD3DCompile_ = reinterpret_cast<D3DCompileFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DCompile"));
#endif

			switch (caps.major_version)
			{
			case 3:
				switch (caps.minor_version)
				{
				case 1:
					gsv_ = GSV_310_ES;
					break;

				default:
				case 0:
					gsv_ = GSV_300_ES;
					break;
				}
				break;

			default:
			case 2:
				gsv_ = GSV_100_ES;
			}
		}

	private:
#ifdef CALL_D3DCOMPILER_DIRECTLY
		typedef HRESULT(WINAPI *D3DCompileFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
			D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
			LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);

		HMODULE mod_d3dcompiler_;
		D3DCompileFunc DynamicD3DCompile_;
#endif
		GLSLVersion gsv_;
	};
#endif
}

namespace KlayGE
{
	namespace Offline
	{
		OGLESShaderObject::OGLESShaderObject(OfflineRenderDeviceCaps const & caps)
			: ShaderObject(caps),
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

#if !(defined(KLAYGE_PLATFORM_IOS) || defined(KLAYGE_PLATFORM_ANDROID))
		std::string OGLESShaderObject::GenHLSLShaderText(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass) const
		{
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

			OfflineRenderDeviceCaps const & caps = caps_;
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
						ss << "Texture";
						if (caps.max_texture_depth <= 1)
						{
							ss << "2D";
						}
						else
						{
							ss << "3D";
						}
						ss << "<" << elem_type << "> " << *param.Name() << ";" << std::endl;
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
#endif

		void OGLESShaderObject::StreamOut(std::ostream& os, ShaderType type)
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
						tex_sampler_pairs.push_back(std::make_pair(*std::get<1>(tex_sampler_binds_[i])->Name(),
							*std::get<2>(tex_sampler_binds_[i])->Name()));
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

		void OGLESShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
				RenderTechnique const & tech, RenderPass const & pass, std::vector<uint32_t> const & shader_desc_ids)
		{
			ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

			(*shader_func_names_)[type] = sd.func_name;

			is_shader_validate_[type] = true;
			switch (type)
			{
			case ST_VertexShader:
			case ST_PixelShader:
				break;

			default:
				is_shader_validate_[type] = false;
				break;
			}

			if (is_shader_validate_[type])
			{
#if !(defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_IOS))
				OfflineRenderDeviceCaps const & caps = caps_;

				std::string max_sm_str = boost::lexical_cast<std::string>(caps.max_shader_model.FullVersion());
				std::string max_tex_array_str = boost::lexical_cast<std::string>(caps.max_texture_array_length);
				std::string max_tex_depth_str = boost::lexical_cast<std::string>(caps.max_texture_depth);
				std::string max_tex_units_str = boost::lexical_cast<std::string>(static_cast<int>(caps.max_pixel_texture_units));
				std::string flipping_str = boost::lexical_cast<std::string>(caps.requires_flipping ? -1 : +1);
				std::string standard_derivatives_str = boost::lexical_cast<std::string>(caps.standard_derivatives_support ? 1 : 0);
				std::string no_tex_lod_str = boost::lexical_cast<std::string>((ST_PixelShader == type) ? (caps.shader_texture_lod_support ? 0 : 1) : 1);
				std::string frag_depth_str = boost::lexical_cast<std::string>(caps.frag_depth_support ? 1 : 0);

				std::string hlsl_shader_text = this->GenHLSLShaderText(type, effect, tech, pass);

				is_shader_validate_[type] = true;

				std::string shader_profile = sd.profile;
				size_t const shader_profile_hash = RT_HASH(shader_profile.c_str());
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
					std::vector<D3D_SHADER_MACRO> macros;
					{
						D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_DXBC2GLSL", "1" };
						macros.push_back(macro_d3d11);
					}
					{
						D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_OPENGLES", "1" };
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
					if (caps.bc5_support)
					{
						D3D_SHADER_MACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_GA", "1" };
						macros.push_back(macro_bc5_as_bc3);
					}
					else
					{
						D3D_SHADER_MACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_AG", "1" };
						macros.push_back(macro_bc5_as_bc3);
					}
					if (!caps.bc4_support)
					{
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
						D3D_SHADER_MACRO macro_frag_depth = { "KLAYGE_FRAG_DEPTH", frag_depth_str.c_str() };
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
					uint32_t flags = D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;

					DXBC2GLSLIniter::Instance(caps_).D3DCompile(hlsl_shader_text, &macros[0],
						sd.func_name, shader_profile,
						flags, 0, code, err_msg);
					if (!err_msg.empty())
					{
						LogError("Error when compiling %s:", sd.func_name.c_str());

						std::map<int, std::vector<std::string>> err_lines;
						{
							std::istringstream err_iss(err_msg);
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
									msgs.push_back(err_str);
								}
							}
						}

						for (auto iter = err_lines.begin(); iter != err_lines.end(); ++ iter)
						{
							if (iter->first >= 0)
							{
								std::istringstream iss(hlsl_shader_text);
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
					}

					if (code.empty())
					{
						is_shader_validate_[type] = false;
					}
					else
					{
						try
						{
							GLSLVersion gsv = DXBC2GLSLIniter::Instance(caps_).GLSLVer();
							DXBC2GLSL::DXBC2GLSL dxbc2glsl;
							uint32_t rules = DXBC2GLSL::DXBC2GLSL::DefaultRules(gsv);
							rules &= ~GSR_UniformBlockBinding;
							rules &= ~GSR_MatrixType;
							rules |= caps.frag_depth_support ? static_cast<uint32_t>(GSR_EXTFragDepth) : 0;
							if (caps.major_version < 3)
							{
								rules |= caps.shader_texture_lod_support ? static_cast<uint32_t>(GSR_EXTShaderTextureLod) : 0;
								rules |= caps.standard_derivatives_support ? static_cast<uint32_t>(GSR_OESStandardDerivatives) : 0;
								rules |= caps.max_simultaneous_rts > 1 ? static_cast<uint32_t>(GSR_EXTDrawBuffers) : 0;
								rules &= ~GSR_VersionDecl;
							}
							else
							{
								rules |= caps.max_simultaneous_rts > 1 ? static_cast<uint32_t>(GSR_DrawBuffers) : 0;
								if (!caps.ubo_support)
								{
									rules &= ~GSR_UseUBO;
								}
							}
							dxbc2glsl.FeedDXBC(&code[0],
								false, static_cast<ShaderTessellatorPartitioning>(ds_partitioning_),
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
#else
				UNREF_PARAM(tech);
				UNREF_PARAM(pass);
#endif
			}
		}

		void OGLESShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/,
				RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & shared_so)
		{
			OGLESShaderObjectPtr so = checked_pointer_cast<OGLESShaderObject>(shared_so);

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

		void OGLESShaderObject::LinkShaders(RenderEffect const & effect)
		{
			UNREF_PARAM(effect);

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
