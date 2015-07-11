/**
* @file WindowsWin.cpp
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

#include <windowsx.h>

#ifndef GET_KEYSTATE_WPARAM
#define GET_KEYSTATE_WPARAM(wParam) (LOWORD(wParam))
#endif

namespace KlayGE
{
	LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4312) // LONG_PTR to Window*, could cast to a greater size
#endif
		Window* win = reinterpret_cast<Window*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
		if (win != nullptr)
		{
			return win->MsgProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	Window::Window(std::string const & name, RenderSettings const & settings)
		: active_(false), ready_(false), closed_(false), hide_(settings.hide_win)
	{
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

		uint32_t style;
		if (settings.full_screen)
		{
			style = WS_POPUP;
		}
		else
		{
			style = WS_OVERLAPPEDWINDOW;
		}

		RECT rc = { 0, 0, settings.width, settings.height };
		::AdjustWindowRect(&rc, style, false);

		// Create our main window
		// Pass pointer to self
		wnd_ = ::CreateWindowW(wname_.c_str(), wname_.c_str(),
			style, settings.left, settings.top,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, nullptr);

		default_wnd_proc_ = ::DefWindowProc;
		external_wnd_ = false;

		::GetClientRect(wnd_, &rc);
		left_ = settings.left;
		top_ = settings.top;
		width_ = rc.right - rc.left;
		height_ = rc.bottom - rc.top;

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244) // Pointer to LONG_TR, possible loss of data
#endif
		::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

		::ShowWindow(wnd_, hide_ ? SW_HIDE : SW_SHOWNORMAL);
		::UpdateWindow(wnd_);
	}

	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false), hide_(settings.hide_win)
	{
		Convert(wname_, name);

		wnd_ = static_cast<HWND>(native_wnd);
		default_wnd_proc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtrW(wnd_, GWLP_WNDPROC));
		::SetWindowLongPtrW(wnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
		external_wnd_ = true;

		RECT rc;
		::GetClientRect(wnd_, &rc);
		left_ = settings.left;
		top_ = settings.top;
		width_ = rc.right - rc.left;
		height_ = rc.bottom - rc.top;

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244) // Pointer to LONG_TR, possible loss of data
#endif
		::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

		::UpdateWindow(wnd_);
	}

	Window::~Window()
	{
		if (wnd_ != nullptr)
		{
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244) // Pointer to LONG_TR, possible loss of data
#endif
			::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
			if (!external_wnd_)
			{
				::DestroyWindow(wnd_);
			}

			wnd_ = nullptr;
		}
	}

	void Window::Recreate()
	{
		if (!external_wnd_)
		{
			HINSTANCE hInst = ::GetModuleHandle(nullptr);

			uint32_t style = static_cast<uint32_t>(::GetWindowLongPtrW(wnd_, GWL_STYLE));
			RECT rc = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
			::AdjustWindowRect(&rc, style, false);

			::DestroyWindow(wnd_);

			wnd_ = ::CreateWindowW(wname_.c_str(), wname_.c_str(),
				style, left_, top_,
				rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, nullptr);

			::GetClientRect(wnd_, &rc);
			width_ = rc.right - rc.left;
			height_ = rc.bottom - rc.top;

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244) // Pointer to LONG_TR, possible loss of data
#endif
			::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

			::ShowWindow(wnd_, hide_ ? SW_HIDE : SW_SHOWNORMAL);
			::UpdateWindow(wnd_);
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

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
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

#elif (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
		case WM_TOUCH:
			this->OnTouch()(*this, reinterpret_cast<HTOUCHINPUT>(lParam), LOWORD(wParam));
			break;
#endif

		case WM_CLOSE:
			this->OnClose()(*this);
			active_ = false;
			ready_ = false;
			closed_ = true;
			::PostQuitMessage(0);
			return 0;
		}

		return default_wnd_proc_(hWnd, uMsg, wParam, lParam);
	}
}

#endif
