// OGLRenderWindow.cpp
// KlayGE OpenGL��Ⱦ������ ʵ���ļ�
// Ver 3.9.0
// ��Ȩ����(C) ������, 2004-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// ֧��OpenGL 3.1 (2009.3.28)
//
// 3.7.0
// ʵ���Ե�linux֧�� (2008.5.19)
//
// 3.6.0
// ��������Vista�´�ȫ��ģʽ�˳�ʱcrash��bug (2007.3.23)
// ֧�ֶ�̬�л�ȫ��/����ģʽ (2007.3.24)
//
// 2.8.0
// ֻ֧��OpenGL 1.5������ (2005.8.12)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <map>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderWindow.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>

namespace KlayGE
{
	OGLRenderWindow::OGLRenderWindow(std::string const & name, RenderSettings const & settings)
						: OGLFrameBuffer(false)
	{
		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isFullScreen_		= settings.full_screen;
		color_bits_ = NumFormatBits(settings.color_fmt);

		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
		on_exit_size_move_connect_ = main_wnd->OnExitSizeMove().connect(std::bind(&OGLRenderWindow::OnExitSizeMove, this,
			std::placeholders::_1));
		on_size_connect_ = main_wnd->OnSize().connect(std::bind(&OGLRenderWindow::OnSize, this,
			std::placeholders::_1, std::placeholders::_2));

		std::vector<std::pair<std::string, std::pair<int, int>>> available_versions;
		available_versions.emplace_back("4.5", std::make_pair(4, 5));
		available_versions.emplace_back("4.4", std::make_pair(4, 4));
		available_versions.emplace_back("4.3", std::make_pair(4, 3));
		available_versions.emplace_back("4.2", std::make_pair(4, 2));
		available_versions.emplace_back("4.1", std::make_pair(4, 1));
		available_versions.emplace_back("4.0", std::make_pair(4, 0));
		available_versions.emplace_back("3.3", std::make_pair(3, 3));
		available_versions.emplace_back("3.2", std::make_pair(3, 2));
		available_versions.emplace_back("3.1", std::make_pair(3, 1));
		available_versions.emplace_back("3.0", std::make_pair(3, 0));

		for (size_t index = 0; index < settings.options.size(); ++ index)
		{
			std::string const & opt_name = settings.options[index].first;
			std::string const & opt_val = settings.options[index].second;
			if (0 == strcmp("version", opt_name.c_str()))
			{
				size_t feature_index = 0;
				for (size_t i = 0; i < available_versions.size(); ++ i)
				{
					if (available_versions[i].first == opt_val)
					{
						feature_index = i;
						break;
					}
				}

				if (feature_index > 0)
				{
					available_versions.erase(available_versions.begin(),
						available_versions.begin() + feature_index);
				}
			}
		}

#if defined KLAYGE_PLATFORM_WINDOWS
		uint32_t depth_bits	= NumDepthBits(settings.depth_stencil_fmt);
		uint32_t stencil_bits = NumStencilBits(settings.depth_stencil_fmt);

		hWnd_ = main_wnd->HWnd();
		hDC_ = ::GetDC(hWnd_);

		uint32_t style;
		if (isFullScreen_)
		{
			left_ = 0;
			top_ = 0;

			DEVMODE devMode;
			devMode.dmSize = sizeof(devMode);
			devMode.dmBitsPerPel = color_bits_;
			devMode.dmPelsWidth = width_;
			devMode.dmPelsHeight = height_;
			devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			::ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);

			style = WS_POPUP;
		}
		else
		{
			// Get colour depth from display
			top_ = settings.top;
			left_ = settings.left;

			style = WS_OVERLAPPEDWINDOW;
		}

		RECT rc = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
		::AdjustWindowRect(&rc, style, false);

