// FrameBuffer.hpp
// KlayGE 渲染到纹理类 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// Color buffer增加至8个 (2009.4.9)
//
// 3.7.0
// 增加了Clear (2008.1.9)
//
// 3.3.0
// 改为FrameBuffer (2006.5.30)
//
// 2.8.0
// 去掉了GetTexture (2005.7.19)
//
// 2.4.0
// 增加了IsTexture和SwapBuffers (2005.3.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _FRAMEBUFFER_HPP
#define _FRAMEBUFFER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <vector>

#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderView.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API FrameBuffer
	{
	public:
		enum Attachment
		{
			ATT_DepthStencil,
			ATT_Color0,
			ATT_Color1,
			ATT_Color2,
			ATT_Color3,
			ATT_Color4,
			ATT_Color5,
			ATT_Color6,
			ATT_Color7
		};

		enum ClearBufferMask
		{
			CBM_Color   = 1UL << 0,
			CBM_Depth   = 1UL << 1,
			CBM_Stencil = 1UL << 2
		};

	public:
		FrameBuffer();
		virtual ~FrameBuffer();

		virtual std::wstring const & Description() const = 0;

		uint32_t Left() const;
		uint32_t Top() const;
		uint32_t Width() const;
		uint32_t Height() const;

		ViewportPtr const & GetViewport() const;
		ViewportPtr& GetViewport();
		void SetViewport(ViewportPtr const & viewport);

		void Attach(uint32_t att, RenderViewPtr const & view);
		void Detach(uint32_t att);
		RenderViewPtr Attached(uint32_t att) const;

		void AttachUAV(uint32_t att, UnorderedAccessViewPtr const & view);
		void DetachUAV(uint32_t att);
		UnorderedAccessViewPtr AttachedUAV(uint32_t att) const;

		virtual void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil) = 0;
		virtual void Discard(uint32_t flags) = 0;

		virtual void OnBind();
		virtual void OnUnbind();

		virtual void SwapBuffers()
		{
		}
		virtual void WaitOnSwapBuffers()
		{
		}

		bool Dirty() const
		{
			return views_dirty_;
		}

	protected:
		uint32_t	left_;
		uint32_t	top_;
		uint32_t	width_;
		uint32_t	height_;

		ViewportPtr viewport_;

		std::vector<RenderViewPtr> clr_views_;
		RenderViewPtr rs_view_;
		std::vector<UnorderedAccessViewPtr> ua_views_;
		bool views_dirty_;
	};
}

#endif			// _FRAMEBUFFER_HPP
