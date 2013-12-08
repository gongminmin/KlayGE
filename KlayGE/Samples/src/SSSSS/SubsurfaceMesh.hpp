#ifndef _SUBSURFACEMESH_HPP
#define _SUBSURFACEMESH_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

class SubsurfaceMesh : public KlayGE::StaticMesh
{
public:
	SubsurfaceMesh(KlayGE::RenderModelPtr const & model, std::wstring const & name);
	virtual void Pass(KlayGE::PassType type) KLAYGE_OVERRIDE;
	void LightPosition(KlayGE::float3 const & pos);
	void EyePosition(KlayGE::float3 const & pos);
	void LightColor(KlayGE::float3 const & color);
	void LightFalloff(KlayGE::float3 const & falloff);
	void ShadowTex(KlayGE::TexturePtr const & tex);
	void AlbedoTex(KlayGE::TexturePtr const & albedo_tex);
	void ShadingTex(KlayGE::TexturePtr const & shading_tex);
	void SSSBlurredTex(KlayGE::TexturePtr const & sss_blurred_tex);

	void NormalMapTexture(KlayGE::TexturePtr const & tex);
	void ColorTexture(KlayGE::TexturePtr const & tex);

	virtual void OnRenderBegin() KLAYGE_OVERRIDE;

private:
	KlayGE::RenderEffectPtr effect_;
};

class MySceneObjectHelper : public KlayGE::SceneObjectHelper
{
public:
	MySceneObjectHelper(std::string const & model_name);

	void LightPosition(KlayGE::float3 const & pos);
	void EyePosition(KlayGE::float3 const & pos);
	void LightColor(KlayGE::float3 const & color);
	void LightFalloff(KlayGE::float3 const & falloff);
	void ShadowTex(KlayGE::TexturePtr const & tex);
};

#endif		// _SUBSURFACEMESH_HPP
