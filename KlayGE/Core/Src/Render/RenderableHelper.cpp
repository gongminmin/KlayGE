// RenderableHelper.cpp
// KlayGE 一些常用的可渲染对象 实现文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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
	RenderableBox::RenderableBox(Box const & box)
		: box_(box),
			vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
	{
		effect_ = LoadRenderEffect("Box.fx");
		effect_->SetTechnique("tec0");

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
		static std::wstring name(L"Box");
		return name;
	}

	Box RenderableBox::GetBound() const
	{
		return box_;
	}
}
