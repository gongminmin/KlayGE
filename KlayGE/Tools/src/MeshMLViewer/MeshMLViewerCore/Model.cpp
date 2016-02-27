#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include "Model.hpp"

using namespace KlayGE;

DetailedSkinnedMesh::DetailedSkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
	: SkinnedMesh(model, name),
			visualize_(-1)
{
}

void DetailedSkinnedMesh::DoBuildMeshInfo()
{
	SkinnedMesh::DoBuildMeshInfo();

	this->BindDeferredEffect(checked_pointer_cast<DetailedSkinnedModel>(model_.lock())->Effect());
}

void DetailedSkinnedMesh::OnRenderBegin()
{
	SkinnedMesh::OnRenderBegin();
	
	RenderModelPtr model = model_.lock();
	if (model)
	{
		*(deferred_effect_->ParameterByName("joint_reals")) = checked_pointer_cast<DetailedSkinnedModel>(model)->GetBindRealParts();
		*(deferred_effect_->ParameterByName("joint_duals")) = checked_pointer_cast<DetailedSkinnedModel>(model)->GetBindDualParts();
	}
}

void DetailedSkinnedMesh::VisualizeLighting()
{
	visualize_ = -1;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::VisualizeVertex(VertexElementUsage usage, uint8_t usage_index)
{
	*(deferred_effect_->ParameterByName("vertex_usage")) = static_cast<int32_t>(usage);
	*(deferred_effect_->ParameterByName("vertex_usage_index")) = static_cast<int32_t>(usage_index);
	visualize_ = 0;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::VisualizeTexture(int slot)
{
	*(deferred_effect_->ParameterByName("texture_slot")) = static_cast<int32_t>(slot);
	visualize_ = 1;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::SetTessFactor(int32_t tess_factor)
{
	mtl_->tess_factors.x() = mtl_->tess_factors.y() = static_cast<float>(tess_factor);
}

void DetailedSkinnedMesh::UpdateTechniques()
{
	std::shared_ptr<DetailedSkinnedModel> model = checked_pointer_cast<DetailedSkinnedModel>(model_.lock());

	if (visualize_ >= 0)
	{
		bool const smooth_mesh = (mtl_->detail_mode != RenderMaterial::SDM_Parallax);

		if (this->AlphaTest())
		{
			gbuffer_mrt_tech_ = model->gbuffer_alpha_test_mrt_techs_[visualize_][smooth_mesh];
		}
		else
		{
			gbuffer_mrt_tech_ = model->gbuffer_mrt_techs_[visualize_][smooth_mesh];
		}

		gbuffer_alpha_blend_back_mrt_tech_ = model->gbuffer_alpha_blend_back_mrt_techs_[visualize_][smooth_mesh];
		gbuffer_alpha_blend_front_mrt_tech_ = model->gbuffer_alpha_blend_front_mrt_techs_[visualize_][smooth_mesh];

		select_mode_tech_ = model->select_mode_tech_;
	}
	else
	{
		SkinnedMesh::UpdateTechniques();
	}
}


DetailedSkinnedModel::DetailedSkinnedModel(std::wstring const & name)
		: SkinnedModel(name),
			is_skinned_(false)
{
}

void DetailedSkinnedModel::DoBuildModelInfo()
{
	bool has_tc = false;
	bool has_normal = false;
	bool has_tangent_quat = false;
	bool has_skinned = false;
	RenderLayout const & rl = subrenderables_[0]->GetRenderLayout();
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
	for (auto const & renderable : subrenderables_)
	{
		StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
		total_num_vertices += mesh->NumVertices();
		total_num_indices += mesh->NumTriangles() * 3;
	}

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	AABBox const & pos_bb = this->PosBound();
	AABBox const & tc_bb = this->TexcoordBound();
	float3 const pos_center = pos_bb.Center();
	float3 const pos_extent = pos_bb.HalfSize();
	float3 const tc_center = tc_bb.Center();
	float3 const tc_extent = tc_bb.HalfSize();

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
				for (uint32_t j = 0; j < total_num_vertices; ++ j)
				{
					positions[j].x() = ((p_16[j * 4 + 0] + 32768) / 65535.0f * 2 - 1) * pos_extent.x() + pos_center.x();
					positions[j].y() = ((p_16[j * 4 + 1] + 32768) / 65535.0f * 2 - 1) * pos_extent.y() + pos_center.y();
					positions[j].z() = ((p_16[j * 4 + 2] + 32768) / 65535.0f * 2 - 1) * pos_extent.z() + pos_center.z();
				}
			}
			break;

		case VEU_TextureCoord:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, vb->Size(), nullptr);
				vb->CopyToBuffer(*vb_cpu);

				GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
				int16_t const * t_16 = mapper.Pointer<int16_t>();
				for (uint32_t j = 0; j < total_num_vertices; ++ j)
				{
					texcoords[j].x() = ((t_16[j * 2 + 0] + 32768) / 65535.0f * 2 - 1) * tc_extent.x() + tc_center.x();
					texcoords[j].y() = ((t_16[j * 2 + 1] + 32768) / 65535.0f * 2 - 1) * tc_extent.y() + tc_center.y();
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
		GraphicsBufferPtr ib = rl.GetIndexStream();
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
		texcoords.resize(total_num_vertices);
		for (size_t i = 0; i < texcoords.size(); ++ i)
		{
			texcoords[i] = float2(positions[i].x(), positions[i].y());
		}

		for (auto const & renderable : subrenderables_)
		{
			StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
			mesh->AddVertexStream(&texcoords[0], static_cast<uint32_t>(sizeof(texcoords[0]) * texcoords.size()),
				vertex_element(VEU_TextureCoord, 0, EF_GR32F), EAH_GPU_Read);
		}
	}

	if (!has_normal && !has_tc)
	{
		for (auto const & renderable : subrenderables_)
		{
			StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
			MathLib::compute_normal(normals.begin() + mesh->StartVertexLocation(),
				indices.begin() + mesh->StartIndexLocation(), indices.begin() + mesh->StartIndexLocation() + mesh->NumTriangles() * 3,
				positions.begin() + mesh->StartVertexLocation(), positions.begin() + mesh->StartVertexLocation() + mesh->NumVertices());
		}

		std::vector<uint32_t> compacted(total_num_vertices);
		ElementFormat fmt;
		if (rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_A2BGR10))
		{	
			fmt = EF_A2BGR10;
			for (size_t j = 0; j < compacted.size(); ++ j)
			{
				float3 n = MathLib::normalize(normals[j]) * 0.5f + 0.5f;
				compacted[j] = MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.x() * 1023), 0, 1023)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.y() * 1023), 0, 1023) << 10)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.z() * 1023), 0, 1023) << 20);
			}
		}
		else if (rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ABGR8))
		{
			fmt = EF_ABGR8;
			for (size_t j = 0; j < compacted.size(); ++ j)
			{
				float3 n = MathLib::normalize(normals[j]) * 0.5f + 0.5f;
				compacted[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.x() * 255), 0, 255) << 0)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.y() * 255), 0, 255) << 8)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.z() * 255), 0, 255) << 16);
			}
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ARGB8));

			fmt = EF_ARGB8;
			for (size_t j = 0; j < compacted.size(); ++ j)
			{
				float3 n = MathLib::normalize(normals[j]) * 0.5f + 0.5f;
				compacted[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.x() * 255), 0, 255) << 16)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.y() * 255), 0, 255) << 8)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.z() * 255), 0, 255) << 0);
			}
		}

		for (auto const & renderable : subrenderables_)
		{
			StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
			mesh->AddVertexStream(&compacted[0], static_cast<uint32_t>(sizeof(compacted[0]) * compacted.size()),
				vertex_element(VEU_Normal, 0, fmt), EAH_GPU_Read);
		}
	}
	else if (!has_tangent_quat)
	{
		std::vector<float3> tangents(total_num_vertices);
		std::vector<float3> binormals(total_num_vertices);
		
		// Compute TBN
		for (auto const & renderable : subrenderables_)
		{
			StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
			MathLib::compute_tangent(tangents.begin() + mesh->StartVertexLocation(), binormals.begin() + mesh->StartVertexLocation(),
				indices.begin() + mesh->StartIndexLocation(), indices.begin() + mesh->StartIndexLocation() + mesh->NumTriangles() * 3,
				positions.begin() + mesh->StartVertexLocation(), positions.begin() + mesh->StartVertexLocation() + mesh->NumVertices(),
				texcoords.begin() + mesh->StartVertexLocation(), normals.begin() + mesh->StartVertexLocation());
		}

		for (size_t j = 0; j < total_num_vertices; ++ j)
		{
			tangent_quats[j] = MathLib::to_quaternion(tangents[j], binormals[j], normals[j], 8);
		}

		std::vector<uint32_t> compacted(total_num_vertices);
		ElementFormat fmt;
		if (rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ABGR8))
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
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ARGB8));

			fmt = EF_ARGB8;
			for (size_t j = 0; j < compacted.size(); ++ j)
			{
				compacted[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].x() * 0.5f + 0.5f) * 255), 0, 255) << 16)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].z() * 0.5f + 0.5f) * 255), 0, 255) << 0)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quats[j].w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
			}
		}

		for (auto const & renderable : subrenderables_)
		{
			StaticMeshPtr mesh = checked_pointer_cast<StaticMesh>(renderable);
			mesh->AddVertexStream(&compacted[0], static_cast<uint32_t>(sizeof(compacted[0]) * compacted.size()),
				vertex_element(VEU_Tangent, 0, fmt), EAH_GPU_Read);
		}
	}

	if (has_skinned)
	{
		effect_ = SyncLoadRenderEffect("MeshMLViewerSkinning128.fxml");
		if (!effect_->TechniqueByName("GBufferRT0Tech")->Validate())
		{
			effect_ = SyncLoadRenderEffect("MeshMLViewerSkinning64.fxml");
		}
	}
	else
	{
		effect_ = SyncLoadRenderEffect("MeshMLViewerNoSkinning.fxml");
	}

	std::string g_buffer_mrt_tech_str;
	std::string g_buffer_alpha_test_mrt_tech_str;
	std::string g_buffer_alpha_blend_back_mrt_tech_str;
	std::string g_buffer_alpha_blend_front_mrt_tech_str;
	for (int vis = 0; vis < 2; ++ vis)
	{
		for (int smooth = 0; smooth < 2; ++ smooth)
		{
			if (0 == vis)
			{
				g_buffer_mrt_tech_str = "VisualizeVertex";
			}
			else
			{
				g_buffer_mrt_tech_str = "VisualizeTexture";
			}

			g_buffer_alpha_test_mrt_tech_str = g_buffer_mrt_tech_str;
			g_buffer_alpha_blend_back_mrt_tech_str = g_buffer_mrt_tech_str;
			g_buffer_alpha_blend_front_mrt_tech_str = g_buffer_mrt_tech_str;
			if (0 == vis)
			{
				g_buffer_alpha_test_mrt_tech_str += "AlphaTest";
				g_buffer_alpha_blend_back_mrt_tech_str += "AlphaBlendBack";
				g_buffer_alpha_blend_front_mrt_tech_str += "AlphaBlendFront";
			}

			if ((vis > 0) && (1 == smooth))
			{
				RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				switch (caps.tess_method)
				{
				case TM_Hardware:
					g_buffer_mrt_tech_str += "Smooth";
					g_buffer_alpha_test_mrt_tech_str += "Smooth";
					g_buffer_alpha_blend_back_mrt_tech_str += "Smooth";
					g_buffer_alpha_blend_front_mrt_tech_str += "Smooth";
					break;

				case TM_No:
					break;
				}
			}

			gbuffer_mrt_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_mrt_tech_str + "MRTTech");
			gbuffer_alpha_test_mrt_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_test_mrt_tech_str + "MRTTech");
			gbuffer_alpha_blend_back_mrt_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_blend_back_mrt_tech_str + "MRTTech");
			gbuffer_alpha_blend_front_mrt_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_blend_front_mrt_tech_str + "MRTTech");
		}
	}

	select_mode_tech_ = effect_->TechniqueByName("SelectModeTech");

	is_skinned_ = has_skinned;
}

