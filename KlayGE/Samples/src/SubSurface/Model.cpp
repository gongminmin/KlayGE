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
#include <KlayGE/FrameBuffer.hpp>

#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include "Model.hpp"

using namespace KlayGE;


DetailedMesh::DetailedMesh(RenderModelPtr const & model, std::wstring const & name)
	: StaticMesh(model, name)
{
	technique_ = Context::Instance().RenderFactoryInstance().LoadEffect("SubSurface.fxml")->TechniqueByName("SubSurfaceTech");
}

void DetailedMesh::BuildMeshInfo()
{
	RenderModel::Material const & mtl = model_.lock()->GetMaterial(this->MaterialID());

	// Ω®¡¢Œ∆¿Ì
	TexturePtr dm, sm;
	TexturePtr bm = checked_pointer_cast<DetailedModel>(model_.lock())->EmptyBumpMap();
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
					bm = LoadTexture(iter->second, EAH_GPU_Read)();
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
			}
		}
	}
	*(technique_->Effect().ParameterByName("diffuse_tex")) = dm;
	*(technique_->Effect().ParameterByName("bump_tex")) = bm;
	*(technique_->Effect().ParameterByName("specular_tex")) = sm;

	*(technique_->Effect().ParameterByName("ambient_clr")) = float4(mtl.ambient.x(), mtl.ambient.y(), mtl.ambient.z(), 1);
	*(technique_->Effect().ParameterByName("diffuse_clr")) = float4(mtl.diffuse.x(), mtl.diffuse.y(), mtl.diffuse.z(), bool(dm));
	*(technique_->Effect().ParameterByName("specular_clr")) = float4(mtl.specular.x(), mtl.specular.y(), mtl.specular.z(), bool(sm));

	*(technique_->Effect().ParameterByName("specular_level")) = mtl.specular_level;
	*(technique_->Effect().ParameterByName("shininess")) = mtl.shininess;
}

void DetailedMesh::OnRenderBegin()
{
	App3DFramework& app = Context::Instance().AppInstance();
	float4x4 const & view = app.ActiveCamera().ViewMatrix();
	float4x4 const & proj = app.ActiveCamera().ProjMatrix();
	*(technique_->Effect().ParameterByName("worldviewproj")) = view * proj;
}

void DetailedMesh::SetLightPos(KlayGE::float3 const & light_pos)
{
	*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
}

void DetailedMesh::SetEyePos(KlayGE::float3 const & eye_pos)
{
	*(technique_->Effect().ParameterByName("eye_pos")) = eye_pos;
}

void DetailedMesh::BackFaceDepthPass(bool dfdp)
{
	if (dfdp)
	{
		technique_ = technique_->Effect().TechniqueByName("BackFaceDepthTech");
	}
	else
	{
		technique_ = technique_->Effect().TechniqueByName("SubSurfaceTech");
	}
}

void DetailedMesh::BackFaceDepthTex(KlayGE::TexturePtr const & tex, bool flipping)
{
	*(technique_->Effect().ParameterByName("back_face_depth_tex")) = tex;
	*(technique_->Effect().ParameterByName("flip")) = static_cast<int32_t>(flipping ? -1 : 1);
}

void DetailedMesh::SigmaT(float sigma_t)
{
	*(technique_->Effect().ParameterByName("sigma_t")) = sigma_t;
}

void DetailedMesh::MtlThickness(float thickness)
{
	*(technique_->Effect().ParameterByName("material_thickness")) = thickness;
}

DetailedModel::DetailedModel(std::wstring const & name)
		: RenderModel(name)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

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
	empty_bump_map_ = rf.MakeTexture2D(1, 1, 1, 1, format, 1, 0, EAH_GPU_Read, &nor_init_data);
}

void DetailedModel::SetLightPos(KlayGE::float3 const & light_pos)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->SetLightPos(light_pos);
	}
}

void DetailedModel::SetEyePos(KlayGE::float3 const & eye_pos)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->SetEyePos(eye_pos);
	}
}

void DetailedModel::BackFaceDepthPass(bool dfdp)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->BackFaceDepthPass(dfdp);
	}
}

void DetailedModel::BackFaceDepthTex(KlayGE::TexturePtr const & tex, bool flipping)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->BackFaceDepthTex(tex, flipping);
	}
}

void DetailedModel::SigmaT(float sigma_t)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->SigmaT(sigma_t);
	}
}

void DetailedModel::MtlThickness(float thickness)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->MtlThickness(thickness);
	}
}
