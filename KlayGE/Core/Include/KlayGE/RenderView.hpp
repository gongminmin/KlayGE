// RenderView.hpp
// KlayGE 渲染视图类 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERVIEW_HPP
#define _RENDERVIEW_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Texture.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{	
	class RenderView
	{
	public:
		virtual ~RenderView()
		{
		}

		static RenderViewPtr NullObject();

		uint32_t Width() const
		{
			return width_;
		}
		uint32_t Height() const
		{
			return height_;
		}
		PixelFormat Format() const
		{
			return pf_;
		}
		uint32_t Bpp() const
		{
			return PixelFormatBits(pf_);
		}

		virtual void OnAttached(FrameBuffer& fb, uint32_t att) = 0;
		virtual void OnDetached(FrameBuffer& fb, uint32_t att) = 0;

		virtual void OnBind(FrameBuffer& fb, uint32_t att);
		virtual void OnUnbind(FrameBuffer& fb, uint32_t att);

	protected:
		uint32_t width_;
		uint32_t height_;
		PixelFormat pf_;
	};
}

#endif			// _RENDERVIEW_HPP
