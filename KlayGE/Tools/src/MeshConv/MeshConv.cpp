/**
 * @file MeshConv.cpp
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
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Mesh.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#ifndef KLAYGE_DEBUG
#define CXXOPTS_NO_RTTI
#endif
#include <cxxopts.hpp>

#include <KlayGE/DevHelper/MeshMetadata.hpp>
#include <KlayGE/DevHelper/MeshConverter.hpp>

using namespace std;
using namespace KlayGE;

int main(int argc, char* argv[])
{
	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.ppaa = false;
	context_cfg.graphics_cfg.gamma = false;
	context_cfg.graphics_cfg.color_grading = false;
	Context::Instance().Config(context_cfg);

	std::string input_name;
	std::string metadata_name;
	std::string output_name;
	std::string target_folder;
	bool quiet = false;

	cxxopts::Options options("ImageConv", "KlayGE Mesh Converter");
	options.add_options()
		("H,help", "Produce help message")
		("I,input-path", "Input mesh path.", cxxopts::value<std::string>())
		("M,metadata-path", "(Optional) Input metadata path.", cxxopts::value<std::string>())
		("O,output-path", "(Optional) Output mesh path.", cxxopts::value<std::string>())
		("T,target-folder", "Target folder.", cxxopts::value<std::string>())
		("q,quiet", "Quiet mode.", cxxopts::value<bool>()->implicit_value("true"))
		("v,version", "Version.");

	int const argc_backup = argc;
	auto vm = options.parse(argc, argv);

	if ((argc_backup <= 1) || (vm.count("help") > 0))
	{
		cout << options.help() << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE Mesh Converter, Version 2.0.0" << endl;
		return 1;
	}
	if (vm.count("input-path") > 0)
	{
		input_name = vm["input-path"].as<std::string>();
	}
	else
	{
		cout << "Need input mesh path." << endl;
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
	if (vm.count("quiet") > 0)
	{
		quiet = vm["quiet"].as<bool>();
	}

	std::string const full_input_name = ResLoader::Instance().Locate(input_name);
	if (full_input_name.empty())
	{
		int ret;
		cout << "Could NOT find " << input_name << '.';

		std::string const possible_output_name = ResLoader::Instance().Locate(input_name + ".model_bin");
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
			output_name = full_input_name + ".model_bin";
		}
		else
		{
			output_name = (target_folder / filesystem::path(full_input_name).filename()).string() + ".model_bin";
		}
	}

	bool conversion = false;
	filesystem::path const output_path(output_name);
	if (output_path.extension() == ".model_bin")
	{
		uint32_t const MODEL_BIN_VERSION = 17;

		ResIdentifierPtr output_file = ResLoader::Instance().Open(output_name);
		if (output_file)
		{
			uint32_t fourcc;
			output_file->read(&fourcc, sizeof(fourcc));
			fourcc = LE2Native(fourcc);
			uint32_t ver;
			output_file->read(&ver, sizeof(ver));
			ver = LE2Native(ver);
			if ((fourcc != MakeFourCC<'K', 'L', 'M', ' '>::value) || (ver != MODEL_BIN_VERSION))
			{
				conversion = true;
			}
			else
			{
				uint64_t const output_file_timestamp = output_file->Timestamp();
				uint64_t const input_file_timestamp = ResLoader::Instance().Timestamp(full_input_name);
				uint64_t const metadata_timestamp = ResLoader::Instance().Timestamp(metadata_name);
				if (((input_file_timestamp > 0) && (output_file_timestamp < input_file_timestamp))
					|| ((metadata_timestamp > 0) && (output_file_timestamp < metadata_timestamp)))
				{
					conversion = true;
				}
			}
		}
		else
		{
			conversion = true;
		}
	}
	else
	{
		conversion = true;
	}

	if (conversion)
	{
		MeshMetadata metadata;
		if (!ResLoader::Instance().Locate(metadata_name).empty())
		{
			metadata.Load(metadata_name);
		}

		MeshConverter mesh_converter;
		auto model = mesh_converter.Load(full_input_name, metadata);

		if (model)
		{
			mesh_converter.Save(*model, output_name);

			if (!quiet)
			{
				for (uint32_t lod = 0; lod < model->NumLods(); ++ lod)
				{
					size_t num_vertices = 0;
					size_t num_triangles = 0;
					for (uint32_t mindex = 0; mindex < model->NumMeshes(); ++ mindex)
					{
						auto const& mesh = checked_cast<StaticMesh&>(*model->Mesh(mindex));

						num_vertices += mesh.NumVertices(lod);
						num_triangles += mesh.NumIndices(lod) / 3;
					}

					cout << "LOD " << lod << ": " << num_vertices << " vertices, " << num_triangles << " triangles." << endl;
				}

				cout << "Mesh has been saved to " << output_name << "." << endl;
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
