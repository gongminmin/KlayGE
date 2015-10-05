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
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12RenderFactory.hpp>
#include <KlayGE/D3D12/D3D12RenderFactoryInternal.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12InterfaceLoader.hpp>
#include <KlayGE/D3D12/D3D12RenderWindow.hpp>

namespace KlayGE
{
	D3D12RenderWindow::D3D12RenderWindow(IDXGIFactory4Ptr const & gi_factory, D3D12AdapterPtr const & adapter,
			std::string const & name, RenderSettings const & settings)
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
						: hWnd_(nullptr),
#else
						: metro_d3d_render_win_(ref new MetroD3D12RenderWindow),
#endif
							adapter_(adapter),
							gi_factory_(gi_factory),
							render_fence_value_(0), compute_fence_value_(0), copy_fence_value_(0)
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
		metro_d3d_render_win_->BindD3D12RenderWindow(this);
#endif
		on_paint_connect_ = main_wnd->OnPaint().connect(std::bind(&D3D12RenderWindow::OnPaint, this,
			std::placeholders::_1));
		on_exit_size_move_connect_ = main_wnd->OnExitSizeMove().connect(std::bind(&D3D12RenderWindow::OnExitSizeMove, this,
			std::placeholders::_1));
		on_size_connect_ = main_wnd->OnSize().connect(std::bind(&D3D12RenderWindow::OnSize, this,
			std::placeholders::_1, std::placeholders::_2));
		on_set_cursor_connect_ = main_wnd->OnSetCursor().connect(std::bind(&D3D12RenderWindow::OnSetCursor, this,
			std::placeholders::_1));

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

			std::vector<std::tuple<char const *, D3D_FEATURE_LEVEL, bool>> available_feature_levels;
			available_feature_levels.push_back(std::make_tuple("12_1", D3D_FEATURE_LEVEL_12_1, true));
			available_feature_levels.push_back(std::make_tuple("12_0", D3D_FEATURE_LEVEL_12_0, true));
			available_feature_levels.push_back(std::make_tuple("11_1", D3D_FEATURE_LEVEL_11_1, true));
			available_feature_levels.push_back(std::make_tuple("11_0", D3D_FEATURE_LEVEL_11_0, true));
			available_feature_levels.push_back(std::make_tuple("10_1", D3D_FEATURE_LEVEL_10_1, false));
			available_feature_levels.push_back(std::make_tuple("10_0", D3D_FEATURE_LEVEL_10_0, false));
			available_feature_levels.push_back(std::make_tuple("9_3", D3D_FEATURE_LEVEL_9_3, false));
			available_feature_levels.push_back(std::make_tuple("9_2", D3D_FEATURE_LEVEL_9_2, false));
			available_feature_levels.push_back(std::make_tuple("9_1", D3D_FEATURE_LEVEL_9_1, false));

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
						if (0 == strcmp(std::get<0>(available_feature_levels[i]), opt_val.c_str()))
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

			std::vector<D3D_FEATURE_LEVEL> feature_levels_for_12;
			for (size_t i = 0; i < available_feature_levels.size(); ++i)
			{
				if (std::get<2>(available_feature_levels[i]))
				{
					feature_levels_for_12.push_back(std::get<1>(available_feature_levels[i]));
				}
			}

			std::vector<D3D_FEATURE_LEVEL> feature_levels_for_11;
			for (size_t i = 0; i < available_feature_levels.size(); ++ i)
			{
				feature_levels_for_11.push_back(std::get<1>(available_feature_levels[i]));
			}

			for (size_t i = 0; i < feature_levels_for_12.size(); ++ i)
			{
				ID3D12Device* device = nullptr;
				if (SUCCEEDED(D3D12InterfaceLoader::Instance().D3D12CreateDevice(adapter_->DXGIAdapter().get(),
						feature_levels_for_12[i], IID_ID3D12Device, reinterpret_cast<void**>(&device))))
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

					D3D12_FEATURE_DATA_FEATURE_LEVELS feature_levels;
					feature_levels.NumFeatureLevels = static_cast<UINT>(feature_levels_for_11.size());
					feature_levels.pFeatureLevelsRequested = &feature_levels_for_11[0];
					d3d_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_levels, sizeof(feature_levels));

					d3d12_re.D3DDevice(d3d_device, d3d_cmd_queue, feature_levels.MaxSupportedFeatureLevel);

