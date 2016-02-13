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

std::vector<GraphicsBufferPtr> tess_pattern_vbs;
std::vector<GraphicsBufferPtr> tess_pattern_ibs;

void InitInstancedTessBuffs()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	tess_pattern_vbs.resize(32);
	tess_pattern_ibs.resize(tess_pattern_vbs.size());

	std::vector<float2> vert;
	vert.push_back(float2(0, 0));
	vert.push_back(float2(1, 0));
	vert.push_back(float2(0, 1));
	tess_pattern_vbs[0] = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, static_cast<uint32_t>(vert.size() * sizeof(vert[0])), &vert[0]);

	std::vector<uint16_t> index;
	index.push_back(0);
	index.push_back(1);
	index.push_back(2);
	tess_pattern_ibs[0] = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, static_cast<uint32_t>(index.size() * sizeof(index[0])), &index[0]);

	for (size_t i = 1; i < tess_pattern_vbs.size(); ++ i)
	{
		for (size_t j = 0; j < vert.size(); ++ j)
		{
			float f = i / (i + 1.0f);
			vert[j] *= f;
		}

		for (size_t j = 0; j < i + 1; ++ j)
		{
			vert.push_back(float2(1 - j / (i + 1.0f), j / (i + 1.0f)));
		}
		vert.push_back(float2(0, 1));

		uint16_t last_1_row = static_cast<uint16_t>(vert.size() - (i + 2));
		uint16_t last_2_row = static_cast<uint16_t>(last_1_row - (i + 1));

		for (size_t j = 0; j < i; ++ j)
		{
			index.push_back(static_cast<uint16_t>(last_2_row + j));
			index.push_back(static_cast<uint16_t>(last_1_row + j));
			index.push_back(static_cast<uint16_t>(last_1_row + j + 1));

			index.push_back(static_cast<uint16_t>(last_2_row + j));
			index.push_back(static_cast<uint16_t>(last_1_row + j + 1));
			index.push_back(static_cast<uint16_t>(last_2_row + j + 1));
		}
		index.push_back(static_cast<uint16_t>(last_2_row + i));
		index.push_back(static_cast<uint16_t>(last_1_row + i));
		index.push_back(static_cast<uint16_t>(last_1_row + i + 1));

		tess_pattern_vbs[i] = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
			static_cast<uint32_t>(vert.size() * sizeof(vert[0])), &vert[0]);
		tess_pattern_ibs[i] = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
			static_cast<uint32_t>(index.size() * sizeof(index[0])), &index[0]);
	}
}

void DeinitInstancedTessBuffs()
{
	tess_pattern_vbs.clear();
	tess_pattern_ibs.clear();
}


DetailedSkinnedMesh::DetailedSkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
	: SkinnedMesh(model, name),
			tess_factor_(5), visualize_(0), smooth_mesh_(false)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
	if (TM_Instanced == caps.tess_method)
	{
		tess_pattern_rl_ = rf.MakeRenderLayout();
		tess_pattern_rl_->TopologyType(RenderLayout::TT_TriangleList);

		skinned_rl_ = rf.MakeRenderLayout();
		skinned_rl_->TopologyType(RenderLayout::TT_TriangleList);

		point_rl_ = rf.MakeRenderLayout();
		point_rl_->TopologyType(RenderLayout::TT_PointList);
	}

	mesh_rl_ = rl_;
}

