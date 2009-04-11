// D3D10Typedefs.hpp
// KlayGE 一些D3D10相关的typedef 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10TYPEDEFS_HPP
#define _D3D10TYPEDEFS_HPP

#pragma KLAYGE_ONCE

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>
#include <d3dx10.h>

namespace KlayGE
{
	typedef boost::shared_ptr<IDXGIFactory>				IDXGIFactoryPtr;
	typedef boost::shared_ptr<IDXGIAdapter>				IDXGIAdapterPtr;
	typedef boost::shared_ptr<IDXGISwapChain>			IDXGISwapChainPtr;
	typedef boost::shared_ptr<ID3D10Device>				ID3D10DevicePtr;
	typedef boost::shared_ptr<ID3D10Texture1D>			ID3D10Texture1DPtr;
	typedef boost::shared_ptr<ID3D10Texture2D>			ID3D10Texture2DPtr;
	typedef boost::shared_ptr<ID3D10Texture3D>			ID3D10Texture3DPtr;
	typedef boost::shared_ptr<ID3D10Texture2D>			ID3D10TextureCubePtr;
	typedef boost::shared_ptr<ID3D10Buffer>				ID3D10BufferPtr;
	typedef boost::shared_ptr<ID3D10InputLayout>		ID3D10InputLayoutPtr;
	typedef boost::shared_ptr<ID3D10Query>				ID3D10QueryPtr;
	typedef boost::shared_ptr<ID3D10Predicate>			ID3D10PredicatePtr;
	typedef boost::shared_ptr<ID3D10VertexShader>		ID3D10VertexShaderPtr;
	typedef boost::shared_ptr<ID3D10PixelShader>		ID3D10PixelShaderPtr;
	typedef boost::shared_ptr<ID3D10GeometryShader>		ID3D10GeometryShaderPtr;
	typedef boost::shared_ptr<ID3D10RenderTargetView>	ID3D10RenderTargetViewPtr;
	typedef boost::shared_ptr<ID3D10DepthStencilView>	ID3D10DepthStencilViewPtr;
	typedef boost::shared_ptr<ID3D10RasterizerState>	ID3D10RasterizerStatePtr;
	typedef boost::shared_ptr<ID3D10DepthStencilState>	ID3D10DepthStencilStatePtr;
	typedef boost::shared_ptr<ID3D10BlendState>			ID3D10BlendStatePtr;
	typedef boost::shared_ptr<ID3D10SamplerState>		ID3D10SamplerStatePtr;
	typedef boost::shared_ptr<ID3D10ShaderResourceView> ID3D10ShaderResourceViewPtr;
	typedef boost::shared_ptr<ID3D10Blob>				ID3D10BlobPtr;
}

#endif		// _D3D10TYPEDEFS_HPP
