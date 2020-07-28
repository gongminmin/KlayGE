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

#include <KFL/Color.hpp>
#include <KFL/Vector.hpp>
#include <KFL/SmartPtrHelper.hpp>

#include <vector>
#include <map>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/D3D11/D3D11AdapterList.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>

namespace KlayGE
{
	class D3D11AdapterList;
	class D3D11Adapter;

	class D3D11RenderEngine final : public RenderEngine
	{
	public:
		D3D11RenderEngine();
		~D3D11RenderEngine() override;

		std::wstring const & Name() const override;

		bool RequiresFlipping() const override
		{
			return true;
		}

		void BeginFrame() override;
		void EndFrame() override;

		IDXGIFactory2* DXGIFactory2() const noexcept;
		IDXGIFactory3* DXGIFactory3() const noexcept;
		IDXGIFactory4* DXGIFactory4() const noexcept;
		IDXGIFactory5* DXGIFactory5() const noexcept;
		IDXGIFactory6* DXGIFactory6() const noexcept;
		uint8_t DXGISubVer() const noexcept;

		ID3D11Device1* D3DDevice1() const noexcept;
		ID3D11Device2* D3DDevice2() const noexcept;
		ID3D11Device3* D3DDevice3() const noexcept;
		ID3D11Device4* D3DDevice4() const noexcept;
		ID3D11Device5* D3DDevice5() const noexcept;
		ID3D11DeviceContext1* D3DDeviceImmContext1() const noexcept;
		ID3D11DeviceContext2* D3DDeviceImmContext2() const noexcept;
		ID3D11DeviceContext3* D3DDeviceImmContext3() const noexcept;
		ID3D11DeviceContext4* D3DDeviceImmContext4() const noexcept;
		uint8_t D3D11RuntimeSubVer() const noexcept;

		D3D_FEATURE_LEVEL DeviceFeatureLevel() const noexcept;

		void D3DDevice(ID3D11Device1* device, ID3D11DeviceContext1* imm_ctx, D3D_FEATURE_LEVEL feature_level);

		void ForceFlush() override;

		TexturePtr const & ScreenDepthStencilTexture() const override;

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void GetCustomAttrib(std::string_view name, void* value) const override;

		bool FullScreen() const override;
		void FullScreen(bool fs) override;

		char const * DefaultShaderProfile(ShaderStage stage) const
		{
			return shader_profiles_[static_cast<uint32_t>(stage)];
		}

		void RSSetState(ID3D11RasterizerState1* ras);
		void OMSetDepthStencilState(ID3D11DepthStencilState* ds, uint16_t stencil_ref);
		void OMSetBlendState(ID3D11BlendState1* bs, Color const & blend_factor, uint32_t sample_mask);
		void VSSetShader(ID3D11VertexShader* shader);
		void PSSetShader(ID3D11PixelShader* shader);
		void GSSetShader(ID3D11GeometryShader* shader);
		void CSSetShader(ID3D11ComputeShader* shader);
		void HSSetShader(ID3D11HullShader* shader);
		void DSSetShader(ID3D11DomainShader* shader);
		void SetShaderResources(
			ShaderStage stage, std::span<std::tuple<void*, uint32_t, uint32_t> const> srvsrcs, std::span<ID3D11ShaderResourceView* const> srvs);
		void SetSamplers(ShaderStage stage, std::span<ID3D11SamplerState* const> samplers);
		void SetConstantBuffers(ShaderStage stage, std::span<ID3D11Buffer* const> cbs);
		void RSSetViewports(UINT NumViewports, D3D11_VIEWPORT const * pViewports);
		void OMSetRenderTargets(UINT num_rtvs, ID3D11RenderTargetView* const * rtvs, ID3D11DepthStencilView* dsv);
		void OMSetRenderTargetsAndUnorderedAccessViews(UINT num_rtvs, ID3D11RenderTargetView* const * rtvs,
			ID3D11DepthStencilView* dsv, 
			UINT uav_start_slot, UINT num_uavs, ID3D11UnorderedAccessView* const * uavs, UINT const * uav_init_counts);
		void CSSetUnorderedAccessViews(UINT start_slot, UINT num_uavs, ID3D11UnorderedAccessView* const * uavs,
			UINT const * uav_init_counts);
		
		void ResetRenderStates();
		void DetachSRV(void* rtv_src, uint32_t rt_first_subres, uint32_t rt_num_subres);
		void InvalidRTVCache();

		HRESULT D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
			D3D_FEATURE_LEVEL const* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice,
			D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const;

