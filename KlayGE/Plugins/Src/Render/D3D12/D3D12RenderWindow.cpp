/**
 * @file D3D12RenderWindow.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

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

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12InterfaceLoader.hpp>
#include <KlayGE/D3D12/D3D12RenderWindow.hpp>

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
	D3D12RenderWindow::D3D12RenderWindow(D3D12Adapter* adapter, std::string const & name, RenderSettings const & settings)
						: adapter_(adapter), dxgi_allow_tearing_(false),
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

		back_buffer_format_ = D3D12Mapping::MappingFormat(format);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& d3d12_re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());

		dxgi_stereo_support_ = d3d12_re.DXGIFactory4()->IsWindowedStereoEnabled() ? true : false;
		if (d3d12_re.DXGISubVer() >= 5)
		{
			BOOL allow_tearing = FALSE;
			if (SUCCEEDED(d3d12_re.DXGIFactory5()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allow_tearing, sizeof(allow_tearing))))
			{
				dxgi_allow_tearing_ = allow_tearing ? true : false;
			}
		}

		viewport_->left		= 0;
		viewport_->top		= 0;
		viewport_->width	= width_;
		viewport_->height	= height_;

		ID3D12Device* d3d_device = nullptr;
		ID3D12CommandQueue* d3d_cmd_queue = nullptr;

		if (d3d12_re.D3DDevice())
		{
			d3d_device = d3d12_re.D3DDevice();
			d3d_cmd_queue = d3d12_re.D3DRenderCmdQueue();

			main_wnd_ = false;
		}
		else
		{
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

			for (size_t i = 0; i < feature_levels.size(); ++ i)
			{
				ID3D12Device* device = nullptr;
				if (SUCCEEDED(D3D12InterfaceLoader::Instance().D3D12CreateDevice(adapter_->DXGIAdapter().get(),
						feature_levels[i], IID_ID3D12Device, reinterpret_cast<void**>(&device))))
				{
					d3d_device = device;

					D3D12_COMMAND_QUEUE_DESC queue_desc;
					queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
					queue_desc.Priority = 0;
					queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
					queue_desc.NodeMask = 0;

					ID3D12CommandQueue* cmd_queue;
					TIFHR(d3d_device->CreateCommandQueue(&queue_desc,
						IID_ID3D12CommandQueue, reinterpret_cast<void**>(&cmd_queue)));
					d3d_cmd_queue = cmd_queue;

					D3D12_FEATURE_DATA_FEATURE_LEVELS req_feature_levels;
					req_feature_levels.NumFeatureLevels = static_cast<UINT>(feature_levels.size());
					req_feature_levels.pFeatureLevelsRequested = &feature_levels[0];
					d3d_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &req_feature_levels, sizeof(req_feature_levels));

					d3d12_re.D3DDevice(d3d_device, d3d_cmd_queue, req_feature_levels.MaxSupportedFeatureLevel);

					if (Context::Instance().AppInstance().ConfirmDevice())
					{
						description_ = adapter_->Description() + L" FL ";
						std::wstring_view fl_str;
						switch (req_feature_levels.MaxSupportedFeatureLevel)
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

						d3d_cmd_queue->Release();
						d3d_cmd_queue = nullptr;
					}
				}
			}

			main_wnd_ = true;
		}

		Verify(!!d3d_device);
		Verify(!!d3d_cmd_queue);

		depth_stencil_fmt_ = settings.depth_stencil_fmt;

		Window::WindowRotation const rotation = main_wnd->Rotation();
		if ((Window::WR_Rotate90 == rotation) || (Window::WR_Rotate270 == rotation))
		{
			std::swap(width_, height_);
		}

		bool stereo = (STM_LCDShutter == settings.stereo_method) && dxgi_stereo_support_;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		d3d12_re.DXGIFactory4()->RegisterStereoStatusWindow(hWnd_, WM_SIZE, &stereo_cookie_);
#else
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
#endif

		sc_desc1_.Width = this->Width();
		sc_desc1_.Height = this->Height();
		sc_desc1_.Format = back_buffer_format_;
		sc_desc1_.Stereo = stereo;
		sc_desc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sc_desc1_.BufferCount = NUM_BACK_BUFFERS;
		sc_desc1_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sc_desc1_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		if (stereo)
		{
			sc_desc1_.SampleDesc.Count = 1;
			sc_desc1_.SampleDesc.Quality = 0;
			sc_desc1_.Scaling = DXGI_SCALING_NONE;
			sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		}
		else
		{
			sc_desc1_.SampleDesc.Count = std::min(static_cast<uint32_t>(D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
			sc_desc1_.SampleDesc.Quality = settings.sample_quality;
			sc_desc1_.Scaling = DXGI_SCALING_STRETCH;
			sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		}
		if (dxgi_allow_tearing_)
		{
			sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		}
#ifdef KLAYGE_PLATFORM_WINDOWS_STORE
		else
		{
			sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

			sync_interval_ = std::max(1U, sync_interval_);
		}
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		sc_fs_desc_.RefreshRate.Numerator = 60;
		sc_fs_desc_.RefreshRate.Denominator = 1;
		sc_fs_desc_.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sc_fs_desc_.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sc_fs_desc_.Windowed = !this->FullScreen();
#endif

		this->CreateSwapChain(d3d_cmd_queue, settings.display_output_method != DOM_sRGB);
		Verify(!!swap_chain_);

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		d3d12_re.DXGIFactory4()->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
		swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#endif

		curr_back_buffer_ = swap_chain_->GetCurrentBackBufferIndex();

		this->UpdateSurfacesPtrs();

#ifdef KLAYGE_DEBUG
		ID3D12InfoQueue* d3d_info_queue = nullptr;
		if (SUCCEEDED(d3d_device->QueryInterface(IID_ID3D12InfoQueue, reinterpret_cast<void**>(&d3d_info_queue))))
		{
			d3d_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			d3d_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

			D3D12_MESSAGE_ID hide[] =
			{
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
			};

			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			d3d_info_queue->AddStorageFilterEntries(&filter);

			d3d_info_queue->Release();
		}
#endif
	}

	D3D12RenderWindow::~D3D12RenderWindow()
	{
		on_exit_size_move_connect_.disconnect();
		on_size_connect_.disconnect();

		this->Destroy();
	}

	std::wstring const & D3D12RenderWindow::Description() const
	{
		return description_;
	}

	// 改变窗口大小
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderWindow::Resize(uint32_t width, uint32_t height)
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
		D3D12RenderEngine& d3d12_re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = d3d12_re.D3DRenderCmdList();
		if (cmd_list)
		{
			cmd_list->ClearState(nullptr);
		}

		for (size_t i = 0; i < clr_views_.size(); ++ i)
		{
			clr_views_[i].reset();
		}
		ds_view_.reset();

		for (size_t i = 0; i < render_targets_.size(); ++ i)
		{
			render_target_render_views_right_eye_[i].reset();
			render_target_render_views_[i].reset();
			render_targets_[i].reset();
		}
		depth_stencil_.reset();

		UINT flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		if (this->FullScreen())
		{
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		}
		if (dxgi_allow_tearing_)
		{
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		}

		this->OnUnbind();

		dxgi_stereo_support_ = d3d12_re.DXGIFactory4()->IsWindowedStereoEnabled() ? true : false;

		sc_desc1_.Width = width_;
		sc_desc1_.Height = height_;
		sc_desc1_.Stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;

		if (!!swap_chain_)
		{
			swap_chain_->ResizeBuffers(2, width_, height_, back_buffer_format_, flags);
		}
		else
		{
			ID3D12CommandQueue* cmd_queue = d3d12_re.D3DRenderCmdQueue();

			this->CreateSwapChain(cmd_queue, Context::Instance().Config().graphics_cfg.display_output_method != DOM_sRGB);
			Verify(!!swap_chain_);

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#endif
		}

		this->UpdateSurfacesPtrs();

		d3d12_re.ResetRenderStates();
	}

	// 改变窗口位置
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderWindow::Reposition(uint32_t left, uint32_t top)
	{
		left_ = left;
		top_ = top;
	}

	// 获取是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	bool D3D12RenderWindow::FullScreen() const
	{
		return isFullScreen_;
	}

	// 设置是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderWindow::FullScreen(bool fs)
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

			sc_desc1_.Width = width_;
			sc_desc1_.Height = height_;
			sc_fs_desc_.Windowed = !fs;

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

	void D3D12RenderWindow::WindowMovedOrResized()
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
		D3D12RenderEngine& d3d12_re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());
		bool stereo_changed = ((d3d12_re.DXGIFactory4()->IsWindowedStereoEnabled() ? true : false) != dxgi_stereo_support_);

		uint32_t new_width = rect.right - rect.left;
		uint32_t new_height = rect.bottom - rect.top;
		if ((new_width != width_) || (new_height != height_) || stereo_changed)
		{
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(new_width, new_height);
		}
	}

	void D3D12RenderWindow::Destroy()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& d3d12_re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());

		d3d12_re.ForceFinish();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (swap_chain_)
		{
			swap_chain_->SetFullscreenState(false, nullptr);
		}

		d3d12_re.DXGIFactory4()->UnregisterStereoStatus(stereo_cookie_);
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

		for (size_t i = 0; i < render_targets_.size(); ++ i)
		{
			render_target_render_views_right_eye_[i].reset();
			render_target_render_views_[i].reset();
			render_targets_[i].reset();
		}

		depth_stencil_.reset();
		swap_chain_.reset();
	}

	void D3D12RenderWindow::UpdateSurfacesPtrs()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
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

		TIFHR(swap_chain_->SetRotation(dxgi_rotation));

		for (size_t i = 0; i < render_targets_.size(); ++ i)
		{
			ID3D12Resource* bb12;
			TIFHR(swap_chain_->GetBuffer(static_cast<UINT>(i),
				IID_ID3D12Resource, reinterpret_cast<void**>(&bb12)));
			render_targets_[i] = MakeSharedPtr<D3D12Texture2D>(MakeCOMPtr<ID3D12Resource>(bb12));
		}
		
		bool stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;

		if (depth_stencil_fmt_ != EF_Unknown)
		{
			depth_stencil_ = rf.MakeTexture2D(this->Width(), this->Height(), 1, stereo ? 2 : 1, depth_stencil_fmt_,
				render_targets_[0]->SampleCount(), render_targets_[0]->SampleQuality(), EAH_GPU_Read | EAH_GPU_Write);
		}

		for (size_t i = 0; i < render_targets_.size(); ++ i)
		{
			render_target_render_views_[i] = rf.Make2DRenderView(*render_targets_[i], 0, 1, 0);
		}
		if (stereo)
		{
			for (size_t i = 0; i < render_targets_.size(); ++ i)
			{
				render_target_render_views_right_eye_[i] = rf.Make2DRenderView(*render_targets_[i], 1, 1, 0);
			}
		}
		this->Attach(ATT_Color0, render_target_render_views_[0]);
		if (depth_stencil_fmt_ != EF_Unknown)
		{
			this->Attach(ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*depth_stencil_, 0, 1, 0));
			if (stereo)
			{
				this->Attach(ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*depth_stencil_, 1, 1, 0));
			}
		}
	}

	void D3D12RenderWindow::CreateSwapChain(ID3D12CommandQueue* d3d_cmd_queue, bool try_hdr_display)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& d3d12_re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());

		IDXGISwapChain1* sc = nullptr;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		d3d12_re.DXGIFactory4()->CreateSwapChainForHwnd(d3d_cmd_queue, hWnd_,
			&sc_desc1_, &sc_fs_desc_, nullptr, &sc);
#else
		d3d12_re.DXGIFactory4()->CreateSwapChainForCoreWindow(d3d_cmd_queue,
			static_cast<IUnknown*>(wnd_.get()), &sc_desc1_, nullptr, &sc);
#endif

		IDXGISwapChain3* sc3 = nullptr;
		sc->QueryInterface(IID_IDXGISwapChain3, reinterpret_cast<void**>(&sc3));
		swap_chain_ = MakeCOMPtr(sc3);
		sc->Release();

		if (frame_latency_waitable_obj_ != 0)
		{
			::CloseHandle(frame_latency_waitable_obj_);
		}
		frame_latency_waitable_obj_ = swap_chain_->GetFrameLatencyWaitableObject();

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

	void D3D12RenderWindow::SwapBuffers()
	{
		if (swap_chain_)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			D3D12RenderEngine& d3d12_re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());

			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			auto rt_tex = checked_cast<D3D12Texture*>(render_targets_[curr_back_buffer_].get());
			if (rt_tex->UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_PRESENT))
			{
				d3d12_re.D3DRenderCmdList()->ResourceBarrier(1, &barrier);
			}

			d3d12_re.CommitRenderCmd();

			bool allow_tearing = dxgi_allow_tearing_ && (sync_interval_ == 0);
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			allow_tearing &= !isFullScreen_;
#endif
			UINT const present_flags = allow_tearing ? DXGI_PRESENT_ALLOW_TEARING : 0;
			TIFHR(swap_chain_->Present(sync_interval_, present_flags));

			curr_back_buffer_ = swap_chain_->GetCurrentBackBufferIndex();

			this->Attach(ATT_Color0, render_target_render_views_[curr_back_buffer_]);
		}
	}

	void D3D12RenderWindow::WaitOnSwapBuffers()
	{
		if (swap_chain_)
		{
			::WaitForSingleObjectEx(frame_latency_waitable_obj_, 1000, true);
		}
	}

	void D3D12RenderWindow::OnExitSizeMove(Window const & /*win*/)
	{
		this->WindowMovedOrResized();
	}

	void D3D12RenderWindow::OnSize(Window const & win, bool active)
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
	HRESULT D3D12RenderWindow::OnStereoEnabledChanged(IDisplayInformation* sender, IInspectable* args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& d3d12_re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());
		if ((d3d12_re.DXGIFactory4()->IsWindowedStereoEnabled() ? true : false) != dxgi_stereo_support_)
		{
			swap_chain_.reset();
			this->WindowMovedOrResized();
		}

		return S_OK;
	}
#endif
}
