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

#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11Adapter.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS_METRO
#include <agile.h>
#endif

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4100 4512 4913 6011)
#endif
#include <boost/signals2.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

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

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND	hWnd_;				// Win32 Window handle
#else
		Platform::Agile<Windows::UI::Core::CoreWindow> wnd_;
#endif
		bool	ready_;				// Is ready i.e. available for update
		bool	isFullScreen_;
		uint32_t sync_interval_;

		D3D11AdapterPtr			adapter_;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		DXGI_SWAP_CHAIN_DESC	sc_desc_;
#else
		DXGI_SWAP_CHAIN_DESC1	sc_desc_;
#endif
		IDXGISwapChainPtr		swap_chain_;
		bool					main_wnd_;

		ID3D11Texture2DPtr			back_buffer_;
		ID3D11Texture2DPtr			depth_stencil_;
		ID3D11RenderTargetViewPtr	render_target_view_;
		ID3D11DepthStencilViewPtr	depth_stencil_view_;

		DXGI_FORMAT					back_buffer_format_;
		DXGI_FORMAT					depth_stencil_format_;

		std::wstring			description_;

		boost::signals2::connection on_active_connect_;
		boost::signals2::connection on_paint_connect_;
		boost::signals2::connection on_enter_size_move_connect_;
		boost::signals2::connection on_exit_size_move_connect_;
		boost::signals2::connection on_size_connect_;
		boost::signals2::connection on_set_cursor_connect_;
		boost::signals2::connection on_close_connect_;
	};

	typedef shared_ptr<D3D11RenderWindow> D3D11RenderWindowPtr;
}

#endif			// _D3D11RENDERWINDOW_HPP
