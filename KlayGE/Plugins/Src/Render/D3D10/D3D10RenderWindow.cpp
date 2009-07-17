// D3D10RenderWindow.cpp
// KlayGE D3D10渲染窗口类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Math.hpp>
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
#include <boost/tuple/tuple.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>

#include <KlayGE/D3D10/D3D10RenderEngine.hpp>
#include <KlayGE/D3D10/D3D10Mapping.hpp>
#include <KlayGE/D3D10/D3D10RenderFactory.hpp>
#include <KlayGE/D3D10/D3D10RenderFactoryInternal.hpp>
#include <KlayGE/D3D10/D3D10RenderView.hpp>
#include <KlayGE/D3D10/D3D10RenderWindow.hpp>

namespace KlayGE
{
	D3D10RenderWindow::D3D10RenderWindow(IDXGIFactoryPtr const & gi_factory, D3D10AdapterPtr const & adapter,
			std::string const & name, RenderSettings const & settings)
						: hWnd_(NULL),
                            ready_(false), closed_(false),
                            adapter_(adapter),
                            gi_factory_(gi_factory)
	{
		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isDepthBuffered_	= IsDepthFormat(settings.depth_stencil_fmt);
		depthBits_			= NumDepthBits(settings.depth_stencil_fmt);
		stencilBits_		= NumStencilBits(settings.depth_stencil_fmt);
		format_				= settings.color_fmt;
		isFullScreen_		= settings.full_screen;

		// Destroy current window if any
		if (hWnd_ != NULL)
		{
			this->Destroy();
		}

		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		hWnd_ = main_wnd->HWnd();
		main_wnd->OnActive().connect(boost::bind(&D3D10RenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().connect(boost::bind(&D3D10RenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().connect(boost::bind(&D3D10RenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().connect(boost::bind(&D3D10RenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().connect(boost::bind(&D3D10RenderWindow::OnSize, this, _1, _2));
		main_wnd->OnSetCursor().connect(boost::bind(&D3D10RenderWindow::OnSetCursor, this, _1));
		main_wnd->OnClose().connect(boost::bind(&D3D10RenderWindow::OnClose, this, _1));

		fs_color_depth_ = NumFormatBits(settings.color_fmt);

		if (this->FullScreen())
		{
			colorDepth_ = fs_color_depth_;
			left_ = 0;
			top_ = 0;
		}
		else
		{
			// Get colour depth from display
			HDC hdc(::GetDC(hWnd_));
			colorDepth_ = ::GetDeviceCaps(hdc, BITSPIXEL);
			::ReleaseDC(hWnd_, hdc);
			top_ = settings.top;
			left_ = settings.left;
		}

		if (this->FullScreen())
		{
			back_buffer_format_ = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		else
		{
			back_buffer_format_ = adapter_->DesktopFormat();
		}

		std::memset(&sc_desc_, 0, sizeof(sc_desc_));
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
		sc_desc_.SampleDesc.Count = std::min(static_cast<uint32_t>(D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
		sc_desc_.SampleDesc.Quality = settings.sample_quality;
		sc_desc_.Windowed = !this->FullScreen();
		sc_desc_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sc_desc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		viewport_.left		= 0;
		viewport_.top		= 0;
		viewport_.width		= width_;
		viewport_.height	= height_;


		D3D10RenderEngine& re(*checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		if (re.D3DDevice())
		{
			d3d_device_ = re.D3DDevice();

			IDXGISwapChain* sc = NULL;
			gi_factory_->CreateSwapChain(d3d_device_.get(), &sc_desc_, &sc);
			swap_chain_ = MakeCOMPtr(sc);

			main_wnd_ = false;
		}
		else
		{
			UINT create_device_flags = D3D10_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
			create_device_flags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

			bool use_nvperfhud = (adapter_->Description() == L"NVIDIA PerfHUD");

			std::vector<boost::tuple<D3D10_DRIVER_TYPE, std::wstring> > dev_type_behaviors;
			if (use_nvperfhud)
			{
				dev_type_behaviors.push_back(boost::make_tuple(D3D10_DRIVER_TYPE_REFERENCE, std::wstring(L"PerfHUD")));
			}
			else
			{
				dev_type_behaviors.push_back(boost::make_tuple(D3D10_DRIVER_TYPE_HARDWARE, std::wstring(L"HW")));
				dev_type_behaviors.push_back(boost::make_tuple(D3D10_DRIVER_TYPE_SOFTWARE, std::wstring(L"SW")));
				dev_type_behaviors.push_back(boost::make_tuple(D3D10_DRIVER_TYPE_REFERENCE, std::wstring(L"REF")));
			}

			BOOST_FOREACH(BOOST_TYPEOF(dev_type_behaviors)::reference dev_type_beh, dev_type_behaviors)
			{
				IDXGISwapChain* sc = NULL;
				ID3D10Device* d3d_device = NULL;
				IDXGIAdapter* dx_adapter = NULL;
				if ((D3D10_DRIVER_TYPE_HARDWARE == boost::get<0>(dev_type_beh)) || use_nvperfhud)
				{
					dx_adapter = adapter_->Adapter().get();
				}
				HMODULE warp = NULL;
				if (D3D10_DRIVER_TYPE_SOFTWARE == boost::get<0>(dev_type_beh))
				{
					warp = LoadLibraryA("D3D10WARP_beta.dll");
				}
				if (SUCCEEDED(re.D3D10CreateDeviceAndSwapChain(dx_adapter, boost::get<0>(dev_type_beh), warp, create_device_flags,
					D3D10_SDK_VERSION, &sc_desc_, &sc, &d3d_device)))
				{
					swap_chain_ = MakeCOMPtr(sc);

					d3d_device_ = MakeCOMPtr(d3d_device);
					re.D3DDevice(d3d_device_);

					if (settings.ConfirmDevice && !settings.ConfirmDevice())
					{
						swap_chain_.reset();
						d3d_device_.reset();
					}
					else
					{
						if (boost::get<0>(dev_type_beh) != D3D10_DRIVER_TYPE_HARDWARE)
						{
							IDXGIDevice* dxgi_device = NULL;
							HRESULT hr = d3d_device_->QueryInterface(IID_IDXGIDevice, reinterpret_cast<void**>(&dxgi_device));
							if (SUCCEEDED(hr) && (dxgi_device != NULL))
							{
								IDXGIAdapter* ada;
								dxgi_device->GetAdapter(&ada);
								adapter_->ResetAdapter(MakeCOMPtr(ada));
							}
							dxgi_device->Release();
						}
						adapter_->Enumerate();

						std::wostringstream oss;
						oss << adapter_->Description() << L" " << boost::get<1>(dev_type_beh);
						if (settings.sample_count > 1)
						{
							oss << L" (" << settings.sample_count << "x AA)";
						}
						description_ = oss.str();
						break;
					}
				}
			}

			Verify(swap_chain_);
			Verify(d3d_device_);

			main_wnd_ = true;
		}

		// Depth-stencil format
		if (isDepthBuffered_)
		{
			BOOST_ASSERT((32 == depthBits_) || (24 == depthBits_) || (16 == depthBits_) || (0 == depthBits_));
			BOOST_ASSERT((8 == stencilBits_) || (0 == stencilBits_));

			UINT format_support;

			if (32 == depthBits_)
			{
				BOOST_ASSERT(0 == stencilBits_);

				// Try 32-bit zbuffer
				d3d_device_->CheckFormatSupport(DXGI_FORMAT_D32_FLOAT, &format_support);
				if (format_support & D3D10_FORMAT_SUPPORT_DEPTH_STENCIL)
				{
					depth_stencil_format_ = DXGI_FORMAT_D32_FLOAT;
					stencilBits_ = 0;
				}
				else
				{
					depthBits_ = 24;
				}
			}
			if (24 == depthBits_)
			{
				d3d_device_->CheckFormatSupport(DXGI_FORMAT_D24_UNORM_S8_UINT, &format_support);
				if (format_support & D3D10_FORMAT_SUPPORT_DEPTH_STENCIL)
				{
					depth_stencil_format_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
					stencilBits_ = 8;
				}
				else
				{
					depthBits_ = 16;
				}
			}
			if (16 == depthBits_)
			{
				d3d_device_->CheckFormatSupport(DXGI_FORMAT_D16_UNORM, &format_support);
				if (format_support & D3D10_FORMAT_SUPPORT_DEPTH_STENCIL)
				{
					depth_stencil_format_ = DXGI_FORMAT_D16_UNORM;
					stencilBits_ = 0;
				}
				else
				{
					depthBits_ = 0;
				}
			}
			if (0 == depthBits_)
			{
				isDepthBuffered_ = false;
				stencilBits_ = 0;
			}
		}
		else
		{
			depthBits_ = 0;
			stencilBits_ = 0;
		}

		gi_factory_->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
		swap_chain_->SetFullscreenState(this->FullScreen(), NULL);

		this->UpdateSurfacesPtrs();

		active_ = true;
		ready_ = true;
	}

	D3D10RenderWindow::~D3D10RenderWindow()
	{
		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		main_wnd->OnActive().disconnect(boost::bind(&D3D10RenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().disconnect(boost::bind(&D3D10RenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().disconnect(boost::bind(&D3D10RenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().disconnect(boost::bind(&D3D10RenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().disconnect(boost::bind(&D3D10RenderWindow::OnSize, this, _1, _2));
		main_wnd->OnSetCursor().disconnect(boost::bind(&D3D10RenderWindow::OnSetCursor, this, _1));
		main_wnd->OnClose().disconnect(boost::bind(&D3D10RenderWindow::OnClose, this, _1));

		this->Destroy();
	}

	bool D3D10RenderWindow::Closed() const
	{
		return closed_;
	}

	bool D3D10RenderWindow::Ready() const
	{
		return ready_;
	}

	void D3D10RenderWindow::Ready(bool ready)
	{
		ready_ = ready;
	}

	std::wstring const & D3D10RenderWindow::Description() const
	{
		return description_;
	}

	// 改变窗口大小
	/////////////////////////////////////////////////////////////////////////////////
	void D3D10RenderWindow::Resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_.width = width;
		viewport_.height = height;

		if (d3d_device_)
		{
			d3d_device_->ClearState();
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

		checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->ResetRenderStates();

		this->OnBind();
	}

	// 改变窗口位置
	/////////////////////////////////////////////////////////////////////////////////
	void D3D10RenderWindow::Reposition(uint32_t left, uint32_t top)
	{
		left_ = left;
		top_ = top;
	}

	// 获取是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	bool D3D10RenderWindow::FullScreen() const
	{
		return isFullScreen_;
	}

	// 设置是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	void D3D10RenderWindow::FullScreen(bool fs)
	{
		if (isFullScreen_ != fs)
		{
			left_ = 0;
			top_ = 0;

			sc_desc_.Windowed = !fs;

			uint32_t style;
			if (fs)
			{
				colorDepth_ = fs_color_depth_;
				back_buffer_format_ = DXGI_FORMAT_R8G8B8A8_UNORM;

				style = WS_POPUP;
			}
			else
			{
				// Get colour depth from display
				HDC hdc(::GetDC(hWnd_));
				colorDepth_ = ::GetDeviceCaps(hdc, BITSPIXEL);
				::ReleaseDC(hWnd_, hdc);

				back_buffer_format_	= adapter_->DesktopFormat();

				style = WS_OVERLAPPEDWINDOW;
			}

			::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);

			RECT rc = { 0, 0, width_, height_ };
			::AdjustWindowRect(&rc, style, false);
			width_ = rc.right - rc.left;
			height_ = rc.bottom - rc.top;
			::SetWindowPos(hWnd_, NULL, left_, top_, width_, height_, SWP_NOZORDER);

			isFullScreen_ = fs;

			swap_chain_->SetFullscreenState(isFullScreen_, NULL);
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
	}

	void D3D10RenderWindow::WindowMovedOrResized()
	{
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
		if ((new_width != width_) || (new_height != height_))
		{
			this->Resize(new_width, new_height);
		}
	}

	void D3D10RenderWindow::Destroy()
	{
		if (d3d_device_)
		{
			d3d_device_->ClearState();
		}

		if (swap_chain_)
		{
			swap_chain_->SetFullscreenState(false, NULL);
		}

		render_target_view_.reset();
		depth_stencil_view_.reset();
		back_buffer_.reset();
		depth_stencil_.reset();
		swap_chain_.reset();
		d3d_device_.reset();
		gi_factory_.reset();
	}

	void D3D10RenderWindow::UpdateSurfacesPtrs()
	{
		// Create a render target view
		ID3D10Texture2D* back_buffer;
		TIF(swap_chain_->GetBuffer(0, IID_ID3D10Texture2D, reinterpret_cast<void**>(&back_buffer)));
		back_buffer_ = MakeCOMPtr(back_buffer);
		D3D10_TEXTURE2D_DESC bb_desc;
		back_buffer_->GetDesc(&bb_desc);
		ID3D10RenderTargetView* render_target_view;
		TIF(d3d_device_->CreateRenderTargetView(back_buffer_.get(), NULL, &render_target_view));
		render_target_view_ = MakeCOMPtr(render_target_view);

		// Create depth stencil texture
		D3D10_TEXTURE2D_DESC ds_desc;
		ds_desc.Width = this->Width();
		ds_desc.Height = this->Height();
		ds_desc.MipLevels = 1;
		ds_desc.ArraySize = 1;
		ds_desc.Format = depth_stencil_format_;
		ds_desc.SampleDesc.Count = bb_desc.SampleDesc.Count;
		ds_desc.SampleDesc.Quality = bb_desc.SampleDesc.Quality;
		ds_desc.Usage = D3D10_USAGE_DEFAULT;
		ds_desc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		ds_desc.CPUAccessFlags = 0;
		ds_desc.MiscFlags = 0;
		ID3D10Texture2D* depth_stencil;
		TIF(d3d_device_->CreateTexture2D(&ds_desc, NULL, &depth_stencil));
		depth_stencil_ = MakeCOMPtr(depth_stencil);

		// Create the depth stencil view
		D3D10_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		dsv_desc.Format = depth_stencil_format_;
		if (bb_desc.SampleDesc.Count > 1)
		{
			dsv_desc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			dsv_desc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		}
		dsv_desc.Texture2D.MipSlice = 0;
		ID3D10DepthStencilView* depth_stencil_view;
		TIF(d3d_device_->CreateDepthStencilView(depth_stencil_.get(), &dsv_desc, &depth_stencil_view));
		depth_stencil_view_ = MakeCOMPtr(depth_stencil_view);

		this->Attach(ATT_Color0, MakeSharedPtr<D3D10RenderTargetRenderView>(render_target_view_,
			this->Width(), this->Height(), D3D10Mapping::MappingFormat(back_buffer_format_)));
		if (depth_stencil_view_)
		{
			this->Attach(ATT_DepthStencil, MakeSharedPtr<D3D10DepthStencilRenderView>(depth_stencil_view_,
				this->Width(), this->Height(), D3D10Mapping::MappingFormat(depth_stencil_format_)));
		}
	}

	void D3D10RenderWindow::SwapBuffers()
	{
		if (d3d_device_)
		{
			TIF(swap_chain_->Present(0, 0));
		}
	}

	void D3D10RenderWindow::OnActive(Window const & /*win*/, bool active)
	{
		active_ = active;
	}

	void D3D10RenderWindow::OnPaint(Window const & /*win*/)
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

	void D3D10RenderWindow::OnEnterSizeMove(Window const & /*win*/)
	{
		// Previent rendering while moving / sizing
		this->Ready(false);
	}

	void D3D10RenderWindow::OnExitSizeMove(Window const & /*win*/)
	{
		this->WindowMovedOrResized();
		this->Ready(true);
	}

	void D3D10RenderWindow::OnSize(Window const & /*win*/, bool active)
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

	void D3D10RenderWindow::OnSetCursor(Window const & /*win*/)
	{
	}

	void D3D10RenderWindow::OnClose(Window const & /*win*/)
	{
		this->Destroy();
		closed_ = true;
	}
}
