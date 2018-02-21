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
#include <KFL/COMPtr.hpp>

#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4471) // A forward declaration of an unscoped enumeration must have an underlying type
#endif
#include <Windows.ApplicationModel.h>
#include <Windows.ApplicationModel.core.h>
#include <windows.graphics.display.h>
#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(pop)
#endif

#include <wrl/client.h>
#include <wrl/event.h>
#include <wrl/wrappers/corewrappers.h>

#include <ppl.h>
#include <ppltasks.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::ApplicationModel;
using namespace ABI::Windows::ApplicationModel::Activation;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::Graphics::Display;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::Input;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace concurrency;
#elif defined(KLAYGE_PLATFORM_ANDROID)
#include <android_native_app_glue.h>
#endif

namespace KlayGE
{
#if defined KLAYGE_PLATFORM_WINDOWS_STORE
	class MetroFrameworkSource;

	class MetroFramework : public RuntimeClass<ABI::Windows::ApplicationModel::Core::IFrameworkView>
	{
	public:
		InspectableClass(L"KlayGE.MetroFramework", BaseTrust);

	public:
		HRESULT RuntimeClassInitialize(App3DFramework* app);

		IFACEMETHOD(Initialize)(ABI::Windows::ApplicationModel::Core::ICoreApplicationView* application_view);
		IFACEMETHOD(SetWindow)(ABI::Windows::UI::Core::ICoreWindow* window);
		IFACEMETHOD(Load)(HSTRING entry_point);
		IFACEMETHOD(Run)();
		IFACEMETHOD(Uninitialize)();

	private:
		HRESULT OnActivated(ABI::Windows::ApplicationModel::Core::ICoreApplicationView* application_view,
			ABI::Windows::ApplicationModel::Activation::IActivatedEventArgs* args);
		HRESULT OnSuspending(IInspectable* sender, ABI::Windows::ApplicationModel::ISuspendingEventArgs* args);
		HRESULT OnResuming(IInspectable* sender, IInspectable* args);

		HRESULT OnWindowSizeChanged(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::IWindowSizeChangedEventArgs* args);
		HRESULT OnWindowClosed(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::ICoreWindowEventArgs* args);
		HRESULT OnVisibilityChanged(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::IVisibilityChangedEventArgs* args);
		HRESULT OnKeyDown(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::IKeyEventArgs* args);
		HRESULT OnKeyUp(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::IKeyEventArgs* args);
		HRESULT OnPointerPressed(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::IPointerEventArgs* args);
		HRESULT OnPointerReleased(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::IPointerEventArgs* args);
		HRESULT OnPointerMoved(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::IPointerEventArgs* args);
		HRESULT OnPointerWheelChanged(ABI::Windows::UI::Core::ICoreWindow* sender,
			ABI::Windows::UI::Core::IPointerEventArgs* args);
		HRESULT OnDpiChanged(ABI::Windows::Graphics::Display::IDisplayInformation* sender, IInspectable* args);
		HRESULT OnOrientationChanged(ABI::Windows::Graphics::Display::IDisplayInformation* sender, IInspectable* args);

	private:
		App3DFramework* app_;
		std::shared_ptr<ABI::Windows::ApplicationModel::Core::ICoreApplicationView> app_view_;
		std::shared_ptr<ABI::Windows::UI::Core::ICoreWindow> window_;

		EventRegistrationToken app_activated_token_;
		EventRegistrationToken app_suspending_token_;
		EventRegistrationToken app_resuming_token_;

		EventRegistrationToken win_size_changed_token_;
		EventRegistrationToken visibility_changed_token_;
		EventRegistrationToken win_closed_token_;
		EventRegistrationToken dpi_changed_token_;
		EventRegistrationToken orientation_changed_token_;
		EventRegistrationToken key_down_token_;
		EventRegistrationToken key_up_token_;
		EventRegistrationToken pointer_pressed_token_;
		EventRegistrationToken pointer_released_token_;
		EventRegistrationToken pointer_moved_token_;
		EventRegistrationToken pointer_wheel_changed_token_;
	};

	class MetroFrameworkSource : public RuntimeClass<ABI::Windows::ApplicationModel::Core::IFrameworkViewSource, FtmBase>
	{
		friend class App3DFramework;

