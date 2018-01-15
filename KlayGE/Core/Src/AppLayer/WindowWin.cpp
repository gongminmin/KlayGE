/**
 * @file WindowWin.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>

#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#include <VersionHelpers.h>
#include <ShellScalingAPI.h>
#endif
#include <windowsx.h>

#ifndef GET_KEYSTATE_WPARAM
#define GET_KEYSTATE_WPARAM(wParam) (LOWORD(wParam))
#endif

namespace KlayGE
{
	LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Window* win = reinterpret_cast<Window*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		if (win != nullptr)
		{
			return win->MsgProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
	BOOL Window::EnumMonProc(HMONITOR mon, HDC dc_mon, RECT* rc_mon, LPARAM lparam)
	{
		KFL_UNUSED(dc_mon);
		KFL_UNUSED(rc_mon);

		HMODULE shcore = ::LoadLibraryEx(TEXT("SHCore.dll"), nullptr, 0);
		if (shcore)
		{
			typedef HRESULT (CALLBACK *GetDpiForMonitorFunc)(HMONITOR mon, MONITOR_DPI_TYPE dpi_type, UINT* dpi_x, UINT* dpi_y);
			GetDpiForMonitorFunc DynamicGetDpiForMonitor
				= reinterpret_cast<GetDpiForMonitorFunc>(::GetProcAddress(shcore, "GetDpiForMonitor"));
			if (DynamicGetDpiForMonitor)
			{
				UINT dpi_x, dpi_y;
				if (S_OK == DynamicGetDpiForMonitor(mon, MDT_DEFAULT, &dpi_x, &dpi_y))
				{
					Window* win = reinterpret_cast<Window*>(lparam);
					win->UpdateDpiScale(std::max(win->dpi_scale_, static_cast<float>(std::max(dpi_x, dpi_y)) / USER_DEFAULT_SCREEN_DPI));
				}
			}

			::FreeLibrary(shcore);
		}

		return TRUE;
	}
#endif

	Window::Window(std::string const & name, RenderSettings const & settings)
		: active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on),
			dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity), hide_(settings.hide_win)
	{
		this->DetectsDpi();
		this->KeepScreenOn();

		HINSTANCE hInst = ::GetModuleHandle(nullptr);

		// Register the window class
		Convert(wname_, name);
		WNDCLASSEXW wc;
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(this);
		wc.hInstance = hInst;
		wc.hIcon = nullptr;
		wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = wname_.c_str();
		wc.hIconSm = nullptr;
		::RegisterClassExW(&wc);

		if (settings.full_screen)
		{
			win_style_ = WS_POPUP;
		}
		else
		{
			win_style_ = WS_OVERLAPPEDWINDOW;
		}

		RECT rc = { 0, 0, static_cast<LONG>(settings.width * dpi_scale_ + 0.5f), static_cast<LONG>(settings.height * dpi_scale_ + 0.5f) };
		::AdjustWindowRect(&rc, win_style_, false);

		// Create our main window
		// Pass pointer to self
		wnd_ = ::CreateWindowW(wname_.c_str(), wname_.c_str(), win_style_, settings.left, settings.top,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, nullptr);

		default_wnd_proc_ = ::DefWindowProc;
		external_wnd_ = false;

		::GetClientRect(wnd_, &rc);
		left_ = rc.left;
		top_ = rc.top;
		width_ = rc.right - rc.left;
		height_ = rc.bottom - rc.top;

		::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		::ShowWindow(wnd_, hide_ ? SW_HIDE : SW_SHOWNORMAL);
		::UpdateWindow(wnd_);

		ready_ = true;
	}

	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on),
			dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity), hide_(settings.hide_win)
	{
		this->DetectsDpi();
		this->KeepScreenOn();

		Convert(wname_, name);

		wnd_ = static_cast<HWND>(native_wnd);
		default_wnd_proc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtrW(wnd_, GWLP_WNDPROC));
		::SetWindowLongPtrW(wnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
		external_wnd_ = true;

		RECT rc;
		::GetClientRect(wnd_, &rc);
		left_ = rc.left;
		top_ = rc.top;
		width_ = rc.right - rc.left;
		height_ = rc.bottom - rc.top;

		::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		::UpdateWindow(wnd_);

		ready_ = true;
	}

	Window::~Window()
	{
		if (keep_screen_on_)
		{
#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
			::SetThreadExecutionState(ES_CONTINUOUS);
#endif
		}

		if (wnd_ != nullptr)
		{
			::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
			if (!external_wnd_)
			{
				::DestroyWindow(wnd_);
			}

			wnd_ = nullptr;
		}
	}

	LRESULT Window::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:
			active_ = (WA_INACTIVE != LOWORD(wParam));
			this->OnActive()(*this, active_);
			break;

		case WM_ERASEBKGND:
			return 1;

		case WM_PAINT:
			this->OnPaint()(*this);
			break;

		case WM_ENTERSIZEMOVE:
			// Previent rendering while moving / sizing
			ready_ = false;
			this->OnEnterSizeMove()(*this);
			break;

		case WM_EXITSIZEMOVE:
			this->OnExitSizeMove()(*this);
			ready_ = true;
			break;

		case WM_SIZE:
			{
				RECT rc;
				::GetClientRect(wnd_, &rc);
				left_ = rc.left;
				top_ = rc.top;
				width_ = rc.right - rc.left;
				height_ = rc.bottom - rc.top;

				// Check to see if we are losing or gaining our window.  Set the
				// active flag to match
				if ((SIZE_MAXHIDE == wParam) || (SIZE_MINIMIZED == wParam))
				{
					active_ = false;
					this->OnSize()(*this, false);
				}
				else
				{
					active_ = true;
					this->OnSize()(*this, true);
				}
			}
			break;

		case WM_GETMINMAXINFO:
			// Prevent the window from going smaller than some minimu size
			reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 100;
			reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 100;
			break;

		case WM_SETCURSOR:
			this->OnSetCursor()(*this);
			break;

		case WM_CHAR:
			this->OnChar()(*this, static_cast<wchar_t>(wParam));
			break;

		case WM_INPUT:
			this->OnRawInput()(*this, reinterpret_cast<HRAWINPUT>(lParam));
			break;

#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		case WM_POINTERDOWN:
			{
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(this->HWnd(), &pt);
				this->OnPointerDown()(*this, int2(pt.x, pt.y), GET_POINTERID_WPARAM(wParam));
			}
			break;

		case WM_POINTERUP:
			{
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(this->HWnd(), &pt);
				this->OnPointerUp()(*this, int2(pt.x, pt.y), GET_POINTERID_WPARAM(wParam));
			}
			break;

		case WM_POINTERUPDATE:
			{
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(this->HWnd(), &pt);
				this->OnPointerUpdate()(*this, int2(pt.x, pt.y), GET_POINTERID_WPARAM(wParam),
					IS_POINTER_INCONTACT_WPARAM(wParam));
			}
			break;

		case WM_POINTERWHEEL:
			{
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(this->HWnd(), &pt);
				this->OnPointerWheel()(*this, int2(pt.x, pt.y), GET_POINTERID_WPARAM(wParam),
					GET_WHEEL_DELTA_WPARAM(wParam));
			}
			break;

		case WM_DPICHANGED:
			{
				RECT rc;
				::GetClientRect(wnd_, &rc);
				rc.left = static_cast<int32_t>(rc.left / dpi_scale_ + 0.5f);
				rc.top = static_cast<int32_t>(rc.top / dpi_scale_ + 0.5f);
				rc.right = static_cast<uint32_t>(rc.right / dpi_scale_ + 0.5f);
				rc.bottom = static_cast<uint32_t>(rc.bottom / dpi_scale_ + 0.5f);

				this->UpdateDpiScale(static_cast<float>(HIWORD(wParam)) / USER_DEFAULT_SCREEN_DPI);

				rc.left = static_cast<int32_t>(rc.left * dpi_scale_ + 0.5f);
				rc.top = static_cast<int32_t>(rc.top * dpi_scale_ + 0.5f);
				rc.right = static_cast<uint32_t>(rc.right * dpi_scale_ + 0.5f);
				rc.bottom = static_cast<uint32_t>(rc.bottom * dpi_scale_ + 0.5f);

				::AdjustWindowRect(&rc, win_style_, false);

				::SetWindowPos(this->HWnd(), HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION);
			}
			break;
#endif

		case WM_CLOSE:
			this->OnClose()(*this);
			active_ = false;
			ready_ = false;
			closed_ = true;
			::PostQuitMessage(0);
			return 0;

		case WM_SYSCOMMAND:
			if (keep_screen_on_)
			{
				switch (wParam)
				{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;

				default:
					break;
				}
			}
			break;
		}

		return default_wnd_proc_(hWnd, uMsg, wParam, lParam);
	}

	void Window::DetectsDpi()
	{
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		HMODULE shcore = ::LoadLibraryEx(TEXT("SHCore.dll"), nullptr, 0);
		if (shcore)
		{
			typedef HRESULT (WINAPI *SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS value);
			SetProcessDpiAwarenessFunc DynamicSetProcessDpiAwareness
				= reinterpret_cast<SetProcessDpiAwarenessFunc>(::GetProcAddress(shcore, "SetProcessDpiAwareness"));

			DynamicSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

			::FreeLibrary(shcore);
		}

		typedef NTSTATUS (WINAPI *RtlGetVersionFunc)(OSVERSIONINFOEXW* pVersionInformation);
		HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
		KLAYGE_ASSUME(ntdll != nullptr);
		RtlGetVersionFunc DynamicRtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(::GetProcAddress(ntdll, "RtlGetVersion"));
		if (DynamicRtlGetVersion)
		{
			OSVERSIONINFOEXW os_ver_info;
			os_ver_info.dwOSVersionInfoSize = sizeof(os_ver_info);
			DynamicRtlGetVersion(&os_ver_info);

			if ((os_ver_info.dwMajorVersion > 6) || ((6 == os_ver_info.dwMajorVersion) && (os_ver_info.dwMinorVersion >= 3)))
			{
				HDC desktop_dc = ::GetDC(nullptr);
				::EnumDisplayMonitors(desktop_dc, nullptr, EnumMonProc, reinterpret_cast<LPARAM>(this));
				::ReleaseDC(nullptr, desktop_dc);
			}
		}
#endif
	}

	void Window::KeepScreenOn()
	{
		if (keep_screen_on_)
		{
#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP)
			::SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
#endif
		}
	}
}

#endif
