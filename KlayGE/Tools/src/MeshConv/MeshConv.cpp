#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KFL/CXX17/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/program_options.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

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

#include <MeshMLLib/MeshMLLib.hpp>

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
	
	struct JointBinding
	{
		int joint_id;
		int vertex_id;
		float weight;
	};

	struct Mesh
	{
		int mtl_id;
		std::string name;

		std::vector<float3> positions;
		std::vector<float3> normals;
		std::vector<Quaternion> tangent_quats;
		std::array<std::vector<float3>, AI_MAX_NUMBER_OF_TEXTURECOORDS> texcoords;
		std::vector<JointBinding> joint_binding;

		std::vector<uint32_t> indices;

		bool has_normal;
		bool has_tangent_frame;
		std::array<bool, AI_MAX_NUMBER_OF_TEXTURECOORDS> has_texcoord;
	};

	struct Joint
	{
		int id;
		int parent_id;
		std::string name;

		float4x4 bone_to_mesh;
		float4x4 local_matrix;	  // local to parent
	};

	typedef std::map<std::string, Joint> JointsMap;

	void RecursiveTransformMesh(MeshMLObj& meshml_obj, uint32_t num_lods, uint32_t lod,
		float4x4 const & parent_mat, aiNode const * node, std::vector<Mesh> const & meshes, std::vector<Mesh> const & lod0_meshes)
	{
		auto const trans_mat = MathLib::transpose(float4x4(&node->mTransformation.a1)) * parent_mat;
		auto const trans_quat = MathLib::to_quaternion(trans_mat);

		for (unsigned int n = 0; n < node->mNumMeshes; ++ n)
		{
			auto const & mesh = meshes[node->mMeshes[n]];

			int mesh_id;
			if (lod == 0)
			{
				mesh_id = meshml_obj.AllocMesh();
				meshml_obj.SetMesh(mesh_id, mesh.mtl_id, mesh.name, num_lods);
			}
			else
			{
				mesh_id = -1;
				for (size_t i = 0; i < lod0_meshes.size(); ++ i)
				{
					if (lod0_meshes[i].mtl_id == mesh.mtl_id)
					{
						mesh_id = static_cast<int>(i);
						break;
					}
				}
				BOOST_ASSERT(mesh_id != -1);
			}

			for (unsigned int ti = 0; ti < mesh.indices.size(); ti += 3)
			{
				int tri_id = meshml_obj.AllocTriangle(mesh_id, lod);
				meshml_obj.SetTriangle(mesh_id, lod, tri_id, mesh.indices[ti + 0],
					mesh.indices[ti + 1], mesh.indices[ti + 2]);
			}

			for (unsigned int vi = 0; vi < mesh.positions.size(); ++ vi)
			{
				int vertex_id = meshml_obj.AllocVertex(mesh_id, lod);

				std::vector<float3> texcoords;
				for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
				{
					if (mesh.has_texcoord[tci])
					{
						texcoords.push_back(mesh.texcoords[tci][vi]);
					}
				}

				auto const pos = MathLib::transform_coord(mesh.positions[vi], trans_mat);
				if (mesh.has_tangent_frame)
				{
					auto const quat = mesh.tangent_quats[vi] * trans_quat;
					meshml_obj.SetVertex(mesh_id, lod, vertex_id, pos, quat, 2, texcoords);
				}
				else
				{
					auto const normal = MathLib::transform_normal(mesh.normals[vi], trans_mat);
					meshml_obj.SetVertex(mesh_id, lod, vertex_id, pos, normal, 2, texcoords);
				}
			}

			for (unsigned int wi = 0; wi < mesh.joint_binding.size(); ++wi)
			{
				auto binding = mesh.joint_binding[wi];
				int bind_id = meshml_obj.AllocJointBinding(mesh_id, lod, binding.vertex_id);
				meshml_obj.SetJointBinding(mesh_id, lod, binding.vertex_id, bind_id, binding.joint_id, binding.weight);
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++ i)
		{
			RecursiveTransformMesh(meshml_obj, num_lods, lod, trans_mat, node->mChildren[i], meshes, lod0_meshes);
		}
	}

	void ConvertMaterials(MeshMLObj& meshml_obj, aiScene const * scene)
	{
		for (unsigned int mi = 0; mi < scene->mNumMaterials; ++ mi)
		{
			int mtl_id = meshml_obj.AllocMaterial();

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

			meshml_obj.SetMaterial(mtl_id, name, float4(albedo.x(), albedo.y(), albedo.z(), opacity),
				metalness, Shininess2Glossiness(shininess),
				emissive, transparent, 0, false, two_sided);

			unsigned int count = aiGetMaterialTextureCount(mtl, aiTextureType_DIFFUSE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE, 0, &str, 0, 0, 0, 0, 0, 0);

				meshml_obj.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Albedo, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_SHININESS);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_SHININESS, 0, &str, 0, 0, 0, 0, 0, 0);

				meshml_obj.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Glossiness, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_EMISSIVE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_EMISSIVE, 0, &str, 0, 0, 0, 0, 0, 0);

				meshml_obj.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Emissive, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_NORMALS);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_NORMALS, 0, &str, 0, 0, 0, 0, 0, 0);

				meshml_obj.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Normal, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_HEIGHT);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_HEIGHT, 0, &str, 0, 0, 0, 0, 0, 0);

				meshml_obj.SetTextureSlot(mtl_id, MeshMLObj::Material::TS_Height, str.C_Str());
			}
		}
	}

	void BuildMeshData(std::vector<Mesh>& meshes, int& vertex_export_settings, JointsMap const& joint_nodes, aiScene const * scene, bool swap_yz, bool inverse_z)
	{
		vertex_export_settings = MeshMLObj::VES_None;
		for (unsigned int mi = 0; mi < scene->mNumMeshes; ++ mi)
		{
			aiMesh const * mesh = scene->mMeshes[mi];

			meshes[mi].mtl_id = mesh->mMaterialIndex;
			meshes[mi].name = mesh->mName.C_Str();

			auto& indices = meshes[mi].indices;
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

			for (unsigned int bi = 0; bi < mesh->mNumBones; ++ bi)
			{
				aiBone* bone = mesh->mBones[bi];
				auto iter = joint_nodes.find(bone->mName.C_Str());
				BOOST_ASSERT_MSG(iter != joint_nodes.end(), "Joint not found!");

				int joint_id = iter->second.id;
				for (unsigned int wi = 0; wi < bone->mNumWeights; ++ wi)
				{
					int vertex_id = bone->mWeights[wi].mVertexId;
					float weight = bone->mWeights[wi].mWeight;
					meshes[mi].joint_binding.push_back({joint_id, vertex_id, weight});
				}
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
	}
	
	void BuildJoints(MeshMLObj& meshml_obj, JointsMap& joint_nodes, aiScene const * scene)
	{
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
					joint.bone_to_mesh = MathLib::inverse(MathLib::transpose(float4x4(&bone->mOffsetMatrix.a1))) * mesh_trans;
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
				joint.bone_to_mesh = float4x4::Identity();
				joint_nodes[name] = joint;
			}

			return child_has_bone;
		};

		std::function<void(aiNode const *, int)> alloc_joints =
			[&meshml_obj, &joint_nodes, &alloc_joints](aiNode const * root, int parent_id)
		{
			std::string name = root->mName.C_Str();
			int joint_id = -1;
			auto iter = joint_nodes.find(name);
			if (iter != joint_nodes.end())
			{
				joint_id = meshml_obj.AllocJoint();
				iter->second.id = joint_id;
				iter->second.local_matrix = MathLib::transpose(float4x4(&root->mTransformation.a1));

				meshml_obj.SetJoint(joint_id, name, parent_id, iter->second.bone_to_mesh);
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

	struct ResampledTransform
	{
		int frame;
		Quaternion bind_real;
		Quaternion bind_dual;
		float scale;
	};

	typedef std::vector<ResampledTransform> JointResampledKeyframe;

	struct JointKeyframe
	{
		std::vector<std::pair<float, float3>> pos;
		std::vector<std::pair<float, Quaternion>> quats;
		std::vector<std::pair<float, float3>> scale;
	};

	struct Animation
	{
		std::string name;
		int frame_num;
		std::map<int/*joint_id*/, JointResampledKeyframe> resampled_frames;
	};

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
			itime_lower = i -1;
			itime_upper = i;
		}

		float diff = vec[itime_upper].first - vec[itime_lower].first;
		return MathLib::clamp((diff == 0) ? 0 : (time - vec[itime_lower].first) / diff, 0.0f, 1.0f);
	}

	void ResampleJointTransform(int start_frame, int end_frame, float fps_scale, JointKeyframe const & okf, JointResampledKeyframe& rkf)
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

			if (!okf.scale.empty())
			{
				fraction = GetInterpTime(okf.scale, time, prev_i, i_scale);
				scale_resampled = MathLib::lerp(okf.scale[prev_i].second, okf.scale[i_scale].second, fraction);
			}
			if (!okf.quats.empty())
			{
				fraction = GetInterpTime(okf.quats, time, prev_i, i_rot);
				bind_real_resampled = MathLib::slerp(okf.quats[prev_i].second, okf.quats[i_rot].second, fraction);
			}
			if (!okf.pos.empty())
			{
				fraction = GetInterpTime(okf.pos, time, prev_i, i_pos);

				auto bind_dual_prev_i = MathLib::quat_trans_to_udq(okf.quats[prev_i].second, okf.pos[prev_i].second);
				auto bind_dual_i_pos = MathLib::quat_trans_to_udq(okf.quats[i_rot].second, okf.pos[i_pos].second);

				auto bind_dq_resampled = MathLib::sclerp(okf.quats[prev_i].second, bind_dual_prev_i,
					okf.quats[i_rot].second, bind_dual_i_pos,
					fraction);

				bind_dual_resampled = bind_dq_resampled.second;
			}

			if (MathLib::SignBit(bind_real_resampled.w()) < 0)
			{
				bind_real_resampled = -bind_real_resampled;
				bind_dual_resampled = -bind_dual_resampled;
			}

			rkf.push_back({i, bind_real_resampled, bind_dual_resampled, scale_resampled.x()});
		}
	}

	void BuildActions(MeshMLObj& meshml_obj, JointsMap const & joint_nodes, aiScene const * scene)
	{
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
				auto iter = joint_nodes.find(cur_joint->mNodeName.C_Str());
				if (iter != joint_nodes.end())
				{
					joint_id = iter->second.id;
				}

				// NOTE: ignore animation if node is not joint
				if (joint_id > 0)
				{
					JointKeyframe kf;
					for (unsigned int i = 0; i < cur_joint->mNumPositionKeys; ++ i)
					{
						auto const & p = cur_joint->mPositionKeys[i];
						kf.pos.push_back(std::make_pair(static_cast<float>(p.mTime), AiVectorToFloat3(p.mValue)));
					}

					for (unsigned int i = 0; i < cur_joint->mNumRotationKeys; ++ i)
					{
						auto const & p = cur_joint->mRotationKeys[i];
						kf.quats.push_back(std::make_pair(static_cast<float>(p.mTime), AiQuatToQuat(p.mValue)));
					}

					for (unsigned int i = 0; i < cur_joint->mNumScalingKeys; ++ i)
					{
						auto const & p = cur_joint->mScalingKeys[i];
						kf.scale.push_back(std::make_pair(static_cast<float>(p.mTime), AiVectorToFloat3(p.mValue)));
					}

					// resample
					JointResampledKeyframe resampled_kf;
					ResampleJointTransform(0, anim.frame_num, static_cast<float>(cur_anim->mTicksPerSecond / resample_fps),
						kf, resampled_kf);

					anim.resampled_frames[joint_id] = resampled_kf;
				}
			}

			for (auto const & joint : joint_nodes)
			{
				int joint_id = joint.second.id;
				if (anim.resampled_frames.find(joint_id) == anim.resampled_frames.end())
				{
					ResampledTransform default_tf;
					default_tf.frame = 0;
					Quaternion quat;
					float3 pos;
					float3 scale;
					MathLib::decompose(scale, quat, pos, joint.second.local_matrix);
					default_tf.bind_real = quat;
					default_tf.bind_dual = MathLib::quat_trans_to_udq(quat, pos);
					if (MathLib::SignBit(default_tf.bind_real.w()) < 0)
					{
						default_tf.bind_real = -default_tf.bind_real;
						default_tf.bind_dual = -default_tf.bind_dual;
					}
					default_tf.scale = scale.x();
					anim.resampled_frames[joint_id].push_back(default_tf);
				}
			}

			animations.push_back(anim);
		}
		
		int action_frame_offset = 0;
		for (auto const & anim : animations)
		{
			int action_id = meshml_obj.AllocAction();
			meshml_obj.SetAction(action_id, anim.name, action_frame_offset, action_frame_offset + anim.frame_num);

			for (auto const & frame : anim.resampled_frames)
			{
				int joint_id = frame.first;
				int kfs_id = meshml_obj.AllocKeyframes();
				meshml_obj.SetKeyframes(kfs_id, joint_id);

				for (auto const & resampled : frame.second)
				{
					int shifted_frame = resampled.frame + action_frame_offset;
					int kf_id = meshml_obj.AllocKeyframe(kfs_id);

					meshml_obj.SetKeyframe(kfs_id, kf_id, shifted_frame, resampled.bind_real * resampled.scale, resampled.bind_dual);
				}
			}

			action_frame_offset = action_frame_offset + anim.frame_num;
		}

		meshml_obj.FrameRate(resample_fps);
		meshml_obj.NumFrames(action_frame_offset);
	}

	bool ConvertScene(std::vector<std::string> const & in_names, std::string const & out_name, float scale, bool swap_yz, bool inverse_z,
		bool quiet)
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

		uint32_t const num_lods = static_cast<uint32_t>(in_names.size());

		std::vector<aiScene const *> scenes(num_lods);
		for (uint32_t lod = 0; lod < num_lods; ++ lod)
		{
			scenes[lod] = aiImportFileExWithProperties(in_names[lod].c_str(),
				ppsteps // configurable pp steps
				| aiProcess_GenSmoothNormals // generate smooth normal vectors if not existing
				| aiProcess_Triangulate // triangulate polygons with more than 3 edges
				| aiProcess_ConvertToLeftHanded // convert everything to D3D left handed space
				| aiProcess_FixInfacingNormals, // find normals facing inwards and inverts them
				nullptr, props);

			if (!scenes[lod])
			{
				cout << "Assimp: Import file error: " << aiGetErrorString() << '\n';
				return false;
			}
		}

		aiReleasePropertyStore(props);

		MeshMLObj meshml_obj(scale);
		ConvertMaterials(meshml_obj, scenes[0]);

		std::vector<std::vector<Mesh>> meshes(num_lods);
		for (uint32_t lod = 0; lod < num_lods; ++ lod)
		{
			meshes[lod].resize(scenes[lod]->mNumMeshes);
		}

		JointsMap joint_nodes;

		BuildJoints(meshml_obj, joint_nodes, scenes[0]);

		int vertex_export_settings = 0;
		for (uint32_t lod = 0; lod < num_lods; ++ lod)
		{
			BuildMeshData(meshes[lod], vertex_export_settings, joint_nodes, scenes[lod], swap_yz, inverse_z);
			RecursiveTransformMesh(meshml_obj, num_lods, lod, float4x4::Identity(), scenes[lod]->mRootNode, meshes[lod], meshes[0]);
		}
		BuildActions(meshml_obj, joint_nodes, scenes[0]);

		std::ofstream ofs(out_name.c_str());
		meshml_obj.WriteMeshML(ofs, vertex_export_settings, 0);

		for (uint32_t lod = 0; lod < num_lods; ++ lod)
		{
			aiReleaseImport(scenes[lod]);
		}

		if (!quiet)
		{
			for (uint32_t lod = 0; lod < num_lods; ++ lod)
			{
				size_t num_vertices = 0;
				size_t num_triangles = 0;
				for (auto const & mesh : meshes[lod])
				{
					num_vertices += mesh.positions.size();
					num_triangles += mesh.indices.size() / 3;
				}

				cout << "LOD " << lod << ": " << num_vertices << " vertices, " << num_triangles << " triangles." << endl;
			}
		}

		return true;
	}
}

