#ifndef _MODEL_HPP
#define _MODEL_HPP

#include <KlayGE/PreDeclare.hpp>
#include <string>
#include <KlayGE/Mesh.hpp>

class MD5SkinnedModel : public KlayGE::SkinnedModel
{
public:
	MD5SkinnedModel();

	void SetTime(float time);
	void SetEyePos(KlayGE::float3 const & eye_pos);
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

	KlayGE::SamplerPtr diffuse_map_, normal_map_, specular_map_;

	KlayGE::float3 eye_pos_;
};

#endif		// _MODEL_HPP
