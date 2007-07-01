// Window.hpp
// KlayGE Window类 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.6.26)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOW_HPP
#define _WINDOW_HPP

#define KLAYGE_LIB_NAME KlayGE_AppLayer
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4103 4251 4275 4512)
#endif
#include <boost/signal.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <string>

namespace KlayGE
{
	class Window
	{
	public:
		Window(std::string const & name, int32_t left, int32_t top,
			uint32_t width, uint32_t height);
		~Window();

		void* WindowHandle() const
		{
			return wnd_handle_;
		}

	public:
		typedef boost::signal<void(Window const &, bool)> ActiveEvent;
		typedef boost::signal<void(Window const &)> PaintEvent;
		typedef boost::signal<void(Window const &)> EnterSizeMoveEvent;
		typedef boost::signal<void(Window const &)> ExitSizeMoveEvent;
		typedef boost::signal<void(Window const &, bool)> SizeEvent;
		typedef boost::signal<void(Window const &)> SetCursorEvent;
		typedef boost::signal<void(Window const &, wchar_t)> CharEvent;
		typedef boost::signal<void(Window const &, wchar_t)> KeyDownEvent;
		typedef boost::signal<void(Window const &, wchar_t)> KeyUpEvent;
		typedef boost::signal<void(Window const &)> CloseEvent;

		ActiveEvent& OnActive()
		{
			return active_event_;
		}
		PaintEvent& OnPaint()
		{
			return paint_event_;
		}
		EnterSizeMoveEvent& OnEnterSizeMove()
		{
			return enter_size_move_event_;
		}
		ExitSizeMoveEvent& OnExitSizeMove()
		{
			return exit_size_move_event_;
		}
		SizeEvent& OnSize()
		{
			return size_event_;
		}
		SetCursorEvent& OnSetCursor()
		{
			return set_cursor_event_;
		}
		CharEvent& OnChar()
		{
			return char_event_;
		}
		KeyDownEvent& OnKeyDown()
		{
			return key_down_event_;
		}
		KeyUpEvent& OnKeyUp()
		{
			return key_up_event_;
		}
		CloseEvent& OnClose()
		{
			return close_event_;
		}

	private:
		ActiveEvent active_event_;
		PaintEvent paint_event_;
		EnterSizeMoveEvent enter_size_move_event_;
		ExitSizeMoveEvent exit_size_move_event_;
		SizeEvent size_event_;
		SetCursorEvent set_cursor_event_;
		CharEvent char_event_;
		KeyDownEvent key_down_event_;
		KeyUpEvent key_up_event_;
		CloseEvent close_event_;

	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		void* wnd_handle_;
	};
}

#endif		// _WINDOW_HPP