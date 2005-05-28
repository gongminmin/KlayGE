// RenderableHelper.cpp
// KlayGE 一些常用的可渲染对象 实现文件
// Ver 2.5.1
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.5.1
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

#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	RenderablePoint::RenderablePoint(Vector3 const & v)
		: vb_(new VertexBuffer(VertexBuffer::BT_PointList))
	{
		effect_ = LoadRenderEffect("RenderableHelper.fx");
		effect_->SetTechnique("PointTec");

		vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
		vb_->GetVertexStream(VST_Positions)->Assign(&v, 1);

		box_ = MathLib::ComputeBoundingBox<float>(&v, &v + 1);
	}

	RenderEffectPtr RenderablePoint::GetRenderEffect() const
	{
		return effect_;
	}

	VertexBufferPtr RenderablePoint::GetVertexBuffer() const
	{
		return vb_;
	}

	std::wstring const & RenderablePoint::Name() const
	{
		static std::wstring const name(L"Point");
		return name;
	}

	Box RenderablePoint::GetBound() const
	{
		return box_;
	}


	RenderableLine::RenderableLine(Vector3 const & v0, Vector3 const & v1)
		: vb_(new VertexBuffer(VertexBuffer::BT_LineList))
	{
		effect_ = LoadRenderEffect("RenderableHelper.fx");
		effect_->SetTechnique("LineTec");

		Vector3 xyzs[] =
		{
			v0, v1
		};

		vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
		vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}

	RenderEffectPtr RenderableLine::GetRenderEffect() const
	{
		return effect_;
	}

	VertexBufferPtr RenderableLine::GetVertexBuffer() const
	{
		return vb_;
	}

	std::wstring const & RenderableLine::Name() const
	{
		static std::wstring const name(L"Line");
		return name;
	}

	Box RenderableLine::GetBound() const
	{
		return box_;
	}


	RenderableTriangle::RenderableTriangle(Vector3 const & v0, Vector3 const & v1, Vector3 const & v2)
		: vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
	{
		effect_ = LoadRenderEffect("RenderableHelper.fx");
		effect_->SetTechnique("TriangleTec");

		Vector3 xyzs[] =
		{
			v0, v1, v2
		};

		vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
		vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));
	}

	RenderEffectPtr RenderableTriangle::GetRenderEffect() const
	{
		return effect_;
	}

	VertexBufferPtr RenderableTriangle::GetVertexBuffer() const
	{
		return vb_;
	}

	std::wstring const & RenderableTriangle::Name() const
	{
		static std::wstring const name(L"Triangle");
		return name;
	}

	Box RenderableTriangle::GetBound() const
	{
		return box_;
	}


	RenderableBox::RenderableBox(Box const & box)
		: box_(box),
			vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
	{
		effect_ = LoadRenderEffect("RenderableHelper.fx");
		effect_->SetTechnique("BoxTec");

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

		vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
		vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
		
		vb_->AddIndexStream();
		vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(indices[0]));
	}

	RenderEffectPtr RenderableBox::GetRenderEffect() const
	{
		return effect_;
	}

	VertexBufferPtr RenderableBox::GetVertexBuffer() const
	{
		return vb_;
	}

	std::wstring const & RenderableBox::Name() const
	{
		static std::wstring const name(L"Box");
		return name;
	}

	Box RenderableBox::GetBound() const
	{
		return box_;
	}


	RenderableSkyBox::RenderableSkyBox()
		: vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
	{
		effect_ = LoadRenderEffect("RenderableHelper.fx");
		effect_->SetTechnique("SkyBoxTec");

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

		box_ = MathLib::ComputeBoundingBox<float>(&xyzs[0], &xyzs[4]);

		vb_->AddVertexStream(VST_Positions, sizeof(float), 3);
		vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));

		vb_->AddIndexStream();
		vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(uint16_t));
	}

	void RenderableSkyBox::CubeMap(TexturePtr const & cube)
	{
		*(effect_->ParameterByName("skybox_cubemap")) = cube;
	}

	void RenderableSkyBox::MVPMatrix(Matrix4 const & mvp)
	{
		inv_mvp_ = MathLib::Inverse(mvp);
	}

	void RenderableSkyBox::OnRenderBegin()
	{
		*(effect_->ParameterByName("inv_mvp")) = inv_mvp_;
	}

	bool RenderableSkyBox::CanBeCulled() const
	{
		return false;
	}

	RenderEffectPtr RenderableSkyBox::GetRenderEffect() const
	{
		return effect_;
	}

	VertexBufferPtr RenderableSkyBox::GetVertexBuffer() const
	{
		return vb_;
	}

	Box RenderableSkyBox::GetBound() const
	{
		return box_;
	}

	std::wstring const & RenderableSkyBox::Name() const
	{
		static std::wstring const name(L"SkyBox");
		return name;
	}
}