void DetailedSkinnedMesh::DoBuildMeshInfo()
{
	SkinnedMesh::DoBuildMeshInfo();

	this->BindDeferredEffect(checked_pointer_cast<DetailedSkinnedModel>(model_.lock())->Effect());

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
	if (TM_Instanced == caps.tess_method)
	{
		skinned_pos_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write,
			this->NumVertices() * sizeof(float4), nullptr, EF_ABGR32F);
		skinned_tex_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write,
			this->NumVertices() * sizeof(float2), nullptr, EF_GR32F);
		skinned_tangent_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write,
			this->NumVertices() * sizeof(float4), nullptr, EF_ABGR32F);
		skinned_rl_->BindVertexStream(skinned_pos_vb_, std::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));
		skinned_rl_->BindVertexStream(skinned_tex_vb_, std::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
		skinned_rl_->BindVertexStream(skinned_tangent_vb_, std::make_tuple(vertex_element(VEU_Tangent, 0, EF_ABGR32F)));
		skinned_rl_->BindIndexStream(rl_->GetIndexStream(), rl_->IndexStreamFormat());

		for (uint32_t i = 0; i < rl_->NumVertexStreams(); ++ i)
		{
			point_rl_->BindVertexStream(rl_->GetVertexStream(i), rl_->VertexStreamFormat(i));
		}
		point_rl_->NumVertices(rl_->NumVertices());
		point_rl_->StartVertexLocation(rl_->StartVertexLocation());

		uint32_t const index_size = (EF_R16UI == rl_->IndexStreamFormat()) ? 2 : 4;

		GraphicsBufferPtr ib_sysmem = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read,
			rl_->GetIndexStream()->Size(), nullptr);
		rl_->GetIndexStream()->CopyToBuffer(*ib_sysmem);
		{
			GraphicsBuffer::Mapper mapper(*ib_sysmem, BA_Read_Only);
			bindable_ib_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
				this->NumTriangles() * 3 * index_size,
				mapper.Pointer<uint8_t>() + this->StartIndexLocation() * index_size,
				rl_->IndexStreamFormat());
		}

		this->SetTessFactor(static_cast<int32_t>(tess_factor_));
	}
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

	if (!this->select_mode_on_)
	{
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		if (caps.tess_method != TM_No)
		{
			*(deferred_effect_->ParameterByName("adaptive_tess")) = true;
			*(deferred_effect_->ParameterByName("tess_factors")) = float4(tess_factor_, tess_factor_, 1.0f, 9.0f);

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			FrameBufferPtr const & fb = re.CurFrameBuffer();
			*(technique_->Effect().ParameterByName("frame_size")) = int2(fb->Width(), fb->Height());

			*(technique_->Effect().ParameterByName("view_vec")) = Context::Instance().AppInstance().ActiveCamera().ForwardVec();

			if (TM_Instanced == caps.tess_method)
			{
				*(deferred_effect_->ParameterByName("skinned_pos_buf")) = skinned_pos_vb_;
				*(deferred_effect_->ParameterByName("skinned_tex_buf")) = skinned_tex_vb_;
				*(deferred_effect_->ParameterByName("skinned_tangent_buf")) = skinned_tangent_vb_;
				*(deferred_effect_->ParameterByName("index_buf")) = bindable_ib_;
			}
		}
	}
}

