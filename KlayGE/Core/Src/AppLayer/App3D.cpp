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
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
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

#ifdef KLAYGE_PLATFORM_WINDOWS_RUNTIME
#include <ppl.h>
#include <ppltasks.h>

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace concurrency;
#endif

namespace KlayGE
{
#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
	ref class MetroFrameworkSource;
	ref class MetroMsgs;

	ref class MetroFramework sealed : public Windows::ApplicationModel::Core::IFrameworkView
	{
		friend class App3DFramework;
		friend MetroFrameworkSource;

	public:
		MetroFramework();
	
		virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ application_view);
		virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
		virtual void Load(Platform::String^ entryPoint);
		virtual void Run();
		virtual void Uninitialize();

	private:
		void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ application_view, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);

		void BindAppFramework(App3DFramework* app);

	private:
		App3DFramework* app_;
		MetroMsgs^ msgs_;
		Platform::Agile<CoreApplicationView> app_view_;
		Platform::Agile<CoreWindow> window_;

		EventRegistrationToken app_activated_token_;
		EventRegistrationToken app_suspending_token_;
		EventRegistrationToken app_resuming_token_;

		EventRegistrationToken win_size_changed_token_;
		EventRegistrationToken visibility_changed_token_;
		EventRegistrationToken win_closed_token_;
		EventRegistrationToken pointer_pressed_token_;
		EventRegistrationToken pointer_released_token_;
		EventRegistrationToken pointer_moved_token_;
		EventRegistrationToken pointer_wheel_changed_token_;
	};

	ref class MetroFrameworkSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
	{
		friend class App3DFramework;

	public:
		virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();

	private:
		void BindAppFramework(App3DFramework* app);

	private:
		App3DFramework* app_;
	};

	ref class MetroMsgs sealed
	{
		friend MetroFramework;

	public:
		MetroMsgs();

	private:
		void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
		void OnLogicalDpiChanged(Platform::Object^ sender);
		void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerWheelChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);

		void BindWindow(WindowPtr const & win);

	private:
		WindowPtr win_;

		std::array<uint32_t, 16> pointer_id_map_;
	};

	MetroFramework::MetroFramework()
		: msgs_(ref new MetroMsgs)
	{
	}

	void MetroFramework::Initialize(CoreApplicationView^ application_view)
	{
		app_view_ = application_view;

		app_activated_token_ = application_view->Activated +=
			ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &MetroFramework::OnActivated);

		app_suspending_token_ = CoreApplication::Suspending +=
			ref new EventHandler<SuspendingEventArgs^>(this, &MetroFramework::OnSuspending);

		app_resuming_token_ = CoreApplication::Resuming +=
			ref new EventHandler<Platform::Object^>(this, &MetroFramework::OnResuming);
	}

	void MetroFramework::SetWindow(CoreWindow^ window)
	{
		window_ = window;

		msgs_->BindWindow(app_->MainWnd());

		win_size_changed_token_ = window->SizeChanged +=
			ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(msgs_, &MetroMsgs::OnWindowSizeChanged);

		visibility_changed_token_ = window->VisibilityChanged +=
			ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(msgs_, &MetroMsgs::OnVisibilityChanged);

		win_closed_token_ = window->Closed += 
			ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(msgs_, &MetroMsgs::OnWindowClosed);

#ifndef KLAYGE_PLATFORM_WINDOWS_PHONE
		window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
#endif

		pointer_pressed_token_ = window->PointerPressed +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(msgs_, &MetroMsgs::OnPointerPressed);
		pointer_released_token_ = window->PointerReleased +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(msgs_, &MetroMsgs::OnPointerReleased);
		pointer_moved_token_ = window->PointerMoved +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(msgs_, &MetroMsgs::OnPointerMoved);
		pointer_wheel_changed_token_ = window->PointerWheelChanged +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(msgs_, &MetroMsgs::OnPointerWheelChanged);

		app_->MainWnd()->SetWindow(Platform::Agile<Windows::UI::Core::CoreWindow>(window));
		app_->MetroCreate();
	}

	void MetroFramework::Load(Platform::String^ entryPoint)
	{
	}

	void MetroFramework::Run()
	{
		app_->MetroRun();
	}

	void MetroFramework::Uninitialize()
	{
		window_->PointerWheelChanged -= pointer_wheel_changed_token_;
		window_->PointerMoved -= pointer_moved_token_;
		window_->PointerReleased -= pointer_released_token_;
		window_->PointerPressed -= pointer_pressed_token_;
		window_->Closed -= win_closed_token_;
		window_->VisibilityChanged -= visibility_changed_token_;
		window_->SizeChanged -= win_size_changed_token_;

		CoreApplication::Resuming -= app_resuming_token_;
		CoreApplication::Suspending -= app_suspending_token_;
		app_view_->Activated -= app_activated_token_;
	}

	void MetroFramework::OnActivated(CoreApplicationView^ application_view, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void MetroFramework::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
	{
		SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

		create_task([this, deferral]()
		{
			app_->Suspend();

			deferral->Complete();
		});
	}

	void MetroFramework::OnResuming(Platform::Object^ sender, Platform::Object^ args)
	{
		app_->Resume();
	}

	void MetroFramework::BindAppFramework(App3DFramework* app)
	{
		app_ = app;
	}
	
	IFrameworkView^ MetroFrameworkSource::CreateView()
	{
		MetroFramework^ ret = ref new MetroFramework;
		ret->BindAppFramework(app_);
		return ret;
	}

	void MetroFrameworkSource::BindAppFramework(App3DFramework* app)
	{
		app_ = app;
	}

	MetroMsgs::MetroMsgs()
	{
		pointer_id_map_.fill(0);
	}

	void MetroMsgs::OnWindowSizeChanged(CoreWindow^ /*sender*/, WindowSizeChangedEventArgs^ /*args*/)
	{
		win_->Active(true);
		win_->Ready(true);
		win_->OnSize()(*win_, true);
	}

	void MetroMsgs::OnVisibilityChanged(CoreWindow^ /*sender*/, VisibilityChangedEventArgs^ args)
	{
		win_->Active(args->Visible);
		win_->OnActive()(*win_, args->Visible);
	}

	void MetroMsgs::OnWindowClosed(CoreWindow^ /*sender*/, CoreWindowEventArgs^ /*args*/)
	{
		win_->OnClose()(*win_);
		win_->Active(false);
		win_->Ready(false);
		win_->Closed(true);
	}

	void MetroMsgs::OnPointerPressed(CoreWindow^ /*sender*/, PointerEventArgs^ args)
	{
		uint32_t conv_id = 0;
		for (size_t i = 0; i < pointer_id_map_.size(); ++ i)
		{
			if (0 == pointer_id_map_[i])
			{
				conv_id = static_cast<uint32_t>(i + 1);
				pointer_id_map_[i] = args->CurrentPoint->PointerId;
				break;
			}
		}
		
		win_->OnPointerDown()(*win_,
			int2(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)),
			conv_id);
	}

	void MetroMsgs::OnPointerReleased(CoreWindow^ /*sender*/, PointerEventArgs^ args)
	{
		uint32_t conv_id = 0;
		for (size_t i = 0; i < pointer_id_map_.size(); ++ i)
		{
			if (args->CurrentPoint->PointerId == pointer_id_map_[i])
			{
				conv_id = static_cast<uint32_t>(i + 1);
				pointer_id_map_[i] = 0;
				break;
			}
		}

		win_->OnPointerUp()(*win_,
			int2(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)),
			conv_id);
	}

	void MetroMsgs::OnPointerMoved(CoreWindow^ /*sender*/, PointerEventArgs^ args)
	{
		uint32_t conv_id = 0;
		for (size_t i = 0; i < pointer_id_map_.size(); ++ i)
		{
			if (args->CurrentPoint->PointerId == pointer_id_map_[i])
			{
				conv_id = static_cast<uint32_t>(i + 1);
				break;
			}
		}

		win_->OnPointerUpdate()(*win_,
			int2(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)),
			conv_id, args->CurrentPoint->IsInContact);
	}

	void MetroMsgs::OnPointerWheelChanged(CoreWindow^ /*sender*/, PointerEventArgs^ args)
	{
		uint32_t conv_id = 0;
		for (size_t i = 0; i < pointer_id_map_.size(); ++ i)
		{
			if (args->CurrentPoint->PointerId == pointer_id_map_[i])
			{
				conv_id = static_cast<uint32_t>(i + 1);
				break;
			}
		}

		win_->OnPointerWheel()(*win_,
			int2(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)),
			conv_id, args->CurrentPoint->Properties->MouseWheelDelta);
	}

	void MetroMsgs::BindWindow(WindowPtr const & win)
	{
		win_ = win;
	}
