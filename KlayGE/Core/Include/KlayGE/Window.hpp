// Window.hpp
// KlayGE Window�� ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2007-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// ����Core (2008.10.16)
//
// 3.7.0
// ʵ���Ե�linux֧�� (2008.5.19)
//
// 3.6.0
// ���ν��� (2007.6.26)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOW_HPP
#define _WINDOW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Signal.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#elif defined KLAYGE_PLATFORM_WINDOWS_STORE
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#if KLAYGE_COMPILER_VERSION >= 142
#pragma warning(disable: 5205) // winrt::impl::implements_delegate doesn't have virtual destructor
#endif
#endif
#include <winrt/Windows.System.Display.h>
#include <winrt/Windows.UI.Core.h>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
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
	class KLAYGE_CORE_API Window final : boost::noncopyable
	{
	public:
		enum WindowRotation
		{
			WR_Unspecified,
			WR_Identity,
			WR_Rotate90,
			WR_Rotate180,
			WR_Rotate270
		};

	public:
		Window(std::string const& name, RenderSettings const& settings, void* native_wnd);
		~Window();

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND HWnd() const
		{
			return wnd_;
		}
#elif defined KLAYGE_PLATFORM_WINDOWS_STORE
		void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window);

		winrt::Windows::UI::Core::CoreWindow GetWindow() const
		{
			return wnd_;
		}

		void OnActivated();
		void OnSizeChanged(winrt::Windows::UI::Core::WindowSizeChangedEventArgs const& args);
		void OnVisibilityChanged(winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args);
		void OnClosed();
		void OnKeyDown(winrt::Windows::UI::Core::KeyEventArgs const& args);
		void OnKeyUp(winrt::Windows::UI::Core::KeyEventArgs const& args);
		void OnPointerPressed(winrt::Windows::UI::Core::PointerEventArgs const& args);
		void OnPointerReleased(winrt::Windows::UI::Core::PointerEventArgs const& args);
		void OnPointerMoved(winrt::Windows::UI::Core::PointerEventArgs const& args);
		void OnPointerWheelChanged(winrt::Windows::UI::Core::PointerEventArgs const& args);
		void OnDpiChanged();
		void OnOrientationChanged();
		void OnDisplayContentsInvalidated();

		bool FullScreen(bool fs);
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
		void CreateGLView(RenderSettings const& settings);
		void CreateGLESView();
		static void PumpEvents();
		void FlushBuffer();
		uint2 GetNSViewSize() const;
		void* NSView()
		{
			return ns_view_;
		}
#elif defined KLAYGE_PLATFORM_IOS
		static void PumpEvents();
		void CreateColorRenderBuffer(ElementFormat pf);
		void FlushBuffer();
		uint2 GetGLKViewSize() const;
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
		float EffectiveDPIScale() const
		{
			return effective_dpi_scale_;
		}

		WindowRotation Rotation() const
		{
			return win_rotation_;
		}

	public:
		typedef Signal::Signal<void(Window const& wnd, bool active)> ActiveEvent;
		typedef Signal::Signal<void(Window const& wnd)> PaintEvent;
		typedef Signal::Signal<void(Window const& wnd)> EnterSizeMoveEvent;
		typedef Signal::Signal<void(Window const& wnd)> ExitSizeMoveEvent;
		typedef Signal::Signal<void(Window const& wnd, bool active)> SizeEvent;
		typedef Signal::Signal<void(Window const& wnd)> SetCursorEvent;
		typedef Signal::Signal<void(Window const& wnd, wchar_t ch)> CharEvent;
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		typedef Signal::Signal<void(Window const& wnd, HRAWINPUT ri)> RawInputEvent;
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_LINUX) || \
	defined(KLAYGE_PLATFORM_DARWIN)
		typedef Signal::Signal<void(Window const& wnd, uint32_t key)> KeyDownEvent;
		typedef Signal::Signal<void(Window const& wnd, uint32_t key)> KeyUpEvent;
#if defined KLAYGE_PLATFORM_ANDROID
		typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t buttons)> MouseDownEvent;
		typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t buttons)> MouseUpEvent;
		typedef Signal::Signal<void(Window const& wnd, int2 const& pt)> MouseMoveEvent;
		typedef Signal::Signal<void(Window const& wnd, int2 const& pt, int32_t wheel_delta)> MouseWheelEvent;
		typedef Signal::Signal<void(Window const& wnd, int32_t axis, int32_t value)> JoystickAxisEvent;
		typedef Signal::Signal<void(Window const& wnd, uint32_t buttons)> JoystickButtonsEvent;
#endif
#endif
		typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t id)> PointerDownEvent;
		typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t id)> PointerUpEvent;
		typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t id, bool down)> PointerUpdateEvent;
		typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t id, int32_t wheel_delta)> PointerWheelEvent;

		typedef Signal::Signal<void(Window const& wnd)> CloseEvent;

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
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_LINUX) || \
	defined(KLAYGE_PLATFORM_DARWIN)
		KeyDownEvent& OnKeyDown()
		{
			return key_down_event_;
		}
		KeyUpEvent& OnKeyUp()
		{
			return key_up_event_;
		}
#if defined KLAYGE_PLATFORM_ANDROID
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
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_LINUX) || \
	defined(KLAYGE_PLATFORM_DARWIN)
		KeyDownEvent key_down_event_;
		KeyUpEvent key_up_event_;
#if defined KLAYGE_PLATFORM_ANDROID
		MouseDownEvent mouse_down_event_;
		MouseUpEvent mouse_up_event_;
		MouseMoveEvent mouse_move_event_;
		MouseWheelEvent mouse_wheel_event_;
		JoystickAxisEvent joystick_axis_event_;
		JoystickButtonsEvent joystick_buttons_event_;
#endif
#endif
		PointerDownEvent pointer_down_event_;
		PointerUpEvent pointer_up_event_;
		PointerUpdateEvent pointer_update_event_;
		PointerWheelEvent pointer_wheel_event_;
		CloseEvent close_event_;

	private:
		void UpdateDpiScale(float scale);

#if defined KLAYGE_PLATFORM_WINDOWS
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		static BOOL CALLBACK EnumMonProc(HMONITOR mon, HDC dc_mon, RECT* rc_mon, LPARAM lparam) noexcept;
#endif
		void KeepScreenOn();

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
		void DetectsOrientation();
#endif

		void DetectsDpi();
#elif defined KLAYGE_PLATFORM_LINUX
	public:
		void MsgProc(XEvent const& event);
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
		bool keep_screen_on_;

		float dpi_scale_;
		float effective_dpi_scale_;
		WindowRotation win_rotation_;

#if defined KLAYGE_PLATFORM_WINDOWS
		bool hide_;
		bool external_wnd_;
		std::wstring wname_;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		uint32_t win_style_;
		HWND wnd_;
		WNDPROC default_wnd_proc_;
#else
		winrt::Windows::UI::Core::CoreWindow wnd_{nullptr};
		winrt::Windows::System::Display::DisplayRequest disp_request_{ nullptr };
		std::array<uint32_t, 16> pointer_id_map_;
		bool full_screen_;
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
} // namespace KlayGE

#endif // _WINDOW_HPP
