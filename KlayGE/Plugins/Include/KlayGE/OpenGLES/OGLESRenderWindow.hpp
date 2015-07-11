// OGLESRenderWindow.hpp
// KlayGE OpenGL ES��Ⱦ������ ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.2)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERWINDOW_HPP
#define _OGLESRENDERWINDOW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/OpenGLES/OGLESFrameBuffer.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4512) // boost::iterators::function_output_iterator<T>::output_proxy doesn't have assignment operator
#pragma warning(disable: 4913) // User defined binary operator ',' exists but no overload could convert all operands
#endif
#include <boost/signals2.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	struct RenderSettings;

	class OGLESRenderWindow : public OGLESFrameBuffer
	{
	public:
		OGLESRenderWindow(std::string const & name, RenderSettings const & settings);
		~OGLESRenderWindow();

		void Destroy();

		void SwapBuffers();

		std::wstring const & Description() const;

		void Resize(uint32_t width, uint32_t height);
		void Reposition(uint32_t left, uint32_t top);

		bool FullScreen() const;
		void FullScreen(bool fs);

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized(Window const & win);

	private:
		void OnPaint(Window const & win);
		void OnExitSizeMove(Window const & win);
		void OnSize(Window const & win, bool active);

	private:
		std::string	name_;

#if defined KLAYGE_PLATFORM_WINDOWS
		HWND	hWnd_;
#elif defined KLAYGE_PLATFORM_LINUX
		::Window x_window_;
#elif defined KLAYGE_PLATFORM_ANDROID
		::ANativeWindow* a_window_;
#endif

#ifndef KLAYGE_PLATFORM_IOS
		EGLDisplay display_;
		EGLSurface surf_;
		EGLConfig cfg_;
		EGLContext context_;
#endif

		bool	isFullScreen_;

		uint32_t color_bits_;

		std::wstring			description_;

		boost::signals2::connection on_paint_connect_;
		boost::signals2::connection on_exit_size_move_connect_;
		boost::signals2::connection on_size_connect_;
	};
}

#endif			// _OGLESRENDERWINDOW_HPP
