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

namespace KlayGE
{
	class KLAYGE_CORE_API FrameBuffer : boost::noncopyable
	{
	public:
		enum class Attachment
		{
			Color0 = 0,
			Color1,
			Color2,
			Color3,
			Color4,
			Color5,
			Color6,
			Color7
		};

		enum ClearBufferMask
		{
			CBM_Color   = 1UL << 0,
			CBM_Depth   = 1UL << 1,
			CBM_Stencil = 1UL << 2
		};

	public:
		FrameBuffer();
		virtual ~FrameBuffer() noexcept;

		static Attachment CalcAttachment(uint32_t index);

		virtual std::wstring const & Description() const = 0;

		uint32_t Left() const;
		uint32_t Top() const;
		uint32_t Width() const;
		uint32_t Height() const;

		ViewportPtr const & Viewport() const;
		ViewportPtr& Viewport();
		void Viewport(ViewportPtr const & viewport);

		void Attach(Attachment att, RenderTargetViewPtr const & view);
		void Detach(Attachment att);
		RenderTargetViewPtr const & AttachedRtv(Attachment att) const;

		void Attach(DepthStencilViewPtr const & view);
		void Detach();
		DepthStencilViewPtr const & AttachedDsv() const;

		void Attach(uint32_t index, UnorderedAccessViewPtr const & view);
		void Detach(uint32_t index);
		UnorderedAccessViewPtr const & AttachedUav(uint32_t index) const;

		virtual void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil) = 0;
		virtual void Discard(uint32_t flags) = 0;

		virtual void OnBind() = 0;
		virtual void OnUnbind() = 0;

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

		std::vector<RenderTargetViewPtr> rt_views_;
		DepthStencilViewPtr ds_view_;
		std::vector<UnorderedAccessViewPtr> ua_views_;
		bool views_dirty_;
	};
}

#endif			// _FRAMEBUFFER_HPP
