// OGLRenderWindow.cpp
// KlayGE OpenGL渲染窗口类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2004-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 修正了在Vista下从全屏模式退出时crash的bug (2007.3.23)
// 支持动态切换全屏/窗口模式 (2007.3.24)
//
// 2.8.0
// 只支持OpenGL 1.5及以上 (2005.8.12)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <map>
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include <glloader/glloader.h>

#include <Cg/CgGL.h>

#include <KlayGE/OpenGL/OGLRenderWindow.hpp>

namespace KlayGE
{
	OGLRenderWindow::OGLRenderWindow(std::string const & name, RenderSettings const & settings)
						: OGLFrameBuffer(false),
							hWnd_(NULL),
							ready_(false), closed_(false)
	{
		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isDepthBuffered_	= IsDepthFormat(settings.depth_stencil_fmt);
		depthBits_			= NumDepthBits(settings.depth_stencil_fmt);
		stencilBits_		= NumStencilBits(settings.depth_stencil_fmt);
		format_				= settings.color_fmt;
		isFullScreen_		= settings.full_screen;

		// Destroy current window if any
		if (hWnd_ != NULL)
		{
			this->Destroy();
		}

		fs_color_depth_ = NumFormatBits(settings.color_fmt);

		uint32_t style;
		if (isFullScreen_)
		{
			colorDepth_ = fs_color_depth_;
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

		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		hWnd_ = static_cast<HWND>(main_wnd->WindowHandle());
		main_wnd->OnActive().connect(boost::bind(&OGLRenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().connect(boost::bind(&OGLRenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().connect(boost::bind(&OGLRenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().connect(boost::bind(&OGLRenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().connect(boost::bind(&OGLRenderWindow::OnSize, this, _1, _2));
		main_wnd->OnClose().connect(boost::bind(&OGLRenderWindow::OnClose, this, _1));


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
		pfd.cDepthBits	= static_cast<BYTE>(depthBits_);
		pfd.iLayerType	= PFD_MAIN_PLANE;

		int pixelFormat(::ChoosePixelFormat(hDC_, &pfd));
		BOOST_ASSERT(pixelFormat != 0);

		::SetPixelFormat(hDC_, pixelFormat, &pfd);
		::DescribePixelFormat(hDC_, pixelFormat, sizeof(pfd), &pfd);

		hRC_ = ::wglCreateContext(hDC_);
		::wglMakeCurrent(hDC_, hRC_);

		if (!glloader_GL_VERSION_2_0() || !glloader_GL_EXT_framebuffer_object()
			|| !glloader_GL_ARB_pixel_buffer_object())
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

		if (glloader_GL_ARB_color_buffer_float())
		{
			glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_TRUE);
			glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_TRUE);
			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_TRUE);
		}

		if (glloader_WGL_EXT_swap_control())
		{
			wglSwapIntervalEXT(0);
		}

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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

	std::wstring const & OGLRenderWindow::Description() const
	{
		return description_;
	}

	// 改变窗口大小
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderWindow::Resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_.width = width;
		viewport_.height = height;

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);
	}

	// 改变窗口位置
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderWindow::Reposition(uint32_t left, uint32_t top)
	{
		left_ = left;
		top_ = top;
	}

	// 获取是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	bool OGLRenderWindow::FullScreen() const
	{
		return isFullScreen_;
	}

	// 设置是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderWindow::FullScreen(bool fs)
	{
		if (isFullScreen_ != fs)
		{
			left_ = 0;
			top_ = 0;

			uint32_t style;
			if (fs)
			{
				colorDepth_ = fs_color_depth_;

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
				colorDepth_ = ::GetDeviceCaps(hDC_, BITSPIXEL);
				::ChangeDisplaySettings(NULL, 0);

				style = WS_OVERLAPPEDWINDOW;
			}

			::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);

			RECT rc = { 0, 0, width_, height_ };
			::AdjustWindowRect(&rc, style, false);
			width_ = rc.right - rc.left;
			height_ = rc.bottom - rc.top;

			isFullScreen_ = fs;

			::ShowWindow(hWnd_, SW_SHOWNORMAL);
			::UpdateWindow(hWnd_);
		}
	}

	void OGLRenderWindow::WindowMovedOrResized()
	{
		::RECT rect;
		::GetClientRect(hWnd_, &rect);

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
			::wglMakeCurrent(NULL, NULL);
			::wglDeleteContext(hRC_);
			::ReleaseDC(hWnd_, hDC_);

			if (isFullScreen_)
			{
				::ChangeDisplaySettings(NULL, 0);
				ShowCursor(TRUE);
			}
		}
	}

	void OGLRenderWindow::SwapBuffers()
	{
		::glFlush();
		::SwapBuffers(hDC_);
	}

	void OGLRenderWindow::OnActive(Window const & /*win*/, bool active)
	{
		active_ = active;
	}

	void OGLRenderWindow::OnPaint(Window const & /*win*/)
	{
		// If we get WM_PAINT messges, it usually means our window was
		// comvered up, so we need to refresh it by re-showing the contents
		// of the current frame.
		if (this->Active() && this->Ready())
		{
			Context::Instance().SceneManagerInstance().Update();
			this->SwapBuffers();
		}
	}

	void OGLRenderWindow::OnEnterSizeMove(Window const & /*win*/)
	{
		// Previent rendering while moving / sizing
		this->Ready(false);
	}

	void OGLRenderWindow::OnExitSizeMove(Window const & /*win*/)
	{
		this->WindowMovedOrResized();
		this->Ready(true);
	}

	void OGLRenderWindow::OnSize(Window const & /*win*/, bool active)
	{
		if (!active)
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
	}

	void OGLRenderWindow::OnClose(Window const & /*win*/)
	{
		this->Destroy();
		closed_ = true;
	}
}
