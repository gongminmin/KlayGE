#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Mesh.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <string>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/program_options.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <KlayGE/MeshMetadata.hpp>
#include <KlayGE/MeshConverter.hpp>
#include <MeshMLLib/MeshMLLib.hpp>

using namespace std;
using namespace KlayGE;

int main(int argc, char* argv[])
{
	std::string input_name;
	std::string metadata_name;
	filesystem::path target_folder;
	bool quiet = false;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-path,I", boost::program_options::value<std::string>(), "Input mesh path.")
		("metadata-path,M", boost::program_options::value<std::string>(), "Input metadata path.")
		("target-folder,T", boost::program_options::value<std::string>(), "Target folder.")
		("quiet,q", boost::program_options::value<bool>()->implicit_value(true), "Quiet mode.")
		("version,v", "Version.");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if ((argc <= 1) || (vm.count("help") > 0))
	{
		cout << desc << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE Mesh Converter, Version 1.0.0" << endl;
		return 1;
	}
	if (vm.count("input-path") > 0)
	{
		input_name = vm["input-path"].as<std::string>();
	}
	else
	{
		cout << "Need input mesh path." << endl;
		return 1;
	}
	if (vm.count("metadata-path") > 0)
	{
		metadata_name = vm["metadata-path"].as<std::string>();
	}
	else
	{
		metadata_name = input_name + ".kmeta";
	}
	if (vm.count("target-folder") > 0)
	{
		target_folder = vm["target-folder"].as<std::string>();
	}
	if (vm.count("quiet") > 0)
	{
		quiet = vm["quiet"].as<bool>();
	}
	
	std::string file_name = ResLoader::Instance().Locate(input_name);
	if (file_name.empty())
	{
		cout << "Could NOT find " << input_name << endl;
		Context::Destroy();
		return 1;
	}

	if (target_folder.empty())
	{
		target_folder = filesystem::path(ResLoader::Instance().Locate(file_name)).parent_path();
	}

	filesystem::path const input_path(input_name);
	filesystem::path const base_name = input_path.stem();
	std::string const output_name = (target_folder / base_name).string() + ".meshml";

	MeshMetadata metadata(metadata_name);

	MeshConverter mesh_converter;

	MeshMLObj meshml_obj(1.0f);
	int vertex_export_settings;
	if (mesh_converter.Convert(file_name, metadata, meshml_obj, vertex_export_settings))
	{
		std::ofstream ofs(output_name);
		meshml_obj.WriteMeshML(ofs, vertex_export_settings, 0);

		if (!quiet)
		{
			for (uint32_t lod = 0; lod < meshml_obj.NumMeshLods(0); ++ lod)
			{
				size_t num_vertices = 0;
				size_t num_triangles = 0;
				for (uint32_t mindex = 0; mindex < meshml_obj.NumMeshes(); ++ mindex)
				{
					num_vertices += meshml_obj.NumVertices(static_cast<int>(mindex), static_cast<int>(lod));
					num_triangles += meshml_obj.NumTriangles(static_cast<int>(mindex), static_cast<int>(lod));
				}

				cout << "LOD " << lod << ": " << num_vertices << " vertices, " << num_triangles << " triangles." << endl;
			}

			cout << "MeshML has been saved to " << output_name << "." << endl;
		}
	}
	else
	{
		LogError() << "FAIL to convert file " << file_name << " with metadata " << metadata_name << std::endl;
	}

	Context::Destroy();

	return 0;
}
