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
	#if defined(KLAYGE_COMPILER_MSVC)
	#pragma warning(push)
	#pragma warning(disable: 4005) // Macro redefinition
	#elif defined(KLAYGE_COMPILER_GCC)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wsign-compare" // Ignore comparison between int and uint
	#endif
	#include <d3d11.h>
	#if defined(KLAYGE_COMPILER_MSVC)
	#pragma warning(pop)
	#elif defined(KLAYGE_COMPILER_GCC)
	#pragma GCC diagnostic pop
	#endif
#elif (_WIN32_WINNT < _WIN32_WINNT_WINBLUE)
	#include <d3d11_1.h>
#elif (_WIN32_WINNT < _WIN32_WINNT_WIN10)
	#include <d3d11_2.h>
#else
	#include <dxgi1_4.h>
	#include <d3d11_3.h>
#endif

#if (_WIN32_WINNT < _WIN32_WINNT_WIN8)
	#define DXGI_FORMAT_B4G4R4A4_UNORM static_cast<DXGI_FORMAT>(115)
	#define D3D_FEATURE_LEVEL_11_1 0xb100
#endif
#if (_WIN32_WINNT < _WIN32_WINNT_WIN10)
	#define D3D_FEATURE_LEVEL_12_0 0xc000
	#define D3D_FEATURE_LEVEL_12_1 0xc100
#endif

namespace KlayGE
{
	typedef std::shared_ptr<IDXGIFactory1>				IDXGIFactory1Ptr;
	typedef std::shared_ptr<IDXGIAdapter1>				IDXGIAdapter1Ptr;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef std::shared_ptr<IDXGIFactory2>				IDXGIFactory2Ptr;
	typedef std::shared_ptr<IDXGIAdapter2>				IDXGIAdapter2Ptr;
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
	typedef std::shared_ptr<IDXGIFactory3>				IDXGIFactory3Ptr;
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
	typedef std::shared_ptr<IDXGIFactory4>				IDXGIFactory4Ptr;
#endif
	typedef std::shared_ptr<IDXGISwapChain>				IDXGISwapChainPtr;
	typedef std::shared_ptr<ID3D11Device>				ID3D11DevicePtr;
	typedef std::shared_ptr<ID3D11DeviceContext>		ID3D11DeviceContextPtr;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef std::shared_ptr<IDXGISwapChain1>			IDXGISwapChain1Ptr;
	typedef std::shared_ptr<ID3D11Device1>				ID3D11Device1Ptr;
	typedef std::shared_ptr<ID3D11DeviceContext1>		ID3D11DeviceContext1Ptr;
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
	typedef std::shared_ptr<IDXGISwapChain2>			IDXGISwapChain2Ptr;
	typedef std::shared_ptr<ID3D11Device2>				ID3D11Device2Ptr;
	typedef std::shared_ptr<ID3D11DeviceContext2>		ID3D11DeviceContext2Ptr;
#endif
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
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef std::shared_ptr<ID3D11RasterizerState1>		ID3D11RasterizerState1Ptr;
#endif
	typedef std::shared_ptr<ID3D11DepthStencilState>	ID3D11DepthStencilStatePtr;
	typedef std::shared_ptr<ID3D11BlendState>			ID3D11BlendStatePtr;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef std::shared_ptr<ID3D11BlendState1>			ID3D11BlendState1Ptr;
#endif
	typedef std::shared_ptr<ID3D11SamplerState>			ID3D11SamplerStatePtr;
	typedef std::shared_ptr<ID3D11ShaderResourceView>	ID3D11ShaderResourceViewPtr;
}

#endif		// _D3D11TYPEDEFS_HPP
