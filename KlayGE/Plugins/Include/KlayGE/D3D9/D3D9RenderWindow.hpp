// D3D9RenderWindow.hpp
// KlayGE D3D9渲染窗口类 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2003-2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 支持动态切换全屏/窗口模式 (2007.3.24)
//
// 2.7.0
// 增加了ResetDevice (2005.7.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERWINDOW_HPP
#define _D3D9RENDERWINDOW_HPP

#pragma once

#include <d3d9.h>
#include <boost/smart_ptr.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9Adapter.hpp>

namespace KlayGE
{
	struct RenderSettings;

	class D3D9RenderWindow : public D3D9FrameBuffer
	{
	public:
		D3D9RenderWindow(ID3D9Ptr const & d3d, D3D9Adapter const & adapter,
			std::string const & name, RenderSettings const & settings);
		~D3D9RenderWindow();

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

		D3D9Adapter const & Adapter() const;
		ID3D9DevicePtr const & D3DDevice() const;
		ID3D9SurfacePtr D3DBackBuffer() const
		{
			return renderSurface_;
		}
		ID3D9SurfacePtr D3DDepthStencilBuffer() const
		{
			return renderZBuffer_;
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

		D3D9Adapter				adapter_;

		// Pointer to the 3D device specific for this window
		ID3D9Ptr				d3d_;
		ID3D9DevicePtr			d3dDevice_;
		D3DPRESENT_PARAMETERS	d3dpp_;
		ID3D9SwapChainPtr		d3d_swap_chain_;
		bool					main_wnd_;

		ID3D9SurfacePtr			renderSurface_;
		ID3D9SurfacePtr			renderZBuffer_;

		std::wstring			description_;

		uint32_t				fs_color_depth_;
	};

	typedef boost::shared_ptr<D3D9RenderWindow> D3D9RenderWindowPtr;
}

#endif			// _D3D9RENDERWINDOW_HPP
