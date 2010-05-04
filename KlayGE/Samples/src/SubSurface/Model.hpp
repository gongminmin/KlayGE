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
	DetailedModel(std::wstring const & name);

	void SetLightPos(KlayGE::float3 const & light_pos);
	void SetEyePos(KlayGE::float3 const & eye_pos);

	void BackFaceDepthPass(bool dfdp);

	void BackFaceDepthTex(KlayGE::TexturePtr const & tex, bool flipping);

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

	void SetLightPos(KlayGE::float3 const & light_pos);
	void SetEyePos(KlayGE::float3 const & eye_pos);

	void BackFaceDepthPass(bool dfdp);

	void BackFaceDepthTex(KlayGE::TexturePtr const & tex, bool flipping);

	void BuildMeshInfo();
};

#endif		// _MODEL_HPP
