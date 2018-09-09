#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/filesystem.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Mesh.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
	std::string target_folder;
	bool quiet = false;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-path,I", boost::program_options::value<std::string>(), "Input mesh path.")
		("metadata-path,M", boost::program_options::value<std::string>(), "Input metadata path.")
		("output-path,O", boost::program_options::value<std::string>(), "(Optional) Output mesh path.")
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
	
	std::string file_name = ResLoader::Instance().Locate(input_name);
	if (file_name.empty())
	{
		cout << "Could NOT find " << input_name << endl;
		Context::Destroy();
		return 1;
	}

	if (metadata_name.empty())
	{
		metadata_name = file_name + ".kmeta";
	}
	if (output_name.empty())
	{
		if (target_folder.empty())
		{
			output_name = file_name + ".model_bin";
		}
		else
		{
			output_name = (target_folder / filesystem::path(file_name).filename()).string() + ".model_bin";
		}
	}

	bool conversion = false;
	filesystem::path const output_path(output_name);
	if (output_path.extension() == ".model_bin")
	{
		uint32_t const MODEL_BIN_VERSION = 15;

		ResIdentifierPtr output_file = ResLoader::Instance().Open(output_name);
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
			uint64_t const input_file_timestamp = ResLoader::Instance().Timestamp(file_name);
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

	if (conversion)
	{
		MeshMetadata metadata;
		if (!ResLoader::Instance().Locate(metadata_name).empty())
		{
			metadata.Load(metadata_name);
		}

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
	}
	else
	{
		cout << "Target file " << output_name << " is up-to-dated. No need to do the conversion." << std::endl;
	}

	Context::Destroy();

	return 0;
}
