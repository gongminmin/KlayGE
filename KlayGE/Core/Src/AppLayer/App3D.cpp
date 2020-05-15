// App3D.cpp
// KlayGE App3D类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2003-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 移入Core (2008.10.16)
//
// 3.7.0
// 改进了Update (2007.8.14)
//
// 3.6.0
// 增加了MakeWindow (2007.6.26)
//
// 3.1.0
// 增加了OnResize (2005.11.20)
//
// 2.7.0
// 增加了Quit (2005.6.28)
//
// 2.0.0
// 初次建立 (2003.7.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/App3D.hpp>

#if defined(KLAYGE_PLATFORM_WINDOWS_STORE)
#include <future>
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Graphics.Display.Core.h>

namespace uwp
{
	using winrt::auto_revoke;
	using winrt::com_ptr;
	using winrt::event_token;
	using winrt::hstring;
	using winrt::implements;
	using winrt::make;

	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::ApplicationModel;
	using namespace winrt::Windows::ApplicationModel::Activation;
	using namespace winrt::Windows::ApplicationModel::Core;
	using namespace winrt::Windows::Graphics::Display;
	using namespace winrt::Windows::UI::Core;
	using namespace winrt::Windows::UI::Input;
} // namespace uwp
#elif defined(KLAYGE_PLATFORM_ANDROID)
#include <android_native_app_glue.h>
#endif

namespace KlayGE
{
#if defined KLAYGE_PLATFORM_WINDOWS_STORE
	class MetroFrameworkSource;

	class MetroFramework : public uwp::implements<MetroFramework, uwp::IFrameworkView>
	{
	public:
		explicit MetroFramework(App3DFramework* app);

		void Initialize(uwp::CoreApplicationView const& application_view);
		void SetWindow(uwp::CoreWindow const& window);
		void Load(uwp::hstring const& entry_point);
		void Run();
		void Uninitialize();

	private:
		HRESULT OnActivated(uwp::CoreApplicationView const& application_view, uwp::IActivatedEventArgs const& args);
		HRESULT OnSuspending(uwp::IInspectable const& sender, uwp::SuspendingEventArgs const& args);
		HRESULT OnResuming(uwp::IInspectable const& sender, uwp::IInspectable const& args);

		HRESULT OnWindowSizeChanged(uwp::CoreWindow const& sender, uwp::WindowSizeChangedEventArgs const& args);
		HRESULT OnWindowClosed(uwp::CoreWindow const& sender, uwp::CoreWindowEventArgs const& args);
		HRESULT OnVisibilityChanged(uwp::CoreWindow const& sender, uwp::VisibilityChangedEventArgs const& args);
		HRESULT OnKeyDown(uwp::CoreWindow const& sender, uwp::KeyEventArgs const& args);
		HRESULT OnKeyUp(uwp::CoreWindow const& sender, uwp::KeyEventArgs const& args);
		HRESULT OnPointerPressed(uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args);
		HRESULT OnPointerReleased(uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args);
		HRESULT OnPointerMoved(uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args);
		HRESULT OnPointerWheelChanged(uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args);
		HRESULT OnDpiChanged(uwp::DisplayInformation const& sender, uwp::IInspectable const& args);
		HRESULT OnOrientationChanged(uwp::DisplayInformation const& sender, uwp::IInspectable const& args);

	private:
		App3DFramework* app_;
		uwp::CoreApplicationView app_view_{nullptr};
		uwp::CoreWindow window_{nullptr};

		std::future<void> suspend_result_;

		uwp::CoreApplicationView::Activated_revoker app_activated_token_;
		uwp::CoreApplication::Suspending_revoker app_suspending_token_;
		uwp::CoreApplication::Resuming_revoker app_resuming_token_;

		uwp::CoreWindow::SizeChanged_revoker win_size_changed_token_;
		uwp::CoreWindow::VisibilityChanged_revoker visibility_changed_token_;
		uwp::CoreWindow::Closed_revoker win_closed_token_;
		uwp::CoreWindow::KeyDown_revoker key_down_token_;
		uwp::CoreWindow::KeyUp_revoker key_up_token_;
		uwp::CoreWindow::PointerPressed_revoker pointer_pressed_token_;
		uwp::CoreWindow::PointerReleased_revoker pointer_released_token_;
		uwp::CoreWindow::PointerMoved_revoker pointer_moved_token_;
		uwp::CoreWindow::PointerWheelChanged_revoker pointer_wheel_changed_token_;

