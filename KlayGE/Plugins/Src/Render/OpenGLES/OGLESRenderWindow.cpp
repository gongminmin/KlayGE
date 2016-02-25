// OGLESRenderWindow.cpp
// KlayGE OpenGL ES 2渲染窗口类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
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

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESRenderWindow.hpp>
#ifdef KLAYGE_PLATFORM_IOS
#include <KlayGE/OpenGLES/OGLESRenderView.hpp>
#endif

namespace KlayGE
{
	OGLESRenderWindow::OGLESRenderWindow(std::string const & name, RenderSettings const & settings)
#if defined(KLAYGE_PLATFORM_IOS)
						: OGLESFrameBuffer(true)
#else
						: OGLESFrameBuffer(false)
#endif
	{
		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isFullScreen_		= settings.full_screen;
		color_bits_			= NumFormatBits(settings.color_fmt);

		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();		
		on_exit_size_move_connect_ = main_wnd->OnExitSizeMove().connect(std::bind(&OGLESRenderWindow::OnExitSizeMove, this,
			std::placeholders::_1));
		on_size_connect_ = main_wnd->OnSize().connect(std::bind(&OGLESRenderWindow::OnSize, this,
			std::placeholders::_1, std::placeholders::_2));

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

#if !(defined KLAYGE_PLATFORM_IOS)
#if defined KLAYGE_PLATFORM_DARWIN
		main_wnd->CreateGLESView();
#endif

		display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);

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

		bool test_es_3_1 = true;
		bool test_es_3_0 = true;
#if defined(KLAYGE_PLATFORM_ANDROID)
		test_es_3_1 = false;
#if (__ANDROID_API__ < 18)
		test_es_3_0 = false;
#endif
#endif
#if defined(KLAYGE_PLATFORM_DARWIN)
		test_es_3_1 = false;
		test_es_3_0 = false;
#endif

		std::vector<std::tuple<std::string, EGLint, int, int>> available_versions;
		if (test_es_3_1)
		{
			available_versions.push_back(std::make_tuple("3.1", EGL_OPENGL_ES3_BIT_KHR, 3, 1));
		}
		if (test_es_3_0)
		{
			available_versions.push_back(std::make_tuple("3.0", EGL_OPENGL_ES3_BIT_KHR, 3, 0));
		}
		available_versions.push_back(std::make_tuple("2.0", EGL_OPENGL_ES2_BIT, 2, 0));

		for (size_t index = 0; index < settings.options.size(); ++ index)
		{
			std::string const & opt_name = settings.options[index].first;
			std::string const & opt_val = settings.options[index].second;
			if (0 == strcmp("version", opt_name.c_str()))
			{
				size_t feature_index = 0;
				for (size_t i = 0; i < available_versions.size(); ++ i)
				{
					if (std::get<0>(available_versions[i]) == opt_val)
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

		std::vector<EGLint> visual_attr;
		visual_attr.push_back(EGL_RENDERABLE_TYPE);
		visual_attr.push_back(std::get<1>(available_versions[0]));
		visual_attr.push_back(EGL_RED_SIZE);
		visual_attr.push_back(r_size);
		visual_attr.push_back(EGL_GREEN_SIZE);
		visual_attr.push_back(g_size);
		visual_attr.push_back(EGL_BLUE_SIZE);
		visual_attr.push_back(b_size);
		visual_attr.push_back(EGL_ALPHA_SIZE);
		visual_attr.push_back(a_size);
		if (d_size > 0)
		{
			visual_attr.push_back(EGL_DEPTH_SIZE);
			visual_attr.push_back(d_size);
		}
		if (s_size > 0)
		{
			visual_attr.push_back(EGL_STENCIL_SIZE);
			visual_attr.push_back(s_size);
		}
		if (settings.sample_count > 1)
		{
			visual_attr.push_back(EGL_SAMPLES);
			visual_attr.push_back(settings.sample_count);
		}
		visual_attr.push_back(EGL_NONE);				// end of list

		EGLint egl_major_ver, egl_minor_ver;
		EGLint num_cfgs;
		eglInitialize(display_, &egl_major_ver, &egl_minor_ver);

		int start_version_index = -1;
		for (size_t i = 0; i < available_versions.size(); ++ i)
		{
			visual_attr[1] = std::get<1>(available_versions[i]);
			if (eglChooseConfig(display_, &visual_attr[0], &cfg_, 1, &num_cfgs))
			{
				if (num_cfgs > 0)
				{
					start_version_index = static_cast<int>(i);
					break;
				}
			}
		}
		if ((start_version_index < 0) && (24 == d_size))
		{
			visual_attr[11] = 16;
			for (size_t i = 0; i < available_versions.size(); ++ i)
			{
				visual_attr[1] = std::get<1>(available_versions[i]);
				if (eglChooseConfig(display_, &visual_attr[0], &cfg_, 1, &num_cfgs))
				{
					if (num_cfgs > 0)
					{
						start_version_index = static_cast<int>(i);
						break;
					}
				}
			}
		}
		BOOST_ASSERT(start_version_index != -1);

		NativeWindowType wnd;
#if defined KLAYGE_PLATFORM_WINDOWS
		wnd = hWnd_ = main_wnd->HWnd();
#elif defined KLAYGE_PLATFORM_LINUX
		wnd = x_window_ = main_wnd->XWindow();
#elif defined KLAYGE_PLATFORM_ANDROID
		wnd = a_window_ = main_wnd->AWindow();
		EGLint format;
		eglGetConfigAttrib(display_, cfg_, EGL_NATIVE_VISUAL_ID, &format);
		ANativeWindow_setBuffersGeometry(wnd, 0, 0, format);
#elif defined KLAYGE_PLATFORM_DARWIN
		wnd = reinterpret_cast<EGLNativeWindowType>(main_wnd->NSView());
#endif

		surf_ = eglCreateWindowSurface(display_, cfg_, wnd, nullptr);

		context_ = nullptr;
		EGLint ctx_attr[] = { EGL_CONTEXT_MAJOR_VERSION_KHR, std::get<2>(available_versions[start_version_index]),
			EGL_CONTEXT_MINOR_VERSION_KHR, std::get<3>(available_versions[start_version_index]), EGL_NONE };
		size_t test_version_index = start_version_index;
		while ((nullptr == context_) && (test_version_index < available_versions.size()))
		{
			ctx_attr[1] = std::get<2>(available_versions[test_version_index]);
			ctx_attr[2] = EGL_CONTEXT_MINOR_VERSION_KHR;
			ctx_attr[3] = std::get<3>(available_versions[test_version_index]);
			context_ = eglCreateContext(display_, cfg_, EGL_NO_CONTEXT, ctx_attr);

			if (nullptr == context_)
			{
				if (0 == ctx_attr[3])
				{
					ctx_attr[2] = EGL_NONE;
				}
				context_ = eglCreateContext(display_, cfg_, EGL_NO_CONTEXT, ctx_attr);
			}

			++ test_version_index;
		}
		BOOST_ASSERT(context_ != nullptr);

		eglMakeCurrent(display_, surf_, surf_, context_);

		if (!glloader_GLES_VERSION_2_0())
		{
			THR(errc::function_not_supported);
		}

		eglSwapInterval(display_, 0);
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
		if (settings.sample_count > 1)
		{
			description_ += L" (" + boost::lexical_cast<std::wstring>(settings.sample_count) + L"x AA)";
		}
	}

	OGLESRenderWindow::~OGLESRenderWindow()
	{
		on_exit_size_move_connect_.disconnect();
		on_size_connect_.disconnect();

		this->Destroy();
	}

	std::wstring const & OGLESRenderWindow::Description() const
	{
		return description_;
	}

	// 改变窗口大小
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderWindow::Resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_->width = width;
		viewport_->height = height;

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);
	}

