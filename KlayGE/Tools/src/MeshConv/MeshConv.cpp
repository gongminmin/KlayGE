#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/Mesh.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>

#if defined(KLAYGE_TS_LIBRARY_FILESYSTEM_V3_SUPPORT)
	#include <experimental/filesystem>
#elif defined(KLAYGE_TS_LIBRARY_FILESYSTEM_V2_SUPPORT)
	#include <filesystem>
	namespace std
	{
		namespace experimental
		{
			namespace filesystem = std::tr2::sys;
		}
	}
#else
	#if defined(KLAYGE_COMPILER_GCC)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
	#endif
	#include <boost/filesystem.hpp>
	#if defined(KLAYGE_COMPILER_GCC)
		#pragma GCC diagnostic pop
	#endif
	namespace std
	{
		namespace experimental
		{
			namespace filesystem = boost::filesystem;
		}
	}
#endif
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/program_options.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <MeshMLLib/MeshMLLib.hpp>

using namespace std;
using namespace KlayGE;
using namespace std::experimental;

namespace
{
	float3 Color4ToFloat3(aiColor4D const & c)
	{
		float3 v;
		v.x() = c.r;
		v.y() = c.g;
		v.z() = c.b;
		return v;
	}

	float4 Color4ToFloat4(aiColor4D const & c)
	{
		float4 v;
		v.x() = c.r;
		v.y() = c.g;
		v.z() = c.b;
		v.w() = c.a;
		return v;
	}

	struct Mesh
	{
		int mtl_id;
		std::string name;

		std::vector<float3> positions;
		std::vector<float3> normals;
		std::vector<Quaternion> tangent_quats;
		std::array<std::vector<float3>, AI_MAX_NUMBER_OF_TEXTURECOORDS> texcoords;

		std::vector<uint32_t> indices;

		bool has_normal;
		bool has_tangent_frame;
		std::array<bool, AI_MAX_NUMBER_OF_TEXTURECOORDS> has_texcoord;
	};

