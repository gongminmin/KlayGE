// D3D9RenderWindow.cpp
// KlayGE D3D9渲染窗口类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>

#define NOMINMAX
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
						: d3d_(d3d),
							adapter_(adapter),
							hWnd_(NULL),
							ready_(false), closed_(false)
	{
		if (settings.multiSample > 16)
		{
			multiSample_ = D3DMULTISAMPLE_16_SAMPLES;
		}
		else
		{
			multiSample_ = D3DMULTISAMPLE_TYPE(settings.multiSample);
		}

		// Store info
		name_				= name;
		width_				= settings.width;
		height_				= settings.height;
		isDepthBuffered_	= settings.depthBuffer;
		depthBits_			= settings.depthBits;
		stencilBits_		= settings.stencilBits;
		isFullScreen_		= settings.fullScreen;

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
		wc.cbWndExtra		= 4;
		wc.hInstance		= hInst;
		wc.hIcon			= NULL;
		wc.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= wname.c_str();
		::RegisterClassW(&wc);

		// Create our main window
		// Pass pointer to self
		hWnd_ = ::CreateWindowW(wname.c_str(), wname.c_str(),
			WS_OVERLAPPEDWINDOW, settings.left, settings.top,
			settings.width, settings.height, 0, 0, hInst, NULL);

		::SetWindowLongPtrW(hWnd_, 0, reinterpret_cast<LONG_PTR>(this));

		::ShowWindow(hWnd_, SW_SHOWNORMAL);
		::UpdateWindow(hWnd_);

		if (this->FullScreen())
		{
			colorDepth_ = settings.colorDepth;
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
		d3dpp_.Flags					= isDepthBuffered_ ? D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL : 0;
		d3dpp_.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;

		if (!this->FullScreen())
		{
			d3dpp_.BackBufferFormat	= adapter_.DesktopMode().Format;
		}
		else
		{
			if (this->ColorDepth() > 16)
			{
				d3dpp_.BackBufferFormat = D3DFMT_A8R8G8B8;
			}
			else
			{
				d3dpp_.BackBufferFormat = D3DFMT_A4R4G4B4;
			}
		}

		// Depth-stencil format
		if (isDepthBuffered_)
		{
			BOOST_ASSERT((32 == depthBits_) || (24 == depthBits_) || (16 == depthBits_) || (15 == depthBits_) || (0 == depthBits_));
			BOOST_ASSERT((8 == stencilBits_) || (1 == stencilBits_) || (0 == stencilBits_));

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
					depthBits_ = 15;
				}
			}
			if (15 == depthBits_)
			{
				if (SUCCEEDED(d3d_->CheckDepthStencilMatch(adapter_.AdapterNo(),
					D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, d3dpp_.BackBufferFormat, D3DFMT_D15S1)))
				{
					d3dpp_.AutoDepthStencilFormat = D3DFMT_D15S1;
					stencilBits_ = 1;
				}
				else
				{
					// Too bad
					isDepthBuffered_ = false;
					depthBits_ = 0;
					stencilBits_ = 0;
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
		d3dpp_.EnableAutoDepthStencil = isDepthBuffered_;


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

		typedef std::vector<std::pair<uint32_t, std::wstring> > BehaviorType;
		BehaviorType behavior;
		behavior.push_back(std::make_pair(D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, std::wstring(L"(pure hw vp)")));
		behavior.push_back(std::make_pair(D3DCREATE_HARDWARE_VERTEXPROCESSING, std::wstring(L"(hw vp)")));
		behavior.push_back(std::make_pair(D3DCREATE_MIXED_VERTEXPROCESSING, std::wstring(L"(mix vp)")));
		behavior.push_back(std::make_pair(D3DCREATE_SOFTWARE_VERTEXPROCESSING, std::wstring(L"(sw vp)")));

		IDirect3DDevice9* d3dDevice(NULL);
		for (BehaviorType::iterator iter = behavior.begin(); iter != behavior.end(); ++ iter)
		{
			if (SUCCEEDED(d3d_->CreateDevice(adapter_.AdapterNo(), D3DDEVTYPE_HAL, hWnd_,
				iter->first, &d3dpp_, &d3dDevice)))
			{
                // Check for ATI instancing support
			    D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			    if (D3D_OK == d3d_->CheckDeviceFormat(D3DADAPTER_DEFAULT,
				    D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
				    static_cast<D3DFORMAT>(MAKEFOURCC('I', 'N', 'S', 'T'))))
			    {
				    // Notify the driver that instancing support is expected
				    d3dDevice->SetRenderState(D3DRS_POINTSIZE, MAKEFOURCC('I', 'N', 'S', 'T'));
			    }

				D3DCAPS9 d3d_caps;
				d3dDevice->GetDeviceCaps(&d3d_caps);
				if (settings.ConfirmDevice && !settings.ConfirmDevice(D3D9Mapping::Mapping(d3d_caps)))
				{
					d3dDevice->Release();
				}
				else
				{
					description_ += iter->second;
					break;
				}
			}
		}

		Verify(d3dDevice != NULL);
        d3dDevice_ = MakeCOMPtr(d3dDevice);

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

	ID3D9SurfacePtr D3D9RenderWindow::D3DRenderSurface() const
	{
		return renderSurface_;
	}

	ID3D9SurfacePtr D3D9RenderWindow::D3DRenderZBuffer() const
	{
		return renderZBuffer_;
	}

	void D3D9RenderWindow::WindowMovedOrResized()
	{
		::RECT rect;
		::GetWindowRect(hWnd_, &rect);

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

	void D3D9RenderWindow::DoResize(uint32_t width, uint32_t height)
	{
		this->ResetDevice();
	}

	void D3D9RenderWindow::UpdateSurfacesPtrs()
	{
		IDirect3DSurface9* renderSurface;
		d3dDevice_->GetRenderTarget(0, &renderSurface);
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
			D3D9RenderFactory& factory = static_cast<D3D9RenderFactory&>(Context::Instance().RenderFactoryInstance());
			factory.OnLostDevice();

			D3DSURFACE_DESC desc;
			renderSurface_->GetDesc(&desc);
			IDirect3DSurface9* surface;
			d3dDevice_->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &surface, NULL);
			d3dDevice_->GetRenderTargetData(renderSurface_.get(), surface);
			renderSurface_ = MakeCOMPtr(surface);
			renderZBuffer_.reset();

			d3dpp_.BackBufferWidth  = this->Width();
			d3dpp_.BackBufferHeight = this->Height();
			TIF(d3dDevice_->Reset(&d3dpp_));

			d3dDevice_->GetRenderTarget(0, &surface);
			d3dDevice_->UpdateSurface(renderSurface_.get(), NULL, surface, NULL);
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
			d3dDevice_->Present(NULL, NULL, NULL, NULL);
		}
	}
}
