/**
 * @file MeshConverter.cpp
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
#include <KFL/CXX2a/format.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Hash.hpp>
#include <KFL/Math.hpp>
#include <KFL/StringUtil.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/ResLoader.hpp>

#include <cstring>
#include <iostream>

#if defined(KLAYGE_COMPILER_GCC) && (KLAYGE_COMPILER_VERSION >= 90)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy" // Ignore comparison between int and uint
#endif
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-pack" // Ignore cross header #pragma pack
#endif
#include <assimp/scene.h>
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif
#include <assimp/pbrmaterial.h>

#include <KlayGE/DevHelper/MeshConverter.hpp>

using namespace std;
using namespace KlayGE;

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

	float3 AiVectorToFloat3(aiVector3D const& v)
	{
		return float3(v.x, v.y, v.z);
	}

	aiVector3D Float3ToAiVector(float3 const& v)
	{
		return aiVector3D(v.x(), v.y(), v.z());
	}

	Quaternion AiQuatToQuat(aiQuaternion const& v)
	{
		return Quaternion(v.x, v.y, v.z, v.w);
	}

	aiQuaternion QuatToAiQuat(Quaternion const& v)
	{
		return aiQuaternion(v.w(), v.x(), v.y(), v.z());
	}

	template <typename T>
	float GetInterpTime(std::vector<T> const & vec, float time, size_t& itime_lower, size_t& itime_upper)
	{
		BOOST_ASSERT(!vec.empty());

		if (vec.size() == 1)
		{
			itime_lower = 0;
			itime_upper = 0;
			return 0;
		}

		// use @itime_upper as a hint to speed up find
		size_t vec_size = vec.size();
		size_t i = 0;
		for (i = itime_upper; i < vec_size; ++ i)
		{
			if (vec[i].first >= time)
			{
				break;
			}
		}

		if (i == 0)
		{
			itime_lower = 0;
			itime_upper = 1;
		}
		else if (i >= vec.size() - 1)
		{
			itime_lower = vec_size - 2;
			itime_upper = vec_size - 1;
		}
		else
		{
			itime_lower = i - 1;
			itime_upper = i;
		}

		float diff = vec[itime_upper].first - vec[itime_lower].first;
		return MathLib::clamp((diff == 0) ? 0 : (time - vec[itime_lower].first) / diff, 0.0f, 1.0f);
	}

	void MatrixToDQ(float4x4 const & mat, Quaternion& bind_real, Quaternion& bind_dual, float& bind_scale)
	{
		float4x4 tmp_mat = mat;
		float flip = 1;
		if (MathLib::dot(MathLib::cross(float3(tmp_mat(0, 0), tmp_mat(0, 1), tmp_mat(0, 2)),
			float3(tmp_mat(1, 0), tmp_mat(1, 1), tmp_mat(1, 2))),
			float3(tmp_mat(2, 0), tmp_mat(2, 1), tmp_mat(2, 2))) < 0)
		{
			tmp_mat(2, 0) = -tmp_mat(2, 0);
			tmp_mat(2, 1) = -tmp_mat(2, 1);
			tmp_mat(2, 2) = -tmp_mat(2, 2);

			flip = -1;
		}

		float3 scale;
		float3 trans;
		MathLib::decompose(scale, bind_real, trans, tmp_mat);

		bind_dual = MathLib::quat_trans_to_udq(bind_real, trans);

		if (flip * MathLib::SignBit(bind_real.w()) < 0)
		{
			bind_real = -bind_real;
			bind_dual = -bind_dual;
		}

		bind_scale = scale.x();
	}

	float4x4 DQToMatrix(Quaternion const& bind_real, Quaternion const& bind_dual, float bind_scale)
	{
		return MathLib::scaling(bind_scale, bind_scale, bind_scale) * MathLib::udq_to_matrix(bind_real, bind_dual);
	}

	template <int N>
	void ExtractFVector(std::string_view value_str, float* v)
	{
		std::vector<std::string_view> strs = StringUtil::Split(value_str, StringUtil::EqualTo(' '));
		for (size_t i = 0; i < N; ++ i)
		{
			if (i < strs.size())
			{
				strs[i] = StringUtil::Trim(strs[i]);
				v[i] = std::stof(std::string(strs[i]));
			}
			else
			{
				v[i] = 0;
			}
		}
	}

	template <int N>
	void ExtractUIVector(std::string_view value_str, uint32_t* v)
	{
		std::vector<std::string_view> strs = StringUtil::Split(value_str, StringUtil::EqualTo(' '));
		for (size_t i = 0; i < N; ++ i)
		{
			if (i < strs.size())
			{
				strs[i] = StringUtil::Trim(strs[i]);
				v[i] = std::stoul(std::string(strs[i]));
			}
			else
			{
				v[i] = 0;
			}
		}
	}

	class MeshLoader
	{
	public:
		RenderModelPtr Load(std::string_view input_name, MeshMetadata const & metadata);

	private:
		void RemoveUnusedJoints();
		void RemoveUnusedMaterials();
		void CompressKeyFrameSet(KeyFrameSet& kf);

		// From assimp
		void BuildNodeData(uint32_t num_lods, uint32_t lod, int16_t parent_id, aiNode const * node);
		void BuildMaterials(aiScene const * scene);
		void BuildMeshData(std::vector<aiScene const*> const & scene_lods);
		void BuildJoints(aiScene const * scene);
		void BuildAnimations(aiScene const * scene);
		void ResampleJointTransform(KeyFrameSet& rkf, float4x4 const& parent_mat, int start_frame, int end_frame, float fps_scale,
			std::vector<std::pair<float, float3>> const & poss, std::vector<std::pair<float, Quaternion>> const & quats,
			std::vector<std::pair<float, float3>> const & scale);
		void LoadFromAssimp(std::string_view input_name, MeshMetadata const & metadata);

		// From MeshML
		void CompileMaterialsChunk(XMLNodePtr const & materials_chunk);
		void CompileMeshBoundingBox(XMLNodePtr const & mesh_node, uint32_t mesh_index,
			bool& recompute_pos_bb, bool& recompute_tc_bb);
		void CompileMeshesChunk(XMLNodePtr const & meshes_chunk);
		void CompileMeshLodChunk(XMLNodePtr const & lod_node, uint32_t mesh_index, uint32_t lod,
			bool recompute_pos_bb, bool recompute_tc_bb);
		void CompileMeshesVerticesChunk(XMLNodePtr const & vertices_chunk, uint32_t mesh_index, uint32_t lod,
			bool recompute_pos_bb, bool recompute_tc_bb);
		void CompileMeshesTrianglesChunk(XMLNodePtr const & triangles_chunk, uint32_t mesh_index, uint32_t lod);
		void CompileBonesChunk(XMLNodePtr const & bones_chunk);
		void CompileKeyFramesChunk(XMLNodePtr const & key_frames_chunk);
		void CompileBBKeyFramesChunk(XMLNodePtr const & bb_kfs_chunk, uint32_t mesh_index);
		void CompileActionsChunk(XMLNodePtr const & animations_chunk);
		void LoadFromMeshML(std::string_view input_name, MeshMetadata const & metadata);

	private:
		RenderModelPtr render_model_;

		static uint32_t constexpr MAX_NUMBER_OF_TEXTURECOORDS = 8;

		struct Mesh
		{
			int mtl_id;
			std::string name;

			struct Lod
			{
				std::vector<float3> positions;
				std::vector<float3> tangents;
				std::vector<float3> binormals;
				std::vector<float3> normals;
				std::vector<Color> diffuses;
				std::vector<Color> speculars;
				std::array<std::vector<float3>, MAX_NUMBER_OF_TEXTURECOORDS> texcoords;
				std::vector<std::vector<std::pair<uint32_t, float>>> joint_bindings;

				std::vector<uint32_t> indices;
			};
			std::vector<Lod> lods;

			bool has_normal;
			bool has_tangent_frame;
			std::array<bool, MAX_NUMBER_OF_TEXTURECOORDS> has_texcoord;

			AABBox pos_bb;
			AABBox tc_bb;
		};

		struct NodeTransform
		{
			SceneNodePtr node;

			std::vector<uint32_t> mesh_indices;

			AABBox aabb_local;

			int16_t parent_id;
		};

		struct JointInfo
		{
			std::string name;
			JointComponentPtr joint;
			int16_t parent_id;
		};

		std::vector<Mesh> meshes_;
		std::vector<NodeTransform> nodes_;
		std::vector<JointInfo> joints_;
		bool has_normal_;
		bool has_tangent_quat_;
		bool has_texcoord_;
		bool has_diffuse_;
		bool has_specular_;
	};

	class MeshSaver
	{
	public:
		void Save(RenderModel const & model, std::string_view output_name);

	private:
		void SaveByAssimp(RenderModel const & model, std::string_view output_name);
	};

	void MeshLoader::BuildNodeData(uint32_t num_lods, uint32_t lod, int16_t parent_id, aiNode const * node)
	{
		auto const trans_mat = MathLib::transpose(float4x4(&node->mTransformation.a1));
		std::wstring name;
		KlayGE::Convert(name, node->mName.C_Str());

		int16_t index = -1;
		if (lod == 0)
		{
			NodeTransform node_transform;
			node_transform.node = MakeSharedPtr<SceneNode>(name, SceneNode::SOA_Cullable);
			node_transform.node->TransformToParent(trans_mat);
			if (node->mNumMeshes > 0)
			{
				node_transform.mesh_indices.assign(node->mMeshes, node->mMeshes + node->mNumMeshes);
			}
			node_transform.parent_id = parent_id;
			if (parent_id >= 0)
			{
				nodes_[parent_id].node->AddChild(node_transform.node);
			}

			index = static_cast<int16_t>(nodes_.size());
			nodes_.push_back(node_transform);
		}
		else
		{
			bool found = false;
			for (size_t i = 0; i < nodes_.size(); ++ i)
			{
				if (((parent_id == -1) && (nodes_[i].parent_id == -1)) || (nodes_[i].node->Name() == name))
				{
					index = static_cast<int16_t>(i);
					if (node->mNumMeshes > 0)
					{
						nodes_[index].mesh_indices.assign(node->mMeshes, node->mMeshes + node->mNumMeshes);
					}
					found = true;

					break;
				}
			}

			if (!found)
			{
				LogError() << "Could NOT find the correspondence node between LoDs" << std::endl;
				Verify(false);
			}
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++ i)
		{
			this->BuildNodeData(num_lods, lod, index, node->mChildren[i]);
		}
	}

	void MeshLoader::BuildMaterials(aiScene const * scene)
	{
		render_model_->NumMaterials(scene->mNumMaterials);

		for (unsigned int mi = 0; mi < scene->mNumMaterials; ++ mi)
		{
			std::string name;
			float3 albedo(0, 0, 0);
			float metalness = 0;
			float shininess = 1;
			float3 emissive(0, 0, 0);
			float opacity = 1;
			bool transparent = false;
			bool two_sided = false;
			float alpha_test = 0;

			aiString ai_name;
			aiColor4D ai_albedo(0, 0, 0, 0);
			float ai_opacity = 1;
			float ai_metallic = 0;
			float ai_shininess = 1;
			aiColor4D ai_emissive(0, 0, 0, 0);
			int ai_two_sided = 0;
			float ai_alpha_test = 0;

			auto mtl = scene->mMaterials[mi];

			if (AI_SUCCESS == aiGetMaterialString(mtl, AI_MATKEY_NAME, &ai_name))
			{
				name = ai_name.C_Str();
			}

			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, &ai_albedo))
			{
				albedo = Color4ToFloat3(ai_albedo);
				opacity = ai_albedo.a;
			}
			else
			{
				if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &ai_albedo))
				{
					albedo = Color4ToFloat3(ai_albedo);
				}
				if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_OPACITY, &ai_opacity))
				{
					opacity = ai_opacity;
				}
			}

			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &ai_emissive))
			{
				emissive = Color4ToFloat3(ai_emissive);
			}

			if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, &ai_metallic))
			{
				metalness = ai_metallic;
			}

			if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &ai_shininess))
			{
				shininess = ai_shininess;
			}
			shininess = MathLib::clamp(shininess, 1.0f, MAX_SHININESS);

			if ((opacity < 1) || (aiGetMaterialTextureCount(mtl, aiTextureType_OPACITY) > 0))
			{
				transparent = true;
			}

			if (AI_SUCCESS == aiGetMaterialInteger(mtl, AI_MATKEY_TWOSIDED, &ai_two_sided))
			{
				two_sided = ai_two_sided ? true : false;
			}

			aiString ai_alpha_mode;
			if (AI_SUCCESS == aiGetMaterialString(mtl, AI_MATKEY_GLTF_ALPHAMODE, &ai_alpha_mode))
			{
				if (strcmp(ai_alpha_mode.C_Str(), "MASK") == 0)
				{
					if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_GLTF_ALPHACUTOFF, &ai_alpha_test))
					{
						alpha_test = ai_alpha_test;
					}
				}
				else if (strcmp(ai_alpha_mode.C_Str(), "BLEND") == 0)
				{
					transparent = true;
				}
				else if(strcmp(ai_alpha_mode.C_Str(), "OPAQUE") == 0)
				{
					transparent = false;
				}
			}

			render_model_->GetMaterial(mi) = MakeSharedPtr<RenderMaterial>();
			auto& render_mtl = *render_model_->GetMaterial(mi);
			render_mtl.Name(name);
			render_mtl.Albedo(float4(albedo.x(), albedo.y(), albedo.z(), opacity));
			render_mtl.Metalness(metalness);
			render_mtl.Glossiness(Shininess2Glossiness(shininess));
			render_mtl.Emissive(emissive);
			render_mtl.Transparent(transparent);
			render_mtl.AlphaTestThreshold(alpha_test);
			render_mtl.Sss(false);
			render_mtl.TwoSided(two_sided);

			unsigned int count = aiGetMaterialTextureCount(mtl, aiTextureType_DIFFUSE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE, 0, &str, 0, 0, 0, 0, 0, 0);
				render_mtl.TextureName(RenderMaterial::TS_Albedo, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_UNKNOWN);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &str, 0, 0, 0, 0, 0, 0);
				render_mtl.TextureName(RenderMaterial::TS_MetalnessGlossiness, str.C_Str());
			}
			else
			{
				count = aiGetMaterialTextureCount(mtl, aiTextureType_SHININESS);
				if (count > 0)
				{
					aiString str;
					aiGetMaterialTexture(mtl, aiTextureType_SHININESS, 0, &str, 0, 0, 0, 0, 0, 0);
					render_mtl.TextureName(RenderMaterial::TS_MetalnessGlossiness, str.C_Str());
				}
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_EMISSIVE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_EMISSIVE, 0, &str, 0, 0, 0, 0, 0, 0);
				render_mtl.TextureName(RenderMaterial::TS_Emissive, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_NORMALS);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_NORMALS, 0, &str, 0, 0, 0, 0, 0, 0);
				render_mtl.TextureName(RenderMaterial::TS_Normal, str.C_Str());

				float normal_scale;
				aiGetMaterialFloat(mtl, AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), &normal_scale);
				render_mtl.NormalScale(normal_scale);
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_HEIGHT);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_HEIGHT, 0, &str, 0, 0, 0, 0, 0, 0);
				render_mtl.TextureName(RenderMaterial::TS_Height, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_LIGHTMAP);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_LIGHTMAP, 0, &str, 0, 0, 0, 0, 0, 0);
				render_mtl.TextureName(RenderMaterial::TS_Occlusion, str.C_Str());

				float occlusion_strength;
				aiGetMaterialFloat(mtl, AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), &occlusion_strength);
				render_mtl.OcclusionStrength(occlusion_strength);
			}

			render_mtl.DetailMode(RenderMaterial::SurfaceDetailMode::ParallaxMapping);
			if (render_mtl.TextureName(RenderMaterial::TS_Height).empty())
			{
				render_mtl.HeightOffset(0);
				render_mtl.HeightScale(0);
			}
			else
			{
				render_mtl.HeightOffset(-0.5f);
				render_mtl.HeightScale(0.06f);

				float ai_bumpscaling;
				if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_BUMPSCALING, &ai_bumpscaling))
				{
					render_mtl.HeightScale(ai_bumpscaling);
				}
			}
			render_mtl.EdgeTessHint(5);
			render_mtl.InsideTessHint(5);
			render_mtl.MinTessFactor(1);
			render_mtl.MaxTessFactor(9);
		}
	}

	void MeshLoader::BuildMeshData(std::vector<aiScene const*> const & scene_lods)
	{
		for (size_t lod = 0; lod < scene_lods.size(); ++ lod)
		{
			for (unsigned int mi = 0; mi < scene_lods[lod]->mNumMeshes; ++ mi)
			{
				aiMesh const * mesh = scene_lods[lod]->mMeshes[mi];

				if (lod == 0)
				{
					meshes_[mi].mtl_id = mesh->mMaterialIndex;
					meshes_[mi].name = mesh->mName.C_Str();
				}

				auto& indices = meshes_[mi].lods[lod].indices;
				for (unsigned int fi = 0; fi < mesh->mNumFaces; ++ fi)
				{
					BOOST_ASSERT(3 == mesh->mFaces[fi].mNumIndices);

					indices.push_back(mesh->mFaces[fi].mIndices[0]);
					indices.push_back(mesh->mFaces[fi].mIndices[1]);
					indices.push_back(mesh->mFaces[fi].mIndices[2]);
				}

				bool has_normal = (mesh->mNormals != nullptr);
				bool has_tangent = (mesh->mTangents != nullptr);
				bool has_binormal = (mesh->mBitangents != nullptr);
				auto& has_texcoord = meshes_[mi].has_texcoord;
				uint32_t first_texcoord = AI_MAX_NUMBER_OF_TEXTURECOORDS;
				for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
				{
					has_texcoord[tci] = (mesh->mTextureCoords[tci] != nullptr);
					if (has_texcoord[tci] && (AI_MAX_NUMBER_OF_TEXTURECOORDS == first_texcoord))
					{
						first_texcoord = tci;
					}
				}

				auto& positions = meshes_[mi].lods[lod].positions;
				auto& normals = meshes_[mi].lods[lod].normals;
				std::vector<float3> tangents(mesh->mNumVertices);
				std::vector<float3> binormals(mesh->mNumVertices);
				auto& texcoords = meshes_[mi].lods[lod].texcoords;
				positions.resize(mesh->mNumVertices);
				normals.resize(mesh->mNumVertices);
				for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
				{
					texcoords[tci].resize(mesh->mNumVertices);
				}
				for (unsigned int vi = 0; vi < mesh->mNumVertices; ++ vi)
				{
					positions[vi] = float3(&mesh->mVertices[vi].x);

					if (has_normal)
					{
						normals[vi] = float3(&mesh->mNormals[vi].x);
					}
					if (has_tangent)
					{
						tangents[vi] = float3(&mesh->mTangents[vi].x);
					}
					if (has_binormal)
					{
						binormals[vi] = float3(&mesh->mBitangents[vi].x);
					}

					for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
					{
						if (has_texcoord[tci])
						{
							BOOST_ASSERT(mesh->mTextureCoords[tci] != nullptr);
							KLAYGE_ASSUME(mesh->mTextureCoords[tci] != nullptr);

							texcoords[tci][vi] = float3(&mesh->mTextureCoords[tci][vi].x);
						}
					}
				}

				if (!has_normal)
				{
					MathLib::compute_normal(normals.begin(), indices.begin(), indices.end(), positions.begin(), positions.end());

					has_normal = true;
				}

				auto& mesh_tangents = meshes_[mi].lods[lod].tangents;
				auto& mesh_binormals = meshes_[mi].lods[lod].binormals;
				mesh_tangents.resize(mesh->mNumVertices);
				mesh_binormals.resize(mesh->mNumVertices);
				if ((!has_tangent || !has_binormal) && (first_texcoord != AI_MAX_NUMBER_OF_TEXTURECOORDS))
				{
					MathLib::compute_tangent(mesh_tangents.begin(), mesh_binormals.begin(), indices.begin(), indices.end(),
						positions.begin(), positions.end(), texcoords[first_texcoord].begin(), normals.begin());

					has_tangent = true;
				}

				meshes_[mi].has_normal = has_normal;
				meshes_[mi].has_tangent_frame = has_tangent || has_binormal;

				if (meshes_[mi].has_tangent_frame)
				{
					has_tangent_quat_ = true;
				}
				else if (meshes_[mi].has_normal)
				{
					has_normal = true;
				}
				if (first_texcoord != AI_MAX_NUMBER_OF_TEXTURECOORDS)
				{
					has_texcoord_ = true;
				}

				if (mesh->mNumBones > 0)
				{
					meshes_[mi].lods[lod].joint_bindings.resize(mesh->mNumVertices);

					for (unsigned int bi = 0; bi < mesh->mNumBones; ++ bi)
					{
						aiBone* bone = mesh->mBones[bi];
						bool found = false;
						for (uint32_t ji = 0; ji < joints_.size(); ++ ji)
						{
							std::string joint_name;
							Convert(joint_name, joints_[ji].name);
							if (joint_name == bone->mName.C_Str())
							{
								for (unsigned int wi = 0; wi < bone->mNumWeights; ++ wi)
								{
									float const weight = bone->mWeights[wi].mWeight;
									if (weight >= 0.5f / 255)
									{
										int const vertex_id = bone->mWeights[wi].mVertexId;
										meshes_[mi].lods[lod].joint_bindings[vertex_id].push_back({ ji, weight });
									}
								}

								found = true;
								break;
							}
						}
						if (!found)
						{
							BOOST_ASSERT_MSG(false, "Joint not found!");
						}
					}

					for (auto& binding : meshes_[mi].lods[lod].joint_bindings)
					{
						std::sort(binding.begin(), binding.end(),
							[](std::pair<uint32_t, float> const & lhs, std::pair<uint32_t, float> const & rhs)
							{
								return lhs.second > rhs.second;
							});
					}
				}
			}

			for (unsigned int mi = 0; mi < scene_lods[lod]->mNumMeshes; ++ mi)
			{
				if (has_tangent_quat_ && !meshes_[mi].has_tangent_frame)
				{
					meshes_[mi].has_tangent_frame = true;
				}
				if (has_texcoord_ && !meshes_[mi].has_texcoord[0])
				{
					meshes_[mi].has_texcoord[0] = true;
				}
			}
		}
	}

	void MeshLoader::BuildJoints(aiScene const * scene)
	{
		std::map<std::string, std::pair<JointComponentPtr, int16_t>> joint_nodes;

		std::function<void(aiNode const *, float4x4 const &)> build_bind_matrix =
			[&build_bind_matrix, &joint_nodes, scene](aiNode const * node, float4x4 const & parent_mat)
		{
			float4x4 const mesh_trans = MathLib::transpose(float4x4(&node->mTransformation.a1)) * parent_mat;
			for (unsigned int i = 0; i < node->mNumMeshes; ++ i)
			{
				aiMesh const * mesh = scene->mMeshes[node->mMeshes[i]];
				for (unsigned int ibone = 0; ibone < mesh->mNumBones; ++ ibone)
				{
					aiBone const * bone = mesh->mBones[ibone];

					auto joint = MakeSharedPtr<JointComponent>();

					auto const bone_to_mesh = MathLib::inverse(MathLib::transpose(float4x4(&bone->mOffsetMatrix.a1))) * mesh_trans;
					Quaternion bind_real, bind_dual;
					float bind_scale;
					MatrixToDQ(bone_to_mesh, bind_real, bind_dual, bind_scale);

					joint->BindParams(bind_real, bind_dual, bind_scale);

					joint_nodes[bone->mName.C_Str()] = std::make_pair(std::move(joint), static_cast<int16_t>(-1));
				}
			}

			for (unsigned int i = 0; i < node->mNumChildren; ++ i)
			{
				build_bind_matrix(node->mChildren[i], mesh_trans);
			}
		};

		std::function<bool(aiNode const *)> mark_joint_nodes = [&mark_joint_nodes, &joint_nodes](aiNode const * node)
		{
			std::string name = node->mName.C_Str();
			bool child_has_bone = false;

			auto iter = joint_nodes.find(name);
			if (iter != joint_nodes.end())
			{
				child_has_bone = true;
			}

			for (unsigned int i = 0; i < node->mNumChildren; ++ i)
			{
				child_has_bone = mark_joint_nodes(node->mChildren[i]) || child_has_bone;
			}

			if (child_has_bone && (iter == joint_nodes.end()))
			{
				auto joint = MakeSharedPtr<JointComponent>();
				joint->BindParams(Quaternion::Identity(), Quaternion(0, 0, 0, 0), 1);
				joint_nodes[name] = std::make_pair(std::move(joint), static_cast<int16_t>(-1));
			}

			return child_has_bone;
		};

		std::function<void(aiNode const *, int)> alloc_joints =
			[this, &joint_nodes, &alloc_joints](aiNode const * node, int parent_id)
		{
			std::string name = node->mName.C_Str();
			int joint_id = -1;
			auto iter = joint_nodes.find(name);
			if (iter != joint_nodes.end())
			{
				joint_id = static_cast<int>(joints_.size());

				iter->second.second = static_cast<int16_t>(parent_id);

				JointInfo joint_info;
				joint_info.name = name;
				joint_info.joint = iter->second.first;
				joint_info.parent_id = iter->second.second;
				joints_.push_back(std::move(joint_info));
			}

			for (unsigned int i = 0; i < node->mNumChildren; ++ i)
			{
				alloc_joints(node->mChildren[i], joint_id);
			}
		};

		build_bind_matrix(scene->mRootNode, float4x4::Identity());
		mark_joint_nodes(scene->mRootNode);
		alloc_joints(scene->mRootNode, -1);
	}

	void MeshLoader::BuildAnimations(aiScene const * scene)
	{
		auto& skinned_model = checked_cast<SkinnedModel&>(*render_model_);

		struct AssimpAnimation
		{
			std::string name;
			int frame_num;
			std::map<int/*joint_id*/, KeyFrameSet> resampled_frames;
		};

		std::vector<AssimpAnimation> assimp_animations;

		int const resample_fps = 25;
		// for animations
		for (unsigned int ianim = 0; ianim < scene->mNumAnimations; ++ ianim)
		{
			aiAnimation const * cur_anim = scene->mAnimations[ianim];
			float duration = static_cast<float>(cur_anim->mDuration / cur_anim->mTicksPerSecond);
			AssimpAnimation anim;
			anim.name = cur_anim->mName.C_Str();
			anim.frame_num = static_cast<int>(ceilf(duration * resample_fps));
			if (anim.frame_num == 0)
			{
				anim.frame_num = 1;
			}

			// import joints animation
			for (unsigned int ichannel = 0; ichannel < cur_anim->mNumChannels; ++ ichannel)
			{
				aiNodeAnim const * cur_joint = cur_anim->mChannels[ichannel];

				int joint_id = -1;
				for (size_t ji = 0; ji < joints_.size(); ++ ji)
				{
					if (joints_[ji].name == cur_joint->mNodeName.C_Str())
					{
						joint_id = static_cast<int>(ji);
						break;
					}
				}

				// NOTE: ignore animation if node is not joint
				if (joint_id > 0)
				{
					std::vector<std::pair<float, float3>> poss;
					for (unsigned int i = 0; i < cur_joint->mNumPositionKeys; ++ i)
					{
						auto const & p = cur_joint->mPositionKeys[i];
						poss.push_back(std::make_pair(static_cast<float>(p.mTime), AiVectorToFloat3(p.mValue)));
					}

					std::vector<std::pair<float, Quaternion>> quats;
					for (unsigned int i = 0; i < cur_joint->mNumRotationKeys; ++ i)
					{
						auto const & p = cur_joint->mRotationKeys[i];
						quats.push_back(std::make_pair(static_cast<float>(p.mTime), AiQuatToQuat(p.mValue)));
					}

					std::vector<std::pair<float, float3>> scales;
					for (unsigned int i = 0; i < cur_joint->mNumScalingKeys; ++ i)
					{
						auto const & p = cur_joint->mScalingKeys[i];
						scales.push_back(std::make_pair(static_cast<float>(p.mTime), AiVectorToFloat3(p.mValue)));
					}

					float4x4 parent_mat;
					if (auto* parent_node = joints_[joint_id].joint->BoundSceneNode()->Parent())
					{
						parent_mat = parent_node->TransformToWorld();
					}
					else
					{
						parent_mat = float4x4::Identity();
					}

					// resample
					this->ResampleJointTransform(anim.resampled_frames[joint_id], parent_mat, 0, anim.frame_num,
						static_cast<float>(cur_anim->mTicksPerSecond / resample_fps), poss, quats, scales);
				}
			}

			for (size_t ji = 0; ji < joints_.size(); ++ ji)
			{
				int joint_id = static_cast<int>(ji);
				if (anim.resampled_frames.find(joint_id) == anim.resampled_frames.end())
				{
					KeyFrameSet default_tf;
					default_tf.frame_id.push_back(0);

					auto const& node_mat = joints_[ji].joint->BoundSceneNode()->TransformToWorld();
					Quaternion bind_real, bind_dual;
					float bind_scale;
					MatrixToDQ(node_mat, bind_real, bind_dual, bind_scale);

					default_tf.bind_real.push_back(bind_real);
					default_tf.bind_dual.push_back(bind_dual);
					default_tf.bind_scale.push_back(bind_scale);
					anim.resampled_frames.emplace(joint_id, default_tf);
				}
			}

			assimp_animations.push_back(anim);
		}

		auto kfs = MakeSharedPtr<std::vector<KeyFrameSet>>(joints_.size());
		auto animations = MakeSharedPtr<std::vector<Animation>>();
		int animation_frame_offset = 0;
		for (auto const & anim : assimp_animations)
		{
			Animation animation;
			animation.name = anim.name;
			animation.start_frame = animation_frame_offset;
			animation.end_frame = animation_frame_offset + anim.frame_num;
			animations->push_back(std::move(animation));

			for (auto const & frame : anim.resampled_frames)
			{
				auto& kf = (*kfs)[frame.first];
				for (size_t f = 0; f < frame.second.frame_id.size(); ++ f)
				{
					int const shifted_frame = frame.second.frame_id[f] + animation_frame_offset;

					kf.frame_id.push_back(shifted_frame);
					kf.bind_real.push_back(frame.second.bind_real[f]);
					kf.bind_dual.push_back(frame.second.bind_dual[f]);
					kf.bind_scale.push_back(frame.second.bind_scale[f]);
				}

				this->CompressKeyFrameSet(kf);
			}

			animation_frame_offset += anim.frame_num;
		}

		skinned_model.AttachKeyFrameSets(kfs);
		skinned_model.AttachAnimations(animations);

		skinned_model.FrameRate(resample_fps);
		skinned_model.NumFrames(animation_frame_offset);
	}

	void MeshLoader::ResampleJointTransform(KeyFrameSet& rkf, float4x4 const& parent_mat, int start_frame, int end_frame, float fps_scale,
		std::vector<std::pair<float, float3>> const & poss, std::vector<std::pair<float, Quaternion>> const & quats,
		std::vector<std::pair<float, float3>> const & scales)
	{
		size_t i_pos = 0;
		size_t i_rot = 0;
		size_t i_scale = 0;
		for (int i = start_frame; i < end_frame; ++ i)
		{
			float time = i * fps_scale;
			size_t prev_i = 0;
			float fraction = 0.0f;
			float3 scale_resampled(1, 1, 1);
			Quaternion bind_real_resampled(0, 0, 0, 1);
			Quaternion bind_dual_resampled(0, 0, 0, 0);

			if (!scales.empty())
			{
				fraction = GetInterpTime(scales, time, prev_i, i_scale);
				scale_resampled = MathLib::lerp(scales[prev_i].second, scales[i_scale].second, fraction);
			}
			if (!quats.empty())
			{
				fraction = GetInterpTime(quats, time, prev_i, i_rot);
				bind_real_resampled = MathLib::slerp(quats[prev_i].second, quats[i_rot].second, fraction);
			}
			if (!poss.empty())
			{
				fraction = GetInterpTime(poss, time, prev_i, i_pos);

				auto bind_dual_prev_i = MathLib::quat_trans_to_udq(quats[prev_i].second, poss[prev_i].second);
				auto bind_dual_i_pos = MathLib::quat_trans_to_udq(quats[i_rot].second, poss[i_pos].second);

				auto bind_dq_resampled = MathLib::sclerp(quats[prev_i].second, bind_dual_prev_i,
					quats[i_rot].second, bind_dual_i_pos,
					fraction);

				bind_dual_resampled = bind_dq_resampled.second;
			}

			float4x4 const node_mat =
				MathLib::scaling(scale_resampled) * MathLib::udq_to_matrix(bind_real_resampled, bind_dual_resampled) * parent_mat;
			MatrixToDQ(node_mat, bind_real_resampled, bind_dual_resampled, scale_resampled.x());

			if (MathLib::SignBit(bind_real_resampled.w()) < 0)
			{
				bind_real_resampled = -bind_real_resampled;
				bind_dual_resampled = -bind_dual_resampled;
			}

			rkf.frame_id.push_back(i);
			rkf.bind_real.push_back(bind_real_resampled);
			rkf.bind_dual.push_back(bind_dual_resampled);
			rkf.bind_scale.push_back(scale_resampled.x());
		}
	}

	void MeshLoader::LoadFromAssimp(std::string_view input_name, MeshMetadata const & metadata)
	{
		unsigned int ppsteps = aiProcess_JoinIdenticalVertices // join identical vertices/ optimize indexing
			| aiProcess_ValidateDataStructure // perform a full validation of the loader's output
			| aiProcess_RemoveRedundantMaterials // remove redundant materials
			| aiProcess_FindInstances; // search for instanced meshes and remove them by references to one master

		uint32_t const num_lods = static_cast<uint32_t>(metadata.NumLods());

		std::vector<Assimp::Importer> importers(num_lods);
		std::vector<aiScene const*> scenes(num_lods);
		for (uint32_t lod = 0; lod < num_lods; ++ lod)
		{
			std::string_view const lod_file_name = (lod == 0) ? input_name : metadata.LodFileName(lod);
			std::string const file_name = (lod == 0) ? std::string(input_name) : ResLoader::Instance().Locate(lod_file_name);
			if (file_name.empty())
			{
				LogError() << "Could NOT find " << lod_file_name << " for LoD " << lod << '.' << std::endl;
				return;
			}

			auto& importer = importers[lod];

			importer.SetPropertyInteger(AI_CONFIG_IMPORT_TER_MAKE_UVS, 1);
			importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80);
			importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, 0);
			importer.SetPropertyInteger(AI_CONFIG_GLOB_MEASURE_TIME, 1);

			scenes[lod] = importer.ReadFile(file_name.c_str(),
				ppsteps // configurable pp steps
				| aiProcess_GenSmoothNormals // generate smooth normal vectors if not existing
				| aiProcess_Triangulate // triangulate polygons with more than 3 edges
				| aiProcess_ConvertToLeftHanded // convert everything to D3D left handed space
				/*| aiProcess_FixInfacingNormals*/);

			if (!scenes[lod])
			{
				LogError() << "Assimp: Import file " << lod_file_name << " error: " << importer.GetErrorString() << std::endl;
				return;
			}
		}

		this->BuildJoints(scenes[0]);

		bool const skinned = !joints_.empty();

		meshes_.resize(scenes[0]->mNumMeshes);
		for (size_t mi = 0; mi < meshes_.size(); ++ mi)
		{
			meshes_[mi].lods.resize(num_lods);
		}

		this->BuildMeshData(scenes);
		for (uint32_t lod = 0; lod < num_lods; ++ lod)
		{
			this->BuildNodeData(num_lods, lod, -1, scenes[lod]->mRootNode);
		}

		for (auto& joint : joints_)
		{
			if (joint.parent_id == -1)
			{
				nodes_[0].node->AddComponent(joint.joint);
			}
			else
			{
				std::wstring joint_name;
				Convert(joint_name, joint.name);
				for (auto& node : nodes_)
				{
					if (node.node->Name() == joint_name)
					{
						node.node->AddComponent(joint.joint);
						break;
					}
				}
			}
		}

		if (skinned)
		{
			render_model_ = MakeSharedPtr<SkinnedModel>(nodes_[0].node);
		}
		else
		{
			render_model_ = MakeSharedPtr<RenderModel>(nodes_[0].node);
		}

		this->BuildMaterials(scenes[0]);

		if (skinned)
		{
			this->BuildAnimations(scenes[0]);
		}

		for (auto& mesh : meshes_)
		{
			auto& lod0 = mesh.lods[0];

			mesh.pos_bb = MathLib::compute_aabbox(lod0.positions.begin(), lod0.positions.end());
			mesh.tc_bb = MathLib::compute_aabbox(lod0.texcoords[0].begin(), lod0.texcoords[0].end());
		}

		for (auto& node : nodes_)
		{
			node.aabb_local.Min() = float3(+1e10f, +1e10f, +1e10f);
			node.aabb_local.Max() = float3(-1e10f, -1e10f, -1e10f);
		}

		for (size_t i = nodes_.size(); i > 0; -- i)
		{
			auto& node = nodes_[i - 1];
			if (!node.mesh_indices.empty())
			{
				for (auto index : node.mesh_indices)
				{
					node.aabb_local |= meshes_[index].pos_bb;
				}
			}

			if (node.parent_id >= 0)
			{
				if ((node.aabb_local.Min().x() < node.aabb_local.Max().x())
					&& (node.aabb_local.Min().y() < node.aabb_local.Max().y())
					&& (node.aabb_local.Min().z() < node.aabb_local.Max().z()))
				{
					nodes_[node.parent_id].aabb_local |= MathLib::transform_aabb(node.aabb_local, node.node->TransformToParent());
				}
			}
		}
	}

	void MeshSaver::Save(RenderModel const & model, std::string_view output_name)
	{
		std::filesystem::path output_path(output_name.begin(), output_name.end());

		auto const output_ext = output_path.extension().string();
		if (output_ext == ".model_bin")
		{
			SaveModel(model, output_path.string());
		}
		else
		{
			this->SaveByAssimp(model, output_path.string());
		}
	}

	void MeshSaver::SaveByAssimp(RenderModel const & model, std::string_view output_name)
	{
		std::filesystem::path const output_path(output_name.begin(), output_name.end());
		auto const output_ext = output_path.extension();

		bool const is_gltf = (output_ext == ".gltf") || (output_ext == ".glb");

		std::vector<aiScene> scene_lods(model.NumLods());
		for (uint32_t lod = 0; lod < model.NumLods(); ++ lod)
		{
			auto& ai_scene = scene_lods[lod];
			bool skinned = false;

			ai_scene.mNumMaterials = static_cast<uint32_t>(model.NumMaterials());
			ai_scene.mMaterials = new aiMaterial*[ai_scene.mNumMaterials];
			for (uint32_t i = 0; i < ai_scene.mNumMaterials; ++ i)
			{
				auto const & mtl = *model.GetMaterial(i);

				ai_scene.mMaterials[i] = new aiMaterial;
				auto& ai_mtl = *ai_scene.mMaterials[i];

				{
					aiString name;
					name.Set(mtl.Name().c_str());
					ai_mtl.AddProperty(&name, AI_MATKEY_NAME);
				}

				{
					if (is_gltf)
					{
						aiColor4D const ai_albedo(mtl.Albedo().x(), mtl.Albedo().y(), mtl.Albedo().z(), mtl.Albedo().w());
						ai_mtl.AddProperty(&ai_albedo, 1, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR);

						float ai_metallic = mtl.Metalness();
						ai_mtl.AddProperty(&ai_metallic, 1, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR);
					}
					else
					{
						float3 const diffuse = mtl.Albedo() * (1 - mtl.Metalness());

						aiColor3D const ai_diffuse(diffuse.x(), diffuse.y(), diffuse.z());
						ai_mtl.AddProperty(&ai_diffuse, 1, AI_MATKEY_COLOR_DIFFUSE);
					}

					float3 const specular = MathLib::lerp(float3(0.04f, 0.04f, 0.04f),
						float3(mtl.Albedo().x(), mtl.Albedo().y(), mtl.Albedo().z()), mtl.Metalness());

					float const ai_shininess_strength = MathLib::max3(specular.x(), specular.y(), specular.z());
					ai_mtl.AddProperty(&ai_shininess_strength, 1, AI_MATKEY_SHININESS_STRENGTH);

					aiColor3D const ai_specular(specular.x() / ai_shininess_strength, specular.y() / ai_shininess_strength,
						specular.z() / ai_shininess_strength);
					ai_mtl.AddProperty(&ai_specular, 1, AI_MATKEY_COLOR_SPECULAR);
				}
				{
					aiColor3D const ai_emissive(mtl.Emissive().x(), mtl.Emissive().y(), mtl.Emissive().z());
					ai_mtl.AddProperty(&ai_emissive, 1, AI_MATKEY_COLOR_EMISSIVE);
				}

				{
					ai_real const ai_opacity = mtl.Albedo().w();
					ai_mtl.AddProperty(&ai_opacity, 1, AI_MATKEY_OPACITY);
				}

				{
					ai_real const ai_shininess = Glossiness2Shininess(mtl.Glossiness());
					ai_mtl.AddProperty(&ai_shininess, 1, AI_MATKEY_SHININESS);
				}

				{
					int const ai_two_sided = mtl.TwoSided();
					ai_mtl.AddProperty(&ai_two_sided, 1, AI_MATKEY_TWOSIDED);
				}

				if (is_gltf)
				{
					aiString ai_alpha_mode;
					if (mtl.AlphaTestThreshold() > 0)
					{
						ai_alpha_mode.Set("MASK");

						ai_real const ai_opacity = mtl.AlphaTestThreshold();
						ai_mtl.AddProperty(&ai_opacity, 1, AI_MATKEY_GLTF_ALPHACUTOFF);
					}
					else if (mtl.Albedo().w() < 1)
					{
						ai_alpha_mode.Set("BLEND");
					}
					else
					{
						ai_alpha_mode.Set("OPAQUE");
					}

					ai_mtl.AddProperty(&ai_alpha_mode, AI_MATKEY_GLTF_ALPHAMODE);
				}

				// TODO: SSS

				if (!mtl.TextureName(RenderMaterial::TS_Albedo).empty())
				{
					aiString name;
					name.Set(mtl.TextureName(RenderMaterial::TS_Albedo));
					if (is_gltf)
					{
						ai_mtl.AddProperty(&name, _AI_MATKEY_TEXTURE_BASE, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE);
					}
					else
					{
						ai_mtl.AddProperty(&name, AI_MATKEY_TEXTURE_DIFFUSE(0));
					}
				}

				if (!mtl.TextureName(RenderMaterial::TS_MetalnessGlossiness).empty())
				{
					aiString name;
					name.Set(mtl.TextureName(RenderMaterial::TS_MetalnessGlossiness));
					if (is_gltf)
					{
						ai_mtl.AddProperty(&name, _AI_MATKEY_TEXTURE_BASE, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE);
					}
					else
					{
						ai_mtl.AddProperty(&name, AI_MATKEY_TEXTURE_SHININESS(0));
					}
				}

				if (!mtl.TextureName(RenderMaterial::TS_Emissive).empty())
				{
					aiString name;
					name.Set(mtl.TextureName(RenderMaterial::TS_Emissive));
					ai_mtl.AddProperty(&name, AI_MATKEY_TEXTURE_EMISSIVE(0));
				}

				if (!mtl.TextureName(RenderMaterial::TS_Normal).empty())
				{
					aiString name;
					name.Set(mtl.TextureName(RenderMaterial::TS_Normal));
					ai_mtl.AddProperty(&name, AI_MATKEY_TEXTURE_NORMALS(0));
				}

				if (!mtl.TextureName(RenderMaterial::TS_Height).empty())
				{
					aiString name;
					name.Set(mtl.TextureName(RenderMaterial::TS_Height));
					ai_mtl.AddProperty(&name, AI_MATKEY_TEXTURE_HEIGHT(0));

					if (!MathLib::equal<float>(mtl.HeightScale(), 0.06f))
					{
						ai_real const ai_bump_scaling = mtl.HeightScale();
						ai_mtl.AddProperty(&ai_bump_scaling, 1, AI_MATKEY_BUMPSCALING);
					}
				}
			}

			ai_scene.mNumMeshes = model.NumMeshes();
			ai_scene.mMeshes = new aiMesh*[ai_scene.mNumMeshes];
			for (uint32_t i = 0; i < ai_scene.mNumMeshes; ++ i)
			{
				auto const& mesh = checked_cast<StaticMesh const&>(*model.Mesh(i));

				ai_scene.mMeshes[i] = new aiMesh;
				auto& ai_mesh = *ai_scene.mMeshes[i];

				ai_mesh.mMaterialIndex = mesh.MaterialID();
				ai_mesh.mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

				std::string name;
				KlayGE::Convert(name, mesh.Name());
				ai_mesh.mName.Set(name.c_str());

				auto const & rl = mesh.GetRenderLayout();

				ai_mesh.mNumVertices = mesh.NumVertices(lod);
				uint32_t const start_vertex = mesh.StartVertexLocation(lod);

				for (uint32_t vi = 0; vi < rl.NumVertexStreams(); ++ vi)
				{
					GraphicsBuffer::Mapper mapper(*rl.GetVertexStream(vi), BA_Read_Only);

					auto const & ve = rl.VertexStreamFormat(vi)[0];
					switch (ve.usage)
					{
					case VEU_Position:
						ai_mesh.mVertices = new aiVector3D[ai_mesh.mNumVertices];

						switch (ve.format)
						{
						case EF_SIGNED_ABGR16:
							{
								auto const & pos_bb = mesh.PosBound();
								float3 const pos_center = pos_bb.Center();
								float3 const pos_extent = pos_bb.HalfSize();

								int16_t const * p_16 = mapper.Pointer<int16_t>() + start_vertex * 4;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									ai_mesh.mVertices[j].x
										= ((p_16[j * 4 + 0] + 32768) / 65535.0f * 2 - 1) * pos_extent.x() + pos_center.x();
									ai_mesh.mVertices[j].y
										= ((p_16[j * 4 + 1] + 32768) / 65535.0f * 2 - 1) * pos_extent.y() + pos_center.y();
									ai_mesh.mVertices[j].z
										= ((p_16[j * 4 + 2] + 32768) / 65535.0f * 2 - 1) * pos_extent.z() + pos_center.z();
								}

								break;
							}

						case EF_BGR32F:
						case EF_ABGR32F:
							{
								uint32_t const num_elems = NumComponents(ve.format);
								float const * p_32f = mapper.Pointer<float>() + start_vertex * num_elems;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									ai_mesh.mVertices[j]
										= aiVector3D(p_32f[j * num_elems + 0], p_32f[j * num_elems + 1], p_32f[j * num_elems + 2]);
								}
								break;
							}

						default:
							KFL_UNREACHABLE("Unsupported position format.");
						}
						break;

					case VEU_Tangent:
						ai_mesh.mTangents = new aiVector3D[ai_mesh.mNumVertices];
						ai_mesh.mBitangents = new aiVector3D[ai_mesh.mNumVertices];
						ai_mesh.mNormals = new aiVector3D[ai_mesh.mNumVertices];

						switch (ve.format)
						{
						case EF_ABGR8:
							{
								uint8_t const * tangent_quats = mapper.Pointer<uint8_t>() + start_vertex * 4;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									Quaternion tangent_quat;
									tangent_quat.x() = (tangent_quats[j * 4 + 0] / 255.0f) * 2 - 1;
									tangent_quat.y() = (tangent_quats[j * 4 + 1] / 255.0f) * 2 - 1;
									tangent_quat.z() = (tangent_quats[j * 4 + 2] / 255.0f) * 2 - 1;
									tangent_quat.w() = (tangent_quats[j * 4 + 3] / 255.0f) * 2 - 1;
									tangent_quat = MathLib::normalize(tangent_quat);

									auto const tangent = MathLib::transform_quat(float3(1, 0, 0), tangent_quat);
									auto const binormal = MathLib::transform_quat(float3(0, 1, 0), tangent_quat)
										* MathLib::sgn(tangent_quat.w());
									auto const normal = MathLib::transform_quat(float3(0, 0, 1), tangent_quat);

									ai_mesh.mTangents[j] = aiVector3D(tangent.x(), tangent.y(), tangent.z());
									ai_mesh.mBitangents[j] = aiVector3D(binormal.x(), binormal.y(), binormal.z());
									ai_mesh.mNormals[j] = aiVector3D(normal.x(), normal.y(), normal.z());
								}
								break;
							}

						default:
							KFL_UNREACHABLE("Unsupported tangent frame format.");
						}
						break;

					case VEU_Normal:
						ai_mesh.mNormals = new aiVector3D[ai_mesh.mNumVertices];

						switch (ve.format)
						{
						case EF_ABGR8:
							{
								uint8_t const * normals = mapper.Pointer<uint8_t>() + start_vertex * 4;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									float3 normal;
									normal.x() = (normals[j * 4 + 0] / 255.0f) * 2 - 1;
									normal.y() = (normals[j * 4 + 1] / 255.0f) * 2 - 1;
									normal.z() = (normals[j * 4 + 2] / 255.0f) * 2 - 1;
									normal = MathLib::normalize(normal);

									ai_mesh.mNormals[j] = aiVector3D(normal.x(), normal.y(), normal.z());
								}
								break;
							}

						default:
							KFL_UNREACHABLE("Unsupported normal format.");
						}
						break;

					case VEU_Diffuse:
						ai_mesh.mColors[0] = new aiColor4D[ai_mesh.mNumVertices];

						switch (ve.format)
						{
						case EF_ABGR8:
							{
								uint8_t const * diffuses = mapper.Pointer<uint8_t>() + start_vertex * 4;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									aiColor4D diffuse;
									diffuse.r = (diffuses[j * 4 + 0] / 255.0f) * 2 - 1;
									diffuse.g = (diffuses[j * 4 + 1] / 255.0f) * 2 - 1;
									diffuse.b = (diffuses[j * 4 + 2] / 255.0f) * 2 - 1;
									diffuse.a = (diffuses[j * 4 + 3] / 255.0f) * 2 - 1;

									ai_mesh.mColors[0][j] = diffuse;
								}
								break;
							}

						case EF_ARGB8:
							{
								uint8_t const * diffuses = mapper.Pointer<uint8_t>() + start_vertex * 4;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									aiColor4D diffuse;
									diffuse.r = (diffuses[j * 4 + 2] / 255.0f) * 2 - 1;
									diffuse.g = (diffuses[j * 4 + 1] / 255.0f) * 2 - 1;
									diffuse.b = (diffuses[j * 4 + 0] / 255.0f) * 2 - 1;
									diffuse.a = (diffuses[j * 4 + 3] / 255.0f) * 2 - 1;

									ai_mesh.mColors[0][j] = diffuse;
								}
								break;
							}

						default:
							KFL_UNREACHABLE("Unsupported normal format.");
						}
						break;

					case VEU_Specular:
						ai_mesh.mColors[1] = new aiColor4D[ai_mesh.mNumVertices];

						switch (ve.format)
						{
						case EF_ABGR8:
							{
								uint8_t const * speculars = mapper.Pointer<uint8_t>() + start_vertex * 4;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									aiColor4D specular;
									specular.r = (speculars[j * 4 + 0] / 255.0f) * 2 - 1;
									specular.g = (speculars[j * 4 + 1] / 255.0f) * 2 - 1;
									specular.b = (speculars[j * 4 + 2] / 255.0f) * 2 - 1;
									specular.a = (speculars[j * 4 + 3] / 255.0f) * 2 - 1;

									ai_mesh.mColors[1][j] = specular;
								}
								break;
							}

						case EF_ARGB8:
							{
								uint8_t const * speculars = mapper.Pointer<uint8_t>() + start_vertex * 4;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									aiColor4D specular;
									specular.r = (speculars[j * 4 + 2] / 255.0f) * 2 - 1;
									specular.g = (speculars[j * 4 + 1] / 255.0f) * 2 - 1;
									specular.b = (speculars[j * 4 + 0] / 255.0f) * 2 - 1;
									specular.a = (speculars[j * 4 + 3] / 255.0f) * 2 - 1;

									ai_mesh.mColors[1][j] = specular;
								}
								break;
							}

						default:
							KFL_UNREACHABLE("Unsupported normal format.");
						}
						break;

					case VEU_TextureCoord:
						ai_mesh.mTextureCoords[ve.usage_index] = new aiVector3D[ai_mesh.mNumVertices];
						ai_mesh.mNumUVComponents[ve.usage_index] = 2;

						switch (ve.format)
						{
						case EF_SIGNED_GR16:
							{
								auto const & tc_bb = mesh.TexcoordBound();
								float3 const tc_center = tc_bb.Center();
								float3 const tc_extent = tc_bb.HalfSize();

								int16_t const * tc_16 = mapper.Pointer<int16_t>() + start_vertex * 2;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									ai_mesh.mTextureCoords[ve.usage_index][j].x
										= ((tc_16[j * 2 + 0] + 32768) / 65535.0f * 2 - 1) * tc_extent.x() + tc_center.x();
									ai_mesh.mTextureCoords[ve.usage_index][j].y
										= ((tc_16[j * 2 + 1] + 32768) / 65535.0f * 2 - 1) * tc_extent.y() + tc_center.y();
								}

								break;
							}

						case EF_GR32F:
							{
								float const * tc_32f = mapper.Pointer<float>() + start_vertex * 2;
								for (uint32_t j = 0; j < ai_mesh.mNumVertices; ++ j)
								{
									ai_mesh.mTextureCoords[ve.usage_index][j] = aiVector3D(tc_32f[j * 2 + 0], tc_32f[j * 2 + 1], 0);
								}
								break;
							}

						default:
							KFL_UNREACHABLE("Unsupported texcoord format.");
						}
						break;

					case VEU_BlendWeight:
						break;

					case VEU_BlendIndex:
						break;

					default:
						KFL_UNREACHABLE("Unsupported vertex format.");
					}
				}

				{
					std::vector<uint8_t> blend_indices_data;
					std::vector<uint8_t> blend_weight_data;
					for (uint32_t vi = 0; vi < rl.NumVertexStreams(); ++vi)
					{
						auto const& ve = rl.VertexStreamFormat(vi)[0];
						if (ve.usage == VEU_BlendIndex)
						{
							GraphicsBuffer::Mapper mapper(*rl.GetVertexStream(vi), BA_Read_Only);
							uint8_t const* bi = mapper.Pointer<uint8_t>() + start_vertex * 4;
							blend_indices_data.insert(blend_indices_data.end(), bi, bi + ai_mesh.mNumVertices * 4);
						}
						else if (ve.usage == VEU_BlendWeight)
						{
							GraphicsBuffer::Mapper mapper(*rl.GetVertexStream(vi), BA_Read_Only);
							uint8_t const* bw = mapper.Pointer<uint8_t>() + start_vertex * 4;
							blend_weight_data.insert(blend_weight_data.end(), bw, bw + ai_mesh.mNumVertices * 4);
						}
					}

					std::vector<std::vector<uint8_t>> blend_indices;
					std::vector<std::vector<float>> blend_weight;
					std::vector<uint8_t> joint_indices;
					if (!blend_indices_data.empty() && !blend_weight_data.empty())
					{
						blend_indices.resize(ai_mesh.mNumVertices);
						blend_weight.resize(ai_mesh.mNumVertices);

						for (uint32_t vi = 0; vi < ai_mesh.mNumVertices; ++vi)
						{
							for (uint32_t wi = 0; wi < 4; ++wi)
							{
								uint8_t const joint_index = (blend_indices_data[vi * 4 + wi]);
								float const weight = blend_weight_data[vi * 4 + wi];
								if (weight > 0)
								{
									blend_indices[vi].push_back(joint_index);
									joint_indices.push_back(joint_index);
									blend_weight[vi].push_back(weight / 255.0f);
								};
							}
						}
					}

					std::sort(joint_indices.begin(), joint_indices.end());
					joint_indices.erase(std::unique(joint_indices.begin(), joint_indices.end()), joint_indices.end());

					if (!joint_indices.empty())
					{
						skinned = true;
						SkinnedModel const& skinned_model = checked_cast<SkinnedModel const&>(model);

						ai_mesh.mNumBones = static_cast<uint32_t>(joint_indices.size());
						ai_mesh.mBones = new aiBone*[ai_mesh.mNumBones];
						
						std::map<uint32_t, uint32_t> joint_mapping;
						for (uint32_t bi = 0; bi < ai_mesh.mNumBones; ++bi)
						{
							joint_mapping.emplace(joint_indices[bi], bi);
						}

						std::vector<std::vector<aiVertexWeight>> vertex_weights(ai_mesh.mNumBones);
						for (uint32_t vi = 0; vi < ai_mesh.mNumVertices; ++vi)
						{
							for (uint32_t wi = 0; wi < blend_indices[vi].size(); ++wi)
							{
								auto& vw = vertex_weights[joint_mapping[blend_indices[vi][wi]]];
								vw.push_back(aiVertexWeight(vi, blend_weight[vi][wi]));
							}
						}

						for (uint32_t bi = 0; bi < ai_mesh.mNumBones; ++bi)
						{
							ai_mesh.mBones[bi] = new aiBone;

							auto const* joint = skinned_model.GetJoint(joint_indices[bi]).get();
							auto const* joint_node = joint->BoundSceneNode();

							SceneNode const* mesh_node = nullptr;
							model.RootNode()->Traverse([&mesh, &mesh_node](SceneNode& node)
								{
									node.ForEachComponentOfType<RenderableComponent>([&mesh, &node, &mesh_node](RenderableComponent& comp)
										{
											if (&comp.BoundRenderable() == &mesh)
											{
												mesh_node = &node;
											}
										});
									return true;
								});
							BOOST_ASSERT(mesh_node != nullptr);

							std::string joint_name;
							Convert(joint_name, joint_node->Name());
							ai_mesh.mBones[bi]->mName.Set(joint_name);

							ai_mesh.mBones[bi]->mNumWeights = static_cast<uint32_t>(vertex_weights[bi].size());
							ai_mesh.mBones[bi]->mWeights = new aiVertexWeight[ai_mesh.mBones[bi]->mNumWeights];
							memcpy(ai_mesh.mBones[bi]->mWeights, vertex_weights[bi].data(),
								ai_mesh.mBones[bi]->mNumWeights * sizeof(aiVertexWeight));

							float4x4 const joint_mat = DQToMatrix(joint->BindReal(), joint->BindDual(), joint->BindScale());
							float4x4 const node_mat = mesh_node->TransformToWorld();

							float4x4 const offset_mat = MathLib::transpose(MathLib::inverse(joint_mat * MathLib::inverse(node_mat)));
							memcpy(&ai_mesh.mBones[bi]->mOffsetMatrix.a1, &offset_mat, sizeof(offset_mat));
						}
					}
				}

				{
					ai_mesh.mNumFaces = mesh.NumIndices(lod) / 3;
					uint32_t const start_index = mesh.StartIndexLocation(lod);

					ai_mesh.mFaces = new aiFace[ai_mesh.mNumFaces];

					GraphicsBuffer::Mapper mapper(*rl.GetIndexStream(), BA_Read_Only);
					if (rl.IndexStreamFormat() == EF_R16UI)
					{
						auto const * indices_16 = mapper.Pointer<uint16_t>() + start_index;

						for (uint32_t j = 0; j < ai_mesh.mNumFaces; ++ j)
						{
							auto& ai_face = ai_mesh.mFaces[j];

							ai_face.mIndices = new unsigned int[3];
							ai_face.mNumIndices = 3;

							ai_face.mIndices[0] = indices_16[j * 3 + 0];
							ai_face.mIndices[1] = indices_16[j * 3 + 1];
							ai_face.mIndices[2] = indices_16[j * 3 + 2];
						}
					}
					else
					{
						auto const * indices_32 = mapper.Pointer<uint32_t>() + start_index;

						for (uint32_t j = 0; j < ai_mesh.mNumFaces; ++ j)
						{
							auto& ai_face = ai_mesh.mFaces[j];

							ai_face.mIndices = new unsigned int[3];
							ai_face.mNumIndices = 3;

							ai_face.mIndices[0] = indices_32[j * 3 + 0];
							ai_face.mIndices[1] = indices_32[j * 3 + 1];
							ai_face.mIndices[2] = indices_32[j * 3 + 2];
						}
					}
				}
			}

			if (skinned)
			{
				SkinnedModel const& skinned_model = checked_cast<SkinnedModel const&>(model);

				ai_scene.mNumAnimations = skinned_model.NumAnimations();
				ai_scene.mAnimations = new aiAnimation*[ai_scene.mNumAnimations];

				for (uint32_t ai = 0; ai < ai_scene.mNumAnimations; ++ai)
				{
					ai_scene.mAnimations[ai] = new aiAnimation;

					auto& animations = *skinned_model.GetAnimations();
					ai_scene.mAnimations[ai]->mName.Set(animations[ai].name);
					ai_scene.mAnimations[ai]->mDuration = animations[ai].end_frame - animations[ai].start_frame;
					ai_scene.mAnimations[ai]->mTicksPerSecond = skinned_model.FrameRate();

					ai_scene.mAnimations[ai]->mNumChannels = 0;
					std::vector<uint32_t> non_trivial_joint_indices;
					for (uint32_t ji = 0; ji < skinned_model.NumJoints(); ++ji)
					{
						auto& key_frame_set = (*skinned_model.GetKeyFrameSets())[ji];
						if (key_frame_set.frame_id.size() > 1)
						{
							non_trivial_joint_indices.push_back(ji);
						}
						else if (key_frame_set.frame_id.size() == 1)
						{
							float4x4 const& node_mat = skinned_model.GetJoint(ji)->BoundSceneNode()->TransformToWorld();
							float4x4 const joint_mat =
								MathLib::scaling(key_frame_set.bind_scale[0], key_frame_set.bind_scale[0], key_frame_set.bind_scale[0]) *
								MathLib::udq_to_matrix(key_frame_set.bind_real[0], key_frame_set.bind_dual[0]);

							for (uint32_t item = 0; item < float4x4::size(); ++item)
							{
								if (std::abs(node_mat[item] - joint_mat[item]) > 1e-3f)
								{
									non_trivial_joint_indices.push_back(ji);
									break;
								}
							}
						}
					}

					ai_scene.mAnimations[ai]->mNumChannels = static_cast<uint32_t>(non_trivial_joint_indices.size());
					ai_scene.mAnimations[ai]->mChannels = new aiNodeAnim*[ai_scene.mAnimations[ai]->mNumChannels];
					for (size_t ci = 0; ci < non_trivial_joint_indices.size(); ++ci)
					{
						uint32_t const ji = non_trivial_joint_indices[ci];
						auto& key_frame_set = (*skinned_model.GetKeyFrameSets())[ji];
						ai_scene.mAnimations[ai]->mChannels[ci] = new aiNodeAnim;
						auto& node_anim = *ai_scene.mAnimations[ai]->mChannels[ci];

						auto& joint_node = *skinned_model.GetJoint(ji)->BoundSceneNode();

						std::string joint_name;
						Convert(joint_name, joint_node.Name());
						node_anim.mNodeName.Set(joint_name);

						float4x4 parent_mat;
						if (auto* parent_node = joint_node.Parent())
						{
							parent_mat = parent_node->TransformToWorld();
						}
						else
						{
							parent_mat = float4x4::Identity();
						}

						node_anim.mNumPositionKeys = node_anim.mNumRotationKeys = node_anim.mNumScalingKeys =
							static_cast<uint32_t>(key_frame_set.frame_id.size());

						node_anim.mPositionKeys = new aiVectorKey[node_anim.mNumPositionKeys];
						node_anim.mRotationKeys = new aiQuatKey[node_anim.mNumPositionKeys];
						node_anim.mScalingKeys = new aiVectorKey[node_anim.mNumPositionKeys];
						for (uint32_t pi = 0; pi < node_anim.mNumPositionKeys; ++pi)
						{
							node_anim.mPositionKeys[pi].mTime = node_anim.mRotationKeys[pi].mTime = node_anim.mScalingKeys[pi].mTime =
								static_cast<float>(key_frame_set.frame_id[pi] - animations[ai].start_frame) / skinned_model.FrameRate();

							float4x4 const key_frame_mat =
								MathLib::scaling(key_frame_set.bind_scale[pi], key_frame_set.bind_scale[pi], key_frame_set.bind_scale[pi]) *
								MathLib::udq_to_matrix(key_frame_set.bind_real[pi], key_frame_set.bind_dual[pi]) *
								MathLib::inverse(parent_mat);

							float3 scale;
							Quaternion rot;
							float3 trans;
							MathLib::decompose(scale, rot, trans, key_frame_mat);

							node_anim.mPositionKeys[pi].mValue = Float3ToAiVector(trans);
							node_anim.mRotationKeys[pi].mValue = QuatToAiQuat(rot);
							node_anim.mScalingKeys[pi].mValue = Float3ToAiVector(scale);
						}
					}
				}
			}

			ai_scene.mRootNode = new aiNode;
			ai_scene.mRootNode->mParent = nullptr;
			std::function<void(aiNode& ai_node, SceneNode const & node)> convert_node_subtree
				= [&convert_node_subtree, model](aiNode& ai_node, SceneNode const & node)
					{
						std::string name;
						KlayGE::Convert(name, node.Name());

						ai_node.mName.Set(name);

						float4x4 const & local_mat = node.TransformToParent();
						ai_node.mTransformation = aiMatrix4x4(
							local_mat(0, 0), local_mat(1, 0), local_mat(2, 0), local_mat(3, 0),
							local_mat(0, 1), local_mat(1, 1), local_mat(2, 1), local_mat(3, 1),
							local_mat(0, 2), local_mat(1, 2), local_mat(2, 2), local_mat(3, 2),
							local_mat(0, 3), local_mat(1, 3), local_mat(2, 3), local_mat(3, 3));

						ai_node.mNumChildren = static_cast<uint32_t>(node.Children().size());
						ai_node.mChildren = ai_node.mNumChildren > 0 ? new aiNode*[ai_node.mNumChildren] : nullptr;
						ai_node.mNumMeshes = node.NumComponentsOfType<RenderableComponent>();
						ai_node.mMeshes = ai_node.mNumMeshes > 0 ? new unsigned int[ai_node.mNumMeshes] : nullptr;

						uint32_t mesh_index = 0;
						node.ForEachComponentOfType<RenderableComponent>([&model, &ai_node, &mesh_index](RenderableComponent& renderable_comp) {
							auto const* mesh = &renderable_comp.BoundRenderable();
							for (uint32_t j = 0; j < model.NumMeshes(); ++j)
							{
								if (mesh == model.Mesh(j).get())
								{
									ai_node.mMeshes[mesh_index] = j;
									++mesh_index;
									break;
								}
							}
						});

						for (uint32_t i = 0; i < ai_node.mNumChildren; ++i)
						{
							ai_node.mChildren[i] = new aiNode;
							ai_node.mChildren[i]->mParent = &ai_node;
							convert_node_subtree(*ai_node.mChildren[i], *node.Children()[i]);
						}
					};
			convert_node_subtree(*ai_scene.mRootNode, *model.RootNode());

			auto lod_output_name = (output_path.parent_path() / output_path.stem()).string();
			if (scene_lods.size() > 1)
			{
				lod_output_name  += std::format("_lod_{}", lod);
			}
			lod_output_name += output_ext.string();

			char const * format_id_table[][2] =
			{
				{ "dae", "collada" },
				{ "stl", "stlb" },
				{ "ply", "plyb" },
				{ "gltf", "gltf2" },
				{ "glb", "glb2" },
			};

			auto format_id = output_ext.string().substr(1);
			for (size_t i = 0; i < std::size(format_id_table); ++ i)
			{
				if (format_id == format_id_table[i][0])
				{
					format_id = format_id_table[i][1];
					break;
				}
			}

			Assimp::Exporter exporter;
			exporter.Export(&ai_scene, format_id.c_str(), lod_output_name.c_str(), aiProcess_ConvertToLeftHanded);
		}
	}

	void MeshLoader::CompileMaterialsChunk(XMLNodePtr const & materials_chunk)
	{
		uint32_t num_mtls = 0;
		for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node;
			mtl_node = mtl_node->NextSibling("material"))
		{
			++ num_mtls;
		}

		render_model_->NumMaterials(num_mtls);

		uint32_t mtl_index = 0;
		for (XMLNodePtr mtl_node = materials_chunk->FirstNode("material"); mtl_node;
			mtl_node = mtl_node->NextSibling("material"), ++ mtl_index)
		{
			render_model_->GetMaterial(mtl_index) = MakeSharedPtr<RenderMaterial>();
			auto& mtl = *render_model_->GetMaterial(mtl_index);

			mtl.Name(std::format("Material {}", mtl_index));

			mtl.Albedo(float4(0, 0, 0, 1));
			mtl.Metalness(0);
			mtl.Glossiness(0);
			mtl.Emissive(float3(0, 0, 0));
			mtl.Transparent(false);
			mtl.AlphaTestThreshold(0);
			mtl.Sss(false);
			mtl.TwoSided(false);

			mtl.DetailMode(RenderMaterial::SurfaceDetailMode::ParallaxMapping);
			mtl.HeightOffset(-0.5f);
			mtl.HeightScale(0.06f);
			mtl.EdgeTessHint(5);
			mtl.InsideTessHint(5);
			mtl.MinTessFactor(1);
			mtl.MaxTessFactor(9);

			{
				XMLAttributePtr attr = mtl_node->Attrib("name");
				if (attr)
				{
					mtl.Name(attr->ValueString());
				}
			}

			XMLNodePtr albedo_node = mtl_node->FirstNode("albedo");
			if (albedo_node)
			{
				XMLAttributePtr attr = albedo_node->Attrib("color");
				if (attr)
				{
					float4 albedo;
					ExtractFVector<4>(attr->ValueString(), &albedo[0]);
					mtl.Albedo(albedo);
				}
				attr = albedo_node->Attrib("texture");
				if (attr)
				{
					mtl.TextureName(RenderMaterial::TS_Albedo, std::string(attr->ValueString()));
				}
			}
			else
			{
				float4 albedo(0, 0, 0, 1);

				XMLAttributePtr attr = mtl_node->Attrib("diffuse");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &albedo[0]);
				}
				else
				{
					attr = mtl_node->Attrib("diffuse_r");
					if (attr)
					{
						albedo.x() = attr->ValueFloat();
					}
					attr = mtl_node->Attrib("diffuse_g");
					if (attr)
					{
						albedo.y() = attr->ValueFloat();
					}
					attr = mtl_node->Attrib("diffuse_b");
					if (attr)
					{
						albedo.z() = attr->ValueFloat();
					}
				}

				attr = mtl_node->Attrib("opacity");
				if (attr)
				{
					albedo.w() = mtl_node->Attrib("opacity")->ValueFloat();
				}

				mtl.Albedo(albedo);
			}

			XMLNodePtr metalness_glossiness_node = mtl_node->FirstNode("metalness_glossiness");
			if (metalness_glossiness_node)
			{
				XMLAttributePtr attr = metalness_glossiness_node->Attrib("metalness");
				if (attr)
				{
					mtl.Metalness(attr->ValueFloat());
				}
				attr = metalness_glossiness_node->Attrib("glossiness");
				if (attr)
				{
					mtl.Glossiness(attr->ValueFloat());
				}
				attr = metalness_glossiness_node->Attrib("texture");
				if (attr)
				{
					mtl.TextureName(RenderMaterial::TS_MetalnessGlossiness, std::string(attr->ValueString()));
				}
			}
			else
			{
				XMLNodePtr metalness_node = mtl_node->FirstNode("metalness");
				if (metalness_node)
				{
					XMLAttributePtr attr = metalness_node->Attrib("value");
					if (attr)
					{
						mtl.Metalness(attr->ValueFloat());
					}
					attr = metalness_node->Attrib("texture");
					if (attr)
					{
						mtl.TextureName(RenderMaterial::TS_MetalnessGlossiness, std::string(attr->ValueString()));
					}
				}

				XMLNodePtr glossiness_node = mtl_node->FirstNode("glossiness");
				if (glossiness_node)
				{
					XMLAttributePtr attr = glossiness_node->Attrib("value");
					if (attr)
					{
						mtl.Glossiness(attr->ValueFloat());
					}
					attr = glossiness_node->Attrib("texture");
					if (attr)
					{
						mtl.TextureName(RenderMaterial::TS_MetalnessGlossiness, std::string(attr->ValueString()));
					}
				}
				else
				{
					XMLAttributePtr attr = mtl_node->Attrib("shininess");
					if (attr)
					{
						float shininess = mtl_node->Attrib("shininess")->ValueFloat();
						shininess = MathLib::clamp(shininess, 1.0f, MAX_SHININESS);
						mtl.Glossiness(Shininess2Glossiness(shininess));
					}
				}
			}

			XMLNodePtr emissive_node = mtl_node->FirstNode("emissive");
			if (emissive_node)
			{
				XMLAttributePtr attr = emissive_node->Attrib("color");
				if (attr)
				{
					float3 emissive;
					ExtractFVector<3>(attr->ValueString(), &emissive[0]);
					mtl.Emissive(emissive);
				}
				attr = emissive_node->Attrib("texture");
				if (attr)
				{
					mtl.TextureName(RenderMaterial::TS_Emissive, std::string(attr->ValueString()));
				}
			}
			else
			{
				float3 emissive(0, 0, 0);

				XMLAttributePtr attr = mtl_node->Attrib("emit");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &emissive[0]);
				}
				else
				{
					attr = mtl_node->Attrib("emit_r");
					if (attr)
					{
						emissive.x() = attr->ValueFloat();
					}
					attr = mtl_node->Attrib("emit_g");
					if (attr)
					{
						emissive.y() = attr->ValueFloat();
					}
					attr = mtl_node->Attrib("emit_b");
					if (attr)
					{
						emissive.z() = attr->ValueFloat();
					}
				}

				mtl.Emissive(emissive);
			}
			
			XMLNodePtr normal_node = mtl_node->FirstNode("normal");
			if (normal_node)
			{
				XMLAttributePtr attr = normal_node->Attrib("texture");
				if (attr)
				{
					mtl.TextureName(RenderMaterial::TS_Normal, std::string(attr->ValueString()));
				}
			}

			XMLNodePtr height_node = mtl_node->FirstNode("height");
			if (!height_node)
			{
				height_node = mtl_node->FirstNode("bump");
			}
			if (height_node)
			{
				XMLAttributePtr attr = height_node->Attrib("texture");
				if (attr)
				{
					mtl.TextureName(RenderMaterial::TS_Height, std::string(attr->ValueString()));
				}

				attr = height_node->Attrib("offset");
				if (attr)
				{
					mtl.HeightOffset(attr->ValueFloat());
				}

				attr = height_node->Attrib("scale");
				if (attr)
				{
					mtl.HeightScale(attr->ValueFloat());
				}
			}

			XMLNodePtr detail_node = mtl_node->FirstNode("detail");
			if (detail_node)
			{
				XMLAttributePtr attr = detail_node->Attrib("mode");
				if (attr)
				{
					std::string_view const mode_str = attr->ValueString();
					size_t const mode_hash = HashRange(mode_str.begin(), mode_str.end());
					if (CT_HASH("Parallax Occlusion Mapping") == mode_hash)
					{
						mtl.DetailMode(RenderMaterial::SurfaceDetailMode::ParallaxOcclusionMapping);
					}
					else if (CT_HASH("Flat Tessellation") == mode_hash)
					{
						mtl.DetailMode(RenderMaterial::SurfaceDetailMode::FlatTessellation);
					}
					else if (CT_HASH("Smooth Tessellation") == mode_hash)
					{
						mtl.DetailMode(RenderMaterial::SurfaceDetailMode::SmoothTessellation);
					}
				}

				attr = detail_node->Attrib("height_offset");
				if (attr)
				{
					mtl.HeightOffset(attr->ValueFloat());
				}

				attr = detail_node->Attrib("height_scale");
				if (attr)
				{
					mtl.HeightScale(attr->ValueFloat());
				}

				XMLNodePtr tess_node = detail_node->FirstNode("tess");
				if (tess_node)
				{
					attr = tess_node->Attrib("edge_hint");
					if (attr)
					{
						mtl.EdgeTessHint(attr->ValueFloat());
					}
					attr = tess_node->Attrib("inside_hint");
					if (attr)
					{
						mtl.InsideTessHint(attr->ValueFloat());
					}
					attr = tess_node->Attrib("min");
					if (attr)
					{
						mtl.MinTessFactor(attr->ValueFloat());
					}
					attr = tess_node->Attrib("max");
					if (attr)
					{
						mtl.MaxTessFactor(attr->ValueFloat());
					}
				}
				else
				{
					attr = detail_node->Attrib("edge_tess_hint");
					if (attr)
					{
						mtl.EdgeTessHint(attr->ValueFloat());
					}
					attr = detail_node->Attrib("inside_tess_hint");
					if (attr)
					{
						mtl.InsideTessHint(attr->ValueFloat());
					}
					attr = detail_node->Attrib("min_tess");
					if (attr)
					{
						mtl.MinTessFactor(attr->ValueFloat());
					}
					attr = detail_node->Attrib("max_tess");
					if (attr)
					{
						mtl.MaxTessFactor(attr->ValueFloat());
					}
				}
			}

			XMLNodePtr transparent_node = mtl_node->FirstNode("transparent");
			if (transparent_node)
			{
				XMLAttributePtr attr = transparent_node->Attrib("value");
				if (attr)
				{
					mtl.Transparent(attr->ValueInt() ? true : false);
				}
			}

			XMLNodePtr alpha_test_node = mtl_node->FirstNode("alpha_test");
			if (alpha_test_node)
			{
				XMLAttributePtr attr = alpha_test_node->Attrib("value");
				if (attr)
				{
					mtl.AlphaTestThreshold(attr->ValueFloat());
				}
			}

			XMLNodePtr sss_node = mtl_node->FirstNode("sss");
			if (sss_node)
			{
				XMLAttributePtr attr = sss_node->Attrib("value");
				if (attr)
				{
					mtl.Sss(attr->ValueInt() ? true : false);
				}
			}
			else
			{
				XMLAttributePtr attr = mtl_node->Attrib("sss");
				if (attr)
				{
					mtl.Sss(attr->ValueInt() ? true : false);
				}
			}

			XMLNodePtr two_sided_node = mtl_node->FirstNode("two_sided");
			if (two_sided_node)
			{
				XMLAttributePtr attr = two_sided_node->Attrib("value");
				if (attr)
				{
					mtl.TwoSided(attr->ValueInt() ? true : false);
				}
			}

			XMLNodePtr tex_node = mtl_node->FirstNode("texture");
			if (!tex_node)
			{
				XMLNodePtr textures_chunk = mtl_node->FirstNode("textures_chunk");
				if (textures_chunk)
				{
					tex_node = textures_chunk->FirstNode("texture");
				}
			}
			if (tex_node)
			{
				for (; tex_node; tex_node = tex_node->NextSibling("texture"))
				{
					auto const type = tex_node->Attrib("type")->ValueString();
					size_t const type_hash = HashRange(type.begin(), type.end());

					std::string const name(tex_node->Attrib("name")->ValueString());

					if ((CT_HASH("Color") == type_hash) || (CT_HASH("Diffuse Color") == type_hash)
						|| (CT_HASH("Diffuse Color Map") == type_hash)
						|| (CT_HASH("Albedo") == type_hash))
					{
						mtl.TextureName(RenderMaterial::TS_Albedo, name);
					}
					else if ((CT_HASH("MetalnessGlossiness") == type_hash) || (CT_HASH("Metalness") == type_hash) ||
							 (CT_HASH("Glossiness") == type_hash) || (CT_HASH("Reflection Glossiness Map") == type_hash))
					{
						mtl.TextureName(RenderMaterial::TS_MetalnessGlossiness, name);
					}
					else if ((CT_HASH("Self-Illumination") == type_hash) || (CT_HASH("Emissive") == type_hash))
					{
						mtl.TextureName(RenderMaterial::TS_Emissive, name);
					}
					else if ((CT_HASH("Normal") == type_hash) || (CT_HASH("Normal Map") == type_hash))
					{
						mtl.TextureName(RenderMaterial::TS_Normal, name);
					}
					else if ((CT_HASH("Bump") == type_hash) || (CT_HASH("Bump Map") == type_hash)
						|| (CT_HASH("Height") == type_hash) || (CT_HASH("Height Map") == type_hash))
					{
						mtl.TextureName(RenderMaterial::TS_Height, name);
					}
				}
			}
		}
	}

	void MeshLoader::CompileMeshBoundingBox(XMLNodePtr const & mesh_node, uint32_t mesh_index,
		bool& recompute_pos_bb, bool& recompute_tc_bb)
	{
		XMLNodePtr pos_bb_node = mesh_node->FirstNode("pos_bb");
		if (pos_bb_node)
		{
			float3 pos_min_bb, pos_max_bb;
			{
				XMLAttributePtr attr = pos_bb_node->Attrib("min");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &pos_min_bb[0]);
				}
				else
				{
					XMLNodePtr pos_min_node = pos_bb_node->FirstNode("min");
					pos_min_bb.x() = pos_min_node->Attrib("x")->ValueFloat();
					pos_min_bb.y() = pos_min_node->Attrib("y")->ValueFloat();
					pos_min_bb.z() = pos_min_node->Attrib("z")->ValueFloat();
				}
			}
			{
				XMLAttributePtr attr = pos_bb_node->Attrib("max");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &pos_max_bb[0]);
				}
				else
				{
					XMLNodePtr pos_max_node = pos_bb_node->FirstNode("max");
					pos_max_bb.x() = pos_max_node->Attrib("x")->ValueFloat();
					pos_max_bb.y() = pos_max_node->Attrib("y")->ValueFloat();
					pos_max_bb.z() = pos_max_node->Attrib("z")->ValueFloat();
				}
			}
			meshes_[mesh_index].pos_bb = AABBox(pos_min_bb, pos_max_bb);

			recompute_pos_bb = false;
		}
		else
		{
			recompute_pos_bb = true;
		}

		XMLNodePtr tc_bb_node = mesh_node->FirstNode("tc_bb");
		if (tc_bb_node)
		{
			float3 tc_min_bb, tc_max_bb;
			{
				XMLAttributePtr attr = tc_bb_node->Attrib("min");
				if (attr)
				{
					ExtractFVector<2>(attr->ValueString(), &tc_min_bb[0]);
				}
				else
				{
					XMLNodePtr tc_min_node = tc_bb_node->FirstNode("min");
					tc_min_bb.x() = tc_min_node->Attrib("x")->ValueFloat();
					tc_min_bb.y() = tc_min_node->Attrib("y")->ValueFloat();
				}
			}
			{
				XMLAttributePtr attr = tc_bb_node->Attrib("max");
				if (attr)
				{
					ExtractFVector<2>(attr->ValueString(), &tc_max_bb[0]);
				}
				else
				{
					XMLNodePtr tc_max_node = tc_bb_node->FirstNode("max");
					tc_max_bb.x() = tc_max_node->Attrib("x")->ValueFloat();
					tc_max_bb.y() = tc_max_node->Attrib("y")->ValueFloat();
				}
			}

			tc_min_bb.z() = 0;
			tc_max_bb.z() = 0;
			meshes_[mesh_index].tc_bb = AABBox(tc_min_bb, tc_max_bb);

			recompute_tc_bb = false;
		}
		else
		{
			recompute_tc_bb = true;
		}
	}

	void MeshLoader::CompileMeshesChunk(XMLNodePtr const & meshes_chunk)
	{
		uint32_t num_meshes = 0;
		for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"))
		{
			++ num_meshes;
		}
		meshes_.resize(num_meshes);
		nodes_.resize(num_meshes);

		uint32_t mesh_index = 0;
		for (XMLNodePtr mesh_node = meshes_chunk->FirstNode("mesh"); mesh_node; mesh_node = mesh_node->NextSibling("mesh"), ++ mesh_index)
		{
			auto const name = std::string(mesh_node->Attrib("name")->ValueString());
			std::wstring wname;
			KlayGE::Convert(wname, name);

			nodes_[mesh_index].node = MakeSharedPtr<SceneNode>(wname, SceneNode::SOA_Cullable);
			nodes_[mesh_index].mesh_indices.push_back(mesh_index);
			if (mesh_index != 0)
			{
				nodes_[0].node->AddChild(nodes_[mesh_index].node);
				nodes_[mesh_index].parent_id = 0;
			}
			else
			{
				nodes_[mesh_index].parent_id = -1;
			}

			meshes_[mesh_index].name = name;
			meshes_[mesh_index].mtl_id = mesh_node->Attrib("mtl_id")->ValueInt();

			bool recompute_pos_bb;
			bool recompute_tc_bb;
			this->CompileMeshBoundingBox(mesh_node, mesh_index, recompute_pos_bb, recompute_tc_bb);
			if (recompute_pos_bb && recompute_tc_bb)
			{
				XMLNodePtr vertices_chunk = mesh_node->FirstNode("vertices_chunk");
				if (vertices_chunk)
				{
					this->CompileMeshBoundingBox(vertices_chunk, mesh_index, recompute_pos_bb, recompute_tc_bb);
				}
			}

			XMLNodePtr lod_node = mesh_node->FirstNode("lod");
			if (lod_node)
			{
				uint32_t mesh_lod = 0;
				for (; lod_node; lod_node = lod_node->NextSibling("lod"))
				{
					++ mesh_lod;
				}

				std::vector<XMLNodePtr> lod_nodes(mesh_lod);
				for (lod_node = mesh_node->FirstNode("lod"); lod_node; lod_node = lod_node->NextSibling("lod"))
				{
					uint32_t const lod = lod_node->Attrib("value")->ValueUInt();
					lod_nodes[lod] = lod_node;
				}

				meshes_[mesh_index].lods.resize(mesh_lod);

				for (uint32_t lod = 0; lod < mesh_lod; ++ lod)
				{
					this->CompileMeshLodChunk(lod_nodes[lod], mesh_index, lod, recompute_pos_bb, recompute_tc_bb);

					recompute_pos_bb = false;
					recompute_tc_bb = false;
				}
			}
			else
			{
				meshes_[mesh_index].lods.resize(1);

				this->CompileMeshLodChunk(mesh_node, mesh_index, 0, recompute_pos_bb, recompute_tc_bb);

				recompute_pos_bb = false;
				recompute_tc_bb = false;
			}
		}
	}

	void MeshLoader::CompileMeshLodChunk(XMLNodePtr const & lod_node, uint32_t mesh_index, uint32_t lod,
		bool recompute_pos_bb, bool recompute_tc_bb)
	{
		XMLNodePtr vertices_chunk = lod_node->FirstNode("vertices_chunk");
		if (vertices_chunk)
		{
			this->CompileMeshesVerticesChunk(vertices_chunk, mesh_index, lod,
				recompute_pos_bb, recompute_tc_bb);
		}

		XMLNodePtr triangles_chunk = lod_node->FirstNode("triangles_chunk");
		if (triangles_chunk)
		{
			CompileMeshesTrianglesChunk(triangles_chunk, mesh_index, lod);
		}
	}

	void MeshLoader::CompileMeshesVerticesChunk(XMLNodePtr const & vertices_chunk, uint32_t mesh_index, uint32_t lod,
		bool recompute_pos_bb, bool recompute_tc_bb)
	{
		auto& mesh = meshes_[mesh_index];
		auto& mesh_lod = mesh.lods[lod];

		std::vector<float4> mesh_tangents;
		std::vector<float3> mesh_binormals;

		bool has_normal = false;
		bool has_diffuse = false;
		bool has_specular = false;
		bool has_tex_coord = false;
		bool has_tangent = false;
		bool has_binormal = false;
		bool has_tangent_quat = false;

		for (XMLNodePtr vertex_node = vertices_chunk->FirstNode("vertex"); vertex_node; vertex_node = vertex_node->NextSibling("vertex"))
		{
			{
				float3 pos;
				XMLAttributePtr attr = vertex_node->Attrib("x");
				if (attr)
				{
					pos.x() = vertex_node->Attrib("x")->ValueFloat();
					pos.y() = vertex_node->Attrib("y")->ValueFloat();
					pos.z() = vertex_node->Attrib("z")->ValueFloat();

					attr = vertex_node->Attrib("u");
					if (attr)
					{
						float3 tex_coord;
						tex_coord.x() = vertex_node->Attrib("u")->ValueFloat();
						tex_coord.y() = vertex_node->Attrib("v")->ValueFloat();
						tex_coord.z() = 0;
						mesh_lod.texcoords[0].push_back(tex_coord);
					}
				}
				else
				{
					ExtractFVector<3>(vertex_node->Attrib("v")->ValueString(), &pos[0]);
				}
				mesh_lod.positions.push_back(pos);
			}

			XMLNodePtr diffuse_node = vertex_node->FirstNode("diffuse");
			if (diffuse_node)
			{
				has_diffuse = true;

				float4 diffuse;
				XMLAttributePtr attr = diffuse_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<4>(attr->ValueString(), &diffuse[0]);
				}
				else
				{
					diffuse.x() = diffuse_node->Attrib("r")->ValueFloat();
					diffuse.y() = diffuse_node->Attrib("g")->ValueFloat();
					diffuse.z() = diffuse_node->Attrib("b")->ValueFloat();
					diffuse.w() = diffuse_node->Attrib("a")->ValueFloat();										
				}
				mesh_lod.diffuses.push_back(Color(diffuse.x(), diffuse.y(), diffuse.z(), diffuse.w()));
			}

			XMLNodePtr specular_node = vertex_node->FirstNode("specular");
			if (specular_node)
			{
				has_specular = true;

				float3 specular;
				XMLAttributePtr attr = specular_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &specular[0]);
				}
				else
				{
					specular.x() = specular_node->Attrib("r")->ValueFloat();
					specular.y() = specular_node->Attrib("g")->ValueFloat();
					specular.z() = specular_node->Attrib("b")->ValueFloat();
				}
				mesh_lod.speculars.push_back(Color(specular.x(), specular.y(), specular.z(), 1));
			}

			if (!vertex_node->Attrib("u"))
			{
				XMLNodePtr tex_coord_node = vertex_node->FirstNode("tex_coord");
				if (tex_coord_node)
				{
					has_tex_coord = true;

					float3 tex_coord;
					XMLAttributePtr attr = tex_coord_node->Attrib("u");
					if (attr)
					{
						tex_coord.x() = tex_coord_node->Attrib("u")->ValueFloat();
						tex_coord.y() = tex_coord_node->Attrib("v")->ValueFloat();
					}
					else
					{
						ExtractFVector<2>(tex_coord_node->Attrib("v")->ValueString(), &tex_coord[0]);
					}
					tex_coord.z() = 0;
					mesh_lod.texcoords[0].push_back(tex_coord);
				}
			}

			XMLNodePtr weight_node = vertex_node->FirstNode("weight");
			if (weight_node)
			{
				std::vector<std::pair<uint32_t, float>> binding;

				XMLAttributePtr attr = weight_node->Attrib("joint");
				if (!attr)
				{
					attr = weight_node->Attrib("bone_index");
				}
				if (attr)
				{
					XMLAttributePtr weight_attr = weight_node->Attrib("weight");

					std::string_view const index_str = attr->ValueString();
					std::string_view const weight_str = weight_attr->ValueString();
					std::vector<std::string_view> index_strs = StringUtil::Split(index_str, StringUtil::EqualTo(' '));
					std::vector<std::string_view> weight_strs = StringUtil::Split(weight_str, StringUtil::EqualTo(' '));
					
					for (size_t num_blend = 0; num_blend < index_strs.size(); ++ num_blend)
					{
						binding.push_back(
							{std::stoul(std::string(index_strs[num_blend])), std::stof(std::string(weight_strs[num_blend]))});
					}
				}
				else
				{
					while (weight_node)
					{
						binding.push_back({ weight_node->Attrib("bone_index")->ValueUInt(),
							weight_node->Attrib("weight")->ValueFloat() });

						weight_node = weight_node->NextSibling("weight");
					}
				}

				mesh_lod.joint_bindings.emplace_back(std::move(binding));
			}
						
			XMLNodePtr normal_node = vertex_node->FirstNode("normal");
			if (normal_node)
			{
				has_normal = true;

				float3 normal;
				XMLAttributePtr attr = normal_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &normal[0]);
				}
				else
				{
					normal.x() = normal_node->Attrib("x")->ValueFloat();
					normal.y() = normal_node->Attrib("y")->ValueFloat();
					normal.z() = normal_node->Attrib("z")->ValueFloat();
				}
				mesh_lod.normals.push_back(normal);
			}

			XMLNodePtr tangent_node = vertex_node->FirstNode("tangent");
			if (tangent_node)
			{
				has_tangent = true;

				float4 tangent;
				XMLAttributePtr attr = tangent_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<4>(attr->ValueString(), &tangent[0]);
				}
				else
				{
					tangent.x() = tangent_node->Attrib("x")->ValueFloat();
					tangent.y() = tangent_node->Attrib("y")->ValueFloat();
					tangent.z() = tangent_node->Attrib("z")->ValueFloat();
					attr = tangent_node->Attrib("w");
					if (attr)
					{
						tangent.w() = attr->ValueFloat();
					}
					else
					{
						tangent.w() = 1;
					}
				}
				mesh_tangents.push_back(tangent);
			}

			XMLNodePtr binormal_node = vertex_node->FirstNode("binormal");
			if (binormal_node)
			{
				has_binormal = true;

				float3 binormal;
				XMLAttributePtr attr = binormal_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<3>(attr->ValueString(), &binormal[0]);
				}
				else
				{
					binormal.x() = binormal_node->Attrib("x")->ValueFloat();
					binormal.y() = binormal_node->Attrib("y")->ValueFloat();
					binormal.z() = binormal_node->Attrib("z")->ValueFloat();
				}
				mesh_binormals.push_back(binormal);
			}

			XMLNodePtr tangent_quat_node = vertex_node->FirstNode("tangent_quat");
			if (tangent_quat_node)
			{
				has_tangent_quat = true;

				Quaternion tangent_quat;
				XMLAttributePtr const & attr = tangent_quat_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<4>(attr->ValueString(), &tangent_quat[0]);
				}
				else
				{
					tangent_quat.x() = tangent_quat_node->Attrib("x")->ValueFloat();
					tangent_quat.y() = tangent_quat_node->Attrib("y")->ValueFloat();
					tangent_quat.z() = tangent_quat_node->Attrib("z")->ValueFloat();
					tangent_quat.w() = tangent_quat_node->Attrib("w")->ValueFloat();
				}

				float3 const tangent = MathLib::transform_quat(float3(1, 0, 0), tangent_quat);
				float3 const binormal = MathLib::transform_quat(float3(0, 1, 0), tangent_quat) * MathLib::sgn(tangent_quat.w());
				float3 const normal = MathLib::transform_quat(float3(0, 0, 1), tangent_quat);

				mesh_lod.tangents.push_back(tangent);
				mesh_lod.binormals.push_back(binormal);
				mesh_lod.normals.push_back(normal);
			}
		}

		bool recompute_tangent_quat = false;

		{
			if (has_diffuse)
			{
				has_diffuse_ = true;
			}

			if (has_specular)
			{
				has_specular_ = true;
			}

			if (has_tex_coord)
			{
				has_texcoord_ = true;
				mesh.has_texcoord[0] = true;
			}
			else
			{
				mesh.has_texcoord[0] = false;
			}

			if (has_tangent_quat)
			{
				has_tangent_quat_ = true;
			}
			else
			{
				if (has_normal && !has_tangent && !has_binormal)
				{
					has_normal_ = true;
					mesh.has_normal = true;
				}
				else
				{
					mesh.has_normal = false;

					if ((has_normal && has_tangent) || (has_normal && has_binormal)
						|| (has_tangent && has_binormal))
					{
						has_tangent_quat_ = true;
						mesh.has_tangent_frame = true;

						if (!has_tangent_quat)
						{
							recompute_tangent_quat = true;
						}
					}
					else
					{
						mesh.has_tangent_frame = false;
					}
				}
			}
		}

		if (recompute_pos_bb && (lod == 0))
		{
			mesh.pos_bb = MathLib::compute_aabbox(mesh_lod.positions.begin(), mesh_lod.positions.end());
		}
		if (recompute_tc_bb && (lod == 0))
		{
			mesh.tc_bb = MathLib::compute_aabbox(mesh_lod.texcoords[0].begin(), mesh_lod.texcoords[0].end());
		}
		if (recompute_tangent_quat)
		{
			mesh_lod.tangents.resize(mesh_lod.positions.size());
			mesh_lod.binormals.resize(mesh_lod.positions.size());
			mesh_lod.normals.resize(mesh_lod.positions.size());
			for (uint32_t index = 0; index < mesh_lod.positions.size(); ++ index)
			{
				float3 tangent, binormal, normal;
				if (has_tangent)
				{
					tangent = float3(mesh_tangents[index].x(), mesh_tangents[index].y(),
						mesh_tangents[index].z());
				}
				if (has_binormal)
				{
					binormal = mesh_binormals[index];
				}
				if (has_normal)
				{
					normal = mesh_lod.normals[index];
				}

				if (!has_tangent)
				{
					BOOST_ASSERT(has_binormal && has_normal);

					tangent = MathLib::cross(binormal, normal);
				}
				if (!has_binormal)
				{
					BOOST_ASSERT(has_tangent && has_normal);

					binormal = MathLib::cross(normal, tangent) * mesh_tangents[index].w();
				}
				if (!has_normal)
				{
					BOOST_ASSERT(has_tangent && has_binormal);

					normal = MathLib::cross(tangent, binormal);
				}

				mesh_lod.tangents[index] = tangent;
				mesh_lod.binormals[index] = binormal;
				mesh_lod.normals[index] = normal;
			}
		}
	}

	void MeshLoader::CompileMeshesTrianglesChunk(XMLNodePtr const & triangles_chunk, uint32_t mesh_index, uint32_t lod)
	{
		auto& mesh = meshes_[mesh_index];
		auto& mesh_lod = mesh.lods[lod];

		for (XMLNodePtr tri_node = triangles_chunk->FirstNode("triangle"); tri_node; tri_node = tri_node->NextSibling("triangle"))
		{
			uint32_t ind[3];
			XMLAttributePtr attr = tri_node->Attrib("index");
			if (attr)
			{
				ExtractUIVector<3>(attr->ValueString(), &ind[0]);
			}
			else
			{
				ind[0] = tri_node->Attrib("a")->ValueUInt();
				ind[1] = tri_node->Attrib("b")->ValueUInt();
				ind[2] = tri_node->Attrib("c")->ValueUInt();
			}
			mesh_lod.indices.push_back(ind[0]);
			mesh_lod.indices.push_back(ind[1]);
			mesh_lod.indices.push_back(ind[2]);
		}
	}

	void MeshLoader::CompileBonesChunk(XMLNodePtr const & bones_chunk)
	{
		for (XMLNodePtr bone_node = bones_chunk->FirstNode("bone"); bone_node; bone_node = bone_node->NextSibling("bone"))
		{
			auto joint = MakeSharedPtr<JointComponent>();

			std::string const joint_name = std::string(bone_node->Attrib("name")->ValueString());
			int16_t const parent_id = static_cast<int16_t>(bone_node->Attrib("parent")->ValueInt());

			Quaternion joint_bind_real, joint_bind_dual;
			float joint_bind_scale;

			XMLNodePtr bind_pos_node = bone_node->FirstNode("bind_pos");
			if (bind_pos_node)
			{
				float3 bind_pos(bind_pos_node->Attrib("x")->ValueFloat(), bind_pos_node->Attrib("y")->ValueFloat(),
					bind_pos_node->Attrib("z")->ValueFloat());

				XMLNodePtr bind_quat_node = bone_node->FirstNode("bind_quat");
				Quaternion bind_quat(bind_quat_node->Attrib("x")->ValueFloat(), bind_quat_node->Attrib("y")->ValueFloat(),
					bind_quat_node->Attrib("z")->ValueFloat(), bind_quat_node->Attrib("w")->ValueFloat());

				float scale = MathLib::length(bind_quat);
				bind_quat /= scale;

				joint_bind_dual = MathLib::quat_trans_to_udq(bind_quat, bind_pos);
				joint_bind_real = bind_quat * scale;
				joint_bind_scale = scale;
			}
			else
			{
				XMLNodePtr bind_real_node = bone_node->FirstNode("real");
				if (!bind_real_node)
				{
					bind_real_node = bone_node->FirstNode("bind_real");
				}
				XMLAttributePtr attr = bind_real_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<4>(attr->ValueString(), &joint_bind_real[0]);
				}
				else
				{
					joint_bind_real.x() = bind_real_node->Attrib("x")->ValueFloat();
					joint_bind_real.y() = bind_real_node->Attrib("y")->ValueFloat();
					joint_bind_real.z() = bind_real_node->Attrib("z")->ValueFloat();
					joint_bind_real.w() = bind_real_node->Attrib("w")->ValueFloat();
				}

				XMLNodePtr bind_dual_node = bone_node->FirstNode("dual");
				if (!bind_dual_node)
				{
					bind_dual_node = bone_node->FirstNode("bind_dual");
				}
				attr = bind_dual_node->Attrib("v");
				if (attr)
				{
					ExtractFVector<4>(attr->ValueString(), &joint_bind_dual[0]);
				}
				else
				{
					joint_bind_dual.x() = bind_dual_node->Attrib("x")->ValueFloat();
					joint_bind_dual.y() = bind_dual_node->Attrib("y")->ValueFloat();
					joint_bind_dual.z() = bind_dual_node->Attrib("z")->ValueFloat();
					joint_bind_dual.w() = bind_dual_node->Attrib("w")->ValueFloat();
				}

				joint_bind_scale = MathLib::length(joint_bind_real);
				joint_bind_real /= joint_bind_scale;
				if (MathLib::SignBit(joint_bind_real.w()) < 0)
				{
					joint_bind_real = -joint_bind_real;
					joint_bind_scale = -joint_bind_scale;
				}
			}

			joint->BindParams(joint_bind_real, joint_bind_dual, joint_bind_scale);
			joint->InitInverseOriginParams();

			JointInfo joint_info;
			joint_info.name = joint_name;
			joint_info.joint = std::move(joint);
			joint_info.parent_id = parent_id;
			joints_.push_back(std::move(joint_info));
		}
	}

	void MeshLoader::CompileKeyFramesChunk(XMLNodePtr const & key_frames_chunk)
	{
		auto& skinned_model = checked_cast<SkinnedModel&>(*render_model_);

		XMLAttributePtr nf_attr = key_frames_chunk->Attrib("num_frames");
		if (nf_attr)
		{
			skinned_model.NumFrames(nf_attr->ValueUInt());
		}
		else
		{
			int32_t start_frame = key_frames_chunk->Attrib("start_frame")->ValueInt();
			int32_t end_frame = key_frames_chunk->Attrib("end_frame")->ValueInt();
			skinned_model.NumFrames(end_frame - start_frame);
		}
		skinned_model.FrameRate(key_frames_chunk->Attrib("frame_rate")->ValueUInt());

		auto kfss = MakeSharedPtr<std::vector<KeyFrameSet>>();
		kfss->resize(joints_.size());
		uint32_t joint_id = 0;
		for (XMLNodePtr kf_node = key_frames_chunk->FirstNode("key_frame"); kf_node; kf_node = kf_node->NextSibling("key_frame"))
		{
			XMLAttributePtr joint_attr = kf_node->Attrib("joint");
			if (joint_attr)
			{
				joint_id = joint_attr->ValueUInt();
			}
			else
			{
				++ joint_id;
			}
			KeyFrameSet& kfs = (*kfss)[joint_id];

			int32_t frame_id = -1;
			for (XMLNodePtr key_node = kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
			{
				XMLAttributePtr id_attr = key_node->Attrib("id");
				if (id_attr)
				{
					frame_id = id_attr->ValueInt();
				}
				else
				{
					++ frame_id;
				}
				kfs.frame_id.push_back(frame_id);

				Quaternion bind_real, bind_dual;
				float bind_scale;
				XMLNodePtr pos_node = key_node->FirstNode("pos");
				if (pos_node)
				{
					float3 bind_pos(pos_node->Attrib("x")->ValueFloat(), pos_node->Attrib("y")->ValueFloat(),
						pos_node->Attrib("z")->ValueFloat());

					XMLNodePtr quat_node = key_node->FirstNode("quat");
					bind_real = Quaternion(quat_node->Attrib("x")->ValueFloat(), quat_node->Attrib("y")->ValueFloat(),
						quat_node->Attrib("z")->ValueFloat(), quat_node->Attrib("w")->ValueFloat());

					bind_scale = MathLib::length(bind_real);
					bind_real /= bind_scale;

					bind_dual = MathLib::quat_trans_to_udq(bind_real, bind_pos);
				}
				else
				{
					XMLNodePtr bind_real_node = key_node->FirstNode("real");
					if (!bind_real_node)
					{
						bind_real_node = key_node->FirstNode("bind_real");
					}
					XMLAttributePtr attr = bind_real_node->Attrib("v");
					if (attr)
					{
						ExtractFVector<4>(attr->ValueString(), &bind_real[0]);
					}
					else
					{
						bind_real.x() = bind_real_node->Attrib("x")->ValueFloat();
						bind_real.y() = bind_real_node->Attrib("y")->ValueFloat();
						bind_real.z() = bind_real_node->Attrib("z")->ValueFloat();
						bind_real.w() = bind_real_node->Attrib("w")->ValueFloat();
					}

					XMLNodePtr bind_dual_node = key_node->FirstNode("dual");
					if (!bind_dual_node)
					{
						bind_dual_node = key_node->FirstNode("bind_dual");
					}
					attr = bind_dual_node->Attrib("v");
					if (attr)
					{
						ExtractFVector<4>(attr->ValueString(), &bind_dual[0]);
					}
					else
					{
						bind_dual.x() = bind_dual_node->Attrib("x")->ValueFloat();
						bind_dual.y() = bind_dual_node->Attrib("y")->ValueFloat();
						bind_dual.z() = bind_dual_node->Attrib("z")->ValueFloat();
						bind_dual.w() = bind_dual_node->Attrib("w")->ValueFloat();
					}

					bind_scale = MathLib::length(bind_real);
					bind_real /= bind_scale;
					if (MathLib::SignBit(bind_real.w()) < 0)
					{
						bind_real = -bind_real;
						bind_scale = -bind_scale;
					}
				}

				kfs.bind_real.push_back(bind_real);
				kfs.bind_dual.push_back(bind_dual);
				kfs.bind_scale.push_back(bind_scale);
			}

			this->CompressKeyFrameSet(kfs);
		}

		skinned_model.AttachKeyFrameSets(kfss);
	}

	void MeshLoader::CompileBBKeyFramesChunk(XMLNodePtr const & bb_kfs_chunk, uint32_t mesh_index)
	{
		auto& skinned_model = checked_cast<SkinnedModel&>(*render_model_);
		auto& skinned_mesh = checked_cast<SkinnedMesh&>(*skinned_model.Mesh(mesh_index));

		auto bb_kfs = MakeSharedPtr<AABBKeyFrameSet>();
		if (bb_kfs_chunk)
		{
			for (XMLNodePtr bb_kf_node = bb_kfs_chunk->FirstNode("bb_key_frame"); bb_kf_node;
				bb_kf_node = bb_kf_node->NextSibling("bb_key_frame"))
			{
				bb_kfs->frame_id.clear();
				bb_kfs->bb.clear();

				int32_t frame_id = -1;
				for (XMLNodePtr key_node = bb_kf_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
				{
					XMLAttributePtr id_attr = key_node->Attrib("id");
					if (id_attr)
					{
						frame_id = id_attr->ValueInt();
					}
					else
					{
						++ frame_id;
					}
					bb_kfs->frame_id.push_back(frame_id);

					float3 bb_min, bb_max;
					XMLAttributePtr attr = key_node->Attrib("min");
					if (attr)
					{
						ExtractFVector<3>(attr->ValueString(), &bb_min[0]);
					}
					else
					{
						XMLNodePtr min_node = key_node->FirstNode("min");
						bb_min.x() = min_node->Attrib("x")->ValueFloat();
						bb_min.y() = min_node->Attrib("y")->ValueFloat();
						bb_min.z() = min_node->Attrib("z")->ValueFloat();
					}
					attr = key_node->Attrib("max");
					if (attr)
					{
						ExtractFVector<3>(attr->ValueString(), &bb_max[0]);
					}
					else
					{
						XMLNodePtr max_node = key_node->FirstNode("max");
						bb_max.x() = max_node->Attrib("x")->ValueFloat();
						bb_max.y() = max_node->Attrib("y")->ValueFloat();
						bb_max.z() = max_node->Attrib("z")->ValueFloat();
					}

					bb_kfs->bb.push_back(AABBox(bb_min, bb_max));
				}
			}
		}
		else
		{
			bb_kfs->frame_id.resize(2);
			bb_kfs->bb.resize(2);

			bb_kfs->frame_id[0] = 0;
			bb_kfs->frame_id[1] = skinned_model.NumFrames() - 1;

			bb_kfs->bb[0] = bb_kfs->bb[1] = skinned_mesh.PosBound();
		}

		skinned_mesh.AttachFramePosBounds(bb_kfs);
	}

	void MeshLoader::CompileActionsChunk(XMLNodePtr const & actions_chunk)
	{
		auto& skinned_model = checked_cast<SkinnedModel&>(*render_model_);

		XMLNodePtr action_node;
		if (actions_chunk)
		{
			action_node = actions_chunk->FirstNode("action");
		}

		auto animations = MakeSharedPtr<std::vector<Animation>>();

		Animation animation;
		if (action_node)
		{
			for (; action_node; action_node = action_node->NextSibling("action"))
			{
				animation.name = std::string(action_node->Attrib("name")->ValueString());

				animation.start_frame = action_node->Attrib("start")->ValueUInt();
				animation.end_frame = action_node->Attrib("end")->ValueUInt();

				animations->push_back(animation);
			}
		}
		else
		{
			animation.name = "root";
			animation.start_frame = 0;
			animation.end_frame = skinned_model.NumFrames();

			animations->push_back(animation);
		}

		skinned_model.AttachAnimations(animations);
	}

	void MeshLoader::LoadFromMeshML(std::string_view input_name, MeshMetadata const & metadata)
	{
		KFL_UNUSED(metadata);

		ResIdentifierPtr file = ResLoader::Instance().Open(input_name);
		KlayGE::XMLDocument doc;
		XMLNodePtr root = doc.Parse(*file);

		BOOST_ASSERT(root->Attrib("version") && (root->Attrib("version")->ValueInt() >= 1));

		XMLNodePtr bones_chunk = root->FirstNode("bones_chunk");
		if (bones_chunk)
		{
			this->CompileBonesChunk(bones_chunk);
		}

		bool const skinned = !joints_.empty();

		XMLNodePtr meshes_chunk = root->FirstNode("meshes_chunk");
		if (meshes_chunk)
		{
			this->CompileMeshesChunk(meshes_chunk);
		}

		if (!joints_.empty())
		{
			std::vector<NodeTransform> merged_nodes(joints_.size());

			for (size_t i = 0; i < joints_.size(); ++i)
			{
				std::wstring joint_name;
				Convert(joint_name, joints_[i].name);
				NodeTransform node_transform;
				node_transform.node = MakeSharedPtr<SceneNode>(joint_name, SceneNode::SOA_Cullable);
				if (joints_[i].parent_id >= 0)
				{
					merged_nodes[joints_[i].parent_id].node->AddChild(node_transform.node);
				}
				node_transform.node->TransformToParent(float4x4::Identity());
				node_transform.node->AddComponent(joints_[i].joint);
				node_transform.parent_id = joints_[i].parent_id;
				merged_nodes[i] = std::move(node_transform);
			}

			int16_t start_node_index = static_cast<int16_t>(joints_.size());
			for (size_t i = 0; i < nodes_.size(); ++i)
			{
				if (nodes_[i].parent_id >= 0)
				{
					nodes_[i].parent_id += start_node_index;
				}
				else
				{
					merged_nodes[0].node->AddChild(nodes_[i].node);
					nodes_[i].parent_id = 0;
				}
				merged_nodes.push_back(nodes_[i]);
			}

			nodes_ = std::move(merged_nodes);
		}

		if (skinned)
		{
			render_model_ = MakeSharedPtr<SkinnedModel>(nodes_[0].node);
		}
		else
		{
			render_model_ = MakeSharedPtr<RenderModel>(nodes_[0].node);
		}

		XMLNodePtr materials_chunk = root->FirstNode("materials_chunk");
		if (materials_chunk)
		{
			this->CompileMaterialsChunk(materials_chunk);
		}

		XMLNodePtr key_frames_chunk = root->FirstNode("key_frames_chunk");
		if (key_frames_chunk)
		{
			this->CompileKeyFramesChunk(key_frames_chunk);

			auto& skinned_model = checked_cast<SkinnedModel&>(*render_model_);
			auto& kfs = *skinned_model.GetKeyFrameSets();

			for (size_t i = 0; i < kfs.size(); ++ i)
			{
				auto& kf = kfs[i];
				if (kf.frame_id.empty())
				{
					auto const& joint = *joints_[i].joint;

					Quaternion inv_parent_real;
					Quaternion inv_parent_dual;
					float inv_parent_scale;
					if (joints_[i].parent_id < 0)
					{
						inv_parent_real = Quaternion::Identity();
						inv_parent_dual = Quaternion(0, 0, 0, 0);
						inv_parent_scale = 1;
					}
					else
					{
						auto const& parent_joint = *joints_[joints_[i].parent_id].joint;
						std::tie(inv_parent_real, inv_parent_dual) = MathLib::inverse(parent_joint.BindReal(), parent_joint.BindDual());
						inv_parent_scale = 1 / parent_joint.BindScale();
					}

					kf.frame_id.push_back(0);
					kf.bind_real.push_back(MathLib::mul_real(joint.BindReal(), inv_parent_real));
					kf.bind_dual.push_back(MathLib::mul_dual(joint.BindReal(), joint.BindDual() * inv_parent_scale,
						inv_parent_real, inv_parent_dual));
					kf.bind_scale.push_back(joint.BindScale() * inv_parent_scale);
				}
			}

			XMLNodePtr bb_kfs_chunk = root->FirstNode("bb_key_frames_chunk");
			for (uint32_t mesh_index = 0; mesh_index < skinned_model.NumMeshes(); ++ mesh_index)
			{
				this->CompileBBKeyFramesChunk(bb_kfs_chunk, mesh_index);
			}
		}

		XMLNodePtr actions_chunk = root->FirstNode("actions_chunk");
		if (actions_chunk)
		{
			this->CompileActionsChunk(actions_chunk);
		}
	}

	void MeshLoader::RemoveUnusedJoints()
	{
		std::vector<uint32_t> joint_mapping(joints_.size());
		std::vector<bool> joints_used(joints_.size(), false);

		for (auto const & mesh : meshes_)
		{
			for (auto const & lod : mesh.lods)
			{
				for (auto const & bindings : lod.joint_bindings)
				{
					for (auto const & bind : bindings)
					{
						joints_used[bind.first] = true;
					}
				}
			}
		}

		for (uint32_t ji = 0; ji < joints_.size(); ++ ji)
		{
			if (joints_used[ji])
			{
				int16_t parent_id = joints_[ji].parent_id;
				while ((parent_id != -1) && !joints_used[parent_id])
				{
					joints_used[parent_id] = true;
					parent_id = joints_[parent_id].parent_id;
				}
			}
		}

		uint32_t new_joint_id = 0;
		for (uint32_t ji = 0; ji < joints_.size(); ++ ji)
		{
			if (joints_used[ji])
			{
				joint_mapping[ji] = new_joint_id;
				++ new_joint_id;
			}
		}

		auto& skinned_model = checked_cast<SkinnedModel&>(*render_model_);
		auto& kfs = *skinned_model.GetKeyFrameSets();

		for (uint32_t ji = 0; ji < joints_.size(); ++ ji)
		{
			if (joints_used[ji])
			{
				BOOST_ASSERT(joint_mapping[ji] <= ji);
				joints_[joint_mapping[ji]] = joints_[ji];
				kfs[joint_mapping[ji]] = kfs[ji];
			}
			else
			{
				joints_[ji].joint->BoundSceneNode()->RemoveComponent(joints_[ji].joint.get());
			}
		}
		joints_.resize(new_joint_id);
		kfs.resize(joints_.size());

		for (auto& mesh : meshes_)
		{
			for (auto& lod : mesh.lods)
			{
				for (auto& bindings : lod.joint_bindings)
				{
					for (auto& bind : bindings)
					{
						bind.first = joint_mapping[bind.first];
					}
				}
			}
		}
	}

	void MeshLoader::RemoveUnusedMaterials()
	{
		std::vector<uint32_t> mtl_mapping(render_model_->NumMaterials());
		std::vector<bool> mtl_used(mtl_mapping.size(), false);

		for (auto& mesh : meshes_)
		{
			mtl_used[mesh.mtl_id] = true;
		}

		uint32_t new_mtl_id = 0;
		for (uint32_t i = 0; i < render_model_->NumMaterials(); ++ i)
		{
			if (mtl_used[i])
			{
				mtl_mapping[i] = new_mtl_id;
				++ new_mtl_id;
			}
		}

		for (uint32_t i = 0; i < render_model_->NumMaterials(); ++ i)
		{
			BOOST_ASSERT(mtl_mapping[i] <= i);
			render_model_->GetMaterial(i) = render_model_->GetMaterial(mtl_mapping[i]);
		}
		render_model_->NumMaterials(new_mtl_id);

		for (auto& mesh : meshes_)
		{
			mesh.mtl_id = mtl_mapping[mesh.mtl_id];
		}
	}

	void MeshLoader::CompressKeyFrameSet(KeyFrameSet& kf)
	{
		float const THRESHOLD = 1e-3f;

		BOOST_ASSERT((kf.bind_real.size() == kf.bind_dual.size())
			&& (kf.frame_id.size() == kf.bind_scale.size())
			&& (kf.frame_id.size() == kf.bind_real.size()));

		int base = 0;
		while (base < static_cast<int>(kf.frame_id.size() - 2))
		{
			int const frame0 = kf.frame_id[base + 0];
			int const frame1 = kf.frame_id[base + 1];
			int const frame2 = kf.frame_id[base + 2];
			float const factor = static_cast<float>(frame1 - frame0) / (frame2 - frame0);
			Quaternion interpolate_real;
			Quaternion interpolate_dual;
			std::tie(interpolate_real, interpolate_dual) = MathLib::sclerp(kf.bind_real[base + 0], kf.bind_dual[base + 0],
				kf.bind_real[base + 2], kf.bind_dual[base + 2], factor);
			float const scale = MathLib::lerp(kf.bind_scale[base + 0], kf.bind_scale[base + 2], factor);

			if (MathLib::dot(kf.bind_real[base + 1], interpolate_real) < 0)
			{
				interpolate_real = -interpolate_real;
				interpolate_dual = -interpolate_dual;
			}

			Quaternion diff_real;
			Quaternion diff_dual;
			std::tie(diff_real, diff_dual) = MathLib::inverse(kf.bind_real[base + 1], kf.bind_dual[base + 1]);
			diff_dual = MathLib::mul_dual(diff_real, diff_dual * scale, interpolate_real, interpolate_dual);
			diff_real = MathLib::mul_real(diff_real, interpolate_real);
			float diff_scale = scale * kf.bind_scale[base + 1];

			if ((MathLib::abs(diff_real.x()) < THRESHOLD) && (MathLib::abs(diff_real.y()) < THRESHOLD)
				&& (MathLib::abs(diff_real.z()) < THRESHOLD) && (MathLib::abs(diff_real.w() - 1) < THRESHOLD)
				&& (MathLib::abs(diff_dual.x()) < THRESHOLD) && (MathLib::abs(diff_dual.y()) < THRESHOLD)
				&& (MathLib::abs(diff_dual.z()) < THRESHOLD) && (MathLib::abs(diff_dual.w()) < THRESHOLD)
				&& (MathLib::abs(diff_scale - 1) < THRESHOLD))
			{
				kf.frame_id.erase(kf.frame_id.begin() + base + 1);
				kf.bind_real.erase(kf.bind_real.begin() + base + 1);
				kf.bind_dual.erase(kf.bind_dual.begin() + base + 1);
				kf.bind_scale.erase(kf.bind_scale.begin() + base + 1);
			}
			else
			{
				++ base;
			}
		}
	}


	RenderModelPtr MeshLoader::Load(std::string_view input_name, MeshMetadata const & metadata)
	{
		std::string const input_name_str = ResLoader::Instance().Locate(input_name);
		if (input_name_str.empty())
		{
			LogError() << "Could NOT find " << input_name << '.' << std::endl;
			return RenderModelPtr();
		}

		std::filesystem::path input_path(input_name_str);
		auto const in_folder = input_path.parent_path().string();
		bool const in_path = ResLoader::Instance().IsInPath(in_folder);
		if (!in_path)
		{
			ResLoader::Instance().AddPath(in_folder);
		}

		render_model_.reset();
		meshes_.clear();
		nodes_.clear();
		joints_.clear();
		has_normal_ = false;
		has_tangent_quat_ = false;
		has_texcoord_ = false;
		has_diffuse_ = false;
		has_specular_ = false;

		auto const input_ext = input_path.extension();
		if (input_ext == ".model_bin")
		{
			render_model_ = LoadSoftwareModel(input_name_str);
			return render_model_;
		}
		else if (input_ext == ".meshml")
		{
			this->LoadFromMeshML(input_name_str, metadata);
		}
		else
		{
			this->LoadFromAssimp(input_name_str, metadata);
		}
		if (!render_model_)
		{
			return RenderModelPtr();
		}

		for (uint32_t i = 0; i < metadata.NumMaterials(); ++i)
		{
			std::string_view mtlml_name = metadata.MaterialFileName(i);
			if (!mtlml_name.empty())
			{
				render_model_->GetMaterial(i) = SyncLoadRenderMaterial(metadata.MaterialFileName(i));
			}
		}

		uint32_t const num_lods = static_cast<uint32_t>(meshes_[0].lods.size());
		bool const skinned = !joints_.empty();

		if (skinned)
		{
			this->RemoveUnusedJoints();
		}
		this->RemoveUnusedMaterials();

		auto global_transform = metadata.Transform();
		if (metadata.AutoCenter())
		{
			global_transform = MathLib::translation(-nodes_[0].aabb_local.Center())
				* nodes_[0].node->TransformToParent() * global_transform;
		}

		std::vector<VertexElement> merged_ves;
		std::vector<std::vector<uint8_t>> merged_vertices;
		std::vector<uint8_t> merged_indices;
		std::vector<uint32_t> mesh_num_vertices;
		std::vector<uint32_t> mesh_base_vertices(1, 0);
		std::vector<uint32_t> mesh_num_indices;
		std::vector<uint32_t> mesh_start_indices(1, 0);
		bool is_index_16_bit;

		int position_stream = -1;
		int normal_stream = -1;
		int tangent_quat_stream = -1;
		int diffuse_stream = -1;
		int specular_stream = -1;
		int texcoord_stream = -1;
		int blend_weights_stream = -1;
		int blend_indices_stream = -1;
		{
			int stream_index = 0;
			{
				merged_ves.push_back(VertexElement(VEU_Position, 0, EF_SIGNED_ABGR16));
				position_stream = stream_index;
			}
			if (has_tangent_quat_)
			{
				merged_ves.push_back(VertexElement(VEU_Tangent, 0, EF_ABGR8));
				++ stream_index;
				tangent_quat_stream = stream_index;
			}
			else if (has_normal_)
			{
				merged_ves.push_back(VertexElement(VEU_Normal, 0, EF_ABGR8));
				++ stream_index;
				normal_stream = stream_index;
			}
			if (has_diffuse_)
			{
				merged_ves.push_back(VertexElement(VEU_Diffuse, 0, EF_ABGR8));
				++ stream_index;
				diffuse_stream = stream_index;
			}
			if (has_specular_)
			{
				merged_ves.push_back(VertexElement(VEU_Specular, 0, EF_ABGR8));
				++ stream_index;
				specular_stream = stream_index;
			}
			if (has_texcoord_)
			{
				merged_ves.push_back(VertexElement(VEU_TextureCoord, 0, EF_SIGNED_GR16));
				++ stream_index;
				texcoord_stream = stream_index;
			}

			if (skinned)
			{
				merged_ves.push_back(VertexElement(VEU_BlendWeight, 0, EF_ABGR8));
				++ stream_index;
				blend_weights_stream = stream_index;

				merged_ves.push_back(VertexElement(VEU_BlendIndex, 0, EF_ABGR8UI));
				++ stream_index;
				blend_indices_stream = stream_index;
			}

			merged_vertices.resize(merged_ves.size());
		}

		{
			for (auto const & mesh : meshes_)
			{
				auto const & pos_bb = mesh.pos_bb;
				auto const & tc_bb = mesh.tc_bb;

				float3 const pos_center = pos_bb.Center();
				float3 const pos_extent = pos_bb.HalfSize();
				float3 const tc_center = tc_bb.Center();
				float3 const tc_extent = tc_bb.HalfSize();

				for (uint32_t lod = 0; lod < num_lods; ++ lod)
				{
					auto& mesh_lod = mesh.lods[lod];

					for (auto const & position : mesh_lod.positions)
					{
						float3 const pos = (position - pos_center) / pos_extent * 0.5f + 0.5f;
						int16_t const s_pos[] =
						{
							static_cast<int16_t>(
								MathLib::clamp<int32_t>(static_cast<int32_t>(pos.x() * 65535 - 32768), -32768, 32767)),
							static_cast<int16_t>(
								MathLib::clamp<int32_t>(static_cast<int32_t>(pos.y() * 65535 - 32768), -32768, 32767)),
							static_cast<int16_t>(
								MathLib::clamp<int32_t>(static_cast<int32_t>(pos.z() * 65535 - 32768), -32768, 32767)),
							32767
						};

						uint8_t const * p = reinterpret_cast<uint8_t const *>(s_pos);
						merged_vertices[position_stream].insert(merged_vertices[position_stream].end(), p, p + sizeof(s_pos));
					}
					if (normal_stream != -1)
					{
						for (auto const & n : mesh_lod.normals)
						{
							float3 const normal = MathLib::normalize(n) * 0.5f + 0.5f;
							uint32_t const compact = MathLib::clamp(static_cast<uint32_t>(normal.x() * 255 + 0.5f), 0U, 255U)
								| (MathLib::clamp(static_cast<uint32_t>(normal.y() * 255 + 0.5f), 0U, 255U) << 8)
								| (MathLib::clamp(static_cast<uint32_t>(normal.z() * 255 + 0.5f), 0U, 255U) << 16);

							uint8_t const * p = reinterpret_cast<uint8_t const *>(&compact);
							merged_vertices[normal_stream].insert(merged_vertices[normal_stream].end(), p, p + sizeof(compact));
						}
					}
					if (tangent_quat_stream != -1)
					{
						for (size_t i = 0; i < mesh_lod.tangents.size(); ++ i)
						{
							float3 const tangent = MathLib::normalize(mesh_lod.tangents[i]);
							float3 const binormal = MathLib::normalize(mesh_lod.binormals[i]);
							float3 const normal = MathLib::normalize(mesh_lod.normals[i]);

							Quaternion const tangent_quat = MathLib::to_quaternion(tangent, binormal, normal, 8);

							uint32_t const compact = (
								MathLib::clamp(
									static_cast<uint32_t>((tangent_quat.x() * 0.5f + 0.5f) * 255 + 0.5f), 0U, 255U) << 0)
								| (MathLib::clamp(
									static_cast<uint32_t>((tangent_quat.y() * 0.5f + 0.5f) * 255 + 0.5f), 0U, 255U) << 8)
								| (MathLib::clamp(
									static_cast<uint32_t>((tangent_quat.z() * 0.5f + 0.5f) * 255 + 0.5f), 0U, 255U) << 16)
								| (MathLib::clamp(
									static_cast<uint32_t>((tangent_quat.w() * 0.5f + 0.5f) * 255 + 0.5f), 0U, 255U) << 24);

							uint8_t const * p = reinterpret_cast<uint8_t const *>(&compact);
							merged_vertices[tangent_quat_stream].insert(merged_vertices[tangent_quat_stream].end(),
								p, p + sizeof(compact));
						}
					}
					if (diffuse_stream != -1)
					{
						for (auto const & diffuse : mesh_lod.diffuses)
						{
							uint32_t const clr = diffuse.ABGR();

							uint8_t const * p = reinterpret_cast<uint8_t const *>(&clr);
							merged_vertices[tangent_quat_stream].insert(merged_vertices[tangent_quat_stream].end(),
								p, p + sizeof(clr));
						}
					}
					if (specular_stream != -1)
					{
						for (auto const & specular : mesh_lod.speculars)
						{
							uint32_t const clr = specular.ABGR();

							uint8_t const * p = reinterpret_cast<uint8_t const *>(&clr);
							merged_vertices[tangent_quat_stream].insert(merged_vertices[tangent_quat_stream].end(),
								p, p + sizeof(clr));
						}
					}
					if (texcoord_stream != -1)
					{
						for (auto const & tc : mesh_lod.texcoords[0])
						{
							float3 tex_coord = float3(tc.x(), tc.y(), 0.0f);
							tex_coord = (tex_coord - tc_center) / tc_extent * 0.5f + 0.5f;
							int16_t const s_tc[2] =
							{
								static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tex_coord.x() * 65535 - 32768),
									-32768, 32767)),
								static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tex_coord.y() * 65535 - 32768),
									-32768, 32767)),
							};

							uint8_t const * p = reinterpret_cast<uint8_t const *>(s_tc);
							merged_vertices[texcoord_stream].insert(merged_vertices[texcoord_stream].end(), p, p + sizeof(s_tc));
						}
					}

					if (blend_weights_stream != -1)
					{
						BOOST_ASSERT(blend_indices_stream != -1);

						for (auto const & binding : mesh_lod.joint_bindings)
						{
							size_t constexpr MAX_BINDINGS = 4;

							float total_weight = 0;
							size_t const num = std::min(MAX_BINDINGS, binding.size());
							for (size_t wi = 0; wi < num; ++ wi)
							{
								total_weight += binding[wi].second;
							}

							uint8_t joint_ids[MAX_BINDINGS];
							uint8_t weights[MAX_BINDINGS];
							for (size_t wi = 0; wi < num; ++ wi)
							{
								joint_ids[wi] = static_cast<uint8_t>(binding[wi].first);

								float const w = binding[wi].second / total_weight;
								weights[wi] = static_cast<uint8_t>(MathLib::clamp(static_cast<uint32_t>(w * 255 + 0.5f), 0U, 255U));
							}
							for (size_t wi = num; wi < MAX_BINDINGS; ++ wi)
							{
								joint_ids[wi] = 0;
								weights[wi] = 0;
							}

							merged_vertices[blend_weights_stream].insert(merged_vertices[blend_weights_stream].end(),
								weights, weights + sizeof(weights));
							merged_vertices[blend_indices_stream].insert(merged_vertices[blend_indices_stream].end(),
								joint_ids, joint_ids + sizeof(joint_ids));
						}
					}

					mesh_num_vertices.push_back(static_cast<uint32_t>(mesh_lod.positions.size()));
					mesh_base_vertices.push_back(mesh_base_vertices.back() + mesh_num_vertices.back());
				}
			}
		}

		{
			uint32_t max_index = 0;
			for (auto const & mesh : meshes_)
			{
				for (auto const & mesh_lod : mesh.lods)
				{
					for (auto index : mesh_lod.indices)
					{
						max_index = std::max(max_index, index);
					}
				}
			}

			is_index_16_bit = (max_index < 0xFFFF);

			for (auto const & mesh : meshes_)
			{
				for (auto const & mesh_lod : mesh.lods)
				{
					for (size_t i = 0; i < mesh_lod.indices.size(); i += 3)
					{
						uint32_t triangle[] = 
						{
							mesh_lod.indices[i + 0],
							mesh_lod.indices[i + 1],
							mesh_lod.indices[i + 2]
						};

						if (metadata.FlipWindingOrder())
						{
							std::swap(triangle[1], triangle[2]);
						}

						if (is_index_16_bit)
						{
							uint16_t const triangle_16[]
							{
								static_cast<uint16_t>(triangle[0]),
								static_cast<uint16_t>(triangle[1]),
								static_cast<uint16_t>(triangle[2])
							};

							uint8_t const * p = reinterpret_cast<uint8_t const *>(triangle_16);
							merged_indices.insert(merged_indices.end(), p, p + sizeof(triangle_16));
						}
						else
						{
							uint8_t const * p = reinterpret_cast<uint8_t const *>(triangle);
							merged_indices.insert(merged_indices.end(), p, p + sizeof(triangle));
						}
					}

					mesh_num_indices.push_back(static_cast<uint32_t>(mesh_lod.indices.size()));
					mesh_start_indices.push_back(mesh_start_indices.back() + mesh_num_indices.back());
				}
			}
		}

		std::vector<GraphicsBufferPtr> merged_vbs(merged_vertices.size());
		for (size_t i = 0; i < merged_vertices.size(); ++ i)
		{
			auto vb = MakeSharedPtr<SoftwareGraphicsBuffer>(static_cast<uint32_t>(merged_vertices[i].size()), false);
			vb->CreateHWResource(merged_vertices[i].data());

			merged_vbs[i] = vb;
		}
		auto merged_ib = MakeSharedPtr<SoftwareGraphicsBuffer>(static_cast<uint32_t>(merged_indices.size()), false);
		merged_ib->CreateHWResource(merged_indices.data());

		uint32_t mesh_lod_index = 0;
		std::vector<StaticMeshPtr> render_meshes;
		for (auto const & mesh : meshes_)
		{
			std::wstring wname;
			KlayGE::Convert(wname, mesh.name);

			StaticMeshPtr render_mesh;
			if (skinned)
			{
				render_mesh = MakeSharedPtr<SkinnedMesh>(wname);
			}
			else
			{
				render_mesh = MakeSharedPtr<StaticMesh>(wname);
			}
			render_meshes.push_back(render_mesh);

			render_mesh->MaterialID(mesh.mtl_id);
			render_mesh->PosBound(mesh.pos_bb);
			render_mesh->TexcoordBound(mesh.tc_bb);

			render_mesh->NumLods(num_lods);
			for (uint32_t lod = 0; lod < num_lods; ++ lod, ++ mesh_lod_index)
			{
				for (uint32_t ve_index = 0; ve_index < merged_vbs.size(); ++ ve_index)
				{
					render_mesh->AddVertexStream(lod, merged_vbs[ve_index], merged_ves[ve_index]);
				}
				render_mesh->AddIndexStream(lod, merged_ib, is_index_16_bit ? EF_R16UI : EF_R32UI);

				render_mesh->NumVertices(lod, mesh_num_vertices[mesh_lod_index]);
				render_mesh->NumIndices(lod, mesh_num_indices[mesh_lod_index]);
				render_mesh->StartVertexLocation(lod, mesh_base_vertices[mesh_lod_index]);
				render_mesh->StartIndexLocation(lod, mesh_start_indices[mesh_lod_index]);
			}
		}

		if (skinned)
		{
			auto& skinned_model = checked_cast<SkinnedModel&>(*render_model_);

			std::vector<JointComponentPtr> joints;
			for (auto& joint : joints_)
			{
				joint.joint->InitInverseOriginParams();
				joints.push_back(joint.joint);
			}
			skinned_model.AssignJoints(joints.begin(), joints.end());

			// TODO: Run skinning on CPU to get the bounding box
			for (uint32_t mesh_index = 0; mesh_index < render_meshes.size(); ++ mesh_index)
			{
				auto& skinned_mesh = checked_cast<SkinnedMesh&>(*render_meshes[mesh_index]);

				auto frame_pos_aabbs = MakeSharedPtr<AABBKeyFrameSet>();

				frame_pos_aabbs->frame_id.resize(2);
				frame_pos_aabbs->bb.resize(2);

				frame_pos_aabbs->frame_id[0] = 0;
				frame_pos_aabbs->frame_id[1] = skinned_model.NumFrames() - 1;

				frame_pos_aabbs->bb[0] = skinned_mesh.PosBound();
				frame_pos_aabbs->bb[1] = skinned_mesh.PosBound();

				skinned_mesh.AttachFramePosBounds(frame_pos_aabbs);
			}
		}

		render_model_->AssignMeshes(render_meshes.begin(), render_meshes.end());

		for (auto const & node : nodes_)
		{
			for (auto const mesh_index : node.mesh_indices)
			{
				node.node->AddComponent(MakeSharedPtr<RenderableComponent>(render_meshes[mesh_index]));
			}
		}

		nodes_[0].node->UpdatePosBoundSubtree();
		nodes_[0].node->TransformToParent(nodes_[0].node->TransformToParent() * global_transform);

		if (!in_path)
		{
			ResLoader::Instance().DelPath(in_folder);
		}

		return render_model_;
	}
}

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

namespace KlayGE
{
	RenderModelPtr MeshConverter::Load(std::string_view input_name, MeshMetadata const & metadata)
	{
		MeshLoader ml;
		return ml.Load(input_name, metadata);
	}

	void MeshConverter::Save(RenderModel& model, std::string_view output_name)
	{
		MeshSaver ms;
		ms.Save(model, output_name);
	}
}

