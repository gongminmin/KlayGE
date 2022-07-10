// FrameBuffer.hpp
// KlayGE ��Ⱦ�������� ͷ�ļ�
// Ver 3.9.0
// ��Ȩ����(C) ������, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// Color buffer������8�� (2009.4.9)
//
// 3.7.0
// ������Clear (2008.1.9)
//
// 3.3.0
// ��ΪFrameBuffer (2006.5.30)
//
// 2.8.0
// ȥ����GetTexture (2005.7.19)
//
// 2.4.0
// ������IsTexture��SwapBuffers (2005.3.6)
//
// �޸ļ�¼
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
