// OGLRenderWindow.cpp
// KlayGE OpenGL渲染窗口类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 只支持OpenGL 1.5及以上 (2005.8.12)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <map>
#include <boost/assert.hpp>

#include <glloader/glloader.h>

#include <Cg/CgGL.h>

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
			BOOST_ASSERT(win != NULL);

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

	OGLRenderWindow::OGLRenderWindow(std::string const & name, RenderSettings const & settings)
						: hWnd_(NULL),
							ready_(false), closed_(false)
	{
		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isDepthBuffered_	= IsDepthFormat(settings.depth_stencil_fmt);
		depthBits_			= NumDepthBits(settings.depth_stencil_fmt);
		stencilBits_		= NumStencilBits(settings.depth_stencil_fmt);
		isFullScreen_		= settings.full_screen;

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

		uint32_t style;
		if (isFullScreen_)
		{
			colorDepth_ = NumFormatBits(settings.color_fmt);
			left_ = 0;
			top_ = 0;

			DEVMODE devMode;
			devMode.dmSize = sizeof(devMode);
			devMode.dmBitsPerPel = colorDepth_;
			devMode.dmPelsWidth = width_;
			devMode.dmPelsHeight = height_;
			devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			::ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);

			style = WS_POPUP;
		}
		else
		{
			// Get colour depth from display
			colorDepth_ = ::GetDeviceCaps(hDC_, BITSPIXEL);
			top_ = settings.top;
			left_ = settings.left;

			style = WS_OVERLAPPEDWINDOW;
		}

		RECT rc = { 0, 0, width_, height_ };
		::AdjustWindowRect(&rc, style, false);

		// Create our main window
		// Pass pointer to self
		hWnd_ = ::CreateWindowW(wname.c_str(), wname.c_str(),
			style, settings.left, settings.top,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, NULL);

		winMap.insert(std::make_pair(hWnd_, this));

		::ShowWindow(hWnd_, SW_SHOWNORMAL);
		::UpdateWindow(hWnd_);


		hDC_ = ::GetDC(hWnd_);

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
		BOOST_ASSERT(pixelFormat != 0);

		::SetPixelFormat(hDC_, pixelFormat, &pfd);

		::DescribePixelFormat(hDC_, pixelFormat, sizeof(pfd), &pfd);

		hRC_ = ::wglCreateContext(hDC_);
		::wglMakeCurrent(hDC_, hRC_);

		if (!glloader_GL_VERSION_2_0() || !glloader_GL_EXT_framebuffer_object())
		{
			THR(E_FAIL);
		}

		glEnable(GL_COLOR_MATERIAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		if (settings.multi_sample != 0)
		{
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_SAMPLE_COVERAGE);
			glSampleCoverage(settings.multi_sample / 16.0f, false);
		}

		viewport_.left = 0;
		viewport_.top = 0;
		viewport_.width = width_;
		viewport_.height = height_;

		std::wstring vendor, renderer, version;
		Convert(vendor, reinterpret_cast<char const *>(glGetString(GL_VENDOR)));
		Convert(renderer, reinterpret_cast<char const *>(glGetString(GL_RENDERER)));
		Convert(version, reinterpret_cast<char const *>(glGetString(GL_VERSION)));
		description_ = vendor + L" " + renderer + L" " + version;

		active_ = true;
		ready_ = true;
	}

	OGLRenderWindow::~OGLRenderWindow()
	{
		this->Destroy();
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

	void OGLRenderWindow::WindowMovedOrResized()
	{
		::RECT rect;
		::GetWindowRect(hWnd_, &rect);

		uint32_t new_left = rect.left;
		uint32_t new_top = rect.top;
		if ((new_left != left_) || (new_top != top_))
		{
			this->Reposition(new_left, new_top);
		}

		uint32_t new_width = rect.right - rect.left;
		uint32_t new_height = rect.bottom - rect.top;
		if ((new_width != width_) || (new_height != height_))
		{
			this->Resize(new_width, new_height);
		}
	}

	void OGLRenderWindow::Destroy()
	{
		if (hWnd_ != NULL)
		{
			if (isFullScreen_)
			{
				::ChangeDisplaySettings(NULL, 0);
				ShowCursor(TRUE);
			}

			::wglMakeCurrent(NULL, NULL);
			::wglDeleteContext(hRC_);
			::ReleaseDC(hWnd_, hDC_);

			::DestroyWindow(hWnd_);
			hWnd_ = NULL;
		}
	}

	void OGLRenderWindow::SwapBuffers()
	{
		::glFlush();
		::SwapBuffers(hDC_);
	}

	void OGLRenderWindow::CustomAttribute(std::string const & /*name*/, void* /*pData*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLRenderWindow::DoReposition(uint32_t /*left*/, uint32_t /*top*/)
	{
	}

    void OGLRenderWindow::DoResize(uint32_t /*width*/, uint32_t /*height*/)
	{
	}

	void OGLRenderWindow::OnBind()
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
}
