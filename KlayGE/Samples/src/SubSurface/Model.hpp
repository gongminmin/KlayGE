#ifndef _MODEL_HPP
#define _MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

class DetailedMesh;

class DetailedModel : public KlayGE::RenderModel
{
	friend class DetailedMesh;

public:
	explicit DetailedModel(std::wstring const & name);

	void EyePos(KlayGE::float3 const & eye_pos);
	void LightPos(KlayGE::float3 const & light_pos);
	void LightColor(KlayGE::float3 const & light_color);
	void LightFalloff(KlayGE::float3 const & light_falloff);

	void BackFaceDepthPass(bool dfdp);
	void BackFaceDepthTex(KlayGE::TexturePtr const & tex);
	void SigmaT(float sigma_t);
	void MtlThickness(float thickness);

	KlayGE::TexturePtr const & EmptyBumpMap() const
	{
		return empty_bump_map_;
	}

private:
	KlayGE::TexturePtr empty_bump_map_;
};

class DetailedMesh : public KlayGE::StaticMesh
{
public:
	DetailedMesh(KlayGE::RenderModelPtr const & model, std::wstring const & name);

	void OnRenderBegin();

	void EyePos(KlayGE::float3 const & eye_pos);
	void LightPos(KlayGE::float3 const & light_pos);
	void LightColor(KlayGE::float3 const & light_color);
	void LightFalloff(KlayGE::float3 const & light_falloff);

	void BackFaceDepthPass(bool dfdp);
	void BackFaceDepthTex(KlayGE::TexturePtr const & tex);
	void SigmaT(float sigma_t);
	void MtlThickness(float thickness);

	virtual void DoBuildMeshInfo() override;

private:
	bool depth_texture_support_;
};

#endif		// _MODEL_HPP
