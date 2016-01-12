// Window.hpp
// KlayGE Window类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2007-2008
// Homepage: http://www.klayge.org
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

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/RenderSettings.hpp>

#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4512) // boost::iterators::function_output_iterator<T>::output_proxy doesn't have assignment operator
#pragma warning(disable: 4913) // User defined binary operator ',' exists but no overload could convert all operands
#elif defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter 'sp'
#endif
#include <boost/signals2.hpp>
#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(pop)
#elif defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#elif defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
#include <windows.ui.core.h>
#elif defined KLAYGE_PLATFORM_LINUX
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#elif defined KLAYGE_PLATFORM_ANDROID
#include <android_native_app_glue.h>
#elif (defined KLAYGE_PLATFORM_DARWIN) || (defined KLAYGE_PLATFORM_IOS)
#ifdef __OBJC__
#define OBJC_CLASS(name) @class name
#else
#define OBJC_CLASS(name) typedef struct objc_object name
#endif
OBJC_CLASS(KlayGEView);
OBJC_CLASS(KlayGEWindow);
OBJC_CLASS(KlayGEWindowListener);
OBJC_CLASS(NSView);
#endif

#include <string>

namespace KlayGE
{
	class KLAYGE_CORE_API Window
	{
	public:
		Window(std::string const & name, RenderSettings const & settings);
		Window(std::string const & name, RenderSettings const & settings, void* native_wnd);
		~Window();

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		void Recreate();

		HWND HWnd() const
		{
			return wnd_;
		}
#elif defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
		void SetWindow(std::shared_ptr<ABI::Windows::UI::Core::ICoreWindow> const & window);

		std::shared_ptr<ABI::Windows::UI::Core::ICoreWindow> GetWindow() const
		{
			return wnd_;
		}

		void DetectsDPI();
#elif defined KLAYGE_PLATFORM_LINUX
		::Display* XDisplay() const
		{
			return x_display_;
		}

		::XVisualInfo* VisualInfo() const
		{
			return vi_;
		}

		::Window XWindow() const
		{
			return x_window_;
		}
#elif defined KLAYGE_PLATFORM_ANDROID
		::ANativeWindow* AWindow() const
		{
			return a_window_;
		}
#elif defined KLAYGE_PLATFORM_DARWIN
		void BindListeners();
		void CreateGLView(RenderSettings const & settings);
		void CreateGLESView();
		static void PumpEvents();
		void FlushBuffer();
		uint2 GetNSViewSize();
		void* NSView()
		{
			return ns_view_;
		}
#elif defined KLAYGE_PLATFORM_IOS
		static void PumpEvents();
		void CreateColorRenderBuffer(ElementFormat pf);
		void FlushBuffer();
		uint2 GetGLKViewSize();
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

		bool Active() const
		{
			return active_;
		}
		void Active(bool active)
		{
			active_ = active;
		}
		bool Ready() const
		{
			return ready_;
		}
		void Ready(bool ready)
		{
			ready_ = ready;
		}
		bool Closed() const
		{
			return closed_;
		}
		void Closed(bool closed)
		{
			closed_ = closed;
		}

		float DPIScale() const
		{
			return dpi_scale_;
		}

	public:
		typedef boost::signals2::signal<void(Window const & wnd, bool active)> ActiveEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> PaintEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> EnterSizeMoveEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> ExitSizeMoveEvent;
		typedef boost::signals2::signal<void(Window const & wnd, bool active)> SizeEvent;
		typedef boost::signals2::signal<void(Window const & wnd)> SetCursorEvent;
		typedef boost::signals2::signal<void(Window const & wnd, wchar_t ch)> CharEvent;
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		typedef boost::signals2::signal<void(Window const & wnd, HRAWINPUT ri)> RawInputEvent;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
		typedef boost::signals2::signal<void(Window const & wnd, HTOUCHINPUT hti, uint32_t num_inputs)> TouchEvent;
#endif
#endif
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t id)> PointerDownEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t id)> PointerUpEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t id, bool down)> PointerUpdateEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t id, int32_t wheel_delta)> PointerWheelEvent;
#if defined KLAYGE_PLATFORM_ANDROID
		typedef boost::signals2::signal<void(Window const & wnd, uint32_t key)> KeyDownEvent;
		typedef boost::signals2::signal<void(Window const & wnd, uint32_t key)> KeyUpEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t buttons)> MouseDownEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, uint32_t buttons)> MouseUpEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt)> MouseMoveEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int2 const & pt, int32_t wheel_delta)> MouseWheelEvent;
		typedef boost::signals2::signal<void(Window const & wnd, int32_t axis, int32_t value)> JoystickAxisEvent;
		typedef boost::signals2::signal<void(Window const & wnd, uint32_t buttons)> JoystickButtonsEvent;
