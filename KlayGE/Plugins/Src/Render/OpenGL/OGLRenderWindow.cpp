#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>

#include <map>
#include <cassert>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderSettings.hpp>
#include <KlayGE/OpenGL/OGLRenderWindow.hpp>

namespace
{
	std::map<HWND, KlayGE::OGLRenderWindow*> winMap;
}

namespace KlayGE
{
	// Window procedure callback
	// This is a static member, so applies to all windows but we store the
	// OGLRenderWindow instance in the window data GetWindowLog/SetWindowLog
	LRESULT OGLRenderWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (winMap.find(hWnd) != winMap.end())
		{
			OGLRenderWindow* win(winMap[hWnd]);
			assert(win != NULL);

			return win->MsgProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	LRESULT OGLRenderWindow::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:
			if (WA_INACTIVE == LOWORD(wParam))
			{
				active_ = false;
			}
			else
			{
				active_ = true;
			}
			break;

		case WM_PAINT:
			// If we get WM_PAINT messges, it usually means our window was
			// comvered up, so we need to refresh it by re-showing the contents
			// of the current frame.
			if (this->Active() && this->Ready())
			{
				this->Update();
			}
			break;

		case WM_ENTERSIZEMOVE:
			// Previent rendering while moving / sizing
			this->Ready(false);
			break;

		case WM_EXITSIZEMOVE:
			this->WindowMovedOrResized();
			this->Ready(true);
			break;

		case WM_SIZE:
			// Check to see if we are losing or gaining our window.  Set the 
			// active flag to match
			if ((SIZE_MAXHIDE == wParam) || (SIZE_MINIMIZED == wParam))
			{
				active_ = false;
			}
			else
			{
				active_ = true;
				if (this->Ready())
				{
					this->WindowMovedOrResized();
				}
			}
			break;

		case WM_GETMINMAXINFO:
			// Prevent the window from going smaller than some minimu size
			reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 100;
			reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 100;
			break;

		case WM_CLOSE:
			::DestroyWindow(hWnd_);
			closed_ = true;
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	OGLRenderWindow::OGLRenderWindow(std::string const & name, OGLRenderSettings const & settings)
						: hWnd_(NULL),
							active_(false), ready_(false), closed_(false)
	{
		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isDepthBuffered_	= settings.depthBuffer;
		isFullScreen_		= settings.fullScreen;

		HINSTANCE hInst(::GetModuleHandle(NULL));

		// Destroy current window if any
		if (hWnd_ != NULL)
		{
			this->Destroy();
		}

		std::wstring wname;
		Convert(wname, name);

		// Register the window class
		WNDCLASSW wc;
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hInst;
		wc.hIcon			= NULL;
		wc.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= wname.c_str();
		::RegisterClassW(&wc);

		// Create our main window
		// Pass pointer to self
		hWnd_ = ::CreateWindowW(wname.c_str(), wname.c_str(),
			WS_OVERLAPPEDWINDOW, settings.left, settings.top,
			settings.width, settings.height, 0, 0, hInst, NULL);

		winMap.insert(std::make_pair(hWnd_, this));

		::ShowWindow(hWnd_, SW_SHOWNORMAL);
		::UpdateWindow(hWnd_);


		hDC_ = ::GetDC(hWnd_);

		if (isFullScreen_)
		{
			colorDepth_ = settings.colorDepth;
			left_ = 0;
			top_ = 0;

			DEVMODE devMode;
			devMode.dmSize = sizeof(devMode);
			devMode.dmBitsPerPel = colorDepth_;
			devMode.dmPelsWidth = width_;
			devMode.dmPelsHeight = height_;
			devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			::ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
		}
		else
		{
			// Get colour depth from display
			colorDepth_ = ::GetDeviceCaps(hDC_, BITSPIXEL);
			top_ = settings.top;
			left_ = settings.left;
		}

		// there is no guarantee that the contents of the stack that become
		// the pfd are zeroed, therefore _make sure_ to clear these bits.
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize		= sizeof(pfd);
		pfd.nVersion	= 1;
		pfd.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType	= PFD_TYPE_RGBA;
		pfd.cColorBits	= static_cast<BYTE>(colorDepth_);
		pfd.cDepthBits	= 16;
		pfd.iLayerType	= PFD_MAIN_PLANE;

		int pixelFormat(::ChoosePixelFormat(hDC_, &pfd));
		assert(pixelFormat != 0);

		::SetPixelFormat(hDC_, pixelFormat, &pfd);

		::DescribePixelFormat(hDC_, pixelFormat, sizeof(pfd), &pfd);

		hRC_ = ::wglCreateContext(hDC_);
		::wglMakeCurrent(hDC_, hRC_);

		glEnable(GL_COLOR_MATERIAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		viewport_.left = left_;
		viewport_.top = top_;
		viewport_.width = width_;
		viewport_.height = height_;

		active_ = true;
		ready_ = true;
	}

	OGLRenderWindow::~OGLRenderWindow()
	{
		this->Destroy();
	}

	bool OGLRenderWindow::Active() const
	{
		return active_;
	}

	bool OGLRenderWindow::Closed() const
	{
		return closed_;
	}

	bool OGLRenderWindow::Ready() const
	{
		return ready_;
	}

	void OGLRenderWindow::Ready(bool ready)
	{
		ready_ = ready;
	}

	HWND OGLRenderWindow::WindowHandle() const
	{
		return hWnd_;
	}

	std::wstring const & OGLRenderWindow::Description() const
	{
		return description_;
	}

	bool OGLRenderWindow::RequiresTextureFlipping() const
	{
		return false;
	}

	void OGLRenderWindow::WindowMovedOrResized()
	{
	}

	void OGLRenderWindow::Destroy()
	{
		if (hWnd_ != NULL)
		{
			::wglMakeCurrent(NULL, NULL);
			::ReleaseDC(hWnd_, hDC_);
			::wglDeleteContext(hRC_);

			if (isFullScreen_)
			{
				::ChangeDisplaySettings(NULL, 0);
			}

			::DestroyWindow(hWnd_);
			hWnd_ = NULL;
		}
	}

	void OGLRenderWindow::Reposition(int left, int top)
	{
		viewport_.left = left;
		viewport_.top = top;
	}

	void OGLRenderWindow::Resize(int width, int height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_.width = width;
		viewport_.height = height;
		// TODO - resize window
	}

	void OGLRenderWindow::SwapBuffers()
	{
		::glFlush();
		::SwapBuffers(hDC_);
	}

	void OGLRenderWindow::CustomAttribute(std::string const & /*name*/, void* /*pData*/)
	{
		assert(false);
	}
}
