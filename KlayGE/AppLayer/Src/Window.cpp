// Window.cpp
// KlayGE Window类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.6.26)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>

#include <windows.h>

#include <KlayGE/Window.hpp>

namespace KlayGE
{
	LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Window* win = reinterpret_cast<Window*>(::GetWindowLongPtrA(hWnd, GWLP_USERDATA));
		if (win != NULL)
		{
			return win->MsgProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	Window::Window(std::string const & name, int32_t left, int32_t top,
			uint32_t width, uint32_t height)
	{
		HINSTANCE hInst = ::GetModuleHandle(NULL);

		// Register the window class
		WNDCLASSEXA wc;
		wc.cbSize			= sizeof(wc);
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= sizeof(this);
		wc.hInstance		= hInst;
		wc.hIcon			= NULL;
		wc.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= name.c_str();
		wc.hIconSm			= NULL;
		::RegisterClassExA(&wc);

		RECT rc = { 0, 0, width, height };
		::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

		// Create our main window
		// Pass pointer to self
		HWND hWnd = ::CreateWindowA(name.c_str(), name.c_str(),
			WS_OVERLAPPEDWINDOW, left, top,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, NULL);

		::GetClientRect(hWnd, &rc);
		left_ = left;
		top_ = top;
		width_ = rc.right - rc.left;
		height_ = rc.bottom - rc.top;

		::SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		::ShowWindow(hWnd, SW_SHOWNORMAL);
		::UpdateWindow(hWnd);

		wnd_handle_ = hWnd;
	}

	Window::~Window()
	{
		if (wnd_handle_ != NULL)
		{
			::DestroyWindow(static_cast<HWND>(wnd_handle_));
			wnd_handle_ = NULL;
		}
	}

	LRESULT Window::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:
			if (WA_INACTIVE == LOWORD(wParam))
			{
				this->OnActive()(*this, false);
			}
			else
			{
				this->OnActive()(*this, true);
			}
			break;

		case WM_PAINT:
			this->OnPaint()(*this);
			break;

		case WM_ENTERSIZEMOVE:
			// Previent rendering while moving / sizing
			this->OnEnterSizeMove()(*this);
			break;

		case WM_EXITSIZEMOVE:
			this->OnExitSizeMove()(*this);
			break;

		case WM_SIZE:
			// Check to see if we are losing or gaining our window.  Set the
			// active flag to match
			if ((SIZE_MAXHIDE == wParam) || (SIZE_MINIMIZED == wParam))
			{
				this->OnSize()(*this, false);
			}
			else
			{
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

		case WM_KEYDOWN:
			this->OnKeyDown()(*this, static_cast<wchar_t>(wParam));
			break;

		case WM_KEYUP:
			this->OnKeyUp()(*this, static_cast<wchar_t>(wParam));
			break;

		case WM_CLOSE:
			this->OnClose()(*this);
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}