	// 改变窗口位置
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderWindow::Reposition(uint32_t left, uint32_t top)
	{
		left_ = left;
		top_ = top;
	}

	// 获取是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	bool OGLESRenderWindow::FullScreen() const
	{
		return isFullScreen_;
	}

	// 设置是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESRenderWindow::FullScreen(bool fs)
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
#elif defined KLAYGE_PLATFORM_ANDROID
			isFullScreen_ = fs;
#endif
		}
	}

	void OGLESRenderWindow::WindowMovedOrResized(Window const & win)
	{
		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
		float const dpi_scale = main_wnd->DPIScale();

#if defined KLAYGE_PLATFORM_WINDOWS
		KFL_UNUSED(win);

		::RECT rect;
		::GetClientRect(hWnd_, &rect);

		uint32_t new_left = static_cast<uint32_t>(rect.left * dpi_scale + 0.5f);
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
#elif defined KLAYGE_PLATFORM_ANDROID
		// TODO: Is it correct?
		uint32_t new_left = win.Left() / 2;
		uint32_t new_top = win.Top() / 2;
		if ((new_left != left_) || (new_top != top_))
		{
			this->Reposition(new_left, new_top);
		}

		EGLint w, h;
		eglQuerySurface(display_, surf_, EGL_WIDTH, &w);
		eglQuerySurface(display_, surf_, EGL_HEIGHT, &h);

		uint32_t new_width = w - new_left;
		uint32_t new_height = h - new_top;
#elif defined KLAYGE_PLATFORM_DARWIN
		KFL_UNUSED(win);
		uint2 screen = Context::Instance().AppInstance().MainWnd()->GetNSViewSize();
		uint32_t new_width = screen[0];
		uint32_t new_height = screen[1];
#elif defined KLAYGE_PLATFORM_IOS
		KFL_UNUSED(win);
		uint2 screen = Context::Instance().AppInstance().MainWnd()->GetGLKViewSize();
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

	void OGLESRenderWindow::Destroy()
	{
#if defined KLAYGE_PLATFORM_WINDOWS
		if (hWnd_ != nullptr)
		{
			if (isFullScreen_)
			{
				::ChangeDisplaySettings(nullptr, 0);
				::ShowCursor(TRUE);
			}
		}
#elif defined KLAYGE_PLATFORM_LINUX
#elif defined KLAYGE_PLATFORM_ANDROID
#endif

#ifndef KLAYGE_PLATFORM_IOS
		if (display_ != nullptr)
		{
			eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroyContext(display_, context_);
			eglTerminate(display_);

			display_ = nullptr;
		}
#endif
	}

	void OGLESRenderWindow::SwapBuffers()
	{
#ifdef KLAYGE_PLATFORM_IOS
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.ScreenFrameBuffer());
		checked_pointer_cast<OGLESEAGLRenderView>(this->Attached(FrameBuffer::ATT_Color0))->BindRenderBuffer();
		Context::Instance().AppInstance().MainWnd()->FlushBuffer();
		re.BindFrameBuffer(re.DefaultFrameBuffer());
#else
		eglSwapBuffers(display_, surf_);
#endif
	}

	void OGLESRenderWindow::OnExitSizeMove(Window const & win)
	{
		this->WindowMovedOrResized(win);
	}

	void OGLESRenderWindow::OnSize(Window const & win, bool active)
	{
		if (active)
		{
			if (win.Ready())
			{
				this->WindowMovedOrResized(win);
			}
		}
	}
}