	private:
		void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) override;
		void DoBindFrameBuffer(FrameBufferPtr const & fb) override;
		void DoBindSOBuffers(RenderLayoutPtr const & rl) override;
		void DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl) override;
		void DoDispatch(RenderEffect const & effect, RenderTechnique const & tech,
			uint32_t tgx, uint32_t tgy, uint32_t tgz) override;
		void DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) override;
		void DoResize(uint32_t width, uint32_t height) override;
		void DoDestroy() override;
		void DoSuspend() override;
		void DoResume() override;

		void FillRenderDeviceCaps();
		void DetectD3D11Runtime(ID3D11Device1* device, ID3D11DeviceContext1* imm_ctx);

		void StereoscopicForLCDShutter(int32_t eye) override;

		void CheckConfig(RenderSettings& settings) override;

		D3D11AdapterList const & D3DAdapters() const noexcept;
		D3D11Adapter& ActiveAdapter() const;

		static void CALLBACK OnDeviceLost(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WAIT wait, TP_WAIT_RESULT wait_result);

	private:
		typedef HRESULT(WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
		typedef HRESULT(WINAPI *CreateDXGIFactory2Func)(UINT flags, REFIID riid, void** ppFactory);
		typedef HRESULT(WINAPI *D3D11CreateDeviceFunc)(IDXGIAdapter* pAdapter,
			D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
			D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
			ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

		CreateDXGIFactory1Func DynamicCreateDXGIFactory1_;
		CreateDXGIFactory2Func DynamicCreateDXGIFactory2_;
		D3D11CreateDeviceFunc DynamicD3D11CreateDevice_;

		HMODULE mod_dxgi_;
		HMODULE mod_d3d11_;

		IDXGIFactory2Ptr gi_factory_2_;
		IDXGIFactory3Ptr gi_factory_3_;
		IDXGIFactory4Ptr gi_factory_4_;
		IDXGIFactory5Ptr gi_factory_5_;
		IDXGIFactory6Ptr gi_factory_6_;
		uint8_t dxgi_sub_ver_;

		ID3D11Device1Ptr d3d_device_1_;
		ID3D11Device2Ptr d3d_device_2_;
		ID3D11Device3Ptr d3d_device_3_;
		ID3D11Device4Ptr d3d_device_4_;
		ID3D11Device5Ptr d3d_device_5_;
		ID3D11DeviceContext1Ptr d3d_imm_ctx_1_;
		ID3D11DeviceContext2Ptr d3d_imm_ctx_2_;
		ID3D11DeviceContext3Ptr d3d_imm_ctx_3_;
		ID3D11DeviceContext4Ptr d3d_imm_ctx_4_;
		uint8_t d3d_11_runtime_sub_ver_;

		D3D_FEATURE_LEVEL d3d_feature_level_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D11AdapterList adapterList_;

		ID3D11RasterizerState1* rasterizer_state_cache_{nullptr};
		ID3D11DepthStencilState* depth_stencil_state_cache_{nullptr};
		uint16_t stencil_ref_cache_{0};
		ID3D11BlendState1* blend_state_cache_{nullptr};
		Color blend_factor_cache_{1, 1, 1, 1};
		uint32_t sample_mask_cache_{0xFFFFFFFF};
		ID3D11VertexShader* vertex_shader_cache_{nullptr};
		ID3D11PixelShader* pixel_shader_cache_{nullptr};
		ID3D11GeometryShader* geometry_shader_cache_{nullptr};
		ID3D11ComputeShader* compute_shader_cache_{nullptr};
		ID3D11HullShader* hull_shader_cache_{nullptr};
		ID3D11DomainShader* domain_shader_cache_{nullptr};
		RenderLayout::topology_type topology_type_cache_{RenderLayout::TT_PointList};
		ID3D11InputLayout* input_layout_cache_{nullptr};
		D3D11_VIEWPORT viewport_cache_{};
		uint32_t num_so_buffs_{0};
		std::vector<ID3D11Buffer*> vb_cache_;
		std::vector<UINT> vb_stride_cache_;
		std::vector<UINT> vb_offset_cache_;
		ID3D11Buffer* ib_cache_{nullptr};

		std::array<std::vector<std::tuple<void*, uint32_t, uint32_t>>, NumShaderStages> shader_srvsrc_cache_;
		std::array<std::vector<ID3D11ShaderResourceView*>, NumShaderStages> shader_srv_ptr_cache_;
		std::array<std::vector<ID3D11SamplerState*>, NumShaderStages> shader_sampler_ptr_cache_;
		std::array<std::vector<ID3D11Buffer*>, NumShaderStages> shader_cb_ptr_cache_;
		std::vector<ID3D11UnorderedAccessView*> render_uav_ptr_cache_;
		std::vector<uint32_t> render_uav_init_count_cache_;
		std::vector<ID3D11UnorderedAccessView*> compute_uav_ptr_cache_;
		std::vector<uint32_t> compute_uav_init_count_cache_;
		std::vector<ID3D11RenderTargetView*> rtv_ptr_cache_;
		ID3D11DepthStencilView* dsv_ptr_cache_{nullptr};

		char const* shader_profiles_[NumShaderStages];

		Win32UniqueHandle device_lost_event_;
		DWORD device_lost_reg_cookie_{0};
		Win32UniqueTpWait thread_pool_wait_;
	};
}

#endif			// _D3D11RENDERENGINE_HPP
