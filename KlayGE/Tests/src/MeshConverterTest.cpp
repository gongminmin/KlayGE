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

		auto sanity_model = LoadSoftwareModel(sanity_name);

		EXPECT_EQ(target.NumMaterials(), sanity_model->NumMaterials());
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

			auto sanity_mtl = sanity_model->GetMaterial(i);

			EXPECT_EQ(name, sanity_mtl->name);
			EXPECT_FLOAT_EQ(albedo.x(), sanity_mtl->albedo.x());
			EXPECT_FLOAT_EQ(albedo.y(), sanity_mtl->albedo.y());
			EXPECT_FLOAT_EQ(albedo.z(), sanity_mtl->albedo.z());
			EXPECT_FLOAT_EQ(albedo.w(), sanity_mtl->albedo.w());
			EXPECT_FLOAT_EQ(metalness, sanity_mtl->metalness);
			EXPECT_FLOAT_EQ(glossiness, sanity_mtl->glossiness);
			EXPECT_FLOAT_EQ(emissive.x(), sanity_mtl->emissive.x());
			EXPECT_FLOAT_EQ(emissive.y(), sanity_mtl->emissive.y());
			EXPECT_FLOAT_EQ(emissive.z(), sanity_mtl->emissive.z());
			EXPECT_EQ(transparent, sanity_mtl->transparent);
			EXPECT_FLOAT_EQ(alpha_test, sanity_mtl->alpha_test);
			EXPECT_EQ(sss, sanity_mtl->sss);
			EXPECT_EQ(two_sided, sanity_mtl->two_sided);

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
				EXPECT_EQ(tex_name, sanity_mtl->tex_names[slot]);
			}
		}

		auto const & rl = checked_cast<StaticMesh*>(sanity_model->Subrenderable(0).get())->GetRenderLayout();

		int position_stream = -1;
		int normal_stream = -1;
		int tangent_quat_stream = -1;
		int texcoord_stream = -1;
		int stream_index = 0;
		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			auto ve = rl.VertexStreamFormat(i)[0];

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

		EXPECT_EQ(target.NumMeshes(), sanity_model->NumSubrenderables());
		for (uint32_t i = 0; i < target.NumMeshes(); ++ i)
		{
			auto const & sanity_mesh = *checked_cast<StaticMesh*>(sanity_model->Subrenderable(i).get());

			int material_id;
			std::string name;
			int num_lods;
			target.GetMesh(i, material_id, name, num_lods);

			std::wstring wname;
			Convert(wname, name);

			EXPECT_EQ(material_id, sanity_mesh.MaterialID());
			EXPECT_EQ(wname, sanity_mesh.Name());
			EXPECT_EQ(num_lods, static_cast<int>(sanity_mesh.NumLods()));

			for (int lod = 0; lod < num_lods; ++ lod)
			{
				EXPECT_EQ(target.NumVertices(i, lod), sanity_mesh.NumVertices(lod));
				EXPECT_EQ(target.NumTriangles(i, lod) * 3, sanity_mesh.NumIndices(lod));

				GraphicsBuffer::Mapper position_mapper(*rl.GetVertexStream(position_stream), BA_Read_Only);
				auto const * position_buff = position_mapper.Pointer<int16_t>();
				auto const pos_center = sanity_mesh.PosBound().Center();
				auto const pos_extent = sanity_mesh.PosBound().HalfSize();
				uint8_t const * normal_buff = nullptr;
				if (normal_stream != -1)
				{
					GraphicsBuffer::Mapper normal_mapper(*rl.GetVertexStream(normal_stream), BA_Read_Only);
					normal_buff = normal_mapper.Pointer<uint8_t>();
				}
				uint8_t const * tangent_buff = nullptr;
				if (tangent_quat_stream != -1)
				{
					GraphicsBuffer::Mapper tangent_quat_mapper(*rl.GetVertexStream(tangent_quat_stream), BA_Read_Only);
					tangent_buff = tangent_quat_mapper.Pointer<uint8_t>();
				}
				int16_t const * texcoord_buff = nullptr;
				if (texcoord_stream != -1)
				{
					GraphicsBuffer::Mapper texcoord_mapper(*rl.GetVertexStream(texcoord_stream), BA_Read_Only);
					texcoord_buff = texcoord_mapper.Pointer<int16_t>();
				}
				auto const tc_center = sanity_mesh.TexcoordBound().Center();
				auto const tc_extent = sanity_mesh.TexcoordBound().HalfSize();
				for (uint32_t vid = 0; vid < sanity_mesh.NumVertices(lod); ++ vid)
				{
					float3 pos;
					float3 normal;
					Quaternion tangent_quat;
					int texcoord_components;
					std::vector<float3> texcoords;
					target.GetVertex(i, lod, vid, pos, normal, tangent_quat, texcoord_components, texcoords);

					EXPECT_EQ(texcoord_components, 2);

					uint32_t const index = vid + sanity_mesh.StartVertexLocation(lod);

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

				GraphicsBuffer::Mapper indices_mapper(*rl.GetIndexStream(), BA_Read_Only);
				auto const * indices_buff_16 = indices_mapper.Pointer<uint16_t>();
				auto const * indices_buff_32 = indices_mapper.Pointer<uint32_t>();
				for (uint32_t tid = 0; tid < sanity_mesh.NumIndices(lod) / 3; ++ tid)
				{
					uint32_t const index = tid + sanity_mesh.StartIndexLocation(lod) / 3;
					uint32_t sanity_tri_index[3];
					if (sanity_mesh.GetRenderLayout().IndexStreamFormat() == EF_R16UI)
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

		if (sanity_model->IsSkinned())
		{
			auto& sanity_skinned_model = *checked_cast<SkinnedModel*>(sanity_model.get());

			EXPECT_EQ(target.NumJoints(), sanity_skinned_model.NumJoints());
			for (uint32_t i = 0; i < target.NumJoints(); ++ i)
			{
				std::string joint_name;
				int parent_id;
				Quaternion bind_real;
				Quaternion bind_dual;
				target.GetJoint(i, joint_name, parent_id, bind_real, bind_dual);

				auto& sanity_joint = sanity_skinned_model.GetJoint(i);

				EXPECT_EQ(joint_name, sanity_joint.name);
				EXPECT_EQ(parent_id, sanity_joint.parent);

				EXPECT_TRUE(std::abs(bind_real.x() - sanity_joint.bind_real.x()) < 1e-4f);
				EXPECT_TRUE(std::abs(bind_real.y() - sanity_joint.bind_real.y()) < 1e-4f);
				EXPECT_TRUE(std::abs(bind_real.z() - sanity_joint.bind_real.z()) < 1e-4f);
				EXPECT_TRUE(std::abs(bind_real.w() - sanity_joint.bind_real.w()) < 1e-4f);

				EXPECT_TRUE(std::abs(bind_dual.x() - sanity_joint.bind_dual.x()) < 1e-4f);
				EXPECT_TRUE(std::abs(bind_dual.y() - sanity_joint.bind_dual.y()) < 1e-4f);
				EXPECT_TRUE(std::abs(bind_dual.z() - sanity_joint.bind_dual.z()) < 1e-4f);
				EXPECT_TRUE(std::abs(bind_dual.w() - sanity_joint.bind_dual.w()) < 1e-4f);
			}

			EXPECT_EQ(target.NumActions(), sanity_skinned_model.NumActions());
			for (uint32_t i = 0; i < target.NumActions(); ++ i)
			{
				std::string name;
				int start_frame;
				int end_frame;
				target.GetAction(i, name, start_frame, end_frame);

				std::string sanity_action_name;
				uint32_t sanity_start_frame;
				uint32_t sanity_end_frame;
				sanity_skinned_model.GetAction(i, sanity_action_name, sanity_start_frame, sanity_end_frame);

				EXPECT_EQ(name, sanity_action_name);
				EXPECT_EQ(start_frame, static_cast<int>(sanity_start_frame));
				EXPECT_EQ(end_frame, static_cast<int>(sanity_end_frame));
			}

			EXPECT_EQ(target.NumKeyframeSets(), sanity_skinned_model.GetKeyFrames()->size());
			for (uint32_t i = 0; i < target.NumKeyframeSets(); ++ i)
			{
				int joint_id;
				target.GetKeyframeSet(i, joint_id);
				EXPECT_EQ(static_cast<int>(i), joint_id);

				auto const & sanity_key_frames = (*(sanity_skinned_model.GetKeyFrames()))[i];

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
					std::tie(sanity_bind_real, sanity_bind_dual, sanity_scale) = sanity_key_frames.Frame(static_cast<float>(frame_id));

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

			if (target.NumKeyframeSets() > 0)
			{
				EXPECT_EQ(target.NumFrames(), static_cast<int>(sanity_skinned_model.NumFrames()));
				EXPECT_EQ(target.FrameRate(), static_cast<int>(sanity_skinned_model.FrameRate()));
			}
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
