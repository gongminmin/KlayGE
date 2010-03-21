// D3D9RenderWindow.cpp
// KlayGE D3D9渲染窗口类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2003-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 支持动态切换全屏/窗口模式 (2007.3.24)
//
// 3.5.0
// 支持NVPerfHUD (2007.1.31)
//
// 3.4.0
// 支持REF设备 (2006.9.20)
//
// 2.8.0
// 自动恢复BackBuffer (2005.7.31)
//
// 2.7.0
// 增加了ResetDevice (2005.7.1)
//
// 2.0.5
// 修正了WindowMovedOrResized的bug (2004.4.9)
//
// 2.0.1
// 修正了只能sw vp的bug (2004.2.15)
//
// 2.0.0
// 初次建立 (2003.8.15)
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

#include <d3d9.h>

#include <KlayGE/D3D9/D3D9Resource.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/D3D9/D3D9RenderFactoryInternal.hpp>
#include <KlayGE/D3D9/D3D9RenderWindow.hpp>

namespace KlayGE
{
	D3D9RenderWindow::D3D9RenderWindow(ID3D9Ptr const & d3d,
										D3D9Adapter const & adapter, std::string const & name,
										RenderSettings const & settings)
						: hWnd_(NULL),
                            ready_(false), closed_(false),
                            adapter_(adapter),
                            d3d_(d3d)
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
		main_wnd->OnActive().connect(boost::bind(&D3D9RenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().connect(boost::bind(&D3D9RenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().connect(boost::bind(&D3D9RenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().connect(boost::bind(&D3D9RenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().connect(boost::bind(&D3D9RenderWindow::OnSize, this, _1, _2));
		main_wnd->OnSetCursor().connect(boost::bind(&D3D9RenderWindow::OnSetCursor, this, _1));
		main_wnd->OnClose().connect(boost::bind(&D3D9RenderWindow::OnClose, this, _1));

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

		std::memset(&d3dpp_, 0, sizeof(d3dpp_));
		d3dpp_.Windowed					= !this->FullScreen();
		d3dpp_.BackBufferCount			= 1;
		d3dpp_.BackBufferWidth			= this->Width();
		d3dpp_.BackBufferHeight			= this->Height();
		d3dpp_.hDeviceWindow			= hWnd_;
		d3dpp_.SwapEffect				= D3DSWAPEFFECT_DISCARD;
		d3dpp_.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;

		if (this->FullScreen())
		{
			d3dpp_.BackBufferFormat = D3DFMT_X8R8G8B8;
		}
		else
		{
			d3dpp_.BackBufferFormat	= adapter_.DesktopMode().Format;
		}

		// Depth-stencil format
		if (isDepthBuffered_)
		{
			BOOST_ASSERT((32 == depthBits_) || (24 == depthBits_) || (16 == depthBits_) || (0 == depthBits_));
			BOOST_ASSERT((8 == stencilBits_) || (0 == stencilBits_));

			if (32 == depthBits_)
			{
				BOOST_ASSERT(0 == stencilBits_);

				// Try 32-bit zbuffer
				if (SUCCEEDED(d3d_->CheckDeviceFormat(adapter_.AdapterNo(),
					D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, D3DUSAGE_DEPTHSTENCIL,
					D3DRTYPE_SURFACE, D3DFMT_D32F_LOCKABLE)))
				{
					d3dpp_.AutoDepthStencilFormat = D3DFMT_D32F_LOCKABLE;
					stencilBits_ = 0;
				}
				else
				{
					depthBits_ = 24;
				}
			}
			if (24 == depthBits_)
			{
				if (0 == stencilBits_)
				{
					if (SUCCEEDED(d3d_->CheckDepthStencilMatch(adapter_.AdapterNo(),
						D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, d3dpp_.BackBufferFormat, D3DFMT_D24X8)))
					{
						d3dpp_.AutoDepthStencilFormat = D3DFMT_D24X8;
					}
					else
					{
						if (SUCCEEDED(d3d_->CheckDepthStencilMatch(adapter_.AdapterNo(),
							D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, d3dpp_.BackBufferFormat, D3DFMT_D24S8)))
						{
							d3dpp_.AutoDepthStencilFormat = D3DFMT_D24S8;
							stencilBits_ = 8;
						}
						else
						{
							depthBits_ = 16;
						}
					}
				}
				else
				{
					if (SUCCEEDED(d3d_->CheckDepthStencilMatch(adapter_.AdapterNo(),
							D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, d3dpp_.BackBufferFormat, D3DFMT_D24S8)))
					{
						d3dpp_.AutoDepthStencilFormat = D3DFMT_D24S8;
						stencilBits_ = 8;
					}
					else
					{
						depthBits_ = 16;
					}
				}
			}
			if (16 == depthBits_)
			{
				if (SUCCEEDED(d3d_->CheckDepthStencilMatch(adapter_.AdapterNo(),
					D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, d3dpp_.BackBufferFormat, D3DFMT_D16)))
				{
					d3dpp_.AutoDepthStencilFormat = D3DFMT_D16;
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
		d3dpp_.EnableAutoDepthStencil	= isDepthBuffered_;
		d3dpp_.Flags					= isDepthBuffered_ ? D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL : 0;


		int sample_count = std::min(static_cast<uint32_t>(16), settings.sample_count);
		if (sample_count > 1)
		{
			while ((sample_count > 1) && (FAILED(d3d_->CheckDeviceMultiSampleType(this->adapter_.AdapterNo(),
				D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, !isFullScreen_,
				static_cast<D3DMULTISAMPLE_TYPE>(sample_count), NULL))))
			{
				-- sample_count;
			}

			d3dpp_.MultiSampleType		= (sample_count > 1) ? static_cast<D3DMULTISAMPLE_TYPE>(sample_count) : D3DMULTISAMPLE_NONE;
			d3dpp_.MultiSampleQuality	= settings.sample_quality;
		}
		else
		{
			d3dpp_.MultiSampleType		= D3DMULTISAMPLE_NONE;
			d3dpp_.MultiSampleQuality	= 0;
		}

		viewport_.left		= 0;
		viewport_.top		= 0;
		viewport_.width		= width_;
		viewport_.height	= height_;


		Convert(description_, adapter_.Description());

		D3D9RenderEngine& re(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		if (re.D3DDevice())
		{
			d3dDevice_ = re.D3DDevice();

			IDirect3DSwapChain9* sc = NULL;
			d3dDevice_->CreateAdditionalSwapChain(&d3dpp_, &sc);
			d3d_swap_chain_ = MakeCOMPtr(sc);

			main_wnd_ = false;
		}
		else
		{
			UINT adapter_to_use = adapter_.AdapterNo();
			bool use_nvperfhud = false;
			for (UINT i = 0; i < d3d->GetAdapterCount(); ++ i)
			{
				D3DADAPTER_IDENTIFIER9 identifier;
				d3d->GetAdapterIdentifier(i, 0, &identifier);
				if (strstr(identifier.Description, "PerfHUD") != 0)
				{
					use_nvperfhud = true;
					adapter_to_use = i;
					break;
				}
			}

			std::vector<boost::tuple<D3DDEVTYPE, uint32_t, std::wstring> > dev_type_behaviors;
			if (use_nvperfhud)
			{
				dev_type_behaviors.push_back(boost::make_tuple(D3DDEVTYPE_REF, D3DCREATE_HARDWARE_VERTEXPROCESSING, std::wstring(L"REF (hw vp)")));
			}
			else
			{
				D3DCAPS9 caps;
				d3d->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);
				if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
				{
					if ((caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0)
					{
						dev_type_behaviors.push_back(boost::make_tuple(D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, std::wstring(L"HAL (pure hw vp)")));
					}
					else
					{
						dev_type_behaviors.push_back(boost::make_tuple(D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING, std::wstring(L"HAL (hw vp)")));
					}
				}
				else
				{
					dev_type_behaviors.push_back(boost::make_tuple(D3DDEVTYPE_HAL, D3DCREATE_MIXED_VERTEXPROCESSING, std::wstring(L"HAL (mix vp)")));
					dev_type_behaviors.push_back(boost::make_tuple(D3DDEVTYPE_HAL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, std::wstring(L"HAL (sw vp)")));
				}

				dev_type_behaviors.push_back(boost::make_tuple(D3DDEVTYPE_REF, D3DCREATE_SOFTWARE_VERTEXPROCESSING, std::wstring(L"REF (sw vp)")));
			}

			BOOST_FOREACH(BOOST_TYPEOF(dev_type_behaviors)::reference dev_type_beh, dev_type_behaviors)
			{
				IDirect3DDevice9* d3d_device = NULL;
				if (SUCCEEDED(d3d_->CreateDevice(adapter_to_use, boost::get<0>(dev_type_beh),
					hWnd_, boost::get<1>(dev_type_beh), &d3dpp_, &d3d_device)))
				{
					// Check for ATI instancing support
					if (D3D_OK == d3d_->CheckDeviceFormat(adapter_to_use,
						boost::get<0>(dev_type_beh), D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
						static_cast<D3DFORMAT>(MakeFourCC<'I', 'N', 'S', 'T'>::value)))
					{
						// Notify the driver that instancing support is expected
						d3d_device->SetRenderState(D3DRS_POINTSIZE, MakeFourCC<'I', 'N', 'S', 'T'>::value);
					}

					d3dDevice_ = MakeCOMPtr(d3d_device);
					re.D3DDevice(d3dDevice_);

					if (settings.ConfirmDevice && !settings.ConfirmDevice())
					{
						d3dDevice_.reset();
					}
					else
					{
						std::wstringstream oss;
						oss << description_ << L" " << boost::get<2>(dev_type_beh);
						if (sample_count > 1)
						{
							oss << L" (" << sample_count << L"x AA)";
						}
						description_ = oss.str();

						break;
					}
				}
			}

			Verify(d3dDevice_);

			IDirect3DSwapChain9* sc = NULL;
			d3dDevice_->GetSwapChain(0, &sc);
			d3d_swap_chain_ = MakeCOMPtr(sc);

			main_wnd_ = true;
		}

		this->UpdateSurfacesPtrs();

		active_ = true;
		ready_ = true;
	}

	D3D9RenderWindow::~D3D9RenderWindow()
	{
		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		main_wnd->OnActive().disconnect(boost::bind(&D3D9RenderWindow::OnActive, this, _1, _2));
		main_wnd->OnPaint().disconnect(boost::bind(&D3D9RenderWindow::OnPaint, this, _1));
		main_wnd->OnEnterSizeMove().disconnect(boost::bind(&D3D9RenderWindow::OnEnterSizeMove, this, _1));
		main_wnd->OnExitSizeMove().disconnect(boost::bind(&D3D9RenderWindow::OnExitSizeMove, this, _1));
		main_wnd->OnSize().disconnect(boost::bind(&D3D9RenderWindow::OnSize, this, _1, _2));
		main_wnd->OnSetCursor().disconnect(boost::bind(&D3D9RenderWindow::OnSetCursor, this, _1));
		main_wnd->OnClose().disconnect(boost::bind(&D3D9RenderWindow::OnClose, this, _1));

		this->Destroy();
	}

	bool D3D9RenderWindow::Closed() const
	{
		return closed_;
	}

	bool D3D9RenderWindow::Ready() const
	{
		return ready_;
	}

	void D3D9RenderWindow::Ready(bool ready)
	{
		ready_ = ready;
	}

	std::wstring const & D3D9RenderWindow::Description() const
	{
		return description_;
	}

	// 改变窗口大小
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderWindow::Resize(uint32_t width, uint32_t height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_.width = width;
		viewport_.height = height;

		this->ResetDevice();

		App3DFramework& app = Context::Instance().AppInstance();
		app.OnResize(width, height);
	}

	// 改变窗口位置
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderWindow::Reposition(uint32_t left, uint32_t top)
	{
		left_ = left;
		top_ = top;
	}

	// 获取是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	bool D3D9RenderWindow::FullScreen() const
	{
		return isFullScreen_;
	}

	// 设置是否是全屏状态
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderWindow::FullScreen(bool fs)
	{
		if (isFullScreen_ != fs)
		{
			left_ = 0;
			top_ = 0;

			d3dpp_.Windowed = !fs;

			uint32_t style;
			if (fs)
			{
				colorDepth_ = fs_color_depth_;
				d3dpp_.BackBufferFormat = D3DFMT_X8R8G8B8;

				style = WS_POPUP;
			}
			else
			{
				// Get colour depth from display
				HDC hdc(::GetDC(hWnd_));
				colorDepth_ = ::GetDeviceCaps(hdc, BITSPIXEL);
				::ReleaseDC(hWnd_, hdc);

				d3dpp_.BackBufferFormat	= adapter_.DesktopMode().Format;

				style = WS_OVERLAPPEDWINDOW;
			}

			::SetWindowLongPtrW(hWnd_, GWL_STYLE, style);

			RECT rc = { 0, 0, width_, height_ };
			::AdjustWindowRect(&rc, style, false);
			width_ = rc.right - rc.left;
			height_ = rc.bottom - rc.top;
			::SetWindowPos(hWnd_, NULL, left_, top_, width_, height_, SWP_NOZORDER);

			isFullScreen_ = fs;

			this->ResetDevice();

			::ShowWindow(hWnd_, SW_SHOWNORMAL);
			::UpdateWindow(hWnd_);
		}
	}

	D3D9Adapter const & D3D9RenderWindow::Adapter() const
	{
		return adapter_;
	}

	ID3D9DevicePtr const & D3D9RenderWindow::D3DDevice() const
	{
		return d3dDevice_;
	}

	void D3D9RenderWindow::WindowMovedOrResized()
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
			Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(new_width, new_height);
		}
	}

	void D3D9RenderWindow::Destroy()
	{
		renderZBuffer_.reset();
		renderSurface_.reset();
		d3d_swap_chain_.reset();
		d3dDevice_.reset();
		d3d_.reset();
	}

	void D3D9RenderWindow::UpdateSurfacesPtrs()
	{
		IDirect3DSurface9* renderSurface;
		d3d_swap_chain_->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &renderSurface);
		renderSurface_ = MakeCOMPtr(renderSurface);

		IDirect3DSurface9* renderZBuffer;
		d3dDevice_->GetDepthStencilSurface(&renderZBuffer);
		if (renderZBuffer != NULL)
		{
			renderZBuffer_ = MakeCOMPtr(renderZBuffer);
		}
	}

	void D3D9RenderWindow::ResetDevice()
	{
		if (d3dDevice_)
		{
			d3d_swap_chain_.reset();

			D3D9RenderFactory& factory = *checked_cast<D3D9RenderFactory*>(&Context::Instance().RenderFactoryInstance());
			factory.OnLostDevice();

			renderSurface_.reset();
			renderZBuffer_.reset();
			this->Detach(ATT_Color0);
			this->Detach(ATT_DepthStencil);

			HRESULT hr = d3dDevice_->TestCooperativeLevel();
			while ((hr != S_OK) && (hr != D3DERR_DEVICENOTRESET))
			{
				Sleep(10);
			}

			d3dpp_.BackBufferWidth  = this->Width();
			d3dpp_.BackBufferHeight = this->Height();
			TIF(d3dDevice_->Reset(&d3dpp_));

			IDirect3DSwapChain9* sc = NULL;
			if (main_wnd_)
			{
				d3dDevice_->GetSwapChain(0, &sc);
			}
			else
			{
				d3dDevice_->CreateAdditionalSwapChain(&d3dpp_, &sc);
			}
			d3d_swap_chain_ = MakeCOMPtr(sc);

			this->UpdateSurfacesPtrs();
			this->Attach(ATT_Color0, MakeSharedPtr<D3D9SurfaceRenderView>(renderSurface_));
			if (renderZBuffer_)
			{
				this->Attach(ATT_DepthStencil, MakeSharedPtr<D3D9SurfaceRenderView>(renderZBuffer_));
			}

			factory.OnResetDevice();
		}
	}

	void D3D9RenderWindow::SwapBuffers()
	{
		if (d3dDevice_)
		{
			if (D3DERR_DEVICELOST == d3d_swap_chain_->Present(NULL, NULL, hWnd_, NULL, 0))
			{
				this->ResetDevice();
			}
		}
	}

	void D3D9RenderWindow::OnActive(Window const & /*win*/, bool active)
	{
		active_ = active;
	}

	void D3D9RenderWindow::OnPaint(Window const & /*win*/)
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

	void D3D9RenderWindow::OnEnterSizeMove(Window const & /*win*/)
	{
		// Previent rendering while moving / sizing
		this->Ready(false);
	}

	void D3D9RenderWindow::OnExitSizeMove(Window const & /*win*/)
	{
		this->WindowMovedOrResized();
		this->Ready(true);
	}

	void D3D9RenderWindow::OnSize(Window const & /*win*/, bool active)
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

	void D3D9RenderWindow::OnSetCursor(Window const & /*win*/)
	{
		d3dDevice_->ShowCursor(true);
	}

	void D3D9RenderWindow::OnClose(Window const & /*win*/)
	{
		this->Destroy();
		closed_ = true;
	}
}
