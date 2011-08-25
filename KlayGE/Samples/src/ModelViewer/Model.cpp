#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
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

#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include "Model.hpp"

using namespace KlayGE;

std::vector<GraphicsBufferPtr> tess_pattern_vbs;
std::vector<GraphicsBufferPtr> tess_pattern_ibs;

void InitInstancedTessBuffs()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	tess_pattern_vbs.resize(32);
	tess_pattern_ibs.resize(tess_pattern_vbs.size());

	ElementInitData init_data;
		
	std::vector<float2> vert;
	vert.push_back(float2(0, 0));
	vert.push_back(float2(1, 0));
	vert.push_back(float2(0, 1));
	init_data.row_pitch = static_cast<uint32_t>(vert.size() * sizeof(vert[0]));
	init_data.slice_pitch = 0;
	init_data.data = &vert[0];
	tess_pattern_vbs[0] = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

	std::vector<uint16_t> index;
	index.push_back(0);
	index.push_back(1);
	index.push_back(2);
	init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
	init_data.slice_pitch = 0;
	init_data.data = &index[0];
	tess_pattern_ibs[0] = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

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

		init_data.row_pitch = static_cast<uint32_t>(vert.size() * sizeof(vert[0]));
		init_data.slice_pitch = 0;
		init_data.data = &vert[0];
		tess_pattern_vbs[i] = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

		init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
		init_data.slice_pitch = 0;
		init_data.data = &index[0];
		tess_pattern_ibs[i] = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
	}
}


DetailedSkinnedMesh::DetailedSkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
	: SkinnedMesh(model, name),
		world_(float4x4::Identity()),
			effect_(checked_pointer_cast<DetailedSkinnedModel>(model)->Effect()),
			light_pos_(1, 1, -1),
			line_mode_(false), smooth_mesh_(false), tess_factor_(5), visualize_("Lighting")
{
	inv_world_ = MathLib::inverse(world_);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
	if (TM_Instanced == caps.tess_method)
	{
		tess_pattern_rl_ = rf.MakeRenderLayout();
		tess_pattern_rl_->TopologyType(RenderLayout::TT_TriangleList);

		skinned_pos_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, NULL, EF_ABGR32F);
		skinned_tex_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, NULL, EF_GR32F);
		skinned_normal_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, NULL, EF_ABGR32F);
		skinned_tangent_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Write, NULL, EF_ABGR32F);
		skinned_rl_ = rf.MakeRenderLayout();
		skinned_rl_->TopologyType(RenderLayout::TT_TriangleList);

		point_rl_ = rf.MakeRenderLayout();
		point_rl_->TopologyType(RenderLayout::TT_PointList);
	}

	mesh_rl_ = rl_;
}

