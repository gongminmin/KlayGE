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
#include <boost/lexical_cast.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12RenderFactory.hpp>
#include <KlayGE/D3D12/D3D12RenderFactoryInternal.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12InterfaceLoader.hpp>
#include <KlayGE/D3D12/D3D12RenderWindow.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
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
	D3D12RenderWindow::D3D12RenderWindow(IDXGIFactory4Ptr const & gi_factory, D3D12AdapterPtr const & adapter,
			std::string const & name, RenderSettings const & settings)
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
						: hWnd_(nullptr),
#else
						:
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
#endif
		on_exit_size_move_connect_ = main_wnd->OnExitSizeMove().connect(std::bind(&D3D12RenderWindow::OnExitSizeMove, this,
			std::placeholders::_1));
		on_size_connect_ = main_wnd->OnSize().connect(std::bind(&D3D12RenderWindow::OnSize, this,
			std::placeholders::_1, std::placeholders::_2));

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

		back_buffer_format_ = D3D12Mapping::MappingFormat(format);

		dxgi_stereo_support_ = gi_factory_->IsWindowedStereoEnabled() ? true : false;

		viewport_->left		= 0;
		viewport_->top		= 0;
		viewport_->width	= width_;
		viewport_->height	= height_;

		ID3D12DevicePtr d3d_device;
		ID3D12CommandQueuePtr d3d_cmd_queue;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& d3d12_re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());
		if (d3d12_re.D3DDevice())
		{
			d3d_device = d3d12_re.D3DDevice();
			d3d_cmd_queue = d3d12_re.D3DRenderCmdQueue();

			main_wnd_ = false;
		}
		else
		{
#ifdef KLAYGE_DEBUG
			{
				ID3D12Debug* debug_ctrl = nullptr;
				if (SUCCEEDED(D3D12InterfaceLoader::Instance().D3D12GetDebugInterface(
						IID_ID3D12Debug, reinterpret_cast<void**>(&debug_ctrl))))
				{
					BOOST_ASSERT(debug_ctrl);
					debug_ctrl->EnableDebugLayer();
					debug_ctrl->Release();
				}
			}
#endif

			std::vector<std::pair<char const *, D3D_FEATURE_LEVEL>> available_feature_levels;
			available_feature_levels.emplace_back("12_1", D3D_FEATURE_LEVEL_12_1);
			available_feature_levels.emplace_back("12_0", D3D_FEATURE_LEVEL_12_0);
			available_feature_levels.emplace_back("11_1", D3D_FEATURE_LEVEL_11_1);
			available_feature_levels.emplace_back("11_0", D3D_FEATURE_LEVEL_11_0);

			for (size_t index = 0; index < settings.options.size(); ++ index)
			{
				std::string const & opt_name = settings.options[index].first;
				std::string const & opt_val = settings.options[index].second;
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

			for (size_t i = 0; i < feature_levels.size(); ++ i)
			{
				ID3D12Device* device = nullptr;
				if (SUCCEEDED(D3D12InterfaceLoader::Instance().D3D12CreateDevice(adapter_->DXGIAdapter().get(),
						feature_levels[i], IID_ID3D12Device, reinterpret_cast<void**>(&device))))
				{
					d3d_device = MakeCOMPtr(device);

					D3D12_COMMAND_QUEUE_DESC queue_desc;
					queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
					queue_desc.Priority = 0;
					queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
					queue_desc.NodeMask = 0;

					ID3D12CommandQueue* cmd_queue;
					TIF(d3d_device->CreateCommandQueue(&queue_desc,
						IID_ID3D12CommandQueue, reinterpret_cast<void**>(&cmd_queue)));
					d3d_cmd_queue = MakeCOMPtr(cmd_queue);

					D3D12_FEATURE_DATA_FEATURE_LEVELS req_feature_levels;
					req_feature_levels.NumFeatureLevels = static_cast<UINT>(feature_levels.size());
					req_feature_levels.pFeatureLevelsRequested = &feature_levels[0];
					d3d_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &req_feature_levels, sizeof(req_feature_levels));

					d3d12_re.D3DDevice(d3d_device, d3d_cmd_queue, req_feature_levels.MaxSupportedFeatureLevel);

					if (Context::Instance().AppInstance().ConfirmDevice())
					{
						description_ = adapter_->Description() + L" FL ";
						wchar_t const * fl_str;
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
						d3d_cmd_queue.reset();
					}
				}
			}

			main_wnd_ = true;
		}

		Verify(!!d3d_device);
		Verify(!!d3d_cmd_queue);

		depth_stencil_fmt_ = settings.depth_stencil_fmt;

		bool stereo = (STM_LCDShutter == settings.stereo_method) && dxgi_stereo_support_;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		gi_factory_->RegisterStereoStatusWindow(hWnd_, WM_SIZE, &stereo_cookie_);
