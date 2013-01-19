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
#include <sstream>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11RenderFactory.hpp>
#include <KlayGE/D3D11/D3D11RenderFactoryInternal.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11RenderWindow.hpp>

namespace KlayGE
{
	D3D11RenderWindow::D3D11RenderWindow(IDXGIFactory1Ptr const & gi_factory, D3D11AdapterPtr const & adapter,
			std::string const & name, RenderSettings const & settings)
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
						: hWnd_(nullptr),
#else
						:
#endif
                            ready_(false), closed_(false),
                            adapter_(adapter),
                            gi_factory_(gi_factory)
	{
		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isFullScreen_		= settings.full_screen;
		sync_interval_		= settings.sync_interval;

		ElementFormat format = settings.color_fmt;

		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		hWnd_ = main_wnd->HWnd();
#else
		wnd_ = main_wnd->GetWindow();
#endif
		main_wnd->OnActive().connect(boost::bind(&D3D11RenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().connect(boost::bind(&D3D11RenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().connect(boost::bind(&D3D11RenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().connect(boost::bind(&D3D11RenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().connect(boost::bind(&D3D11RenderWindow::OnSize, this, _1, _2));
		main_wnd->OnSetCursor().connect(boost::bind(&D3D11RenderWindow::OnSetCursor, this, _1));
		main_wnd->OnClose().connect(boost::bind(&D3D11RenderWindow::OnClose, this, _1));

		if (this->FullScreen())
		{
			left_ = 0;
			top_ = 0;
		}
		else
		{
			top_ = settings.top;
			left_ = settings.left;
		}

		back_buffer_format_ = D3D11Mapping::MappingFormat(format);

		std::memset(&sc_desc_, 0, sizeof(sc_desc_));
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		sc_desc_.BufferCount = 1;
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
#else
		sc_desc_.BufferCount = 2;
		sc_desc_.Width = this->Width();
		sc_desc_.Height = this->Height();
		sc_desc_.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sc_desc_.Stereo = false;
		sc_desc_.SampleDesc.Count = 1;
		sc_desc_.SampleDesc.Quality = 0;
		sc_desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sc_desc_.Scaling = DXGI_SCALING_NONE;
		sc_desc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		sc_desc_.Flags = 0;
#endif

		viewport_->left		= 0;
		viewport_->top		= 0;
		viewport_->width	= width_;
		viewport_->height	= height_;


		D3D11RenderEngine& re(*checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		if (re.D3DDevice())
		{
			d3d_device_ = re.D3DDevice();
			d3d_imm_ctx_ = re.D3DDeviceImmContext();

			main_wnd_ = false;
		}
		else
		{
			UINT create_device_flags = 0;
#ifdef _DEBUG
			create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			std::vector<tuple<D3D_DRIVER_TYPE, std::wstring> > dev_type_behaviors;
			dev_type_behaviors.push_back(KlayGE::make_tuple(D3D_DRIVER_TYPE_HARDWARE, std::wstring(L"HW")));
			dev_type_behaviors.push_back(KlayGE::make_tuple(D3D_DRIVER_TYPE_WARP, std::wstring(L"WARP")));
			dev_type_behaviors.push_back(KlayGE::make_tuple(D3D_DRIVER_TYPE_SOFTWARE, std::wstring(L"SW")));
			dev_type_behaviors.push_back(KlayGE::make_tuple(D3D_DRIVER_TYPE_REFERENCE, std::wstring(L"REF")));

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
			bool d3d_11_1 = false;
			IDXGIFactory2* gi_factory_2;
			gi_factory->QueryInterface(IID_IDXGIFactory2, reinterpret_cast<void**>(&gi_factory_2));
			if (gi_factory_2 != nullptr)
			{
				d3d_11_1 = true;
				gi_factory_2->Release();
			}
#endif

			std::vector<std::pair<std::string, D3D_FEATURE_LEVEL> > available_feature_levels;	
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
			if (d3d_11_1)
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
			boost::algorithm::split(strs, settings.options, boost::bind(std::equal_to<char>(), ',', _1));
			for (size_t index = 0; index < strs.size(); ++ index)
			{
				std::string& opt = strs[index];
				boost::algorithm::trim(opt);
				std::string::size_type loc = opt.find(':');
				std::string opt_name = opt.substr(0, loc);
				std::string opt_val = opt.substr(loc + 1);

				if ("level" == opt_name)
				{
					size_t feature_index = 0;
					for (size_t i = 0; i < available_feature_levels.size(); ++ i)
					{
						if (available_feature_levels[i].first == opt_val)
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
				ID3D11Device* d3d_device = nullptr;
				ID3D11DeviceContext* d3d_imm_ctx = nullptr;
				IDXGIAdapter* dx_adapter = nullptr;
				D3D_DRIVER_TYPE dev_type = get<0>(dev_type_beh);
				if (D3D_DRIVER_TYPE_HARDWARE == dev_type)
				{
					dx_adapter = adapter_->Adapter().get();
					dev_type = D3D_DRIVER_TYPE_UNKNOWN;
				}
				D3D_FEATURE_LEVEL out_feature_level;
				if (SUCCEEDED(re.D3D11CreateDevice(dx_adapter, dev_type, nullptr, create_device_flags,
					&feature_levels[0], static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION, &d3d_device,
					&out_feature_level, &d3d_imm_ctx)))
				{
					d3d_device_ = MakeCOMPtr(d3d_device);
					d3d_imm_ctx_ = MakeCOMPtr(d3d_imm_ctx);
					re.D3DDevice(d3d_device_, d3d_imm_ctx_, out_feature_level);

					if (!Context::Instance().AppInstance().ConfirmDevice())
					{
						d3d_device_.reset();
						d3d_imm_ctx_.reset();
					}
					else
					{
						if (dev_type != D3D_DRIVER_TYPE_HARDWARE)
						{
							IDXGIDevice1* dxgi_device = nullptr;
							HRESULT hr = d3d_device_->QueryInterface(IID_IDXGIDevice1, reinterpret_cast<void**>(&dxgi_device));
							if (SUCCEEDED(hr) && (dxgi_device != nullptr))
							{
								IDXGIAdapter* ada;
								dxgi_device->GetAdapter(&ada);
								adapter_->ResetAdapter(MakeCOMPtr(ada));
								adapter_->Enumerate();
							}
#ifdef KLAYGE_PLATFORM_WINDOWS_METRO
							dxgi_device->SetMaximumFrameLatency(1);
#endif
							dxgi_device->Release();
						}

						std::wostringstream oss;
						oss << adapter_->Description() << L" " << get<1>(dev_type_beh);
						switch (out_feature_level)
						{
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
						case D3D_FEATURE_LEVEL_11_1:
							oss << L" D3D Level 11.1";
							break;
#endif

						case D3D_FEATURE_LEVEL_11_0:
							oss << L" D3D Level 11";
							break;

						case D3D_FEATURE_LEVEL_10_1:
							oss << L" D3D Level 10.1";
							break;

						case D3D_FEATURE_LEVEL_10_0:
							oss << L" D3D Level 10";
							break;

						case D3D_FEATURE_LEVEL_9_3:
							oss << L" D3D Level 9.3";
							break;

						case D3D_FEATURE_LEVEL_9_2:
							oss << L" D3D Level 9.2";
							break;

						case D3D_FEATURE_LEVEL_9_1:
							oss << L" D3D Level 9.1";
							break;
						}
						if (settings.sample_count > 1)
						{
							oss << L" (" << settings.sample_count << "x AA)";
						}
						description_ = oss.str();
						break;
					}
				}
			}

			main_wnd_ = true;
		}

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		IDXGISwapChain* sc = nullptr;
		gi_factory_->CreateSwapChain(d3d_device_.get(), &sc_desc_, &sc);
#else
		IDXGISwapChain1* sc = nullptr;
		IDXGIFactory2* gi_factory_2;
		gi_factory->QueryInterface(IID_IDXGIFactory2, reinterpret_cast<void**>(&gi_factory_2));
		if (gi_factory_2 != nullptr)
		{
			gi_factory_2->CreateSwapChainForCoreWindow(d3d_device_.get(),
				reinterpret_cast<IUnknown*>(wnd_.Get()), &sc_desc_, nullptr, &sc);
			gi_factory_2->Release();
		}
#endif
		swap_chain_ = MakeCOMPtr(sc);

		Verify(swap_chain_);
		Verify(d3d_device_);
		Verify(d3d_imm_ctx_);

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
				d3d_device_->CheckFormatSupport(DXGI_FORMAT_D32_FLOAT, &format_support);
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
				d3d_device_->CheckFormatSupport(DXGI_FORMAT_D24_UNORM_S8_UINT, &format_support);
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
				d3d_device_->CheckFormatSupport(DXGI_FORMAT_D16_UNORM, &format_support);
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
		gi_factory_->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
		swap_chain_->SetFullscreenState(this->FullScreen(), nullptr);
#endif

		this->UpdateSurfacesPtrs();

		active_ = true;
		ready_ = true;
	}

	D3D11RenderWindow::~D3D11RenderWindow()
	{
		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		main_wnd->OnActive().disconnect(boost::bind(&D3D11RenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().disconnect(boost::bind(&D3D11RenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().disconnect(boost::bind(&D3D11RenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().disconnect(boost::bind(&D3D11RenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().disconnect(boost::bind(&D3D11RenderWindow::OnSize, this, _1, _2));
		main_wnd->OnSetCursor().disconnect(boost::bind(&D3D11RenderWindow::OnSetCursor, this, _1));
		main_wnd->OnClose().disconnect(boost::bind(&D3D11RenderWindow::OnClose, this, _1));

		this->Destroy();
	}

	bool D3D11RenderWindow::Closed() const
	{
		return closed_;
	}

	bool D3D11RenderWindow::Ready() const
	{
		return ready_;
	}

	void D3D11RenderWindow::Ready(bool ready)
	{
		ready_ = ready;
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

		if (d3d_imm_ctx_)
		{
			d3d_imm_ctx_->ClearState();
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

		swap_chain_->ResizeBuffers(1, width, height, back_buffer_format_, flags);

		this->UpdateSurfacesPtrs();

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);

		checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->ResetRenderStates();

		this->OnBind();
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

			sc_desc_.Windowed = !fs;

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

		uint32_t new_width = rect.right - rect.left;
		uint32_t new_height = rect.bottom - rect.top;
		if ((new_width != width_) || (new_height != height_))
		{
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(new_width, new_height);
		}
	}

	void D3D11RenderWindow::Destroy()
	{
		if (d3d_imm_ctx_)
		{
			d3d_imm_ctx_->ClearState();
		}

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (swap_chain_)
		{
			swap_chain_->SetFullscreenState(false, nullptr);
		}
#endif

		render_target_view_.reset();
		depth_stencil_view_.reset();
		back_buffer_.reset();
		depth_stencil_.reset();
		swap_chain_.reset();
		d3d_imm_ctx_.reset();
		d3d_device_.reset();
		gi_factory_.reset();
	}

	void D3D11RenderWindow::UpdateSurfacesPtrs()
	{
		// Create a render target view
		ID3D11Texture2D* back_buffer;
		TIF(swap_chain_->GetBuffer(0, IID_ID3D11Texture2D, reinterpret_cast<void**>(&back_buffer)));
		back_buffer_ = MakeCOMPtr(back_buffer);
		D3D11_TEXTURE2D_DESC bb_desc;
		back_buffer_->GetDesc(&bb_desc);
		ID3D11RenderTargetView* render_target_view;
		TIF(d3d_device_->CreateRenderTargetView(back_buffer_.get(), nullptr, &render_target_view));
		render_target_view_ = MakeCOMPtr(render_target_view);

		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC ds_desc;
		ds_desc.Width = this->Width();
		ds_desc.Height = this->Height();
		ds_desc.MipLevels = 1;
		ds_desc.ArraySize = 1;
		ds_desc.Format = depth_stencil_format_;
		ds_desc.SampleDesc.Count = bb_desc.SampleDesc.Count;
		ds_desc.SampleDesc.Quality = bb_desc.SampleDesc.Quality;
		ds_desc.Usage = D3D11_USAGE_DEFAULT;
		ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		ds_desc.CPUAccessFlags = 0;
		ds_desc.MiscFlags = 0;
		ID3D11Texture2D* depth_stencil;
		TIF(d3d_device_->CreateTexture2D(&ds_desc, nullptr, &depth_stencil));
		depth_stencil_ = MakeCOMPtr(depth_stencil);

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		dsv_desc.Format = depth_stencil_format_;
		if (bb_desc.SampleDesc.Count > 1)
		{
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}
		dsv_desc.Flags = 0;
		dsv_desc.Texture2D.MipSlice = 0;
		ID3D11DepthStencilView* depth_stencil_view;
		TIF(d3d_device_->CreateDepthStencilView(depth_stencil_.get(), &dsv_desc, &depth_stencil_view));
		depth_stencil_view_ = MakeCOMPtr(depth_stencil_view);

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
		if (d3d_device_)
		{
			TIF(swap_chain_->Present(sync_interval_, 0));
		}
	}

	void D3D11RenderWindow::OnActive(Window const & /*win*/, bool active)
	{
		active_ = active;
	}

	void D3D11RenderWindow::OnPaint(Window const & /*win*/)
	{
		// If we get WM_PAINT messges, it usually means our window was
		// comvered up, so we need to refresh it by re-showing the contents
		// of the current frame.
		if (this->Active() && this->Ready())
		{
			Context::Instance().SceneManagerInstance().Update();
			this->SwapBuffers();
		}
	}

	void D3D11RenderWindow::OnEnterSizeMove(Window const & /*win*/)
	{
		// Previent rendering while moving / sizing
		this->Ready(false);
	}

	void D3D11RenderWindow::OnExitSizeMove(Window const & /*win*/)
	{
		this->WindowMovedOrResized();
		this->Ready(true);
	}

	void D3D11RenderWindow::OnSize(Window const & /*win*/, bool active)
	{
		if (!active)
		{
			active_ = false;
		}
		else
		{
			active_ = true;
			if (this->Ready())
			{
				this->WindowMovedOrResized();
			}
		}
	}

	void D3D11RenderWindow::OnSetCursor(Window const & /*win*/)
	{
	}

	void D3D11RenderWindow::OnClose(Window const & /*win*/)
	{
		this->Destroy();
		closed_ = true;
	}
}
