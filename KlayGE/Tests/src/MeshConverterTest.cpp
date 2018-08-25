/**
 * @file MeshConverterTest.cpp
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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KFL/CXX17/filesystem.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/MeshConverter.hpp>
#include <KlayGE/MeshMetadata.hpp>
#include <MeshMLLib/MeshMLLib.hpp>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

class MeshConverterTest : public testing::Test
{
public:
	void SetUp() override
	{
		ResLoader::Instance().AddPath("../../Tests/media/MeshConverter");
	}

	void RunTest(std::string_view input_name, std::string_view metadata_name, std::string_view sanity_name)
	{
		MeshMetadata metadata(metadata_name);

		MeshMLObj target(1.0f);
		int vertex_export_settings;

		MeshConverter mc;
		EXPECT_TRUE(mc.Convert(input_name, metadata, target, vertex_export_settings));

		std::vector<RenderMaterialPtr> mtls;
		std::vector<VertexElement> merged_ves;
		char all_is_index_16_bit;
		std::vector<std::vector<uint8_t>> merged_buff;
		std::vector<uint8_t> merged_indices;
		std::vector<std::string> mesh_names;
		std::vector<int32_t> mtl_ids;
		std::vector<uint32_t> mesh_lods;
		std::vector<AABBox> pos_bbs;
		std::vector<AABBox> tc_bbs;
		std::vector<uint32_t> mesh_num_vertices;
		std::vector<uint32_t> mesh_base_vertices;
		std::vector<uint32_t> mesh_num_indices;
		std::vector<uint32_t> mesh_base_indices;
		std::vector<Joint> joints;
		std::shared_ptr<AnimationActionsType> actions;
		std::shared_ptr<KeyFramesType> kfs;
		uint32_t num_frames;
		uint32_t frame_rate;
		std::vector<std::shared_ptr<AABBKeyFrames>> frame_pos_bbs;

		LoadModel(sanity_name, mtls,
			merged_ves, all_is_index_16_bit,
			merged_buff, merged_indices,
			mesh_names, mtl_ids, mesh_lods,
			pos_bbs, tc_bbs,
			mesh_num_vertices, mesh_base_vertices,
			mesh_num_indices, mesh_base_indices,
			joints, actions,
			kfs, num_frames, frame_rate,
			frame_pos_bbs);

		EXPECT_EQ(target.NumMaterials(), mtls.size());
		for (uint32_t i = 0; i < target.NumMaterials(); ++ i)
		{
			std::string name;
			float4 albedo;
			float metalness;
			float glossiness;
			float3 emissive;
			bool transparent;
			float alpha_test;
			bool sss;
			bool two_sided;
			target.GetMaterial(i, name, albedo, metalness, glossiness, emissive, transparent, alpha_test, sss, two_sided);

			EXPECT_EQ(name, mtls[i]->name);
			EXPECT_FLOAT_EQ(albedo.x(), mtls[i]->albedo.x());
			EXPECT_FLOAT_EQ(albedo.y(), mtls[i]->albedo.y());
			EXPECT_FLOAT_EQ(albedo.z(), mtls[i]->albedo.z());
			EXPECT_FLOAT_EQ(albedo.w(), mtls[i]->albedo.w());
			EXPECT_FLOAT_EQ(metalness, mtls[i]->metalness);
			EXPECT_FLOAT_EQ(glossiness, mtls[i]->glossiness);
			EXPECT_FLOAT_EQ(emissive.x(), mtls[i]->emissive.x());
			EXPECT_FLOAT_EQ(emissive.y(), mtls[i]->emissive.y());
			EXPECT_FLOAT_EQ(emissive.z(), mtls[i]->emissive.z());
			EXPECT_EQ(transparent, mtls[i]->transparent);
			EXPECT_FLOAT_EQ(alpha_test, mtls[i]->alpha_test);
			EXPECT_EQ(sss, mtls[i]->sss);
			EXPECT_EQ(two_sided, mtls[i]->two_sided);

			MeshMLObj::Material::SurfaceDetailMode detail_mode;
			float height_offset;
			float height_scale;
			float edge_tess_hint;
			float inside_tess_hint;
			float min_tess;
			float max_tess;
			target.GetDetailMaterial(i, detail_mode, height_offset, height_scale, edge_tess_hint, inside_tess_hint, min_tess, max_tess);

			/*EXPECT_EQ(detail_mode, mtls[i]->detail_mode);
			EXPECT_FLOAT_EQ(height_offset, mtls[i]->height_offset_scale.x());
			EXPECT_FLOAT_EQ(height_scale, mtls[i]->height_offset_scale.y());
			if (detail_mode != MeshMLObj::Material::SDM_Parallax)
			{
				EXPECT_FLOAT_EQ(edge_tess_hint, mtls[i]->tess_factors.x());
				EXPECT_FLOAT_EQ(inside_tess_hint, mtls[i]->tess_factors.y());
				EXPECT_FLOAT_EQ(min_tess, mtls[i]->tess_factors.z());
				EXPECT_FLOAT_EQ(max_tess, mtls[i]->tess_factors.w());
			}*/

			for (uint32_t slot = RenderMaterial::TS_Albedo; slot != RenderMaterial::TS_Height; ++ slot)
			{
				std::string tex_name;
				target.GetTextureSlot(i, static_cast<MeshMLObj::Material::TextureSlot>(slot), tex_name);
				EXPECT_EQ(tex_name, mtls[i]->tex_names[slot]);
			}
		}

		int position_stream = -1;
		int normal_stream = -1;
		int tangent_quat_stream = -1;
		int texcoord_stream = -1;
		int stream_index = 0;
		for (auto const & ve : merged_ves)
		{
			if (ve.usage == VEU_Position)
			{
				position_stream = stream_index;
			}
			if ((ve.usage == VEU_Normal) && (ve.usage_index == 0) && (ve.format == EF_ABGR8))
			{
				normal_stream = stream_index;
			}
			if ((ve.usage == VEU_Tangent) && (ve.usage_index == 0) && (ve.format == EF_ABGR8))
			{
				tangent_quat_stream = stream_index;
			}
			if ((ve.usage == VEU_TextureCoord) && (ve.usage_index == 0) && (ve.format == EF_SIGNED_GR16))
			{
				texcoord_stream = stream_index;
			}

			++ stream_index;
		}

		EXPECT_NE(position_stream, -1);
		if (vertex_export_settings & MeshMLObj::VES_Normal)
		{
			EXPECT_NE(normal_stream, -1);
		}
		if (vertex_export_settings & MeshMLObj::VES_TangentQuat)
		{
			EXPECT_NE(tangent_quat_stream, -1);
		}
		if (vertex_export_settings & MeshMLObj::VES_Texcoord)
		{
			EXPECT_NE(texcoord_stream, -1);
		}

		EXPECT_EQ(target.NumMeshes(), mesh_names.size());
		uint32_t mesh_lod_index = 0;
		for (uint32_t i = 0; i < target.NumMeshes(); ++ i)
		{
			int material_id;
			std::string name;
			int num_lods;
			target.GetMesh(i, material_id, name, num_lods);

			EXPECT_EQ(material_id, mtl_ids[i]);
			EXPECT_EQ(name, mesh_names[i]);
			EXPECT_EQ(num_lods, static_cast<int>(mesh_lods[i]));

			for (int lod = 0; lod < num_lods; ++ lod, ++ mesh_lod_index)
			{
				EXPECT_EQ(target.NumVertices(i, lod), mesh_num_vertices[mesh_lod_index]);
				EXPECT_EQ(target.NumTriangles(i, lod) * 3, mesh_num_indices[mesh_lod_index]);

				auto const * position_buff = reinterpret_cast<int16_t const *>(merged_buff[position_stream].data());
				auto const pos_center = pos_bbs[i].Center();
				auto const pos_extent = pos_bbs[i].HalfSize();
				auto const * normal_buff = (normal_stream != -1) ? merged_buff[normal_stream].data() : nullptr;
				auto const * tangent_buff = (tangent_quat_stream != -1) ? merged_buff[tangent_quat_stream].data() : nullptr;
				auto const * texcoord_buff = (texcoord_stream != -1)
					? reinterpret_cast<int16_t const *>(merged_buff[texcoord_stream].data()) : nullptr;
				auto const tc_center = tc_bbs[i].Center();
				auto const tc_extent = tc_bbs[i].HalfSize();
				for (uint32_t vid = 0; vid < mesh_num_vertices[mesh_lod_index]; ++ vid)
				{
					float3 pos;
					float3 normal;
					Quaternion tangent_quat;
					int texcoord_components;
					std::vector<float3> texcoords;
					target.GetVertex(i, lod, vid, pos, normal, tangent_quat, texcoord_components, texcoords);

					EXPECT_EQ(texcoord_components, 2);

					uint32_t const index = vid + mesh_base_vertices[mesh_lod_index];

					{
						float3 sanity_pos;
						sanity_pos.x() = ((position_buff[index * 4 + 0] + 32768) / 65535.0f * 2 - 1) * pos_extent.x() + pos_center.x();
						sanity_pos.y() = ((position_buff[index * 4 + 1] + 32768) / 65535.0f * 2 - 1) * pos_extent.y() + pos_center.y();
						sanity_pos.z() = ((position_buff[index * 4 + 2] + 32768) / 65535.0f * 2 - 1) * pos_extent.z() + pos_center.z();

						EXPECT_TRUE(std::abs(pos.x() - sanity_pos.x()) < 1e-3f);
						EXPECT_TRUE(std::abs(pos.y() - sanity_pos.y()) < 1e-3f);
						EXPECT_TRUE(std::abs(pos.z() - sanity_pos.z()) < 1e-3f);
					}
					if (vertex_export_settings & MeshMLObj::VES_Normal)
					{
						float4 sanity_normal;
						sanity_normal.x() = (normal_buff[index * 4 + 0] / 255.0f) * 2 - 1;
						sanity_normal.y() = (normal_buff[index * 4 + 1] / 255.0f) * 2 - 1;
						sanity_normal.z() = (normal_buff[index * 4 + 2] / 255.0f) * 2 - 1;
						normal = MathLib::normalize(normal);
						sanity_normal = MathLib::normalize(sanity_normal);

						EXPECT_TRUE(std::abs(normal.x() - sanity_normal.x()) < 2e-2f);
						EXPECT_TRUE(std::abs(normal.y() - sanity_normal.y()) < 2e-2f);
						EXPECT_TRUE(std::abs(normal.z() - sanity_normal.z()) < 2e-2f);
					}
					if (vertex_export_settings & MeshMLObj::VES_TangentQuat)
					{
						float4 sanity_tangent;
						sanity_tangent.x() = (tangent_buff[index * 4 + 0] / 255.0f) * 2 - 1;
						sanity_tangent.y() = (tangent_buff[index * 4 + 1] / 255.0f) * 2 - 1;
						sanity_tangent.z() = (tangent_buff[index * 4 + 2] / 255.0f) * 2 - 1;
						sanity_tangent.w() = (tangent_buff[index * 4 + 3] / 255.0f) * 2 - 1;
						tangent_quat = MathLib::normalize(tangent_quat);
						sanity_tangent = MathLib::normalize(sanity_tangent);

						EXPECT_TRUE(std::abs(tangent_quat.x() - sanity_tangent.x()) < 2e-2f);
						EXPECT_TRUE(std::abs(tangent_quat.y() - sanity_tangent.y()) < 2e-2f);
						EXPECT_TRUE(std::abs(tangent_quat.z() - sanity_tangent.z()) < 2e-2f);
						EXPECT_TRUE(std::abs(tangent_quat.w() - sanity_tangent.w()) < 2e-2f);
					}
					if (vertex_export_settings & MeshMLObj::VES_Texcoord)
					{
						float3 sanity_tc;
						sanity_tc.x() = ((texcoord_buff[index * 2 + 0] + 32768) / 65535.0f * 2 - 1) * tc_extent.x() + tc_center.x();
						sanity_tc.y() = ((texcoord_buff[index * 2 + 1] + 32768) / 65535.0f * 2 - 1) * tc_extent.y() + tc_center.y();

						EXPECT_TRUE(std::abs(texcoords[0].x() - sanity_tc.x()) < 1e-3f);
						EXPECT_TRUE(std::abs(texcoords[0].y() - sanity_tc.y()) < 1e-3f);
					}
				}

				auto const * indices_buff_16 = reinterpret_cast<uint16_t const *>(merged_indices.data());
				auto const * indices_buff_32 = reinterpret_cast<uint32_t const *>(merged_indices.data());
				for (uint32_t tid = 0; tid < mesh_num_indices[mesh_lod_index] / 3; ++ tid)
				{
					uint32_t const index = tid + mesh_base_indices[mesh_lod_index] / 3;
					uint32_t sanity_tri_index[3];
					if (all_is_index_16_bit)
					{
						sanity_tri_index[0] = indices_buff_16[index * 3 + 0];
						sanity_tri_index[1] = indices_buff_16[index * 3 + 1];
						sanity_tri_index[2] = indices_buff_16[index * 3 + 2];
					}
					else
					{
						sanity_tri_index[0] = indices_buff_32[index * 3 + 0];
						sanity_tri_index[1] = indices_buff_32[index * 3 + 1];
						sanity_tri_index[2] = indices_buff_32[index * 3 + 2];
					}

					int tri_index[3];
					target.GetTriangle(i, lod, tid, tri_index[0], tri_index[1], tri_index[2]);

					EXPECT_EQ(tri_index[0], static_cast<int>(sanity_tri_index[0]));
					EXPECT_EQ(tri_index[1], static_cast<int>(sanity_tri_index[1]));
					EXPECT_EQ(tri_index[2], static_cast<int>(sanity_tri_index[2]));
				}
			}
		}

		EXPECT_EQ(target.NumJoints(), joints.size());
		for (uint32_t i = 0; i < target.NumJoints(); ++ i)
		{
			std::string joint_name;
			int parent_id;
			Quaternion bind_real;
			Quaternion bind_dual;
			target.GetJoint(i, joint_name, parent_id, bind_real, bind_dual);

			EXPECT_EQ(joint_name, joints[i].name);
			EXPECT_EQ(parent_id, joints[i].parent);

			EXPECT_TRUE(std::abs(bind_real.x() - joints[i].bind_real.x()) < 1e-4f);
			EXPECT_TRUE(std::abs(bind_real.y() - joints[i].bind_real.y()) < 1e-4f);
			EXPECT_TRUE(std::abs(bind_real.z() - joints[i].bind_real.z()) < 1e-4f);
			EXPECT_TRUE(std::abs(bind_real.w() - joints[i].bind_real.w()) < 1e-4f);

			EXPECT_TRUE(std::abs(bind_dual.x() - joints[i].bind_dual.x()) < 1e-4f);
			EXPECT_TRUE(std::abs(bind_dual.y() - joints[i].bind_dual.y()) < 1e-4f);
			EXPECT_TRUE(std::abs(bind_dual.z() - joints[i].bind_dual.z()) < 1e-4f);
			EXPECT_TRUE(std::abs(bind_dual.w() - joints[i].bind_dual.w()) < 1e-4f);
		}

		if (actions)
		{
			EXPECT_EQ(target.NumActions(), actions->size());
			for (uint32_t i = 0; i < target.NumActions(); ++ i)
			{
				std::string name;
				int start_frame;
				int end_frame;
				target.GetAction(i, name, start_frame, end_frame);

				EXPECT_EQ(name, (*actions)[i].name);
				EXPECT_EQ(start_frame, static_cast<int>((*actions)[i].start_frame));
				EXPECT_EQ(end_frame, static_cast<int>((*actions)[i].end_frame));
			}
		}
		else
		{
			EXPECT_EQ(target.NumActions(), 0U);
		}

		if (kfs)
		{
			EXPECT_EQ(target.NumKeyframeSets(), kfs->size());
			for (uint32_t i = 0; i < target.NumKeyframeSets(); ++ i)
			{
				int joint_id;
				target.GetKeyframeSet(i, joint_id);
				EXPECT_EQ(static_cast<int>(i), joint_id);

				for (uint32_t j = 0; j < target.NumKeyframes(i); ++ j)
				{
					int frame_id;
					Quaternion bind_real;
					Quaternion bind_dual;
					float bind_scale;
					target.GetKeyframe(i, j, frame_id, bind_real, bind_dual, bind_scale);

					Quaternion sanity_bind_real;
					Quaternion sanity_bind_dual;
					float sanity_scale;
					std::tie(sanity_bind_real, sanity_bind_dual, sanity_scale) = (*kfs)[i].Frame(static_cast<float>(frame_id));

					EXPECT_TRUE(std::abs(bind_real.x() - sanity_bind_real.x()) < 1e-4f);
					EXPECT_TRUE(std::abs(bind_real.y() - sanity_bind_real.y()) < 1e-4f);
					EXPECT_TRUE(std::abs(bind_real.z() - sanity_bind_real.z()) < 1e-4f);
					EXPECT_TRUE(std::abs(bind_real.w() - sanity_bind_real.w()) < 1e-4f);

					EXPECT_TRUE(std::abs(bind_dual.x() - sanity_bind_dual.x()) < 1e-4f);
					EXPECT_TRUE(std::abs(bind_dual.y() - sanity_bind_dual.y()) < 1e-4f);
					EXPECT_TRUE(std::abs(bind_dual.z() - sanity_bind_dual.z()) < 1e-4f);
					EXPECT_TRUE(std::abs(bind_dual.w() - sanity_bind_dual.w()) < 1e-4f);

					EXPECT_TRUE(std::abs(bind_scale - sanity_scale) < 1e-4f);
				}
			}

			EXPECT_EQ(target.NumFrames(), static_cast<int>(num_frames));
			EXPECT_EQ(target.FrameRate(), static_cast<int>(frame_rate));
		}
		else
		{
			EXPECT_EQ(target.NumKeyframeSets(), 0U);
		}
	}
};

TEST_F(MeshConverterTest, StaticNoLod)
{
	RunTest("tree2a_lod0.obj", "tree2a.nolod.kmeta", "tree2a.nolod.meshml");
}

TEST_F(MeshConverterTest, StaticLod)
{
	RunTest("tree2a_lod0.obj", "tree2a.lod.kmeta", "tree2a.lod.meshml");
}

TEST_F(MeshConverterTest, StaticLodAutoCenter)
{
	RunTest("tree2a_lod0.obj", "tree2a.lod_autocenter.kmeta", "tree2a.lod_autocenter.meshml");
}

TEST_F(MeshConverterTest, StaticLodAxisMapping)
{
	RunTest("tree2a_lod0.obj", "tree2a.lod_axismapping.kmeta", "tree2a.lod_axismapping.meshml");
}

TEST_F(MeshConverterTest, StaticLodTransforms)
{
	RunTest("tree2a_lod0.obj", "tree2a.lod_trans.kmeta", "tree2a.lod_trans.meshml");
}

TEST_F(MeshConverterTest, AnimationPassThrough)
{
	RunTest("anim.fbx", "", "anim.meshml");
}