					if (Context::Instance().AppInstance().ConfirmDevice())
					{
						description_ = adapter_->Description() + L" FL ";
						wchar_t const * fl_str;
						switch (feature_levels.MaxSupportedFeatureLevel)
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
		using namespace Windows::Graphics::Display;
		using namespace Windows::Foundation;
		stereo_enabled_changed_token_ = DisplayInformation::GetForCurrentView()->StereoEnabledChanged +=
			ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(metro_d3d_render_win_, &MetroD3D12RenderWindow::OnStereoEnabledChanged);
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

		sc_fs_desc_.RefreshRate.Numerator = 60;
		sc_fs_desc_.RefreshRate.Denominator = 1;
		sc_fs_desc_.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sc_fs_desc_.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sc_fs_desc_.Windowed = !this->FullScreen();

		IDXGISwapChain1* sc = nullptr;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		gi_factory_->CreateSwapChainForHwnd(d3d_cmd_queue.get(), hWnd_,
			&sc_desc1_, &sc_fs_desc_, nullptr, &sc);
#else
		gi_factory_->CreateSwapChainForCoreWindow(d3d_12_cmd_queue.get(),
			reinterpret_cast<IUnknown*>(wnd_.Get()), &sc_desc1_, nullptr, &sc);
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
		
		ID3D12Fence* fence;
		TIF(d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence, reinterpret_cast<void**>(&fence)));
		render_fence_ = MakeCOMPtr(fence);
		render_fence_value_ = 1;

		render_fence_event_ = ::CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		TIF(d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence, reinterpret_cast<void**>(&fence)));
		compute_fence_ = MakeCOMPtr(fence);
		compute_fence_value_ = 1;

		compute_fence_event_ = ::CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		TIF(d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_ID3D12Fence, reinterpret_cast<void**>(&fence)));
		copy_fence_ = MakeCOMPtr(fence);
		copy_fence_value_ = 1;

		copy_fence_event_ = ::CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		this->UpdateSurfacesPtrs();

		this->WaitForGPU();
	}

	D3D12RenderWindow::~D3D12RenderWindow()
	{
		on_paint_connect_.disconnect();
		on_exit_size_move_connect_.disconnect();
		on_size_connect_.disconnect();
		on_set_cursor_connect_.disconnect();

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
				reinterpret_cast<IUnknown*>(wnd_.Get()), &sc_desc1_, nullptr, &sc);
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
		UNREF_PARAM(fs);
#endif
	}

	void D3D12RenderWindow::WindowMovedOrResized()
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
		using namespace Windows::Graphics::Display;
		DisplayInformation::GetForCurrentView()->StereoEnabledChanged -= stereo_enabled_changed_token_;
#endif

		for (size_t i = 0; i < render_targets_.size(); ++ i)
		{
			render_target_render_views_right_eye_[i].reset();
			render_target_render_views_[i].reset();
			render_targets_[i].reset();
		}

		render_fence_.reset();
		::CloseHandle(render_fence_event_);

		compute_fence_.reset();
		::CloseHandle(compute_fence_event_);

		copy_fence_.reset();
		::CloseHandle(copy_fence_event_);

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

	void D3D12RenderWindow::OnPaint(Window const & win)
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

	void D3D12RenderWindow::OnSetCursor(Window const & /*win*/)
	{
	}

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
	void D3D12RenderWindow::MetroD3D12RenderWindow::OnStereoEnabledChanged(
		Windows::Graphics::Display::DisplayInformation^ /*sender*/, Platform::Object^ /*args*/)
	{
		if ((win_->gi_factory_2_->IsWindowedStereoEnabled() ? true : false) != win_->dxgi_stereo_support_)
		{
			win_->swap_chain_.reset();
			win_->WindowMovedOrResized();
		}
	}

	void D3D12RenderWindow::MetroD3D12RenderWindow::BindD3D12RenderWindow(D3D12RenderWindow* win)
	{
		win_ = win;
	}
#endif

	void D3D12RenderWindow::WaitForGPU()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&rf.RenderEngineInstance());

		ID3D12CommandQueuePtr const & render_cmd_queue = re.D3DRenderCmdQueue();
		uint64_t const render_fence = render_fence_value_;
		TIF(render_cmd_queue->Signal(render_fence_.get(), render_fence));
		++ render_fence_value_;

		if (render_fence_->GetCompletedValue() < render_fence)
		{
			TIF(render_fence_->SetEventOnCompletion(render_fence, render_fence_event_));
			::WaitForSingleObjectEx(render_fence_event_, INFINITE, FALSE);
		}

		ID3D12CommandQueuePtr const & compute_cmd_queue = re.D3DComputeCmdQueue();
		uint64_t const compute_fence = compute_fence_value_;
		TIF(compute_cmd_queue->Signal(compute_fence_.get(), compute_fence));
		++ compute_fence_value_;

		if (compute_fence_->GetCompletedValue() < compute_fence)
		{
			TIF(compute_fence_->SetEventOnCompletion(compute_fence, compute_fence_event_));
			::WaitForSingleObjectEx(compute_fence_event_, INFINITE, FALSE);
		}

		ID3D12CommandQueuePtr const & copy_cmd_queue = re.D3DCopyCmdQueue();
		uint64_t const copy_fence = copy_fence_value_;
		TIF(copy_cmd_queue->Signal(copy_fence_.get(), copy_fence));
		++ copy_fence_value_;

		if (copy_fence_->GetCompletedValue() < copy_fence)
		{
			TIF(copy_fence_->SetEventOnCompletion(copy_fence, copy_fence_event_));
			::WaitForSingleObjectEx(copy_fence_event_, INFINITE, FALSE);
		}
	}
}
