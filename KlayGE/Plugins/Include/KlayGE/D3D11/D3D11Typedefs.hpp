// D3D11Typedefs.hpp
// KlayGE 一些D3D11相关的typedef 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
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
#include <d3d11.h>
#include <d3dx11.h>

namespace KlayGE
{
	typedef boost::shared_ptr<IDXGIFactory>					IDXGIFactoryPtr;
	typedef boost::shared_ptr<IDXGIAdapter>					IDXGIAdapterPtr;
	typedef boost::shared_ptr<IDXGISwapChain>				IDXGISwapChainPtr;
	typedef boost::shared_ptr<ID3D11Device>					ID3D11DevicePtr;
	typedef boost::shared_ptr<ID3D11DeviceContext>			ID3D11DeviceContextPtr;
	typedef boost::shared_ptr<ID3D11Texture1D>				ID3D11Texture1DPtr;
	typedef boost::shared_ptr<ID3D11Texture2D>				ID3D11Texture2DPtr;
	typedef boost::shared_ptr<ID3D11Texture3D>				ID3D11Texture3DPtr;
	typedef boost::shared_ptr<ID3D11Texture2D>				ID3D11TextureCubePtr;
	typedef boost::shared_ptr<ID3D11Buffer>					ID3D11BufferPtr;
	typedef boost::shared_ptr<ID3D11InputLayout>			ID3D11InputLayoutPtr;
	typedef boost::shared_ptr<ID3D11Query>					ID3D11QueryPtr;
	typedef boost::shared_ptr<ID3D11Predicate>				ID3D11PredicatePtr;
	typedef boost::shared_ptr<ID3D11VertexShader>			ID3D11VertexShaderPtr;
	typedef boost::shared_ptr<ID3D11PixelShader>			ID3D11PixelShaderPtr;
	typedef boost::shared_ptr<ID3D11GeometryShader>			ID3D11GeometryShaderPtr;
	typedef boost::shared_ptr<ID3D11ComputeShader>			ID3D11ComputeShaderPtr;
	typedef boost::shared_ptr<ID3D11HullShader>				ID3D11HullShaderPtr;
	typedef boost::shared_ptr<ID3D11DomainShader>			ID3D11DomainShaderPtr;
	typedef boost::shared_ptr<ID3D11RenderTargetView>		ID3D11RenderTargetViewPtr;
	typedef boost::shared_ptr<ID3D11DepthStencilView>		ID3D11DepthStencilViewPtr;
	typedef boost::shared_ptr<ID3D11UnorderedAccessView>	ID3D11UnorderedAccessViewPtr;
	typedef boost::shared_ptr<ID3D11RasterizerState>		ID3D11RasterizerStatePtr;
	typedef boost::shared_ptr<ID3D11DepthStencilState>		ID3D11DepthStencilStatePtr;
	typedef boost::shared_ptr<ID3D11BlendState>				ID3D11BlendStatePtr;
	typedef boost::shared_ptr<ID3D11SamplerState>			ID3D11SamplerStatePtr;
	typedef boost::shared_ptr<ID3D11ShaderResourceView>		ID3D11ShaderResourceViewPtr;
	typedef boost::shared_ptr<ID3D10Blob>					ID3D10BlobPtr;
}

#endif		// _D3D11TYPEDEFS_HPP
