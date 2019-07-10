/**
 * @file D3D12Mapping.hpp
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

#ifndef _D3D12MAPPING_HPP
#define _D3D12MAPPING_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX2a/span.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Texture.hpp>

namespace KlayGE
{
	class D3D12Mapping
	{
	public:
		static uint32_t MappingColorMask(uint32_t mask);

		static D3D12_COMPARISON_FUNC Mapping(CompareFunction func);

		static D3D12_STENCIL_OP Mapping(StencilOperation op);

		static D3D12_BLEND Mapping(AlphaBlendFactor factor);
		static D3D12_CULL_MODE Mapping(CullMode mode);
		static D3D12_FILL_MODE Mapping(PolygonMode mode);
		static D3D12_BLEND_OP Mapping(BlendOperation bo);
		static D3D12_TEXTURE_ADDRESS_MODE Mapping(TexAddressingMode mode);
		static D3D12_FILTER Mapping(TexFilterOp filter);
		static D3D12_DEPTH_WRITE_MASK Mapping(bool depth_write_mask);
		static D3D12_LOGIC_OP Mapping(LogicOperation lo);

		static D3D12_PRIMITIVE_TOPOLOGY Mapping(RenderLayout::topology_type tt);
		static D3D12_PRIMITIVE_TOPOLOGY_TYPE MappingPriTopoType(RenderLayout::topology_type tt);
		static void Mapping(std::vector<D3D12_INPUT_ELEMENT_DESC>& elements, size_t stream, std::span<VertexElement const> vet,
			RenderLayout::stream_type type, uint32_t freq);

		static D3D12_SO_DECLARATION_ENTRY Mapping(ShaderDesc::StreamOutputDecl const & decl);

		static DXGI_FORMAT MappingFormat(ElementFormat pf);
		static ElementFormat MappingFormat(DXGI_FORMAT d3dfmt);
	};
}

#endif			// _D3D12MAPPING_HPP
