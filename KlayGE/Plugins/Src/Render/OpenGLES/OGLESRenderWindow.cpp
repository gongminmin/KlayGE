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
#include <KFL/CXX17.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
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
#include <system_error>
#include <boost/assert.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

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
		on_exit_size_move_connect_ = main_wnd->OnExitSizeMove().connect(
			[this](Window const & win)
			{
				this->OnExitSizeMove(win);
			});
		on_size_connect_ = main_wnd->OnSize().connect(
			[this](Window const & win, bool active)
			{
				this->OnSize(win, active);
			});

		if (isFullScreen_)
		{
			left_ = 0;
			top_ = 0;
		}
		else
		{
			left_ = main_wnd->Left();
			top_ = main_wnd->Top();
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
			KFL_UNREACHABLE("Invalid color format");
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

		static std::pair<int, int> constexpr all_versions[] =
		{
			std::make_pair(3, 2),
			std::make_pair(3, 1),
			std::make_pair(3, 0)
		};

		ArrayRef<std::pair<int, int>> available_versions;
		{
			static std::string_view const all_version_names[] =
			{
				"3.2",
				"3.1",
				"3.0"
			};
			KLAYGE_STATIC_ASSERT(std::size(all_version_names) == std::size(all_versions));

			bool test_es_3_2 = true;
			bool test_es_3_1 = true;
#if defined(KLAYGE_PLATFORM_ANDROID)
			// TODO
			test_es_3_2 = false;
#endif
#if defined(KLAYGE_PLATFORM_DARWIN)
			// TODO
			test_es_3_2 = false;
			test_es_3_1 = false;
#endif

			uint32_t version_start_index = 0;
			if (!test_es_3_2)
			{
				version_start_index = 1;
			}
			if (!test_es_3_1)
			{
				version_start_index = 2;
			}

			for (size_t index = 0; index < settings.options.size(); ++ index)
			{
				std::string_view opt_name = settings.options[index].first;
				std::string_view opt_val = settings.options[index].second;
				if ("version" == opt_name)
				{
					for (uint32_t i = version_start_index; i < std::size(all_version_names); ++ i)
					{
						if (all_version_names[i] == opt_val)
						{
							version_start_index = i;
							break;
						}
					}
				}
			}

			available_versions = ArrayRef<std::pair<int, int>>(all_versions).Slice(version_start_index);
		}

		std::vector<EGLint> visual_attr =
		{
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
			EGL_RED_SIZE, r_size,
			EGL_GREEN_SIZE, g_size,
			EGL_BLUE_SIZE, b_size,
			EGL_ALPHA_SIZE, a_size
		};
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
		eglInitialize(display_, &egl_major_ver, &egl_minor_ver);

		EGLint num_cfgs;
		if (eglChooseConfig(display_, &visual_attr[0], &cfg_, 1, &num_cfgs))
		{
			if ((num_cfgs == 0) && (24 == d_size))
			{
				visual_attr[11] = 16;
				eglChooseConfig(display_, &visual_attr[0], &cfg_, 1, &num_cfgs);
			}
		}

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
		EGLint ctx_attr[] =
		{
			EGL_CONTEXT_MAJOR_VERSION_KHR, available_versions[0].first,
			EGL_CONTEXT_MINOR_VERSION_KHR, available_versions[0].second,
			EGL_NONE
		};
		size_t test_version_index = 0;
		while ((nullptr == context_) && (test_version_index < available_versions.size()))
		{
			ctx_attr[1] = available_versions[test_version_index].first;
			ctx_attr[2] = EGL_CONTEXT_MINOR_VERSION_KHR;
			ctx_attr[3] = available_versions[test_version_index].second;
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

		if (!glloader_GLES_VERSION_3_0())
		{
			TERRC(std::errc::function_not_supported);
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
		KFL_UNUSED(win);

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
#elif defined KLAYGE_PLATFORM_LINUX
		int screen = DefaultScreen(x_display_);
		uint32_t new_width = DisplayWidth(x_display_, screen);
		uint32_t new_height = DisplayHeight(x_display_, screen);
#elif defined KLAYGE_PLATFORM_ANDROID
		EGLint w, h;
		eglQuerySurface(display_, surf_, EGL_WIDTH, &w);
		eglQuerySurface(display_, surf_, EGL_HEIGHT, &h);

		uint32_t new_width = w;
		uint32_t new_height = h;
#elif defined KLAYGE_PLATFORM_DARWIN
		uint2 screen = Context::Instance().AppInstance().MainWnd()->GetNSViewSize();
		uint32_t new_width = screen[0];
		uint32_t new_height = screen[1];
#elif defined KLAYGE_PLATFORM_IOS
		uint2 screen = Context::Instance().AppInstance().MainWnd()->GetGLKViewSize();
		uint32_t new_width = screen[0];
		uint32_t new_height = screen[1];
#endif

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
