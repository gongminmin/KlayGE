#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderMaterial.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <iterator>

#include "Model.hpp"

using namespace KlayGE;

DetailedSkinnedMesh::DetailedSkinnedMesh(std::wstring_view name)
	: SkinnedMesh(name),
		visualize_(-1)
{
}

void DetailedSkinnedMesh::DoBuildMeshInfo(RenderModel const & model)
{
	SkinnedMesh::DoBuildMeshInfo(model);

	model_ = checked_cast<DetailedSkinnedModel const *>(&model);
	this->BindDeferredEffect(model_->Effect());
}

void DetailedSkinnedMesh::VisualizeLighting()
{
	visualize_ = -1;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::VisualizeVertex(VertexElementUsage usage, uint8_t usage_index)
{
	*(effect_->ParameterByName("vertex_usage")) = static_cast<int32_t>(usage);
	*(effect_->ParameterByName("vertex_usage_index")) = static_cast<int32_t>(usage_index);
	visualize_ = 0;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::VisualizeTexture(int slot)
{
	*(effect_->ParameterByName("texture_slot")) = static_cast<int32_t>(slot);
	visualize_ = 1;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::UpdateEffectAttrib()
{
	effect_attrs_ &= ~EA_TransparencyBack;
	effect_attrs_ &= ~EA_TransparencyFront;
	effect_attrs_ &= ~EA_AlphaTest;
	effect_attrs_ &= ~EA_SSS;
	effect_attrs_ &= ~EA_SpecialShading;

	if (mtl_->Transparent())
	{
		effect_attrs_ |= EA_TransparencyBack;
		effect_attrs_ |= EA_TransparencyFront;
	}
	if (mtl_->AlphaTestThreshold() > 0)
	{
		effect_attrs_ |= EA_AlphaTest;
	}
	if (mtl_->Sss())
	{
		effect_attrs_ |= EA_SSS;
	}
	if ((mtl_->Emissive().x() > 0) || (mtl_->Emissive().y() > 0) || (mtl_->Emissive().z() > 0) ||
		mtl_->Texture(RenderMaterial::TS_Emissive) || (effect_attrs_ & EA_TransparencyBack) || (effect_attrs_ & EA_TransparencyFront) ||
		(effect_attrs_ & EA_ObjectReflection))
	{
		effect_attrs_ |= EA_SpecialShading;
	}

	auto drl = Context::Instance().DeferredRenderingLayerInstance();
	if (drl)
	{
		this->BindDeferredEffect(drl->GBufferEffect(mtl_.get(), false, model_->IsSkinned()));
	}
}

void DetailedSkinnedMesh::UpdateMaterial()
{
	for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++ i)
	{
		mtl_->Texture(static_cast<RenderMaterial::TextureSlot>(i), ShaderResourceViewPtr());
	}

	StaticMesh::BuildMeshInfo(*model_);
}

void DetailedSkinnedMesh::UpdateTechniques()
{
	SkinnedMesh::UpdateTechniques();

	if (visualize_ >= 0)
	{
		gbuffer_tech_ = model_->visualize_gbuffer_techs_[visualize_];
		gbuffer_alpha_blend_back_tech_ = gbuffer_tech_;
		gbuffer_alpha_blend_front_tech_ = gbuffer_tech_;
	}
}


DetailedSkinnedModel::DetailedSkinnedModel(std::wstring_view name, uint32_t node_attrib)
		: SkinnedModel(name, node_attrib),
			is_skinned_(false)
{
}

void DetailedSkinnedModel::DoBuildModelInfo()
{
	bool has_tc = false;
	bool has_normal = false;
	bool has_tangent_quat = false;
	bool has_skinned = false;
	RenderLayout const & rl = meshes_[0]->GetRenderLayout();
	for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
	{
		switch (rl.VertexStreamFormat(i)[0].usage)
		{
		case VEU_TextureCoord:
			has_tc = true;
			break;

		case VEU_Normal:
			has_normal = true;
			break;

		case VEU_Tangent:
			has_tangent_quat = true;
			break;

		case VEU_BlendIndex:
		case VEU_BlendWeight:
			has_skinned = true;
			break;

		default:
			break;
		}
	}

	uint32_t total_num_vertices = 0;
	uint32_t total_num_indices = 0;
	for (auto const & renderable : meshes_)
	{
		StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
		for (uint32_t lod = 0; lod < mesh->NumLods(); ++ lod)
		{
			total_num_vertices += mesh->NumVertices(lod);
			total_num_indices += mesh->NumIndices(lod);
		}
	}

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	std::vector<float3> positions(total_num_vertices);
	std::vector<float2> texcoords(total_num_vertices);
	std::vector<float3> normals(total_num_vertices);
	std::vector<Quaternion> tangent_quats(total_num_vertices);
	for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
	{
		GraphicsBufferPtr const & vb = rl.GetVertexStream(i);
		switch (rl.VertexStreamFormat(i)[0].usage)
		{
		case VEU_Position:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, vb->Size(), nullptr);
				vb->CopyToBuffer(*vb_cpu);

				GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
				int16_t const * p_16 = mapper.Pointer<int16_t>();
				uint32_t j = 0;
				for (auto const & renderable : meshes_)
				{
					auto mesh = checked_pointer_cast<StaticMesh>(renderable);
					AABBox const & pos_bb = mesh->PosBound();
					float3 const pos_center = pos_bb.Center();
					float3 const pos_extent = pos_bb.HalfSize();
					for (uint32_t lod = 0; lod < mesh->NumLods(); ++ lod)
					{
						for (uint32_t v = 0; v < mesh->NumVertices(lod); ++ v)
						{
							positions[j].x() = ((p_16[j * 4 + 0] + 32768) / 65535.0f * 2 - 1) * pos_extent.x() + pos_center.x();
							positions[j].y() = ((p_16[j * 4 + 1] + 32768) / 65535.0f * 2 - 1) * pos_extent.y() + pos_center.y();
							positions[j].z() = ((p_16[j * 4 + 2] + 32768) / 65535.0f * 2 - 1) * pos_extent.z() + pos_center.z();
							++ j;
						}
					}
				}
			}
			break;

		case VEU_TextureCoord:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, vb->Size(), nullptr);
				vb->CopyToBuffer(*vb_cpu);

				GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
				int16_t const * t_16 = mapper.Pointer<int16_t>();
				uint32_t j = 0;
				for (auto const & renderable : meshes_)
				{
					auto mesh = checked_pointer_cast<StaticMesh>(renderable);
					AABBox const & tc_bb = mesh->TexcoordBound();
					float3 const tc_center = tc_bb.Center();
					float3 const tc_extent = tc_bb.HalfSize();
					for (uint32_t lod = 0; lod < mesh->NumLods(); ++ lod)
					{
						for (uint32_t v = 0; v < mesh->NumVertices(lod); ++ v)
						{
							texcoords[j].x() = ((t_16[j * 2 + 0] + 32768) / 65535.0f * 2 - 1) * tc_extent.x() + tc_center.x();
							texcoords[j].y() = ((t_16[j * 2 + 1] + 32768) / 65535.0f * 2 - 1) * tc_extent.y() + tc_center.y();
							++ j;
						}
					}
				}
			}
			break;

		case VEU_Normal:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, vb->Size(), nullptr);
				vb->CopyToBuffer(*vb_cpu);

				GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
				uint32_t const * n_32 = mapper.Pointer<uint32_t>();
				if (EF_A2BGR10 == rl.VertexStreamFormat(i)[0].format)
				{
					for (uint32_t j = 0; j < total_num_vertices; ++ j)
					{
						normals[j].x() = ((n_32[j] >>  0) & 0x3FF) / 1023.0f * 2 - 1;
						normals[j].y() = ((n_32[j] >> 10) & 0x3FF) / 1023.0f * 2 - 1;
						normals[j].z() = ((n_32[j] >> 20) & 0x3FF) / 1023.0f * 2 - 1;
					}
				}
				else if (EF_ABGR8 == rl.VertexStreamFormat(i)[0].format)
				{
					for (uint32_t j = 0; j < total_num_vertices; ++ j)
					{
						normals[j].x() = ((n_32[j] >>  0) & 0xFF) / 255.0f * 2 - 1;
						normals[j].y() = ((n_32[j] >>  8) & 0xFF) / 255.0f * 2 - 1;
						normals[j].z() = ((n_32[j] >> 16) & 0xFF) / 255.0f * 2 - 1;
					}
				}
				else
				{
					BOOST_ASSERT(EF_ARGB8 == rl.VertexStreamFormat(i)[0].format);

					for (uint32_t j = 0; j < total_num_vertices; ++ j)
					{
						normals[j].x() = ((n_32[j] >> 16) & 0xFF) / 255.0f * 2 - 1;
						normals[j].y() = ((n_32[j] >>  8) & 0xFF) / 255.0f * 2 - 1;
						normals[j].z() = ((n_32[j] >>  0) & 0xFF) / 255.0f * 2 - 1;
					}
				}
			}
			break;

		case VEU_Tangent:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, vb->Size(), nullptr);
				vb->CopyToBuffer(*vb_cpu);

				GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
				uint32_t const * t_32 = mapper.Pointer<uint32_t>();
				if (EF_ABGR8 == rl.VertexStreamFormat(i)[0].format)
				{
					for (uint32_t j = 0; j < total_num_vertices; ++ j)
					{
						tangent_quats[j].x() = ((t_32[j] >>  0) & 0xFF) / 255.0f * 2 - 1;
						tangent_quats[j].y() = ((t_32[j] >>  8) & 0xFF) / 255.0f * 2 - 1;
						tangent_quats[j].z() = ((t_32[j] >> 16) & 0xFF) / 255.0f * 2 - 1;
						tangent_quats[j].w() = ((t_32[j] >> 24) & 0xFF) / 255.0f * 2 - 1;
					}
				}
				else
				{
					BOOST_ASSERT(EF_ARGB8 == rl.VertexStreamFormat(i)[0].format);

					for (uint32_t j = 0; j < total_num_vertices; ++ j)
					{
						tangent_quats[j].x() = ((t_32[j] >> 16) & 0xFF) / 255.0f * 2 - 1;
						tangent_quats[j].y() = ((t_32[j] >>  8) & 0xFF) / 255.0f * 2 - 1;
						tangent_quats[j].z() = ((t_32[j] >>  0) & 0xFF) / 255.0f * 2 - 1;
						tangent_quats[j].w() = ((t_32[j] >> 24) & 0xFF) / 255.0f * 2 - 1;
					}
				}
			}
			break;

		default:
			break;
		}
	}
	std::vector<uint32_t> indices(total_num_indices);
	{
		GraphicsBufferPtr const & ib = rl.GetIndexStream();
		GraphicsBufferPtr ib_cpu = rf.MakeIndexBuffer(BU_Static, EAH_CPU_Read, ib->Size(), nullptr);
		ib->CopyToBuffer(*ib_cpu);

		GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);
		if (EF_R16UI == rl.IndexStreamFormat())
		{
			std::copy(mapper.Pointer<uint16_t>(), mapper.Pointer<uint16_t>() + indices.size(), indices.begin());
		}
		else
		{
			BOOST_ASSERT(EF_R32UI == rl.IndexStreamFormat());
			std::copy(mapper.Pointer<uint32_t>(), mapper.Pointer<uint32_t>() + indices.size(), indices.begin());
		}
	}

	if (!has_tc)
	{
		std::vector<int16_t> tcs16(total_num_vertices);
		texcoords.resize(total_num_vertices);
		uint32_t i = 0;
		for (auto const & renderable : meshes_)
		{
			auto mesh = checked_pointer_cast<StaticMesh>(renderable);
			AABBox const & tc_bb = mesh->TexcoordBound();
			float3 const tc_center = tc_bb.Center();
			float3 const tc_extent = tc_bb.HalfSize();
			for (uint32_t lod = 0; lod < mesh->NumLods(); ++lod)
			{
				for (uint32_t v = 0; v < mesh->NumVertices(lod); ++v)
				{
					texcoords[i] = float2(positions[i].x(), positions[i].y());

					float3 tc16 = float3(texcoords[i].x(), texcoords[i].y(), 0.0f);
					tc16 = (tc16 - tc_center) / tc_extent * 0.5f + 0.5f;
					tcs16[i] = static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tc16.x() * 65535 - 32768), -32768, 32767));
					tcs16[i] = static_cast<int16_t>(MathLib::clamp<int32_t>(static_cast<int32_t>(tc16.y() * 65535 - 32768), -32768, 32767));
					++ i;
				}
			}
		}

		for (auto const & renderable : meshes_)
		{
			StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
			for (uint32_t lod = 0; lod < mesh->NumLods(); ++ lod)
			{
				mesh->AddVertexStream(lod, &texcoords[0], static_cast<uint32_t>(sizeof(tcs16[0]) * tcs16.size()),
					VertexElement(VEU_TextureCoord, 0, EF_GR16), EAH_GPU_Read);
			}
		}
	}

	if (!has_tangent_quat)
	{
		if (!has_normal)
		{
			for (auto const & renderable : meshes_)
			{
				StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
				for (uint32_t lod = 0; lod < mesh->NumLods(); ++ lod)
				{
					MathLib::compute_normal(normals.begin() + mesh->StartVertexLocation(lod),
						indices.begin() + mesh->StartIndexLocation(lod), indices.begin() + mesh->StartIndexLocation(lod) + mesh->NumIndices(lod),
						positions.begin() + mesh->StartVertexLocation(lod), positions.begin() + mesh->StartVertexLocation(lod) + mesh->NumVertices(lod));
				}
			}
		}

		std::vector<float3> tangents(total_num_vertices);
		std::vector<float3> binormals(total_num_vertices);

		// Compute TBN
		for (auto const & renderable : meshes_)
		{
			StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
			for (uint32_t lod = 0; lod < mesh->NumLods(); ++ lod)
			{
				MathLib::compute_tangent(tangents.begin() + mesh->StartVertexLocation(lod), binormals.begin() + mesh->StartVertexLocation(lod),
					indices.begin() + mesh->StartIndexLocation(lod), indices.begin() + mesh->StartIndexLocation(lod) + mesh->NumIndices(lod),
					positions.begin() + mesh->StartVertexLocation(lod), positions.begin() + mesh->StartVertexLocation(lod) + mesh->NumVertices(lod),
					texcoords.begin() + mesh->StartVertexLocation(lod), normals.begin() + mesh->StartVertexLocation(lod));
			}
		}

		for (size_t j = 0; j < total_num_vertices; ++ j)
		{
			tangent_quats[j] = MathLib::to_quaternion(tangents[j], binormals[j], normals[j], 8);
		}

		std::vector<uint32_t> compacted(total_num_vertices);
		ElementFormat fmt;
		if (rf.RenderEngineInstance().DeviceCaps().VertexFormatSupport(EF_ABGR8))
		{	
			fmt = EF_ABGR8;
			for (size_t j = 0; j < compacted.size(); ++ j)
			{
				compacted[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
			}
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().VertexFormatSupport(EF_ARGB8));

			fmt = EF_ARGB8;
			for (size_t j = 0; j < compacted.size(); ++ j)
			{
				compacted[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].x() * 0.5f + 0.5f) * 255), 0, 255) << 16)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].z() * 0.5f + 0.5f) * 255), 0, 255) << 0)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
			}
		}

		for (auto const & renderable : meshes_)
		{
			StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
			for (uint32_t lod = 0; lod < mesh->NumLods(); ++ lod)
			{
				mesh->AddVertexStream(lod, &compacted[0], static_cast<uint32_t>(sizeof(compacted[0]) * compacted.size()),
					VertexElement(VEU_Tangent, 0, fmt), EAH_GPU_Read);
			}
		}
	}

	if (has_skinned)
	{
		effect_ = SyncLoadRenderEffects(MakeSpan<std::string>({"MtlEditor.fxml", "GBufferSkinning.fxml"}));
	}
	else
	{
		effect_ = SyncLoadRenderEffect("MtlEditor.fxml");
	}

	std::string g_buffer_tech_str;
	for (int vis = 0; vis < 2; ++ vis)
	{
		if (0 == vis)
		{
			g_buffer_tech_str = "VisualizeVertexTech";
		}
		else
		{
			g_buffer_tech_str = "VisualizeTextureTech";
		}
		visualize_gbuffer_techs_[vis] = effect_->TechniqueByName(g_buffer_tech_str);
	}

	is_skinned_ = has_skinned;
}

