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

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
	#ifdef KLAYGE_COMPILER_MSVC
	#pragma warning(push)
	#pragma warning(disable: 4005)
	#endif
	#include <d3d11.h>
	#ifdef KLAYGE_COMPILER_MSVC
	#pragma warning(pop)
	#endif

	#define DXGI_FORMAT_B4G4R4A4_UNORM static_cast<DXGI_FORMAT>(115)
	#define D3D_FEATURE_LEVEL_11_1 0xb100
#elif (_WIN32_WINNT < _WIN32_WINNT_WINBLUE)
	#include <d3d11_1.h>
#else
	#include <d3d11_2.h>
#endif

namespace KlayGE
{
	typedef shared_ptr<IDXGIFactory1>				IDXGIFactory1Ptr;
	typedef shared_ptr<IDXGIAdapter1>				IDXGIAdapter1Ptr;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef shared_ptr<IDXGIFactory2>				IDXGIFactory2Ptr;
	typedef shared_ptr<IDXGIAdapter2>				IDXGIAdapter2Ptr;
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
	typedef shared_ptr<IDXGIFactory3>				IDXGIFactory3Ptr;
#endif
	typedef shared_ptr<IDXGISwapChain>				IDXGISwapChainPtr;
	typedef shared_ptr<ID3D11Device>				ID3D11DevicePtr;
	typedef shared_ptr<ID3D11DeviceContext>			ID3D11DeviceContextPtr;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef shared_ptr<IDXGISwapChain1>				IDXGISwapChain1Ptr;
	typedef shared_ptr<ID3D11Device1>				ID3D11Device1Ptr;
	typedef shared_ptr<ID3D11DeviceContext1>		ID3D11DeviceContext1Ptr;
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
	typedef shared_ptr<IDXGISwapChain2>				IDXGISwapChain2Ptr;
	typedef shared_ptr<ID3D11Device2>				ID3D11Device2Ptr;
	typedef shared_ptr<ID3D11DeviceContext2>		ID3D11DeviceContext2Ptr;
#endif
	typedef shared_ptr<ID3D11Resource>				ID3D11ResourcePtr;
	typedef shared_ptr<ID3D11Texture1D>				ID3D11Texture1DPtr;
	typedef shared_ptr<ID3D11Texture2D>				ID3D11Texture2DPtr;
	typedef shared_ptr<ID3D11Texture3D>				ID3D11Texture3DPtr;
	typedef shared_ptr<ID3D11Texture2D>				ID3D11TextureCubePtr;
	typedef shared_ptr<ID3D11Buffer>				ID3D11BufferPtr;
	typedef shared_ptr<ID3D11InputLayout>			ID3D11InputLayoutPtr;
	typedef shared_ptr<ID3D11Query>					ID3D11QueryPtr;
	typedef shared_ptr<ID3D11Predicate>				ID3D11PredicatePtr;
	typedef shared_ptr<ID3D11VertexShader>			ID3D11VertexShaderPtr;
	typedef shared_ptr<ID3D11PixelShader>			ID3D11PixelShaderPtr;
	typedef shared_ptr<ID3D11GeometryShader>		ID3D11GeometryShaderPtr;
	typedef shared_ptr<ID3D11ComputeShader>			ID3D11ComputeShaderPtr;
	typedef shared_ptr<ID3D11HullShader>			ID3D11HullShaderPtr;
	typedef shared_ptr<ID3D11DomainShader>			ID3D11DomainShaderPtr;
	typedef shared_ptr<ID3D11RenderTargetView>		ID3D11RenderTargetViewPtr;
	typedef shared_ptr<ID3D11DepthStencilView>		ID3D11DepthStencilViewPtr;
	typedef shared_ptr<ID3D11UnorderedAccessView>	ID3D11UnorderedAccessViewPtr;
	typedef shared_ptr<ID3D11RasterizerState>		ID3D11RasterizerStatePtr;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef shared_ptr<ID3D11RasterizerState1>		ID3D11RasterizerState1Ptr;
#endif
	typedef shared_ptr<ID3D11DepthStencilState>		ID3D11DepthStencilStatePtr;
	typedef shared_ptr<ID3D11BlendState>			ID3D11BlendStatePtr;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef shared_ptr<ID3D11BlendState1>			ID3D11BlendState1Ptr;
#endif
	typedef shared_ptr<ID3D11SamplerState>			ID3D11SamplerStatePtr;
	typedef shared_ptr<ID3D11ShaderResourceView>	ID3D11ShaderResourceViewPtr;
}

#endif		// _D3D11TYPEDEFS_HPP
