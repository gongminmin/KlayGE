/**
 * @file D3D12AdapterList.cpp
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
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>

#include <vector>
#include <system_error>
#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12InterfaceLoader.hpp>
#include <KlayGE/D3D12/D3D12AdapterList.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D12AdapterList::D3D12AdapterList() noexcept = default;

	void D3D12AdapterList::Destroy()
	{
		adapters_.clear();
		current_adapter_ = 0;
	}

	// 获取系统显卡数目
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D12AdapterList::NumAdapter() const noexcept
	{
		return adapters_.size();
	}

	// 获取显卡
	/////////////////////////////////////////////////////////////////////////////////
	D3D12Adapter& D3D12AdapterList::Adapter(size_t index) const
	{
		BOOST_ASSERT(index < adapters_.size());

		return *adapters_[index];
	}

	// 获取当前显卡索引
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D12AdapterList::CurrentAdapterIndex() const noexcept
	{
		return current_adapter_;
	}

	// 设置当前显卡索引
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12AdapterList::CurrentAdapterIndex(uint32_t index) noexcept
	{
		current_adapter_ = index;
	}

	// 枚举系统显卡
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12AdapterList::Enumerate(IDXGIFactory4* gi_factory)
	{
		// 枚举系统中的适配器
		UINT adapter_no = 0;
		com_ptr<IDXGIAdapter1> dxgi_adapter;
		while (gi_factory->EnumAdapters1(adapter_no, dxgi_adapter.release_and_put()) != DXGI_ERROR_NOT_FOUND)
		{
			if (dxgi_adapter != nullptr)
			{
				com_ptr<ID3D12Device> device;
				if (SUCCEEDED(D3D12InterfaceLoader::Instance().D3D12CreateDevice(dxgi_adapter.get(), D3D_FEATURE_LEVEL_11_0,
					IID_ID3D12Device, device.put_void())))
				{
					auto adapter = MakeUniquePtr<D3D12Adapter>(adapter_no, dxgi_adapter.as<IDXGIAdapter2>(IID_IDXGIAdapter2).get());
					adapter->Enumerate();
					adapters_.push_back(std::move(adapter));
				}
			}

			++ adapter_no;
		}

		// 如果没有找到兼容的设备则抛出错误
		if (adapters_.empty())
		{
			TERRC(std::errc::function_not_supported);
		}
	}

	void D3D12AdapterList::Enumerate(IDXGIFactory6* gi_factory)
	{
		UINT adapter_no = 0;
		IDXGIAdapter2Ptr dxgi_adapter;
		while (SUCCEEDED(gi_factory->EnumAdapterByGpuPreference(adapter_no, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_IDXGIAdapter2, dxgi_adapter.release_and_put_void())))
		{
			if (dxgi_adapter != nullptr)
			{
				com_ptr<ID3D12Device> device;
				if (SUCCEEDED(D3D12InterfaceLoader::Instance().D3D12CreateDevice(dxgi_adapter.get(), D3D_FEATURE_LEVEL_11_0,
					IID_ID3D12Device, device.put_void())))
				{
					auto adapter = MakeUniquePtr<D3D12Adapter>(adapter_no, dxgi_adapter.get());
					adapter->Enumerate();
					adapters_.push_back(std::move(adapter));
				}
			}

			++ adapter_no;
		}

		if (adapters_.empty())
		{
			auto gi_factory4 = com_ptr<IDXGIFactory6>(gi_factory).as<IDXGIFactory4>(IID_IDXGIFactory4);
			this->Enumerate(gi_factory4.get());
		}

		if (adapters_.empty())
		{
			TERRC(std::errc::function_not_supported);
		}
	}
}
