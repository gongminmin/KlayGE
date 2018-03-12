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
#include <KFL/CXX17.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Hash.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
#include <KFL/ArrayRef.hpp>

#include <cstring>
#include <boost/assert.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <KlayGE/D3D11/D3D11Adapter.hpp>
#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11RenderWindow.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>

#include "AMDQuadBuffer.hpp"

#if defined KLAYGE_PLATFORM_WINDOWS_STORE
#include <wrl/client.h>
#include <wrl/event.h>
#include <wrl/wrappers/corewrappers.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Graphics::Display;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
#endif

namespace KlayGE
{
	D3D11RenderWindow::D3D11RenderWindow(D3D11Adapter* adapter, std::string const & name, RenderSettings const & settings)
						: adapter_(adapter), dxgi_stereo_support_(false), dxgi_allow_tearing_(false), dxgi_async_swap_chain_(false),
							frame_latency_waitable_obj_(0)
	{
		// Store info
		name_				= name;
		isFullScreen_		= settings.full_screen;
		sync_interval_		= settings.sync_interval;

		ElementFormat format = settings.color_fmt;

		auto main_wnd = Context::Instance().AppInstance().MainWnd().get();
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		hWnd_ = main_wnd->HWnd();
#else
		wnd_ = main_wnd->GetWindow();
#endif
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

		if (this->FullScreen())
		{
			left_ = 0;
			top_ = 0;
			width_ = settings.width;
			height_ = settings.height;
		}
		else
		{
			left_ = main_wnd->Left();
			top_ = main_wnd->Top();
			width_ = main_wnd->Width();
			height_ = main_wnd->Height();
		}

		back_buffer_format_ = D3D11Mapping::MappingFormat(format);

		viewport_->left		= 0;
		viewport_->top		= 0;
		viewport_->width	= width_;
		viewport_->height	= height_;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
		ID3D11Device* d3d_device = d3d11_re.D3DDevice();
		ID3D11DeviceContext* d3d_imm_ctx = nullptr;

		if (d3d11_re.DXGISubVer() >= 2)
		{
			dxgi_stereo_support_ = d3d11_re.DXGIFactory2()->IsWindowedStereoEnabled() ? true : false;

#ifdef KLAYGE_PLATFORM_WINDOWS_STORE
			// Async swap chain doesn't get along very well with desktop full screen
			dxgi_async_swap_chain_ = true;
#endif
		}
		if (d3d11_re.DXGISubVer() >= 5)
		{
			BOOL allow_tearing = FALSE;
			if (SUCCEEDED(d3d11_re.DXGIFactory5()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allow_tearing, sizeof(allow_tearing))))
			{
				dxgi_allow_tearing_ = allow_tearing ? true : false;
			}
		}

