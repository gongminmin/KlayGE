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

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
#include <windows.ui.core.h>
#include <windows.graphics.display.h>
#endif

#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4512) // boost::iterators::function_output_iterator<T>::output_proxy doesn't have assignment operator
#pragma warning(disable: 4913) // User defined binary operator ',' exists but no overload could convert all operands
#elif defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/signals2.hpp>
#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(pop)
#elif defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

class IAmdDxExtQuadBufferStereo;
typedef std::shared_ptr<IAmdDxExtQuadBufferStereo> IAmdDxExtQuadBufferStereoPtr;

namespace KlayGE
{
	struct RenderSettings;

	class D3D11RenderWindow : public D3D11FrameBuffer
	{
	public:
		D3D11RenderWindow(D3D11AdapterPtr const & adapter, std::string const & name, RenderSettings const & settings);
		~D3D11RenderWindow();

		void Destroy();

		void SwapBuffers() override;
		void WaitOnSwapBuffers() override;

		std::wstring const & Description() const;

		void Resize(uint32_t width, uint32_t height);
		void Reposition(uint32_t left, uint32_t top);

		bool FullScreen() const;
		void FullScreen(bool fs);

		D3D11Adapter const & Adapter() const
		{
			return *adapter_;
		}
		TexturePtr const & D3DBackBuffer() const
		{
			return back_buffer_;
		}
		TexturePtr const & D3DDepthStencilBuffer() const
		{
			return depth_stencil_;
		}
		RenderViewPtr const & D3DBackBufferRTV() const
		{
			return render_target_view_;
		}
		RenderViewPtr const & D3DBackBufferRightEyeRTV() const
		{
			return render_target_view_right_eye_;
		}
		uint32_t StereoRightEyeHeight() const
		{
			return stereo_amd_right_eye_height_;
		}

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	private:
		void OnExitSizeMove(Window const & win);
		void OnSize(Window const & win, bool active);

#ifdef KLAYGE_PLATFORM_WINDOWS_RUNTIME
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		HRESULT OnStereoEnabledChanged(ABI::Windows::Graphics::Display::IDisplayInformation* sender,
			IInspectable* args);
#else
		HRESULT OnStereoEnabledChanged(IInspectable* sender);
#endif
#endif

	private:
		void UpdateSurfacesPtrs();
		void CreateSwapChain(ID3D11Device* d3d_device);

	private:
		std::string	name_;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND	hWnd_;				// Win32 Window handle
#else
		std::shared_ptr<ABI::Windows::UI::Core::ICoreWindow> wnd_;
		EventRegistrationToken stereo_enabled_changed_token_;
#endif
		bool	isFullScreen_;
		uint32_t sync_interval_;

		D3D11AdapterPtr			adapter_;
		bool dxgi_stereo_support_;
		bool dxgi_allow_tearing_;
		bool dxgi_async_swap_chain_;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		DXGI_SWAP_CHAIN_DESC sc_desc_;
		DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_desc_;
		DWORD stereo_cookie_;
#else
		DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
#endif
		IDXGISwapChainPtr		swap_chain_;
		IDXGISwapChain1Ptr		swap_chain_1_;
		bool					main_wnd_;
		HANDLE frame_latency_waitable_obj_;

		IAmdDxExtQuadBufferStereoPtr stereo_amd_qb_ext_;
		uint32_t stereo_amd_right_eye_height_;

		TexturePtr					back_buffer_;
		TexturePtr					depth_stencil_;
		RenderViewPtr				render_target_view_;
		RenderViewPtr				depth_stencil_view_;
		RenderViewPtr				render_target_view_right_eye_;
		RenderViewPtr				depth_stencil_view_right_eye_;

		DXGI_FORMAT					back_buffer_format_;
		ElementFormat				depth_stencil_fmt_;

		std::wstring			description_;

		boost::signals2::connection on_exit_size_move_connect_;
		boost::signals2::connection on_size_connect_;
	};

	typedef std::shared_ptr<D3D11RenderWindow> D3D11RenderWindowPtr;
}

#endif			// _D3D11RENDERWINDOW_HPP