		::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);
		::SetWindowPos(hWnd_, nullptr, settings.left, settings.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_SHOWWINDOW | SWP_NOZORDER);

		// there is no guarantee that the contents of the stack that become
		// the pfd are zeroed, therefore _make sure_ to clear these bits.
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize		= sizeof(pfd);
		pfd.nVersion	= 1;
		pfd.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		if (STM_LCDShutter == settings.stereo_method)
		{
			pfd.dwFlags |= PFD_STEREO;
		}
		pfd.iPixelType	= PFD_TYPE_RGBA;
		pfd.cColorBits	= static_cast<BYTE>(color_bits_);
		pfd.cDepthBits	= static_cast<BYTE>(depth_bits);
		pfd.cStencilBits = static_cast<BYTE>(stencil_bits);
		pfd.iLayerType	= PFD_MAIN_PLANE;

		int pixelFormat = ::ChoosePixelFormat(hDC_, &pfd);
		BOOST_ASSERT(pixelFormat != 0);

		::SetPixelFormat(hDC_, pixelFormat, &pfd);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		hRC_ = re.wglCreateContext(hDC_);
		re.wglMakeCurrent(hDC_, hRC_);

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
					WGL_COLOR_BITS_ARB, static_cast<int>(color_bits_),
					WGL_DEPTH_BITS_ARB, static_cast<int>(depth_bits),
					WGL_STENCIL_BITS_ARB, static_cast<int>(stencil_bits),
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
					WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
					WGL_SAMPLES_ARB, static_cast<int>(sample_count),
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
				re.wglMakeCurrent(hDC_, nullptr);
				re.wglDeleteContext(hRC_);
				::ReleaseDC(hWnd_, hDC_);

				main_wnd->Recreate();

				hWnd_ = main_wnd->HWnd();
				hDC_ = ::GetDC(hWnd_);

				::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);
				::SetWindowPos(hWnd_, nullptr, settings.left, settings.top, rc.right - rc.left, rc.bottom - rc.top,
					SWP_SHOWWINDOW | SWP_NOZORDER);

				::SetPixelFormat(hDC_, pixelFormat, &pfd);

				hRC_ = re.wglCreateContext(hDC_);
				re.wglMakeCurrent(hDC_, hRC_);

				// reinit glloader
				glloader_init();
			}
		}

		if (glloader_WGL_ARB_create_context())
		{
			int flags = 0;
#ifndef KLAYGE_SHIP
			flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

			int attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 0, WGL_CONTEXT_MINOR_VERSION_ARB, 0,
				WGL_CONTEXT_FLAGS_ARB, flags, WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0 };
			for (size_t i = 0; i < available_versions.size(); ++ i)
			{
				attribs[1] = available_versions[i].second.first;
				attribs[3] = available_versions[i].second.second;
				HGLRC hRC_new = wglCreateContextAttribsARB(hDC_, nullptr, attribs);
				if (hRC_new != nullptr)
				{
					re.wglMakeCurrent(hDC_, nullptr);
					re.wglDeleteContext(hRC_);
					hRC_ = hRC_new;

					re.wglMakeCurrent(hDC_, hRC_);

					// reinit glloader
					glloader_init();

					break;
				}
			}
		}

#elif defined KLAYGE_PLATFORM_LINUX
		if (isFullScreen_)
		{
			left_ = 0;
			top_ = 0;
		}
		else
		{
			top_ = settings.top;
			left_ = settings.left;
		}

		x_display_ = main_wnd->XDisplay();
		x_window_ = main_wnd->XWindow();
		XVisualInfo* vi = main_wnd->VisualInfo();

		// Create an OpenGL rendering context
		x_context_ = glXCreateContext(x_display_, vi,
					nullptr,		// No sharing of display lists
					GL_TRUE);	// Direct rendering if possible

		glXMakeCurrent(x_display_, x_window_, x_context_);

		glloader_init();

		uint32_t sample_count = settings.sample_count;

		if (glloader_GLX_ARB_create_context())
		{
			int attribs[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 0, GLX_CONTEXT_MINOR_VERSION_ARB, 0, 0 };
			for (size_t i = 0; i < available_versions.size(); ++ i)
			{
				attribs[1] = available_versions[i].second.first;
				attribs[3] = available_versions[i].second.second;
				GLXContext x_context_new = glXCreateContextAttribsARB(x_display_, nullptr, nullptr, GL_TRUE, attribs);
				if (x_context_new != nullptr)
				{
					glXMakeCurrent(x_display_, x_window_, nullptr);
					glXDestroyContext(x_display_, x_context_);
					x_context_ = x_context_new;

					glXMakeCurrent(x_display_, x_window_, x_context_);

					// reinit glloader
					glloader_init();

					break;
				}
			}
		}
#elif defined KLAYGE_PLATFORM_DARWIN	
		if (isFullScreen_)
		{
			left_ = 0;
			top_ = 0;
		}
		else
		{
			left_ = settings.left;
			top_ = settings.top;
		}
		
		main_wnd->CreateGLView(settings);
		glloader_init();

		uint32_t sample_count = settings.sample_count;
#endif

		if (!glloader_GL_VERSION_3_0()
			&& (!glloader_GL_VERSION_2_1()
				|| !(glloader_GL_ARB_framebuffer_object()
					|| (glloader_GL_EXT_framebuffer_object()
						&& glloader_GL_EXT_framebuffer_blit()))))
		{
			THR(errc::function_not_supported);
		}

		if (glloader_GL_VERSION_3_0())
		{
			glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
			glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
			glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
		}
		else if (glloader_GL_ARB_color_buffer_float())
		{
			glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
			glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
		}

		if (glloader_GL_VERSION_3_2() || glloader_GL_ARB_seamless_cube_map())
		{
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}

#if defined KLAYGE_PLATFORM_WINDOWS
		if (glloader_WGL_EXT_swap_control())
		{
			wglSwapIntervalEXT(settings.sync_interval);
		}
#elif defined KLAYGE_PLATFORM_LINUX
		if (glloader_GLX_SGI_swap_control())
		{
			glXSwapIntervalSGI(settings.sync_interval);
		}
