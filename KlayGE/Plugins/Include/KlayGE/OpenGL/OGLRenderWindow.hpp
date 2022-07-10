// OGLRenderWindow.hpp
// KlayGE OpenGL��Ⱦ������ ͷ�ļ�
// Ver 3.7.0
// ��Ȩ����(C) ������, 2004-2008
// Homepage: http://www.klayge.org
//
// 3.7.0
// ʵ���Ե�linux֧�� (2008.5.19)
//
// 3.6.0
// ֧�ֶ�̬�л�ȫ��/����ģʽ (2007.3.24)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDERWINDOW_HPP
#define _OGLRENDERWINDOW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Signal.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>

namespace KlayGE
{
	struct RenderSettings;

	class OGLRenderWindow final : public OGLFrameBuffer
	{
	public:
		OGLRenderWindow(std::string const & name, RenderSettings const & settings);
		~OGLRenderWindow() override;

		void Destroy();

		void SwapBuffers() override;

		std::wstring const & Description() const override;

		void Resize(uint32_t width, uint32_t height);
		void Reposition(uint32_t left, uint32_t top);

		bool FullScreen() const;
		void FullScreen(bool fs);

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized(Window const& win);

	private:
		void OnExitSizeMove(Window const& win);
		void OnSize(Window const& win, bool active);

	private:
		std::string	name_;

#if defined KLAYGE_PLATFORM_WINDOWS
		HWND	hWnd_;
		HGLRC	hRC_;
		HDC		hDC_;
#elif defined KLAYGE_PLATFORM_LINUX
		::Display* x_display_;
		::Window x_window_;
		::GLXContext x_context_;
#endif

		bool	isFullScreen_;

		uint32_t color_bits_;

		std::wstring			description_;

		Signal::Connection on_exit_size_move_connect_;
		Signal::Connection on_size_connect_;
	};
}

#endif			// _OGLRENDERWINDOW_HPP
