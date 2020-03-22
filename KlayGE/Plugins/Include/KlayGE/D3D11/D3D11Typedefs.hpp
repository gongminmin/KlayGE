// D3D11Typedefs.hpp
// KlayGE 一些D3D11相关的typedef 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11TYPEDEFS_HPP
#define _D3D11TYPEDEFS_HPP

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
} // namespace KlayGE

#endif // _D3D11TYPEDEFS_HPP
