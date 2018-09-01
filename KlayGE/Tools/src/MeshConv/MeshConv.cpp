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

using namespace std;
using namespace KlayGE;

int main(int argc, char* argv[])
{
	std::string input_name;
	std::string metadata_name;
	std::string output_name;
	bool quiet = false;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-path,I", boost::program_options::value<std::string>(), "Input mesh path.")
		("metadata-path,M", boost::program_options::value<std::string>(), "Input metadata path.")
		("output-path,O", boost::program_options::value<std::string>(), "(Optional) Output image path.")
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
	if (vm.count("output-path") > 0)
	{
		output_name = vm["output-path"].as<std::string>();
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

	filesystem::path input_folder = filesystem::path(ResLoader::Instance().Locate(file_name)).parent_path();
	if (output_name.empty())
	{
		filesystem::path const input_path(input_name);
		filesystem::path const base_name = input_path.stem();
		output_name = (input_folder / base_name).string() + ".meshml";
	}

	MeshMetadata metadata(metadata_name);

	MeshConverter mesh_converter;
	auto model = mesh_converter.Convert(file_name, metadata);

	if (model)
	{
		SaveModel(model, output_name);

		if (!quiet)
		{
			for (uint32_t lod = 0; lod < model->NumLods(); ++ lod)
			{
				size_t num_vertices = 0;
				size_t num_triangles = 0;
				for (uint32_t mindex = 0; mindex < model->NumSubrenderables(); ++ mindex)
				{
					auto const & mesh = *checked_cast<StaticMesh*>(model->Subrenderable(mindex).get());

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
		LogError() << "FAIL to convert file " << file_name << " with metadata " << metadata_name << std::endl;
	}

	Context::Destroy();

	return 0;
}