#elif defined KLAYGE_PLATFORM_LINUX
		typedef boost::signals2::signal<void(Window const & wnd, uint32_t key)> KeyDownEvent;
		typedef boost::signals2::signal<void(Window const & wnd, uint32_t key)> KeyUpEvent;
#elif defined KLAYGE_PLATFORM_DARWIN
		typedef boost::signals2::signal<void(Window const & wnd, uint32_t key)> KeyDownEvent;
		typedef boost::signals2::signal<void(Window const & wnd, uint32_t key)> KeyUpEvent;
#endif
		typedef boost::signals2::signal<void(Window const & wnd)> CloseEvent;

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
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		RawInputEvent& OnRawInput()
		{
			return raw_input_event_;
		}
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
		TouchEvent& OnTouch()
		{
			return touch_event_;
		}
#endif
#endif
		PointerDownEvent& OnPointerDown()
		{
			return pointer_down_event_;
		}
		PointerUpEvent& OnPointerUp()
		{
			return pointer_up_event_;
		}
		PointerUpdateEvent& OnPointerUpdate()
		{
			return pointer_update_event_;
		}
		PointerWheelEvent& OnPointerWheel()
		{
			return pointer_wheel_event_;
		}
#if defined KLAYGE_PLATFORM_ANDROID
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
		MouseMoveEvent& OnMouseMove()
		{
			return mouse_move_event_;
		}
		MouseWheelEvent& OnMouseWheel()
		{
			return mouse_wheel_event_;
		}
		JoystickAxisEvent& OnJoystickAxis()
		{
			return joystick_axis_event_;
		}
		JoystickButtonsEvent& OnJoystickButtons()
		{
			return joystick_buttons_event_;
		}
#elif defined KLAYGE_PLATFORM_LINUX
		KeyDownEvent& OnKeyDown()
		{
			return key_down_event_;
		}
		KeyUpEvent& OnKeyUp()
		{
			return key_up_event_;
		}
#elif defined KLAYGE_PLATFORM_DARWIN
		KeyDownEvent& OnKeyDown()
		{
			return key_down_event_;
		}
		KeyUpEvent& OnKeyUp()
		{
			return key_up_event_;
		}
#endif
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
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		RawInputEvent raw_input_event_;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
		TouchEvent touch_event_;
#endif
#endif
		PointerDownEvent pointer_down_event_;
		PointerUpEvent pointer_up_event_;
		PointerUpdateEvent pointer_update_event_;
		PointerWheelEvent pointer_wheel_event_;
#if defined KLAYGE_PLATFORM_ANDROID
		KeyDownEvent key_down_event_;
		KeyUpEvent key_up_event_;
		MouseDownEvent mouse_down_event_;
		MouseUpEvent mouse_up_event_;
		MouseMoveEvent mouse_move_event_;
		MouseWheelEvent mouse_wheel_event_;
		JoystickAxisEvent joystick_axis_event_;
		JoystickButtonsEvent joystick_buttons_event_;
#elif defined KLAYGE_PLATFORM_LINUX
		KeyDownEvent key_down_event_;
		KeyUpEvent key_up_event_;
#elif defined KLAYGE_PLATFORM_DARWIN
		KeyDownEvent key_down_event_;
		KeyUpEvent key_up_event_;
#endif
		CloseEvent close_event_;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		static BOOL CALLBACK EnumMonProc(HMONITOR mon, HDC dc_mon, RECT* rc_mon, LPARAM lparam);
#endif

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void DetectsDPI();
#elif defined KLAYGE_PLATFORM_LINUX
	public:
		void MsgProc(XEvent const & event);
#elif defined KLAYGE_PLATFORM_ANDROID
	public:
		static void HandleCMD(android_app* app, int32_t cmd);
		static int32_t HandleInput(android_app* app, AInputEvent* event);
#endif

	private:
		int32_t left_;
		int32_t top_;
		uint32_t width_;
		uint32_t height_;

		bool active_;
		bool ready_;
		bool closed_;

		float dpi_scale_;

#if defined KLAYGE_PLATFORM_WINDOWS
		bool hide_;
		bool external_wnd_;
		std::wstring wname_;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND wnd_;
		WNDPROC default_wnd_proc_;
#else
		std::shared_ptr<ABI::Windows::UI::Core::ICoreWindow> wnd_;
#endif
#elif defined KLAYGE_PLATFORM_LINUX
		::Display* x_display_;
		::XVisualInfo* vi_;
		::Window x_window_;
		::Atom wm_delete_window_;
#elif defined KLAYGE_PLATFORM_ANDROID
		::ANativeWindow* a_window_;
#elif defined KLAYGE_PLATFORM_DARWIN
		KlayGEWindow* ns_window_;
		KlayGEWindowListener* ns_window_listener_;
		::NSView* ns_view_;
#elif defined KLAYGE_PLATFORM_IOS
		KlayGEView* eagl_view_;
#endif
	};
}

#endif		// _WINDOW_HPP
