// D3D9RenderWindow.cpp
// KlayGE D3D9渲染窗口类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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
#include <cassert>
#include <cstring>

#include <d3d9.h>

#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/D3D9/D3D9RenderWindow.hpp>

namespace KlayGE
{
	LRESULT D3D9RenderWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		D3D9RenderWindow* win(reinterpret_cast<D3D9RenderWindow*>(::GetWindowLong(hWnd, 0)));
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

		case WM_CLOSE:
			this->Destroy();
			closed_ = true;
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	D3D9RenderWindow::D3D9RenderWindow(boost::shared_ptr<IDirect3D9> const & d3d,
										D3D9Adapter const & adapter, std::string const & name,
										RenderSettings const & settings)
						: d3d_(d3d),
							adapter_(adapter),
							hWnd_(NULL),
							active_(false), ready_(false), closed_(false)
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

		::SetWindowLong(hWnd_, 0, reinterpret_cast<long>(this));

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
		d3dpp_.EnableAutoDepthStencil	= isDepthBuffered_;
		d3dpp_.Flags					= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
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
		if (this->ColorDepth() > 16)
		{
			// Try to create a 32-bit depth, 8-bit stencil
			if (FAILED(d3d_->CheckDeviceFormat(adapter_.AdapterNo(),
				D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, D3DUSAGE_DEPTHSTENCIL, 
				D3DRTYPE_SURFACE, D3DFMT_D24S8)))
			{
				// Bugger, no 8-bit hardware stencil, just try 32-bit zbuffer 
				if (FAILED(d3d_->CheckDeviceFormat(adapter_.AdapterNo(),
					D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, D3DUSAGE_DEPTHSTENCIL, 
					D3DRTYPE_SURFACE, D3DFMT_D32)))
				{
					// Jeez, what a naff card. Fall back on 16-bit depth buffering
					d3dpp_.AutoDepthStencilFormat = D3DFMT_D16;
				}
				else
				{
					d3dpp_.AutoDepthStencilFormat = D3DFMT_D32;
				}
			}
			else
			{
				// Woohoo!
				if (SUCCEEDED(d3d_->CheckDepthStencilMatch(adapter_.AdapterNo(),
					D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, d3dpp_.BackBufferFormat, D3DFMT_D24X8)))
				{
					d3dpp_.AutoDepthStencilFormat = D3DFMT_D24X8; 
				}
				else
				{
					d3dpp_.AutoDepthStencilFormat = D3DFMT_D24S8;
				}
			}
		}
		else
		{
			// 16-bit depth, software stencil
			d3dpp_.AutoDepthStencilFormat = D3DFMT_D16;
		}

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

		viewport_.left		= left_;
		viewport_.top		= top_;
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
				D3DCREATE_MULTITHREADED | iter->first, &d3dpp_, &d3dDevice)))
			{
				D3DCAPS9 d3d_caps;
				d3dDevice->GetDeviceCaps(&d3d_caps);
				if (settings.ConfirmDevice(D3D9Mapping::Mapping(d3d_caps)))
				{
					description_ += iter->second;
					break;
				}
				else
				{
					d3dDevice->Release();
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

	bool D3D9RenderWindow::Active() const
	{
		return active_;
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

	boost::shared_ptr<IDirect3DDevice9> D3D9RenderWindow::D3DDevice() const
	{
		return d3dDevice_;
	}

	boost::shared_ptr<IDirect3DSurface9> D3D9RenderWindow::D3DRenderSurface() const
	{
		return renderSurface_;
	}

    boost::shared_ptr<IDirect3DSurface9> D3D9RenderWindow::D3DRenderZBuffer() const
	{
		return renderZBuffer_;
	}

	bool D3D9RenderWindow::RequiresTextureFlipping() const
	{
		return false;
	}

	void D3D9RenderWindow::WindowMovedOrResized()
	{
		::RECT rect;
		::GetWindowRect(hWnd_, &rect);

		this->Reposition(rect.left, rect.top);
		this->Resize(rect.right - rect.left, rect.bottom - rect.top);
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

	void D3D9RenderWindow::Reposition(int /*left*/, int /*top*/)
	{
	}

	void D3D9RenderWindow::Resize(int width, int height)
	{
		if ((width_ != width) || (height_ != height))
		{
			width_ = width;
			height_ = height;

			// Notify viewports of resize
			viewport_.width = width;
			viewport_.height = height;

			this->ResetDevice();
		}
	}

	void D3D9RenderWindow::UpdateSurfacesPtrs()
	{
		IDirect3DSurface9* renderSurface;
		d3dDevice_->GetRenderTarget(0, &renderSurface);
		renderSurface_ = MakeCOMPtr(renderSurface);

		IDirect3DSurface9* renderZBuffer;
		d3dDevice_->GetDepthStencilSurface(&renderZBuffer);
		renderZBuffer_ = MakeCOMPtr(renderZBuffer);
	}

	void D3D9RenderWindow::ResetDevice()
	{
		if (d3dDevice_)
		{
			D3D9RenderFactory& factory = static_cast<D3D9RenderFactory&>(Context::Instance().RenderFactoryInstance());
			factory.OnLostDevice();

			renderSurface_.reset();
			renderZBuffer_.reset();

			d3dpp_.BackBufferWidth  = this->Width();
			d3dpp_.BackBufferHeight = this->Height();
			TIF(d3dDevice_->Reset(&d3dpp_));

			this->UpdateSurfacesPtrs();

			factory.RenderEngineInstance().ActiveRenderTarget(factory.RenderEngineInstance().ActiveRenderTarget());

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

	void D3D9RenderWindow::CustomAttribute(std::string const & name, void* pData)
	{
		// Valid attributes and their equvalent native functions:
		// D3DZBUFFER			: Z buffer surface
		// DDBACKBUFFER			: Back buffer surface
		// DDFRONTBUFFER		: Front buffer surface

		if ("D3DZBUFFER" == name)
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = renderZBuffer_.get();

			return;
		}

		if ("DDBACKBUFFER" == name)
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = renderSurface_.get();

			return;
		}

		if ("DDFRONTBUFFER" == name)
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = renderSurface_.get();

			return;
		}

		assert(false);
	}
}
