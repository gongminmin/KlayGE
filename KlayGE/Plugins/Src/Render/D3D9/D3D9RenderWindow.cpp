// D3D9RenderWindow.cpp
// KlayGE D3D9渲染窗口类 实现文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.1
// 修正了只能sw vp的bug (2005.2.15)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/CommFuncs.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>

#include <vector>
#include <cassert>
#include <d3d9.h>

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

	D3D9RenderWindow::D3D9RenderWindow(const COMPtr<IDirect3D9>& d3d,
										const D3D9Adapter& adapter, const String& name,
										const D3D9RenderWindowSettings& settings)
						: d3d_(d3d),
							adapter_(adapter),
							hWnd_(NULL),
							active_(false), ready_(false), closed_(false),
							multiSampleQuality_(settings.multiSampleQuality)
	{
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

		WString wname;
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

		SetWindowLong(hWnd_, 0, reinterpret_cast<long>(this));

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

		Engine::MemoryInstance().Zero(&d3dpp_, sizeof(d3dpp_));
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

		if (multiSampleQuality_ < 2) // NONE
		{
			multiSampleQuality_ = 0;
		}
		if (multiSampleQuality_ > 16) // MAX
		{
			multiSampleQuality_ = 16;
		}

		if ((multiSampleQuality_ != 0) && SUCCEEDED(d3d_->CheckDeviceMultiSampleType(this->adapter_.AdapterNo(), 
			D3DDEVTYPE_HAL, d3dpp_.BackBufferFormat, isFullScreen_,
			D3DMULTISAMPLE_NONMASKABLE, &multiSampleQuality_)))
		{
			d3dpp_.MultiSampleType		= D3DMULTISAMPLE_NONMASKABLE;
			d3dpp_.MultiSampleQuality	= multiSampleQuality_;
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

		typedef std::vector<std::pair<U32, WString> > BehaviorType;
		BehaviorType behavior;
		behavior.push_back(std::make_pair(D3DCREATE_HARDWARE_VERTEXPROCESSING, WString(L"(hw vp)")));
		behavior.push_back(std::make_pair(D3DCREATE_MIXED_VERTEXPROCESSING, WString(L"(mix vp)")));
		behavior.push_back(std::make_pair(D3DCREATE_SOFTWARE_VERTEXPROCESSING, WString(L"(sw vp)")));

		IDirect3DDevice9* d3dDevice(NULL);
		for (BehaviorType::iterator iter = behavior.begin(); iter != behavior.end(); ++ iter)
		{
			if (SUCCEEDED(d3d_->CreateDevice(adapter_.AdapterNo(), D3DDEVTYPE_HAL, hWnd_,
				iter->first, &d3dpp_, &d3dDevice)))
			{
				D3DCAPS9 caps;
				d3dDevice->GetDeviceCaps(&caps);
				if (settings.ConfirmDevice(caps, iter->first, d3dpp_.BackBufferFormat))
				{
					description_ += iter->second;
					break;
				}
				else
				{
					SafeRelease(d3dDevice);
				}
			}
		}

		Verify(d3dDevice != NULL);
		d3dDevice_ = COMPtr<IDirect3DDevice9>(d3dDevice);

		IDirect3DSurface9* renderSurface;
		d3dDevice_->GetRenderTarget(0, &renderSurface);
		renderSurface_ = COMPtr<IDirect3DSurface9>(renderSurface);

		IDirect3DSurface9* renderZBuffer;
		d3dDevice_->GetDepthStencilSurface(&renderZBuffer);
		renderZBuffer_ = COMPtr<IDirect3DSurface9>(renderZBuffer);

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

	const WString& D3D9RenderWindow::Description() const
	{
		return description_;
	}

	const D3D9Adapter& D3D9RenderWindow::Adapter() const
	{
		return adapter_;
	}

	const COMPtr<IDirect3DDevice9>& D3D9RenderWindow::D3DDevice() const
	{
		return d3dDevice_;
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
		if (hWnd_ != NULL)
		{
			::DestroyWindow(hWnd_);
			hWnd_ = NULL;
		}
	}

	void D3D9RenderWindow::Reposition(int left, int top)
	{
		viewport_.left = left;
		viewport_.top = top;
	}

	void D3D9RenderWindow::Resize(int width, int height)
	{
		width_ = width;
		height_ = height;

		// Notify viewports of resize
		viewport_.width = width;
		viewport_.height = height;

		D3DVIEWPORT9 d3dvp = { viewport_.left, viewport_.top, viewport_.width, viewport_.height, 0, 1 };
		TIF(d3dDevice_->SetViewport(&d3dvp));
	}

	void D3D9RenderWindow::SwapBuffers()
	{
		if (d3dDevice_.Get() != NULL)
		{
			d3dDevice_->Present(NULL, NULL, NULL, NULL);
		}
	}

	void D3D9RenderWindow::CustomAttribute(const String& name, void* pData)
	{
		// Valid attributes and their equvalent native functions:
		// D3DDEVICE			: D3DDevice
		// HWND					: WindowHandle

		if ("D3DDEVICE" == name)
		{
			IDirect3DDevice9** pDev = reinterpret_cast<IDirect3DDevice9**>(pData);
			*pDev = d3dDevice_.Get();

			return;
		}
		
		if ("HWND" == name)
		{
			HWND* phWnd = reinterpret_cast<HWND*>(pData);
			*phWnd = hWnd_;

			return;
		}
		
		if ("IsTexture" == name)
		{
			bool* b = reinterpret_cast<bool*>(pData);
			*b = false;

			return;
		}
		
		if ("D3DZBUFFER" == name)
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = renderZBuffer_.Get();

			return;
		}

		if ("DDBACKBUFFER" == name)
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = renderSurface_.Get();

			return;
		}

		if ("DDFRONTBUFFER" == name)
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = renderSurface_.Get();

			return;
		}
	}
}
