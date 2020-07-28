/**
 * @file D3D12RenderLayout.hpp
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

#ifndef _D3D12RENDERLAYOUT_HPP
#define _D3D12RENDERLAYOUT_HPP

#pragma once

#include <vector>

#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/D3D12/D3D12Typedefs.hpp>

namespace KlayGE
{
	class D3D12RenderLayout final : public RenderLayout
	{
	public:
		D3D12RenderLayout();

		std::vector<D3D12_INPUT_ELEMENT_DESC> const & InputElementDesc() const;

		void Active(ID3D12GraphicsCommandList* cmd_list) const;

		size_t PsoHashValue();
		void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc, bool has_tessellation);

	private:
		void UpdateViewPointers();

	private:
		mutable std::vector<D3D12_INPUT_ELEMENT_DESC> vertex_elems_;

		mutable std::vector<D3D12_VERTEX_BUFFER_VIEW> vbvs_;
		mutable D3D12_INDEX_BUFFER_VIEW ibv_;

		// For PSOs
		size_t pso_hash_value_;
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ib_strip_cut_value_;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE prim_topology_;
	};
}

#endif			// _D3D12RENDERLAYOUT_HPP
