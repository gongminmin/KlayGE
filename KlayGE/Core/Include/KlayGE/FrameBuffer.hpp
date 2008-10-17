// FrameBuffer.hpp
// KlayGE 渲染到纹理类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
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

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

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
			ATT_Color3
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

		static FrameBufferPtr NullObject();

		virtual std::wstring const & Description() const = 0;

		virtual uint32_t Left() const;
		virtual uint32_t Top() const;
		virtual uint32_t Width() const;
		virtual uint32_t Height() const;
		virtual uint32_t ColorDepth() const;
		virtual uint32_t DepthBits() const;
		virtual uint32_t StencilBits() const;
		virtual ElementFormat Format() const;

		virtual Viewport const & GetViewport() const;
		virtual Viewport& GetViewport();
		virtual void SetViewport(Viewport const & viewport);

		virtual bool Active() const;
		virtual void Active(bool state);

		virtual bool RequiresFlipping() const = 0;

		void Attach(uint32_t att, RenderViewPtr view);
		void Detach(uint32_t att);
		RenderViewPtr Attached(uint32_t att);

		virtual void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil) = 0;

		virtual void OnBind();
		virtual void OnUnbind();

		virtual void SwapBuffers()
		{
		}

	protected:
		uint32_t	left_;
		uint32_t	top_;
		uint32_t	width_;
		uint32_t	height_;
		uint32_t	colorDepth_;
		ElementFormat format_;

		bool		isDepthBuffered_;
		uint32_t	depthBits_;
		uint32_t	stencilBits_;

		bool	active_;	// Is active i.e. visible

		Viewport viewport_;

		std::vector<RenderViewPtr> clr_views_;
		RenderViewPtr rs_view_;
	};
}

#endif			// _FRAMEBUFFER_HPP