	public:
		InspectableClass(L"KlayGE.MetroFrameworkSource", BaseTrust);

	public:
		HRESULT RuntimeClassInitialize(App3DFramework* app);

		IFACEMETHOD(CreateView)(ABI::Windows::ApplicationModel::Core::IFrameworkView** framework_view);

	private:
		App3DFramework* app_;
	};

	HRESULT MetroFramework::RuntimeClassInitialize(App3DFramework* app)
	{
		app_ = app;

		return S_OK;
	}

	IFACEMETHODIMP MetroFramework::Initialize(ICoreApplicationView* application_view)
	{
		app_view_ = MakeCOMPtr(application_view);

		app_view_->add_Activated(Callback<ITypedEventHandler<CoreApplicationView*, IActivatedEventArgs*>>(
			[this](ICoreApplicationView* application_view, IActivatedEventArgs* args)
			{
				return this->OnActivated(application_view, args);
			}).Get(),
			&app_activated_token_);

		ComPtr<ICoreApplication> core_app;
		GetActivationFactory(HStringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(),
			&core_app);

		core_app->add_Suspending(Callback<IEventHandler<SuspendingEventArgs*>>(
			[this](IInspectable* sender, ISuspendingEventArgs* args)
			{
				return this->OnSuspending(sender, args);
			}).Get(),
			&app_suspending_token_);
		core_app->add_Resuming(Callback<IEventHandler<IInspectable*>>(
			[this](IInspectable* sender, IInspectable* args)
			{
				return this->OnResuming(sender, args);
			}).Get(),
			&app_resuming_token_);

		return S_OK;
	}

