// RenderWindow.hpp
// KlayGE 渲染窗口类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 增加了IsTexture (2005.3.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERWINDOW_HPP
#define _RENDERWINDOW_HPP

#include <KlayGE/RenderTarget.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class RenderWindow : public RenderTarget
	{
	public:
		RenderWindow();
		virtual ~RenderWindow();

		static RenderWindowPtr NullObject();

		virtual std::wstring const & Description() const = 0;

		virtual void Destroy() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Reposition(uint32_t left, uint32_t top) = 0;

		virtual bool Closed() const = 0;

		bool FullScreen() const;

		bool IsTexture() const
		{
			return false;
		}

	protected:
		bool isFullScreen_;
	};
}

#endif			// _RENDERWINDOW_HPP
