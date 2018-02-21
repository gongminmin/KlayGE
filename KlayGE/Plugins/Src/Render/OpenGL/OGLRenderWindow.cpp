// OGLRenderWindow.cpp
// KlayGE OpenGL渲染窗口类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2004-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 支持OpenGL 3.1 (2009.3.28)
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
#include <KFL/ArrayRef.hpp>

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

		static std::pair<int, int> constexpr all_versions[] =
		{
			std::make_pair(4, 6),
			std::make_pair(4, 5),
			std::make_pair(4, 4),
			std::make_pair(4, 3),
			std::make_pair(4, 2),
			std::make_pair(4, 1)
		};

		ArrayRef<std::pair<int, int>> available_versions;
		{
			static std::string_view const all_version_names[] =
			{
				"4.6",
				"4.5",
				"4.4",
				"4.3",
				"4.2",
				"4.1"
			};
			KLAYGE_STATIC_ASSERT(std::size(all_version_names) == std::size(all_versions));

			uint32_t version_start_index = 0;
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
			left_ = main_wnd->Left();
			top_ = main_wnd->Top();

			style = WS_OVERLAPPEDWINDOW;
		}

		RECT rc = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
		::AdjustWindowRect(&rc, style, false);

		::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);
		::SetWindowPos(hWnd_, nullptr, settings.left, settings.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_SHOWWINDOW | SWP_NOZORDER);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		uint32_t sample_count = settings.sample_count;
		int requested_pixel_format = -1;
		PIXELFORMATDESCRIPTOR requested_pfd{};
		{
			WNDCLASSEXW wc;
			wc.cbSize = sizeof(wc);
			wc.style = CS_OWNDC;
			wc.lpfnWndProc = DefWindowProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = sizeof(this);
			wc.hInstance = ::GetModuleHandle(nullptr);
			wc.hIcon = nullptr;
			wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
			wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
			wc.lpszMenuName = nullptr;
			wc.lpszClassName = L"DummyWindow";
			wc.hIconSm = nullptr;
			::RegisterClassExW(&wc);

			HWND dummy_wnd = ::CreateWindowW(wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, 0, 0, wc.hInstance, nullptr);
			HDC dummy_dc = ::GetDC(dummy_wnd);

			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = static_cast<BYTE>(color_bits_);
			pfd.cDepthBits = static_cast<BYTE>(depth_bits);
			pfd.cStencilBits = static_cast<BYTE>(stencil_bits);
			pfd.iLayerType = PFD_MAIN_PLANE;

			int dummy_pixel_format = ::ChoosePixelFormat(dummy_dc, &pfd);
			BOOST_ASSERT(dummy_pixel_format != 0);

			::SetPixelFormat(dummy_dc, dummy_pixel_format, &pfd);

			HGLRC dummy_rc = re.wglCreateContext(dummy_dc);
			re.wglMakeCurrent(dummy_dc, dummy_rc);

			auto color_fmt = settings.color_fmt;
			if (color_fmt == EF_A2BGR10)
			{
				// TODO: Figure out why A2BGR10 doesn't work
				color_fmt = EF_ABGR16F;
			}

			int r_bits, g_bits, b_bits, a_bits;
			switch (color_fmt)
			{
			case EF_ARGB8:
			case EF_ABGR8:
				r_bits = 8;
				g_bits = 8;
				b_bits = 8;
				a_bits = 8;
				break;

			case EF_A2BGR10:
				r_bits = 10;
				g_bits = 10;
				b_bits = 10;
				a_bits = 2;
				break;

			case EF_ABGR16F:
				r_bits = 16;
				g_bits = 16;
				b_bits = 16;
				a_bits = 16;
				break;

			default:
				KFL_UNREACHABLE("Invalid color format");
			}

			int pixel_format;
			UINT num_formats;
			float float_attrs[] = { 0, 0 };
			BOOL valid;
			do
			{
				std::vector<int> int_attrs =
				{
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_RED_BITS_ARB, r_bits,
					WGL_GREEN_BITS_ARB, g_bits,
					WGL_BLUE_BITS_ARB, b_bits,
					WGL_ALPHA_BITS_ARB, a_bits,
					WGL_DEPTH_BITS_ARB, static_cast<int>(depth_bits),
					WGL_STENCIL_BITS_ARB, static_cast<int>(stencil_bits),
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE
				};

				if (IsFloatFormat(color_fmt))
				{
					int_attrs.push_back(WGL_PIXEL_TYPE_ARB);
					int_attrs.push_back(WGL_TYPE_RGBA_FLOAT_ARB);
				}
				if (sample_count > 1)
				{
					int_attrs.push_back(WGL_SAMPLE_BUFFERS_ARB);
					int_attrs.push_back(GL_TRUE);
					int_attrs.push_back(WGL_SAMPLES_ARB);
					int_attrs.push_back(static_cast<int>(sample_count));
				}
				if (settings.stereo_method == STM_LCDShutter)
				{
					int_attrs.push_back(WGL_STEREO_ARB);
					int_attrs.push_back(GL_TRUE);
				}
				int_attrs.push_back(0);
				int_attrs.push_back(0);

				valid = wglChoosePixelFormatARB(dummy_dc, &int_attrs[0], float_attrs, 1, &pixel_format, &num_formats);
				if (valid && (num_formats > 0))
				{
					break;
				}
				else
				{
					-- sample_count;
				}
			} while (sample_count > 0);

			if (valid && (sample_count > 0))
			{
				requested_pixel_format = pixel_format;
				requested_pfd = pfd;

				re.wglMakeCurrent(dummy_dc, nullptr);
				re.wglDeleteContext(dummy_rc);
				::ReleaseDC(dummy_wnd, dummy_dc);
				::DestroyWindow(dummy_wnd);
			}
		}

		::SetPixelFormat(hDC_, requested_pixel_format, &requested_pfd);
		hRC_ = re.wglCreateContext(hDC_);
		re.wglMakeCurrent(hDC_, hRC_);

		if (glloader_WGL_ARB_create_context())
		{
			int flags = 0;
#ifndef KLAYGE_SHIP
			flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

			int attribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, 0,
				WGL_CONTEXT_MINOR_VERSION_ARB, 0,
				WGL_CONTEXT_FLAGS_ARB, flags,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			};
			for (size_t i = 0; i < available_versions.size(); ++ i)
			{
				attribs[1] = available_versions[i].first;
				attribs[3] = available_versions[i].second;
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
			left_ = main_wnd->Left();
			top_ = main_wnd->Top();
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
			int attribs[] =
			{
				GLX_CONTEXT_MAJOR_VERSION_ARB, 0,
				GLX_CONTEXT_MINOR_VERSION_ARB, 0,
				0
			};
			for (size_t i = 0; i < available_versions.size(); ++ i)
			{
				attribs[1] = available_versions[i].first;
				attribs[3] = available_versions[i].second;
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
			left_ = main_wnd->Left();
			top_ = main_wnd->Top();
		}
		
		main_wnd->CreateGLView(settings);
		glloader_init();

		uint32_t sample_count = settings.sample_count;
#endif

		if (!glloader_GL_VERSION_4_1())
		{
			TERRC(std::errc::function_not_supported);
		}

		glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
		glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
		glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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

	// 改变窗口大小
	/////////////////////////////////////////////////////////////////////////////////
	void OGLRenderWindow::Resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_->width = width;
		viewport_->height = height;
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
#elif defined KLAYGE_PLATFORM_DARWIN
		uint2 screen = Context::Instance().AppInstance().MainWnd()->GetNSViewSize();
		uint32_t new_width = screen[0];
		uint32_t new_height = screen[1];
#endif

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
