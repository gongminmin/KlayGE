// OGLRenderWindow.cpp
// KlayGE OpenGL渲染窗口类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2004-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 实验性的linux支持 (2008.5.19)
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
#include <KlayGE/Math.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <map>
#include <sstream>
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include <glloader/glloader.h>

#include <Cg/cgGL.h>

#include <KlayGE/OpenGL/OGLRenderWindow.hpp>

namespace KlayGE
{
	OGLRenderWindow::OGLRenderWindow(std::string const & name, RenderSettings const & settings)
						: OGLFrameBuffer(false),
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

		fs_color_depth_ = NumFormatBits(settings.color_fmt);

		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		main_wnd->OnActive().connect(boost::bind(&OGLRenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().connect(boost::bind(&OGLRenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().connect(boost::bind(&OGLRenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().connect(boost::bind(&OGLRenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().connect(boost::bind(&OGLRenderWindow::OnSize, this, _1, _2));
		main_wnd->OnClose().connect(boost::bind(&OGLRenderWindow::OnClose, this, _1));

#if defined KLAYGE_PLATFORM_WINDOWS
		hWnd_ = main_wnd->HWnd();
		hDC_ = ::GetDC(hWnd_);

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

		RECT rc = { 0, 0, width_, height_ };
		::AdjustWindowRect(&rc, style, false);

		::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);
		::SetWindowPos(hWnd_, NULL, settings.left, settings.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_SHOWWINDOW | SWP_NOZORDER);

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
		pfd.cStencilBits = static_cast<BYTE>(stencilBits_);
		pfd.iLayerType	= PFD_MAIN_PLANE;

		int pixelFormat = ::ChoosePixelFormat(hDC_, &pfd);
		BOOST_ASSERT(pixelFormat != 0);

		::SetPixelFormat(hDC_, pixelFormat, &pfd);
		::DescribePixelFormat(hDC_, pixelFormat, sizeof(pfd), &pfd);

		hRC_ = ::wglCreateContext(hDC_);
		::wglMakeCurrent(hDC_, hRC_);

		uint32_t sample_count = settings.sample_count;

		if (sample_count > 1)
		{
			UINT num_formats;
			float float_attrs[] = { 0, 0 };
			BOOL valid;
			do
			{
				int int_attrs[] =
				{
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_COLOR_BITS_ARB, colorDepth_,
					WGL_DEPTH_BITS_ARB, depthBits_,
					WGL_STENCIL_BITS_ARB, stencilBits_,
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
					WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
					WGL_SAMPLES_ARB, sample_count,
					0, 0
				};

				valid = wglChoosePixelFormatARB(hDC_, int_attrs, float_attrs, 1, &pixelFormat, &num_formats);
				if (!valid || (num_formats < 1))
				{
					-- sample_count;
				}
			} while ((sample_count > 1) && (!valid || (num_formats < 1)));

			if (valid && (sample_count > 1))
			{
				main_wnd->Recreate();

				hWnd_ = main_wnd->HWnd();
				hDC_ = ::GetDC(hWnd_);

				::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);
				::SetWindowPos(hWnd_, NULL, settings.left, settings.top, rc.right - rc.left, rc.bottom - rc.top,
					SWP_SHOWWINDOW | SWP_NOZORDER);

				::SetPixelFormat(hDC_, pixelFormat, &pfd);

				::wglDeleteContext(hRC_);

				hRC_ = ::wglCreateContext(hDC_);
				::wglMakeCurrent(hDC_, hRC_);

				// reinit glloader
				glloader_init();
			}
		}

		if (glloader_WGL_ARB_create_context())
		{
			int attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 0, 0 };
			HGLRC hRC3 = wglCreateContextAttribsARB(hDC_, NULL, attribs);
			if (hRC3 != NULL)
			{
				::wglDeleteContext(hRC_);
				hRC_ = hRC3;

				::wglMakeCurrent(hDC_, hRC_);

				// reinit glloader
				glloader_init();
			}
		}

#elif defined KLAYGE_PLATFORM_LINUX
		if (isFullScreen_)
		{
			colorDepth_ = fs_color_depth_;
			left_ = 0;
			top_ = 0;
		}
		else
		{
			colorDepth_ = fs_color_depth_;
			top_ = settings.top;
			left_ = settings.left;
		}

		x_display_ = main_wnd->XDisplay();
		x_window_ = main_wnd->XWindow();
		x_context_ = main_wnd->XContext();

		uint32_t sample_count = settings.sample_count;

		if (glloader_GLX_ARB_create_context())
		{
			GLXFBConfig* fbc = main_wnd->GetFBC();

			int attribs[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 3, GLX_CONTEXT_MINOR_VERSION_ARB, 0, 0 };
			GLXContext x_context3 = glXCreateContextAttribsARB(x_display_, fbc[0], NULL, GL_TRUE, attribs);
			if (x_context3 != NULL)
			{
				glXDestroyContext(x_display_, x_context_);
				x_context_ = x_context3;

				glXMakeCurrent(x_display_, x_window_, x_context_);

				// reinit glloader
				glloader_init();
			}
		}
#endif

		if (!glloader_GL_VERSION_2_0() || !glloader_GL_EXT_framebuffer_object()
			|| !glloader_GL_ARB_pixel_buffer_object())
		{
			THR(boost::system::posix_error::not_supported);
		}

		glEnable(GL_COLOR_MATERIAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		if (glloader_GL_ARB_color_buffer_float())
		{
			glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
			glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
		}

#if defined KLAYGE_PLATFORM_WINDOWS
		if (glloader_WGL_EXT_swap_control())
		{
			wglSwapIntervalEXT(0);
		}
#elif defined KLAYGE_PLATFORM_LINUX
		if (glloader_GLX_SGI_swap_control())
		{
			glXSwapIntervalSGI(0);
		}
#endif

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
		std::wostringstream oss;
		oss << vendor << L" " << renderer << L" " << version;
		if (sample_count > 1)
		{
			oss << L" (" << sample_count << L"x AA)";
		}
		description_ = oss.str();

		active_ = true;
		ready_ = true;
	}

	OGLRenderWindow::~OGLRenderWindow()
	{
		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		main_wnd->OnActive().disconnect(boost::bind(&OGLRenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().disconnect(boost::bind(&OGLRenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().disconnect(boost::bind(&OGLRenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().disconnect(boost::bind(&OGLRenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().disconnect(boost::bind(&OGLRenderWindow::OnSize, this, _1, _2));
		main_wnd->OnClose().disconnect(boost::bind(&OGLRenderWindow::OnClose, this, _1));

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

#if defined KLAYGE_PLATFORM_WINDOWS
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
#elif defined KLAYGE_PLATFORM_LINUX
			colorDepth_ = fs_color_depth_;
			isFullScreen_ = fs;
			XFlush(x_display_);
#endif
		}
	}

	void OGLRenderWindow::WindowMovedOrResized()
	{
#if defined KLAYGE_PLATFORM_WINDOWS
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
#elif defined KLAYGE_PLATFORM_LINUX
#endif
	}

	void OGLRenderWindow::Destroy()
	{
#if defined KLAYGE_PLATFORM_WINDOWS
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
#elif defined KLAYGE_PLATFORM_LINUX
		if (x_display_ != NULL)
		{
			glXDestroyContext(x_display_, x_context_);
		}
#endif
	}

	void OGLRenderWindow::SwapBuffers()
	{
#if defined KLAYGE_PLATFORM_WINDOWS
		::SwapBuffers(hDC_);
#elif defined KLAYGE_PLATFORM_LINUX
		::glXSwapBuffers(x_display_, x_window_);
#endif
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