#else
		ComPtr<IDisplayInformationStatics> disp_info_stat;
		TIF(GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(),
			&disp_info_stat));

		auto callback = Callback<ITypedEventHandler<DisplayInformation*, IInspectable*>>(
			std::bind(&D3D12RenderWindow::OnStereoEnabledChanged, this, std::placeholders::_1, std::placeholders::_2));

		ComPtr<IDisplayInformation> disp_info;
		TIF(disp_info_stat->GetForCurrentView(&disp_info));
		disp_info->add_StereoEnabledChanged(callback.Get(), &stereo_enabled_changed_token_);
#endif

		sc_desc1_.Width = this->Width();
		sc_desc1_.Height = this->Height();
		sc_desc1_.Format = back_buffer_format_;
		sc_desc1_.Stereo = stereo;
		sc_desc1_.SampleDesc.Count = stereo ? 1 : std::min(static_cast<uint32_t>(D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
		sc_desc1_.SampleDesc.Quality = stereo ? 0 : settings.sample_quality;
		sc_desc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sc_desc1_.BufferCount = NUM_BACK_BUFFERS;
		sc_desc1_.Scaling = DXGI_SCALING_NONE;
		sc_desc1_.SwapEffect = stereo ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sc_desc1_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sc_desc1_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		sc_fs_desc_.RefreshRate.Numerator = 60;
		sc_fs_desc_.RefreshRate.Denominator = 1;
		sc_fs_desc_.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sc_fs_desc_.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sc_fs_desc_.Windowed = !this->FullScreen();
#endif

		IDXGISwapChain1* sc = nullptr;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		gi_factory_->CreateSwapChainForHwnd(d3d_cmd_queue.get(), hWnd_,
			&sc_desc1_, &sc_fs_desc_, nullptr, &sc);
#else
		gi_factory_->CreateSwapChainForCoreWindow(d3d_cmd_queue.get(),
			static_cast<IUnknown*>(wnd_.get()), &sc_desc1_, nullptr, &sc);
#endif

		IDXGISwapChain3* sc3 = nullptr;
		sc->QueryInterface(IID_IDXGISwapChain3, reinterpret_cast<void**>(&sc3));
		swap_chain_ = MakeCOMPtr(sc3);
		sc->Release();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		gi_factory->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
		swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#endif

		Verify(!!swap_chain_);

		curr_back_buffer_ = swap_chain_->GetCurrentBackBufferIndex();

		this->UpdateSurfacesPtrs();

		this->WaitForGPU();
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

		// Notify viewports of resize
		viewport_->width = width;
		viewport_->height = height;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();
		if (cmd_list)
		{
			cmd_list->ClearState(nullptr);
		}

		for (size_t i = 0; i < clr_views_.size(); ++ i)
		{
			clr_views_[i].reset();
		}
		rs_view_.reset();

		for (size_t i = 0; i < render_targets_.size(); ++ i)
		{
			render_target_render_views_right_eye_[i].reset();
			render_target_render_views_[i].reset();
			render_targets_[i].reset();
		}
		depth_stencil_.reset();

		UINT flags = 0;
		if (this->FullScreen())
		{
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		}

		this->OnUnbind();

		dxgi_stereo_support_ = gi_factory_->IsWindowedStereoEnabled() ? true : false;

		sc_desc1_.Width = width_;
		sc_desc1_.Height = height_;
		sc_desc1_.Stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;

		if (!!swap_chain_)
		{
			swap_chain_->ResizeBuffers(2, width, height, back_buffer_format_, flags);
		}
		else
		{
			ID3D12CommandQueuePtr const & cmd_queue = re.D3DRenderCmdQueue();

			IDXGISwapChain1* sc = nullptr;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			gi_factory_->CreateSwapChainForHwnd(cmd_queue.get(), hWnd_,
				&sc_desc1_, &sc_fs_desc_, nullptr, &sc);
#else
			gi_factory_->CreateSwapChainForCoreWindow(cmd_queue.get(),
				static_cast<IUnknown*>(wnd_.get()), &sc_desc1_, nullptr, &sc);
#endif

			IDXGISwapChain3* sc3 = nullptr;
			sc->QueryInterface(IID_IDXGISwapChain3, reinterpret_cast<void**>(&sc3));
			swap_chain_ = MakeCOMPtr(sc3);
			sc->Release();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
			swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#endif

			Verify(!!swap_chain_);
		}

		this->UpdateSurfacesPtrs();

		re.ResetRenderStates();

		this->OnBind();

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);
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
		KFL_UNUSED(fs);
#endif
	}

	void D3D12RenderWindow::WindowMovedOrResized()
	{
		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
		float const dpi_scale = main_wnd->DPIScale();

		::RECT rect;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		::GetClientRect(hWnd_, &rect);
#else
		ABI::Windows::Foundation::Rect rc;
		wnd_->get_Bounds(&rc);
		rect.left = static_cast<LONG>(rc.X);
		rect.right = static_cast<LONG>(rc.X + rc.Width);
		rect.top = static_cast<LONG>(rc.Y);
		rect.bottom = static_cast<LONG>(rc.Y + rc.Height);
#endif

		rect.left = static_cast<int32_t>(rect.left * dpi_scale + 0.5f);
		rect.top = static_cast<int32_t>(rect.top * dpi_scale + 0.5f);
		rect.right = static_cast<int32_t>(rect.right * dpi_scale + 0.5f);
		rect.bottom = static_cast<int32_t>(rect.bottom * dpi_scale + 0.5f);

		uint32_t new_left = rect.left;
		uint32_t new_top = rect.top;
		if ((new_left != left_) || (new_top != top_))
		{
			this->Reposition(new_left, new_top);
		}

		bool stereo_changed = ((gi_factory_->IsWindowedStereoEnabled() ? true : false) != dxgi_stereo_support_);

		uint32_t new_width = rect.right - rect.left;
		uint32_t new_height = rect.bottom - rect.top;
		if ((new_width != width_) || (new_height != height_) || stereo_changed)
		{
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(new_width, new_height);
		}
	}

	void D3D12RenderWindow::Destroy()
	{
		this->WaitForGPU();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (swap_chain_)
		{
			swap_chain_->SetFullscreenState(false, nullptr);
		}

		gi_factory_->UnregisterStereoStatus(stereo_cookie_);
#else
		ComPtr<IDisplayInformationStatics> disp_info_stat;
		TIF(GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(),
			&disp_info_stat));

		ComPtr<IDisplayInformation> disp_info;
		TIF(disp_info_stat->GetForCurrentView(&disp_info));
		disp_info->remove_StereoEnabledChanged(stereo_enabled_changed_token_);
