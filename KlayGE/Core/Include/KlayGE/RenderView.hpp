// RenderView.hpp
// KlayGE 渲染视图类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://www.klayge.org
//
// 3.7.0
// 增加了Clear (2007.8.23)
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERVIEW_HPP
#define _RENDERVIEW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ElementFormat.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderView : boost::noncopyable
	{
	public:
		virtual ~RenderView();

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
		uint32_t SampleCount() const
		{
			return sample_count_;
		}
		uint32_t SampleQuality() const
		{
			return sample_quality_;
		}

		virtual void ClearColor(Color const & clr) = 0;
		virtual void ClearDepth(float depth) = 0;
		virtual void ClearStencil(int32_t stencil) = 0;
		virtual void ClearDepthStencil(float depth, int32_t stencil) = 0;

		virtual void Discard() = 0;

		virtual void OnAttached(FrameBuffer& fb, uint32_t att) = 0;
		virtual void OnDetached(FrameBuffer& fb, uint32_t att) = 0;

	protected:
		uint32_t width_;
		uint32_t height_;
		ElementFormat pf_;
		uint32_t sample_count_;
		uint32_t sample_quality_;
	};

	class KLAYGE_CORE_API UnorderedAccessView : boost::noncopyable
	{
	public:
		UnorderedAccessView();
		virtual ~UnorderedAccessView();

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
