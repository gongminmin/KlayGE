// Window.hpp
// KlayGE Window类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2007-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 移入Core (2008.10.16)
//
// 3.7.0
// 实验性的linux支持 (2008.5.19)
//
// 3.6.0
// 初次建立 (2007.6.26)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOW_HPP
#define _WINDOW_HPP

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/RenderSettings.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4103 4251 4275 4512)
#endif
#include <boost/signal.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#if defined KLAYGE_PLATFORM_WINDOWS
#include <windows.h>
#elif defined KLAYGE_PLATFORM_LINUX
#include <glloader/glloader.h>
#ifdef Bool
#undef Bool		// for boost::foreach
#endif
#endif
#include <string>

namespace KlayGE
{
	enum MouseButtons
	{
		MB_None = 0,
		MB_Left = 1UL << 0,
		MB_Right = 1UL << 1,
		MB_Middle = 1UL << 2,
		MB_Shift = 1UL << 3,
		MB_Ctrl = 1UL << 4,
		MB_Alt = 1UL << 5
	};

	class KLAYGE_CORE_API Window
	{
	public:
		Window(std::string const & name, RenderSettings const & settings);
		~Window();

		void Recreate();

#if defined KLAYGE_PLATFORM_WINDOWS
		HWND HWnd() const
		{
			return wnd_;
		}
#elif defined KLAYGE_PLATFORM_LINUX
		::Display* XDisplay() const
		{
			return x_display_;
		}

		::Window XWindow() const
		{
			return x_window_;
		}

		::GLXContext XContext() const
		{
		    return x_context_;
		}

		::GLXFBConfig* GetFBC() const
		{
		    return fbc_;
		}
#endif

		int32_t Left() const
		{
			return left_;
		}
		int32_t Top() const
		{
			return top_;
		}

		uint32_t Width() const
		{
			return width_;
		}
		uint32_t Height() const
		{
			return height_;
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
		typedef boost::signal<void(Window const &, uint32_t, Vector_T<int32_t, 2> const &)> MouseDownEvent;
		typedef boost::signal<void(Window const &, uint32_t, Vector_T<int32_t, 2> const &)> MouseUpEvent;
		typedef boost::signal<void(Window const &, uint32_t, Vector_T<int32_t, 2> const &, int32_t)> MouseWheelEvent;
		typedef boost::signal<void(Window const &, uint32_t, Vector_T<int32_t, 2> const &)> MouseOverEvent;
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
		MouseDownEvent& OnMouseDown()
		{
			return mouse_down_event_;
		}
		MouseUpEvent& OnMouseUp()
		{
			return mouse_up_event_;
		}
		MouseWheelEvent& OnMouseWheel()
		{
			return mouse_wheel_event_;
		}
		MouseOverEvent& OnMouseOver()
		{
			return mouse_over_event_;
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
		MouseDownEvent mouse_down_event_;
		MouseUpEvent mouse_up_event_;
		MouseWheelEvent mouse_wheel_event_;
		MouseOverEvent mouse_over_event_;
		CloseEvent close_event_;

#if defined KLAYGE_PLATFORM_WINDOWS
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam);

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined KLAYGE_PLATFORM_LINUX
	public:
		void MsgProc(XEvent const & event);
#endif

	private:
		int32_t left_;
		int32_t top_;
		uint32_t width_;
		uint32_t height_;

#if defined KLAYGE_PLATFORM_WINDOWS
#ifdef KLAYGE_COMPILER_GCC
		std::string name_;
#else
		std::wstring wname_;
#endif

		HWND wnd_;
#elif defined KLAYGE_PLATFORM_LINUX
		::Display* x_display_;
		::Window x_window_;
		::GLXContext x_context_;
		::GLXFBConfig* fbc_;
		::Atom wm_delete_window_;
#endif
	};
}

#endif		// _WINDOW_HPP
