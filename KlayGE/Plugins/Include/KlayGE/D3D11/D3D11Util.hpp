/**
 * @file D3D11Util.hpp
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

#ifndef KLAYGE_PLUGINS_D3D11_UTIL_HPP
#define KLAYGE_PLUGINS_D3D11_UTIL_HPP

#pragma once

#include <KlayGE/SALWrapper.hpp>
#include <dxgi1_6.h>
#ifndef D3D10_NO_HELPERS
#define D3D10_NO_HELPERS
#endif
#ifndef D3D11_NO_HELPERS
#define D3D11_NO_HELPERS
#endif
#include <d3d11_4.h>

#include <KFL/com_ptr.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX20/span.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/Texture.hpp>

namespace KlayGE
{
	using IDXGIFactory2Ptr = com_ptr<IDXGIFactory2>;
	using IDXGIFactory3Ptr = com_ptr<IDXGIFactory3>;
	using IDXGIFactory4Ptr = com_ptr<IDXGIFactory4>;
	using IDXGIFactory5Ptr = com_ptr<IDXGIFactory5>;
	using IDXGIFactory6Ptr = com_ptr<IDXGIFactory6>;
	using IDXGIAdapter2Ptr = com_ptr<IDXGIAdapter2>;
	using IDXGISwapChain1Ptr = com_ptr<IDXGISwapChain1>;
	using IDXGISwapChain2Ptr = com_ptr<IDXGISwapChain2>;
	using IDXGISwapChain3Ptr = com_ptr<IDXGISwapChain3>;
	using IDXGISwapChain4Ptr = com_ptr<IDXGISwapChain4>;
	using ID3D11Device1Ptr = com_ptr<ID3D11Device1>;
	using ID3D11Device2Ptr = com_ptr<ID3D11Device2>;
	using ID3D11Device3Ptr = com_ptr<ID3D11Device3>;
	using ID3D11Device4Ptr = com_ptr<ID3D11Device4>;
	using ID3D11Device5Ptr = com_ptr<ID3D11Device5>;
	using ID3D11DeviceContext1Ptr = com_ptr<ID3D11DeviceContext1>;
	using ID3D11DeviceContext2Ptr = com_ptr<ID3D11DeviceContext2>;
	using ID3D11DeviceContext3Ptr = com_ptr<ID3D11DeviceContext3>;
	using ID3D11DeviceContext4Ptr = com_ptr<ID3D11DeviceContext4>;
	using ID3D11ResourcePtr = com_ptr<ID3D11Resource>;
	using ID3D11Texture1DPtr = com_ptr<ID3D11Texture1D>;
	using ID3D11Texture2DPtr = com_ptr<ID3D11Texture2D>;
	using ID3D11Texture3DPtr = com_ptr<ID3D11Texture3D>;
	using ID3D11TextureCubePtr = com_ptr<ID3D11Texture2D>;
	using ID3D11BufferPtr = com_ptr<ID3D11Buffer>;
	using ID3D11FencePtr = com_ptr<ID3D11Fence>;
	using ID3D11InputLayoutPtr = com_ptr<ID3D11InputLayout>;
	using ID3D11QueryPtr = com_ptr<ID3D11Query>;
	using ID3D11PredicatePtr = com_ptr<ID3D11Predicate>;
	using ID3D11VertexShaderPtr = com_ptr<ID3D11VertexShader>;
	using ID3D11PixelShaderPtr = com_ptr<ID3D11PixelShader>;
	using ID3D11GeometryShaderPtr = com_ptr<ID3D11GeometryShader>;
	using ID3D11ComputeShaderPtr = com_ptr<ID3D11ComputeShader>;
	using ID3D11HullShaderPtr = com_ptr<ID3D11HullShader>;
	using ID3D11DomainShaderPtr = com_ptr<ID3D11DomainShader>;
	using ID3D11RenderTargetViewPtr = com_ptr<ID3D11RenderTargetView>;
	using ID3D11DepthStencilViewPtr = com_ptr<ID3D11DepthStencilView>;
	using ID3D11UnorderedAccessViewPtr = com_ptr<ID3D11UnorderedAccessView>;
	using ID3D11RasterizerState1Ptr = com_ptr<ID3D11RasterizerState1>;
	using ID3D11DepthStencilStatePtr = com_ptr<ID3D11DepthStencilState>;
	using ID3D11BlendState1Ptr = com_ptr<ID3D11BlendState1>;
	using ID3D11SamplerStatePtr = com_ptr<ID3D11SamplerState>;
	using ID3D11ShaderResourceViewPtr = com_ptr<ID3D11ShaderResourceView>;

	constexpr uint32_t D3D11CalcSubresource(uint32_t mip_slice, uint32_t array_slice, uint32_t mip_levels) noexcept
	{
		return mip_slice + array_slice * mip_levels;
	}

	class D3D11Mapping final
	{
	public:
		static uint32_t MappingColorMask(uint32_t mask);

		static D3D11_COMPARISON_FUNC Mapping(CompareFunction func);

		static D3D11_STENCIL_OP Mapping(StencilOperation op);

		static D3D11_MAP Mapping(TextureMapAccess tma, Texture::TextureType type, uint32_t access_hint, uint32_t numMipMaps);

		static D3D11_BLEND Mapping(AlphaBlendFactor factor);
		static D3D11_CULL_MODE Mapping(CullMode mode);
		static D3D11_FILL_MODE Mapping(PolygonMode mode);
		static D3D11_BLEND_OP Mapping(BlendOperation bo);
		static D3D11_TEXTURE_ADDRESS_MODE Mapping(TexAddressingMode mode);
		static D3D11_FILTER Mapping(TexFilterOp filter);
		static D3D11_DEPTH_WRITE_MASK Mapping(bool depth_write_mask);
		static D3D11_LOGIC_OP Mapping(LogicOperation lo);

		static D3D11_PRIMITIVE_TOPOLOGY Mapping(RenderLayout::topology_type tt);
		static void Mapping(std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, size_t stream, std::span<VertexElement const> vet,
			RenderLayout::stream_type type, uint32_t freq);

		static D3D11_SO_DECLARATION_ENTRY Mapping(ShaderDesc::StreamOutputDecl const& decl);

		static DXGI_FORMAT MappingFormat(ElementFormat pf);
		static ElementFormat MappingFormat(DXGI_FORMAT d3dfmt);
	};
} // namespace KlayGE

#endif // KLAYGE_PLUGINS_D3D11_UTIL_HPP
