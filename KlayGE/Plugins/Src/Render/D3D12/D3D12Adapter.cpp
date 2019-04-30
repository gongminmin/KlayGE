/**
 * @file D3D12Adapter.cpp
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

#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12Adapter.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D12Adapter::D3D12Adapter(uint32_t adapter_no, IDXGIAdapter1Ptr const & adapter)
					: adapter_no_(adapter_no)
	{
		this->ResetAdapter(adapter);
	}

	// 获取设备描述字符串
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const D3D12Adapter::Description() const
	{
		return std::wstring(adapter_desc_.Description);
	}

	// 获取支持的显示模式数目
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D12Adapter::NumVideoMode() const
	{
		return modes_.size();
	}

	// 获取显示模式
	/////////////////////////////////////////////////////////////////////////////////
	D3D12VideoMode const & D3D12Adapter::VideoMode(size_t index) const
	{
		BOOST_ASSERT(index < modes_.size());

		return modes_[index];
	}

	// 枚举显示模式
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12Adapter::Enumerate()
	{
		static DXGI_FORMAT constexpr formats[] =
		{
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
			DXGI_FORMAT_R10G10B10A2_UNORM
		};

		UINT i = 0;
		IDXGIOutput* output = nullptr;
		while (adapter_->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			if (output != nullptr)
			{
				for (auto const & format : formats)
				{
					UINT num = 0;
					output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, 0);
					if (num > 0)
					{
						std::vector<DXGI_MODE_DESC> mode_descs(num);
						output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, &mode_descs[0]);

						for (auto const & mode_desc : mode_descs)
						{
							D3D12VideoMode const video_mode(mode_desc.Width, mode_desc.Height,
								mode_desc.Format);

							// 如果找到一个新模式, 加入模式列表
							if (std::find(modes_.begin(), modes_.end(), video_mode) == modes_.end())
							{
								modes_.push_back(video_mode);
							}
						}
					}
				}

				output->Release();
				output = nullptr;
			}

			++ i;
		}

		std::sort(modes_.begin(), modes_.end());
	}

	void D3D12Adapter::ResetAdapter(IDXGIAdapter1Ptr const & ada)
	{
		adapter_ = ada;
		adapter_->GetDesc1(&adapter_desc_);
		modes_.resize(0);

		IDXGIAdapter3* adapter3;
		adapter_->QueryInterface(IID_IDXGIAdapter3, reinterpret_cast<void**>(&adapter3));
		if (adapter3 != nullptr)
		{
			DXGI_ADAPTER_DESC2 desc2;
			adapter3->GetDesc2(&desc2);
			memcpy(adapter_desc_.Description, desc2.Description, sizeof(desc2.Description));
			adapter_desc_.VendorId = desc2.VendorId;
			adapter_desc_.DeviceId = desc2.DeviceId;
			adapter_desc_.SubSysId = desc2.SubSysId;
			adapter_desc_.Revision = desc2.Revision;
			adapter_desc_.DedicatedVideoMemory = desc2.DedicatedVideoMemory;
			adapter_desc_.DedicatedSystemMemory = desc2.DedicatedSystemMemory;
			adapter_desc_.SharedSystemMemory = desc2.SharedSystemMemory;
			adapter_desc_.AdapterLuid = desc2.AdapterLuid;
			adapter_desc_.Flags = desc2.Flags;
			adapter3->Release();
		}
	}
}