		uwp::DisplayInformation::DpiChanged_revoker dpi_changed_token_;
		uwp::DisplayInformation::OrientationChanged_revoker orientation_changed_token_;
	};

	class MetroFrameworkSource : public uwp::implements<MetroFrameworkSource, uwp::IFrameworkViewSource>
	{
		friend class App3DFramework;

	public:
		explicit MetroFrameworkSource(App3DFramework* app);

		uwp::IFrameworkView CreateView();

	private:
		App3DFramework* app_;
		uwp::IFrameworkView view_;
	};

	MetroFramework::MetroFramework(App3DFramework* app) : app_(app)
	{
	}

	void MetroFramework::Initialize(uwp::CoreApplicationView const& application_view)
	{
		app_view_ = application_view;

		app_activated_token_ = application_view.Activated(
			uwp::auto_revoke, [this](uwp::CoreApplicationView const& application_view, uwp::IActivatedEventArgs const& args) {
				this->OnActivated(application_view, args);
			});

		app_suspending_token_ = uwp::CoreApplication::Suspending(uwp::auto_revoke,
			[this](uwp::IInspectable const& sender, uwp::SuspendingEventArgs const& args) { this->OnSuspending(sender, args); });
		app_resuming_token_ = uwp::CoreApplication::Resuming(
			uwp::auto_revoke, [this](uwp::IInspectable const& sender, uwp::IInspectable const& args) { this->OnResuming(sender, args); });
	}

	void MetroFramework::SetWindow(uwp::CoreWindow const& window)
	{
		window_ = window;

		win_size_changed_token_ =
			window_.SizeChanged(uwp::auto_revoke, [this](uwp::CoreWindow const& sender, uwp::WindowSizeChangedEventArgs const& args) {
				return this->OnWindowSizeChanged(sender, args);
			});

		visibility_changed_token_ =
			window_.VisibilityChanged(uwp::auto_revoke, [this](uwp::CoreWindow const& sender, uwp::VisibilityChangedEventArgs const& args) {
				return this->OnVisibilityChanged(sender, args);
			});

		win_closed_token_ = window_.Closed(uwp::auto_revoke,
			[this](uwp::CoreWindow const& sender, uwp::CoreWindowEventArgs const& args) { return this->OnWindowClosed(sender, args); });

		auto cursor = uwp::CoreCursor(uwp::CoreCursorType::Arrow, 0);
		window_.PointerCursor(cursor);

		key_down_token_ = window_.KeyDown(uwp::auto_revoke,
			[this](uwp::CoreWindow const& sender, uwp::KeyEventArgs const& args) { return this->OnKeyDown(sender, args); });
		key_up_token_ = window_.KeyUp(
			uwp::auto_revoke, [this](uwp::CoreWindow const& sender, uwp::KeyEventArgs const& args) { return this->OnKeyUp(sender, args); });

		pointer_pressed_token_ = window_.PointerPressed(uwp::auto_revoke,
			[this](uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args) { return this->OnPointerPressed(sender, args); });
		pointer_released_token_ = window_.PointerReleased(uwp::auto_revoke,
			[this](uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args) { return this->OnPointerReleased(sender, args); });
		pointer_moved_token_ = window_.PointerMoved(uwp::auto_revoke,
			[this](uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args) { return this->OnPointerMoved(sender, args); });
		pointer_wheel_changed_token_ = window_.PointerWheelChanged(uwp::auto_revoke,
			[this](uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args) { return this->OnPointerWheelChanged(sender, args); });

		auto disp_info = uwp::DisplayInformation::GetForCurrentView();

		dpi_changed_token_ = disp_info.DpiChanged(uwp::auto_revoke,
			[this](uwp::DisplayInformation const& sender, uwp::IInspectable const& args) { return this->OnDpiChanged(sender, args); });

		orientation_changed_token_ =
			disp_info.OrientationChanged(uwp::auto_revoke, [this](uwp::DisplayInformation const& sender, uwp::IInspectable const& args) {
				return this->OnOrientationChanged(sender, args);
			});

		app_->MainWnd()->SetWindow(window_);
		app_->MetroCreate();
	}

