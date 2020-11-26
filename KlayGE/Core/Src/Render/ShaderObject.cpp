/**
 * @file ShaderObject.cpp
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
#include <KFL/CustomizedStreamBuf.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/com_ptr.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/CustomizedStreamBuf.hpp>

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>

#include <KlayGE/ShaderObject.hpp>

#if KLAYGE_IS_DEV_PLATFORM

#ifdef KLAYGE_PLATFORM_WINDOWS
#define CALL_D3DCOMPILER_DIRECTLY
#endif

#ifdef CALL_D3DCOMPILER_DIRECTLY
#include <KlayGE/SALWrapper.hpp>
#include <d3dcompiler.h>
#else
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
typedef char const * LPCSTR;
typedef long HRESULT;

#define S_OK                                        0x00000000

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

struct D3D_SHADER_MACRO
{
	LPCSTR Name;
	LPCSTR Definition;
};
#endif

namespace
{
	using namespace KlayGE;

	class D3DCompilerLoader
	{
	public:
		~D3DCompilerLoader()
		{
#ifdef CALL_D3DCOMPILER_DIRECTLY
			::FreeLibrary(mod_d3dcompiler_);
#endif
		}

		static D3DCompilerLoader& Instance()
		{
			static D3DCompilerLoader initer;
			return initer;
		}

		HRESULT D3DCompile(std::string const & src_data,
			D3D_SHADER_MACRO const * defines, char const * entry_point,
			char const * target, uint32_t flags1, uint32_t flags2,
			std::vector<uint8_t>& code, std::string& error_msgs) const
		{
#ifdef CALL_D3DCOMPILER_DIRECTLY
			com_ptr<ID3DBlob> code_blob;
			com_ptr<ID3DBlob> error_msgs_blob;
			HRESULT hr = DynamicD3DCompile_(src_data.c_str(), static_cast<UINT>(src_data.size()),
				nullptr, defines, nullptr, entry_point,
				target, flags1, flags2, code_blob.put(), error_msgs_blob.put());
			if (code_blob)
			{
				uint8_t const * p = static_cast<uint8_t const *>(code_blob->GetBufferPointer());
				code.assign(p, p + code_blob->GetBufferSize());
			}
			else
			{
				code.clear();
			}
			if (error_msgs_blob)
			{
				char const * p = static_cast<char const *>(error_msgs_blob->GetBufferPointer());
				error_msgs.assign(p, p + error_msgs_blob->GetBufferSize());
			}
			else
			{
				error_msgs.clear();
			}
			return hr;
#else
			std::string mark = std::to_string(reinterpret_cast<uint64_t>(src_data.c_str()));
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
			
			std::ostringstream ss;
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
				ss << KFL_STRINGIZE(WINE_PATH) << "wineserver -p";
				int err = system(ss.str().c_str());
				KFL_UNUSED(err);
				// We should hold on a persistant wineserver, or XCode will lost connection after wineserver instance close and wine may not be able to find '.exe.so' file
				first = false;
				ss.str(std::string());
			}
			d3dcompiler_wrapper_name += ".exe.so";
			std::string wrapper_path = ResLoader::Instance().Locate(d3dcompiler_wrapper_name);
			ss << KFL_STRINGIZE(WINE_PATH) << "wine " << wrapper_path;
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

		HRESULT D3DReflect(std::vector<uint8_t> const & shader_code, void** reflector)
		{
#ifdef CALL_D3DCOMPILER_DIRECTLY
			static GUID const IID_ID3D11ShaderReflection_47
				= { 0x8d536ca1, 0x0cca, 0x4956, { 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84 } };

			return DynamicD3DReflect_(&shader_code[0], static_cast<UINT>(shader_code.size()), IID_ID3D11ShaderReflection_47, reflector);
#else
			// TODO
			KFL_UNUSED(shader_code);
			KFL_UNUSED(reflector);
			return S_OK;
#endif
		}

		HRESULT D3DStripShader(std::vector<uint8_t> const & shader_code, uint32_t strip_flags, std::vector<uint8_t>& stripped_code)
		{
#ifdef CALL_D3DCOMPILER_DIRECTLY
			com_ptr<ID3DBlob> stripped_blob;
			HRESULT hr = DynamicD3DStripShader_(&shader_code[0], static_cast<UINT>(shader_code.size()), strip_flags, stripped_blob.put());

			uint8_t const * p = static_cast<uint8_t const *>(stripped_blob->GetBufferPointer());
			stripped_code.assign(p, p + stripped_blob->GetBufferSize());

			return hr;
#else
			// TODO
			KFL_UNUSED(shader_code);
			KFL_UNUSED(strip_flags);
			KFL_UNUSED(stripped_code);
			return S_OK;
#endif
		}

	private:
		D3DCompilerLoader()
		{
#ifdef CALL_D3DCOMPILER_DIRECTLY
			mod_d3dcompiler_ = ::LoadLibraryEx(TEXT("d3dcompiler_47.dll"), nullptr, 0);
			KLAYGE_ASSUME(mod_d3dcompiler_ != nullptr);

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
			DynamicD3DCompile_ = reinterpret_cast<D3DCompileFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DCompile"));
			DynamicD3DReflect_ = reinterpret_cast<D3DReflectFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DReflect"));
			DynamicD3DStripShader_ = reinterpret_cast<D3DStripShaderFunc>(::GetProcAddress(mod_d3dcompiler_, "D3DStripShader"));
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#endif
		}

	private:
#ifdef CALL_D3DCOMPILER_DIRECTLY
		typedef HRESULT (WINAPI *D3DCompileFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
			D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
			LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
		typedef HRESULT (WINAPI *D3DReflectFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector);
		typedef HRESULT (WINAPI *D3DStripShaderFunc)(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags,
			ID3DBlob** ppStrippedBlob);

		HMODULE mod_d3dcompiler_;
		D3DCompileFunc DynamicD3DCompile_;
		D3DReflectFunc DynamicD3DReflect_;
		D3DStripShaderFunc DynamicD3DStripShader_;
#endif
	};
}

#endif

namespace KlayGE
{
	ShaderStageObject::ShaderStageObject(ShaderStage stage) : stage_(stage)
	{
	}

	ShaderStageObject::~ShaderStageObject() noexcept = default;

#if KLAYGE_IS_DEV_PLATFORM
	std::vector<uint8_t> ShaderStageObject::CompileToDXBC(ShaderStage stage, RenderEffect const & effect,
		RenderTechnique const & tech, RenderPass const & pass,
		std::vector<std::pair<char const *, char const *>> const & api_special_macros,
		char const * func_name, char const * shader_profile, uint32_t flags)
	{
		RenderEngine const & re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();

		std::vector<uint8_t> code;

		std::string const & hlsl_shader_text = effect.HLSLShaderText();

		std::string max_sm_str = std::to_string(caps.max_shader_model.FullVersion());
		std::string max_tex_array_str = std::to_string(caps.max_texture_array_length);
		std::string max_tex_depth_str = std::to_string(caps.max_texture_depth);
		std::string max_tex_units_str = std::to_string(static_cast<int>(caps.max_pixel_texture_units));
		std::string flipping_str = std::to_string(re.RequiresFlipping() ? -1 : +1);
		std::string render_to_tex_array_str = std::to_string(caps.render_to_texture_array_support ? 1 : 0);

		std::string err_msg;
		std::vector<D3D_SHADER_MACRO> macros;

		for (uint32_t i = 0; i < api_special_macros.size(); ++i)
		{
			D3D_SHADER_MACRO macro = { api_special_macros[i].first, api_special_macros[i].second };
			macros.push_back(macro);
		}

		{
			D3D_SHADER_MACRO macro = { "KLAYGE_SHADER_MODEL", max_sm_str.c_str() };
			macros.push_back(macro);
		}
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_MAX_TEX_ARRAY_LEN", max_tex_array_str.c_str() };
			macros.push_back(macro);
		}
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_MAX_TEX_DEPTH", max_tex_depth_str.c_str() };
			macros.push_back(macro);
		}
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_MAX_TEX_UNITS", max_tex_units_str.c_str() };
			macros.push_back(macro);
		}
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_FLIPPING", flipping_str.c_str() };
			macros.push_back(macro);
		}
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_RENDER_TO_TEX_ARRAY", render_to_tex_array_str.c_str() };
			macros.push_back(macro);
		}
		if (!caps.fp_color_support)
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_NO_FP_COLOR", "1" };
			macros.push_back(macro);
		}
		if (caps.pack_to_rgba_required)
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_PACK_TO_RGBA", "1" };
			macros.push_back(macro);
		}
		if (caps.UavFormatSupport(EF_ABGR16F))
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_TYPED_UAV_SUPPORT", "1" };
			macros.push_back(macro);
		}
		if (caps.uavs_at_every_stage_support)
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_UAVS_AT_EVERY_STAGE_SUPPORT", "1" };
			macros.push_back(macro);
		}
		if (caps.explicit_multi_sample_support)
		{
			D3D_SHADER_MACRO macro = { "KLAYGE_EXPLICIT_MULTI_SAMPLE_SUPPORT", "1" };
			macros.push_back(macro);
		}
		if (caps.vp_rt_index_at_every_stage_support)
		{
			D3D_SHADER_MACRO macro = {"KLAYGE_VP_RT_INDEX_AT_EVERY_STAGE_SUPPORT", "1"};
			macros.push_back(macro);
		}
		{
			D3D_SHADER_MACRO macro_shader_type = { "", "1" };
			switch (stage)
			{
			case ShaderStage::Vertex:
				macro_shader_type.Name = "KLAYGE_VERTEX_SHADER";
				break;

			case ShaderStage::Pixel:
				macro_shader_type.Name = "KLAYGE_PIXEL_SHADER";
				break;

			case ShaderStage::Geometry:
				macro_shader_type.Name = "KLAYGE_GEOMETRY_SHADER";
				break;

			case ShaderStage::Compute:
				macro_shader_type.Name = "KLAYGE_COMPUTE_SHADER";
				break;

			case ShaderStage::Hull:
				macro_shader_type.Name = "KLAYGE_HULL_SHADER";
				break;

			case ShaderStage::Domain:
				macro_shader_type.Name = "KLAYGE_DOMAIN_SHADER";
				break;

			default:
				KFL_UNREACHABLE("Invalid shader stage");
			}
			macros.push_back(macro_shader_type);
		}

		for (uint32_t i = 0; i < tech.NumMacros(); ++i)
		{
			std::pair<std::string, std::string> const & name_value = tech.MacroByIndex(i);
			D3D_SHADER_MACRO macro = { name_value.first.c_str(), name_value.second.c_str() };
			macros.push_back(macro);
		}

		for (uint32_t i = 0; i < pass.NumMacros(); ++i)
		{
			std::pair<std::string, std::string> const & name_value = pass.MacroByIndex(i);
			D3D_SHADER_MACRO macro = { name_value.first.c_str(), name_value.second.c_str() };
			macros.push_back(macro);
		}

		{
			D3D_SHADER_MACRO macro_end = { nullptr, nullptr };
			macros.push_back(macro_end);
		}

		D3DCompilerLoader::Instance().D3DCompile(hlsl_shader_text, &macros[0],
			func_name, shader_profile,
			flags, 0, code, err_msg);
		if (!err_msg.empty())
		{
			LogError() << "Error when compiling " << func_name << ":" << std::endl;

			std::map<int, std::vector<std::string>> err_lines;
			{
				MemInputStreamBuf err_msg_buff(err_msg.data(), err_msg.size());
				std::istream err_iss(&err_msg_buff);

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
						MemInputStreamBuf stream_buff(part_err_str.data(), part_err_str.size());
						std::istream(&stream_buff) >> err_line;
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

			for (auto iter = err_lines.begin(); iter != err_lines.end(); ++iter)
			{
				if (iter->first >= 0)
				{
					MemInputStreamBuf hlsl_buff(hlsl_shader_text.data(), hlsl_shader_text.size());
					std::istream iss(&hlsl_buff);

					std::string s;
					int line = 1;

					LogInfo() << "..." << std::endl;
					while (iss && ((iter->first - line) >= 3))
					{
						std::getline(iss, s);
						++line;
					}
					while (iss && (abs(line - iter->first) < 3))
					{
						std::getline(iss, s);

						while (!s.empty() && (('\r' == s[s.size() - 1]) || ('\n' == s[s.size() - 1])))
						{
							s.resize(s.size() - 1);
						}

						LogInfo() << line << ' ' << s << std::endl;

						++line;
					}
					LogInfo() << "..." << std::endl;
				}

				for (auto const & msg : iter->second)
				{
					LogError() << msg << std::endl;
				}
			}
		}

		return code;
	}

	void ShaderStageObject::ReflectDXBC(std::vector<uint8_t> const & code, void** reflector)
	{
		D3DCompilerLoader::Instance().D3DReflect(code, reflector);
	}

	std::vector<uint8_t> ShaderStageObject::StripDXBC(std::vector<uint8_t> const & code, uint32_t strip_flags)
	{
		std::vector<uint8_t> ret;
		D3DCompilerLoader::Instance().D3DStripShader(code, strip_flags, ret);
		return ret;
	}
#endif


	ShaderObject::ShaderObject() : ShaderObject(MakeSharedPtr<ShaderObjectTemplate>())
	{
	}

	ShaderObject::ShaderObject(std::shared_ptr<ShaderObjectTemplate> so_template) : so_template_(std::move(so_template))
	{
	}

	ShaderObject::~ShaderObject() = default;

	void ShaderObject::AttachStage(ShaderStage stage, ShaderStageObjectPtr const& shader_stage)
	{
		auto& curr_shader_stage = so_template_->shader_stages_[static_cast<uint32_t>(stage)];
		if (curr_shader_stage != shader_stage)
		{
			curr_shader_stage = shader_stage;
			shader_stages_dirty_ = true;
			hw_res_ready_ = false;
		}
	}
	
	ShaderStageObjectPtr const& ShaderObject::Stage(ShaderStage stage) const
	{
		return so_template_->shader_stages_[static_cast<uint32_t>(stage)];
	}

	void ShaderObject::LinkShaders(RenderEffect const & effect)
	{
		if (shader_stages_dirty_)
		{
			is_validate_ = true;
			for (uint32_t stage_index = 0; stage_index < NumShaderStages; ++stage_index)
			{
				ShaderStage const stage = static_cast<ShaderStage>(stage_index);
				auto const& shader_stage = this->Stage(stage);
				if (shader_stage)
				{
					if (shader_stage->Validate())
					{
						this->CreateHwResources(stage, effect);
					}
					is_validate_ &= shader_stage->Validate();
				}
			}

			this->DoLinkShaders(effect);

			shader_stages_dirty_ = false;
			hw_res_ready_ = true;
		}
	}
}