void DetailedSkinnedModel::SetTime(float time)
{
	this->SetFrame(time * frame_rate_);
}

uint32_t DetailedSkinnedModel::CopyMaterial(uint32_t mtl_index)
{
	uint32_t new_index = static_cast<uint32_t>(materials_.size());
	materials_.push_back(materials_[mtl_index]->Clone());
	return new_index;
}

uint32_t DetailedSkinnedModel::ImportMaterial(std::string const & name)
{
	uint32_t new_index = static_cast<uint32_t>(materials_.size());
	materials_.push_back(SyncLoadRenderMaterial(name));
	return new_index;
}


SkeletonMesh::SkeletonMesh(RenderModel const & model)
	: SkinnedMesh(L"SkeletonMesh"),
		model_(checked_cast<DetailedSkinnedModel const *>(&model))
{
	std::vector<float4> positions;
	std::vector<uint32_t> bone_indices;
	std::vector<uint16_t> indices;

	auto const& skinned_model = checked_cast<DetailedSkinnedModel const&>(model);

	std::map<JointComponent const*, int16_t> joint_indices;
	for (uint32_t i = 0; i < skinned_model.NumJoints(); ++i)
	{
		joint_indices.emplace(skinned_model.GetJoint(i).get(), static_cast<int16_t>(i));
	}

	struct JointInfo
	{
		JointComponent const* joint;
		int16_t parent_id;
	};

	std::vector<JointInfo> joints(skinned_model.NumJoints());
	for (uint32_t i = 0; i < skinned_model.NumJoints(); ++i)
	{
		joints[i].joint = skinned_model.GetJoint(i).get();

		auto const* parent_node = joints[i].joint->BoundSceneNode()->Parent();
		if (parent_node != nullptr)
		{
			auto const* parent_joint = parent_node->FirstComponentOfType<JointComponent>();
			BOOST_ASSERT(joint_indices.find(parent_joint) != joint_indices.end());
			joints[i].parent_id = static_cast<int16_t>(joint_indices[parent_joint]);
		}
		else
		{
			joints[i].parent_id = static_cast<int16_t>(-1);
		}
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(joints.size()); ++ i)
	{
		auto const& joint = *joints[i].joint;
		int16_t const parent_id = joints[i].parent_id;
		auto const* parent_node = joint.BoundSceneNode()->Parent();
		if (parent_node && parent_node->FirstComponentOfType<JointComponent>())
		{
			uint16_t const num_vertices = static_cast<uint16_t>(positions.size());

			float const color = i / (skinned_model.NumJoints() - 1.0f);

			Quaternion bind_real = joint.InverseOriginReal();
			Quaternion bind_dual = joint.InverseOriginDual();
			float bind_scale = joint.InverseOriginScale();
			if (MathLib::SignBit(bind_scale) > 0)
			{
				bind_dual *= bind_scale;
			}

			float4x4 mat = MathLib::inverse(
				MathLib::scaling(MathLib::abs(bind_scale), MathLib::abs(bind_scale), bind_scale)
				* MathLib::to_matrix(bind_real)
				* MathLib::translation(MathLib::udq_to_trans(bind_real, bind_dual)));
			float3 const joint_pos = MathLib::transform_coord(float3(0, 0, 0), mat);

			positions.emplace_back(joint_pos.x(), joint_pos.y(), joint_pos.z(), color);
			bone_indices.emplace_back(i);

			auto& parent_joint = *parent_node->FirstComponentOfType<JointComponent>();

			bind_real = parent_joint.InverseOriginReal();
			bind_dual = parent_joint.InverseOriginDual();
			bind_scale = parent_joint.InverseOriginScale();
			if (MathLib::SignBit(bind_scale) > 0)
			{
				bind_dual *= bind_scale;
			}

			float4x4 parent_mat = MathLib::inverse(
				MathLib::scaling(MathLib::abs(bind_scale), MathLib::abs(bind_scale), bind_scale)
				* MathLib::to_matrix(bind_real)
				* MathLib::translation(MathLib::udq_to_trans(bind_real, bind_dual)));
			float3 parent_joint_pos = MathLib::transform_coord(float3(0, 0, 0), parent_mat);

			float3 bone_vec = joint_pos - parent_joint_pos;
			float const len = MathLib::length(bone_vec);
			bone_vec /= len;
			float3 const x_dir = MathLib::cross(bone_vec, float3(+1, 0, 0));
			float3 const y_dir = MathLib::cross(bone_vec, x_dir);
			float3 const px_dir = parent_joint_pos + x_dir * len * 0.1f;
			float3 const py_dir = parent_joint_pos + y_dir * len * 0.1f;
			float3 const nx_dir = parent_joint_pos - x_dir * len * 0.1f;
			float3 const ny_dir = parent_joint_pos - y_dir * len * 0.1f;

			positions.emplace_back(px_dir.x(), px_dir.y(), px_dir.z(), color);
			bone_indices.emplace_back(parent_id);

			positions.emplace_back(ny_dir.x(), ny_dir.y(), ny_dir.z(), color);
			bone_indices.emplace_back(parent_id);

			positions.emplace_back(nx_dir.x(), nx_dir.y(), nx_dir.z(), color);
			bone_indices.emplace_back(parent_id);

			positions.emplace_back(py_dir.x(), py_dir.y(), py_dir.z(), color);
			bone_indices.emplace_back(parent_id);

			indices.push_back(num_vertices + 0);
			indices.push_back(num_vertices + 1);
			indices.push_back(num_vertices + 2);

			indices.push_back(num_vertices + 0);
			indices.push_back(num_vertices + 2);
			indices.push_back(num_vertices + 3);

			indices.push_back(num_vertices + 0);
			indices.push_back(num_vertices + 3);
			indices.push_back(num_vertices + 4);

			indices.push_back(num_vertices + 0);
			indices.push_back(num_vertices + 4);
			indices.push_back(num_vertices + 1);
		}
	}

	pos_aabb_ = MathLib::compute_aabbox(positions.begin(), positions.end());

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
		static_cast<uint32_t>(positions.size() * sizeof(positions[0])), &positions[0]);
	GraphicsBufferPtr bone_index_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
		static_cast<uint32_t>(bone_indices.size() * sizeof(bone_indices[0])), &bone_indices[0]);

	GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
		static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]);

	this->NumLods(1);
	rls_[0]->TopologyType(RenderLayout::TT_TriangleList);
	rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_ABGR32F));
	rls_[0]->BindVertexStream(bone_index_vb, VertexElement(VEU_BlendIndex, 0, EF_ABGR8UI));
	rls_[0]->BindIndexStream(ib, EF_R16UI);

	effect_attrs_ |= EA_SimpleForward;

	this->BindDeferredEffect(checked_cast<DetailedSkinnedModel const&>(model).Effect());
	simple_forward_tech_ = effect_->TechniqueByName("SkeletonTech");

	{
		// From https://github.com/BIDS/colormap/blob/master/parula.py
		uint32_t const color_map[] =
		{
			Color(0.2081f, 0.1663f, 0.5292f, 1).ABGR(),
			Color(0.2116238095f, 0.1897809524f, 0.5776761905f, 1).ABGR(),
			Color(0.212252381f, 0.2137714286f, 0.6269714286f, 1).ABGR(),
			Color(0.2081f, 0.2386f, 0.6770857143f, 1).ABGR(),
			Color(0.1959047619f, 0.2644571429f, 0.7279f, 1).ABGR(),
			Color(0.1707285714f, 0.2919380952f, 0.779247619f, 1).ABGR(),
			Color(0.1252714286f, 0.3242428571f, 0.8302714286f, 1).ABGR(),
			Color(0.0591333333f, 0.3598333333f, 0.8683333333f, 1).ABGR(),
			Color(0.0116952381f, 0.3875095238f, 0.8819571429f, 1).ABGR(),
			Color(0.0059571429f, 0.4086142857f, 0.8828428571f, 1).ABGR(),
			Color(0.0165142857f, 0.4266f, 0.8786333333f, 1).ABGR(),
			Color(0.032852381f, 0.4430428571f, 0.8719571429f, 1).ABGR(),
			Color(0.0498142857f, 0.4585714286f, 0.8640571429f, 1).ABGR(),
			Color(0.0629333333f, 0.4736904762f, 0.8554380952f, 1).ABGR(),
			Color(0.0722666667f, 0.4886666667f, 0.8467f, 1).ABGR(),
			Color(0.0779428571f, 0.5039857143f, 0.8383714286f, 1).ABGR(),
			Color(0.079347619f, 0.5200238095f, 0.8311809524f, 1).ABGR(),
			Color(0.0749428571f, 0.5375428571f, 0.8262714286f, 1).ABGR(),
			Color(0.0640571429f, 0.5569857143f, 0.8239571429f, 1).ABGR(),
			Color(0.0487714286f, 0.5772238095f, 0.8228285714f, 1).ABGR(),
			Color(0.0343428571f, 0.5965809524f, 0.819852381f, 1).ABGR(),
			Color(0.0265f, 0.6137f, 0.8135f, 1).ABGR(),
			Color(0.0238904762f, 0.6286619048f, 0.8037619048f, 1).ABGR(),
			Color(0.0230904762f, 0.6417857143f, 0.7912666667f, 1).ABGR(),
			Color(0.0227714286f, 0.6534857143f, 0.7767571429f, 1).ABGR(),
			Color(0.0266619048f, 0.6641952381f, 0.7607190476f, 1).ABGR(),
			Color(0.0383714286f, 0.6742714286f, 0.743552381f, 1).ABGR(),
			Color(0.0589714286f, 0.6837571429f, 0.7253857143f, 1).ABGR(),
			Color(0.0843f, 0.6928333333f, 0.7061666667f, 1).ABGR(),
			Color(0.1132952381f, 0.7015f, 0.6858571429f, 1).ABGR(),
			Color(0.1452714286f, 0.7097571429f, 0.6646285714f, 1).ABGR(),
			Color(0.1801333333f, 0.7176571429f, 0.6424333333f, 1).ABGR(),
			Color(0.2178285714f, 0.7250428571f, 0.6192619048f, 1).ABGR(),
			Color(0.2586428571f, 0.7317142857f, 0.5954285714f, 1).ABGR(),
			Color(0.3021714286f, 0.7376047619f, 0.5711857143f, 1).ABGR(),
			Color(0.3481666667f, 0.7424333333f, 0.5472666667f, 1).ABGR(),
			Color(0.3952571429f, 0.7459f, 0.5244428571f, 1).ABGR(),
			Color(0.4420095238f, 0.7480809524f, 0.5033142857f, 1).ABGR(),
			Color(0.4871238095f, 0.7490619048f, 0.4839761905f, 1).ABGR(),
			Color(0.5300285714f, 0.7491142857f, 0.4661142857f, 1).ABGR(),
			Color(0.5708571429f, 0.7485190476f, 0.4493904762f, 1).ABGR(),
			Color(0.609852381f, 0.7473142857f, 0.4336857143f, 1).ABGR(),
			Color(0.6473f, 0.7456f, 0.4188f, 1).ABGR(),
			Color(0.6834190476f, 0.7434761905f, 0.4044333333f, 1).ABGR(),
			Color(0.7184095238f, 0.7411333333f, 0.3904761905f, 1).ABGR(),
			Color(0.7524857143f, 0.7384f, 0.3768142857f, 1).ABGR(),
			Color(0.7858428571f, 0.7355666667f, 0.3632714286f, 1).ABGR(),
			Color(0.8185047619f, 0.7327333333f, 0.3497904762f, 1).ABGR(),
			Color(0.8506571429f, 0.7299f, 0.3360285714f, 1).ABGR(),
			Color(0.8824333333f, 0.7274333333f, 0.3217f, 1).ABGR(),
			Color(0.9139333333f, 0.7257857143f, 0.3062761905f, 1).ABGR(),
			Color(0.9449571429f, 0.7261142857f, 0.2886428571f, 1).ABGR(),
			Color(0.9738952381f, 0.7313952381f, 0.266647619f, 1).ABGR(),
			Color(0.9937714286f, 0.7454571429f, 0.240347619f, 1).ABGR(),
			Color(0.9990428571f, 0.7653142857f, 0.2164142857f, 1).ABGR(),
			Color(0.9955333333f, 0.7860571429f, 0.196652381f, 1).ABGR(),
			Color(0.988f, 0.8066f, 0.1793666667f, 1).ABGR(),
			Color(0.9788571429f, 0.8271428571f, 0.1633142857f, 1).ABGR(),
			Color(0.9697f, 0.8481380952f, 0.147452381f, 1).ABGR(),
			Color(0.9625857143f, 0.8705142857f, 0.1309f, 1).ABGR(),
			Color(0.9588714286f, 0.8949f, 0.1132428571f, 1).ABGR(),
			Color(0.9598238095f, 0.9218333333f, 0.0948380952f, 1).ABGR(),
			Color(0.9661f, 0.9514428571f, 0.0755333333f, 1).ABGR(),
			Color(0.9763f, 0.9831f, 0.0538f, 1).ABGR()
		};

		ElementInitData init_data;
		init_data.data = color_map;
		init_data.row_pitch = sizeof(color_map);
		init_data.slice_pitch = init_data.row_pitch * 1;
		TexturePtr color_map_tex = rf.MakeTexture2D(static_cast<uint32_t>(std::size(color_map)), 1, 1, 1, EF_ABGR8,
			1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data));
		*(effect_->ParameterByName("color_map")) = color_map_tex;
	}

	hw_res_ready_ = true;
}
