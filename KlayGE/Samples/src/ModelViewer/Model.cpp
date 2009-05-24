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


DetailedSkinnedMesh::DetailedSkinnedMesh(RenderModelPtr const & model, std::wstring const & name)
	: SkinnedMesh(model, name),
		world_(float4x4::Identity()),
			effect_(Context::Instance().RenderFactoryInstance().LoadEffect("ModelViewer.fxml")),
			light_pos_(1, 1, -1),
			line_mode_(false), visualize_("Lighting")
{
	inv_world_ = MathLib::inverse(world_);
}

void DetailedSkinnedMesh::BuildMeshInfo()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	bool has_normal = false;
	bool has_tangent = false;
	bool has_binormal = false;
	for (uint32_t i = 0; i < rl_->NumVertexStreams(); ++ i)
	{
		switch (rl_->VertexStreamFormat(i)[0].usage)
		{
		case VEU_Normal:
			has_normal = true;
			break;

		case VEU_Tangent:
			has_tangent = true;
			break;

		case VEU_Binormal:
			has_binormal = true;
			break;

		default:
			break;
		}
	}

	std::vector<float3> positions(this->NumVertices());
	std::vector<float2> texcoords(this->NumVertices());
	std::vector<float3> normals;
	std::vector<float3> tangents;
	std::vector<float3> binormals;
	bool has_skinned = false;
	for (uint32_t i = 0; i < rl_->NumVertexStreams(); ++ i)
	{
		GraphicsBufferPtr const & vb = rl_->GetVertexStream(i);
		switch (rl_->VertexStreamFormat(i)[0].usage)
		{
		case VEU_Position:
			{
				GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
				vb_cpu->Resize(vb->Size());
				vb->CopyToBuffer(*vb_cpu);

				{
					GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
					std::copy(mapper.Pointer<float3>(), mapper.Pointer<float3>() + positions.size(), positions.begin());

					std::copy(positions.begin(), positions.end(), mapper.Pointer<float3>());
				}
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
			if (!has_tangent)
			{
				normals.resize(this->NumVertices());
				{
					GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
					vb_cpu->Resize(vb->Size());
					vb->CopyToBuffer(*vb_cpu);

					{
						GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
						std::copy(mapper.Pointer<float3>(), mapper.Pointer<float3>() + normals.size(), normals.begin());

						std::copy(normals.begin(), normals.end(), mapper.Pointer<float3>());
					}
				}
			}
			break;

		case VEU_Tangent:
			if (!has_normal)
			{
				tangents.resize(this->NumVertices());
				{
					GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
					vb_cpu->Resize(vb->Size());
					vb->CopyToBuffer(*vb_cpu);

					{
						GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
						std::copy(mapper.Pointer<float3>(), mapper.Pointer<float3>() + tangents.size(), tangents.begin());

						std::copy(tangents.begin(), tangents.end(), mapper.Pointer<float3>());
					}
				}
			}
			break;

		case VEU_Binormal:
			if (!has_normal)
			{
				binormals.resize(this->NumVertices());
				{
					GraphicsBufferPtr vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, NULL);
					vb_cpu->Resize(vb->Size());
					vb->CopyToBuffer(*vb_cpu);

					{
						GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
						std::copy(mapper.Pointer<float3>(), mapper.Pointer<float3>() + binormals.size(), binormals.begin());

						std::copy(binormals.begin(), binormals.end(), mapper.Pointer<float3>());
					}
				}
			}
			break;

		case VEU_BlendIndex:
		case VEU_BlendWeight:
			has_skinned = true;
			break;

		default:
			break;
		}
	}
	std::vector<uint16_t> indices(this->NumTriangles() * 3);
	{
		GraphicsBufferPtr ib = rl_->GetIndexStream();
		GraphicsBufferPtr ib_cpu = rf.MakeIndexBuffer(BU_Static, EAH_CPU_Read, NULL);
		ib_cpu->Resize(ib->Size());
		ib->CopyToBuffer(*ib_cpu);

		GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);
		std::copy(mapper.Pointer<uint16_t>(), mapper.Pointer<uint16_t>() + indices.size(), indices.begin());
	}

	if (!has_normal)
	{
		normals.resize(this->NumVertices());

		if (!has_tangent || !has_binormal)
		{
			MathLib::compute_normal<float>(normals.begin(),
				indices.begin(), indices.end(), positions.begin(), positions.end());
		}
		else
		{
			for (size_t i = 0; i < normals.size(); ++ i)
			{
				normals[i] = MathLib::cross(tangents[i], binormals[i]);
			}
		}

		this->AddVertexStream(&normals[0], static_cast<uint32_t>(sizeof(normals[0]) * normals.size()),
			vertex_element(VEU_Normal, 0, EF_BGR32F), EAH_GPU_Read);
	}

	if (!has_tangent)
	{
		tangents.resize(this->NumVertices());
		binormals.resize(this->NumVertices());

		// 计算TBN
		MathLib::compute_tangent<float>(tangents.begin(), binormals.begin(),
			indices.begin(), indices.end(),
			positions.begin(), positions.end(),
			texcoords.begin(), normals.begin());
		this->AddVertexStream(&tangents[0], static_cast<uint32_t>(sizeof(tangents[0]) * tangents.size()),
				vertex_element(VEU_Tangent, 0, EF_BGR32F), EAH_GPU_Read);
	}

	if (!has_skinned)
	{
		std::vector<int> blend_indices(this->NumVertices(), 0);
		this->AddVertexStream(&blend_indices[0], static_cast<uint32_t>(sizeof(blend_indices[0]) * blend_indices.size()),
				vertex_element(VEU_BlendIndex, 0, EF_ABGR8), EAH_GPU_Read);
		std::vector<float4> blend_weights(this->NumVertices(), float4(0, 0, 0, 0));
		this->AddVertexStream(&blend_weights[0], static_cast<uint32_t>(sizeof(blend_weights[0]) * blend_weights.size()),
				vertex_element(VEU_BlendWeight, 0, EF_ABGR32F), EAH_GPU_Read);
	}

	if (!positions.empty())
	{
		box_ = MathLib::compute_bounding_box<float>(positions.begin(), positions.end());
	}

	RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());

	// 建立纹理
	has_opacity_map_ = false;
	TexturePtr dm, sm, em, om;
	TexturePtr nm = checked_pointer_cast<DetailedSkinnedModel>(model_.lock())->EmptyNormalMap();
	RenderModel::TextureSlotsType const & texture_slots = mtl.texture_slots;
	for (RenderModel::TextureSlotsType::const_iterator iter = texture_slots.begin();
		iter != texture_slots.end(); ++ iter)
	{
		if (("DiffuseMap" == iter->first) || ("Diffuse Color" == iter->first))
		{
			if (!ResLoader::Instance().Locate(iter->second).empty())
			{
				dm = LoadTexture(iter->second, EAH_GPU_Read)();
			}
		}
		else
		{
			if (("NormalMap" == iter->first) || ("Bump" == iter->first))
			{
				if (!ResLoader::Instance().Locate(iter->second).empty())
				{
					nm = LoadTexture(iter->second, EAH_GPU_Read)();
				}
			}
			else
			{
				if (("SpecularMap" == iter->first) || ("Specular Level" == iter->first))
				{
					if (!ResLoader::Instance().Locate(iter->second).empty())
					{
						sm = LoadTexture(iter->second, EAH_GPU_Read)();
					}
				}
				else
				{
					if (("EmitMap" == iter->first) || ("Self-Illumination" == iter->first))
					{
						if (!ResLoader::Instance().Locate(iter->second).empty())
						{
							em = LoadTexture(iter->second, EAH_GPU_Read)();
						}
					}
					else
					{
						if (("OpacityMap" == iter->first) || ("Opacity" == iter->first))
						{
							if (!ResLoader::Instance().Locate(iter->second).empty())
							{
								om = LoadTexture(iter->second, EAH_GPU_Read)();

								if (om)
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
	*(effect_->ParameterByName("diffuse_tex")) = dm;
	*(effect_->ParameterByName("normal_tex")) = nm;
	*(effect_->ParameterByName("specular_tex")) = sm;
	*(effect_->ParameterByName("emit_tex")) = em;
	*(effect_->ParameterByName("opacity_tex")) = om;

	*(effect_->ParameterByName("has_skinned")) = has_skinned;

	*(effect_->ParameterByName("ambient_clr")) = float4(mtl.ambient.x(), mtl.ambient.y(), mtl.ambient.z(), 1);
	*(effect_->ParameterByName("diffuse_clr")) = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), bool(dm));
	*(effect_->ParameterByName("specular_clr")) = float4(mtl.specular.x(), mtl.specular.y(), mtl.specular.z(), bool(sm));
	*(effect_->ParameterByName("emit_clr")) = float4(mtl.emit.x(), mtl.emit.y(), mtl.emit.z(), bool(em));
	*(effect_->ParameterByName("opacity_clr")) = float4(mtl.opacity, mtl.opacity, mtl.opacity, bool(om));

	*(effect_->ParameterByName("specular_level")) = mtl.specular_level;
	*(effect_->ParameterByName("shininess")) = mtl.shininess;

	this->UpdateTech();
}

void DetailedSkinnedMesh::OnRenderBegin()
{
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
	tech += "Tech";

	technique_ = effect_->TechniqueByName(tech);
}


DetailedSkinnedModel::DetailedSkinnedModel(std::wstring const & name)
		: SkinnedModel(name)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	uint32_t const empty_nor = 0x80808080;

	ElementInitData nor_init_data;
	nor_init_data.data = &empty_nor;
	nor_init_data.slice_pitch = nor_init_data.row_pitch = sizeof(empty_nor);

	ElementFormat format;
	if (rf.RenderEngineInstance().DeviceCaps().argb8_support)
	{
		format = EF_ARGB8;
	}
	else
	{
		format = EF_ABGR8;
	}
	empty_normal_map_ = rf.MakeTexture2D(1, 1, 1, format, 1, 0, EAH_GPU_Read, &nor_init_data);
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
	this->SetFrame(static_cast<int>(time * frame_rate_));
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