int main(int argc, char* argv[])
{
	std::string input_name;
	filesystem::path target_folder;
	float scale = 1;
	bool swap_yz = false;
	bool inverse_z = false;
	bool lod_inputs = false;
	bool quiet = false;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-path,I", boost::program_options::value<std::string>(), "Input mesh path.")
		("target-folder,T", boost::program_options::value<std::string>(), "Target folder.")
		("scale,S", boost::program_options::value<float>(), "Scale.")
		("swap-yz,W", "Swap Y and Z axis.")
		("inverse-z,Z", "Inverse Z axis.")
		("lod,L", "Consume lod meshes, e.g. <INPUT>_lod0.xxx as lod 0, <INPUT>_lod1.xxx as lod 1, etc.")
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
	if (vm.count("lod") > 0)
	{
		lod_inputs = true;
	}
	if (vm.count("quiet") > 0)
	{
		quiet = vm["quiet"].as<bool>();
	}

	filesystem::path const input_path(input_name);
	filesystem::path const base_name = input_path.stem();

	std::vector<std::string> lod_file_names;
	if (lod_inputs)
	{
		filesystem::path const source_folder = input_path.parent_path();
		filesystem::path const ext_name = input_path.extension();

		for (uint32_t num_lods = 0;; ++ num_lods)
		{
			std::string const lod_file_name = base_name.string() + "_lod" + boost::lexical_cast<std::string>(num_lods) + ext_name.string();
			std::string const file_name = ResLoader::Instance().Locate((source_folder / lod_file_name).string());
			if (file_name.empty())
			{
				break;
			}
			else
			{
				lod_file_names.push_back(file_name);
			}
		}
	}
	else
	{
		std::string file_name = ResLoader::Instance().Locate(input_name);
		if (!file_name.empty())
		{
			lod_file_names.push_back(file_name);
		}
	}

	if (lod_file_names.empty())
	{
		cout << "Could NOT find " << input_name << endl;
		Context::Destroy();
		return 1;
	}

	if (target_folder.empty())
	{
		target_folder = filesystem::path(ResLoader::Instance().Locate(lod_file_names[0])).parent_path();
	}

	std::string const output_name = (target_folder / base_name).string() + ".meshml";

	bool succ = ConvertScene(lod_file_names, output_name, scale, swap_yz, inverse_z, quiet);

	if (succ && !quiet)
	{
		cout << "MeshML has been saved to " << output_name << "." << endl;
	}

	Context::Destroy();

	return 0;
}
