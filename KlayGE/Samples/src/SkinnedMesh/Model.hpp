#ifndef _MODEL_HPP
#define _MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

class DetailedSkinnedMesh;

class DetailedSkinnedModel : public KlayGE::SkinnedModel
{
	friend class DetailedSkinnedMesh;

public:
	DetailedSkinnedModel();

	void SetTime(float time);
	void SetLightPos(KlayGE::float3 const & light_pos);
	void SetEyePos(KlayGE::float3 const & eye_pos);

	bool reversed_;
};

class DetailedSkinnedMesh : public KlayGE::SkinnedMesh
{
public:
	DetailedSkinnedMesh(KlayGE::RenderModelPtr model, std::wstring const & name);

	void OnRenderBegin();

	void SetWorld(KlayGE::float4x4 const & mat);
	void SetLightPos(KlayGE::float3 const & light_pos);
	void SetEyePos(KlayGE::float3 const & eye_pos);

	void BuildMeshInfo();

private:
	KlayGE::float4x4 world_;
	KlayGE::float4x4 inv_world_;
	KlayGE::RenderEffectPtr effect_;
};

#endif		// _MODEL_HPP
