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
#include <KFL/Math.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/ResLoader.hpp>

#include <cstring>
#include <iostream>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore int enum
#endif
#include <assimp/cimport.h>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <KlayGE/MeshConverter.hpp>

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

	Quaternion AiQuatToQuat(aiQuaternion const& v)
	{
		return Quaternion(v.x, v.y, v.z, v.w);
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
}

namespace KlayGE
{
	void MeshConverter::RecursiveTransformMesh(uint32_t lod, float4x4 const & parent_mat, aiNode const * node)
	{
		auto const trans_mat = MathLib::transpose(float4x4(&node->mTransformation.a1)) * parent_mat;
		auto const trans_quat = MathLib::to_quaternion(trans_mat);

		for (uint32_t n = 0; n < node->mNumMeshes; ++ n)
		{
			auto& mesh = meshes_[node->mMeshes[n]];

			for (size_t vi = 0; vi < mesh.lods[lod].positions.size(); ++ vi)
			{
				auto const pos = MathLib::transform_coord(mesh.lods[lod].positions[vi], trans_mat);
				mesh.lods[lod].positions[vi] = pos;

				if (mesh.has_tangent_frame)
				{
					auto const quat = mesh.lods[lod].tangent_quats[vi] * trans_quat;
					mesh.lods[lod].tangent_quats[vi] = quat;
				}
				else
				{
					auto const normal = MathLib::transform_normal(mesh.lods[lod].normals[vi], trans_mat);
					mesh.lods[lod].normals[vi] = normal;
				}
			}

			for (size_t wi = 0; wi < mesh.lods[lod].joint_binding.size(); ++ wi)
			{
				auto& binding = mesh.lods[lod].joint_binding[wi];
				std::sort(binding.begin(), binding.end(),
					[](std::pair<uint32_t, float> const & lhs, std::pair<uint32_t, float> const & rhs)
					{
						return lhs.second > rhs.second;
					});
			}
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++ i)
		{
			this->RecursiveTransformMesh(lod, trans_mat, node->mChildren[i]);
		}
	}

	void MeshConverter::BuildMaterials(aiScene const * scene)
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

			aiString ai_name;
			aiColor4D ai_albedo;
			float ai_opacity;
			float ai_shininess;
			aiColor4D ai_emissive;
			int ai_two_sided;

			auto mtl = scene->mMaterials[mi];

