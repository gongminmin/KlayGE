// OGLES2RenderWindow.hpp
// KlayGE OpenGL ES 2渲染窗口类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2RENDERWINDOW_HPP
#define _OGLES2RENDERWINDOW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/OpenGLES2/OGLES2FrameBuffer.hpp>

namespace KlayGE
{
	struct RenderSettings;

	class OGLES2RenderWindow : public OGLES2FrameBuffer
	{
	public:
		OGLES2RenderWindow(std::string const & name, RenderSettings const & settings);
		~OGLES2RenderWindow();

		void Destroy();

		bool Closed() const;

		bool Ready() const;
		void Ready(bool ready);

		void SwapBuffers();

		std::wstring const & Description() const;

		void Resize(uint32_t width, uint32_t height);
		void Reposition(uint32_t left, uint32_t top);

		bool FullScreen() const;
		void FullScreen(bool fs);

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

		bool RequiresFlipping() const
		{
			return false;
		}

	private:
		void OnActive(Window const & win, bool active);
		void OnPaint(Window const & win);
		void OnEnterSizeMove(Window const & win);
		void OnExitSizeMove(Window const & win);
		void OnSize(Window const & win, bool active);
		void OnClose(Window const & win);

	private:
		std::string	name_;

#if defined KLAYGE_PLATFORM_WINDOWS
		HWND	hWnd_;
		HDC		hDC_;
#elif defined KLAYGE_PLATFORM_LINUX
		::Window x_window_;
#endif

		EGLDisplay display_;
		EGLSurface surf_;
		EGLConfig cfg_;
		EGLContext context_;

		bool	ready_;				// Is ready i.e. available for update
		bool	closed_;
		bool	isFullScreen_;
		
		uint32_t color_bits_;

		std::wstring			description_;
	};
}

#endif			// _OGLES2RENDERWINDOW_HPP
