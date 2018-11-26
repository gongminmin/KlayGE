/**
 * @file ImageConv.cpp
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
#include <KFL/CXX17/filesystem.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/ResLoader.hpp>

#include <iostream>
#include <vector>
#include <string>

#ifndef KLAYGE_DEBUG
#define CXXOPTS_NO_RTTI
#endif
#include <cxxopts.hpp>

#include <KlayGE/DevHelper/PlatformDefinition.hpp>
#include <KlayGE/DevHelper/TexConverter.hpp>
#include <KlayGE/DevHelper/TexMetadata.hpp>

using namespace std;
using namespace KlayGE;

int main(int argc, char* argv[])
{
	std::string input_name;
	std::string metadata_name;
	std::string output_name;
	std::string target_folder;
	std::string platform;
	bool quiet = false;

	cxxopts::Options options("ImageConv", "KlayGE Image Converter");
	options.add_options()
		("H,help", "Produce help message.")
		("I,input-path", "Input image path.", cxxopts::value<std::string>())
		("M,metadata-path", "(Optional) Input metadata path.", cxxopts::value<std::string>())
		("O,output-path", "(Optional) Output image path.", cxxopts::value<std::string>())
		("T,target-folder", "Target folder.", cxxopts::value<std::string>())
		("P,platform", "Platform name.", cxxopts::value<std::string>())
		("q,quiet", "Quiet mode.", cxxopts::value<bool>()->implicit_value("true"))
		("V,version", "Version.");

	int const argc_backup = argc;
	auto vm = options.parse(argc, argv);

	if ((argc_backup <= 1) || (vm.count("help") > 0))
	{
		cout << options.help() << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE Image Converter, Version 2.0.0" << endl;
		return 1;
	}
	if (vm.count("input-path") > 0)
	{
		input_name = vm["input-path"].as<std::string>();
	}
	else
	{
		cout << "Need input image path." << endl;
		cout << options.help() << endl;
		return 1;
	}
	if (vm.count("metadata-path") > 0)
	{
		metadata_name = vm["metadata-path"].as<std::string>();
	}
	if (vm.count("target-folder") > 0)
	{
		target_folder = vm["target-folder"].as<std::string>();
	}
	if (vm.count("output-path") > 0)
	{
		output_name = vm["output-path"].as<std::string>();
	}
	if (vm.count("platform") > 0)
	{
		platform = vm["platform"].as<std::string>();
	}
	else
	{
		platform = "d3d_11_0";
	}
	if (vm.count("quiet") > 0)
	{
		quiet = vm["quiet"].as<bool>();
	}

	std::string const full_input_name = ResLoader::Instance().Locate(input_name);
	if (full_input_name.empty())
	{
		int ret;
		cout << "Could NOT find " << input_name << '.';

		std::string const possible_output_name = ResLoader::Instance().Locate(input_name + ".dds");
		if (std::filesystem::exists(possible_output_name))
		{
			cout << " But " << possible_output_name << " does exist.";
			ret = 0;
		}
		else
		{
			ret = 1;
		}

		cout << endl;

		Context::Destroy();
		return ret;
	}

	if (metadata_name.empty())
	{
		metadata_name = full_input_name + ".kmeta";
	}
	if (output_name.empty())
	{
		if (target_folder.empty())
		{
			output_name = full_input_name + ".dds";
		}
		else
		{
			output_name = (target_folder / filesystem::path(full_input_name).filename()).string() + ".dds";
		}
	}

	bool conversion = false;
	filesystem::path const output_path(output_name);
	if (output_path.extension() == ".dds")
	{
		if (ResLoader::Instance().Locate(output_name).empty())
		{
			conversion = true;
		}
		else
		{
			uint64_t const output_file_timestamp = ResLoader::Instance().Timestamp(output_name);
			uint64_t const input_file_timestamp = ResLoader::Instance().Timestamp(full_input_name);
			uint64_t const metadata_timestamp = ResLoader::Instance().Timestamp(metadata_name);
			if (((input_file_timestamp > 0) && (output_file_timestamp < input_file_timestamp))
				|| (((metadata_timestamp > 0) && (output_file_timestamp < metadata_timestamp))))
			{
				conversion = true;
			}
		}
	}
	else
	{
		conversion = true;
	}

	if (conversion)
	{
		PlatformDefinition platform_def(platform + ".plat");

		TexMetadata metadata;
		if (!ResLoader::Instance().Locate(metadata_name).empty())
		{
			metadata.Load(metadata_name);
		}
		metadata.DeviceDependentAdjustment(platform_def.device_caps);

		TexConverter tc;
		TexturePtr output_tex = tc.Load(full_input_name, metadata);
		if (output_tex)
		{
			SaveTexture(output_tex, output_name);

			if (!quiet)
			{
				cout << "Texture has been saved to " << output_name << "." << endl;
			}
		}
		else
		{
			LogError() << "FAIL to convert file " << full_input_name << " with metadata " << metadata_name << std::endl;
		}
	}
	else
	{
		cout << "Target file " << output_name << " is up-to-date. No need to do the conversion." << std::endl;
	}

	Context::Destroy();

	return 0;
}