#endif

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		viewport_->left = 0;
		viewport_->top = 0;
		viewport_->width = width_;
		viewport_->height = height_;

		std::wstring vendor, renderer, version;
		Convert(vendor, reinterpret_cast<char const *>(glGetString(GL_VENDOR)));
		Convert(renderer, reinterpret_cast<char const *>(glGetString(GL_RENDERER)));
		Convert(version, reinterpret_cast<char const *>(glGetString(GL_VERSION)));
		description_ = vendor + L" " + renderer + L" " + version;
		if (sample_count > 1)
		{
			description_ += L" (" + boost::lexical_cast<std::wstring>(sample_count) + L"x AA)";
		}
	}

	OGLRenderWindow::~OGLRenderWindow()
	{
		on_exit_size_move_connect_.disconnect();
		on_size_connect_.disconnect();

		this->Destroy();
	}

	std::wstring const & OGLRenderWindow::Description() const
	{
		return description_;
	}

	// �ı䴰�ڴ�С
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderWindow::Resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_->width = width;
		viewport_->height = height;

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);
	}

	// �ı䴰��λ��
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderWindow::Reposition(uint32_t left, uint32_t top)
	{
		left_ = left;
		top_ = top;
	}

	// ��ȡ�Ƿ���ȫ��״̬
	/////////////////////////////////////////////////////////////////////////////////
	bool OGLRenderWindow::FullScreen() const
	{
		return isFullScreen_;
	}

	// �����Ƿ���ȫ��״̬
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
				DEVMODE devMode;
				devMode.dmSize = sizeof(devMode);
				devMode.dmBitsPerPel = color_bits_;
				devMode.dmPelsWidth = width_;
				devMode.dmPelsHeight = height_;
				devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				::ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);

				style = WS_POPUP;
			}
			else
			{
				::ChangeDisplaySettings(nullptr, 0);

				style = WS_OVERLAPPEDWINDOW;
			}

			::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);

			RECT rc = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
			::AdjustWindowRect(&rc, style, false);
			width_ = rc.right - rc.left;
			height_ = rc.bottom - rc.top;

			isFullScreen_ = fs;

			::ShowWindow(hWnd_, SW_SHOWNORMAL);
			::UpdateWindow(hWnd_);
#elif defined KLAYGE_PLATFORM_LINUX
			isFullScreen_ = fs;
			XFlush(x_display_);
#endif
		}
	}

	void OGLRenderWindow::WindowMovedOrResized()
	{
		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
		float const dpi_scale = main_wnd->DPIScale();

#if defined KLAYGE_PLATFORM_WINDOWS
		::RECT rect;
		::GetClientRect(hWnd_, &rect);

		uint32_t new_left = static_cast<uint32_t>(rect.left * dpi_scale + 0.5);
		uint32_t new_top = static_cast<uint32_t>(rect.top * dpi_scale + 0.5f);
		if ((new_left != left_) || (new_top != top_))
		{
			this->Reposition(new_left, new_top);
		}

		uint32_t new_width = rect.right - rect.left;
		uint32_t new_height = rect.bottom - rect.top;
#elif defined KLAYGE_PLATFORM_LINUX
		int screen = DefaultScreen(x_display_);
		uint32_t new_width = DisplayWidth(x_display_, screen);
		uint32_t new_height = DisplayHeight(x_display_, screen);
#elif defined KLAYGE_PLATFORM_DARWIN
		uint2 screen = Context::Instance().AppInstance().MainWnd()->GetNSViewSize();
		uint32_t new_width = screen[0];
		uint32_t new_height = screen[1];
#endif

		new_width = static_cast<uint32_t>(new_width * dpi_scale + 0.5f);
		new_height = static_cast<uint32_t>(new_height * dpi_scale + 0.5f);

		if ((new_width != width_) || (new_height != height_))
		{
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(new_width, new_height);
		}
	}

	void OGLRenderWindow::Destroy()
	{
#if defined KLAYGE_PLATFORM_WINDOWS
		if (hWnd_ != nullptr)
		{
			if (hDC_ != nullptr)
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				re.wglMakeCurrent(hDC_, nullptr);
				if (hRC_ != nullptr)
				{
					re.wglDeleteContext(hRC_);
					hRC_ = nullptr;
				}
				::ReleaseDC(hWnd_, hDC_);
				hDC_ = nullptr;
			}

			if (isFullScreen_)
			{
				::ChangeDisplaySettings(nullptr, 0);
				ShowCursor(TRUE);
			}
		}
#elif defined KLAYGE_PLATFORM_LINUX
		if (x_display_ != nullptr)
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
#elif defined KLAYGE_PLATFORM_DARWIN
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.ScreenFrameBuffer());
		Context::Instance().AppInstance().MainWnd()->FlushBuffer();
		re.BindFrameBuffer(re.DefaultFrameBuffer());
#endif
	}

	void OGLRenderWindow::OnExitSizeMove(Window const & /*win*/)
	{
		this->WindowMovedOrResized();
	}

	void OGLRenderWindow::OnSize(Window const & win, bool active)
	{
		if (active)
		{
			if (win.Ready())
			{
				this->WindowMovedOrResized();
			}
		}
	}
}
