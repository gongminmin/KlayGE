/**
 * @file PlatformDeployer.cpp
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
#include <KFL/Hash.hpp>
#include <KFL/StringUtil.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/JudaTexture.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/XMLDom.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <regex>

#include <nonstd/scope.hpp>

#ifndef KLAYGE_DEBUG
#define CXXOPTS_NO_RTTI
#endif
#include <cxxopts.hpp>

#include <KlayGE/ToolCommon.hpp>
#include <KlayGE/DevHelper/PlatformDefinition.hpp>
#include <KlayGE/DevHelper/MeshConverter.hpp>
#include <KlayGE/DevHelper/MeshMetadata.hpp>
#include <KlayGE/DevHelper/TexConverter.hpp>
#include <KlayGE/DevHelper/TexMetadata.hpp>

using namespace std;
using namespace KlayGE;

TexMetadata DefaultTextureMetadata(size_t res_type_hash, RenderDeviceCaps const & caps)
{
	TexMetadata default_metadata;
	switch (res_type_hash)
	{
	case CT_HASH("albedo"):
	case CT_HASH("emissive"):
		default_metadata.Slot((res_type_hash == CT_HASH("albedo")) ? RenderMaterial::TS_Albedo : RenderMaterial::TS_Emissive);
		default_metadata.ForceSRGB(true);
		break;

	case CT_HASH("metalness_glossiness"):
		default_metadata.Slot(RenderMaterial::TS_MetalnessGlossiness);
		default_metadata.ForceSRGB(false);
		break;

	case CT_HASH("normal"):
	case CT_HASH("bump"):
		default_metadata.Slot(RenderMaterial::TS_Normal);
		default_metadata.ForceSRGB(false);
		if (res_type_hash == CT_HASH("bump"))
		{
			default_metadata.BumpToNormal(true);
			default_metadata.BumpScale(1.0f);
		}
		break;

	case CT_HASH("height"):
		default_metadata.Slot(RenderMaterial::TS_Height);
		default_metadata.ForceSRGB(false);
		break;
	}

	default_metadata.MipmapEnabled(true);
	default_metadata.AutoGenMipmap(true);

	default_metadata.DeviceDependentAdjustment(caps);

	return default_metadata;
}

TexMetadata LoadTextureMetadata(std::string const & res_name, TexMetadata const & default_metadata)
{
	std::string metadata_name = res_name + ".kmeta";
	if (ResLoader::Instance().Locate(metadata_name).empty())
	{
		return default_metadata;
	}
	else
	{
		return TexMetadata(metadata_name);
	}
}

MeshMetadata LoadMeshMetadata(std::string const & res_name, MeshMetadata const & default_metadata)
{
	std::string metadata_name = res_name + ".kmeta";
	if (ResLoader::Instance().Locate(metadata_name).empty())
	{
		return default_metadata;
	}
	else
	{
		return MeshMetadata(metadata_name);
	}
}

void Deploy(std::vector<std::string> const& res_names, std::string_view res_type, RenderDeviceCaps const& caps, std::string_view platform,
	std::string_view dest_folder)
{
	size_t const res_type_hash = HashRange(res_type.begin(), res_type.end());

	if ((CT_HASH("albedo") == res_type_hash)
		|| (CT_HASH("emissive") == res_type_hash)
		|| (CT_HASH("glossiness") == res_type_hash)
		|| (CT_HASH("metalness") == res_type_hash)
		|| (CT_HASH("normal") == res_type_hash)
		|| (CT_HASH("bump") == res_type_hash)
		|| (CT_HASH("height") == res_type_hash))
	{
		TexMetadata const default_metadata = DefaultTextureMetadata(res_type_hash, caps);

		TexConverter tc;
		for (size_t i = 0; i < res_names.size(); ++ i)
		{
			std::string_view real_res_type;
			auto metadata = LoadTextureMetadata(res_names[i], default_metadata);
			switch (metadata.Slot())
			{
			case RenderMaterial::TS_Albedo:
				real_res_type = "albedo";
				break;
			case RenderMaterial::TS_MetalnessGlossiness:
				real_res_type = "metalness & glossiness";
				break;
			case RenderMaterial::TS_Emissive:
				real_res_type = "emissive";
				break;
			case RenderMaterial::TS_Normal:
				real_res_type = "normal";
				break;
			case RenderMaterial::TS_Height:
				real_res_type = "height";
				break;
			case RenderMaterial::TS_Occlusion:
				real_res_type = "occlusion";
				break;

			default:
				KFL_UNREACHABLE("Invalid texture slot");
			}

			std::cout << "Converting " << res_names[i] << " to " << real_res_type << std::endl;

			auto output_tex = tc.Load(res_names[i], metadata);
			if (output_tex)
			{
				FILESYSTEM_NS::path res_path(res_names[i]);
				if (!dest_folder.empty())
				{
					res_path = FILESYSTEM_NS::path(dest_folder.begin(), dest_folder.end()) / res_path.filename();
				}
				SaveTexture(output_tex, res_path.string() + ".dds");
			}
		}
	}
	else if (CT_HASH("model") == res_type_hash)
	{
		MeshMetadata const default_metadata;

		MeshConverter mc;
		for (size_t i = 0; i < res_names.size(); ++ i)
		{
			std::cout << "Converting " << res_names[i] << " to " << res_type << std::endl;

			auto metadata = LoadMeshMetadata(res_names[i], default_metadata);
			auto output_model = mc.Load(res_names[i], metadata);
			if (output_model)
			{
				FILESYSTEM_NS::path res_path(res_names[i]);
				if (!dest_folder.empty())
				{
					res_path = FILESYSTEM_NS::path(dest_folder.begin(), dest_folder.end()) / res_path.filename();
				}
				SaveModel(*output_model, res_path.string() + ".model_bin");
			}
		}
	}
	else
	{
		std::ofstream ofs("convert.bat");

		if (CT_HASH("cubemap") == res_type_hash)
		{
			std::string y_fmt;
			std::string c_fmt;
			if (caps.BestMatchTextureFormat(MakeSpan({EF_R16, EF_R16F})) == EF_R16)
			{
				y_fmt = "R16";
			}
			else
			{
				y_fmt = "R16F";
			}
			if (caps.BestMatchTextureFormat(MakeSpan({EF_BC5, EF_BC3})) == EF_BC5)
			{
				c_fmt = "BC5";
			}
			else
			{
				c_fmt = "BC3";
			}

			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				std::cout << "Converting " << res_names[i] << " to " << res_type << std::endl;

				ofs << "@echo Processing: " << res_names[i] << std::endl;

				ofs << "@echo off" << std::endl << std::endl;
				ofs << "HDRCompressor \"" << res_names[i] << "\" " << y_fmt << ' ' << c_fmt;
				if (!dest_folder.empty())
				{
					ofs << " \"" << dest_folder << "\"";
				}
				ofs << std::endl;
				ofs << "@echo on" << std::endl << std::endl;
			}
		}
		else if (CT_HASH("effect") == res_type_hash)
		{
			for (size_t i = 0; i < res_names.size(); ++ i)
			{
				std::cout << "Converting " << res_names[i] << " to " << res_type << std::endl;

				ofs << "@echo Processing: " << res_names[i] << std::endl;

				ofs << "@echo off" << std::endl << std::endl;
				ofs << "FxmlJit -P " << platform << " -I \"" << res_names[i] << "\"";
				if (!dest_folder.empty())
				{
					ofs << " -D \"" << dest_folder << "\"";
				}
				ofs << std::endl;
				ofs << "@echo on" << std::endl << std::endl;
			}
		}
		else
		{
			std::cout << "Error: Unknown resource type." << std::endl;
		}

		ofs.close();

		int err = system("convert.bat");
		err = system("del convert.bat");
		KFL_UNUSED(err);
	}
}

int main(int argc, char* argv[])
{
	auto on_exit = nonstd::make_scope_exit([] { Context::Destroy(); });

	std::vector<std::string> res_names;
	std::string res_type;
	std::string platform;
	std::string dest_folder;

	cxxopts::Options options("PlatformDeployer", "KlayGE PlatformDeployer");
	// clang-format off
	options.add_options()
		("H,help", "Produce help message.")
		("I,input-path", "Input resource path.", cxxopts::value<std::string>())
		("T,type", "Resource type (auto by default).", cxxopts::value<std::string>())
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
		cout << "KlayGE PlatformDeployer, Version 2.0.0" << endl;
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
				res_names.push_back(std::string(arg));
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
							res_names.push_back((parent / name).string());
						}
					}
				}
			}
		}
	}
	else
	{
		cout << "Need input resources names." << endl;
		cout << options.help() << endl;
		return 1;
	}

	for (auto iter = res_names.begin(); iter != res_names.end();)
	{
		std::string const res_name = *iter;

		bool need_convert = true;
		std::string possible_asset_name;
		std::string const full_res_name = ResLoader::Instance().Locate(res_name);
		if (full_res_name.empty())
		{
			cout << "Could NOT find " << res_name << '.';

			std::string const possible_dds_name = ResLoader::Instance().Locate(res_name + ".dds");
			if (FILESYSTEM_NS::exists(possible_dds_name))
			{
				possible_asset_name = possible_dds_name;
			}
			else
			{
				std::string const possible_model_bin_name = ResLoader::Instance().Locate(res_name + ".model_bin");
				if (FILESYSTEM_NS::exists(possible_model_bin_name))
				{
					possible_asset_name = possible_model_bin_name;
				}
			}

			if (!possible_asset_name.empty())
			{
				cout << " But " << possible_asset_name << " does exist." << endl;
				need_convert = false;
			}
			else
			{
				return 1;
			}
		}

		if (need_convert)
		{
			++iter;
		}
		else
		{
			iter = res_names.erase(iter);

			FILESYSTEM_NS::path res_path(possible_asset_name);
			if (dest_folder != res_path.parent_path())
			{
				FILESYSTEM_NS::copy_file(res_path, dest_folder / res_path.filename(), FILESYSTEM_NS::copy_options::overwrite_existing);
			}
		}
	}
	if (res_names.empty())
	{
		return 0;
	}

	if (vm.count("type") > 0)
	{
		res_type = vm["type"].as<std::string>();
	}
	else
	{
		if (TexConverter::IsSupported(res_names[0]))
		{
			res_type = "albedo";
		}
		else if (MeshConverter::IsSupported(res_names[0]))
		{
			res_type = "model";
		}
		else
		{
			cout << "Need resource type name." << endl;
			return 1;
		}
	}
	if (vm.count("platform") > 0)
	{
		platform = vm["platform"].as<std::string>();
	}
	else
	{
		platform = "d3d_11_0";
	}

	StringUtil::ToLower(res_type);
	StringUtil::ToLower(platform);

	if (("pc_dx11" == platform) || ("pc_dx10" == platform) || ("pc_dx9" == platform) || ("win_tegra3" == platform)
		|| ("pc_gl4" == platform) || ("pc_gl3" == platform) || ("pc_gl2" == platform)
		|| ("android_tegra3" == platform) || ("ios" == platform))
	{
		if ("pc_dx11" == platform)
		{
			platform = "d3d_11_0";
		}
		else if ("pc_dx10" == platform)
		{
			platform = "d3d_10_0";
		}
		else if ("pc_dx9" == platform)
		{
			platform = "d3d_9_3";
		}
		else if ("win_tegra3" == platform)
		{
			platform = "d3d_9_1";
		}
		else if ("pc_gl4" == platform)
		{
			platform = "gl_4_0";
		}
		else if ("pc_gl3" == platform)
		{
			platform = "gl_3_0";
		}
		else if ("pc_gl2" == platform)
		{
			platform = "gl_2_0";
		}
		else if ("android_tegra3" == platform)
		{
			platform = "gles_2_0";
		}
		else if ("ios" == platform)
		{
			platform = "gles_2_0";
		}
	}

	PlatformDefinition platform_def(platform + ".plat");
	Deploy(res_names, res_type, platform_def.device_caps, platform, dest_folder);

	return 0;
}