#endif

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	App3DFramework::App3DFramework(std::string const & name)
						: name_(name), total_num_frames_(0),
							fps_(0), accumulate_time_(0), num_frames_(0),
							app_time_(0), frame_time_(0)
	{
		Context::Instance().AppInstance(*this);

		ContextCfg cfg = Context::Instance().Config();
		main_wnd_ = this->MakeWindow(name_, cfg.graphics_cfg);
#ifndef KLAYGE_PLATFORM_WINDOWS_RUNTIME
		cfg.graphics_cfg.left = main_wnd_->Left();
		cfg.graphics_cfg.top = main_wnd_->Top();
		cfg.graphics_cfg.width = main_wnd_->Width();
		cfg.graphics_cfg.height = main_wnd_->Height();
		Context::Instance().Config(cfg);
#endif
	}

	App3DFramework::App3DFramework(std::string const & name, void* native_wnd)
						: name_(name), total_num_frames_(0),
							fps_(0), accumulate_time_(0), num_frames_(0),
							app_time_(0), frame_time_(0)
	{
		Context::Instance().AppInstance(*this);

		ContextCfg cfg = Context::Instance().Config();
		main_wnd_ = this->MakeWindow(name_, cfg.graphics_cfg, native_wnd);
#ifndef KLAYGE_PLATFORM_WINDOWS_RUNTIME
		cfg.graphics_cfg.left = main_wnd_->Left();
		cfg.graphics_cfg.top = main_wnd_->Top();
		cfg.graphics_cfg.width = main_wnd_->Width();
		cfg.graphics_cfg.height = main_wnd_->Height();
		Context::Instance().Config(cfg);
#endif
	}

	App3DFramework::~App3DFramework()
	{
		this->Destroy();
	}

	// 建立应用程序主窗口
	/////////////////////////////////////////////////////////////////////////////////
