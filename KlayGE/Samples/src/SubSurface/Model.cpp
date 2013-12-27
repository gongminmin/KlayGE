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


DetailedMesh::DetailedMesh(RenderModelPtr const & model, std::wstring const & name)
	: StaticMesh(model, name)
{
	technique_ = SyncLoadRenderEffect("SubSurface.fxml")->TechniqueByName("SubSurfaceTech");
}

void DetailedMesh::BuildMeshInfo()
{
	StaticMesh::BuildMeshInfo();

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.texture_format_support(EF_D24S8) || caps.texture_format_support(EF_D16))
	{
		depth_texture_support_ = true;
	}
	else
	{
		depth_texture_support_ = false;
	}

	*(technique_->Effect().ParameterByName("diffuse_tex")) = diffuse_tl_;
	*(technique_->Effect().ParameterByName("bump_tex")) = normal_tl_;
	*(technique_->Effect().ParameterByName("specular_tex")) = specular_tl_;
	*(technique_->Effect().ParameterByName("specular_tex")) = shininess_tl_;

	*(technique_->Effect().ParameterByName("ambient_clr")) = float4(mtl_->ambient.x() * 0.2f, mtl_->ambient.y() * 0.2f, mtl_->ambient.z() * 0.2f, 1);
	*(technique_->Effect().ParameterByName("diffuse_clr")) = float4(mtl_->diffuse.x(), mtl_->diffuse.y(), mtl_->diffuse.z(), !!diffuse_tl_);
	*(technique_->Effect().ParameterByName("specular_clr")) = float4(mtl_->specular.x(), mtl_->specular.y(), mtl_->specular.z(), !!specular_tl_);
	*(technique_->Effect().ParameterByName("shininess_clr")) = float2(mtl_->shininess, !!shininess_tl_);

	float3 extinction_coefficient(0.2f, 0.8f, 0.12f);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		extinction_coefficient.x() = MathLib::srgb_to_linear(extinction_coefficient.x());
		extinction_coefficient.y() = MathLib::srgb_to_linear(extinction_coefficient.y());
		extinction_coefficient.z() = MathLib::srgb_to_linear(extinction_coefficient.z());
	}
	*(technique_->Effect().ParameterByName("extinction_coefficient")) = extinction_coefficient;

	AABBox const & pos_bb = this->PosBound();
	*(technique_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
	*(technique_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();

	AABBox const & tc_bb = this->TexcoordBound();
	*(technique_->Effect().ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
	*(technique_->Effect().ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
}

void DetailedMesh::OnRenderBegin()
{
	App3DFramework& app = Context::Instance().AppInstance();
	*(technique_->Effect().ParameterByName("worldviewproj")) = app.ActiveCamera().ViewProjMatrix();
}

void DetailedMesh::EyePos(KlayGE::float3 const & eye_pos)
{
	*(technique_->Effect().ParameterByName("eye_pos")) = eye_pos;
}

void DetailedMesh::LightPos(KlayGE::float3 const & light_pos)
{
	*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
}

void DetailedMesh::LightColor(KlayGE::float3 const & light_color)
{
	*(technique_->Effect().ParameterByName("light_color")) = light_color;
}

void DetailedMesh::LightFalloff(KlayGE::float3 const & light_falloff)
{
	*(technique_->Effect().ParameterByName("light_falloff")) = light_falloff;
}

void DetailedMesh::BackFaceDepthPass(bool dfdp)
{
	if (dfdp)
	{
		if (depth_texture_support_)
		{
			technique_ = technique_->Effect().TechniqueByName("BackFaceDepthTech");
		}
		else
		{
			technique_ = technique_->Effect().TechniqueByName("BackFaceDepthTechWODepthTexture");
		}
	}
	else
	{
		if (depth_texture_support_)
		{
			technique_ = technique_->Effect().TechniqueByName("SubSurfaceTech");
		}
		else
		{
			technique_ = technique_->Effect().TechniqueByName("SubSurfaceTechWODepthTexture");
		}
	}
}

void DetailedMesh::BackFaceDepthTex(KlayGE::TexturePtr const & tex)
{
	*(technique_->Effect().ParameterByName("back_face_depth_tex")) = tex;

	App3DFramework const & app = Context::Instance().AppInstance();
	Camera const & camera = app.ActiveCamera();
	if (depth_texture_support_)
	{
		float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
		*(technique_->Effect().ParameterByName("near_q")) = float2(camera.NearPlane() * q, q);
	}
	*(technique_->Effect().ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
}

void DetailedMesh::SigmaT(float sigma_t)
{
	*(technique_->Effect().ParameterByName("sigma_t")) = -sigma_t;
}

void DetailedMesh::MtlThickness(float thickness)
{
	*(technique_->Effect().ParameterByName("material_thickness")) = -thickness;
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
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
	{
		format = EF_ABGR8;
	}
	else
	{
		format = EF_ARGB8;
	}
	empty_bump_map_ = rf.MakeTexture2D(1, 1, 1, 1, format, 1, 0, EAH_GPU_Read | EAH_Immutable, &nor_init_data);
}

void DetailedModel::EyePos(KlayGE::float3 const & eye_pos)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->EyePos(eye_pos);
	}
}

void DetailedModel::LightPos(KlayGE::float3 const & light_pos)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->LightPos(light_pos);
	}
}

void DetailedModel::LightColor(KlayGE::float3 const & light_color)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->LightColor(light_color);
	}
}

void DetailedModel::LightFalloff(KlayGE::float3 const & light_falloff)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->LightFalloff(light_falloff);
	}
}

void DetailedModel::BackFaceDepthPass(bool dfdp)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->BackFaceDepthPass(dfdp);
	}
}

void DetailedModel::BackFaceDepthTex(KlayGE::TexturePtr const & tex)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<DetailedMesh>(*iter)->BackFaceDepthTex(tex);
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