		if (d3d_device)
		{
			d3d_imm_ctx = d3d11_re.D3DDeviceImmContext();

			main_wnd_ = false;
		}
		else
		{
			UINT const create_device_flags = 0;
			static UINT const available_create_device_flags[] =
			{
#ifdef KLAYGE_DEBUG
				create_device_flags | D3D11_CREATE_DEVICE_DEBUG,
#endif
				create_device_flags
			};

			static std::pair<D3D_DRIVER_TYPE, std::wstring_view> const dev_type_behaviors[] =
			{
				std::make_pair(D3D_DRIVER_TYPE_HARDWARE, L"HW"),
				std::make_pair(D3D_DRIVER_TYPE_WARP, L"WARP")
			};

			static D3D_FEATURE_LEVEL constexpr all_feature_levels[] =
			{
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0
			};

			ArrayRef<D3D_FEATURE_LEVEL> feature_levels;
			{
				static size_t constexpr feature_level_name_hashes[] =
				{
					CT_HASH("12_1"),
					CT_HASH("12_0"),
					CT_HASH("11_1"),
					CT_HASH("11_0")
				};
				KLAYGE_STATIC_ASSERT(std::size(feature_level_name_hashes) == std::size(all_feature_levels));

				uint32_t feature_level_start_index = 0;
				if (d3d11_re.DXGISubVer() < 4)
				{
					feature_level_start_index = 2;

					if (d3d11_re.DXGISubVer() < 2)
					{
						feature_level_start_index = 3;
					}
				}
				for (size_t index = 0; index < settings.options.size(); ++ index)
				{
					size_t const opt_name_hash = RT_HASH(settings.options[index].first.c_str());
					size_t const opt_val_hash = RT_HASH(settings.options[index].second.c_str());
					if (CT_HASH("level") == opt_name_hash)
					{
						for (uint32_t i = feature_level_start_index; i < std::size(feature_level_name_hashes); ++ i)
						{
							if (feature_level_name_hashes[i] == opt_val_hash)
							{
								feature_level_start_index = i;
								break;
							}
						}
					}
				}

				feature_levels = ArrayRef<D3D_FEATURE_LEVEL>(all_feature_levels).Slice(feature_level_start_index);
			}

			for (auto const & dev_type_beh : dev_type_behaviors)
			{
				d3d_device = nullptr;
				d3d_imm_ctx = nullptr;
				IDXGIAdapter* dx_adapter = nullptr;
				D3D_DRIVER_TYPE dev_type = dev_type_beh.first;
				if (D3D_DRIVER_TYPE_HARDWARE == dev_type)
				{
					dx_adapter = adapter_->DXGIAdapter().get();
					dev_type = D3D_DRIVER_TYPE_UNKNOWN;
				}
				D3D_FEATURE_LEVEL out_feature_level = D3D_FEATURE_LEVEL_11_0;
				HRESULT hr = E_FAIL;
				for (auto const & flags : available_create_device_flags)
				{
					ID3D11Device* this_device = nullptr;
					ID3D11DeviceContext* this_imm_ctx = nullptr;
					D3D_FEATURE_LEVEL this_out_feature_level;
					hr = d3d11_re.D3D11CreateDevice(dx_adapter, dev_type, nullptr, flags,
						&feature_levels[0], static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION, &this_device,
						&this_out_feature_level, &this_imm_ctx);
					if (SUCCEEDED(hr))
					{
						d3d_device = this_device;
						d3d_imm_ctx = this_imm_ctx;
						out_feature_level = this_out_feature_level;
						break;
					}
				}
				if (SUCCEEDED(hr))
				{
					d3d11_re.D3DDevice(d3d_device, d3d_imm_ctx, out_feature_level);

					if (Context::Instance().AppInstance().ConfirmDevice())
					{
						if (dev_type != D3D_DRIVER_TYPE_HARDWARE)
						{
							IDXGIDevice1* dxgi_device = nullptr;
							hr = d3d_device->QueryInterface(IID_IDXGIDevice1, reinterpret_cast<void**>(&dxgi_device));
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
#ifdef KLAYGE_PLATFORM_WINDOWS_STORE
							dxgi_device->SetMaximumFrameLatency(1);
#endif
							dxgi_device->Release();
						}

						description_ = adapter_->Description() + L" " + dev_type_beh.second.data() + L" FL ";
						std::wstring_view fl_str;
						switch (static_cast<uint32_t>(out_feature_level))
						{
						case D3D_FEATURE_LEVEL_12_1:
							fl_str = L"12.1";
							break;

						case D3D_FEATURE_LEVEL_12_0:
							fl_str = L"12.0";
							break;

						case D3D_FEATURE_LEVEL_11_1:
							fl_str = L"11.1";
							break;

						case D3D_FEATURE_LEVEL_11_0:
							fl_str = L"11.0";
							break;

						default:
							fl_str = L"Unknown";
							break;
						}
						description_ += fl_str.data();
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
						d3d_device->Release();
						d3d_device = nullptr;

						d3d_imm_ctx->Release();
						d3d_imm_ctx = nullptr;
					}
				}
			}

			main_wnd_ = true;
		}

		Verify(d3d_device != nullptr);
		Verify(d3d_imm_ctx != nullptr);
		KLAYGE_ASSUME(d3d_device != nullptr);
		KLAYGE_ASSUME(d3d_imm_ctx != nullptr);