#ifdef KLAYGE_PLATFORM_WINDOWS_RUNTIME
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

		if (cfg.deferred_rendering)
		{
			Context::Instance().DeferredRenderingLayerInstance(MakeSharedPtr<DeferredRenderingLayer>());
		}

		this->OnCreate();
		this->OnResize(cfg.graphics_cfg.width, cfg.graphics_cfg.height);
	}

	void App3DFramework::Destroy()
	{
		this->OnDestroy();
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().DestroyRenderWindow();

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
		return MakeSharedPtr<Window>(name, settings);
	}

	WindowPtr App3DFramework::MakeWindow(std::string const & name, RenderSettings const & settings, void* native_wnd)
	{
		return MakeSharedPtr<Window>(name, settings, native_wnd);
	}

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
	void App3DFramework::Run()
	{
		MetroFrameworkSource^ metro_app = ref new MetroFrameworkSource;
		metro_app->BindAppFramework(this);
		CoreApplication::Run(metro_app);
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
#elif defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
		while (!main_wnd_->Closed())
		{
			if (main_wnd_->Active())
			{
				CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
				re.Refresh();
			}
			else
			{
				CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
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
		CameraPtr const & camera = re.CurFrameBuffer()->GetViewport()->camera;
		BOOST_ASSERT(camera);

		return *camera;
	}

	Camera& App3DFramework::ActiveCamera()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		CameraPtr const & camera = re.CurFrameBuffer()->GetViewport()->camera;
		BOOST_ASSERT(camera);

		return *camera;
	}

	// 设置观察矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::LookAt(float3 const & vEye, float3 const & vLookAt)
	{
		this->ActiveCamera().ViewParams(vEye, vLookAt, float3(0, 1, 0));
	}

	void App3DFramework::LookAt(float3 const & vEye, float3 const & vLookAt,
												float3 const & vUp)
	{
		this->ActiveCamera().ViewParams(vEye, vLookAt, vUp);
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
