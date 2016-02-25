// RenderView.hpp
// KlayGE ��Ⱦ��ͼ�� ͷ�ļ�
// Ver 3.7.0
// ��Ȩ����(C) ������, 2006-2007
// Homepage: http://www.klayge.org
//
// 3.7.0
// ������Clear (2007.8.23)
//
// 3.3.0
// ���ν��� (2006.5.31)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERVIEW_HPP
#define _RENDERVIEW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ElementFormat.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderView
	{
	public:
		virtual ~RenderView()
		{
		}

		uint32_t Width() const
		{
			return width_;
		}
		uint32_t Height() const
		{
			return height_;
		}
		ElementFormat Format() const
		{
			return pf_;
		}

		virtual void ClearColor(Color const & clr) = 0;
		virtual void ClearDepth(float depth) = 0;
		virtual void ClearStencil(int32_t stencil) = 0;
		virtual void ClearDepthStencil(float depth, int32_t stencil) = 0;

		virtual void Discard() = 0;

		virtual void OnAttached(FrameBuffer& fb, uint32_t att) = 0;
		virtual void OnDetached(FrameBuffer& fb, uint32_t att) = 0;

		virtual void OnBind(FrameBuffer& fb, uint32_t att);
		virtual void OnUnbind(FrameBuffer& fb, uint32_t att);

	protected:
		uint32_t width_;
		uint32_t height_;
		ElementFormat pf_;
	};

	class KLAYGE_CORE_API UnorderedAccessView
	{
	public:
		UnorderedAccessView()
			: init_count_(0)
		{
		}
		virtual ~UnorderedAccessView()
		{
		}

		uint32_t Width() const
		{
			return width_;
		}
		uint32_t Height() const
		{
			return height_;
		}
		ElementFormat Format() const
		{
			return pf_;
		}

		virtual void Clear(float4 const & val) = 0;
		virtual void Clear(uint4 const & val) = 0;

		virtual void Discard() = 0;

		virtual void OnAttached(FrameBuffer& fb, uint32_t att) = 0;
		virtual void OnDetached(FrameBuffer& fb, uint32_t att) = 0;

		virtual void OnBind(FrameBuffer& fb, uint32_t att);
		virtual void OnUnbind(FrameBuffer& fb, uint32_t att);

		void InitCount(uint32_t count)
		{
			init_count_ = count;
		}
		uint32_t InitCount() const
		{
			return init_count_;
		}

	protected:
		uint32_t width_;
		uint32_t height_;
		ElementFormat pf_;

		uint32_t init_count_;
	};
}

#endif			// _RENDERVIEW_HPP
