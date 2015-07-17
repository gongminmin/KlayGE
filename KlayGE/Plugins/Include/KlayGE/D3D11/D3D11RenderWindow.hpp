// D3D11RenderWindow.hpp
// KlayGE D3D11��Ⱦ������ ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERWINDOW_HPP
#define _D3D11RENDERWINDOW_HPP

#pragma once

#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11Adapter.hpp>

#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
#include <agile.h>
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
#if defined KLAYGE_PLATFORM_WINDOWS_RUNTIME
	ref class MetroD3D11RenderWindow;
#endif

	struct RenderSettings;

	class D3D11RenderWindow : public D3D11FrameBuffer
	{
	public:
		D3D11RenderWindow(IDXGIFactory1Ptr const & gi_factory, D3D11AdapterPtr const & adapter,
			std::string const & name, RenderSettings const & settings);
		~D3D11RenderWindow();

		void Destroy();

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
		ID3D11RenderTargetViewPtr const & D3DBackBufferRTV() const
		{
			return render_target_view_;
		}
		ID3D11DepthStencilViewPtr const & D3DDepthStencilBufferDSV() const
		{
			return depth_stencil_view_;
		}
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		ID3D11RenderTargetViewPtr const & D3DBackBufferRightEyeRTV() const
		{
			return render_target_view_right_eye_;
		}
#endif
		uint32_t StereoRightEyeHeight() const
		{
			return stereo_amd_right_eye_height_;
		}

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		ID3D11DepthStencilViewPtr const & D3DDepthStencilBufferRightEyeDSV() const
		{
			return depth_stencil_view_right_eye_;
		}
#endif

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	private:
		void OnPaint(Window const & win);
		void OnExitSizeMove(Window const & win);
		void OnSize(Window const & win, bool active);
		void OnSetCursor(Window const & win);

	private:
		void UpdateSurfacesPtrs();
		void ResetDevice();

	private:
		std::string	name_;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND	hWnd_;				// Win32 Window handle
#else
		Platform::Agile<Windows::UI::Core::CoreWindow> wnd_;

		ref class MetroD3D11RenderWindow
		{
			friend class D3D11RenderWindow;

		public:
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
			void OnStereoEnabledChanged(Windows::Graphics::Display::DisplayInformation^ sender,
				Platform::Object^ args);
#else
			void OnStereoEnabledChanged(Platform::Object^ sender);
#endif

		private:
			void BindD3D11RenderWindow(D3D11RenderWindow* win);

			D3D11RenderWindow* win_;
		};

		MetroD3D11RenderWindow^ metro_d3d_render_win_;
#endif
		bool	isFullScreen_;
		uint32_t sync_interval_;

		D3D11AdapterPtr			adapter_;

		uint8_t dxgi_sub_ver_;
		IDXGIFactory1Ptr gi_factory_;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		IDXGIFactory2Ptr gi_factory_2_;
		bool dxgi_stereo_support_;
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		IDXGIFactory3Ptr gi_factory_3_;
#endif

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		DXGI_SWAP_CHAIN_DESC sc_desc_;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_desc_;
		DWORD stereo_cookie_;
#endif
#else
		DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
#endif
		IDXGISwapChainPtr		swap_chain_;
		bool					main_wnd_;

		IAmdDxExtQuadBufferStereoPtr stereo_amd_qb_ext_;
		uint32_t stereo_amd_right_eye_height_;

		ID3D11Texture2DPtr			back_buffer_;
		ID3D11Texture2DPtr			depth_stencil_;
		ID3D11RenderTargetViewPtr	render_target_view_;
		ID3D11DepthStencilViewPtr	depth_stencil_view_;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		ID3D11RenderTargetViewPtr	render_target_view_right_eye_;
		ID3D11DepthStencilViewPtr	depth_stencil_view_right_eye_;
#endif

		DXGI_FORMAT					back_buffer_format_;
		DXGI_FORMAT					depth_stencil_format_;

		std::wstring			description_;

		boost::signals2::connection on_paint_connect_;
		boost::signals2::connection on_exit_size_move_connect_;
		boost::signals2::connection on_size_connect_;
		boost::signals2::connection on_set_cursor_connect_;
	};

	typedef std::shared_ptr<D3D11RenderWindow> D3D11RenderWindowPtr;
}

#endif			// _D3D11RENDERWINDOW_HPP
