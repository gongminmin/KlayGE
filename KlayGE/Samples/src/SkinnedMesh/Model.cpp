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
#include <KlayGE/Camera.hpp>

#include "Model.hpp"

using namespace KlayGE;


MD5SkinnedMesh::MD5SkinnedMesh(RenderModelPtr model, std::wstring const & /*name*/)
	: SkinnedMesh(model, L"MD5SkinnedMesh"),
		world_(float4x4::Identity())
{
	technique_ = Context::Instance().RenderFactoryInstance().LoadEffect("SkinnedMesh.kfx")->TechniqueByName("SkinnedMeshTech");
}

void MD5SkinnedMesh::BuildMeshInfo()
{
	std::vector<float3> positions(this->NumVertices());
	std::vector<float2> texcoords(this->NumVertices());
	for (uint32_t i = 0; i < rl_->NumVertexStreams(); ++ i)
	{
		GraphicsBufferPtr vb = rl_->GetVertexStream(i);
		switch (rl_->VertexStreamFormat(i)[0].usage)
		{
		case VEU_Position:
			{
				GraphicsBuffer::Mapper mapper(*vb, BA_Read_Only);
				std::copy(mapper.Pointer<float3>(), mapper.Pointer<float3>() + positions.size(), positions.begin());
			}
			break;

		case VEU_TextureCoord:
			{
				GraphicsBuffer::Mapper mapper(*vb, BA_Read_Only);
				std::copy(mapper.Pointer<float2>(), mapper.Pointer<float2>() + texcoords.size(), texcoords.begin());
			}
			break;

		default:
			break;
		}
	}
	std::vector<uint16_t> indices(this->NumTriangles() * 3);
	{
		GraphicsBuffer::Mapper mapper(*rl_->GetIndexStream(), BA_Read_Only);
		std::copy(mapper.Pointer<uint16_t>(), mapper.Pointer<uint16_t>() + indices.size(), indices.begin());
	}

	std::vector<float3> normals(this->NumVertices());
	MathLib::compute_normal<float>(normals.begin(),
		indices.begin(), indices.end(), positions.begin(), positions.end());

	// 计算TBN
	std::vector<float3> tangents(this->NumVertices());
	std::vector<float3> binormals(this->NumVertices());
	MathLib::compute_tangent<float>(tangents.begin(), binormals.begin(),
		indices.begin(), indices.end(),
		positions.begin(), positions.end(),
		texcoords.begin(), normals.begin());
	this->AddVertexStream(&tangents[0], static_cast<uint32_t>(sizeof(tangents[0]) * tangents.size()),
			vertex_element(VEU_Tangent, 0, EF_BGR32F));
	this->AddVertexStream(&binormals[0], static_cast<uint32_t>(sizeof(binormals[0]) * binormals.size()),
			vertex_element(VEU_Binormal, 0, EF_BGR32F));

	// 建立纹理
	for (StaticMesh::TextureSlotsType::iterator iter = texture_slots_.begin();
		iter != texture_slots_.end(); ++ iter)
	{
		if ("DiffuseMap" == iter->first)
		{
			*(technique_->Effect().ParameterByName("diffuse_map")) = LoadTexture(iter->second);
		}
		if ("NormalMap" == iter->first)
		{
			*(technique_->Effect().ParameterByName("normal_map")) = LoadTexture(iter->second);
		}
		if ("SpecularMap" == iter->first)
		{
			*(technique_->Effect().ParameterByName("specular_map")) = LoadTexture(iter->second);
		}
	}
}

void MD5SkinnedMesh::OnRenderBegin()
{
	App3DFramework& app = Context::Instance().AppInstance();
	float4x4 worldview(world_ * app.ActiveCamera().ViewMatrix());
	*(technique_->Effect().ParameterByName("worldviewproj")) = worldview * app.ActiveCamera().ProjMatrix();

	boost::shared_ptr<RenderModel> model = model_.lock();
	if (model)
	{
		*(technique_->Effect().ParameterByName("joint_rots")) = checked_pointer_cast<MD5SkinnedModel>(model)->GetBindRotations();
		*(technique_->Effect().ParameterByName("joint_poss")) = checked_pointer_cast<MD5SkinnedModel>(model)->GetBindPositions();
	}
}

void MD5SkinnedMesh::SetWorld(const float4x4& mat)
{
	world_ = mat;
}

void MD5SkinnedMesh::SetEyePos(const KlayGE::float3& eye_pos)
{
	*(technique_->Effect().ParameterByName("eye_pos")) = eye_pos;
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
	this->SetFrame(static_cast<int>(time * frame_rate_));
}
