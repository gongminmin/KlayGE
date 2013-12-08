#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>

#include "SubsurfaceMesh.hpp"

using namespace KlayGE;

SubsurfaceMesh::SubsurfaceMesh(KlayGE::RenderModelPtr const & model, std::wstring const & name)
		: KlayGE::StaticMesh(model, name)
{
	effect_ = SyncLoadRenderEffect("SubsurfaceMesh.fxml");
	technique_ = effect_->TechniqueByName("GenShadowMap");
}

void SubsurfaceMesh::Pass(PassType type)
{
	type_ = type;
	switch (type_)
	{
	case PT_GenShadowMap:
		technique_ = effect_->TechniqueByName("GenShadowMap");
		break;

	default:
		technique_ = effect_->TechniqueByName("Shading");
		break;
	}
}

void SubsurfaceMesh::EyePosition(KlayGE::float3 const & eye_pos)
{
	*(technique_->Effect().ParameterByName("eye_pos")) = eye_pos;
}

void SubsurfaceMesh::LightPosition(KlayGE::float3 const & light_pos)
{
	*(technique_->Effect().ParameterByName("light_pos")) = light_pos;
}

void SubsurfaceMesh::LightColor(KlayGE::float3 const & light_color)
{
	*(technique_->Effect().ParameterByName("light_color")) = light_color;
}

void SubsurfaceMesh::LightFalloff(KlayGE::float3 const & light_falloff)
{
	*(technique_->Effect().ParameterByName("light_falloff")) = light_falloff;
}

void SubsurfaceMesh::ShadowTex(KlayGE::TexturePtr const & tex)
{
	*(technique_->Effect().ParameterByName("shadow_tex")) = tex;
}

void SubsurfaceMesh::AlbedoTex(KlayGE::TexturePtr const & albedo_tex)
{
	*(technique_->Effect().ParameterByName("albedo_tex")) = albedo_tex;
}

void SubsurfaceMesh::ShadingTex(KlayGE::TexturePtr const & shading_tex)
{
	*(technique_->Effect().ParameterByName("shading_tex")) = shading_tex;
}

void SubsurfaceMesh::SSSBlurredTex(KlayGE::TexturePtr const & sss_blurred_tex)
{
	*(technique_->Effect().ParameterByName("sss_blurred_tex")) = sss_blurred_tex;
}

void SubsurfaceMesh::OnRenderBegin()
{
	StaticMesh::OnRenderBegin();

	KlayGE::App3DFramework const & app = KlayGE::Context::Instance().AppInstance();
	switch (type_)
	{
	case PT_GenShadowMap:
		break;

	default:
		*(technique_->Effect().ParameterByName("model_view")) = app.ActiveCamera().ViewMatrix();
		*(technique_->Effect().ParameterByName("diffuse_tex")) = diffuse_tex_;
		*(technique_->Effect().ParameterByName("normal_tex")) = normal_tex_;
		break;
	}

	*(technique_->Effect().ParameterByName("mvp")) = app.ActiveCamera().ViewProjMatrix();
	KlayGE::AABBox const & pos_bb = this->PosBound();
	*(technique_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
	*(technique_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();
	KlayGE::AABBox const & tc_bb = this->TexcoordBound();
	*(technique_->Effect().ParameterByName("tc_center")) = KlayGE::float2(tc_bb.Center().x(), tc_bb.Center().y());
	*(technique_->Effect().ParameterByName("tc_extent")) = KlayGE::float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
}


MySceneObjectHelper::MySceneObjectHelper(std::string const & modelName)
		: SceneObjectHelper(SOA_Cullable)
{
	renderable_ = SyncLoadModel(modelName, EAH_GPU_Read | EAH_Immutable,
		CreateModelFactory<RenderModel>(), CreateMeshFactory<SubsurfaceMesh>())->Mesh(0);
}

void MySceneObjectHelper::EyePosition(float3 const & eye_pos)
{
	checked_pointer_cast<SubsurfaceMesh>(renderable_)->EyePosition(eye_pos);
}

void MySceneObjectHelper::LightPosition(float3 const & light_pos)
{
	checked_pointer_cast<SubsurfaceMesh>(renderable_)->LightPosition(light_pos);
}
void MySceneObjectHelper::LightColor(float3 const & light_color)
{
	checked_pointer_cast<SubsurfaceMesh>(renderable_)->LightColor(light_color);
}

void MySceneObjectHelper::LightFalloff(float3 const & light_falloff)
{
	checked_pointer_cast<SubsurfaceMesh>(renderable_)->LightFalloff(light_falloff);
}

void MySceneObjectHelper::ShadowTex(TexturePtr const & tex)
{
	checked_pointer_cast<SubsurfaceMesh>(renderable_)->ShadowTex(tex);
}
