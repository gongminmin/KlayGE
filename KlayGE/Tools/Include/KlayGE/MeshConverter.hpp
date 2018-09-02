/**
 * @file MeshConverter.hpp
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

#ifndef KLAYGE_TOOLS_TOOL_COMMON_MESH_CONVERTER_HPP
#define KLAYGE_TOOLS_TOOL_COMMON_MESH_CONVERTER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX17/string_view.hpp>
#include <KlayGE/Mesh.hpp>

#include <map>
#include <vector>

#include <KlayGE/ToolCommon.hpp>
#include <KlayGE/MeshMetadata.hpp>

struct aiNode;
struct aiScene;

namespace KlayGE
{
	class KLAYGE_TOOL_API MeshConverter
	{
	private:
		static uint32_t constexpr MAX_NUMBER_OF_TEXTURECOORDS = 8;

		struct Mesh
		{
			int mtl_id;
			std::string name;

			struct Lod
			{
				std::vector<float3> positions;
				std::vector<float3> normals;
				std::vector<Quaternion> tangent_quats;
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

	public:
		RenderModelPtr Convert(std::string_view input_name, MeshMetadata const & metadata);

	private:
		void CompressKeyFrameSet(KeyFrameSet& kf);

		// From assimp
		void RecursiveTransformMesh(uint32_t lod, float4x4 const & parent_mat, aiNode const * node);
		void BuildMaterials(aiScene const * scene);
		void BuildMeshData(std::vector<std::shared_ptr<aiScene const>> const & scene_lods, MeshMetadata const & input_metadata);
		void BuildJoints(aiScene const * scene);
		void BuildActions(aiScene const * scene);
		void ResampleJointTransform(KeyFrameSet& rkf, int start_frame, int end_frame, float fps_scale,
			std::vector<std::pair<float, float3>> const & poss, std::vector<std::pair<float, Quaternion>> const & quats,
			std::vector<std::pair<float, float3>> const & scale);
		void LoadFromAssimp(std::string const & input_name, MeshMetadata const & metadata);

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
		void CompileActionsChunk(XMLNodePtr const & actions_chunk);
		void LoadFromMeshML(std::string const & input_name, MeshMetadata const & metadata);

	private:
		RenderModelPtr render_model_;

		std::vector<Mesh> meshes_;
		std::vector<Joint> joints_;
		bool has_normal_;
		bool has_tangent_quat_;
		bool has_texcoord_;
		bool has_diffuse_;
		bool has_specular_;
	};
}

#endif		// KLAYGE_TOOLS_TOOL_COMMON_MESH_CONVERTER_HPP
