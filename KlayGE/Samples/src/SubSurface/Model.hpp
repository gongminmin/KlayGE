#ifndef _MODEL_HPP
#define _MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

class DetailedMesh : public KlayGE::StaticMesh
{
public:
	explicit DetailedMesh(std::wstring_view name);

	void OnRenderBegin();

	void EyePos(KlayGE::float3 const & eye_pos);
	void LightPos(KlayGE::float3 const & light_pos);
	void LightColor(KlayGE::float3 const & light_color);
	void LightFalloff(KlayGE::float3 const & light_falloff);

	void BackFaceDepthPass(bool dfdp);
	void BackFaceDepthTex(KlayGE::TexturePtr const & tex);
	void SigmaT(float sigma_t);
	void MtlThickness(float thickness);

protected:
	void DoBuildMeshInfo(KlayGE::RenderModel const & model) override;

private:
	bool depth_texture_support_;
};

#endif		// _MODEL_HPP