void DetailedSkinnedMesh::BuildMeshInfo()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	has_skinned_ = false;
	for (uint32_t i = 0; i < rl_->NumVertexStreams(); ++ i)
	{
		GraphicsBufferPtr const & vb = rl_->GetVertexStream(i);
		if (VEU_BlendWeight == rl_->VertexStreamFormat(i)[0].usage)
		{
			GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
			vb_cpu->Resize(vb->Size());
			vb->CopyToBuffer(*vb_cpu);

			GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
			if ((mapper.Pointer<float4>() + this->StartVertexLocation())->x() > 0)
			{
				has_skinned_ = true;
			}

			break;
		}
	}


	RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());

	// Ω®¡¢Œ∆¿Ì
	has_opacity_map_ = false;
	normal_map_ = checked_pointer_cast<DetailedSkinnedModel>(model_.lock())->EmptyNormalMap();
	RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
	for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
		iter != texture_slots.end(); ++ iter)
	{
		if (("DiffuseMap" == iter->first) || ("Diffuse Color" == iter->first) || ("Diffuse Color Map" == iter->first))
		{
			if (!ResLoader::Instance().Locate(iter->second).empty())
			{
				diffuse_map_ = LoadTexture(iter->second, EAH_GPU_Read | EAH_Immutable)();
			}
		}
		else
		{
			if (("NormalMap" == iter->first) || ("Bump" == iter->first) || ("Bump Map" == iter->first))
			{
				if (!ResLoader::Instance().Locate(iter->second).empty())
				{
					normal_map_ = LoadTexture(iter->second, EAH_GPU_Read | EAH_Immutable)();
				}
			}
			else
			{
				if (("SpecularMap" == iter->first) || ("Specular Level" == iter->first) || ("Reflection Glossiness Map" == iter->first))
				{
					if (!ResLoader::Instance().Locate(iter->second).empty())
					{
						specular_map_ = LoadTexture(iter->second, EAH_GPU_Read | EAH_Immutable)();
					}
				}
				else
				{
					if (("EmitMap" == iter->first) || ("Self-Illumination" == iter->first))
					{
						if (!ResLoader::Instance().Locate(iter->second).empty())
						{
							emit_map_ = LoadTexture(iter->second, EAH_GPU_Read | EAH_Immutable)();
						}
					}
					else
					{
						if (("OpacityMap" == iter->first) || ("Opacity" == iter->first))
						{
							if (!ResLoader::Instance().Locate(iter->second).empty())
							{
								opacity_map_ = LoadTexture(iter->second, EAH_GPU_Read | EAH_Immutable)();

								if (opacity_map_)
								{
									has_opacity_map_ = true;
								}
							}
						}
					}
				}
			}
		}
	}

	ambient_clr_ = float4(mtl.ambient.x(), mtl.ambient.y(), mtl.ambient.z(), 1);
	diffuse_clr_ = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), bool(diffuse_map_));
	specular_clr_ = float4(mtl.specular.x(), mtl.specular.y(), mtl.specular.z(), bool(specular_map_));
	emit_clr_ = float4(mtl.emit.x(), mtl.emit.y(), mtl.emit.z(), bool(emit_map_));
	opacity_clr_ = float4(mtl.opacity, mtl.opacity, mtl.opacity, bool(opacity_map_));
	specular_level_ = mtl.specular_level;
	shininess_ = std::max(1e-6f, mtl.shininess);

	RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
	if (TM_Instanced == caps.tess_method)
	{
		skinned_pos_vb_->Resize(this->NumVertices() * sizeof(float4));
		skinned_tex_vb_->Resize(this->NumVertices() * sizeof(float2));
		skinned_normal_vb_->Resize(this->NumVertices() * sizeof(float4));
		skinned_tangent_vb_->Resize(this->NumVertices() * sizeof(float4));
		skinned_rl_->BindVertexStream(skinned_pos_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));
		skinned_rl_->BindVertexStream(skinned_tex_vb_, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
		skinned_rl_->BindVertexStream(skinned_normal_vb_, boost::make_tuple(vertex_element(VEU_Normal, 1, EF_ABGR32F)));
		skinned_rl_->BindVertexStream(skinned_tangent_vb_, boost::make_tuple(vertex_element(VEU_Tangent, 2, EF_ABGR32F)));
		skinned_rl_->BindIndexStream(rl_->GetIndexStream(), rl_->IndexStreamFormat());

		for (uint32_t i = 0; i < rl_->NumVertexStreams(); ++ i)
		{
			point_rl_->BindVertexStream(rl_->GetVertexStream(i), rl_->VertexStreamFormat(i));
		}
		point_rl_->NumVertices(rl_->NumVertices());
		point_rl_->StartVertexLocation(rl_->StartVertexLocation());

		uint32_t const index_size = (EF_R16UI == rl_->IndexStreamFormat()) ? 2 : 4;

		GraphicsBufferPtr ib_sysmem = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
		ib_sysmem->Resize(rl_->GetIndexStream()->Size());
		rl_->GetIndexStream()->CopyToBuffer(*ib_sysmem);
		{
			GraphicsBuffer::Mapper mapper(*ib_sysmem, BA_Read_Only);
			ElementInitData init_data;
			init_data.data = mapper.Pointer<uint8_t>() + this->StartIndexLocation() * index_size;
			init_data.row_pitch = this->NumTriangles() * 3 * index_size;
			init_data.slice_pitch = init_data.row_pitch;
			bindable_ib_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data, rl_->IndexStreamFormat());
		}

		this->SetTessFactor(static_cast<int32_t>(tess_factor_));
	}

	this->UpdateTech();
}

