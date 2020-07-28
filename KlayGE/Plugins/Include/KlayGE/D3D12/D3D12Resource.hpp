/**
 * @file D3D12Resource.hpp
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

#ifndef _D3D12RESOURCE_HPP
#define _D3D12RESOURCE_HPP

#pragma once

#include <KlayGE/D3D12/D3D12Typedefs.hpp>

namespace KlayGE
{
	class D3D12Resource
	{
	public:
		D3D12Resource();
		~D3D12Resource();

		ID3D12Resource* D3DResource() const noexcept
		{
			return d3d_resource_.get();
		}

		uint32_t D3DResourceOffset() const noexcept
		{
			return d3d_resource_offset_;
		}

		void UpdateResourceBarrier(ID3D12GraphicsCommandList* cmd_list, uint32_t sub_res, D3D12_RESOURCE_STATES target_state);

	protected:
		ID3D12ResourcePtr d3d_resource_;
		uint32_t d3d_resource_offset_;

		std::vector<D3D12_RESOURCE_STATES> curr_states_;
	};
	typedef std::shared_ptr<D3D12Resource> D3D12ResourcePtr;
}

#endif			// _D3D12RESOURCE_HPP
