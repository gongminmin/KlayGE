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


DetailedMesh::DetailedMesh(std::wstring_view name)
	: StaticMesh(name)
{
	effect_ = SyncLoadRenderEffect("SubSurface.fxml");
	technique_ = effect_->TechniqueByName("SubSurfaceTech");
}

void DetailedMesh::DoBuildMeshInfo(RenderModel const & model)
{
	StaticMesh::DoBuildMeshInfo(model);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	depth_texture_support_ = caps.depth_texture_support;

	float3 extinction_coefficient(0.2f, 0.8f, 0.12f);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		extinction_coefficient.x() = MathLib::srgb_to_linear(extinction_coefficient.x());
		extinction_coefficient.y() = MathLib::srgb_to_linear(extinction_coefficient.y());
		extinction_coefficient.z() = MathLib::srgb_to_linear(extinction_coefficient.z());
	}
	*(effect_->ParameterByName("extinction_coefficient")) = extinction_coefficient;
}

void DetailedMesh::OnRenderBegin()
{
	StaticMesh::OnRenderBegin();

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