	void RecursiveTransformMesh(MeshMLObj& meshml_obj, aiNode const * node, std::vector<Mesh> const & meshes)
	{
		auto const mat = MathLib::transpose(float4x4(&node->mTransformation.a1));

		for (unsigned int n = 0; n < node->mNumMeshes; ++ n)
		{
			auto const & mesh = meshes[node->mMeshes[n]];

			int mesh_id = meshml_obj.AllocMesh();
			meshml_obj.SetMesh(mesh_id, mesh.mtl_id, mesh.name);

			for (unsigned int ti = 0; ti < mesh.indices.size(); ti += 3)
			{
				int tri_id = meshml_obj.AllocTriangle(mesh_id);
				meshml_obj.SetTriangle(mesh_id, tri_id, mesh.indices[ti + 0],
					mesh.indices[ti + 1], mesh.indices[ti + 2]);
			}

			for (unsigned int vi = 0; vi < mesh.positions.size(); ++ vi)
			{
				int vertex_id = meshml_obj.AllocVertex(mesh_id);

				std::vector<float3> texcoords;
				for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
				{
					if (mesh.has_texcoord[tci])
					{
						texcoords.push_back(mesh.texcoords[tci][vi]);
					}
				}

				auto const pos = MathLib::transform_coord(mesh.positions[vi], mat);
				if (mesh.has_tangent_frame)
				{
					auto const quat = mesh.tangent_quats[vi] * MathLib::to_quaternion(mat);
					meshml_obj.SetVertex(mesh_id, vertex_id, pos, quat, 2, texcoords);
				}
				else
				{
					auto const normal = MathLib::transform_normal(mesh.normals[vi], mat);
					meshml_obj.SetVertex(mesh_id, vertex_id, pos, normal, 2, texcoords);
				}
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++ i)
		{
			RecursiveTransformMesh(meshml_obj, node->mChildren[i], meshes);
		}
	}

	void ConvertMesh(std::string const & in_name, std::string const & out_name, float scale, bool swap_yz, bool inverse_z)
	{
		aiPropertyStore* props = aiCreatePropertyStore();
		aiSetImportPropertyInteger(props, AI_CONFIG_IMPORT_TER_MAKE_UVS, 1);
		aiSetImportPropertyFloat(props, AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80);
		aiSetImportPropertyInteger(props, AI_CONFIG_PP_SBP_REMOVE, 0);

		aiSetImportPropertyInteger(props, AI_CONFIG_GLOB_MEASURE_TIME, 1);

		unsigned int ppsteps = aiProcess_JoinIdenticalVertices // join identical vertices/ optimize indexing
			| aiProcess_ValidateDataStructure // perform a full validation of the loader's output
			| aiProcess_RemoveRedundantMaterials // remove redundant materials
			| aiProcess_FindInstances; // search for instanced meshes and remove them by references to one master

		aiScene const * scene = aiImportFileExWithProperties(in_name.c_str(),
			ppsteps // configurable pp steps
			| aiProcess_GenSmoothNormals // generate smooth normal vectors if not existing
			| aiProcess_Triangulate // triangulate polygons with more than 3 edges
			| aiProcess_ConvertToLeftHanded // convert everything to D3D left handed space
			| aiProcess_FixInfacingNormals, // find normals facing inwards and inverts them
			nullptr, props);

		aiReleasePropertyStore(props);

		MeshMLObj meshml_obj(scale);

		for (unsigned int mi = 0; mi < scene->mNumMaterials; ++ mi)
		{
			int mtl_id = meshml_obj.AllocMaterial();

			float3 ambient(0, 0, 0);
			float3 diffuse(0, 0, 0);
			float3 specular(0, 0, 0);
			float3 emit(0, 0, 0);
			float opacity = 1;
			float shininess = 1;

			aiColor4D ai_ambient;
			aiColor4D ai_diffuse;
			aiColor4D ai_specular;
			aiColor4D ai_emit;
			float ai_opacity;
			float ai_shininess;

			auto mtl = scene->mMaterials[mi];
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ai_ambient))
			{
				ambient = Color4ToFloat3(ai_ambient);
			}
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &ai_diffuse))
			{
				diffuse = Color4ToFloat3(ai_diffuse);
			}
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &ai_specular))
			{
				specular = Color4ToFloat3(ai_specular);
			}
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &ai_emit))
			{
				emit = Color4ToFloat3(ai_emit);
			}

			unsigned int max = 1;
			if (AI_SUCCESS == aiGetMaterialFloatArray(mtl, AI_MATKEY_OPACITY, &ai_opacity, &max))
			{
				opacity = ai_opacity;
			}

			max = 1;
			if (AI_SUCCESS == aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &ai_shininess, &max))
			{
				shininess = ai_shininess;

				max = 1;
				float strength;
				if (AI_SUCCESS == aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max))
				{
					shininess *= strength;
				}
			}

			meshml_obj.SetMaterial(mtl_id, ambient, diffuse, specular, emit, opacity, shininess);

			unsigned int count = aiGetMaterialTextureCount(mtl, aiTextureType_DIFFUSE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE, 0, &str, 0, 0, 0, 0, 0, 0);

				int slot_id = meshml_obj.AllocTextureSlot(mtl_id);
				meshml_obj.SetTextureSlot(mtl_id, slot_id, "Diffuse Color", str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_SPECULAR);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_SPECULAR, 0, &str, 0, 0, 0, 0, 0, 0);

				int slot_id = meshml_obj.AllocTextureSlot(mtl_id);
				meshml_obj.SetTextureSlot(mtl_id, slot_id, "Specular Color", str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_SHININESS);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_SHININESS, 0, &str, 0, 0, 0, 0, 0, 0);

				int slot_id = meshml_obj.AllocTextureSlot(mtl_id);
				meshml_obj.SetTextureSlot(mtl_id, slot_id, "Glossiness", str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_NORMALS);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_NORMALS, 0, &str, 0, 0, 0, 0, 0, 0);

				int slot_id = meshml_obj.AllocTextureSlot(mtl_id);
				meshml_obj.SetTextureSlot(mtl_id, slot_id, "Bump Map", str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_HEIGHT);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_HEIGHT, 0, &str, 0, 0, 0, 0, 0, 0);

				int slot_id = meshml_obj.AllocTextureSlot(mtl_id);
				meshml_obj.SetTextureSlot(mtl_id, slot_id, "Height Map", str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_EMISSIVE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_EMISSIVE, 0, &str, 0, 0, 0, 0, 0, 0);

				int slot_id = meshml_obj.AllocTextureSlot(mtl_id);
				meshml_obj.SetTextureSlot(mtl_id, slot_id, "Self-Illumination", str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_OPACITY);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_OPACITY, 0, &str, 0, 0, 0, 0, 0, 0);

				int slot_id = meshml_obj.AllocTextureSlot(mtl_id);
				meshml_obj.SetTextureSlot(mtl_id, slot_id, "Opacity", str.C_Str());
			}
		}

		std::vector<Mesh> meshes(scene->mNumMeshes);

		int vertex_export_settings = MeshMLObj::VES_None;
		for (unsigned int mi = 0; mi < scene->mNumMeshes; ++ mi)
		{
			aiMesh const * mesh = scene->mMeshes[mi];

			meshes[mi].mtl_id = mesh->mMaterialIndex;
			meshes[mi].name = mesh->mName.C_Str();

			unsigned int max = 1;
			int two_sided = 0;
			if (aiGetMaterialIntegerArray(scene->mMaterials[mesh->mMaterialIndex], AI_MATKEY_TWOSIDED, &two_sided, &max) != AI_SUCCESS)
			{
				two_sided = 0;
			}

			auto& indices = meshes[mi].indices;
			for (unsigned int fi = 0; fi < mesh->mNumFaces; ++ fi)
			{
				BOOST_ASSERT(3 == mesh->mFaces[fi].mNumIndices);

				indices.push_back(mesh->mFaces[fi].mIndices[0]);
				indices.push_back(mesh->mFaces[fi].mIndices[1]);
				indices.push_back(mesh->mFaces[fi].mIndices[2]);

				if (two_sided)
				{
					indices.push_back(mesh->mFaces[fi].mIndices[0]);
					indices.push_back(mesh->mFaces[fi].mIndices[2]);
					indices.push_back(mesh->mFaces[fi].mIndices[1]);
				}
			}

			bool has_normal = (mesh->mNormals != nullptr);
			bool has_tangent = (mesh->mTangents != nullptr);
			bool has_binormal = (mesh->mBitangents != nullptr);
			auto& has_texcoord = meshes[mi].has_texcoord;
			uint32_t first_texcoord = AI_MAX_NUMBER_OF_TEXTURECOORDS;
			for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
			{
				has_texcoord[tci] = (mesh->mTextureCoords[tci] != nullptr);
				if (has_texcoord[tci] && (AI_MAX_NUMBER_OF_TEXTURECOORDS == first_texcoord))
				{
					first_texcoord = tci;
				}
			}

			auto& positions = meshes[mi].positions;
			auto& normals = meshes[mi].normals;
			std::vector<float3> tangents(mesh->mNumVertices);
			std::vector<float3> binormals(mesh->mNumVertices);
			auto& texcoords = meshes[mi].texcoords;
			positions.resize(mesh->mNumVertices);
			normals.resize(mesh->mNumVertices);
			for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
			{
				texcoords[tci].resize(mesh->mNumVertices);
			}
			for (unsigned int vi = 0; vi < mesh->mNumVertices; ++ vi)
			{
				positions[vi] = float3(&mesh->mVertices[vi].x);
				if (inverse_z)
				{
					positions[vi].z() = -positions[vi].z();
				}
				if (swap_yz)
				{
					std::swap(positions[vi].y(), positions[vi].z());
				}

				if (has_normal)
				{
					normals[vi] = float3(&mesh->mNormals[vi].x);
					if (inverse_z)
					{
						normals[vi].z() = -normals[vi].z();
					}
					if (swap_yz)
					{
						std::swap(normals[vi].y(), normals[vi].z());
					}
				}
				if (has_tangent)
				{
					tangents[vi] = float3(&mesh->mTangents[vi].x);
					if (inverse_z)
					{
						tangents[vi].z() = -tangents[vi].z();
					}
					if (swap_yz)
					{
						std::swap(tangents[vi].y(), tangents[vi].z());
					}
				}
				if (has_binormal)
				{
					binormals[vi] = float3(&mesh->mBitangents[vi].x);
					if (inverse_z)
					{
						binormals[vi].z() = -binormals[vi].z();
					}
					if (swap_yz)
					{
						std::swap(binormals[vi].y(), binormals[vi].z());
					}
				}

				for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
				{
					if (has_texcoord[tci])
					{
						texcoords[tci][vi] = float3(&mesh->mTextureCoords[tci][vi].x);
					}
				}
			}

			if (!has_normal)
			{
				MathLib::compute_normal(normals.begin(), indices.begin(), indices.end(), positions.begin(), positions.end());

				has_normal = true;
			}

			auto& tangent_quats = meshes[mi].tangent_quats;
			tangent_quats.resize(mesh->mNumVertices);
			if ((!has_tangent || !has_binormal) && (first_texcoord != AI_MAX_NUMBER_OF_TEXTURECOORDS))
			{
				MathLib::compute_tangent(tangents.begin(), binormals.begin(), indices.begin(), indices.end(),
					positions.begin(), positions.end(), texcoords[first_texcoord].begin(), normals.begin());

				for (size_t i = 0; i < positions.size(); ++ i)
				{
					tangent_quats[i] = MathLib::to_quaternion(tangents[i], binormals[i], normals[i], 8);
				}

				has_tangent = true;
			}

			meshes[mi].has_normal = has_normal;
			meshes[mi].has_tangent_frame = has_tangent || has_binormal;

			if (has_tangent || has_binormal)
			{
				vertex_export_settings |= MeshMLObj::VES_TangentQuat;
			}
			if (first_texcoord != AI_MAX_NUMBER_OF_TEXTURECOORDS)
			{
				vertex_export_settings |= MeshMLObj::VES_Texcoord;
			}
		}

		for (unsigned int mi = 0; mi < scene->mNumMeshes; ++ mi)
		{
			if ((vertex_export_settings & MeshMLObj::VES_TangentQuat) && !meshes[mi].has_tangent_frame)
			{
				meshes[mi].has_tangent_frame = true;
			}
			if ((vertex_export_settings & MeshMLObj::VES_Texcoord) && !meshes[mi].has_texcoord[0])
			{
				meshes[mi].has_texcoord[0] = true;
			}
		}

		RecursiveTransformMesh(meshml_obj, scene->mRootNode, meshes);

		std::ofstream ofs(out_name.c_str());
		meshml_obj.WriteMeshML(ofs, vertex_export_settings, 0);

		aiReleaseImport(scene);
	}
}