void DetailedSkinnedMesh::OnRenderBegin()
{
	*(effect_->ParameterByName("diffuse_tex")) = diffuse_map_;
	*(effect_->ParameterByName("normal_tex")) = normal_map_;
	*(effect_->ParameterByName("specular_tex")) = specular_map_;
	*(effect_->ParameterByName("emit_tex")) = emit_map_;
	*(effect_->ParameterByName("opacity_tex")) = opacity_map_;

	*(effect_->ParameterByName("has_skinned")) = static_cast<int32_t>(has_skinned_);

	*(effect_->ParameterByName("ambient_clr")) = ambient_clr_;
	*(effect_->ParameterByName("diffuse_clr")) = diffuse_clr_;
	*(effect_->ParameterByName("specular_clr")) = specular_clr_;
	*(effect_->ParameterByName("emit_clr")) = emit_clr_;
	*(effect_->ParameterByName("opacity_clr")) = opacity_clr_;

	*(effect_->ParameterByName("specular_level")) = specular_level_;
	*(effect_->ParameterByName("shininess")) = shininess_;

	App3DFramework& app = Context::Instance().AppInstance();
	float4x4 const & view = app.ActiveCamera().ViewMatrix();
	float4x4 worldview = world_ * view;
	*(effect_->ParameterByName("worldviewproj")) = worldview * app.ActiveCamera().ProjMatrix();

	*(effect_->ParameterByName("light_pos")) = MathLib::transform_coord(MathLib::transform_coord(light_pos_, MathLib::inverse(view)), inv_world_);

	RenderModelPtr model = model_.lock();
	if (model)
	{
		*(effect_->ParameterByName("joint_rots")) = checked_pointer_cast<DetailedSkinnedModel>(model)->GetBindRotations();
		*(effect_->ParameterByName("joint_poss")) = checked_pointer_cast<DetailedSkinnedModel>(model)->GetBindPositions();
	}

	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.tess_method != TM_No)
	{
		*(effect_->ParameterByName("adaptive_tess")) = true;
		*(effect_->ParameterByName("tess_factors")) = float4(tess_factor_, tess_factor_, 1.0f, 9.0f);

		if (TM_Instanced == caps.tess_method)
		{
			*(effect_->ParameterByName("skinned_pos_buf")) = skinned_pos_vb_;
			*(effect_->ParameterByName("skinned_tex_buf")) = skinned_tex_vb_;
			*(effect_->ParameterByName("skinned_normal_buf")) = skinned_normal_vb_;
			*(effect_->ParameterByName("skinned_tangent_buf")) = skinned_tangent_vb_;
			*(effect_->ParameterByName("index_buf")) = bindable_ib_;
			*(effect_->ParameterByName("start_index_loc")) = static_cast<int32_t>(this->StartIndexLocation());
			*(effect_->ParameterByName("base_vertex_loc")) = static_cast<int32_t>(this->BaseVertexLocation());
		}
	}
}

void DetailedSkinnedMesh::SetWorld(const float4x4& mat)
{
	world_ = mat;
	inv_world_ = MathLib::inverse(world_);
}

void DetailedSkinnedMesh::SetLightPos(KlayGE::float3 const & light_pos)
{
	light_pos_ = light_pos;
}

void DetailedSkinnedMesh::SetEyePos(KlayGE::float3 const & eye_pos)
{
	*(effect_->ParameterByName("eye_pos")) = MathLib::transform_coord(eye_pos, inv_world_);
}

void DetailedSkinnedMesh::VisualizeLighting()
{
	visualize_ = "Lighting";
	this->UpdateTech();
}

void DetailedSkinnedMesh::VisualizeVertex(VertexElementUsage usage, uint8_t usage_index)
{
	*(effect_->ParameterByName("vertex_usage")) = static_cast<int32_t>(usage);
	*(effect_->ParameterByName("vertex_usage_index")) = static_cast<int32_t>(usage_index);
	visualize_ = "VisualizeVertex";
	this->UpdateTech();
}

void DetailedSkinnedMesh::VisualizeTexture(int slot)
{
	*(effect_->ParameterByName("texture_slot")) = static_cast<int32_t>(slot);
	visualize_ = "VisualizeTexture";
	this->UpdateTech();
}

void DetailedSkinnedMesh::LineMode(bool line_mode)
{
	line_mode_ = line_mode;
	this->UpdateTech();
}