	IFACEMETHODIMP MetroFramework::SetWindow(ICoreWindow* window)
	{
		window_ = MakeCOMPtr(window);

		window_->add_SizeChanged(Callback<ITypedEventHandler<CoreWindow*, WindowSizeChangedEventArgs*>>(
			[this](ICoreWindow* sender, IWindowSizeChangedEventArgs* args)
			{
				return this->OnWindowSizeChanged(sender, args);
			}).Get(),
			&win_size_changed_token_);

		window_->add_VisibilityChanged(Callback<ITypedEventHandler<CoreWindow*, VisibilityChangedEventArgs*>>(
			[this](ICoreWindow* sender, IVisibilityChangedEventArgs* args)
			{
				return this->OnVisibilityChanged(sender, args);
			}).Get(),
			&visibility_changed_token_);

		window_->add_Closed(Callback<ITypedEventHandler<CoreWindow*, CoreWindowEventArgs*>>(
			[this](ICoreWindow* sender, ICoreWindowEventArgs* args)
			{
				return this->OnWindowClosed(sender, args);
			}).Get(),
			&win_closed_token_);

		ComPtr<ICoreCursorFactory> cursor_factory;
		GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Core_CoreCursor).Get(),
			&cursor_factory);
		ComPtr<ICoreCursor> cursor;
		cursor_factory->CreateCursor(CoreCursorType::CoreCursorType_Arrow, 0, &cursor);
		window_->put_PointerCursor(cursor.Get());

		window_->add_KeyDown(Callback<ITypedEventHandler<CoreWindow*, KeyEventArgs*>>(
			[this](ICoreWindow* sender, IKeyEventArgs* args)
			{
				return this->OnKeyDown(sender, args);
			}).Get(),
			&key_down_token_);
		window_->add_KeyUp(Callback<ITypedEventHandler<CoreWindow*, KeyEventArgs*>>(
			[this](ICoreWindow* sender, IKeyEventArgs* args)
			{
				return this->OnKeyUp(sender, args);
			}).Get(),
			&key_up_token_);

		window_->add_PointerPressed(Callback<ITypedEventHandler<CoreWindow*, PointerEventArgs*>>(
			[this](ICoreWindow* sender, IPointerEventArgs* args)
			{
				return this->OnPointerPressed(sender, args);
			}).Get(),
			&pointer_pressed_token_);
		window_->add_PointerReleased(Callback<ITypedEventHandler<CoreWindow*, PointerEventArgs*>>(
			[this](ICoreWindow* sender, IPointerEventArgs* args)
			{
				return this->OnPointerReleased(sender, args);
			}).Get(),
			&pointer_released_token_);
		window_->add_PointerMoved(Callback<ITypedEventHandler<CoreWindow*, PointerEventArgs*>>(
			[this](ICoreWindow* sender, IPointerEventArgs* args)
			{
				return this->OnPointerMoved(sender, args);
			}).Get(),
			&pointer_moved_token_);
		window_->add_PointerWheelChanged(Callback<ITypedEventHandler<CoreWindow*, PointerEventArgs*>>(
			[this](ICoreWindow* sender, IPointerEventArgs* args)
			{
				return this->OnPointerWheelChanged(sender, args);
			}).Get(),
			&pointer_wheel_changed_token_);

		ComPtr<IDisplayInformationStatics> disp_info_stat;
		GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(),
			&disp_info_stat);

		ComPtr<IDisplayInformation> disp_info;
		disp_info_stat->GetForCurrentView(&disp_info);

		disp_info->add_DpiChanged(Callback<ITypedEventHandler<DisplayInformation*, IInspectable*>>(
			[this](IDisplayInformation* sender, IInspectable* args)
			{
				return this->OnDpiChanged(sender, args);
			}).Get(),
			&dpi_changed_token_);

		disp_info->add_OrientationChanged(Callback<ITypedEventHandler<DisplayInformation*, IInspectable*>>(
			[this](IDisplayInformation* sender, IInspectable* args)
			{
				return this->OnOrientationChanged(sender, args);
			}).Get(),
			&orientation_changed_token_);

		app_->MainWnd()->SetWindow(window_);
		app_->MetroCreate();

		return S_OK;
	}

	IFACEMETHODIMP MetroFramework::Load(HSTRING entry_point)
	{
		KFL_UNUSED(entry_point);

		return S_OK;
	}

	IFACEMETHODIMP MetroFramework::Run()
	{
		app_->MetroRun();

		return S_OK;
	}

	IFACEMETHODIMP MetroFramework::Uninitialize()
	{
		ComPtr<IDisplayInformationStatics> disp_info_stat;
		GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(),
			&disp_info_stat);

		ComPtr<IDisplayInformation> disp_info;
		disp_info_stat->GetForCurrentView(&disp_info);

		disp_info->remove_DpiChanged(dpi_changed_token_);
		disp_info->remove_OrientationChanged(orientation_changed_token_);

		window_->remove_PointerWheelChanged(pointer_wheel_changed_token_);
		window_->remove_PointerMoved(pointer_moved_token_);
		window_->remove_PointerReleased(pointer_released_token_);
		window_->remove_PointerPressed(pointer_pressed_token_);
		window_->remove_KeyUp(key_up_token_);
		window_->remove_KeyDown(key_down_token_);
		window_->remove_Closed(win_closed_token_);
		window_->remove_VisibilityChanged(visibility_changed_token_);
		window_->remove_SizeChanged(win_size_changed_token_);

		ComPtr<ICoreApplication> core_app;
		GetActivationFactory(HStringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(),
			&core_app);

		core_app->remove_Resuming(app_resuming_token_);
		core_app->remove_Suspending(app_suspending_token_);
		app_view_->remove_Activated(app_activated_token_);

		return S_OK;
	}

	HRESULT MetroFramework::OnActivated(ICoreApplicationView* application_view,
			IActivatedEventArgs* args)
	{
		KFL_UNUSED(application_view);
		KFL_UNUSED(args);

		ComPtr<ICoreWindowStatic> core_win_stat;
		TIFHR(GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Core_CoreWindow).Get(),
			&core_win_stat));

		ComPtr<ICoreWindow> core_win;
		core_win_stat->GetForCurrentThread(&core_win);
		core_win->Activate();

		WindowPtr const & win = app_->MainWnd();
		win->OnActivated();

		return S_OK;
	}

	HRESULT MetroFramework::OnSuspending(IInspectable* sender, ISuspendingEventArgs* args)
	{
		KFL_UNUSED(sender);

		ComPtr<ISuspendingOperation> op;
		args->get_SuspendingOperation(&op);

		ComPtr<ISuspendingDeferral> deferral;
		op->GetDeferral(&deferral);

		create_task([this, deferral]()
		{
			app_->Suspend();

			deferral->Complete();
		});

		return S_OK;
	}

	HRESULT MetroFramework::OnResuming(IInspectable* sender, IInspectable* args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		app_->Resume();

		return S_OK;
	}

	HRESULT MetroFramework::OnWindowSizeChanged(ICoreWindow* sender, IWindowSizeChangedEventArgs* args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnSizeChanged(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnVisibilityChanged(ICoreWindow* sender, IVisibilityChangedEventArgs* args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnVisibilityChanged(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnWindowClosed(ICoreWindow* sender, ICoreWindowEventArgs* args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		WindowPtr const & win = app_->MainWnd();
		win->OnClosed();

		return S_OK;
	}

	HRESULT MetroFramework::OnKeyDown(ICoreWindow* sender, IKeyEventArgs* args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnKeyDown(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnKeyUp(ICoreWindow* sender, IKeyEventArgs* args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnKeyUp(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnPointerPressed(ICoreWindow* sender, IPointerEventArgs* args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnPointerPressed(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnPointerReleased(ICoreWindow* sender, IPointerEventArgs* args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnPointerReleased(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnPointerMoved(ICoreWindow* sender, IPointerEventArgs* args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnPointerMoved(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnPointerWheelChanged(ICoreWindow* sender, IPointerEventArgs* args)
	{
		KFL_UNUSED(sender);

		WindowPtr const & win = app_->MainWnd();
		win->OnPointerWheelChanged(args);

		return S_OK;
	}

	HRESULT MetroFramework::OnDpiChanged(IDisplayInformation* sender, IInspectable* args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		WindowPtr const & win = app_->MainWnd();
		win->OnDpiChanged();

		return S_OK;
	}

	HRESULT MetroFramework::OnOrientationChanged(IDisplayInformation* sender, IInspectable* args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		WindowPtr const & win = app_->MainWnd();
		win->OnOrientationChanged();

		return S_OK;
	}

	HRESULT MetroFrameworkSource::RuntimeClassInitialize(App3DFramework* app)
	{
		app_ = app;
		return S_OK;
	}

	IFACEMETHODIMP MetroFrameworkSource::CreateView(IFrameworkView** framework_view)
	{
		return MakeAndInitialize<MetroFramework>(framework_view, app_);
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
#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
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
#ifndef KLAYGE_PLATFORM_WINDOWS_STORE
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
		return MakeSharedPtr<Window>(name, settings);
	}

	WindowPtr App3DFramework::MakeWindow(std::string const & name, RenderSettings const & settings, void* native_wnd)
	{
		return MakeSharedPtr<Window>(name, settings, native_wnd);
	}

	bool App3DFramework::ConfirmDevice() const
	{
		bool confirmed = true;

		ContextCfg const & cfg = Context::Instance().Config();
		if (cfg.deferred_rendering)
		{
			confirmed &= DeferredRenderingLayer::ConfirmDevice();
		}

		return confirmed;
	}

#if defined KLAYGE_PLATFORM_WINDOWS_STORE
	void App3DFramework::Run()
	{
		ComPtr<ICoreApplication> core_app;
		TIFHR(GetActivationFactory(HStringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(),
			&core_app));

		ComPtr<MetroFrameworkSource> metro_app;
		MakeAndInitialize<MetroFrameworkSource>(&metro_app, this);
		TIFHR(core_app->Run(metro_app.Get()));
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
		ComPtr<ICoreWindowStatic> core_win_stat;
		TIFHR(GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Core_CoreWindow).Get(),
			&core_win_stat));

		ComPtr<ICoreWindow> core_win;
		core_win_stat->GetForCurrentThread(&core_win);

		ComPtr<ICoreDispatcher> dispatcher;
		core_win->get_Dispatcher(&dispatcher);

		while (!main_wnd_->Closed())
		{
			if (main_wnd_->Active())
			{
				dispatcher->ProcessEvents(CoreProcessEventsOption::CoreProcessEventsOption_ProcessAllIfPresent);
				re.Refresh();
			}
			else
			{
				dispatcher->ProcessEvents(CoreProcessEventsOption::CoreProcessEventsOption_ProcessOneAndAllPending);
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
