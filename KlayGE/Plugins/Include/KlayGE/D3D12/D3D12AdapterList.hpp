/**
 * @file D3D12AdapterList.hpp
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

#ifndef _D3D12ADAPTERLIST_HPP
#define _D3D12ADAPTERLIST_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12Adapter.hpp>

namespace KlayGE
{
	class D3D12Adapter;

	class D3D12AdapterList
	{
	public:
		D3D12AdapterList();

		void Destroy();

		void Enumerate(IDXGIFactory4Ptr const & gi_factory);

		size_t NumAdapter() const;
		D3D12Adapter& Adapter(size_t index) const;

		uint32_t CurrentAdapterIndex() const;
		void CurrentAdapterIndex(uint32_t index);

	private:
		std::vector<std::unique_ptr<D3D12Adapter>> adapters_;
		uint32_t			current_adapter_;
	};

	typedef std::shared_ptr<D3D12AdapterList> D3D12AdapterListPtr;
}

#endif			// _D3D12ADAPTERLIST_HPP
