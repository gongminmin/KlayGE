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

#include "Model.hpp"

using namespace KlayGE;


DetailedMesh::DetailedMesh(RenderModelPtr const & model, std::wstring const & name)
	: StaticMesh(model, name)
{
	effect_ = SyncLoadRenderEffect("SubSurface.fxml");
	technique_ = effect_->TechniqueByName("SubSurfaceTech");
}

void DetailedMesh::DoBuildMeshInfo()
{
	StaticMesh::DoBuildMeshInfo();

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	depth_texture_support_ = caps.depth_texture_support;

	*(effect_->ParameterByName("albedo_tex")) = textures_[RenderMaterial::TS_Albedo];
	*(effect_->ParameterByName("normal_tex")) = textures_[RenderMaterial::TS_Normal];
	*(effect_->ParameterByName("glossiness_tex")) = textures_[RenderMaterial::TS_Glossiness];
	*(effect_->ParameterByName("normal_map_enabled")) = static_cast<int32_t>(!!textures_[RenderMaterial::TS_Normal]);

	*(effect_->ParameterByName("albedo_clr")) = mtl_->albedo;
	*(effect_->ParameterByName("albedo_map_enabled")) = static_cast<int32_t>(!!textures_[RenderMaterial::TS_Albedo]);
	*(effect_->ParameterByName("glossiness_clr")) = float2(mtl_->glossiness, !!textures_[RenderMaterial::TS_Glossiness]);

	float3 extinction_coefficient(0.2f, 0.8f, 0.12f);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		extinction_coefficient.x() = MathLib::srgb_to_linear(extinction_coefficient.x());
		extinction_coefficient.y() = MathLib::srgb_to_linear(extinction_coefficient.y());
		extinction_coefficient.z() = MathLib::srgb_to_linear(extinction_coefficient.z());
	}
	*(effect_->ParameterByName("extinction_coefficient")) = extinction_coefficient;

	AABBox const & pos_bb = this->PosBound();
	*(effect_->ParameterByName("pos_center")) = pos_bb.Center();
	*(effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();

	AABBox const & tc_bb = this->TexcoordBound();
	*(effect_->ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
	*(effect_->ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
}

void DetailedMesh::OnRenderBegin()
{
	App3DFramework& app = Context::Instance().AppInstance();
	*(effect_->ParameterByName("worldviewproj")) = app.ActiveCamera().ViewProjMatrix();
}

void DetailedMesh::EyePos(KlayGE::float3 const & eye_pos)
{
	*(effect_->ParameterByName("eye_pos")) = eye_pos;
}

void DetailedMesh::LightPos(KlayGE::float3 const & light_pos)
{
	*(effect_->ParameterByName("light_pos")) = light_pos;
}

void DetailedMesh::LightColor(KlayGE::float3 const & light_color)
{
	*(effect_->ParameterByName("light_color")) = light_color;
}

void DetailedMesh::LightFalloff(KlayGE::float3 const & light_falloff)
{
	*(effect_->ParameterByName("light_falloff")) = light_falloff;
}

void DetailedMesh::BackFaceDepthPass(bool dfdp)
{
	if (dfdp)
	{
		if (depth_texture_support_)
		{
			technique_ = effect_->TechniqueByName("BackFaceDepthTech");
		}
		else
		{
			technique_ = effect_->TechniqueByName("BackFaceDepthTechWODepthTexture");
		}
	}
	else
	{
		if (depth_texture_support_)
		{
			technique_ = effect_->TechniqueByName("SubSurfaceTech");
		}
		else
		{
			technique_ = effect_->TechniqueByName("SubSurfaceTechWODepthTexture");
		}
	}
}

void DetailedMesh::BackFaceDepthTex(KlayGE::TexturePtr const & tex)
{
	*(effect_->ParameterByName("back_face_depth_tex")) = tex;

	App3DFramework const & app = Context::Instance().AppInstance();
	Camera const & camera = app.ActiveCamera();
	if (depth_texture_support_)
	{
		float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
		*(effect_->ParameterByName("near_q")) = float2(camera.NearPlane() * q, q);
	}
	*(effect_->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
}

void DetailedMesh::SigmaT(float sigma_t)
{
	*(effect_->ParameterByName("sigma_t")) = -sigma_t;
}

void DetailedMesh::MtlThickness(float thickness)
{
	*(effect_->ParameterByName("material_thickness")) = -thickness;
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
	for (auto const & mesh : subrenderables_)
	{
		checked_pointer_cast<DetailedMesh>(mesh)->EyePos(eye_pos);
	}
}

void DetailedModel::LightPos(KlayGE::float3 const & light_pos)
{
	for (auto const & mesh : subrenderables_)
	{
		checked_pointer_cast<DetailedMesh>(mesh)->LightPos(light_pos);
	}
}

void DetailedModel::LightColor(KlayGE::float3 const & light_color)
{
	for (auto const & mesh : subrenderables_)
	{
		checked_pointer_cast<DetailedMesh>(mesh)->LightColor(light_color);
	}
}

void DetailedModel::LightFalloff(KlayGE::float3 const & light_falloff)
{
	for (auto const & mesh : subrenderables_)
	{
		checked_pointer_cast<DetailedMesh>(mesh)->LightFalloff(light_falloff);
	}
}

void DetailedModel::BackFaceDepthPass(bool dfdp)
{
	for (auto const & mesh : subrenderables_)
	{
		checked_pointer_cast<DetailedMesh>(mesh)->BackFaceDepthPass(dfdp);
	}
}

void DetailedModel::BackFaceDepthTex(KlayGE::TexturePtr const & tex)
{
	for (auto const & mesh : subrenderables_)
	{
		checked_pointer_cast<DetailedMesh>(mesh)->BackFaceDepthTex(tex);
	}
}

void DetailedModel::SigmaT(float sigma_t)
{
	for (auto const & mesh : subrenderables_)
	{
		checked_pointer_cast<DetailedMesh>(mesh)->SigmaT(sigma_t);
	}
}

void DetailedModel::MtlThickness(float thickness)
{
	for (auto const & mesh : subrenderables_)
	{
		checked_pointer_cast<DetailedMesh>(mesh)->MtlThickness(thickness);
	}
}
