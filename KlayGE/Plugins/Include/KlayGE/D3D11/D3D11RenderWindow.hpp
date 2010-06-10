// D3D11RenderWindow.hpp
// KlayGE D3D11渲染窗口类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERWINDOW_HPP
#define _D3D11RENDERWINDOW_HPP

#pragma once

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>
#include <boost/smart_ptr.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11Adapter.hpp>

namespace KlayGE
{
	struct RenderSettings;

	class D3D11RenderWindow : public D3D11FrameBuffer
	{
	public:
		D3D11RenderWindow(IDXGIFactory1Ptr const & gi_factory, D3D11AdapterPtr const & adapter,
			std::string const & name, RenderSettings const & settings);
		~D3D11RenderWindow();

		void Destroy();

		bool Closed() const;

		bool Ready() const;
		void Ready(bool ready);

		void SwapBuffers();

		std::wstring const & Description() const;

		void Resize(uint32_t width, uint32_t height);
		void Reposition(uint32_t left, uint32_t top);

		bool FullScreen() const;
		void FullScreen(bool fs);

		D3D11Adapter const & Adapter() const
		{
			return *adapter_;
		}
		ID3D11DevicePtr const & D3DDevice() const
		{
			return d3d_device_;
		}
		ID3D11Texture2DPtr const & D3DBackBuffer() const
		{
			return back_buffer_;
		}
		ID3D11Texture2DPtr const & D3DDepthStencilBuffer() const
		{
			return depth_stencil_;
		}

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

		bool RequiresFlipping() const
		{
			return false;
		}

	private:
		void OnActive(Window const & win, bool active);
		void OnPaint(Window const & win);
		void OnEnterSizeMove(Window const & win);
		void OnExitSizeMove(Window const & win);
		void OnSize(Window const & win, bool active);
		void OnSetCursor(Window const & win);
		void OnClose(Window const & win);

	private:
		void UpdateSurfacesPtrs();
		void ResetDevice();

	private:
		std::string	name_;

		HWND	hWnd_;				// Win32 Window handle
		bool	ready_;				// Is ready i.e. available for update
		bool	closed_;
		bool	isFullScreen_;

		D3D11AdapterPtr			adapter_;

		// Pointer to the 3D device specific for this window
		IDXGIFactory1Ptr		gi_factory_;
		ID3D11DevicePtr			d3d_device_;
		ID3D11DeviceContextPtr	d3d_imm_ctx_;
		DXGI_SWAP_CHAIN_DESC	sc_desc_;
		IDXGISwapChainPtr		swap_chain_;
		bool					main_wnd_;

		ID3D11Texture2DPtr			back_buffer_;
		ID3D11Texture2DPtr			depth_stencil_;
		ID3D11RenderTargetViewPtr	render_target_view_;
		ID3D11DepthStencilViewPtr	depth_stencil_view_;

		DXGI_FORMAT					back_buffer_format_;
		DXGI_FORMAT					depth_stencil_format_;

		std::wstring			description_;

		uint32_t				fs_color_depth_;
	};

	typedef boost::shared_ptr<D3D11RenderWindow> D3D11RenderWindowPtr;
}

#endif			// _D3D11RENDERWINDOW_HPP