void DetailedSkinnedModel::SetTime(float time)
{
	this->SetFrame(time * frame_rate_);
}

void DetailedSkinnedModel::VisualizeLighting()
{
	for (auto const & renderable : subrenderables_)
	{
		DetailedSkinnedMesh* mesh = checked_cast<DetailedSkinnedMesh*>(renderable.get());
		mesh->VisualizeLighting();
	}
}

void DetailedSkinnedModel::VisualizeVertex(VertexElementUsage usage, uint8_t usage_index)
{
	for (auto const & renderable : subrenderables_)
	{
		DetailedSkinnedMesh* mesh = checked_cast<DetailedSkinnedMesh*>(renderable.get()); 
		mesh->VisualizeVertex(usage, usage_index);
	}
}

void DetailedSkinnedModel::VisualizeTexture(int slot)
{
	for (auto const & renderable : subrenderables_)
	{
		DetailedSkinnedMesh* mesh = checked_cast<DetailedSkinnedMesh*>(renderable.get());
		mesh->VisualizeTexture(slot);
	}
}

void DetailedSkinnedModel::SetTessFactor(int32_t tess_factor)
{
	for (auto const & renderable : subrenderables_)
	{
		DetailedSkinnedMesh* mesh = checked_cast<DetailedSkinnedMesh*>(renderable.get());
		mesh->SetTessFactor(tess_factor);
	}
}