void DetailedSkinnedMesh::SmoothMesh(bool smooth)
{
	smooth_mesh_ = smooth;
	this->UpdateTech();
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
		tess_pattern_rl_->BindVertexStream(tess_pattern_vbs[tess_factor - 1], boost::make_tuple(vertex_element(VEU_TextureCoord, 1, EF_GR32F)),
			RenderLayout::ST_Geometry, mesh_rl_->NumIndices() * 3);
	}

	tess_factor_ = static_cast<float>(tess_factor);
}

void DetailedSkinnedMesh::UpdateTech()
{
	std::string tech = visualize_;
	if (line_mode_)
	{
		tech += "Line";
	}
	else
	{
		tech += "Fill";

		if (("Lighting" == visualize_)
			&& ((model_.lock()->GetMaterial(this->MaterialID()).opacity < 0.99f) || this->HasOpacityMap()))
		{
			tech += "Blend";
		}
	}
	if (smooth_mesh_)
	{
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		switch (caps.tess_method)
		{
		case TM_Hardware:
			tech += "Smooth5";
			break;

		case TM_Instanced:
			tech += "Smooth4";
			break;

		case TM_No:
			break;
		}
	}
	tech += "Tech";

	technique_ = effect_->TechniqueByName(tech);
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
			technique_ = technique_->Effect().TechniqueByName("SkinnedStreamOut");
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
		: SkinnedModel(name)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	std::vector<std::pair<std::string, std::string> > num_joints_macro;
	num_joints_macro.push_back(std::make_pair("NUM_JOINTS", "128"));
	num_joints_macro.push_back(std::make_pair("", ""));
	effect_ = rf.LoadEffect("ModelViewer.fxml", &num_joints_macro[0]);
	if (!effect_->TechniqueByName("LightingFillTech")->Validate())
	{
		num_joints_macro[0].second = "64";
		effect_ = rf.LoadEffect("ModelViewer.fxml", &num_joints_macro[0]);
	}

	uint32_t const empty_nor = 0x80808080;

	ElementInitData nor_init_data;
	nor_init_data.data = &empty_nor;
	nor_init_data.slice_pitch = nor_init_data.row_pitch = sizeof(empty_nor);

	ElementFormat format;
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8))
	{
		format = EF_ARGB8;
	}
	else
	{
		format = EF_ABGR8;
	}
	empty_normal_map_ = rf.MakeTexture2D(1, 1, 1, 1, format, 1, 0, EAH_GPU_Read, &nor_init_data);
}

