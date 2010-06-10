// D3D10Mapping.hpp
// KlayGE RenderEngine和D3D10本地之间的映射 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10MAPPING_HPP
#define _D3D10MAPPING_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Texture.hpp>

namespace KlayGE
{
	class D3D10Mapping
	{
	public:
		static uint32_t MappingColorMask(uint32_t mask);

		static D3D10_COMPARISON_FUNC Mapping(CompareFunction func);

		static D3D10_STENCIL_OP Mapping(StencilOperation op);

		static D3D10_MAP Mapping(TextureMapAccess tma, Texture::TextureType type, uint32_t access_hint, uint32_t numMipMaps);

		static D3D10_BLEND Mapping(AlphaBlendFactor factor);
		static D3D10_CULL_MODE Mapping(CullMode mode);
		static D3D10_FILL_MODE Mapping(PolygonMode mode);
		static D3D10_BLEND_OP Mapping(BlendOperation bo);
		static D3D10_TEXTURE_ADDRESS_MODE Mapping(TexAddressingMode mode);
		static D3D10_FILTER Mapping(TexFilterOp filter);
		static D3D10_DEPTH_WRITE_MASK Mapping(bool depth_write_mask);

		static D3D10_PRIMITIVE_TOPOLOGY Mapping(RenderLayout::topology_type tt);
		static void Mapping(std::vector<D3D10_INPUT_ELEMENT_DESC>& elements, size_t stream, vertex_elements_type const & vet, RenderLayout::stream_type type, uint32_t freq);

		static D3D10_SO_DECLARATION_ENTRY Mapping(shader_desc::stream_output_decl const & decl, uint8_t slot);

		static DXGI_FORMAT MappingFormat(ElementFormat pf);
		static ElementFormat MappingFormat(DXGI_FORMAT d3dfmt);
	};
}

#endif			// _D3D10MAPPING_HPP
