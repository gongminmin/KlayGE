// RenderableHelper.cpp
// KlayGE 一些常用的可渲染对象 实现文件
// Ver 2.7.1
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.1
// 增加了RenderableHelper基类 (2005.7.10)
//
// 2.6.0
// 增加了RenderableSkyBox (2005.5.26)
//
// 2.5.0
// 增加了RenderablePoint，RenderableLine和RenderableTriangle (2005.4.13)
//
// 2.4.0
// 初次建立 (2005.3.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/App3D.hpp>

#include <boost/tuple/tuple.hpp>

#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	RenderableHelper::RenderableHelper(std::wstring const & name, bool can_be_culled, bool short_age)
		: name_(name),
			can_be_culled_(can_be_culled), short_age_(short_age)
	{
	}

	RenderEffectPtr RenderableHelper::GetRenderEffect() const
	{
		return effect_;
	}

	VertexBufferPtr RenderableHelper::GetVertexBuffer() const
	{
		return vb_;
	}

	Box RenderableHelper::GetBound() const
	{
		return box_;
	}

	bool RenderableHelper::CanBeCulled() const
	{
		return can_be_culled_;
	}
	
	bool RenderableHelper::ShortAge() const
	{
		return short_age_;
	}

	std::wstring const & RenderableHelper::Name() const
	{
		return name_;
	}


	RenderablePoint::RenderablePoint(Vector3 const & v, bool can_be_culled, bool short_age)
		: RenderableHelper(L"Point", can_be_culled, short_age)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("PointTec");

		vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_PointList);

		VertexStreamPtr vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VET_Positions, sizeof(float), 3)), true);
		vs->Assign(&v, 1);
		vb_->AddVertexStream(vs);

		box_ = MathLib::ComputeBoundingBox<float>(&v, &v + 1);
	}


	RenderableLine::RenderableLine(Vector3 const & v0, Vector3 const & v1, bool can_be_culled, bool short_age)
		: RenderableHelper(L"Line", can_be_culled, short_age)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("LineTec");

		Vector3 xyzs[] =
		{
			v0, v1
		};

		vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_LineList);

		VertexStreamPtr vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VET_Positions, sizeof(float), 3)), true);
		vs->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
		vb_->AddVertexStream(vs);

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}


	RenderableTriangle::RenderableTriangle(Vector3 const & v0, Vector3 const & v1, Vector3 const & v2,
											bool can_be_culled, bool short_age)
		: RenderableHelper(L"Triangle", can_be_culled, short_age)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("TriangleTec");

		Vector3 xyzs[] =
		{
			v0, v1, v2
		};

		vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_TriangleList);

		VertexStreamPtr vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VET_Positions, sizeof(float), 3)), true);
		vs->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
		vb_->AddVertexStream(vs);

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}


	RenderableBox::RenderableBox(Box const & box, bool can_be_culled, bool short_age)
		: RenderableHelper(L"Box", can_be_culled, short_age)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		box_ = box;

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("BoxTec");

		Vector3 xyzs[] =
		{
			box[0], box[1], box[2], box[3], box[4], box[5], box[6], box[7]
		};

		uint16_t indices[] =
		{
			0, 1, 2, 2, 3, 0,
			7, 6, 5, 5, 4, 7,
			4, 0, 3, 3, 7, 4,
			4, 5, 1, 1, 0, 4,
			1, 5, 6, 6, 2, 1,
			3, 2, 6, 6, 7, 3,
		};

		vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_TriangleList);

		VertexStreamPtr vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VET_Positions, sizeof(float), 3)), true);
		vs->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
		vb_->AddVertexStream(vs);
		
		vb_->SetIndexStream(rf.MakeIndexStream(true));
		vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(indices[0]));
	}


	RenderableSkyBox::RenderableSkyBox()
		: RenderableHelper(L"SkyBox", false, false),
			cube_sampler_(new Sampler)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		effect_ = rf.LoadEffect("RenderableHelper.fx");
		effect_->ActiveTechnique("SkyBoxTec");

		Vector3 xyzs[] =
		{
			Vector3(1.0f, 1.0f, 1.0f),
			Vector3(1.0f, -1.0f, 1.0f),
			Vector3(-1.0f, -1.0f, 1.0f),
			Vector3(-1.0f, 1.0f, 1.0f),
		};

		uint16_t indices[] =
		{
			0, 1, 2, 2, 3, 0,
		};

		vb_ = rf.MakeVertexBuffer(VertexBuffer::BT_TriangleList);

		VertexStreamPtr vs = rf.MakeVertexStream(boost::make_tuple(vertex_element(VET_Positions, sizeof(float), 3)), true);
		vs->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
		vb_->AddVertexStream(vs);

		vb_->SetIndexStream(rf.MakeIndexStream(true));
		vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(uint16_t));

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[4]);

		cube_sampler_->Filtering(Sampler::TFO_Bilinear);
		cube_sampler_->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Clamp);
		cube_sampler_->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Clamp);
		cube_sampler_->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
		*(effect_->ParameterByName("skybox_cubeMapSampler")) = cube_sampler_;
	}

	void RenderableSkyBox::CubeMap(TexturePtr const & cube)
	{
		cube_sampler_->SetTexture(cube);
	}

	void RenderableSkyBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		Matrix4 rot_view = camera.ViewMatrix();
		rot_view(3, 0) = 0;
		rot_view(3, 1) = 0;
		rot_view(3, 2) = 0;
		*(effect_->ParameterByName("inv_mvp")) = MathLib::Inverse(rot_view * camera.ProjMatrix());
	}
}