void DetailedSkinnedModel::BuildModelInfo()
{
	bool has_tc = false;
	bool has_normal = false;
	bool has_tangent = false;
	bool has_binormal = false;
	bool has_skinned = false;
	RenderLayoutPtr const & rl = meshes_[0]->GetRenderLayout();
	for (uint32_t i = 0; i < rl->NumVertexStreams(); ++ i)
	{
		switch (rl->VertexStreamFormat(i)[0].usage)
		{
		case VEU_TextureCoord:
			has_tc = true;
			break;

		case VEU_Normal:
			has_normal = true;
			break;

		case VEU_Tangent:
			has_tangent = true;
			break;

		case VEU_Binormal:
			has_binormal = true;
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
	BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
	{
		total_num_vertices += mesh->NumVertices();
		total_num_indices += mesh->NumTriangles() * 3;
	}

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	std::vector<float3> positions(total_num_vertices);
	std::vector<float2> texcoords(total_num_vertices);
	std::vector<float3> normals(total_num_vertices);
	std::vector<float3> tangents(total_num_vertices);
	std::vector<float3> binormals(total_num_vertices);
	for (uint32_t i = 0; i < rl->NumVertexStreams(); ++ i)
	{
		GraphicsBufferPtr const & vb = rl->GetVertexStream(i);
		switch (rl->VertexStreamFormat(i)[0].usage)
		{
		case VEU_Position:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
				vb_cpu->Resize(vb->Size());
				vb->CopyToBuffer(*vb_cpu);

				GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
				std::copy(mapper.Pointer<float3>(), mapper.Pointer<float3>() + positions.size(), positions.begin());
			}
			break;

		case VEU_TextureCoord:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
				vb_cpu->Resize(vb->Size());
				vb->CopyToBuffer(*vb_cpu);

				GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
				std::copy(mapper.Pointer<float2>(), mapper.Pointer<float2>() + texcoords.size(), texcoords.begin());
			}
			break;

		case VEU_Normal:
		case VEU_Tangent:
		case VEU_Binormal:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
				vb_cpu->Resize(vb->Size());
				vb->CopyToBuffer(*vb_cpu);

				std::vector<uint32_t> n_32(total_num_vertices);
				{
					GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
					std::copy(mapper.Pointer<uint32_t>(), mapper.Pointer<uint32_t>() + n_32.size(), n_32.begin());
				}

				std::vector<float3>* p;
				switch (rl->VertexStreamFormat(i)[0].usage)
				{
				case VEU_Normal:
					p = &normals;
					break;

				case VEU_Tangent:
					p = &tangents;
					break;

				default:
					p = &binormals;
					break;
				}

				if (EF_A2BGR10 == rl->VertexStreamFormat(i)[0].format)
				{
					for (uint32_t j = 0; j < total_num_vertices; ++ j)
					{
						(*p)[j].x() = ((n_32[j] >>  0) & 0x3FF) / 1023.0f * 2 - 1;
						(*p)[j].y() = ((n_32[j] >> 10) & 0x3FF) / 1023.0f * 2 - 1;
						(*p)[j].z() = ((n_32[j] >> 20) & 0x3FF) / 1023.0f * 2 - 1;
					}
				}
				else
				{
					BOOST_ASSERT(EF_ARGB8 == rl->VertexStreamFormat(i)[0].format);

					for (uint32_t j = 0; j < total_num_vertices; ++ j)
					{
						(*p)[j].x() = ((n_32[j] >> 16) & 0xFF) / 255.0f * 2 - 1;
						(*p)[j].y() = ((n_32[j] >>  8) & 0xFF) / 255.0f * 2 - 1;
						(*p)[j].z() = ((n_32[j] >>  0) & 0xFF) / 255.0f * 2 - 1;
					}
				}
			}
			break;

		default:
			break;
		}
	}
	std::vector<uint16_t> indices(total_num_indices);
	{
		GraphicsBufferPtr ib = rl->GetIndexStream();
		GraphicsBufferPtr ib_cpu = rf.MakeIndexBuffer(BU_Static, EAH_CPU_Read, NULL);
		ib_cpu->Resize(ib->Size());
		ib->CopyToBuffer(*ib_cpu);

		GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);
		std::copy(mapper.Pointer<uint16_t>(), mapper.Pointer<uint16_t>() + indices.size(), indices.begin());
	}

	if (!has_tc)
	{
		texcoords.resize(total_num_vertices);
		for (size_t i = 0; i < texcoords.size(); ++ i)
		{
			texcoords[i] = float2(positions[i].x(), positions[i].y());
		}

		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
		{
			mesh->AddVertexStream(&texcoords[0], static_cast<uint32_t>(sizeof(texcoords[0]) * texcoords.size()),
				vertex_element(VEU_TextureCoord, 0, EF_GR32F), EAH_GPU_Read);
		}
	}

	if (!has_normal)
	{
		if (!has_tangent || !has_binormal)
		{
			BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
			{
				MathLib::compute_normal(normals.begin() + mesh->BaseVertexLocation(),
					indices.begin() + mesh->StartIndexLocation(), indices.begin() + mesh->StartIndexLocation() + mesh->NumTriangles() * 3,
					positions.begin() + mesh->BaseVertexLocation(), positions.begin() + mesh->BaseVertexLocation() + mesh->NumVertices());
			}
		}
		else
		{
			for (size_t i = 0; i < normals.size(); ++ i)
			{
				normals[i] = MathLib::cross(tangents[i], binormals[i]);
			}
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

		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
		{
			mesh->AddVertexStream(&compacted[0], static_cast<uint32_t>(sizeof(compacted[0]) * compacted.size()),
				vertex_element(VEU_Normal, 0, fmt), EAH_GPU_Read);
		}
	}

	if (!has_tangent)
	{
		// Compute TBN
		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
		{
			MathLib::compute_tangent(tangents.begin() + mesh->BaseVertexLocation(), binormals.begin() + mesh->BaseVertexLocation(),
				indices.begin() + mesh->StartIndexLocation(), indices.begin() + mesh->StartIndexLocation() + mesh->NumTriangles() * 3,
				positions.begin() + mesh->BaseVertexLocation(), positions.begin() + mesh->BaseVertexLocation() + mesh->NumVertices(),
				texcoords.begin() + mesh->BaseVertexLocation(), normals.begin() + mesh->BaseVertexLocation());
		}

		std::vector<uint32_t> compacted(total_num_vertices);
		ElementFormat fmt;
		if (rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_A2BGR10))
		{	
			fmt = EF_A2BGR10;
			for (size_t j = 0; j < compacted.size(); ++ j)
			{
				float3 n = MathLib::normalize(tangents[j]) * 0.5f + 0.5f;
				compacted[j] = MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.x() * 1023), 0, 1023)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.y() * 1023), 0, 1023) << 10)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.z() * 1023), 0, 1023) << 20);
			}
		}
		else
		{
			BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ARGB8));

			fmt = EF_ARGB8;
			for (size_t j = 0; j < compacted.size(); ++ j)
			{
				float3 n = MathLib::normalize(tangents[j]) * 0.5f + 0.5f;
				compacted[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.x() * 255), 0, 255) << 16)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.y() * 255), 0, 255) << 8)
					| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(n.z() * 255), 0, 255) << 0);
			}
		}

		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
		{
			mesh->AddVertexStream(&compacted[0], static_cast<uint32_t>(sizeof(compacted[0]) * compacted.size()),
				vertex_element(VEU_Tangent, 0, fmt), EAH_GPU_Read);
		}
	}

	if (!has_skinned)
	{
		GraphicsBufferPtr blend_indices_vb;
		GraphicsBufferPtr blend_weights_vb;

		ElementInitData init_data;
		{
			std::vector<int> blend_indices(total_num_vertices, 0);
			init_data.data = &blend_indices[0];
			init_data.row_pitch = total_num_vertices * sizeof(blend_indices[0]);
			init_data.slice_pitch = 0;
			blend_indices_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		}
		{
			std::vector<float4> blend_weights(total_num_vertices, float4(-1, -1, -1, -1));
			init_data.data = &blend_weights[0];
			init_data.row_pitch = total_num_vertices * sizeof(blend_weights[0]);
			init_data.slice_pitch = 0;
			blend_weights_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		}

		BOOST_FOREACH(BOOST_TYPEOF(meshes_)::const_reference mesh, meshes_)
		{
			mesh->AddVertexStream(blend_indices_vb,	vertex_element(VEU_BlendIndex, 0, EF_ABGR8));
			mesh->AddVertexStream(blend_weights_vb, vertex_element(VEU_BlendWeight, 0, EF_ABGR32F));
		}
	}
}

