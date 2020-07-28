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

#include <KFL/com_ptr.hpp>
#include <KFL/SmartPtrHelper.hpp>

#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/Signal.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS_STORE
#include <winrt/Windows.Graphics.Display.Core.h>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 5205) // winrt::impl::implements_delegate doesn't have virtual destructor
#endif
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace uwp
{
	using winrt::get_abi;

	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::Graphics::Display;
	using namespace winrt::Windows::UI::Core;
}
#endif

namespace KlayGE
{
	struct RenderSettings;
	class D3D11Adapter;

	class D3D11RenderWindow final : public D3D11FrameBuffer
	{
	public:
		D3D11RenderWindow(D3D11Adapter* adapter, std::string const& name, RenderSettings const& settings);
		~D3D11RenderWindow();

		void Destroy();

		void SwapBuffers() override;
		void WaitOnSwapBuffers() override;

		std::wstring const& Description() const override;

		void Resize(uint32_t width, uint32_t height);
		void Reposition(uint32_t left, uint32_t top);

		bool FullScreen() const;
		void FullScreen(bool fs);

		D3D11Adapter const& Adapter() const
		{
			return *adapter_;
		}
		TexturePtr const& D3DBackBuffer() const
		{
			return back_buffer_;
		}
		TexturePtr const& D3DDepthStencilBuffer() const
		{
			return depth_stencil_;
		}
		RenderTargetViewPtr const& D3DBackBufferRtv() const
		{
			return render_target_view_;
		}
		RenderTargetViewPtr const& D3DBackBufferRightEyeRtv() const
		{
			return render_target_view_right_eye_;
		}

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	private:
		void OnExitSizeMove(Window const& win);
		void OnSize(Window const& win, bool active);

#ifdef KLAYGE_PLATFORM_WINDOWS_STORE
		HRESULT OnStereoEnabledChanged(uwp::DisplayInformation const& sender, uwp::IInspectable const& args);
#endif

	private:
		void UpdateSurfacesPtrs();
		void CreateSwapChain(ID3D11Device* d3d_device, bool try_hdr_display);

	private:
		std::string name_;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND hWnd_; // Win32 Window handle
#else
		uwp::CoreWindow wnd_{nullptr};
		uwp::DisplayInformation::StereoEnabledChanged_revoker stereo_enabled_changed_token_;
#endif
		bool isFullScreen_;
		uint32_t sync_interval_;

		D3D11Adapter* adapter_;

		bool dxgi_stereo_support_{false};
		bool dxgi_allow_tearing_{false};
		bool dxgi_async_swap_chain_{false};

		DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_desc_;
		DWORD stereo_cookie_;
#endif
		IDXGISwapChain1Ptr swap_chain_1_;
		bool main_wnd_;
		Win32UniqueHandle frame_latency_waitable_obj_;

		TexturePtr back_buffer_;
		TexturePtr depth_stencil_;
		RenderTargetViewPtr render_target_view_;
		DepthStencilViewPtr depth_stencil_view_;
		RenderTargetViewPtr render_target_view_right_eye_;
		DepthStencilViewPtr depth_stencil_view_right_eye_;

		DXGI_FORMAT back_buffer_format_;
		ElementFormat depth_stencil_fmt_;

		std::wstring description_;

		Signal::Connection on_exit_size_move_connect_;
		Signal::Connection on_size_connect_;
	};

	typedef std::shared_ptr<D3D11RenderWindow> D3D11RenderWindowPtr;
} // namespace KlayGE

#endif // _D3D11RENDERWINDOW_HPP