int main(int argc, char* argv[])
{
	std::string input_name;
	filesystem::path target_folder;
	std::string platform;
	float scale = 1;
	bool swap_yz = false;
	bool inverse_z = false;
	bool quiet = false;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-path,I", boost::program_options::value<std::string>(), "Input mesh path.")
		("target-folder,T", boost::program_options::value<std::string>(), "Target folder.")
		("scale,S", boost::program_options::value<float>(), "Scale.")
		("swap-yz,W", "Swap Y and Z axis.")
		("inverse-z,Z", "Inverse Z axis.")
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
	if (vm.count("target-folder") > 0)
	{
		target_folder = vm["target-folder"].as<std::string>();
	}
	if (vm.count("scale") > 0)
	{
		scale = vm["scale"].as<float>();
	}
	if (vm.count("swap-yz") > 0)
	{
		swap_yz = true;
	}
	if (vm.count("inverse-z") > 0)
	{
		inverse_z = true;
	}
	if (vm.count("quiet") > 0)
	{
		quiet = vm["quiet"].as<bool>();
	}

	std::string file_name = ResLoader::Instance().Locate(input_name);
	if (file_name.empty())
	{
		cout << "Could NOT find " << input_name << endl;
		return 1;
	}

	filesystem::path input_path(file_name);
	filesystem::path base_name = input_path.stem();
	if (target_folder.empty())
	{
		target_folder = input_path.parent_path();
	}

	std::string output_name = (target_folder / base_name).string() + ".meshml";

	ConvertMesh(file_name, output_name, scale, swap_yz, inverse_z);

	if (!quiet)
	{
		cout << "MeshML has been saved to " << output_name << "." << endl;
	}

	Context::Destroy();

	return 0;
}