void DetailedSkinnedModel::SetLightPos(KlayGE::float3 const & light_pos)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedSkinnedMesh>(*iter)->SetLightPos(light_pos);
	}
}

void DetailedSkinnedModel::SetEyePos(KlayGE::float3 const & eye_pos)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedSkinnedMesh>(*iter)->SetEyePos(eye_pos);
	}
}

void DetailedSkinnedModel::SetTime(float time)
{
	this->SetFrame(time * frame_rate_);
}

void DetailedSkinnedModel::VisualizeLighting()
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedSkinnedMesh>(*iter)->VisualizeLighting();
	}
}

void DetailedSkinnedModel::VisualizeVertex(VertexElementUsage usage, uint8_t usage_index)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedSkinnedMesh>(*iter)->VisualizeVertex(usage, usage_index);
	}
}

void DetailedSkinnedModel::VisualizeTexture(int slot)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedSkinnedMesh>(*iter)->VisualizeTexture(slot);
	}
}

void DetailedSkinnedModel::LineMode(bool line_mode)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedSkinnedMesh>(*iter)->LineMode(line_mode);
	}
}

void DetailedSkinnedModel::SmoothMesh(bool smooth)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedSkinnedMesh>(*iter)->SmoothMesh(smooth);
	}
}

void DetailedSkinnedModel::SetTessFactor(int32_t tess_factor)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedSkinnedMesh>(*iter)->SetTessFactor(tess_factor);
	}
}
