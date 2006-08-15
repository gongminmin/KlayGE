// RenderTarget.hpp
// KlayGE 渲染目标类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 增加了IsTexture和SwapBuffers (2005.3.6)
//
// 2.2.0
// 改用boost::timer计时 (2004.11.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERTARGET_HPP
#define _RENDERTARGET_HPP

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/Viewport.hpp>
#include <KlayGE/Timer.hpp>

namespace KlayGE
{
	class RenderTarget
	{
	public:
		RenderTarget();
		virtual ~RenderTarget();

		virtual uint32_t Left() const;
		virtual uint32_t Top() const;
		virtual uint32_t Width() const;
		virtual uint32_t Height() const;
		virtual uint32_t ColorDepth() const;
		virtual uint32_t DepthBits() const;
		virtual uint32_t StencilBits() const;

		virtual void Update();

		virtual Viewport const & GetViewport() const;
		virtual Viewport& GetViewport();
		virtual void SetViewport(Viewport const & viewport);

		virtual bool Active() const;
		virtual void Active(bool state);

		virtual void SwapBuffers() = 0;

		virtual void OnBind() = 0;
		virtual void OnUnbind() = 0;

	protected:
		uint32_t	left_;
		uint32_t	top_;
		uint32_t	width_;
		uint32_t	height_;
		uint32_t	colorDepth_;

		bool		isDepthBuffered_;
		uint32_t	depthBits_;
		uint32_t	stencilBits_;

		bool	active_;	// Is active i.e. visible

		Viewport viewport_;
	};
}

#endif			// _RENDERTARGET_HPP
