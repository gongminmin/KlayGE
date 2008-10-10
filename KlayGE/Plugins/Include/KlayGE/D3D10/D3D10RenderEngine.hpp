// D3D10RenderEngine.hpp
// KlayGE D3D10渲染引擎类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10RENDERENGINE_HPP
#define _D3D10RENDERENGINE_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D10
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/Vector.hpp>
#include <KlayGE/Color.hpp>

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>
#include <d3dx10.h>

#include <vector>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/D3D10/D3D10AdapterList.hpp>
#include <KlayGE/D3D10/D3D10Typedefs.hpp>
#include <KlayGE/D3D10/D3D10GraphicsBuffer.hpp>

namespace KlayGE
{
	class D3D10AdapterList;
	class D3D10Adapter;

	class D3D10RenderEngine : public RenderEngine
	{
	public:
		D3D10RenderEngine();
		~D3D10RenderEngine();

		std::wstring const & Name() const;

		IDXGIFactoryPtr const & DXGIFactory() const;
		ID3D10DevicePtr const & D3DDevice() const;
		void D3DDevice(ID3D10DevicePtr const & device);

		void CreateRenderWindow(std::string const & name, RenderSettings const & settings);

		void StartRendering();

		void BeginFrame();
		void EndFrame();

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

		HRESULT D3D10CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter,
								D3D10_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, UINT SDKVersion,
								DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
								ID3D10Device** ppDevice) const
		{
			return DynamicD3D10CreateDeviceAndSwapChain_(pAdapter, DriverType, Software, Flags, SDKVersion,
				pSwapChainDesc, ppSwapChain, ppDevice);
		}
		LPCSTR D3D10GetVertexShaderProfile(ID3D10Device* pDevice) const
		{
			return DynamicD3D10GetVertexShaderProfile_(pDevice);
		}
		LPCSTR D3D10GetPixelShaderProfile(ID3D10Device* pDevice) const
		{
			return DynamicD3D10GetPixelShaderProfile_(pDevice);
		}

		void RSSetState(ID3D10RasterizerStatePtr const & ras);
		void OMSetDepthStencilState(ID3D10DepthStencilStatePtr const & ds, uint16_t stencil_ref);
		void OMSetBlendState(ID3D10BlendStatePtr const & bs, Color const & blend_factor, uint32_t sample_mask);
		void VSSetShader(ID3D10VertexShaderPtr const & shader);
		void PSSetShader(ID3D10PixelShaderPtr const & shader);

	private:
		void DoBindFrameBuffer(FrameBufferPtr fb);
		void DoRender(RenderTechnique const & tech, RenderLayout const & rl);

		void FillRenderDeviceCaps();
		void InitRenderStates();

		void RenderRL(RenderTechnique const & tech, RenderLayout const & rl);

	private:
		D3D10AdapterList const & D3DAdapters() const;
		D3D10AdapterPtr const & ActiveAdapter() const;

		HMODULE mod_dxgi_;
		HMODULE mod_d3d10_;

		typedef HRESULT (WINAPI *CreateDXGIFactoryFunc)(REFIID riid, void** ppFactory);
		typedef HRESULT (WINAPI *D3D10CreateDeviceAndSwapChainFunc)(IDXGIAdapter* pAdapter,
								D3D10_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, UINT SDKVersion,
								DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
								ID3D10Device** ppDevice);
		typedef LPCSTR (WINAPI *D3D10GetVertexShaderProfileFunc)(ID3D10Device* pDevice);
		typedef LPCSTR (WINAPI *D3D10GetPixelShaderProfileFunc)(ID3D10Device* pDevice);

		CreateDXGIFactoryFunc DynamicCreateDXGIFactory_;
		D3D10CreateDeviceAndSwapChainFunc DynamicD3D10CreateDeviceAndSwapChain_;
		D3D10GetVertexShaderProfileFunc DynamicD3D10GetVertexShaderProfile_;
		D3D10GetPixelShaderProfileFunc DynamicD3D10GetPixelShaderProfile_;


		// Direct3D rendering device
		// Only created after top-level window created
		IDXGIFactoryPtr		gi_factory_;
		ID3D10DevicePtr		d3d_device_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D10AdapterList adapterList_;

		ID3D10RasterizerStatePtr rasterizer_state_cache_;
		ID3D10DepthStencilStatePtr depth_stencil_state_cache_;
		uint16_t stencil_ref_cache_;
		ID3D10BlendStatePtr blend_state_cache_;
		Color blend_factor_cache_;
		uint32_t sample_mask_cache_;
		ID3D10VertexShaderPtr vertex_shader_cache_;
		ID3D10PixelShaderPtr pixel_shader_cache_;
	};

	typedef boost::shared_ptr<D3D10RenderEngine> D3D10RenderEnginePtr;
}

#endif			// _D3D10RENDERENGINE_HPP
