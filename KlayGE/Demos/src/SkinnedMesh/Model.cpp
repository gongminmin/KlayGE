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
		world_(Matrix4::Identity()),
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
	Matrix4 worldview(world_ * app.ActiveCamera().ViewMatrix());
	*(technique_->Effect().ParameterByName("worldviewproj")) = worldview * app.ActiveCamera().ProjMatrix();

	*(technique_->Effect().ParameterByName("diffuse_map")) = diffuse_map_;
	*(technique_->Effect().ParameterByName("normal_map")) = normal_map_;
	*(technique_->Effect().ParameterByName("specular_map")) = specular_map_;

	*(technique_->Effect().ParameterByName("joint_rots")) = model_->GetBindRotations();
	*(technique_->Effect().ParameterByName("joint_poss")) = model_->GetBindPositions();

	*(technique_->Effect().ParameterByName("eye_pos")) = eye_pos_;
}

void MD5SkinnedMesh::SetWorld(const Matrix4& mat)
{
	world_ = mat;
}

void MD5SkinnedMesh::SetShaderName(std::string const & shader)
{
	shader_ = shader;
}

void MD5SkinnedMesh::SetEyePos(const KlayGE::Vector3& eye_pos)
{
	eye_pos_ = eye_pos;
}

void MD5SkinnedMesh::BuildRenderable()
{
	if (!beBuilt_)
	{
		// 计算TBN
		std::vector<Vector3> t(xyzs_.size());
		std::vector<Vector3> b(xyzs_.size());
		MathLib::ComputeTangent<float>(t.begin(), b.begin(),
			indices_.begin(), indices_.end(),
			xyzs_.begin(), xyzs_.end(),
			multi_tex_coords_[0].begin());
		GraphicsBufferPtr tan = Context::Instance().RenderFactoryInstance().MakeVertexBuffer(BU_Static);
		tan->Resize(static_cast<uint32_t>(t.size() * sizeof(t[0])));
		{
			GraphicsBuffer::Mapper mapper(*tan, BA_Write_Only);
			std::copy(t.begin(), t.end(), mapper.Pointer<Vector3>());
		}
		rl_->BindVertexStream(tan, boost::make_tuple(vertex_element(VEU_TextureCoord, 1, sizeof(float), 3)));
		GraphicsBufferPtr bn = Context::Instance().RenderFactoryInstance().MakeVertexBuffer(BU_Static);
		bn->Resize(static_cast<uint32_t>(b.size() * sizeof(b[0])));
		{
			GraphicsBuffer::Mapper mapper(*bn, BA_Write_Only);
			std::copy(b.begin(), b.end(), mapper.Pointer<Vector3>());
		}
		rl_->BindVertexStream(bn, boost::make_tuple(vertex_element(VEU_TextureCoord, 2, sizeof(float), 3)));

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

void MD5SkinnedModel::SetEyePos(const KlayGE::Vector3& eye_pos)
{
	for (StaticMeshesPtrType::iterator iter = meshes_.begin(); iter != meshes_.end(); ++ iter)
	{
		checked_cast<MD5SkinnedMesh*>(iter->get())->SetEyePos(eye_pos);
	}
}

void MD5SkinnedModel::SetTime(float time)
{
	this->SetFrame(static_cast<int>(time * 24));
}