			if (AI_SUCCESS == aiGetMaterialString(mtl, AI_MATKEY_NAME, &ai_name))
			{
				name = ai_name.C_Str();
			}

			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &ai_albedo))
			{
				albedo = Color4ToFloat3(ai_albedo);
			}
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &ai_emissive))
			{
				emissive = Color4ToFloat3(ai_emissive);
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
			shininess = MathLib::clamp(shininess, 1.0f, MAX_SHININESS);

			if ((opacity < 1) || (aiGetMaterialTextureCount(mtl, aiTextureType_OPACITY) > 0))
			{
				transparent = true;
			}

			max = 1;
			if (AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &ai_two_sided, &max))
			{
				two_sided = ai_two_sided ? true : false;
			}

			render_model_->GetMaterial(mi) = MakeSharedPtr<RenderMaterial>();
			auto& render_mtl = *render_model_->GetMaterial(mi);
			render_mtl.name = name;
			render_mtl.albedo = float4(albedo.x(), albedo.y(), albedo.z(), opacity);
			render_mtl.metalness = metalness;
			render_mtl.glossiness = Shininess2Glossiness(shininess);
			render_mtl.emissive = emissive;
			render_mtl.transparent = transparent;
			render_mtl.alpha_test = 0;
			render_mtl.sss = false;
			render_mtl.two_sided = two_sided;

			unsigned int count = aiGetMaterialTextureCount(mtl, aiTextureType_DIFFUSE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE, 0, &str, 0, 0, 0, 0, 0, 0);

				render_mtl.tex_names[RenderMaterial::TS_Albedo] = str.C_Str();
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_SHININESS);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_SHININESS, 0, &str, 0, 0, 0, 0, 0, 0);

				render_mtl.tex_names[RenderMaterial::TS_Glossiness] = str.C_Str();
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_EMISSIVE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_EMISSIVE, 0, &str, 0, 0, 0, 0, 0, 0);

				render_mtl.tex_names[RenderMaterial::TS_Emissive] = str.C_Str();
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_NORMALS);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_NORMALS, 0, &str, 0, 0, 0, 0, 0, 0);

				render_mtl.tex_names[RenderMaterial::TS_Normal] = str.C_Str();
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_HEIGHT);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_HEIGHT, 0, &str, 0, 0, 0, 0, 0, 0);

				render_mtl.tex_names[RenderMaterial::TS_Height] = str.C_Str();
			}

			render_mtl.detail_mode = RenderMaterial::SDM_Parallax;
			if (render_mtl.tex_names[RenderMaterial::TS_Height].empty())
			{
				render_mtl.height_offset_scale = float2(0, 0);
			}
			else
			{
				render_mtl.height_offset_scale = float2(-0.5f, 0.06f);
			}
			render_mtl.tess_factors = float4(5, 5, 1, 9);
		}
	}

	void MeshConverter::BuildMeshData(std::vector<std::shared_ptr<aiScene const>> const & scene_lods, MeshMetadata const & input_metadata)
	{
		float3 center(0, 0, 0);
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
					positions[vi] = MathLib::transform_coord(float3(&mesh->mVertices[vi].x), input_metadata.Transform()) - center;

					if (has_normal)
					{
						normals[vi] = MathLib::transform_normal(float3(&mesh->mNormals[vi].x), input_metadata.TransformIT());
					}
					if (has_tangent)
					{
						tangents[vi] = MathLib::transform_normal(float3(&mesh->mTangents[vi].x), input_metadata.Transform());
					}
					if (has_binormal)
					{
						binormals[vi] = MathLib::transform_normal(float3(&mesh->mBitangents[vi].x), input_metadata.Transform());
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

				auto& tangent_quats = meshes_[mi].lods[lod].tangent_quats;
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
					meshes_[mi].lods[lod].joint_binding.resize(mesh->mNumVertices);

					for (unsigned int bi = 0; bi < mesh->mNumBones; ++ bi)
					{
						aiBone* bone = mesh->mBones[bi];
						bool found = false;
						for (uint32_t ji = 0; ji < joints_.size(); ++ ji)
						{
							if (joints_[ji].name == bone->mName.C_Str())
							{
								for (unsigned int wi = 0; wi < bone->mNumWeights; ++ wi)
								{
									int const vertex_id = bone->mWeights[wi].mVertexId;
									float const weight = bone->mWeights[wi].mWeight;
									meshes_[mi].lods[lod].joint_binding[vertex_id].push_back({ ji, weight });
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
				}
			}

			if (input_metadata.AutoCenter() && (lod == 0))
			{
				auto aabb = MathLib::compute_aabbox(meshes_[0].lods[lod].positions.begin(), meshes_[0].lods[lod].positions.end());
				for (unsigned int mi = 0; mi < scene_lods[lod]->mNumMeshes; ++ mi)
				{
					auto aabb_mesh = MathLib::compute_aabbox(meshes_[mi].lods[lod].positions.begin(),
						meshes_[mi].lods[lod].positions.end());
					aabb = AABBox(MathLib::minimize(aabb.Min(), aabb_mesh.Min()), MathLib::maximize(aabb.Max(), aabb_mesh.Max()));
				}
				center = aabb.Center();
				for (unsigned int mi = 0; mi < scene_lods[lod]->mNumMeshes; ++ mi)
				{
					for (auto& pos : meshes_[mi].lods[lod].positions)
					{
						pos -= center;
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

	void MeshConverter::BuildJoints(aiScene const * scene)
	{
		std::map<std::string, Joint> joint_nodes;

		std::function<void(aiNode const *, float4x4 const &)> build_bind_matrix =
			[&build_bind_matrix, &joint_nodes, scene](aiNode const * meshNode, float4x4 const & parent_mat)
		{
			float4x4 const mesh_trans = MathLib::transpose(float4x4(&meshNode->mTransformation.a1)) * parent_mat;
			for (unsigned int i = 0; i < meshNode->mNumMeshes; ++ i)
			{
				aiMesh const * mesh = scene->mMeshes[meshNode->mMeshes[i]];
				for (unsigned int ibone = 0; ibone < mesh->mNumBones; ++ ibone)
				{
					aiBone const * bone = mesh->mBones[ibone];

					Joint joint;
					joint.name = bone->mName.C_Str();

					auto const bone_to_mesh = MathLib::inverse(MathLib::transpose(float4x4(&bone->mOffsetMatrix.a1))) * mesh_trans;					
					MatrixToDQ(bone_to_mesh, joint.bind_real, joint.bind_dual, joint.bind_scale);

					joint_nodes[joint.name] = joint;
				}
			}

			for (unsigned int i = 0; i < meshNode->mNumChildren; ++ i)
			{
				build_bind_matrix(meshNode->mChildren[i], mesh_trans);
			}
		};

		std::function<bool(aiNode const *)> mark_joint_nodes = [&mark_joint_nodes, &joint_nodes](aiNode const * root)
		{
			std::string name = root->mName.C_Str();
			bool child_has_bone = false;

			auto iter = joint_nodes.find(name);
			if (iter != joint_nodes.end())
			{
				child_has_bone = true;
			}

			for (unsigned int i = 0; i < root->mNumChildren; ++ i)
			{
				child_has_bone = mark_joint_nodes(root->mChildren[i]) || child_has_bone;
			}

			if (child_has_bone && (iter == joint_nodes.end()))
			{
				Joint joint;
				joint.name = name;
				joint.bind_real = Quaternion::Identity();
				joint.bind_dual = Quaternion(0, 0, 0, 0);
				joint.bind_scale = 1;
				joint_nodes[name] = joint;
			}

			return child_has_bone;
		};

		std::function<void(aiNode const *, int)> alloc_joints =
			[this, &joint_nodes, &alloc_joints](aiNode const * root, int parent_id)
		{
			std::string name = root->mName.C_Str();
			int joint_id = -1;
			auto iter = joint_nodes.find(name);
			if (iter != joint_nodes.end())
			{
				joint_id = static_cast<int>(joints_.size());

				auto const local_matrix = MathLib::transpose(float4x4(&root->mTransformation.a1));
				// Borrow those variables to store a local matrix
				MatrixToDQ(local_matrix, iter->second.inverse_origin_real, iter->second.inverse_origin_dual,
					iter->second.inverse_origin_scale);

				iter->second.parent = static_cast<int16_t>(parent_id);

				joints_.push_back(iter->second);
			}

			for (unsigned int i = 0; i < root->mNumChildren; ++ i)
			{
				alloc_joints(root->mChildren[i], joint_id);
			}
		};

		build_bind_matrix(scene->mRootNode, float4x4::Identity());
		mark_joint_nodes(scene->mRootNode);
		alloc_joints(scene->mRootNode, -1);
	}

	void MeshConverter::BuildActions(aiScene const * scene)
	{
		auto& skinned_model = *checked_pointer_cast<SkinnedModel>(render_model_);

		struct Animation
		{
			std::string name;
			int frame_num;
			std::map<int/*joint_id*/, KeyFrameSet> resampled_frames;
		};

		std::vector<Animation> animations;

		int const resample_fps = 25;
		// for actions
		for (unsigned int ianim = 0; ianim < scene->mNumAnimations; ++ ianim)
		{
			aiAnimation const * cur_anim = scene->mAnimations[ianim];
			float duration = static_cast<float>(cur_anim->mDuration / cur_anim->mTicksPerSecond);
			Animation anim;
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

					// resample
					this->ResampleJointTransform(anim.resampled_frames[joint_id], 0, anim.frame_num,
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
					// Borrow those variables to store a local matrix
					default_tf.bind_real.push_back(joints_[ji].inverse_origin_real);
					default_tf.bind_dual.push_back(joints_[ji].inverse_origin_dual);
					default_tf.bind_scale.push_back(joints_[ji].inverse_origin_scale);
					anim.resampled_frames.emplace(joint_id, default_tf);
				}
			}

			animations.push_back(anim);
		}

		auto kfs = MakeSharedPtr<std::vector<KeyFrameSet>>(joints_.size());
		auto actions = MakeSharedPtr<std::vector<AnimationAction>>();
		int action_frame_offset = 0;
		for (auto const & anim : animations)
		{
			AnimationAction action;
			action.name = anim.name;
			action.start_frame = action_frame_offset;
			action.end_frame = action_frame_offset + anim.frame_num;
			actions->push_back(action);

			for (auto const & frame : anim.resampled_frames)
			{
				int joint_id = frame.first;
				auto& kf = (*kfs)[joint_id];
				for (size_t f = 0; f < frame.second.frame_id.size(); ++ f)
				{
					int const shifted_frame = frame.second.frame_id[f] + action_frame_offset;

					kf.frame_id.push_back(shifted_frame);
					kf.bind_real.push_back(frame.second.bind_real[f]);
					kf.bind_dual.push_back(frame.second.bind_dual[f]);
					kf.bind_scale.push_back(frame.second.bind_scale[f]);
				}
			}

			action_frame_offset = action_frame_offset + anim.frame_num;
		}

		for (size_t i = 0; i < kfs->size(); ++ i)
		{
			CompressKeyFrameSet((*kfs)[i]);
		}

		skinned_model.AttachKeyFrameSets(kfs);
		skinned_model.AttachActions(actions);

		skinned_model.FrameRate(resample_fps);
		skinned_model.NumFrames(action_frame_offset);
	}

	void MeshConverter::ResampleJointTransform(KeyFrameSet& rkf, int start_frame, int end_frame, float fps_scale,
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

	void MeshConverter::CompressKeyFrameSet(KeyFrameSet& kf)
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

	RenderModelPtr MeshConverter::Convert(std::string_view input_name, MeshMetadata const & metadata)
	{
		std::string const in_name_str = ResLoader::Instance().Locate(input_name);
		if (in_name_str.empty())
		{
			LogError() << "Could NOT find " << input_name << '.' << std::endl;
			return RenderModelPtr();
		}

		auto in_folder = std::filesystem::path(ResLoader::Instance().Locate(in_name_str)).parent_path().string();
		bool const in_path = ResLoader::Instance().IsInPath(in_folder);
		if (!in_path)
		{
			ResLoader::Instance().AddPath(in_folder);
		}

		auto ai_property_store_deleter = [](aiPropertyStore* props)
		{
			aiReleasePropertyStore(props);
		};

		std::unique_ptr<aiPropertyStore, decltype(ai_property_store_deleter)> props(aiCreatePropertyStore(), ai_property_store_deleter);
		aiSetImportPropertyInteger(props.get(), AI_CONFIG_IMPORT_TER_MAKE_UVS, 1);
		aiSetImportPropertyFloat(props.get(), AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80);
		aiSetImportPropertyInteger(props.get(), AI_CONFIG_PP_SBP_REMOVE, 0);

		aiSetImportPropertyInteger(props.get(), AI_CONFIG_GLOB_MEASURE_TIME, 1);

		unsigned int ppsteps = aiProcess_JoinIdenticalVertices // join identical vertices/ optimize indexing
			| aiProcess_ValidateDataStructure // perform a full validation of the loader's output
			| aiProcess_RemoveRedundantMaterials // remove redundant materials
			| aiProcess_FindInstances; // search for instanced meshes and remove them by references to one master

		uint32_t const num_lods = static_cast<uint32_t>(metadata.NumLods());

		auto ai_scene_deleter = [](aiScene const * scene)
		{
			aiReleaseImport(scene);
		};

		std::vector<std::shared_ptr<aiScene const>> scenes(num_lods);
		for (uint32_t lod = 0; lod < num_lods; ++ lod)
		{
			std::string_view const lod_file_name = (lod == 0) ? in_name_str : metadata.LodFileName(lod);
			std::string const file_name = (lod == 0) ? in_name_str : ResLoader::Instance().Locate(lod_file_name);
			if (file_name.empty())
			{
				LogError() << "Could NOT find " << lod_file_name << " for LoD " << lod << '.' << std::endl;
				return RenderModelPtr();
			}

			scenes[lod].reset(aiImportFileExWithProperties(file_name.c_str(),
				ppsteps // configurable pp steps
				| aiProcess_GenSmoothNormals // generate smooth normal vectors if not existing
				| aiProcess_Triangulate // triangulate polygons with more than 3 edges
				| aiProcess_ConvertToLeftHanded // convert everything to D3D left handed space
				| aiProcess_FixInfacingNormals, // find normals facing inwards and inverts them
				nullptr, props.get()), ai_scene_deleter);

			if (!scenes[lod])
			{
				LogError() << "Assimp: Import file " << lod_file_name << " error: " << aiGetErrorString() << std::endl;
				return RenderModelPtr();
			}
		}

		this->BuildJoints(scenes[0].get());

		bool const skinned = !joints_.empty();

		if (skinned)
		{
			render_model_ = MakeSharedPtr<SkinnedModel>(L"Software");
		}
		else
		{
			render_model_ = MakeSharedPtr<RenderModel>(L"Software");
		}

		this->BuildMaterials(scenes[0].get());

		meshes_.resize(scenes[0]->mNumMeshes);
		for (size_t mi = 0; mi < meshes_.size(); ++ mi)
		{
			meshes_[mi].lods.resize(num_lods);
		}

		has_normal_ = false;
		has_tangent_quat_ = false;
		has_texcoord_ = false;
		this->BuildMeshData(scenes, metadata);
		for (uint32_t lod = 0; lod < num_lods; ++ lod)
		{
			this->RecursiveTransformMesh(lod, float4x4::Identity(), scenes[lod]->mRootNode);
		}

		std::vector<AABBox> pos_bbs(meshes_.size());
		std::vector<AABBox> tc_bbs(meshes_.size());
		for (size_t mi = 0; mi < meshes_.size(); ++ mi)
		{
			auto& mesh = meshes_[mi].lods[0];

			pos_bbs[mi] = MathLib::compute_aabbox(mesh.positions.begin(), mesh.positions.end());
			tc_bbs[mi] = MathLib::compute_aabbox(mesh.texcoords[0].begin(), mesh.texcoords[0].end());
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
			uint32_t mesh_lod_index = 0;
			for (uint32_t mesh_index = 0; mesh_index < meshes_.size(); ++ mesh_index)
			{
				float3 const pos_center = pos_bbs[mesh_index].Center();
				float3 const pos_extent = pos_bbs[mesh_index].HalfSize();
				float3 const tc_center = tc_bbs[mesh_index].Center();
				float3 const tc_extent = tc_bbs[mesh_index].HalfSize();
				for (uint32_t lod = 0; lod < num_lods; ++ lod, ++ mesh_lod_index)
				{
					auto& mesh = meshes_[mesh_index].lods[lod];

					for (size_t index = 0; index < mesh.positions.size(); ++ index)
					{
						float3 const pos = (mesh.positions[index] - pos_center) / pos_extent * 0.5f + 0.5f;
						int16_t const s_pos[] =
						{
							static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.x() * 65535 - 32768), -32768, 32767)),
							static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.y() * 65535 - 32768), -32768, 32767)),
							static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(pos.z() * 65535 - 32768), -32768, 32767)),
							32767
						};

						uint8_t const * p = reinterpret_cast<uint8_t const *>(s_pos);
						merged_vertices[position_stream].insert(merged_vertices[position_stream].end(), p, p + sizeof(s_pos));
					}
					if (normal_stream != -1)
					{
						for (size_t index = 0; index < mesh.normals.size(); ++ index)
						{
							float3 const normal = MathLib::normalize(mesh.normals[index]) * 0.5f + 0.5f;
							uint32_t compact = MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.x() * 255), 0, 255)
								| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.y() * 255), 0, 255) << 8)
								| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal.z() * 255), 0, 255) << 16);

							uint8_t const * p = reinterpret_cast<uint8_t const *>(&compact);
							merged_vertices[normal_stream].insert(merged_vertices[normal_stream].end(), p, p + sizeof(compact));
						}
					}
					if (tangent_quat_stream != -1)
					{
						for (size_t index = 0; index < mesh.normals.size(); ++ index)
						{
							Quaternion const & tangent_quat = mesh.tangent_quats[index];
							uint32_t compact = (
								MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
								| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
								| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
								| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat.w() * 0.5f + 0.5f) * 255), 0, 255) << 24);

							uint8_t const * p = reinterpret_cast<uint8_t const *>(&compact);
							merged_vertices[tangent_quat_stream].insert(merged_vertices[tangent_quat_stream].end(), p, p + sizeof(compact));
						}
					}
					if (texcoord_stream != -1)
					{
						for (size_t index = 0; index < mesh.texcoords[0].size(); ++ index)
						{
							float3 tex_coord = float3(mesh.texcoords[0][index].x(), mesh.texcoords[0][index].y(), 0.0f);
							tex_coord = (tex_coord - tc_center) / tc_extent * 0.5f + 0.5f;
							int16_t s_tc[2] =
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

						for (size_t index = 0; index < mesh.joint_binding.size(); ++ index)
						{
							auto const & binding = mesh.joint_binding[index];

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
								weights[wi] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(w * 255), 0, 255));
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

					mesh_num_vertices.push_back(static_cast<uint32_t>(mesh.positions.size()));
					mesh_base_vertices.push_back(mesh_base_vertices.back() + mesh_num_vertices.back());
				}
			}
		}

		{
			uint32_t max_index = 0;
			for (uint32_t mesh_index = 0; mesh_index < meshes_.size(); ++ mesh_index)
			{
				auto& mesh = meshes_[mesh_index];
				for (uint32_t lod = 0; lod < num_lods; ++ lod)
				{
					for (size_t index = 0; index < mesh.lods[lod].indices.size(); ++ index)
					{
						max_index = std::max(max_index, mesh.lods[lod].indices[index]);
					}
				}
			}

			is_index_16_bit = (max_index < 0xFFFF);

			uint32_t mesh_lod_index = 0;
			for (uint32_t mesh_index = 0; mesh_index < meshes_.size(); ++ mesh_index)
			{
				auto& mesh = meshes_[mesh_index];
				for (uint32_t lod = 0; lod < num_lods; ++ lod, ++ mesh_lod_index)
				{
					for (size_t index = 0; index < mesh.lods[lod].indices.size(); ++ index)
					{
						if (is_index_16_bit)
						{
							uint16_t const i16 = static_cast<uint16_t>(mesh.lods[lod].indices[index]);

							uint8_t const * p = reinterpret_cast<uint8_t const *>(&i16);
							merged_indices.insert(merged_indices.end(), p, p + sizeof(i16));
						}
						else
						{
							uint32_t const i32 = mesh.lods[lod].indices[index];

							uint8_t const * p = reinterpret_cast<uint8_t const *>(&i32);
							merged_indices.insert(merged_indices.end(), p, p + sizeof(i32));
						}
					}

					mesh_num_indices.push_back(static_cast<uint32_t>(mesh.lods[lod].indices.size()));
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
		std::vector<StaticMeshPtr> render_meshes(meshes_.size());
		for (uint32_t mesh_index = 0; mesh_index < meshes_.size(); ++ mesh_index)
		{
			std::wstring wname;
			KlayGE::Convert(wname, meshes_[mesh_index].name);

			if (skinned)
			{
				render_meshes[mesh_index] = MakeSharedPtr<SkinnedMesh>(render_model_, wname);
			}
			else
			{
				render_meshes[mesh_index] = MakeSharedPtr<StaticMesh>(render_model_, wname);
			}
			auto& render_mesh = *render_meshes[mesh_index];

			render_mesh.MaterialID(meshes_[mesh_index].mtl_id);
			render_mesh.PosBound(pos_bbs[mesh_index]);
			render_mesh.TexcoordBound(tc_bbs[mesh_index]);

			render_mesh.NumLods(num_lods);
			for (uint32_t lod = 0; lod < num_lods; ++ lod, ++ mesh_lod_index)
			{
				for (uint32_t ve_index = 0; ve_index < merged_vertices.size(); ++ ve_index)
				{
					render_mesh.AddVertexStream(lod, merged_vbs[ve_index], merged_ves[ve_index]);
				}
				render_mesh.AddIndexStream(lod, merged_ib, is_index_16_bit ? EF_R16UI : EF_R32UI);

				render_mesh.NumVertices(lod, mesh_num_vertices[mesh_lod_index]);
				render_mesh.NumIndices(lod, mesh_num_indices[mesh_lod_index]);
				render_mesh.StartVertexLocation(lod, mesh_base_vertices[mesh_lod_index]);
				render_mesh.StartIndexLocation(lod, mesh_start_indices[mesh_lod_index]);
			}
		}

		if (skinned)
		{
			this->BuildActions(scenes[0].get());

			auto& skinned_model = *checked_pointer_cast<SkinnedModel>(render_model_);

			for (auto& joint : joints_)
			{
				std::tie(joint.inverse_origin_real, joint.inverse_origin_dual) = MathLib::inverse(joint.bind_real, joint.bind_dual);
				joint.inverse_origin_scale = 1 / joint.bind_scale;
			}
			skinned_model.AssignJoints(joints_.begin(), joints_.end());

			// TODO: Run skinning on CPU to get the bounding box
			for (size_t mesh_index = 0; mesh_index < meshes_.size(); ++ mesh_index)
			{
				auto& skinned_mesh = *checked_pointer_cast<SkinnedMesh>(render_meshes[mesh_index]);

				auto frame_pos_aabbs = MakeSharedPtr<AABBKeyFrameSet>();

				frame_pos_aabbs->frame_id.resize(2);
				frame_pos_aabbs->bb.resize(2);

				frame_pos_aabbs->frame_id[0] = 0;
				frame_pos_aabbs->frame_id[1] = skinned_model.NumFrames() - 1;

				frame_pos_aabbs->bb[0] = pos_bbs[mesh_index];
				frame_pos_aabbs->bb[1] = pos_bbs[mesh_index];

				skinned_mesh.AttachFramePosBounds(frame_pos_aabbs);
			}
		}

		render_model_->AssignSubrenderables(render_meshes.begin(), render_meshes.end());

		if (!in_path)
		{
			ResLoader::Instance().DelPath(in_folder);
		}

		return render_model_;
	}
}
