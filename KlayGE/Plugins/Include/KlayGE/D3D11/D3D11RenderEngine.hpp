// D3D11RenderEngine.hpp
// KlayGE D3D11渲染引擎类 头文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2009-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Remove TexelToPixelOffset (2010.9.26)
//
// 3.10.0
// 升级到DXGI 1.1 (2010.2.8)
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
#include <D3D11Shader.h>

#include <vector>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6385)
#endif
#include <boost/array.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011 6334)
#endif
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
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

		IDXGIFactory1Ptr const & DXGIFactory() const;
		ID3D11DevicePtr const & D3DDevice() const;
		ID3D11DeviceContextPtr const & D3DDeviceImmContext() const;
		D3D_FEATURE_LEVEL DeviceFeatureLevel() const;
		void D3DDevice(ID3D11DevicePtr const & device, ID3D11DeviceContextPtr const & imm_ctx, D3D_FEATURE_LEVEL feature_level);

		void StartRendering();

		void BeginFrame();
		void EndFrame();
		void BeginPass();
		void EndPass();

		void ForceFlush();

		uint16_t StencilBufferBitDepth();

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

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
		std::string const & ComputeShaderProfile() const
		{
			return cs_profile_;
		}
		std::string const & HullShaderProfile() const
		{
			return hs_profile_;
		}
		std::string const & DomainShaderProfile() const
		{
			return ds_profile_;
		}

		void RSSetState(ID3D11RasterizerStatePtr const & ras);
		void OMSetDepthStencilState(ID3D11DepthStencilStatePtr const & ds, uint16_t stencil_ref);
		void OMSetBlendState(ID3D11BlendStatePtr const & bs, Color const & blend_factor, uint32_t sample_mask);
		void VSSetShader(ID3D11VertexShaderPtr const & shader);
		void PSSetShader(ID3D11PixelShaderPtr const & shader);
		void GSSetShader(ID3D11GeometryShaderPtr const & shader);
		void CSSetShader(ID3D11ComputeShaderPtr const & shader);
		void HSSetShader(ID3D11HullShaderPtr const & shader);
		void DSSetShader(ID3D11DomainShaderPtr const & shader);
		void SetShaderResources(ShaderObject::ShaderType st, std::vector<ID3D11ShaderResourceView*> const & srvs);
		void SetSamplers(ShaderObject::ShaderType st, std::vector<ID3D11SamplerState*> const & samplers);
		void SetConstantBuffers(ShaderObject::ShaderType st, std::vector<ID3D11Buffer*> const & cbs);
		
		void ResetRenderStates();
		void DetachTextureByRTV(ID3D11RenderTargetView* rtv);

		ID3D11InputLayoutPtr const & CreateD3D11InputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> const & elems, size_t signature, ID3DBlobPtr const & vs_code);

	private:
		void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings);
		void DoBindFrameBuffer(FrameBufferPtr const & fb);
		void DoBindSOBuffers(RenderLayoutPtr const & rl);
		void DoRender(RenderTechnique const & tech, RenderLayout const & rl);
		void DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz);
		void DoResize(uint32_t width, uint32_t height);

		void FillRenderDeviceCaps();

		void StereoscopicForLCDShutter();

		bool VertexFormatSupport(ElementFormat elem_fmt);
		bool TextureFormatSupport(ElementFormat elem_fmt);

	private:
		D3D11AdapterList const & D3DAdapters() const;
		D3D11AdapterPtr const & ActiveAdapter() const;

		HMODULE mod_dxgi_;
		HMODULE mod_d3d11_;

		typedef HRESULT (WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
		typedef HRESULT (WINAPI *D3D11CreateDeviceAndSwapChainFunc)(IDXGIAdapter* pAdapter,
								D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
								D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
								DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
								ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

		CreateDXGIFactory1Func DynamicCreateDXGIFactory1_;
		D3D11CreateDeviceAndSwapChainFunc DynamicD3D11CreateDeviceAndSwapChain_;


		// Direct3D rendering device
		// Only created after top-level window created
		IDXGIFactory1Ptr	gi_factory_;
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
		ID3D11ComputeShaderPtr compute_shader_cache_;
		ID3D11HullShaderPtr hull_shader_cache_;
		ID3D11DomainShaderPtr domain_shader_cache_;
		RenderLayout::topology_type topology_type_cache_;
		ID3D11InputLayoutPtr input_layout_cache_;

		boost::array<std::vector<ID3D11ShaderResourceView*>, ShaderObject::ST_NumShaderTypes> shader_srv_cache_;
		boost::array<std::vector<ID3D11SamplerState*>, ShaderObject::ST_NumShaderTypes> shader_sampler_cache_;
		boost::array<std::vector<ID3D11Buffer*>, ShaderObject::ST_NumShaderTypes> shader_cb_cache_;

		boost::unordered_map<size_t, ID3D11InputLayoutPtr, boost::hash<size_t>, std::equal_to<size_t>,
			boost::fast_pool_allocator<std::pair<size_t, ID3D11InputLayoutPtr > > > input_layout_bank_;

		std::string vs_profile_, ps_profile_, gs_profile_, cs_profile_, hs_profile_, ds_profile_;

		TexturePtr stereo_lr_tex_;
	};

	typedef boost::shared_ptr<D3D11RenderEngine> D3D11RenderEnginePtr;
}

#endif			// _D3D11RENDERENGINE_HPP
