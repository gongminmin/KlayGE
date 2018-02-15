/**
 * @file MFShow.hpp
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

#ifndef KLAYGE_PLUGINS_MF_SHOW_HPP
#define KLAYGE_PLUGINS_MF_SHOW_HPP

#pragma once

#include <KFL/Thread.hpp>

#include <windows.h>
#include <dxgiformat.h>
#include <d3dcommon.h>
#include <string>

#include <KlayGE/Show.hpp>

struct IDXGIAdapter;
struct IDXGIFactory1;
struct IDXGIOutput;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct IMFAttributes;
struct IMFDXGIDeviceManager;
struct IMFMediaEngine;

namespace KlayGE
{
	class MFShowEngine : public ShowEngine
	{
	public:
		MFShowEngine();
		~MFShowEngine() override;

		bool IsComplete() override;

		void Load(std::string const & file_name) override;
		TexturePtr PresentTexture() override;

		ShowState State(long msTimeout = -1) override;

		void OnMediaEngineEvent(DWORD me_event);

	private:
		void Init();
		void Free();

		void DoSuspend() override;
		void DoResume() override;

		void DoPlay() override;
		void DoStop() override;
		void DoPause() override;

		void StartTimer();
		void StopTimer();
		void RealVSyncTimer();
		void StartPlaying();

	private:
		typedef HRESULT (WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
		typedef HRESULT (WINAPI *D3D11CreateDeviceFunc)(IDXGIAdapter* pAdapter,
			D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
			D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
			ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
		typedef HRESULT (WINAPI *MFStartupFunc)(ULONG Version, DWORD dwFlags);
		typedef HRESULT (WINAPI *MFCreateDXGIDeviceManagerFunc)(UINT* resetToken, IMFDXGIDeviceManager** ppDeviceManager);
		typedef HRESULT (WINAPI *MFCreateAttributesFunc)(IMFAttributes** ppMFAttributes, UINT32 cInitialSize);
		typedef HRESULT (WINAPI *MFShutdownFunc)();

		CreateDXGIFactory1Func DynamicCreateDXGIFactory1_;
		D3D11CreateDeviceFunc DynamicD3D11CreateDevice_;
		MFStartupFunc DynamicMFStartup_;
		MFCreateDXGIDeviceManagerFunc DynamicMFCreateDXGIDeviceManager_;
		MFCreateAttributesFunc DynamicMFCreateAttributes_;
		MFShutdownFunc DynamicMFShutdown_;

		HMODULE mod_dxgi_ = nullptr;
		HMODULE mod_d3d11_ = nullptr;
		HMODULE mod_mfplat_ = nullptr;

		std::shared_ptr<IDXGIFactory1> dxgi_factory_;
		std::shared_ptr<ID3D11Device> d3d_device_;
		std::shared_ptr<ID3D11DeviceContext> d3d_imm_ctx_;
		std::shared_ptr<IDXGIOutput> dxgi_output_;
		std::shared_ptr<ID3D11Texture2D> d3d_present_tex_;
		std::shared_ptr<ID3D11Texture2D> d3d_present_cpu_tex_;
		std::shared_ptr<IMFDXGIDeviceManager> dxgi_dev_manager_;
		std::shared_ptr<IMFMediaEngine> media_engine_;

		uint8_t dxgi_sub_ver_;
		bool eos_ = false;
		bool stop_timer_ = true;
		RECT rc_target_ = { 0, 0, 0, 0 };
		DXGI_FORMAT d3d_format_ = DXGI_FORMAT_B8G8R8A8_UNORM;
		ElementFormat format_ = EF_ARGB8_SRGB;
		bool present_tex_dirty_ = false;

		std::mutex mutex_;
		joiner<void> play_thread_;

		TexturePtr present_tex_;
	};
}

#endif		// KLAYGE_PLUGINS_MF_SHOW_HPP