		depth_stencil_fmt_ = settings.depth_stencil_fmt;
		if (IsDepthFormat(depth_stencil_fmt_))
		{
			BOOST_ASSERT((EF_D32F == depth_stencil_fmt_) || (EF_D24S8 == depth_stencil_fmt_)
				|| (EF_D16 == depth_stencil_fmt_));

			UINT format_support;

			if (EF_D32F == depth_stencil_fmt_)
			{
				// Try 32-bit zbuffer
				d3d_device->CheckFormatSupport(DXGI_FORMAT_D32_FLOAT, &format_support);
				if (!(format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL))
				{
					depth_stencil_fmt_ = EF_D24S8;
				}
			}
			if (EF_D24S8 == depth_stencil_fmt_)
			{
				d3d_device->CheckFormatSupport(DXGI_FORMAT_D24_UNORM_S8_UINT, &format_support);
				if (!(format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL))
				{
					depth_stencil_fmt_ = EF_D16;
				}
			}
			if (EF_D16 == depth_stencil_fmt_)
			{
				d3d_device->CheckFormatSupport(DXGI_FORMAT_D16_UNORM, &format_support);
				if (!(format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL))
				{
					depth_stencil_fmt_ = EF_Unknown;
				}
			}
		}

		Window::WindowRotation const rotation = main_wnd->Rotation();
		if ((Window::WR_Rotate90 == rotation) || (Window::WR_Rotate270 == rotation))
		{
			std::swap(width_, height_);
		}

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (d3d11_re.DXGISubVer() >= 2)
		{
			bool stereo = (STM_LCDShutter == settings.stereo_method) && dxgi_stereo_support_;

			d3d11_re.DXGIFactory2()->RegisterStereoStatusWindow(hWnd_, WM_SIZE, &stereo_cookie_);

			sc_desc1_.Width = this->Width();
			sc_desc1_.Height = this->Height();
			sc_desc1_.Format = back_buffer_format_;
			sc_desc1_.Stereo = stereo;
			sc_desc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sc_desc1_.BufferCount = 2;
			sc_desc1_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			sc_desc1_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			if (stereo)
			{
				sc_desc1_.SampleDesc.Count = 1;
				sc_desc1_.SampleDesc.Quality = 0;
				sc_desc1_.Scaling = DXGI_SCALING_NONE;
				sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			}
			else
			{
				sc_desc1_.SampleDesc.Count = std::min(static_cast<uint32_t>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
				sc_desc1_.SampleDesc.Quality = settings.sample_quality;
				sc_desc1_.Scaling = DXGI_SCALING_STRETCH;
				if (dxgi_allow_tearing_)
				{
					sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				}
				else
				{
					sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
				}
			}
			if (dxgi_allow_tearing_)
			{
				sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			}
			if (dxgi_async_swap_chain_)
			{
				sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
			}

			sc_fs_desc_.RefreshRate.Numerator = 60;
			sc_fs_desc_.RefreshRate.Denominator = 1;
			sc_fs_desc_.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sc_fs_desc_.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sc_fs_desc_.Windowed = !this->FullScreen();
		}
		else
		{
			sc_desc_.BufferDesc.Width = this->Width();
			sc_desc_.BufferDesc.Height = this->Height();
			sc_desc_.BufferDesc.RefreshRate.Numerator = 60;
			sc_desc_.BufferDesc.RefreshRate.Denominator = 1;
			sc_desc_.BufferDesc.Format = back_buffer_format_;
			sc_desc_.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sc_desc_.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sc_desc_.SampleDesc.Count = std::min(static_cast<uint32_t>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
			sc_desc_.SampleDesc.Quality = settings.sample_quality;
			sc_desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sc_desc_.BufferCount = 2;
			sc_desc_.OutputWindow = hWnd_;
			sc_desc_.Windowed = !this->FullScreen();
			sc_desc_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			sc_desc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		}
#else
		bool stereo = (STM_LCDShutter == settings.stereo_method) && dxgi_stereo_support_;

		ComPtr<IDisplayInformationStatics> disp_info_stat;
		TIFHR(GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(),
			&disp_info_stat));

		auto callback = Callback<ITypedEventHandler<DisplayInformation*, IInspectable*>>(
			[this](IDisplayInformation* sender, IInspectable* args)
			{
				return this->OnStereoEnabledChanged(sender, args);
			});

		ComPtr<IDisplayInformation> disp_info;
		TIFHR(disp_info_stat->GetForCurrentView(&disp_info));
		disp_info->add_StereoEnabledChanged(callback.Get(), &stereo_enabled_changed_token_);

		sc_desc1_.Width = this->Width();
		sc_desc1_.Height = this->Height();
		sc_desc1_.Format = back_buffer_format_;
		sc_desc1_.Stereo = stereo;
		sc_desc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sc_desc1_.BufferCount = 2;
		sc_desc1_.Scaling = DXGI_SCALING_NONE;
		sc_desc1_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sc_desc1_.Flags = 0;
		if (stereo)
		{
			sc_desc1_.SampleDesc.Count = 1;
			sc_desc1_.SampleDesc.Quality = 0;
		}
		else
		{
			sc_desc1_.SampleDesc.Count = std::min(static_cast<uint32_t>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
			sc_desc1_.SampleDesc.Quality = settings.sample_quality;
		}
		if (dxgi_allow_tearing_)
		{
			sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		}
		else
		{
			sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

			sync_interval_ = std::max(1U, sync_interval_);
		}
		if (dxgi_async_swap_chain_)
		{
			sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		}
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if ((STM_LCDShutter == settings.stereo_method) && !dxgi_stereo_support_)
		{
			if (adapter_->Description().find(L"AMD", 0) != std::wstring::npos)
			{
#ifdef KLAYGE_PLATFORM_WIN64
				const TCHAR* ati_driver = TEXT("atidxx64.dll");
#else
				const TCHAR* ati_driver = TEXT("atidxx32.dll");
#endif
				HMODULE dll = ::GetModuleHandle(ati_driver);
				if (dll != nullptr)
				{
					PFNAmdDxExtCreate11 AmdDxExtCreate11 = reinterpret_cast<PFNAmdDxExtCreate11>(::GetProcAddress(dll, "AmdDxExtCreate11"));
					if (AmdDxExtCreate11 != nullptr)
					{
						IAmdDxExt* amd_dx_ext;
						HRESULT hr = AmdDxExtCreate11(d3d_device, &amd_dx_ext);
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
#endif

		this->CreateSwapChain(d3d_device, settings.display_output_method != DOM_sRGB);
		Verify(!!swap_chain_);

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		d3d11_re.DXGIFactory1()->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
		swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#endif

		{
			ID3D10Multithread* d3d_multithread;
			if (SUCCEEDED(d3d_device->QueryInterface(IID_ID3D10Multithread, reinterpret_cast<void**>(&d3d_multithread))))
			{
				d3d_multithread->SetMultithreadProtected(true);
				d3d_multithread->Release();
			}
		}

		this->UpdateSurfacesPtrs();

#ifdef KLAYGE_DEBUG
		ID3D11InfoQueue* d3d_info_queue = nullptr;
		if (SUCCEEDED(d3d_device->QueryInterface(IID_ID3D11InfoQueue, reinterpret_cast<void**>(&d3d_info_queue))))
		{
			d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

			d3d_info_queue->Release();
		}
#endif
	}

	D3D11RenderWindow::~D3D11RenderWindow()
	{
		on_exit_size_move_connect_.disconnect();
		on_size_connect_.disconnect();

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

		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
		Window::WindowRotation const rotation = main_wnd->Rotation();
		if ((Window::WR_Rotate90 == rotation) || (Window::WR_Rotate270 == rotation))
		{
			std::swap(width_, height_);
		}

		// Notify viewports of resize
		viewport_->width = width_;
		viewport_->height = height_;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
		ID3D11DeviceContext* d3d_imm_ctx = d3d11_re.D3DDeviceImmContext();
		if (d3d_imm_ctx)
		{
			d3d_imm_ctx->ClearState();
			d3d_imm_ctx->Flush();
		}

		for (size_t i = 0; i < clr_views_.size(); ++ i)
		{
			clr_views_[i].reset();
		}
		ds_view_.reset();

		render_target_view_right_eye_.reset();
		depth_stencil_view_right_eye_.reset();
		render_target_view_.reset();
		depth_stencil_view_.reset();
		back_buffer_.reset();
		depth_stencil_.reset();

		UINT flags = 0;
		if (this->FullScreen())
		{
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		}
		if (dxgi_allow_tearing_)
		{
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		}
		if (dxgi_async_swap_chain_)
		{
			flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		}

		this->OnUnbind();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (d3d11_re.DXGISubVer() >= 2)
		{
			dxgi_stereo_support_ = d3d11_re.DXGIFactory2()->IsWindowedStereoEnabled() ? true : false;

			sc_desc1_.Width = width_;
			sc_desc1_.Height = height_;
			sc_desc1_.Stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;
		}
		else
		{
			dxgi_stereo_support_ = false;

			sc_desc_.BufferDesc.Width = width_;
			sc_desc_.BufferDesc.Height = height_;
		}
#else
		dxgi_stereo_support_ = d3d11_re.DXGIFactory2()->IsWindowedStereoEnabled() ? true : false;

		sc_desc1_.Width = width_;
		sc_desc1_.Height = height_;
		sc_desc1_.Stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;
#endif

		if (!!swap_chain_)
		{
			swap_chain_->ResizeBuffers(2, width_, height_, back_buffer_format_, flags);
		}
		else
		{
			ID3D11Device* d3d_device = d3d11_re.D3DDevice();
			this->CreateSwapChain(d3d_device, Context::Instance().Config().graphics_cfg.display_output_method != DOM_sRGB);
			Verify(!!swap_chain_);

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#endif
		}

		this->UpdateSurfacesPtrs();

		d3d11_re.ResetRenderStates();
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

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
			if (d3d11_re.DXGISubVer() >= 2)
			{
				sc_desc1_.Width = width_;
				sc_desc1_.Height = height_;
				sc_fs_desc_.Windowed = !fs;
			}
			else
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
				desc.Width = width_;
				desc.Height = height_;
				desc.RefreshRate.Numerator = 60;
				desc.RefreshRate.Denominator = 1;
				desc.Format = back_buffer_format_;
				desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				swap_chain_->ResizeTarget(&desc);
			}

			::ShowWindow(hWnd_, SW_SHOWNORMAL);
			::UpdateWindow(hWnd_);
		}
#else
		if (isFullScreen_ != fs)
		{
			WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
			if (main_wnd->FullScreen(fs))
			{
				isFullScreen_ = fs;
			}
		}
#endif
	}

	void D3D11RenderWindow::WindowMovedOrResized()
	{
		::RECT rect;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		::GetClientRect(hWnd_, &rect);
#else
		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
		float const dpi_scale = main_wnd->DPIScale();

		ABI::Windows::Foundation::Rect rc;
		wnd_->get_Bounds(&rc);
		rect.left = static_cast<LONG>(rc.X * dpi_scale + 0.5f);
		rect.right = static_cast<LONG>((rc.X + rc.Width) * dpi_scale + 0.5f);
		rect.top = static_cast<LONG>(rc.Y * dpi_scale + 0.5f);
		rect.bottom = static_cast<LONG>((rc.Y + rc.Height) * dpi_scale + 0.5f);
#endif

		uint32_t new_left = rect.left;
		uint32_t new_top = rect.top;
		if ((new_left != left_) || (new_top != top_))
		{
			this->Reposition(new_left, new_top);
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());

		bool stereo_changed = false;
		if (d3d11_re.DXGISubVer() >= 2)
		{
			stereo_changed = ((d3d11_re.DXGIFactory2()->IsWindowedStereoEnabled() ? true : false) != dxgi_stereo_support_);
		}

		uint32_t new_width = std::max(rect.right - rect.left, 16L);
		uint32_t new_height = std::max(rect.bottom - rect.top, 16L);
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

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
		if (d3d11_re.DXGISubVer() >= 2)
		{
			d3d11_re.DXGIFactory2()->UnregisterStereoStatus(stereo_cookie_);
		}
#else
		ComPtr<IDisplayInformationStatics> disp_info_stat;
		TIFHR(GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(),
			&disp_info_stat));

		ComPtr<IDisplayInformation> disp_info;
		TIFHR(disp_info_stat->GetForCurrentView(&disp_info));
		disp_info->remove_StereoEnabledChanged(stereo_enabled_changed_token_);

		this->FullScreen(false);
#endif

		if (frame_latency_waitable_obj_ != 0)
		{
			::CloseHandle(frame_latency_waitable_obj_);
		}

		render_target_view_right_eye_.reset();
		depth_stencil_view_right_eye_.reset();
		render_target_view_.reset();
		depth_stencil_view_.reset();
		back_buffer_.reset();
		depth_stencil_.reset();
		swap_chain_.reset();
	}

	void D3D11RenderWindow::UpdateSurfacesPtrs()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (dxgi_allow_tearing_)
#endif
		{
			D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
			if (d3d11_re.DXGISubVer() >= 2)
			{
				BOOST_ASSERT(swap_chain_1_);

				WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
				Window::WindowRotation const rotation = main_wnd->Rotation();

				DXGI_MODE_ROTATION dxgi_rotation;
				switch (rotation)
				{
				case Window::WR_Identity:
					dxgi_rotation = DXGI_MODE_ROTATION_IDENTITY;
					break;

				case Window::WR_Rotate90:
					dxgi_rotation = DXGI_MODE_ROTATION_ROTATE90;
					break;

				case Window::WR_Rotate180:
					dxgi_rotation = DXGI_MODE_ROTATION_ROTATE180;
					break;

				case Window::WR_Rotate270:
					dxgi_rotation = DXGI_MODE_ROTATION_ROTATE270;
					break;

				default:
					KFL_UNREACHABLE("Invalid rotation mode");
				}

				TIFHR(swap_chain_1_->SetRotation(dxgi_rotation));
			}
		}

		ID3D11Texture2D* back_buffer;
		TIFHR(swap_chain_->GetBuffer(0, IID_ID3D11Texture2D, reinterpret_cast<void**>(&back_buffer)));
		back_buffer_ = MakeSharedPtr<D3D11Texture2D>(MakeCOMPtr(back_buffer));

		render_target_view_ = rf.Make2DRenderView(*back_buffer_, 0, 1, 0);

		bool stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;
	
		if (stereo)
		{
			render_target_view_right_eye_ = rf.Make2DRenderView(*back_buffer_, 1, 1, 0);
		}

		if (depth_stencil_fmt_ != EF_Unknown)
		{
			depth_stencil_ = rf.MakeTexture2D(width_, height_, 1, stereo ? 2 : 1, depth_stencil_fmt_,
				back_buffer_->SampleCount(), back_buffer_->SampleQuality(),
				EAH_GPU_Read | EAH_GPU_Write);

			depth_stencil_view_ = rf.Make2DDepthStencilRenderView(*depth_stencil_, 0, 1, 0);

			if (stereo)
			{
				depth_stencil_view_right_eye_ = rf.Make2DDepthStencilRenderView(*depth_stencil_, 1, 1, 0);
			}
		}

		if (!!stereo_amd_qb_ext_)
		{
			stereo_amd_right_eye_height_ = stereo_amd_qb_ext_->GetLineOffset(swap_chain_.get());
		}

		this->Attach(ATT_Color0, render_target_view_);
		if (depth_stencil_view_)
		{
			this->Attach(ATT_DepthStencil, depth_stencil_view_);
		}
	}

	void D3D11RenderWindow::CreateSwapChain(ID3D11Device* d3d_device, bool try_hdr_display)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (d3d11_re.DXGISubVer() >= 2)
		{
			IDXGISwapChain1* sc1 = nullptr;
			d3d11_re.DXGIFactory2()->CreateSwapChainForHwnd(d3d_device, hWnd_,
				&sc_desc1_, &sc_fs_desc_, nullptr, &sc1);
			swap_chain_1_ = MakeCOMPtr(sc1);

			IDXGISwapChain* sc;
			swap_chain_1_->QueryInterface(IID_IDXGISwapChain, reinterpret_cast<void**>(&sc));
			swap_chain_ = MakeCOMPtr(sc);
		}
		else
		{
			IDXGISwapChain* sc = nullptr;
			d3d11_re.DXGIFactory1()->CreateSwapChain(d3d_device, &sc_desc_, &sc);
			swap_chain_ = MakeCOMPtr(sc);

			IDXGISwapChain1* sc1;
			if (SUCCEEDED(swap_chain_->QueryInterface(IID_IDXGISwapChain1, reinterpret_cast<void**>(&sc1))))
			{
				swap_chain_1_ = MakeCOMPtr(sc1);
			}
		}
#else
		IDXGISwapChain1* sc1 = nullptr;
		d3d11_re.DXGIFactory2()->CreateSwapChainForCoreWindow(d3d_device,
			static_cast<IUnknown*>(wnd_.get()), &sc_desc1_, nullptr, &sc1);
		swap_chain_1_ = MakeCOMPtr(sc1);

		IDXGISwapChain* sc;
		swap_chain_1_->QueryInterface(IID_IDXGISwapChain, reinterpret_cast<void**>(&sc));
		swap_chain_ = MakeCOMPtr(sc);
#endif

		if (dxgi_async_swap_chain_)
		{
			IDXGISwapChain3* sc3;
			if (SUCCEEDED(swap_chain_->QueryInterface(IID_IDXGISwapChain3, reinterpret_cast<void**>(&sc3))))
			{
				if (frame_latency_waitable_obj_ != 0)
				{
					::CloseHandle(frame_latency_waitable_obj_);
				}
				frame_latency_waitable_obj_ = sc3->GetFrameLatencyWaitableObject();
				sc3->Release();
			}
		}

		if (try_hdr_display)
		{
			IDXGISwapChain4* sc4;
			if (SUCCEEDED(swap_chain_->QueryInterface(IID_IDXGISwapChain4, reinterpret_cast<void**>(&sc4))))
			{
				UINT color_space_support;
				if (SUCCEEDED(sc4->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &color_space_support))
					&& (color_space_support & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
				{
					sc4->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
				}

				sc4->Release();
			}
		}
	}

	void D3D11RenderWindow::SwapBuffers()
	{
		if (swap_chain_)
		{
			bool allow_tearing = dxgi_allow_tearing_ && (sync_interval_ == 0);
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			allow_tearing &= !isFullScreen_;
#endif
			UINT const present_flags = allow_tearing ? DXGI_PRESENT_ALLOW_TEARING : 0;
			TIFHR(swap_chain_->Present(sync_interval_, present_flags));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
			if (d3d11_re.DXGISubVer() >= 2)
			{
				render_target_view_->Discard();
				if (depth_stencil_view_)
				{
					depth_stencil_view_->Discard();
				}
				if (render_target_view_right_eye_)
				{
					render_target_view_right_eye_->Discard();
				}
				if (depth_stencil_view_right_eye_)
				{
					depth_stencil_view_right_eye_->Discard();
				}
			}

			if (DXGI_PRESENT_ALLOW_TEARING == present_flags)
			{
				d3d11_re.InvalidRTVCache();
				views_dirty_ = true;
			}
		}
	}

	void D3D11RenderWindow::WaitOnSwapBuffers()
	{
		if (swap_chain_ && dxgi_async_swap_chain_)
		{
			::WaitForSingleObjectEx(frame_latency_waitable_obj_, 1000, true);
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

#if defined KLAYGE_PLATFORM_WINDOWS_STORE
	HRESULT D3D11RenderWindow::OnStereoEnabledChanged(IDisplayInformation* sender, IInspectable* args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D11RenderEngine& d3d11_re = *checked_cast<D3D11RenderEngine*>(&rf.RenderEngineInstance());
		if ((d3d11_re.DXGIFactory2()->IsWindowedStereoEnabled() ? true : false) != dxgi_stereo_support_)
		{
			swap_chain_.reset();
			this->WindowMovedOrResized();
		}

		return S_OK;
	}
#endif
}
