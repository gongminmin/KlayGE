// D3D11RenderWindow.cpp
// KlayGE D3D11渲染窗口类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11RenderFactory.hpp>
#include <KlayGE/D3D11/D3D11RenderFactoryInternal.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11RenderWindow.hpp>

#include "AMDQuadBuffer.hpp"

namespace KlayGE
{
	D3D11RenderWindow::D3D11RenderWindow(IDXGIFactory1Ptr const & gi_factory, D3D11AdapterPtr const & adapter,
			std::string const & name, RenderSettings const & settings)
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
						: hWnd_(nullptr),
#else
						: metro_d3d_render_win_(ref new MetroD3D11RenderWindow),
#endif
							adapter_(adapter),
							gi_factory_(gi_factory)
	{
		// Store info
		name_				= name;
		isFullScreen_		= settings.full_screen;
		sync_interval_		= settings.sync_interval;
#ifdef KLAYGE_PLATFORM_WINDOWS_RUNTIME
		sync_interval_ = std::max(1U, sync_interval_);
#endif

		ElementFormat format = settings.color_fmt;

		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		hWnd_ = main_wnd->HWnd();
#else
		wnd_ = main_wnd->GetWindow();
		metro_d3d_render_win_->BindD3D11RenderWindow(this);
#endif
		on_paint_connect_ = main_wnd->OnPaint().connect(bind(&D3D11RenderWindow::OnPaint, this,
			placeholders::_1));
		on_exit_size_move_connect_ = main_wnd->OnExitSizeMove().connect(bind(&D3D11RenderWindow::OnExitSizeMove, this,
			placeholders::_1));
		on_size_connect_ = main_wnd->OnSize().connect(bind(&D3D11RenderWindow::OnSize, this,
			placeholders::_1, placeholders::_2));
		on_set_cursor_connect_ = main_wnd->OnSetCursor().connect(bind(&D3D11RenderWindow::OnSetCursor, this,
			placeholders::_1));

		if (this->FullScreen())
		{
			left_ = 0;
			top_ = 0;
			width_ = settings.width;
			height_ = settings.height;
		}
		else
		{
			top_ = settings.top;
			left_ = settings.left;
			width_ = main_wnd->Width();
			height_ = main_wnd->Height();
		}

		back_buffer_format_ = D3D11Mapping::MappingFormat(format);

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		has_dxgi_1_2_ = false;
		dxgi_stereo_support_ = false;
		{
			IDXGIFactory2* factory;
			gi_factory->QueryInterface(IID_IDXGIFactory2, reinterpret_cast<void**>(&factory));
			if (factory != nullptr)
			{
				gi_factory_2_ = MakeCOMPtr(factory);
				has_dxgi_1_2_ = true;
				dxgi_stereo_support_ = gi_factory_2_->IsWindowedStereoEnabled() ? true : false;
			}
		}
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		has_dxgi_1_3_ = false;
		{
			IDXGIFactory3* factory;
			gi_factory->QueryInterface(IID_IDXGIFactory3, reinterpret_cast<void**>(&factory));
			if (factory != nullptr)
			{
				gi_factory_3_ = MakeCOMPtr(factory);
				has_dxgi_1_3_ = true;
			}
		}
#endif

		viewport_->left		= 0;
		viewport_->top		= 0;
		viewport_->width	= width_;
		viewport_->height	= height_;

		ID3D11DevicePtr d3d_device;
		ID3D11DeviceContextPtr d3d_imm_ctx;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
		if (d3d11_re.D3DDevice())
		{
			d3d_device = d3d11_re.D3DDevice();
			d3d_imm_ctx = d3d11_re.D3DDeviceImmContext();

			main_wnd_ = false;
		}
		else
		{
			UINT create_device_flags = 0;
#ifdef KLAYGE_DEBUG
			create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			std::vector<std::pair<D3D_DRIVER_TYPE, wchar_t const *> > dev_type_behaviors;
			dev_type_behaviors.push_back(std::make_pair(D3D_DRIVER_TYPE_HARDWARE, L"HW"));
			dev_type_behaviors.push_back(std::make_pair(D3D_DRIVER_TYPE_WARP, L"WARP"));
			dev_type_behaviors.push_back(std::make_pair(D3D_DRIVER_TYPE_REFERENCE, L"REF"));

			std::vector<std::pair<char const *, D3D_FEATURE_LEVEL> > available_feature_levels;	
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
			if (has_dxgi_1_2_)
			{
				available_feature_levels.push_back(std::make_pair("11_1", D3D_FEATURE_LEVEL_11_1));
			}
#endif
			available_feature_levels.push_back(std::make_pair("11_0", D3D_FEATURE_LEVEL_11_0));
			available_feature_levels.push_back(std::make_pair("10_1", D3D_FEATURE_LEVEL_10_1));
			available_feature_levels.push_back(std::make_pair("10_0", D3D_FEATURE_LEVEL_10_0));
			available_feature_levels.push_back(std::make_pair("9_3", D3D_FEATURE_LEVEL_9_3));
			available_feature_levels.push_back(std::make_pair("9_2", D3D_FEATURE_LEVEL_9_2));
			available_feature_levels.push_back(std::make_pair("9_1", D3D_FEATURE_LEVEL_9_1));

			std::vector<std::string> strs;
			boost::algorithm::split(strs, settings.options, boost::is_any_of(","));
			for (size_t index = 0; index < strs.size(); ++ index)
			{
				std::string& opt = strs[index];
				boost::algorithm::trim(opt);
				std::string::size_type loc = opt.find(':');
				std::string opt_name = opt.substr(0, loc);
				std::string opt_val = opt.substr(loc + 1);

				if (0 == strcmp("level", opt_name.c_str()))
				{
					size_t feature_index = 0;
					for (size_t i = 0; i < available_feature_levels.size(); ++ i)
					{
						if (0 == strcmp(available_feature_levels[i].first, opt_val.c_str()))
						{
							feature_index = i;
							break;
						}
					}

					if (feature_index > 0)
					{
						available_feature_levels.erase(available_feature_levels.begin(),
							available_feature_levels.begin() + feature_index);
					}
				}
			}

			std::vector<D3D_FEATURE_LEVEL> feature_levels;
			for (size_t i = 0; i < available_feature_levels.size(); ++ i)
			{
				feature_levels.push_back(available_feature_levels[i].second);
			}

			typedef KLAYGE_DECLTYPE(dev_type_behaviors) DevTypeBehaviorsType;
			KLAYGE_FOREACH(DevTypeBehaviorsType::reference dev_type_beh, dev_type_behaviors)
			{
				ID3D11Device* device = nullptr;
				ID3D11DeviceContext* imm_ctx = nullptr;
				IDXGIAdapter* dx_adapter = nullptr;
				D3D_DRIVER_TYPE dev_type = dev_type_beh.first;
				if (D3D_DRIVER_TYPE_HARDWARE == dev_type)
				{
					dx_adapter = adapter_->DXGIAdapter().get();
					dev_type = D3D_DRIVER_TYPE_UNKNOWN;
				}
				D3D_FEATURE_LEVEL out_feature_level;
				if (SUCCEEDED(d3d11_re.D3D11CreateDevice(dx_adapter, dev_type, nullptr, create_device_flags,
					&feature_levels[0], static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION, &device,
					&out_feature_level, &imm_ctx)))
				{
					d3d_device = MakeCOMPtr(device);
					d3d_imm_ctx = MakeCOMPtr(imm_ctx);
					d3d11_re.D3DDevice(d3d_device, d3d_imm_ctx, out_feature_level);

					if (Context::Instance().AppInstance().ConfirmDevice())
					{
						if (dev_type != D3D_DRIVER_TYPE_HARDWARE)
						{
							IDXGIDevice1* dxgi_device = nullptr;
							HRESULT hr = d3d_device->QueryInterface(IID_IDXGIDevice1, reinterpret_cast<void**>(&dxgi_device));
							if (SUCCEEDED(hr) && (dxgi_device != nullptr))
							{
								IDXGIAdapter* ada;
								dxgi_device->GetAdapter(&ada);
								IDXGIAdapter1* ada1;
								ada->QueryInterface(IID_IDXGIAdapter1, reinterpret_cast<void**>(&ada1));
								ada->Release();
								adapter_->ResetAdapter(MakeCOMPtr(ada1));
								adapter_->Enumerate();
							}
#ifdef KLAYGE_PLATFORM_WINDOWS_RUNTIME
							dxgi_device->SetMaximumFrameLatency(1);
#endif
							dxgi_device->Release();
						}

						description_ = adapter_->Description() + L" " + dev_type_beh.second;
						wchar_t const * fl_str;
						switch (out_feature_level)
						{
						case D3D_FEATURE_LEVEL_11_1:
							fl_str = L"11.1";
							break;

						case D3D_FEATURE_LEVEL_11_0:
							fl_str = L"11.0";
							break;

						case D3D_FEATURE_LEVEL_10_1:
							fl_str = L"10.1";
							break;

						case D3D_FEATURE_LEVEL_10_0:
							fl_str = L"10.0";
							break;

						case D3D_FEATURE_LEVEL_9_3:
							fl_str = L"9.3";
							break;

						case D3D_FEATURE_LEVEL_9_2:
							fl_str = L"9.2";
							break;

						case D3D_FEATURE_LEVEL_9_1:
							fl_str = L"9.1";
							break;

						default:
							fl_str = L"Unknown";
							break;
						}
						description_ += L" D3D Level ";
						description_ += fl_str;
						if (settings.sample_count > 1)
						{
							description_ += L" ("
								+ boost::lexical_cast<std::wstring>(settings.sample_count)
								+ L"x AA)";
						}
						break;
					}
					else
					{
						d3d_device.reset();
						d3d_imm_ctx.reset();
					}
				}
			}

			main_wnd_ = true;
		}

		Verify(!!d3d_device);
		Verify(!!d3d_imm_ctx);

		bool isDepthBuffered = IsDepthFormat(settings.depth_stencil_fmt);
		uint32_t depthBits = NumDepthBits(settings.depth_stencil_fmt);
		uint32_t stencilBits = NumStencilBits(settings.depth_stencil_fmt);
		if (isDepthBuffered)
		{
			BOOST_ASSERT((32 == depthBits) || (24 == depthBits) || (16 == depthBits) || (0 == depthBits));
			BOOST_ASSERT((8 == stencilBits) || (0 == stencilBits));

			UINT format_support;

			if (32 == depthBits)
			{
				BOOST_ASSERT(0 == stencilBits);

				// Try 32-bit zbuffer
				d3d_device->CheckFormatSupport(DXGI_FORMAT_D32_FLOAT, &format_support);
				if (format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
				{
					depth_stencil_format_ = DXGI_FORMAT_D32_FLOAT;
					stencilBits = 0;
				}
				else
				{
					depthBits = 24;
				}
			}
			if (24 == depthBits)
			{
				d3d_device->CheckFormatSupport(DXGI_FORMAT_D24_UNORM_S8_UINT, &format_support);
				if (format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
				{
					depth_stencil_format_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
					stencilBits = 8;
				}
				else
				{
					depthBits = 16;
				}
			}
			if (16 == depthBits)
			{
				d3d_device->CheckFormatSupport(DXGI_FORMAT_D16_UNORM, &format_support);
				if (format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
				{
					depth_stencil_format_ = DXGI_FORMAT_D16_UNORM;
					stencilBits = 0;
				}
				else
				{
					depthBits = 0;
				}
			}
		}

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (has_dxgi_1_2_)
		{
			bool stereo = (STM_LCDShutter == settings.stereo_method) && dxgi_stereo_support_;

			gi_factory_2_->RegisterStereoStatusWindow(hWnd_, WM_SIZE, &stereo_cookie_);

			std::memset(&sc_desc1_, 0, sizeof(sc_desc1_));
			sc_desc1_.BufferCount = 2;
			sc_desc1_.Width = this->Width();
			sc_desc1_.Height = this->Height();
			sc_desc1_.Format = back_buffer_format_;
			sc_desc1_.Stereo = stereo;
			sc_desc1_.SampleDesc.Count = stereo ? 1 : std::min(static_cast<uint32_t>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
			sc_desc1_.SampleDesc.Quality = stereo ? 0 : settings.sample_quality;
			sc_desc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sc_desc1_.Scaling = stereo ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
			sc_desc1_.SwapEffect = stereo ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_DISCARD;
			sc_desc1_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			sc_fs_desc_.RefreshRate.Numerator = 60;
			sc_fs_desc_.RefreshRate.Denominator = 1;
			sc_fs_desc_.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sc_fs_desc_.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sc_fs_desc_.Windowed = !this->FullScreen();
		}
		else
#endif
		{
			std::memset(&sc_desc_, 0, sizeof(sc_desc_));
			sc_desc_.BufferCount = 2;
			sc_desc_.BufferDesc.Width = this->Width();
			sc_desc_.BufferDesc.Height = this->Height();
			sc_desc_.BufferDesc.Format = back_buffer_format_;
			sc_desc_.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sc_desc_.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sc_desc_.BufferDesc.RefreshRate.Numerator = 60;
			sc_desc_.BufferDesc.RefreshRate.Denominator = 1;
			sc_desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sc_desc_.OutputWindow = hWnd_;
			sc_desc_.SampleDesc.Count = std::min(static_cast<uint32_t>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
			sc_desc_.SampleDesc.Quality = settings.sample_quality;
			sc_desc_.Windowed = !this->FullScreen();
			sc_desc_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			sc_desc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		}
#else
		bool stereo = (STM_LCDShutter == settings.stereo_method) && dxgi_stereo_support_;

		using namespace Windows::Graphics::Display;
		using namespace Windows::Foundation;
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		DisplayInformation::GetForCurrentView()->StereoEnabledChanged +=
			ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(metro_d3d_render_win_, &MetroD3D11RenderWindow::OnStereoEnabledChanged);
#else
		DisplayProperties::StereoEnabledChanged +=
			ref new DisplayPropertiesEventHandler(metro_d3d_render_win_, &MetroD3D11RenderWindow::OnStereoEnabledChanged);
#endif

		std::memset(&sc_desc1_, 0, sizeof(sc_desc1_));
		sc_desc1_.BufferCount = 2;
		sc_desc1_.Width = this->Width();
		sc_desc1_.Height = this->Height();
		sc_desc1_.Format = back_buffer_format_;
		sc_desc1_.Stereo = stereo;
		sc_desc1_.SampleDesc.Count = stereo ? 1 : std::min(static_cast<uint32_t>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
		sc_desc1_.SampleDesc.Quality = stereo ? 0 : settings.sample_quality;
		sc_desc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sc_desc1_.Scaling = DXGI_SCALING_NONE;
		sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		sc_desc1_.Flags = 0;
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if ((STM_LCDShutter == settings.stereo_method)
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
			&& !dxgi_stereo_support_
#endif
			)
		{
			if (adapter_->Description().find(L"AMD", 0) != std::wstring::npos)
			{
#ifdef KLAYGE_PLATFORM_WIN64
				HMODULE dll = ::GetModuleHandle(TEXT("atidxx64.dll"));
#else
				HMODULE dll = ::GetModuleHandle(TEXT("atidxx32.dll"));
#endif
				if (dll != nullptr)
				{
					PFNAmdDxExtCreate11 AmdDxExtCreate11 = reinterpret_cast<PFNAmdDxExtCreate11>(::GetProcAddress(dll, "AmdDxExtCreate11"));
					if (AmdDxExtCreate11 != nullptr)
					{
						IAmdDxExt* amd_dx_ext;
						HRESULT hr = AmdDxExtCreate11(d3d_device.get(), &amd_dx_ext);
						if (SUCCEEDED(hr))
						{
							AmdDxExtVersion ext_version;
							hr = amd_dx_ext->GetVersion(&ext_version);
							if (SUCCEEDED(hr))
							{
								IAmdDxExtInterface* adti = amd_dx_ext->GetExtInterface(AmdDxExtQuadBufferStereoID);
								IAmdDxExtQuadBufferStereo* stereo = static_cast<IAmdDxExtQuadBufferStereo*>(adti);
								if (stereo != nullptr)
								{
									stereo_amd_qb_ext_ = MakeCOMPtr(stereo);
									stereo_amd_qb_ext_->EnableQuadBufferStereo(true);
								}
							}
						}
						amd_dx_ext->Release();
					}
				}
			}
		}

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (has_dxgi_1_2_)
		{
			IDXGISwapChain1* sc = nullptr;
			gi_factory_2_->CreateSwapChainForHwnd(d3d_device.get(), hWnd_,
				&sc_desc1_, &sc_fs_desc_, nullptr, &sc);
			swap_chain_ = MakeCOMPtr(sc);
		}
		else
#endif
		{
			IDXGISwapChain* sc = nullptr;
			gi_factory->CreateSwapChain(d3d_device.get(), &sc_desc_, &sc);
			swap_chain_ = MakeCOMPtr(sc);
		}

		gi_factory->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
		swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#else
		IDXGISwapChain1* sc = nullptr;
		gi_factory_2_->CreateSwapChainForCoreWindow(d3d_device.get(),
			reinterpret_cast<IUnknown*>(wnd_.Get()), &sc_desc1_, nullptr, &sc);
		swap_chain_ = MakeCOMPtr(sc);
#endif

		Verify(!!swap_chain_);

		this->UpdateSurfacesPtrs();
	}

	D3D11RenderWindow::~D3D11RenderWindow()
	{
		on_paint_connect_.disconnect();
		on_exit_size_move_connect_.disconnect();
		on_size_connect_.disconnect();
		on_set_cursor_connect_.disconnect();

		this->Destroy();
	}

	std::wstring const & D3D11RenderWindow::Description() const
	{
		return description_;
	}

	// 改变窗口大小
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderWindow::Resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_->width = width;
		viewport_->height = height;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
		ID3D11DeviceContextPtr d3d_imm_ctx = d3d11_re.D3DDeviceImmContext();
		if (d3d_imm_ctx)
		{
			d3d_imm_ctx->ClearState();
		}

		for (size_t i = 0; i < clr_views_.size(); ++ i)
		{
			clr_views_[i].reset();
		}
		rs_view_.reset();

		render_target_view_.reset();
		depth_stencil_view_.reset();
		back_buffer_.reset();
		depth_stencil_.reset();

		UINT flags = 0;
		if (this->FullScreen())
		{
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		}

		this->OnUnbind();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (has_dxgi_1_2_)
		{
			dxgi_stereo_support_ = gi_factory_2_->IsWindowedStereoEnabled() ? true : false;

			sc_desc1_.Width = width_;
			sc_desc1_.Height = height_;
			sc_desc1_.Stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;
		}
		else
#endif
		{
			sc_desc_.BufferDesc.Width = width_;
			sc_desc_.BufferDesc.Height = height_;
		}
#else
		dxgi_stereo_support_ = gi_factory_2_->IsWindowedStereoEnabled() ? true : false;

		sc_desc1_.Width = width_;
		sc_desc1_.Height = height_;
		sc_desc1_.Stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;
#endif

		if (!!swap_chain_)
		{
			swap_chain_->ResizeBuffers(2, width, height, back_buffer_format_, flags);
		}
		else
		{
			ID3D11DevicePtr d3d_device = d3d11_re.D3DDevice();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
			if (has_dxgi_1_2_)
			{
				IDXGISwapChain1* sc = nullptr;
				gi_factory_2_->CreateSwapChainForHwnd(d3d_device.get(), hWnd_,
					&sc_desc1_, &sc_fs_desc_, nullptr, &sc);
				swap_chain_ = MakeCOMPtr(sc);
			}
			else
#endif
			{
				IDXGISwapChain* sc = nullptr;
				gi_factory_->CreateSwapChain(d3d_device.get(), &sc_desc_, &sc);
				swap_chain_ = MakeCOMPtr(sc);
			}

			swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#else
			IDXGISwapChain1* sc = nullptr;
			gi_factory_2_->CreateSwapChainForCoreWindow(d3d_device.get(),
				reinterpret_cast<IUnknown*>(wnd_.Get()), &sc_desc1_, nullptr, &sc);
			swap_chain_ = MakeCOMPtr(sc);
#endif

			Verify(!!swap_chain_);
		}

		this->UpdateSurfacesPtrs();

		d3d11_re.ResetRenderStates();

		this->OnBind();

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);
	}

	// 改变窗口位置
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderWindow::Reposition(uint32_t left, uint32_t top)
	{
		left_ = left;
		top_ = top;
	}

	// 获取是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	bool D3D11RenderWindow::FullScreen() const
	{
		return isFullScreen_;
	}

	// 设置是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11RenderWindow::FullScreen(bool fs)
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (isFullScreen_ != fs)
		{
			left_ = 0;
			top_ = 0;

			uint32_t style;
			if (fs)
			{
				style = WS_POPUP;
			}
			else
			{
				style = WS_OVERLAPPEDWINDOW;
			}

			::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);

			RECT rc = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
			::AdjustWindowRect(&rc, style, false);
			width_ = rc.right - rc.left;
			height_ = rc.bottom - rc.top;
			::SetWindowPos(hWnd_, nullptr, left_, top_, width_, height_, SWP_NOZORDER);

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
			if (has_dxgi_1_2_)
			{
				sc_desc1_.Width = width_;
				sc_desc1_.Height = height_;
				sc_fs_desc_.Windowed = !fs;
			}
			else
#endif
			{
				sc_desc_.BufferDesc.Width = width_;
				sc_desc_.BufferDesc.Height = height_;
				sc_desc_.Windowed = !fs;
			}

			isFullScreen_ = fs;

			swap_chain_->SetFullscreenState(isFullScreen_, nullptr);
			if (isFullScreen_)
			{
				DXGI_MODE_DESC desc;
				std::memset(&desc, 0, sizeof(desc));
				desc.Width = width_;
				desc.Height = height_;
				desc.Format = back_buffer_format_;
				desc.RefreshRate.Numerator = 60;
				desc.RefreshRate.Denominator = 1;
				swap_chain_->ResizeTarget(&desc);
			}

			::ShowWindow(hWnd_, SW_SHOWNORMAL);
			::UpdateWindow(hWnd_);
		}
#else
		UNREF_PARAM(fs);
#endif
	}

	void D3D11RenderWindow::WindowMovedOrResized()
	{
		::RECT rect;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		::GetClientRect(hWnd_, &rect);
#else
		rect.left = static_cast<LONG>(wnd_->Bounds.Left);
		rect.right = static_cast<LONG>(wnd_->Bounds.Right);
		rect.top = static_cast<LONG>(wnd_->Bounds.Top);
		rect.bottom = static_cast<LONG>(wnd_->Bounds.Bottom);
#endif

		uint32_t new_left = rect.left;
		uint32_t new_top = rect.top;
		if ((new_left != left_) || (new_top != top_))
		{
			this->Reposition(new_left, new_top);
		}

		bool stereo_changed = false;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (has_dxgi_1_2_)
		{
			stereo_changed = ((gi_factory_2_->IsWindowedStereoEnabled() ? true : false) != dxgi_stereo_support_);
		}
#endif

		uint32_t new_width = rect.right - rect.left;
		uint32_t new_height = rect.bottom - rect.top;
		if ((new_width != width_) || (new_height != height_) || stereo_changed)
		{
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(new_width, new_height);
		}
	}

	void D3D11RenderWindow::Destroy()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (swap_chain_)
		{
			swap_chain_->SetFullscreenState(false, nullptr);
		}

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (has_dxgi_1_2_ && !!gi_factory_2_)
		{
			gi_factory_2_->UnregisterStereoStatus(stereo_cookie_);
		}
#endif
#endif

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		render_target_view_right_eye_.reset();
		depth_stencil_view_right_eye_.reset();
#endif
		render_target_view_.reset();
		depth_stencil_view_.reset();
		back_buffer_.reset();
		depth_stencil_.reset();
		swap_chain_.reset();
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		gi_factory_2_.reset();
#endif
		gi_factory_.reset();
	}

	void D3D11RenderWindow::UpdateSurfacesPtrs()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
		ID3D11DevicePtr d3d_device = d3d11_re.D3DDevice();

		// Create a render target view
		ID3D11Texture2D* back_buffer;
		TIF(swap_chain_->GetBuffer(0, IID_ID3D11Texture2D, reinterpret_cast<void**>(&back_buffer)));
		back_buffer_ = MakeCOMPtr(back_buffer);
		D3D11_TEXTURE2D_DESC bb_desc;
		back_buffer_->GetDesc(&bb_desc);

		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		rtv_desc.Format = bb_desc.Format;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (has_dxgi_1_2_)
		{
			if (bb_desc.SampleDesc.Count > 1)
			{
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
				rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
				rtv_desc.Texture2DMSArray.ArraySize = 1;
			}
			else
			{
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtv_desc.Texture2DArray.MipSlice = 0;
				rtv_desc.Texture2DArray.FirstArraySlice = 0;
				rtv_desc.Texture2DArray.ArraySize = 1;
			}
		}
		else
#endif
		{
			if (bb_desc.SampleDesc.Count > 1)
			{
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				rtv_desc.Texture2D.MipSlice = 0;
			}
		}
		ID3D11RenderTargetView* render_target_view;
		TIF(d3d_device->CreateRenderTargetView(back_buffer_.get(), &rtv_desc, &render_target_view));
		render_target_view_ = MakeCOMPtr(render_target_view);

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		bool stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;

		if (stereo)
		{
			if (bb_desc.SampleDesc.Count > 1)
			{
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
				rtv_desc.Texture2DMSArray.FirstArraySlice = 1;
				rtv_desc.Texture2DMSArray.ArraySize = 1;
			}
			else
			{
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtv_desc.Texture2DArray.MipSlice = 0;
				rtv_desc.Texture2DArray.FirstArraySlice = 1;
				rtv_desc.Texture2DArray.ArraySize = 1;
			}
			TIF(d3d_device->CreateRenderTargetView(back_buffer_.get(), &rtv_desc, &render_target_view));
			render_target_view_right_eye_ = MakeCOMPtr(render_target_view);
		}
#else
		bool stereo = false;
#endif

		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC ds_desc;
		ds_desc.Width = this->Width();
		ds_desc.Height = this->Height();
		ds_desc.MipLevels = 1;
		ds_desc.ArraySize = stereo ? 2 : 1;
		ds_desc.Format = depth_stencil_format_;
		ds_desc.SampleDesc.Count = bb_desc.SampleDesc.Count;
		ds_desc.SampleDesc.Quality = bb_desc.SampleDesc.Quality;
		ds_desc.Usage = D3D11_USAGE_DEFAULT;
		ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		ds_desc.CPUAccessFlags = 0;
		ds_desc.MiscFlags = 0;
		ID3D11Texture2D* depth_stencil;
		TIF(d3d_device->CreateTexture2D(&ds_desc, nullptr, &depth_stencil));
		depth_stencil_ = MakeCOMPtr(depth_stencil);

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		dsv_desc.Format = depth_stencil_format_;
		dsv_desc.Flags = 0;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (has_dxgi_1_2_)
		{
			if (bb_desc.SampleDesc.Count > 1)
			{
				dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
				dsv_desc.Texture2DMSArray.FirstArraySlice = 0;
				dsv_desc.Texture2DMSArray.ArraySize = 1;
			}
			else
			{
				dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsv_desc.Texture2DArray.MipSlice = 0;
				dsv_desc.Texture2DArray.FirstArraySlice = 0;
				dsv_desc.Texture2DArray.ArraySize = 1;
			}
		}
		else
#endif
		{
			if (bb_desc.SampleDesc.Count > 1)
			{
				dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsv_desc.Texture2D.MipSlice = 0;
			}
		}
		ID3D11DepthStencilView* depth_stencil_view;
		TIF(d3d_device->CreateDepthStencilView(depth_stencil_.get(), &dsv_desc, &depth_stencil_view));
		depth_stencil_view_ = MakeCOMPtr(depth_stencil_view);

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (stereo)
		{
			if (bb_desc.SampleDesc.Count > 1)
			{
				dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
				dsv_desc.Texture2DMSArray.FirstArraySlice = 1;
				dsv_desc.Texture2DMSArray.ArraySize = 1;
			}
			else
			{
				dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsv_desc.Texture2DArray.MipSlice = 0;
				dsv_desc.Texture2DArray.FirstArraySlice = 1;
				dsv_desc.Texture2DArray.ArraySize = 1;
			}
			TIF(d3d_device->CreateDepthStencilView(depth_stencil_.get(), &dsv_desc, &depth_stencil_view));
			depth_stencil_view_right_eye_ = MakeCOMPtr(depth_stencil_view);
		}
#endif

		if (!!stereo_amd_qb_ext_)
		{
			stereo_amd_right_eye_height_ = stereo_amd_qb_ext_->GetLineOffset(swap_chain_.get());
		}

		this->Attach(ATT_Color0, MakeSharedPtr<D3D11RenderTargetRenderView>(render_target_view_,
			this->Width(), this->Height(), D3D11Mapping::MappingFormat(back_buffer_format_)));
		if (depth_stencil_view_)
		{
			this->Attach(ATT_DepthStencil, MakeSharedPtr<D3D11DepthStencilRenderView>(depth_stencil_view_,
				this->Width(), this->Height(), D3D11Mapping::MappingFormat(depth_stencil_format_)));
		}
	}

