/**
 * @file D3D12Adapter.hpp
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

#ifndef _D3D12ADAPTER_HPP
#define _D3D12ADAPTER_HPP

#pragma once

#include <vector>
#include <string>
#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12VideoMode.hpp>

namespace KlayGE
{
	// 保存适配器的信息，包含该适配器支持的设备列表
	/////////////////////////////////////////////////////////////////////////////////
	class D3D12Adapter final
	{
	public:
		D3D12Adapter(uint32_t adapter_no, IDXGIAdapter2* adapter);

		void Enumerate();

		// 访问设备描述字符串
		std::wstring const Description() const;
		void ResetAdapter(IDXGIAdapter2* adapter);

		uint32_t AdapterNo() const noexcept
		{
			return adapter_no_;
		}

		IDXGIAdapter2* DXGIAdapter() const noexcept
		{
			return adapter_.get();
		}

		DXGI_FORMAT DesktopFormat() const noexcept
		{
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		size_t NumVideoMode() const noexcept;
		D3D12VideoMode const & VideoMode(size_t index) const;

	private:
		// 这是第几块适配器
		uint32_t			adapter_no_;

		// 适配器信息
		IDXGIAdapter2Ptr adapter_;
		DXGI_ADAPTER_DESC2 adapter_desc_{};

		// 显示模式列表
		std::vector<D3D12VideoMode> modes_;
	};

	typedef std::shared_ptr<D3D12Adapter> D3D12AdapterPtr;
}

#endif			// _D3D12ADAPTER_HPP
