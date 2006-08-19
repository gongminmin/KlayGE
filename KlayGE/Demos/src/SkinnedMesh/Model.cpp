#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Sampler.hpp>

#include <cassert>

#include "md5.hpp"

using namespace KlayGE;


MD5SkinnedMesh::MD5SkinnedMesh(boost::shared_ptr<MD5SkinnedModel> model)
	: SkinnedMesh(L"MD5SkinnedMesh"),
		world_(float4x4::Identity()),
		model_(model)
{
	technique_ = Context::Instance().RenderFactoryInstance().LoadEffect("SkinnedMesh.fx")->Technique("SkinnedMeshTech");

	diffuse_map_.reset(new Sampler);
	diffuse_map_->Filtering(Sampler::TFO_Bilinear);
	diffuse_map_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
	diffuse_map_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);

	normal_map_.reset(new Sampler);
	normal_map_->Filtering(Sampler::TFO_Bilinear);
	normal_map_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
	normal_map_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);

	specular_map_.reset(new Sampler);
	specular_map_->Filtering(Sampler::TFO_Bilinear);
	specular_map_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
	specular_map_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
}

void MD5SkinnedMesh::OnRenderBegin()
{
	App3DFramework& app = Context::Instance().AppInstance();
	float4x4 worldview(world_ * app.ActiveCamera().ViewMatrix());
	*(technique_->Effect().ParameterByName("worldviewproj")) = worldview * app.ActiveCamera().ProjMatrix();

	*(technique_->Effect().ParameterByName("diffuse_map")) = diffuse_map_;
	*(technique_->Effect().ParameterByName("normal_map")) = normal_map_;
	*(technique_->Effect().ParameterByName("specular_map")) = specular_map_;

	*(technique_->Effect().ParameterByName("eye_pos")) = eye_pos_;

	boost::shared_ptr<MD5SkinnedModel> model = model_.lock();
	if (model)
	{
		*(technique_->Effect().ParameterByName("joint_rots")) = model->GetBindRotations();
		*(technique_->Effect().ParameterByName("joint_poss")) = model->GetBindPositions();
	}
}

void MD5SkinnedMesh::SetWorld(const float4x4& mat)
{
	world_ = mat;
}

void MD5SkinnedMesh::SetShaderName(std::string const & shader)
{
	shader_ = shader;
}

void MD5SkinnedMesh::SetEyePos(const KlayGE::float3& eye_pos)
{
	eye_pos_ = eye_pos;
}

void MD5SkinnedMesh::BuildRenderable()
{
	if (!beBuilt_)
	{
		NormalsType normals(positions_.size());
		MathLib::compute_normal<float>(normals.begin(),
			indices_.begin(), indices_.end(), positions_.begin(), positions_.end());

		// 计算TBN
		tangents_.resize(positions_.size());
		binormals_.resize(positions_.size());
		MathLib::compute_tangent<float>(tangents_.begin(), binormals_.begin(),
			indices_.begin(), indices_.end(),
			positions_.begin(), positions_.end(),
			multi_tex_coords_[0].begin(), normals.begin());

		// 建立纹理
		std::string shader(shader_);

		if (shader.find_last_of("_d") == shader.length() - 1)
		{
			shader = shader.substr(0, shader.length() - 2);
			diffuse_map_->SetTexture(LoadTexture(shader + "_d.dds"));
			normal_map_->SetTexture(LoadTexture(shader + "_local.dds"));
			specular_map_->SetTexture(LoadTexture(shader + "_s.dds"));
		}
		else
		{
			diffuse_map_->SetTexture(LoadTexture(shader + ".dds"));
			normal_map_->SetTexture(TexturePtr());
			specular_map_->SetTexture(TexturePtr());
		}
	}

	SkinnedMesh::BuildRenderable();
}


MD5SkinnedModel::MD5SkinnedModel()
		: SkinnedModel(L"MD5SkinnedModel")
{
}

void MD5SkinnedModel::SetEyePos(const KlayGE::float3& eye_pos)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_pointer_cast<MD5SkinnedMesh>(*iter)->SetEyePos(eye_pos);
	}
}

void MD5SkinnedModel::SetTime(float time)
{
	this->SetFrame(static_cast<int>(time * 24));
}
