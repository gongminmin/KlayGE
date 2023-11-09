/**
 * @file D3D12InterfaceLoader.cpp
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

#include <KlayGE/KlayGE.hpp>

#include <ostream>

#include <KFL/Log.hpp>

#include "D3D12InterfaceLoader.hpp"

namespace KlayGE
{
	D3D12InterfaceLoader::D3D12InterfaceLoader()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		// Dynamic loading because these dlls can't be loaded on WinXP
		if (!mod_dxgi_.Load("dxgi.dll"))
		{
			LogError() << "COULDN'T load dxgi.dll" << std::endl;
			Verify(false);
		}
		if (!mod_d3d12_.Load("d3d12.dll"))
		{
			LogError() << "COULDN'T load d3d12.dll" << std::endl;
			Verify(false);
		}

		DynamicCreateDXGIFactory2_ = reinterpret_cast<CreateDXGIFactory2Func>(mod_dxgi_.GetProcAddress("CreateDXGIFactory2"));
		DynamicD3D12CreateDevice_ = reinterpret_cast<D3D12CreateDeviceFunc>(mod_d3d12_.GetProcAddress("D3D12CreateDevice"));
		DynamicD3D12GetDebugInterface_ = reinterpret_cast<D3D12GetDebugInterfaceFunc>(mod_d3d12_.GetProcAddress("D3D12GetDebugInterface"));
		DynamicD3D12SerializeRootSignature_ =
			reinterpret_cast<D3D12SerializeRootSignatureFunc>(mod_d3d12_.GetProcAddress("D3D12SerializeRootSignature"));
#else
		DynamicCreateDXGIFactory2_ = ::CreateDXGIFactory2;
		DynamicD3D12CreateDevice_ = ::D3D12CreateDevice;
		DynamicD3D12GetDebugInterface_ = ::D3D12GetDebugInterface;
		DynamicD3D12SerializeRootSignature_ = ::D3D12SerializeRootSignature;
#endif
	}

	D3D12InterfaceLoader::~D3D12InterfaceLoader()
	{
		this->Destroy();
	}

	D3D12InterfaceLoader& D3D12InterfaceLoader::Instance()
	{
		static D3D12InterfaceLoader ret;
		return ret;
	}

	void D3D12InterfaceLoader::Destroy()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		mod_d3d12_.Free();
		mod_dxgi_.Free();
#endif

	}

	HRESULT D3D12InterfaceLoader::CreateDXGIFactory2(UINT flags, REFIID riid, void** ppFactory) const
	{
		return DynamicCreateDXGIFactory2_(flags, riid, ppFactory);
	}

	HRESULT D3D12InterfaceLoader::D3D12CreateDevice(IUnknown* pAdapter,
		D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid,
		void** ppDevice)
	{
		return DynamicD3D12CreateDevice_(pAdapter, MinimumFeatureLevel, riid, ppDevice);
	}

	HRESULT D3D12InterfaceLoader::D3D12GetDebugInterface(REFIID riid, void** ppvDebug) const
	{
		return DynamicD3D12GetDebugInterface_(riid, ppvDebug);
	}

	HRESULT D3D12InterfaceLoader::D3D12SerializeRootSignature(D3D12_ROOT_SIGNATURE_DESC const * pRootSignature,
		D3D_ROOT_SIGNATURE_VERSION Version, ID3DBlob** ppBlob, ID3DBlob** ppErrorBlob) const
	{
		return DynamicD3D12SerializeRootSignature_(pRootSignature, Version, ppBlob, ppErrorBlob);
	}
}
