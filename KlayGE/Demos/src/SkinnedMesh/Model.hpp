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
	void SetEyePos(const KlayGE::float3& eye_pos);
};

class MD5SkinnedMesh : public KlayGE::SkinnedMesh
{
public:
	explicit MD5SkinnedMesh(boost::shared_ptr<MD5SkinnedModel> model);

	void OnRenderBegin();

	void SetWorld(const KlayGE::float4x4& mat);
	void SetShaderName(std::string const & shader);
	void SetEyePos(const KlayGE::float3& eye_pos);

	void ComputeTB();

private:
	KlayGE::float4x4 world_;

	std::string shader_;
	KlayGE::SamplerPtr diffuse_map_, normal_map_, specular_map_;

	KlayGE::float3 eye_pos_;

	boost::weak_ptr<MD5SkinnedModel> model_;
};

#endif		// _MODEL_HPP
