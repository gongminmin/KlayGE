#ifndef _MODEL_HPP
#define _MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

class MD5SkinnedMesh;

class MD5SkinnedModel : public KlayGE::SkinnedModel
{
	friend class MD5SkinnedMesh;

public:
	MD5SkinnedModel();

	void SetTime(float time);
	void SetEyePos(KlayGE::float3 const & eye_pos);

	bool reversed_;
};

class MD5SkinnedMesh : public KlayGE::SkinnedMesh
{
public:
	MD5SkinnedMesh(KlayGE::RenderModelPtr model, std::wstring const & name);

	void OnRenderBegin();

	void SetWorld(KlayGE::float4x4 const & mat);
	void SetEyePos(KlayGE::float3 const & eye_pos);

	void BuildMeshInfo();

private:
	KlayGE::float4x4 world_;
	KlayGE::RenderEffectPtr effect_;
};

#endif		// _MODEL_HPP
