// D3D11RenderEngine.hpp
// KlayGE D3D11渲染引擎类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERENGINE_HPP
#define _D3D11RENDERENGINE_HPP

#pragma once

#include <KlayGE/Vector.hpp>
#include <KlayGE/Color.hpp>

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>
#include <d3dx11.h>

#include <vector>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/D3D11/D3D11AdapterList.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>

namespace KlayGE
{
	class D3D11AdapterList;
	class D3D11Adapter;

	class D3D11RenderEngine : public RenderEngine
	{
	public:
		D3D11RenderEngine();
		~D3D11RenderEngine();

		std::wstring const & Name() const;

		IDXGIFactoryPtr const & DXGIFactory() const;
		ID3D11DevicePtr const & D3DDevice() const;
		ID3D11DeviceContextPtr const & D3DDeviceImmContext() const;
		D3D_FEATURE_LEVEL DeviceFeatureLevel() const;
		void D3DDevice(ID3D11DevicePtr const & device, ID3D11DeviceContextPtr const & imm_ctx, D3D_FEATURE_LEVEL feature_level);

		void CreateRenderWindow(std::string const & name, RenderSettings const & settings);

		void StartRendering();

		void BeginFrame();
		void EndFrame();
		void BeginPass();
		void EndPass();

		uint16_t StencilBufferBitDepth();

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		// Directly mapping texels to pixels by offset by 0.5
		// For more info see the doc page titled "Directly Mapping Texels to Pixels"
		float4 TexelToPixelOffset() const
		{
			return float4(0, 0, 0, 0);
		}

		void Resize(uint32_t width, uint32_t height);
		bool FullScreen() const;
		void FullScreen(bool fs);

		HRESULT D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter,
								D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
								D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
								DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
								ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const
		{
			return DynamicD3D11CreateDeviceAndSwapChain_(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion,
				pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
		}
		std::string const & VertexShaderProfile() const
		{
			return vs_profile_;
		}
		std::string const & PixelShaderProfile() const
		{
			return ps_profile_;
		}
		std::string const & GeometryShaderProfile() const
		{
			return gs_profile_;
		}

		void RSSetState(ID3D11RasterizerStatePtr const & ras);
		void OMSetDepthStencilState(ID3D11DepthStencilStatePtr const & ds, uint16_t stencil_ref);
		void OMSetBlendState(ID3D11BlendStatePtr const & bs, Color const & blend_factor, uint32_t sample_mask);
		void VSSetShader(ID3D11VertexShaderPtr const & shader);
		void PSSetShader(ID3D11PixelShaderPtr const & shader);
		void GSSetShader(ID3D11GeometryShaderPtr const & shader);
		
		void ResetRenderStates();

	private:
		void DoBindFrameBuffer(FrameBufferPtr const & fb);
		void DoBindSOBuffers(size_t num_buffs, GraphicsBufferPtr* buffs, size_t* offsets);
		void DoRender(RenderTechnique const & tech, RenderLayout const & rl);

		void FillRenderDeviceCaps();

	private:
		D3D11AdapterList const & D3DAdapters() const;
		D3D11AdapterPtr const & ActiveAdapter() const;

		HMODULE mod_dxgi_;
		HMODULE mod_d3d11_;

		typedef HRESULT (WINAPI *CreateDXGIFactoryFunc)(REFIID riid, void** ppFactory);
		typedef HRESULT (WINAPI *D3D11CreateDeviceAndSwapChainFunc)(IDXGIAdapter* pAdapter,
								D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
								D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
								DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
								ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

		CreateDXGIFactoryFunc DynamicCreateDXGIFactory_;
		D3D11CreateDeviceAndSwapChainFunc DynamicD3D11CreateDeviceAndSwapChain_;


		// Direct3D rendering device
		// Only created after top-level window created
		IDXGIFactoryPtr		gi_factory_;
		ID3D11DevicePtr		d3d_device_;
		ID3D11DeviceContextPtr d3d_imm_ctx_;
		D3D_FEATURE_LEVEL d3d_feature_level_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D11AdapterList adapterList_;

		ID3D11RasterizerStatePtr rasterizer_state_cache_;
		ID3D11DepthStencilStatePtr depth_stencil_state_cache_;
		uint16_t stencil_ref_cache_;
		ID3D11BlendStatePtr blend_state_cache_;
		Color blend_factor_cache_;
		uint32_t sample_mask_cache_;
		ID3D11VertexShaderPtr vertex_shader_cache_;
		ID3D11PixelShaderPtr pixel_shader_cache_;
		ID3D11GeometryShaderPtr geometry_shader_cache_;

		std::string vs_profile_, ps_profile_, gs_profile_;
	};

	typedef boost::shared_ptr<D3D11RenderEngine> D3D11RenderEnginePtr;
}

#endif			// _D3D11RENDERENGINE_HPP