	void MetroFramework::Load(uwp::hstring const& entry_point)
	{
		KFL_UNUSED(entry_point);
	}

	void MetroFramework::Run()
	{
		app_->MetroRun();
	}

	void MetroFramework::Uninitialize()
	{
		auto disp_info = uwp::DisplayInformation::GetForCurrentView();

		dpi_changed_token_.revoke();
		orientation_changed_token_.revoke();

		pointer_wheel_changed_token_.revoke();
		pointer_moved_token_.revoke();
		pointer_released_token_.revoke();
		pointer_pressed_token_.revoke();
		key_up_token_.revoke();
		key_down_token_.revoke();
		win_closed_token_.revoke();
		visibility_changed_token_.revoke();
		win_size_changed_token_.revoke();

		app_suspending_token_.revoke();
		app_suspending_token_.revoke();
		app_activated_token_.revoke();
	}

	HRESULT MetroFramework::OnActivated(uwp::CoreApplicationView const& application_view, uwp::IActivatedEventArgs const& args)
	{
		KFL_UNUSED(application_view);
		KFL_UNUSED(args);

		auto core_win = uwp::CoreWindow::GetForCurrentThread();
		core_win.Activate();

		WindowPtr const & win = app_->MainWnd();
		win->OnActivated();

		return S_OK;
	}

	HRESULT MetroFramework::OnSuspending(uwp::IInspectable const& sender, uwp::SuspendingEventArgs const& args)
	{
		KFL_UNUSED(sender);

		auto deferral = args.SuspendingOperation().GetDeferral();
		suspend_result_ = std::async(std::launch::async, [this, deferral]() {
			app_->Suspend();

			deferral.Complete();
		});

		return S_OK;
	}

	HRESULT MetroFramework::OnResuming(uwp::IInspectable const& sender, uwp::IInspectable const& args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		app_->Resume();

		return S_OK;
	}

