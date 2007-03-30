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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>

#include <d3d9.h>

#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/D3D9/D3D9RenderWindow.hpp>

namespace KlayGE
{
	LRESULT D3D9RenderWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		D3D9RenderWindow* win(reinterpret_cast<D3D9RenderWindow*>(::GetWindowLongPtrW(hWnd, 0)));
		if (win != NULL)
		{
			return win->MsgProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	LRESULT D3D9RenderWindow::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:
			if (WA_INACTIVE == LOWORD(wParam))
			{
				active_ = false;
			}
			else
			{
				active_ = true;
			}
			break;

		case WM_PAINT:
			// If we get WM_PAINT messges, it usually means our window was
			// comvered up, so we need to refresh it by re-showing the contents
			// of the current frame.
			if (this->Active() && this->Ready())
			{
				this->Update();
			}
			break;

		case WM_ENTERSIZEMOVE:
			// Previent rendering while moving / sizing
			this->Ready(false);
			break;

		case WM_EXITSIZEMOVE:
			this->WindowMovedOrResized();
			this->Ready(true);
			break;

		case WM_SIZE:
			// Check to see if we are losing or gaining our window.  Set the
			// active flag to match
			if ((SIZE_MAXHIDE == wParam) || (SIZE_MINIMIZED == wParam))
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
			break;

		case WM_GETMINMAXINFO:
			// Prevent the window from going smaller than some minimu size
			reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 100;
			reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 100;
			break;

		case WM_SETCURSOR:
		    d3dDevice_->ShowCursor(true);
			break;

		case WM_CLOSE:
			this->Destroy();
			closed_ = true;
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	D3D9RenderWindow::D3D9RenderWindow(ID3D9Ptr const & d3d,
										D3D9Adapter const & adapter, std::string const & name,
										RenderSettings const & settings)
						: hWnd_(NULL),
                            ready_(false), closed_(false),
                            adapter_(adapter),
                            d3d_(d3d)
	{
		if (settings.multi_sample > 16)
		{
			multiSample_ = D3DMULTISAMPLE_16_SAMPLES;
		}
		else
		{
			multiSample_ = static_cast<D3DMULTISAMPLE_TYPE>(settings.multi_sample);
		}

		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isDepthBuffered_	= IsDepthFormat(settings.depth_stencil_fmt);
		depthBits_			= NumDepthBits(settings.depth_stencil_fmt);
		stencilBits_		= NumStencilBits(settings.depth_stencil_fmt);
		format_				= settings.color_fmt;
		isFullScreen_		= settings.full_screen;

		HINSTANCE hInst(::GetModuleHandle(NULL));

		// Destroy current window if any
		if (hWnd_ != NULL)
		{
			this->Destroy();
		}

		std::wstring wname;
		Convert(wname, name);

		// Register the window class
		WNDCLASSW wc;
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= sizeof(this);
		wc.hInstance		= hInst;
		wc.hIcon			= NULL;
		wc.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= wname.c_str();
		::RegisterClassW(&wc);

		RECT rc = { 0, 0, width_, height_ };
		::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

		// Create our main window
		// Pass pointer to self
		hWnd_ = ::CreateWindowW(wname.c_str(), wname.c_str(),
			WS_OVERLAPPEDWINDOW, settings.left, settings.top,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, NULL);

		::SetWindowLongPtrW(hWnd_, 0, reinterpret_cast<LONG_PTR>(this));

		::ShowWindow(hWnd_, SW_SHOWNORMAL);
		::UpdateWindow(hWnd_);

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
					D3DRTYPE_SURFACE, D3DFMT_D32)))
				{
					d3dpp_.AutoDepthStencilFormat = D3DFMT_D32;
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


		if ((multiSample_ != 0) && SUCCEEDED(d3d_->CheckDeviceMultiSampleType(this->adapter_.AdapterNo(),
			D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, !isFullScreen_,
			multiSample_, NULL)))
		{
			d3dpp_.MultiSampleType		= multiSample_;
			d3dpp_.MultiSampleQuality	= 0;
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
		description_ += L' ';

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
			for (UINT adapter = 0; adapter < d3d->GetAdapterCount(); ++ adapter)
			{
				D3DADAPTER_IDENTIFIER9 identifier;
				d3d->GetAdapterIdentifier(adapter, 0, &identifier);
				if (strstr(identifier.Description, "NVPerfHUD") != 0)
				{
					use_nvperfhud = true;
					adapter_to_use = adapter;
					break;
				}
			}

			typedef std::vector<std::pair<uint32_t, std::wstring> > BehaviorType;
			BehaviorType behavior;

			typedef std::vector<std::pair<D3DDEVTYPE, std::wstring> > DevTypeType;
			DevTypeType dev_type;

			if (use_nvperfhud)
			{
				behavior.push_back(std::make_pair(D3DCREATE_HARDWARE_VERTEXPROCESSING, std::wstring(L"(hw vp)")));	
				dev_type.push_back(std::make_pair(D3DDEVTYPE_REF, std::wstring(L"REF")));
			}
			else
			{
				behavior.push_back(std::make_pair(D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, std::wstring(L"(pure hw vp)")));
				behavior.push_back(std::make_pair(D3DCREATE_HARDWARE_VERTEXPROCESSING, std::wstring(L"(hw vp)")));
				behavior.push_back(std::make_pair(D3DCREATE_MIXED_VERTEXPROCESSING, std::wstring(L"(mix vp)")));
				behavior.push_back(std::make_pair(D3DCREATE_SOFTWARE_VERTEXPROCESSING, std::wstring(L"(sw vp)")));

				dev_type.push_back(std::make_pair(D3DDEVTYPE_HAL, std::wstring(L"HAL")));
				dev_type.push_back(std::make_pair(D3DDEVTYPE_REF, std::wstring(L"REF")));
			}

			IDirect3DDevice9* d3d_device(NULL);
			for (DevTypeType::iterator dev_iter = dev_type.begin();
				(dev_iter != dev_type.end()) && (NULL == d3d_device); ++ dev_iter)
			{
				for (BehaviorType::iterator beh_iter = behavior.begin(); beh_iter != behavior.end(); ++ beh_iter)
				{
					if (SUCCEEDED(d3d_->CreateDevice(adapter_to_use, dev_iter->first,
						hWnd_, beh_iter->first, &d3dpp_, &d3d_device)))
					{
						// Check for ATI instancing support
						if (D3D_OK == d3d_->CheckDeviceFormat(D3DADAPTER_DEFAULT,
							D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
							static_cast<D3DFORMAT>(MakeFourCC<'I', 'N', 'S', 'T'>::value)))
						{
							// Notify the driver that instancing support is expected
							d3d_device->SetRenderState(D3DRS_POINTSIZE, MakeFourCC<'I', 'N', 'S', 'T'>::value);
						}

						D3DCAPS9 d3d_caps;
						d3d_device->GetDeviceCaps(&d3d_caps);
						if (settings.ConfirmDevice && !settings.ConfirmDevice(D3D9Mapping::Mapping(d3d_caps)))
						{
							d3d_device->Release();
							d3d_device = NULL;
						}
						else
						{
							description_ += dev_iter->second + L" " + beh_iter->second;
							break;
						}
					}
				}
			}

			Verify(d3d_device != NULL);
			d3dDevice_ = MakeCOMPtr(d3d_device);

			IDirect3DSwapChain9* sc = NULL;
			d3dDevice_->GetSwapChain(0, &sc);
			d3d_swap_chain_ = MakeCOMPtr(sc);

			if (d3dpp_.MultiSampleType != D3DMULTISAMPLE_NONE)
			{
				d3dDevice_->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, true);
			}

			main_wnd_ = true;
		}

		this->UpdateSurfacesPtrs();

		active_ = true;
		ready_ = true;
	}

	D3D9RenderWindow::~D3D9RenderWindow()
	{
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

	HWND D3D9RenderWindow::WindowHandle() const
	{
		return hWnd_;
	}

	std::wstring const & D3D9RenderWindow::Description() const
	{
		return description_;
	}

	D3D9Adapter const & D3D9RenderWindow::Adapter() const
	{
		return adapter_;
	}

	ID3D9DevicePtr D3D9RenderWindow::D3DDevice() const
	{
		return d3dDevice_;
	}

	ID3D9SurfacePtr D3D9RenderWindow::D3DRenderSurface(uint32_t n) const
	{
		if (0 == n)
		{
			return renderSurface_;
		}
		else
		{
			return ID3D9SurfacePtr();
		}
	}

	ID3D9SurfacePtr D3D9RenderWindow::D3DRenderZBuffer() const
	{
		return renderZBuffer_;
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
			this->Resize(new_width, new_height);
		}
	}

	void D3D9RenderWindow::Destroy()
	{
		renderZBuffer_.reset();
		renderSurface_.reset();
		d3dDevice_.reset();
		d3d_.reset();

		if (hWnd_ != NULL)
		{
			::DestroyWindow(hWnd_);
			hWnd_ = NULL;
		}
	}

	void D3D9RenderWindow::DoReposition(uint32_t /*left*/, uint32_t /*top*/)
	{
	}

	void D3D9RenderWindow::DoResize(uint32_t /*width*/, uint32_t /*height*/)
	{
	}

	void D3D9RenderWindow::DoFullScreen(bool fs)
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

	void D3D9RenderWindow::UpdateSurfacesPtrs()
	{
		IDirect3DSurface9* renderSurface;
		d3d_swap_chain_->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &renderSurface);
		renderSurface_ = MakeCOMPtr(renderSurface);

		if (isDepthBuffered_)
		{
			IDirect3DSurface9* renderZBuffer;
			d3dDevice_->GetDepthStencilSurface(&renderZBuffer);
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

			IDirect3DSurface9* surface;
			d3d_swap_chain_->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surface);
			renderSurface_ = MakeCOMPtr(surface);

			if (isDepthBuffered_)
			{
				d3dDevice_->GetDepthStencilSurface(&surface);
				renderZBuffer_ = MakeCOMPtr(surface);
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

	void D3D9RenderWindow::OnBind()
	{
		D3D9RenderEngine& re(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3dDevice = re.D3DDevice();

		for (uint32_t i = 0; i < re.DeviceCaps().max_simultaneous_rts; ++ i)
		{
			TIF(d3dDevice->SetRenderTarget(i, this->D3DRenderSurface(i).get()));
		}
		TIF(d3dDevice->SetDepthStencilSurface(this->D3DRenderZBuffer().get()));
	}
}
