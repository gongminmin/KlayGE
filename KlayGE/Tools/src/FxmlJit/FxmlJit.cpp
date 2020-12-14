/**
 * @file FxmlJit.cpp
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
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/XMLDom.hpp>
#include <KFL/CXX17/filesystem.hpp>
#include <KFL/StringUtil.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <iostream>

#include <nonstd/scope.hpp>

#ifndef KLAYGE_DEBUG
#define CXXOPTS_NO_RTTI
#endif
#include <cxxopts.hpp>

#include <KlayGE/ToolCommon.hpp>
#include <KlayGE/DevHelper/PlatformDefinition.hpp>

using namespace std;
using namespace KlayGE;

uint32_t const KFX_VERSION = 0x0150;

int main(int argc, char* argv[])
{
	auto on_exit = nonstd::make_scope_exit([] { Context::Destroy(); });

	std::vector<std::string> input_names;
	std::string platform;
	std::string dest_folder;

	cxxopts::Options options("FxmlJit", "KlayGE fxml compiler");
	// clang-format off
	options.add_options()
		("H,help", "Produce help message.")
		("I,input-path", "Input resource path.", cxxopts::value<std::string>())
		("P,platform", "Platform name.", cxxopts::value<std::string>())
		("D,dest-folder", "Destination folder.", cxxopts::value<std::string>())
		("v,version", "Version.");
	// clang-format on

	int const argc_backup = argc;
	auto vm = options.parse(argc, argv);

	if ((argc_backup <= 1) || (vm.count("help") > 0))
	{
		cout << options.help() << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE fxml compiler, Version 2.0.0" << endl;
		return 1;
	}
	if (vm.count("dest-folder") > 0)
	{
		dest_folder = vm["dest-folder"].as<std::string>();
	}
	if (vm.count("input-path") > 0)
	{
		std::string input_name_str = vm["input-path"].as<std::string>();

		std::vector<std::string_view> tokens = StringUtil::Split(input_name_str, StringUtil::IsAnyOf(",;"));
		for (auto& arg : tokens)
		{
			arg = StringUtil::Trim(arg);
			if ((std::string::npos == arg.find('*')) && (std::string::npos == arg.find('?')))
			{
				input_names.push_back(std::string(arg));
			}
			else
			{
				FILESYSTEM_NS::path arg_path(arg.begin(), arg.end());
				auto const parent = arg_path.parent_path();
				auto const file_name = arg_path.filename();

				std::regex const filter(DosWildcardToRegex(file_name.string()));

				FILESYSTEM_NS::directory_iterator end_itr;
				for (FILESYSTEM_NS::directory_iterator i(parent); i != end_itr; ++i)
				{
					if (FILESYSTEM_NS::is_regular_file(i->status()))
					{
						std::smatch what;
						std::string const name = i->path().filename().string();
						if (std::regex_match(name, what, filter))
						{
							input_names.push_back((parent / name).string());
						}
					}
				}
			}
		}
	}
	else
	{
		cout << "Need input fxml names." << endl;
		cout << options.help() << endl;
		return 1;
	}

	if (vm.count("platform") > 0)
	{
		platform = vm["platform"].as<std::string>();
	}
	else
	{
		platform = "d3d_11_0";
	}

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.render_factory_name = "NullRender";
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.ppaa = false;
	context_cfg.graphics_cfg.gamma = false;
	context_cfg.graphics_cfg.color_grading = false;
	Context::Instance().Config(context_cfg);

	PlatformDefinition platform_def(platform + ".plat");

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	int major_version = platform_def.major_version;
	int minor_version = platform_def.minor_version;
	bool frag_depth_support = platform_def.frag_depth_support;
	re.SetCustomAttrib("PLATFORM", &platform_def.platform);
	re.SetCustomAttrib("MAJOR_VERSION", &major_version);
	re.SetCustomAttrib("MINOR_VERSION", &minor_version);
	re.SetCustomAttrib("NATIVE_SHADER_FOURCC", &platform_def.native_shader_fourcc);
	re.SetCustomAttrib("NATIVE_SHADER_VERSION", &platform_def.native_shader_version);
	re.SetCustomAttrib("REQUIRES_FLIPPING", &platform_def.requires_flipping);
	re.SetCustomAttrib("DEVICE_CAPS", &platform_def.device_caps);
	re.SetCustomAttrib("FRAG_DEPTH_SUPPORT", &frag_depth_support);

	for (auto const& fxml_name : input_names)
	{
		FILESYSTEM_NS::path const fxml_path(fxml_name);
		std::string const base_name = fxml_path.stem().string();
		FILESYSTEM_NS::path const fxml_directory = fxml_path.parent_path();
		ResLoader::Instance().AddPath(fxml_directory.string());

		FILESYSTEM_NS::path const kfx_name = fxml_path.filename().replace_extension("kfx");
		FILESYSTEM_NS::path kfx_path = fxml_directory / kfx_name;
		bool skip_jit = false;
		if (FILESYSTEM_NS::exists(fxml_path) && FILESYSTEM_NS::exists(kfx_path))
		{
			ResIdentifierPtr source = ResLoader::Instance().Open(fxml_name);
			ResIdentifierPtr kfx_source = ResLoader::Instance().Open(kfx_path.string());

			uint64_t src_timestamp = source->Timestamp();

			uint32_t fourcc;
			kfx_source->read(&fourcc, sizeof(fourcc));
			fourcc = LE2Native(fourcc);

			uint32_t ver;
			kfx_source->read(&ver, sizeof(ver));
			ver = LE2Native(ver);

			if ((MakeFourCC<'K', 'F', 'X', ' '>::value == fourcc) && (KFX_VERSION == ver))
			{
				uint32_t shader_fourcc;
				kfx_source->read(&shader_fourcc, sizeof(shader_fourcc));
				shader_fourcc = LE2Native(shader_fourcc);

				uint32_t shader_ver;
				kfx_source->read(&shader_ver, sizeof(shader_ver));
				shader_ver = LE2Native(shader_ver);

				uint8_t shader_platform_name_len;
				kfx_source->read(&shader_platform_name_len, sizeof(shader_platform_name_len));
				std::string shader_platform_name(shader_platform_name_len, 0);
				kfx_source->read(&shader_platform_name[0], shader_platform_name_len);

				if ((re.NativeShaderFourCC() == shader_fourcc) && (re.NativeShaderVersion() == shader_ver) &&
					(re.NativeShaderPlatformName() == shader_platform_name))
				{
					uint64_t timestamp;
					kfx_source->read(&timestamp, sizeof(timestamp));
					timestamp = LE2Native(timestamp);
					if (src_timestamp <= timestamp)
					{
						skip_jit = true;
					}
				}
			}
		}

		if (!skip_jit)
		{
			std::vector<string> fxml_names;
			if (ResLoader::Instance().Locate(fxml_name).empty())
			{
				std::vector<std::string_view> frags = StringUtil::Split(base_name, StringUtil::EqualTo('+'));
				for (auto const& frag : frags)
				{
					fxml_names.push_back(ResLoader::Instance().Locate(std::string(frag) + ".fxml"));
				}
			}
			else
			{
				fxml_names.push_back(fxml_name);
			}

			RenderEffect effect;
			effect.Load(fxml_names);
			effect.CompileShaders();
		}
		if (!dest_folder.empty())
		{
			FILESYSTEM_NS::copy_file(kfx_path, dest_folder / kfx_name, FILESYSTEM_NS::copy_options::overwrite_existing);
			kfx_path = dest_folder / kfx_name;
		}

		if (FILESYSTEM_NS::exists(kfx_path))
		{
			cout << "Compiled kfx has been saved to " << kfx_path << "." << endl;
		}
		else
		{
			cout << "Couldn't find " << fxml_name << "." << endl;
		}
	}

	return 0;
}