	HRESULT MetroFramework::OnWindowSizeChanged(uwp::CoreWindow const& sender, uwp::WindowSizeChangedEventArgs const& args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnSizeChanged(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnVisibilityChanged(uwp::CoreWindow const& sender, uwp::VisibilityChangedEventArgs const& args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnVisibilityChanged(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnWindowClosed(uwp::CoreWindow const& sender, uwp::CoreWindowEventArgs const& args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		WindowPtr const & win = app_->MainWnd();
		win->OnClosed();

		return S_OK;
	}

	HRESULT MetroFramework::OnKeyDown(uwp::CoreWindow const& sender, uwp::KeyEventArgs const& args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnKeyDown(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnKeyUp(uwp::CoreWindow const& sender, uwp::KeyEventArgs const& args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnKeyUp(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnPointerPressed(uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnPointerPressed(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnPointerReleased(uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnPointerReleased(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnPointerMoved(uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnPointerMoved(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnPointerWheelChanged(uwp::CoreWindow const& sender, uwp::PointerEventArgs const& args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnPointerWheelChanged(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnDpiChanged(uwp::DisplayInformation const& sender, uwp::IInspectable const& args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		WindowPtr const & win = app_->MainWnd();
		win->OnDpiChanged();

		return S_OK;
	}

	HRESULT MetroFramework::OnOrientationChanged(uwp::DisplayInformation const& sender, uwp::IInspectable const& args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		WindowPtr const & win = app_->MainWnd();
		win->OnOrientationChanged();

		return S_OK;
	}

	MetroFrameworkSource::MetroFrameworkSource(App3DFramework* app) : view_(uwp::make<MetroFramework>(app))
	{
		app_ = app;
	}

	uwp::IFrameworkView MetroFrameworkSource::CreateView()
	{
		return view_;
	}
#endif

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	App3DFramework::App3DFramework(std::string const & name)
						: App3DFramework(name, nullptr)
	{
	}

	App3DFramework::App3DFramework(std::string const & name, void* native_wnd)
						: name_(name), total_num_frames_(0),
							fps_(0), accumulate_time_(0), num_frames_(0),
							app_time_(0), frame_time_(0)
	{
		Context::Instance().AppInstance(*this);

		ContextCfg cfg = Context::Instance().Config();

		if (cfg.deferred_rendering)
		{
			DeferredRenderingLayer::Register();
		}

		main_wnd_ = this->MakeWindow(name_, cfg.graphics_cfg, native_wnd);
#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
		auto const & win = Context::Instance().AppInstance().MainWnd();
		float const eff_dpi_scale = win->EffectiveDPIScale();
		cfg.graphics_cfg.left = static_cast<uint32_t>(main_wnd_->Left() / eff_dpi_scale + 0.5f);
		cfg.graphics_cfg.top = static_cast<uint32_t>(main_wnd_->Top() / eff_dpi_scale + 0.5f);
		cfg.graphics_cfg.width = static_cast<uint32_t>(main_wnd_->Width() / eff_dpi_scale + 0.5f);
		cfg.graphics_cfg.height = static_cast<uint32_t>(main_wnd_->Height() / eff_dpi_scale + 0.5f);
		Context::Instance().Config(cfg);
#endif
	}

	App3DFramework::~App3DFramework()
	{
		this->Destroy();
	}

	// 建立应用程序主窗口
	/////////////////////////////////////////////////////////////////////////////////
#ifdef KLAYGE_PLATFORM_WINDOWS_STORE
	void App3DFramework::Create()
	{
	}

	void App3DFramework::MetroCreate()
	{
#else
	void App3DFramework::Create()
	{
#endif
		ContextCfg cfg = Context::Instance().Config();
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().CreateRenderWindow(name_,
			cfg.graphics_cfg);
		Context::Instance().Config(cfg);

		this->OnCreate();

		this->OnResize(cfg.graphics_cfg.width, cfg.graphics_cfg.height);
	}

	void App3DFramework::Destroy()
	{
		this->OnDestroy();
		if (Context::Instance().RenderFactoryValid())
		{
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().DestroyRenderWindow();
		}

		main_wnd_.reset();

		Context::Destroy();
	}

	void App3DFramework::Suspend()
	{
		Context::Instance().Suspend();
		this->OnSuspend();
	}

	void App3DFramework::Resume()
	{
		Context::Instance().Resume();
		this->OnResume();
	}

	void App3DFramework::Refresh()
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().Refresh();
	}

	WindowPtr App3DFramework::MakeWindow(std::string const & name, RenderSettings const & settings)
	{
		return MakeSharedPtr<Window>(name, settings, nullptr);
	}

	WindowPtr App3DFramework::MakeWindow(std::string const & name, RenderSettings const & settings, void* native_wnd)
	{
		return MakeSharedPtr<Window>(name, settings, native_wnd);
	}

#if defined KLAYGE_PLATFORM_WINDOWS_STORE
	void App3DFramework::Run()
	{
		uwp::CoreApplication::Run(uwp::make<MetroFrameworkSource>(this));
	}

	void App3DFramework::MetroRun()
#else
	void App3DFramework::Run()
#endif
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		bool gotMsg;
		MSG  msg;

		::PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);

		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if (main_wnd_->Active())
			{
				gotMsg = (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0);
			}
			else
			{
				gotMsg = (::GetMessage(&msg, nullptr, 0, 0) != 0);
			}

			if (gotMsg)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			else
			{
				re.Refresh();
			}
		}
#elif defined KLAYGE_PLATFORM_WINDOWS_STORE
		auto core_win = uwp::CoreWindow::GetForCurrentThread();
		auto dispatcher = core_win.Dispatcher();

		while (!main_wnd_->Closed())
		{
			if (main_wnd_->Active())
			{
				dispatcher.ProcessEvents(uwp::CoreProcessEventsOption::ProcessAllIfPresent);
				re.Refresh();
			}
			else
			{
				dispatcher.ProcessEvents(uwp::CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
#elif defined KLAYGE_PLATFORM_LINUX
		::Display* x_display = main_wnd_->XDisplay();
		XEvent event;
		while (!main_wnd_->Closed())
		{
			do
			{
				XNextEvent(x_display, &event);
				main_wnd_->MsgProc(event);
			} while(XPending(x_display));

			re.Refresh();
		}
#elif defined KLAYGE_PLATFORM_ANDROID
		while (!main_wnd_->Closed())
		{
			// Read all pending events.
			int ident;
			int events;
			android_poll_source* source;

			android_app* state = Context::Instance().AppState();

			do
			{
				ident = ALooper_pollAll(main_wnd_->Active() ? 0 : -1, nullptr, &events, reinterpret_cast<void**>(&source));

				// Process this event.
				if (source != nullptr)
				{
					source->process(state, source);
				}

				// Check if we are exiting.
				if (state->destroyRequested != 0)
				{
					return;
				}
			} while (ident >= 0);

			re.Refresh();
		}
#elif (defined KLAYGE_PLATFORM_DARWIN) || (defined KLAYGE_PLATFORM_IOS)
		while (!main_wnd_->Closed())
		{
			Window::PumpEvents();
			re.Refresh();
		}
#endif

		this->OnDestroy();
	}

	// 获取当前摄像机
	/////////////////////////////////////////////////////////////////////////////////
	Camera const & App3DFramework::ActiveCamera() const
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		CameraPtr const& camera = re.CurFrameBuffer()->Viewport()->Camera();
		BOOST_ASSERT(camera);

		return *camera;
	}

	Camera& App3DFramework::ActiveCamera()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		CameraPtr const& camera = re.CurFrameBuffer()->Viewport()->Camera();
		BOOST_ASSERT(camera);

		return *camera;
	}

	// 设置观察矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::LookAt(float3 const & vEye, float3 const & vLookAt)
	{
		this->LookAt(vEye, vLookAt, float3(0, 1, 0));
	}

	void App3DFramework::LookAt(float3 const & vEye, float3 const & vLookAt,
												float3 const & vUp)
	{
		auto& camera = this->ActiveCamera();
		camera.LookAtDist(MathLib::length(vLookAt - vEye));

		auto& camera_node = *camera.BoundSceneNode();
		camera_node.TransformToWorld(MathLib::inverse(MathLib::look_at_lh(vEye, vLookAt, vUp)));
	}

	// 设置投射矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::Proj(float nearPlane, float farPlane)
	{
		BOOST_ASSERT(nearPlane != 0);
		BOOST_ASSERT(farPlane != 0);

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		FrameBuffer& fb = *re.CurFrameBuffer();

		this->ActiveCamera().ProjParams(re.DefaultFOV(), static_cast<float>(fb.Width()) / fb.Height(),
			nearPlane, farPlane);
	}

	// 退出程序
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::Quit()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		::PostQuitMessage(0);
#endif
#else
		exit(0);
#endif
	}

	// 更新场景
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t App3DFramework::Update(uint32_t pass)
	{
		if (0 == pass)
		{
			this->UpdateStats();
			this->DoUpdateOverlay();

			ResLoader::Instance().Update();
		}

		return this->DoUpdate(pass);
	}

	// 响应窗口大小变化
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::OnResize(uint32_t /*width*/, uint32_t /*height*/)
	{
		this->Proj(this->ActiveCamera().NearPlane(), this->ActiveCamera().FarPlane());
	}

	// 更新状态
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::UpdateStats()
	{
		++ total_num_frames_;

		// measure statistics
		frame_time_ = static_cast<float>(timer_.elapsed());
		++ num_frames_;
		accumulate_time_ += frame_time_;
		app_time_ += frame_time_;

		// check if new second
		if (accumulate_time_ > 1)
		{
			// new second - not 100% precise
			fps_ = num_frames_ / accumulate_time_;

			accumulate_time_ = 0;
			num_frames_  = 0;
		}

		timer_.restart();
	}

	uint32_t App3DFramework::TotalNumFrames() const
	{
		return total_num_frames_;
	}

	// 获取渲染目标的每秒帧数
	/////////////////////////////////////////////////////////////////////////////////
	float App3DFramework::FPS() const
	{
		return fps_;
	}

	float App3DFramework::AppTime() const
	{
		return app_time_;
	}

	float App3DFramework::FrameTime() const
	{
		return frame_time_;
	}
}
