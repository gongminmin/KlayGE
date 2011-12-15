// Window.cpp
// KlayGE Window类 实现文件
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

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>

#ifdef KLAYGE_PLATFORM_ANDROID
#include <KlayGE/App3D.hpp>
#endif

#include <KlayGE/Window.hpp>

#ifndef GET_KEYSTATE_WPARAM
#define GET_KEYSTATE_WPARAM(wParam) (LOWORD(wParam))
#endif

namespace KlayGE
{
#ifdef KLAYGE_PLATFORM_WINDOWS
	LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4312)
#endif
		Window* win = reinterpret_cast<Window*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
		if (win != NULL)
		{
			return win->MsgProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	Window::Window(std::string const & name, RenderSettings const & settings)
	{
		HINSTANCE hInst = ::GetModuleHandle(NULL);

		// Register the window class
#ifdef KLAYGE_COMPILER_GCC
		name_ = name;
		WNDCLASSEXA wc;
#else
		Convert(wname_, name);
		WNDCLASSEXW wc;
#endif
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
#ifdef KLAYGE_COMPILER_GCC
		wc.lpszClassName	= name_.c_str();
#else
		wc.lpszClassName	= wname_.c_str();
#endif
		wc.hIconSm			= NULL;
#ifdef KLAYGE_COMPILER_GCC
		::RegisterClassExA(&wc);
#else
		::RegisterClassExW(&wc);
#endif

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
#ifdef KLAYGE_COMPILER_GCC
		wnd_ = ::CreateWindowA(name_.c_str(), name_.c_str(),
			style, settings.left, settings.top,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, NULL);
#else
		wnd_ = ::CreateWindowW(wname_.c_str(), wname_.c_str(),
			style, settings.left, settings.top,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, NULL);
#endif

		::GetClientRect(wnd_, &rc);
		left_ = settings.left;
		top_ = settings.top;
		width_ = rc.right - rc.left;
		height_ = rc.bottom - rc.top;

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif
		::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

		::ShowWindow(wnd_, SW_SHOWNORMAL);
		::UpdateWindow(wnd_);
	}

	Window::~Window()
	{
		if (wnd_ != NULL)
		{
			::DestroyWindow(wnd_);
			wnd_ = NULL;
		}
	}

	void Window::Recreate()
	{
		HINSTANCE hInst = ::GetModuleHandle(NULL);

		uint32_t style = static_cast<uint32_t>(::GetWindowLongPtrW(wnd_, GWL_STYLE));
		RECT rc = { 0, 0, width_, height_ };
		::AdjustWindowRect(&rc, style, false);

		::DestroyWindow(wnd_);

#ifdef KLAYGE_COMPILER_GCC
		wnd_ = ::CreateWindowA(name_.c_str(), name_.c_str(),
			style, left_, top_,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, NULL);
#else
		wnd_ = ::CreateWindowW(wname_.c_str(), wname_.c_str(),
			style, left_, top_,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, NULL);
#endif

		::GetClientRect(wnd_, &rc);
		width_ = rc.right - rc.left;
		height_ = rc.bottom - rc.top;

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif
		::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

		::ShowWindow(wnd_, SW_SHOWNORMAL);
		::UpdateWindow(wnd_);
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

		case WM_ERASEBKGND:
			return 1;

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

		case WM_LBUTTONDOWN:
			{
				uint32_t buttons = MB_Left;
				if (wParam & MK_RBUTTON)
				{
					buttons |= MB_Right;
				}
				if (wParam & MK_MBUTTON)
				{
					buttons |= MB_Middle;
				}
				if (wParam & MK_CONTROL)
				{
					buttons |= MB_Ctrl;
				}
				if (wParam & MK_SHIFT)
				{
					buttons |= MB_Shift;
				}

				this->OnMouseDown()(*this, buttons, Vector_T<int32_t, 2>(LOWORD(lParam), HIWORD(lParam)));
			}
			break;

		case WM_RBUTTONDOWN:
			{
				uint32_t buttons = MB_Right;
				if (wParam & MK_LBUTTON)
				{
					buttons |= MB_Left;
				}
				if (wParam & MK_MBUTTON)
				{
					buttons |= MB_Middle;
				}
				if (wParam & MK_CONTROL)
				{
					buttons |= MB_Ctrl;
				}
				if (wParam & MK_SHIFT)
				{
					buttons |= MB_Shift;
				}

				this->OnMouseDown()(*this, buttons, Vector_T<int32_t, 2>(LOWORD(lParam), HIWORD(lParam)));
			}
			break;

		case WM_MBUTTONDOWN:
			{
				uint32_t buttons = MB_Middle;
				if (wParam & MK_LBUTTON)
				{
					buttons |= MB_Left;
				}
				if (wParam & MK_RBUTTON)
				{
					buttons |= MB_Right;
				}
				if (wParam & MK_CONTROL)
				{
					buttons |= MB_Ctrl;
				}
				if (wParam & MK_SHIFT)
				{
					buttons |= MB_Shift;
				}

				this->OnMouseDown()(*this, buttons, Vector_T<int32_t, 2>(LOWORD(lParam), HIWORD(lParam)));
			}
			break;

		case WM_LBUTTONUP:
			{
				uint32_t buttons = MB_Left;
				if (wParam & MK_RBUTTON)
				{
					buttons |= MB_Right;
				}
				if (wParam & MK_MBUTTON)
				{
					buttons |= MB_Middle;
				}
				if (wParam & MK_CONTROL)
				{
					buttons |= MB_Ctrl;
				}
				if (wParam & MK_SHIFT)
				{
					buttons |= MB_Shift;
				}

				this->OnMouseUp()(*this, buttons, Vector_T<int32_t, 2>(LOWORD(lParam), HIWORD(lParam)));
			}
			break;

		case WM_RBUTTONUP:
			{
				uint32_t buttons = MB_Right;
				if (wParam & MK_LBUTTON)
				{
					buttons |= MB_Left;
				}
				if (wParam & MK_MBUTTON)
				{
					buttons |= MB_Middle;
				}
				if (wParam & MK_CONTROL)
				{
					buttons |= MB_Ctrl;
				}
				if (wParam & MK_SHIFT)
				{
					buttons |= MB_Shift;
				}

				this->OnMouseUp()(*this, buttons, Vector_T<int32_t, 2>(LOWORD(lParam), HIWORD(lParam)));
			}
			break;

		case WM_MBUTTONUP:
			{
				uint32_t buttons = MB_Middle;
				if (wParam & MK_LBUTTON)
				{
					buttons |= MB_Left;
				}
				if (wParam & MK_RBUTTON)
				{
					buttons |= MB_Right;
				}
				if (wParam & MK_CONTROL)
				{
					buttons |= MB_Ctrl;
				}
				if (wParam & MK_SHIFT)
				{
					buttons |= MB_Shift;
				}

				this->OnMouseUp()(*this, buttons, Vector_T<int32_t, 2>(LOWORD(lParam), HIWORD(lParam)));
			}
			break;

		case WM_MOUSEWHEEL:
			{
				uint32_t buttons = MB_None;
				if (GET_KEYSTATE_WPARAM(wParam) & MK_LBUTTON)
				{
					buttons |= MB_Left;
				}
				if (GET_KEYSTATE_WPARAM(wParam) & MK_RBUTTON)
				{
					buttons |= MB_Right;
				}
				if (GET_KEYSTATE_WPARAM(wParam) & MK_MBUTTON)
				{
					buttons |= MB_Middle;
				}
				if (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL)
				{
					buttons |= MB_Ctrl;
				}
				if (GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT)
				{
					buttons |= MB_Shift;
				}

				this->OnMouseWheel()(*this, buttons, Vector_T<int32_t, 2>(LOWORD(lParam), HIWORD(lParam)), GET_WHEEL_DELTA_WPARAM(wParam));
			}
			break;

		case WM_MOUSEMOVE:
			{
				uint32_t buttons = MB_None;
				if (GET_KEYSTATE_WPARAM(wParam) & MK_LBUTTON)
				{
					buttons |= MB_Left;
				}
				if (GET_KEYSTATE_WPARAM(wParam) & MK_RBUTTON)
				{
					buttons |= MB_Right;
				}
				if (GET_KEYSTATE_WPARAM(wParam) & MK_MBUTTON)
				{
					buttons |= MB_Middle;
				}
				if (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL)
				{
					buttons |= MB_Ctrl;
				}
				if (GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT)
				{
					buttons |= MB_Shift;
				}

				this->OnMouseOver()(*this, buttons, Vector_T<int32_t, 2>(LOWORD(lParam), HIWORD(lParam)));
			}
			break;

		case WM_CLOSE:
			this->OnClose()(*this);
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
#elif defined KLAYGE_PLATFORM_LINUX
	Window::Window(std::string const & name, RenderSettings const & settings)
	{
		x_display_ = XOpenDisplay(NULL);

		int r_size, g_size, b_size, a_size, d_size, s_size;
		switch (settings.color_fmt)
		{
		case EF_ARGB8:
		case EF_ABGR8:
			r_size = 8;
			g_size = 8;
			b_size = 8;
			a_size = 8;
			break;

		case EF_A2BGR10:
			r_size = 10;
			g_size = 10;
			b_size = 10;
			a_size = 2;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		switch (settings.depth_stencil_fmt)
		{
		case EF_D16:
			d_size = 16;
			s_size = 0;
			break;

		case EF_D24S8:
			d_size = 24;
			s_size = 8;
			break;

		case EF_D32F:
			d_size = 32;
			s_size = 0;
			break;

		default:
			d_size = 0;
			s_size = 0;
			break;
		}

		std::vector<int> visual_attr;
		//visual_attr.push_back(GLX_RENDER_TYPE);
		//visual_attr.push_back(GLX_RGBA_BIT);
		visual_attr.push_back(GLX_RGBA);
		visual_attr.push_back(GLX_RED_SIZE);
		visual_attr.push_back(r_size);
		visual_attr.push_back(GLX_GREEN_SIZE);
		visual_attr.push_back(g_size);
		visual_attr.push_back(GLX_BLUE_SIZE);
		visual_attr.push_back(b_size);
		visual_attr.push_back(GLX_ALPHA_SIZE);
		visual_attr.push_back(a_size);
		//visual_attr.push_back(GLX_DRAWABLE_TYPE);
		//visual_attr.push_back(GLX_WINDOW_BIT);
		if (d_size > 0)
		{
			visual_attr.push_back(GLX_DEPTH_SIZE);
			visual_attr.push_back(d_size);
		}
		if (s_size > 0)
		{
			visual_attr.push_back(GLX_STENCIL_SIZE);
			visual_attr.push_back(s_size);
		}
		visual_attr.push_back(GLX_DOUBLEBUFFER);
		visual_attr.push_back(True);
		if (settings.sample_count > 1)
		{
			visual_attr.push_back(GLX_SAMPLE_BUFFERS);
			visual_attr.push_back(1);
			visual_attr.push_back(GLX_BUFFER_SIZE);
			visual_attr.push_back(settings.sample_count);
		}
		visual_attr.push_back(None);				// end of list

		glXChooseFBConfig = (glXChooseFBConfigFUNC)(glloader_get_gl_proc_address("glXChooseFBConfig"));
		glXGetVisualFromFBConfig = (glXGetVisualFromFBConfigFUNC)(glloader_get_gl_proc_address("glXGetVisualFromFBConfig"));

		//int num_elements;
		//fbc_ = glXChooseFBConfig(x_display_, DefaultScreen(x_display_), &visual_attr[0], &num_elements);

		//vi_ = glXGetVisualFromFBConfig(x_display_, fbc_[0]);
		vi_ = glXChooseVisual(x_display_, DefaultScreen(x_display_), &visual_attr[0]);

		XSetWindowAttributes attr;
		attr.colormap     = XCreateColormap(x_display_, RootWindow(x_display_, vi_->screen), vi_->visual, AllocNone);
		attr.border_pixel = 0;
		attr.event_mask   = ExposureMask
								| VisibilityChangeMask
								| KeyPressMask
								| KeyReleaseMask
								| ButtonPressMask
								| ButtonReleaseMask
								| PointerMotionMask
								| StructureNotifyMask
								| SubstructureNotifyMask
								| FocusChangeMask
								| ResizeRedirectMask;
		x_window_ = XCreateWindow(x_display_, RootWindow(x_display_, vi_->screen),
					settings.left, settings.top, settings.width, settings.height, 0, vi_->depth,
					InputOutput, vi_->visual, CWBorderPixel | CWColormap | CWEventMask, &attr);
		XStoreName(x_display_, x_window_, name.c_str());
		XMapWindow(x_display_, x_window_);
		XFlush(x_display_);

		XWindowAttributes win_attr;
		XGetWindowAttributes(x_display_, x_window_, &win_attr);
		left_ = win_attr.x;
		top_ = win_attr.y;
		width_ = win_attr.width;
		height_ = win_attr.height;

		wm_delete_window_ = XInternAtom(x_display_, "WM_DELETE_WINDOW", false);
		XSetWMProtocols(x_display_, x_window_, &wm_delete_window_, 1);
	}

	Window::~Window()
	{
		//XFree(fbc_);
		XFree(vi_);
		XDestroyWindow(x_display_, x_window_);
		XCloseDisplay(x_display_);
	}

	void Window::MsgProc(XEvent const & event)
	{
		switch (event.type)
		{
		case FocusIn:
			this->OnActive()(*this, true);
			break;

		case FocusOut:
			this->OnActive()(*this, false);
			break;

		case Expose:
			this->OnPaint()(*this);
			break;

		case ResizeRequest:
			{
				XResizeRequestEvent const & resize_ev = reinterpret_cast<XResizeRequestEvent const &>(event);
				if ((0 == resize_ev.width) || (0 == resize_ev.height))
				{
					this->OnSize()(*this, false);
				}
				else
				{
					this->OnSize()(*this, true);
				}
			}
			break;

		case KeyPress:
			{
				XKeyEvent const & key_ev = reinterpret_cast<XKeyEvent const &>(event);
				this->OnKeyDown()(*this, static_cast<wchar_t>(key_ev.keycode));
			}
			break;

		case KeyRelease:
			{
				XKeyEvent const & key_ev = reinterpret_cast<XKeyEvent const &>(event);
				this->OnKeyUp()(*this, static_cast<wchar_t>(key_ev.keycode));
			}
			break;

		case ClientMessage:
			if (wm_delete_window_ == static_cast<Atom>(event.xclient.data.l[0]))
			{
				this->OnClose()(*this);
				XDestroyWindow(x_display_, x_window_);
				XCloseDisplay(x_display_);
				exit(0);
			}
			break;
		}
	}
#elif defined KLAYGE_PLATFORM_ANDROID
	Window::Window(std::string const & /*name*/, RenderSettings const & /*settings*/)
	{
		a_window_ = NULL;

		android_app* state = Context::Instance().AppInstance().AppState();
		state->userData = this;
		state->onAppCmd = MsgProc;

		for (;;)
		{
			// Read all pending events.
			int ident;
			int events;
			android_poll_source* source;

			do
			{
				ident = ALooper_pollAll(0, NULL, &events, reinterpret_cast<void**>(&source));

				// Process this event.
				if (source != NULL)
				{
					source->process(state, source);
				}

				if (a_window_ != NULL)
				{
					return;
				}

				// Check if we are exiting.
				if (state->destroyRequested != 0)
				{
					return;
				}
			} while (ident >= 0);
		}
	}

	Window::~Window()
	{
	}

	void Window::MsgProc(android_app* app, int32_t cmd)
	{
		Window* win = static_cast<Window*>(app->userData);
		switch (cmd)
		{
		case APP_CMD_SAVE_STATE:
			break;

		case APP_CMD_INIT_WINDOW:
			win->a_window_ = app->window;
			break;
        
		case APP_CMD_TERM_WINDOW:
			win->OnClose()(*win);
			break;

		case APP_CMD_GAINED_FOCUS:
			win->OnActive()(*win, true);
            break;

		case APP_CMD_LOST_FOCUS:
			win->OnActive()(*win, false);
			break;
		}
	}
#endif
}