	void D3D11RenderWindow::SwapBuffers()
	{
		if (swap_chain_)
		{
			TIF(swap_chain_->Present(sync_interval_, 0));

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
			if (has_dxgi_1_2_)
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
				ID3D11DeviceContextPtr d3d_imm_ctx = d3d11_re.D3DDeviceImmContext();
				ID3D11DeviceContext1Ptr const & d3d_imm_ctx_1 = static_pointer_cast<ID3D11DeviceContext1>(d3d_imm_ctx);
				d3d_imm_ctx_1->DiscardView(render_target_view_.get());
				d3d_imm_ctx_1->DiscardView(depth_stencil_view_.get());
				if (render_target_view_right_eye_)
				{
					d3d_imm_ctx_1->DiscardView(render_target_view_right_eye_.get());
				}
				if (depth_stencil_view_right_eye_)
				{
					d3d_imm_ctx_1->DiscardView(depth_stencil_view_right_eye_.get());
				}
			}
#endif
		}
	}

	void D3D11RenderWindow::OnPaint(Window const & win)
	{
		// If we get WM_PAINT messges, it usually means our window was
		// comvered up, so we need to refresh it by re-showing the contents
		// of the current frame.
		if (win.Active() && win.Ready())
		{
			Context::Instance().SceneManagerInstance().Update();
			this->SwapBuffers();
		}
	}

	void D3D11RenderWindow::OnExitSizeMove(Window const & /*win*/)
	{
		this->WindowMovedOrResized();
	}

	void D3D11RenderWindow::OnSize(Window const & win, bool active)
	{
		if (active)
		{
			if (win.Ready())
			{
				this->WindowMovedOrResized();
			}
		}
	}

	void D3D11RenderWindow::OnSetCursor(Window const & /*win*/)
	{
	}

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
	void D3D11RenderWindow::MetroD3D11RenderWindow::OnStereoEnabledChanged(
		Windows::Graphics::Display::DisplayInformation^ /*sender*/, Platform::Object^ /*args*/)
#else
	void D3D11RenderWindow::MetroD3D11RenderWindow::OnStereoEnabledChanged(Platform::Object^ /*sender*/)
#endif
	{
		if ((win_->gi_factory_2_->IsWindowedStereoEnabled() ? true : false) != win_->dxgi_stereo_support_)
		{
			win_->swap_chain_.reset();
			win_->WindowMovedOrResized();
		}
	}

	void D3D11RenderWindow::MetroD3D11RenderWindow::BindD3D11RenderWindow(D3D11RenderWindow* win)
	{
		win_ = win;
	}
#endif
}