#endif

		for (size_t i = 0; i < render_targets_.size(); ++ i)
		{
			render_target_render_views_right_eye_[i].reset();
			render_target_render_views_[i].reset();
			render_targets_[i].reset();
		}

		depth_stencil_.reset();
		swap_chain_.reset();
		gi_factory_.reset();
	}

	void D3D12RenderWindow::UpdateSurfacesPtrs()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		for (size_t i = 0; i < render_targets_.size(); ++ i)
		{
			ID3D12Resource* bb12;
			TIF(swap_chain_->GetBuffer(static_cast<UINT>(i),
				IID_ID3D12Resource, reinterpret_cast<void**>(&bb12)));
			render_targets_[i] = MakeSharedPtr<D3D12Texture2D>(MakeCOMPtr<ID3D12Resource>(bb12));
		}
		
		bool stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;

		if (depth_stencil_fmt_ != EF_Unknown)
		{
			depth_stencil_ = rf.MakeTexture2D(this->Width(), this->Height(), 1, stereo ? 2 : 1, depth_stencil_fmt_,
				render_targets_[0]->SampleCount(), render_targets_[0]->SampleQuality(), EAH_GPU_Read | EAH_GPU_Write, nullptr);
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

	void D3D12RenderWindow::SwapBuffers()
	{
		if (swap_chain_)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());

			re.ForceFlush();

			TIF(swap_chain_->Present(sync_interval_, 0));

			curr_back_buffer_ = swap_chain_->GetCurrentBackBufferIndex();

			this->WaitForGPU();

			re.ResetRenderCmd();
			re.ResetComputeCmd();
			re.ResetCopyCmd();
			re.ClearPSOCache();
		}
	}

	void D3D12RenderWindow::OnBind()
	{
		this->Attach(ATT_Color0, render_target_render_views_[curr_back_buffer_]);

		D3D12FrameBuffer::OnBind();
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

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
	HRESULT D3D12RenderWindow::OnStereoEnabledChanged(IDisplayInformation* sender, IInspectable* args)
	{
		KFL_UNUSED(sender);
		KFL_UNUSED(args);

		if ((gi_factory_->IsWindowedStereoEnabled() ? true : false) != dxgi_stereo_support_)
		{
			swap_chain_.reset();
			this->WindowMovedOrResized();
		}

		return S_OK;
	}
#endif

	void D3D12RenderWindow::WaitForGPU()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());

		re.SyncRenderCmd();
		re.SyncComputeCmd();
		re.SyncCopyCmd();
	}
}