void DetailedSkinnedMesh::VisualizeLighting()
{
	visualize_ = 0;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::VisualizeVertex(VertexElementUsage usage, uint8_t usage_index)
{
	*(deferred_effect_->ParameterByName("vertex_usage")) = static_cast<int32_t>(usage);
	*(deferred_effect_->ParameterByName("vertex_usage_index")) = static_cast<int32_t>(usage_index);
	visualize_ = 1;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::VisualizeTexture(int slot)
{
	*(deferred_effect_->ParameterByName("texture_slot")) = static_cast<int32_t>(slot);
	visualize_ = 2;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::SmoothMesh(bool smooth)
{
	smooth_mesh_ = smooth;
	this->UpdateTechniques();
}

void DetailedSkinnedMesh::SetTessFactor(int32_t tess_factor)
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (TM_Instanced == caps.tess_method)
	{
		if (tess_pattern_vbs.empty())
		{
			InitInstancedTessBuffs();
		}

		tess_factor = std::min(tess_factor, static_cast<int32_t>(tess_pattern_vbs.size()));

		tess_pattern_rl_->BindIndexStream(tess_pattern_ibs[tess_factor - 1], EF_R16UI);
		tess_pattern_rl_->BindVertexStream(tess_pattern_vbs[tess_factor - 1], std::make_tuple(vertex_element(VEU_TextureCoord, 1, EF_GR32F)),
			RenderLayout::ST_Geometry, mesh_rl_->NumIndices() * 3);
	}

	tess_factor_ = static_cast<float>(tess_factor);
}

void DetailedSkinnedMesh::UpdateTechniques()
{
	std::shared_ptr<DetailedSkinnedModel> model = checked_pointer_cast<DetailedSkinnedModel>(model_.lock());

	if (this->AlphaTest())
	{
		depth_tech_ = model->depth_alpha_test_techs_[visualize_][smooth_mesh_];
		gbuffer_rt0_tech_ = model->gbuffer_alpha_test_rt0_techs_[visualize_][smooth_mesh_];
		gbuffer_rt1_tech_ = model->gbuffer_alpha_test_rt1_techs_[visualize_][smooth_mesh_];
		gbuffer_mrt_tech_ = model->gbuffer_alpha_test_mrt_techs_[visualize_][smooth_mesh_];
	}
	else
	{
		depth_tech_ = model->depth_techs_[visualize_][smooth_mesh_];
		gbuffer_rt0_tech_ = model->gbuffer_rt0_techs_[visualize_][smooth_mesh_];
		gbuffer_rt1_tech_ = model->gbuffer_rt1_techs_[visualize_][smooth_mesh_];
		gbuffer_mrt_tech_ = model->gbuffer_mrt_techs_[visualize_][smooth_mesh_];
	}

	depth_alpha_blend_back_tech_ = model->depth_alpha_blend_back_techs_[visualize_][smooth_mesh_];
	depth_alpha_blend_front_tech_ = model->depth_alpha_blend_front_techs_[visualize_][smooth_mesh_];
	gbuffer_alpha_blend_back_rt0_tech_ = model->gbuffer_alpha_blend_back_rt0_techs_[visualize_][smooth_mesh_];
	gbuffer_alpha_blend_front_rt0_tech_ = model->gbuffer_alpha_blend_front_rt0_techs_[visualize_][smooth_mesh_];
	gbuffer_alpha_blend_back_rt1_tech_ = model->gbuffer_alpha_blend_back_rt1_techs_[visualize_][smooth_mesh_];
	gbuffer_alpha_blend_front_rt1_tech_ = model->gbuffer_alpha_blend_front_rt1_techs_[visualize_][smooth_mesh_];
	gbuffer_alpha_blend_back_mrt_tech_ = model->gbuffer_alpha_blend_back_mrt_techs_[visualize_][smooth_mesh_];
	gbuffer_alpha_blend_front_mrt_tech_ = model->gbuffer_alpha_blend_front_mrt_techs_[visualize_][smooth_mesh_];
	special_shading_tech_ = model->special_shading_techs_[visualize_][smooth_mesh_];
	special_shading_alpha_blend_back_tech_ = model->special_shading_alpha_blend_back_techs_[visualize_][smooth_mesh_];
	special_shading_alpha_blend_front_tech_ = model->special_shading_alpha_blend_front_techs_[visualize_][smooth_mesh_];

	select_mode_tech_ = model->select_mode_tech_;
}

void DetailedSkinnedMesh::Render()
{
	if (smooth_mesh_)
	{
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		if (TM_Hardware == caps.tess_method)
		{
			rl_ = mesh_rl_;
			SkinnedMesh::Render();
		}
		else
		{
			RenderTechniquePtr backup_tech = technique_;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			re.BindSOBuffers(skinned_rl_);
			rl_ = point_rl_;
			technique_ = technique_->Effect().TechniqueByName("StreamOutTech");
			SkinnedMesh::Render();
			re.BindSOBuffers(RenderLayoutPtr());

			technique_ = backup_tech;
			rl_ = tess_pattern_rl_;
			SkinnedMesh::Render();
		}
	}
	else
	{
		rl_ = mesh_rl_;
		SkinnedMesh::Render();
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

	std::string depth_tech_str;
	std::string depth_alpha_test_tech_str;
	std::string depth_alpha_blend_back_tech_str;
	std::string depth_alpha_blend_front_tech_str;
	std::string g_buffer_tech_str;
	std::string g_buffer_alpha_test_tech_str;
	std::string g_buffer_alpha_blend_back_tech_str;
	std::string g_buffer_alpha_blend_front_tech_str;
	std::string g_buffer_mrt_tech_str;
	std::string g_buffer_alpha_test_mrt_tech_str;
	std::string g_buffer_alpha_blend_back_mrt_tech_str;
	std::string g_buffer_alpha_blend_front_mrt_tech_str;
	std::string special_shading_tech_str;
	std::string special_shading_alpha_blend_back_tech_str;
	std::string special_shading_alpha_blend_front_tech_str;
	for (int vis = 0; vis < 3; ++ vis)
	{
		for (int smooth = 0; smooth < 2; ++ smooth)
		{
			switch (vis)
			{
			case 0:
				depth_tech_str = "Depth";
				g_buffer_tech_str = "GBuffer";
				special_shading_tech_str = "SpecialShading";
				break;

			case 1:
				g_buffer_tech_str = "VisualizeVertex";
				break;

			default:
				g_buffer_tech_str = "VisualizeTexture";
				break;
			}

			depth_alpha_test_tech_str = depth_tech_str;
			depth_alpha_blend_back_tech_str = depth_tech_str;
			depth_alpha_blend_front_tech_str = depth_tech_str;
			g_buffer_alpha_test_tech_str = g_buffer_tech_str;
			g_buffer_alpha_blend_back_tech_str = g_buffer_tech_str;
			g_buffer_alpha_blend_front_tech_str = g_buffer_tech_str;
			g_buffer_mrt_tech_str = g_buffer_tech_str;
			g_buffer_alpha_test_mrt_tech_str = g_buffer_mrt_tech_str;
			g_buffer_alpha_blend_back_mrt_tech_str = g_buffer_mrt_tech_str;
			g_buffer_alpha_blend_front_mrt_tech_str = g_buffer_mrt_tech_str;
			special_shading_alpha_blend_back_tech_str = special_shading_tech_str;
			special_shading_alpha_blend_front_tech_str = special_shading_tech_str;
			if (0 == vis)
			{
				depth_alpha_test_tech_str += "AlphaTest";
				depth_alpha_blend_back_tech_str += "AlphaBlendBack";
				depth_alpha_blend_front_tech_str += "AlphaBlendFront";

				g_buffer_alpha_test_tech_str += "AlphaTest";
				g_buffer_alpha_blend_back_tech_str += "AlphaBlendBack";
				g_buffer_alpha_blend_front_tech_str += "AlphaBlendFront";

				g_buffer_alpha_test_mrt_tech_str += "AlphaTest";
				g_buffer_alpha_blend_back_mrt_tech_str += "AlphaBlendBack";
				g_buffer_alpha_blend_front_mrt_tech_str += "AlphaBlendFront";

				special_shading_alpha_blend_back_tech_str += "AlphaBlendBack";
				special_shading_alpha_blend_front_tech_str += "AlphaBlendFront";
			}

			if (1 == smooth)
			{
				RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				switch (caps.tess_method)
				{
				case TM_Hardware:
					depth_tech_str += "Smooth5";
					depth_alpha_test_tech_str += "Smooth5";
					depth_alpha_blend_back_tech_str += "Smooth5";
					depth_alpha_blend_front_tech_str += "Smooth5";
					g_buffer_tech_str += "Smooth5";
					g_buffer_alpha_test_tech_str += "Smooth5";
					g_buffer_alpha_blend_back_tech_str += "Smooth5";
					g_buffer_alpha_blend_front_tech_str += "Smooth5";
					g_buffer_mrt_tech_str += "Smooth5";
					g_buffer_alpha_test_mrt_tech_str += "Smooth5";
					g_buffer_alpha_blend_back_mrt_tech_str += "Smooth5";
					g_buffer_alpha_blend_front_mrt_tech_str += "Smooth5";
					special_shading_tech_str += "Smooth5";
					special_shading_alpha_blend_back_tech_str += "Smooth5";
					special_shading_alpha_blend_front_tech_str += "Smooth5";
					break;

				case TM_Instanced:
					depth_tech_str += "Smooth4";
					depth_alpha_test_tech_str += "Smooth4";
					depth_alpha_blend_back_tech_str += "Smooth4";
					depth_alpha_blend_front_tech_str += "Smooth4";
					g_buffer_tech_str += "Smooth4";
					g_buffer_alpha_test_tech_str += "Smooth4";
					g_buffer_alpha_blend_back_tech_str += "Smooth4";
					g_buffer_alpha_blend_front_tech_str += "Smooth4";
					g_buffer_mrt_tech_str += "Smooth4";
					g_buffer_alpha_test_mrt_tech_str += "Smooth4";
					g_buffer_alpha_blend_back_mrt_tech_str += "Smooth4";
					g_buffer_alpha_blend_front_mrt_tech_str += "Smooth4";
					special_shading_tech_str += "Smooth4";
					special_shading_alpha_blend_back_tech_str += "Smooth4";
					special_shading_alpha_blend_front_tech_str += "Smooth4";
					break;

				case TM_No:
					break;
				}
			}

			depth_techs_[vis][smooth] = effect_->TechniqueByName(depth_tech_str + "Tech");
			depth_alpha_test_techs_[vis][smooth] = effect_->TechniqueByName(depth_alpha_test_tech_str + "Tech");
			depth_alpha_blend_back_techs_[vis][smooth] = effect_->TechniqueByName(depth_alpha_blend_back_tech_str + "Tech");
			depth_alpha_blend_front_techs_[vis][smooth] = effect_->TechniqueByName(depth_alpha_blend_front_tech_str + "Tech");

			gbuffer_rt0_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_tech_str + "RT0Tech");
			gbuffer_alpha_test_rt0_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_test_tech_str + "RT0Tech");
			gbuffer_alpha_blend_back_rt0_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_blend_back_tech_str + "RT0Tech");
			gbuffer_alpha_blend_front_rt0_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_blend_front_tech_str + "RT0Tech");

			gbuffer_rt1_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_tech_str + "RT1Tech");
			gbuffer_alpha_test_rt1_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_test_tech_str + "RT1Tech");
			gbuffer_alpha_blend_back_rt1_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_blend_back_tech_str + "RT1Tech");
			gbuffer_alpha_blend_front_rt1_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_blend_front_tech_str + "RT1Tech");

			gbuffer_mrt_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_mrt_tech_str + "MRTTech");
			gbuffer_alpha_test_mrt_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_test_mrt_tech_str + "MRTTech");
			gbuffer_alpha_blend_back_mrt_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_blend_back_mrt_tech_str + "MRTTech");
			gbuffer_alpha_blend_front_mrt_techs_[vis][smooth] = effect_->TechniqueByName(g_buffer_alpha_blend_front_mrt_tech_str + "MRTTech");

			if (0 == vis)
			{
				special_shading_techs_[vis][smooth] = effect_->TechniqueByName(special_shading_tech_str + "Tech");
				special_shading_alpha_blend_back_techs_[vis][smooth] = effect_->TechniqueByName(special_shading_alpha_blend_back_tech_str + "Tech");
				special_shading_alpha_blend_front_techs_[vis][smooth] = effect_->TechniqueByName(special_shading_alpha_blend_front_tech_str + "Tech");
			}
			else
			{
				special_shading_techs_[vis][smooth] = effect_->TechniqueByName("SpecialShadingTech");
				special_shading_alpha_blend_back_techs_[vis][smooth] = effect_->TechniqueByName("SpecialShadingAlphaBlendBackTech");
				special_shading_alpha_blend_front_techs_[vis][smooth] = effect_->TechniqueByName("SpecialShadingAlphaBlendFrontTech");
			}
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

void DetailedSkinnedModel::SmoothMesh(bool smooth)
{
	for (auto const & renderable : subrenderables_)
	{
		DetailedSkinnedMesh* mesh = checked_cast<DetailedSkinnedMesh*>(renderable.get());
		mesh->SmoothMesh(smooth);
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
