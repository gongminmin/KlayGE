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
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore int enum
#endif
#include <dxgi1_6.h>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare" // Ignore comparison between int and uint
#elif defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-const-init" // Ignore const init (a Microsoft extension)
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore int enum
#pragma clang diagnostic ignored "-Wsign-compare" // Ignore comparison between int and uint
#endif
#include <d3d11_4.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

namespace KlayGE
{
	typedef std::shared_ptr<IDXGIFactory1>				IDXGIFactory1Ptr;
	typedef std::shared_ptr<IDXGIFactory2>				IDXGIFactory2Ptr;
	typedef std::shared_ptr<IDXGIFactory3>				IDXGIFactory3Ptr;
	typedef std::shared_ptr<IDXGIFactory4>				IDXGIFactory4Ptr;
	typedef std::shared_ptr<IDXGIFactory5>				IDXGIFactory5Ptr;
	typedef std::shared_ptr<IDXGIAdapter1>				IDXGIAdapter1Ptr;
	typedef std::shared_ptr<IDXGIAdapter2>				IDXGIAdapter2Ptr;
	typedef std::shared_ptr<IDXGISwapChain>				IDXGISwapChainPtr;
	typedef std::shared_ptr<IDXGISwapChain1>			IDXGISwapChain1Ptr;
	typedef std::shared_ptr<IDXGISwapChain2>			IDXGISwapChain2Ptr;
	typedef std::shared_ptr<IDXGISwapChain3>			IDXGISwapChain3Ptr;
	typedef std::shared_ptr<IDXGISwapChain4>			IDXGISwapChain4Ptr;
	typedef std::shared_ptr<ID3D11Device>				ID3D11DevicePtr;
	typedef std::shared_ptr<ID3D11Device1>				ID3D11Device1Ptr;
	typedef std::shared_ptr<ID3D11Device2>				ID3D11Device2Ptr;
	typedef std::shared_ptr<ID3D11Device3>				ID3D11Device3Ptr;
	typedef std::shared_ptr<ID3D11Device4>				ID3D11Device4Ptr;
	typedef std::shared_ptr<ID3D11DeviceContext>		ID3D11DeviceContextPtr;
	typedef std::shared_ptr<ID3D11DeviceContext1>		ID3D11DeviceContext1Ptr;
	typedef std::shared_ptr<ID3D11DeviceContext2>		ID3D11DeviceContext2Ptr;
	typedef std::shared_ptr<ID3D11DeviceContext3>		ID3D11DeviceContext3Ptr;
	typedef std::shared_ptr<ID3D11Resource>				ID3D11ResourcePtr;
	typedef std::shared_ptr<ID3D11Texture1D>			ID3D11Texture1DPtr;
	typedef std::shared_ptr<ID3D11Texture2D>			ID3D11Texture2DPtr;
	typedef std::shared_ptr<ID3D11Texture3D>			ID3D11Texture3DPtr;
	typedef std::shared_ptr<ID3D11Texture2D>			ID3D11TextureCubePtr;
	typedef std::shared_ptr<ID3D11Buffer>				ID3D11BufferPtr;
	typedef std::shared_ptr<ID3D11InputLayout>			ID3D11InputLayoutPtr;
	typedef std::shared_ptr<ID3D11Query>				ID3D11QueryPtr;
	typedef std::shared_ptr<ID3D11Predicate>			ID3D11PredicatePtr;
	typedef std::shared_ptr<ID3D11VertexShader>			ID3D11VertexShaderPtr;
	typedef std::shared_ptr<ID3D11PixelShader>			ID3D11PixelShaderPtr;
	typedef std::shared_ptr<ID3D11GeometryShader>		ID3D11GeometryShaderPtr;
	typedef std::shared_ptr<ID3D11ComputeShader>		ID3D11ComputeShaderPtr;
	typedef std::shared_ptr<ID3D11HullShader>			ID3D11HullShaderPtr;
	typedef std::shared_ptr<ID3D11DomainShader>			ID3D11DomainShaderPtr;
	typedef std::shared_ptr<ID3D11RenderTargetView>		ID3D11RenderTargetViewPtr;
	typedef std::shared_ptr<ID3D11DepthStencilView>		ID3D11DepthStencilViewPtr;
	typedef std::shared_ptr<ID3D11UnorderedAccessView>	ID3D11UnorderedAccessViewPtr;
	typedef std::shared_ptr<ID3D11RasterizerState>		ID3D11RasterizerStatePtr;
	typedef std::shared_ptr<ID3D11RasterizerState1>		ID3D11RasterizerState1Ptr;
	typedef std::shared_ptr<ID3D11DepthStencilState>	ID3D11DepthStencilStatePtr;
	typedef std::shared_ptr<ID3D11BlendState>			ID3D11BlendStatePtr;
	typedef std::shared_ptr<ID3D11BlendState1>			ID3D11BlendState1Ptr;
	typedef std::shared_ptr<ID3D11SamplerState>			ID3D11SamplerStatePtr;
	typedef std::shared_ptr<ID3D11ShaderResourceView>	ID3D11ShaderResourceViewPtr;
}

#endif		// _D3D11TYPEDEFS_HPP
