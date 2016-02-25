/**
 * @file D3D12RenderWindow.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _D3D12RENDERWINDOW_HPP
#define _D3D12RENDERWINDOW_HPP

#pragma once

#include <KlayGE/D3D12/D3D12FrameBuffer.hpp>
#include <KlayGE/D3D12/D3D12Adapter.hpp>
#include <KlayGE/D3D12/D3D12RenderEngine.hpp>

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

namespace KlayGE
{
	struct RenderSettings;

	class D3D12RenderWindow : public D3D12FrameBuffer
	{
	public:
		D3D12RenderWindow(IDXGIFactory4Ptr const & gi_factory, D3D12AdapterPtr const & adapter,
			std::string const & name, RenderSettings const & settings);
		~D3D12RenderWindow();

		void Destroy();

		void SwapBuffers();

		std::wstring const & Description() const;

		void Resize(uint32_t width, uint32_t height);
		void Reposition(uint32_t left, uint32_t top);

		bool FullScreen() const;
		void FullScreen(bool fs);

		D3D12Adapter const & Adapter() const
		{
			return *adapter_;
		}

		TexturePtr const & D3DDepthStencilBuffer() const
		{
			return depth_stencil_;
		}
		RenderViewPtr const & D3DBackBufferRTV() const
		{
			return render_target_render_views_[curr_back_buffer_];
		}
		RenderViewPtr const & D3DBackBufferRightEyeRTV() const
		{
			return render_target_render_views_right_eye_[curr_back_buffer_];
		}

		uint32_t CurrBackBuffer() const
		{
			return curr_back_buffer_;
		}

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized();

	private:
		virtual void OnBind() override;
		void OnExitSizeMove(Window const & win);
		void OnSize(Window const & win, bool active);

#ifdef KLAYGE_PLATFORM_WINDOWS_RUNTIME
		HRESULT OnStereoEnabledChanged(ABI::Windows::Graphics::Display::IDisplayInformation* sender,
			IInspectable* args);
#endif

	private:
		void UpdateSurfacesPtrs();
		void ResetDevice();
		void WaitForGPU();

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

		D3D12AdapterPtr			adapter_;

		IDXGIFactory4Ptr gi_factory_;
		bool dxgi_stereo_support_;

		DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_desc_;
		DWORD stereo_cookie_;
#endif
		IDXGISwapChain3Ptr		swap_chain_;
		bool					main_wnd_;

		std::array<TexturePtr, NUM_BACK_BUFFERS> render_targets_;
		std::array<RenderViewPtr, NUM_BACK_BUFFERS> render_target_render_views_;
		std::array<RenderViewPtr, NUM_BACK_BUFFERS> render_target_render_views_right_eye_;

		TexturePtr depth_stencil_;

		uint32_t curr_back_buffer_;

		DXGI_FORMAT					back_buffer_format_;
		ElementFormat				depth_stencil_fmt_;

		std::wstring			description_;

		boost::signals2::connection on_exit_size_move_connect_;
		boost::signals2::connection on_size_connect_;
	};

	typedef std::shared_ptr<D3D12RenderWindow> D3D12RenderWindowPtr;
}

#endif			// _D3D12RENDERWINDOW_HPP
