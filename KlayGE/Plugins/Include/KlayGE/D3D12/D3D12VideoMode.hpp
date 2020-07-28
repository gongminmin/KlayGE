/**
 * @file D3D12VideoMode.hpp
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

#ifndef _D3D12VIDEOMODE_HPP
#define _D3D12VIDEOMODE_HPP

#pragma once

#include <KlayGE/D3D12/D3D12Typedefs.hpp>

namespace KlayGE
{
	// 保存显示模式信息
	/////////////////////////////////////////////////////////////////////////////////
	class D3D12VideoMode final
	{
	public:
		D3D12VideoMode();
		D3D12VideoMode(uint32_t width, uint32_t height, DXGI_FORMAT format);

		uint32_t Width() const noexcept;
		uint32_t Height() const noexcept;
		DXGI_FORMAT Format() const noexcept;

	private:
		uint32_t		width_;
		uint32_t		height_;
		DXGI_FORMAT		format_;
	};

	bool operator<(D3D12VideoMode const & lhs, D3D12VideoMode const & rhs) noexcept;
	bool operator==(D3D12VideoMode const & lhs, D3D12VideoMode const & rhs) noexcept;
}

#endif			// _D3D12VIDEOMODE_HPP
