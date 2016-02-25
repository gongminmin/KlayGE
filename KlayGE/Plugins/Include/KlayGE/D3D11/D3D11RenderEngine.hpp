// D3D11RenderEngine.hpp
// KlayGE D3D11��Ⱦ������ ͷ�ļ�
// Ver 3.11.0
// ��Ȩ����(C) ������, 2009-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Remove TexelToPixelOffset (2010.9.26)
//
// 3.10.0
// ������DXGI 1.1 (2010.2.8)
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11RENDERENGINE_HPP
#define _D3D11RENDERENGINE_HPP

#pragma once

#include <KFL/Vector.hpp>
#include <KFL/Color.hpp>

#include <vector>
#include <set>
#include <map>
#include <unordered_map>

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

		bool RequiresFlipping() const
		{
			return true;
		}

		void BeginFrame() override;
		void EndFrame() override;
		void UpdateGPUTimestampsFrequency() override;

		IDXGIFactory1* DXGIFactory1() const;
		IDXGIFactory2* DXGIFactory2() const;
		IDXGIFactory3* DXGIFactory3() const;
		IDXGIFactory4* DXGIFactory4() const;
		uint8_t DXGISubVer() const;

		ID3D11Device* D3DDevice() const;
		ID3D11Device1* D3DDevice1() const;
		ID3D11Device2* D3DDevice2() const;
		ID3D11Device3* D3DDevice3() const;
		ID3D11DeviceContext* D3DDeviceImmContext() const;
		ID3D11DeviceContext1* D3DDeviceImmContext1() const;
		ID3D11DeviceContext2* D3DDeviceImmContext2() const;
		ID3D11DeviceContext3* D3DDeviceImmContext3() const;
		uint8_t D3D11RuntimeSubVer() const;

		D3D_FEATURE_LEVEL DeviceFeatureLevel() const;

		void D3DDevice(ID3D11Device* device, ID3D11DeviceContext* imm_ctx, D3D_FEATURE_LEVEL feature_level);

		void ForceFlush();

		virtual TexturePtr const & ScreenDepthStencilTexture() const override;

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		virtual void GetCustomAttrib(std::string const & name, void* value) override;

		bool FullScreen() const;
		void FullScreen(bool fs);

		char const * VertexShaderProfile() const
		{
			return vs_profile_;
		}
		char const * PixelShaderProfile() const
		{
			return ps_profile_;
		}
		char const * GeometryShaderProfile() const
		{
			return gs_profile_;
		}
		char const * ComputeShaderProfile() const
		{
			return cs_profile_;
		}
		char const * HullShaderProfile() const
		{
			return hs_profile_;
		}
		char const * DomainShaderProfile() const
		{
			return ds_profile_;
		}

		double InvTimestampFreq() const
		{
			return inv_timestamp_freq_;
		}

		void RSSetState(ID3D11RasterizerState* ras);
		void OMSetDepthStencilState(ID3D11DepthStencilState* ds, uint16_t stencil_ref);
		void OMSetBlendState(ID3D11BlendState* bs, Color const & blend_factor, uint32_t sample_mask);
		void VSSetShader(ID3D11VertexShader* shader);
		void PSSetShader(ID3D11PixelShader* shader);
		void GSSetShader(ID3D11GeometryShader* shader);
		void CSSetShader(ID3D11ComputeShader* shader);
		void HSSetShader(ID3D11HullShader* shader);
		void DSSetShader(ID3D11DomainShader* shader);
		void SetShaderResources(ShaderObject::ShaderType st, std::vector<std::tuple<void*, uint32_t, uint32_t>> const & srvsrcs, std::vector<ID3D11ShaderResourceView*> const & srvs);
		void SetSamplers(ShaderObject::ShaderType st, std::vector<ID3D11SamplerState*> const & samplers);
		void SetConstantBuffers(ShaderObject::ShaderType st, std::vector<ID3D11Buffer*> const & cbs);
		void RSSetViewports(UINT NumViewports, D3D11_VIEWPORT const * pViewports);
		
		void ResetRenderStates();
		void DetachSRV(void* rtv_src, uint32_t rt_first_subres, uint32_t rt_num_subres);

		ID3D11InputLayoutPtr const & CreateD3D11InputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> const & elems, size_t signature, std::vector<uint8_t> const & vs_code);

		HRESULT D3D11CreateDevice(IDXGIAdapter* pAdapter,
								D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
								D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
								ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const;

	private:
		virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) override;
		virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) override;
		virtual void DoBindSOBuffers(RenderLayoutPtr const & rl) override;
		virtual void DoRender(RenderTechnique const & tech, RenderLayout const & rl) override;
		virtual void DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz) override;
		virtual void DoDispatchIndirect(RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) override;
		virtual void DoResize(uint32_t width, uint32_t height) override;
		virtual void DoDestroy() override;
		virtual void DoSuspend() override;
		virtual void DoResume() override;

		void FillRenderDeviceCaps();
		void DetectD3D11Runtime(ID3D11Device* device, ID3D11DeviceContext* imm_ctx);

		virtual void StereoscopicForLCDShutter(int32_t eye) override;

		bool VertexFormatSupport(ElementFormat elem_fmt);
		bool TextureFormatSupport(ElementFormat elem_fmt);
		bool RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality);

		virtual void CheckConfig(RenderSettings& settings) override;

	private:
		D3D11AdapterList const & D3DAdapters() const;
		D3D11AdapterPtr const & ActiveAdapter() const;

		typedef HRESULT(WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
		typedef HRESULT(WINAPI *D3D11CreateDeviceFunc)(IDXGIAdapter* pAdapter,
			D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
			D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
			ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

		CreateDXGIFactory1Func DynamicCreateDXGIFactory1_;
		D3D11CreateDeviceFunc DynamicD3D11CreateDevice_;

		HMODULE mod_dxgi_;
		HMODULE mod_d3d11_;

		IDXGIFactory1Ptr gi_factory_1_;
		IDXGIFactory2Ptr gi_factory_2_;
		IDXGIFactory3Ptr gi_factory_3_;
		IDXGIFactory4Ptr gi_factory_4_;
		uint8_t dxgi_sub_ver_;
		
		ID3D11DevicePtr  d3d_device_;
		ID3D11Device1Ptr d3d_device_1_;
		ID3D11Device2Ptr d3d_device_2_;
		ID3D11Device3Ptr d3d_device_3_;
		ID3D11DeviceContextPtr  d3d_imm_ctx_;
		ID3D11DeviceContext1Ptr d3d_imm_ctx_1_;
		ID3D11DeviceContext2Ptr d3d_imm_ctx_2_;
		ID3D11DeviceContext3Ptr d3d_imm_ctx_3_;
		uint8_t d3d_11_runtime_sub_ver_;

		D3D_FEATURE_LEVEL d3d_feature_level_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D11AdapterList adapterList_;

		ID3D11RasterizerState* rasterizer_state_cache_;
		ID3D11DepthStencilState* depth_stencil_state_cache_;
		uint16_t stencil_ref_cache_;
		ID3D11BlendState* blend_state_cache_;
		Color blend_factor_cache_;
		uint32_t sample_mask_cache_;
		ID3D11VertexShader* vertex_shader_cache_;
		ID3D11PixelShader* pixel_shader_cache_;
		ID3D11GeometryShader* geometry_shader_cache_;
		ID3D11ComputeShader* compute_shader_cache_;
		ID3D11HullShader* hull_shader_cache_;
		ID3D11DomainShader* domain_shader_cache_;
		RenderLayout::topology_type topology_type_cache_;
		ID3D11InputLayout* input_layout_cache_;
		D3D11_VIEWPORT viewport_cache_;
		uint32_t num_so_buffs_;
		std::vector<ID3D11Buffer*> vb_cache_;
		std::vector<UINT> vb_stride_cache_;
		std::vector<UINT> vb_offset_cache_;
		ID3D11Buffer* ib_cache_;

		std::array<std::vector<std::tuple<void*, uint32_t, uint32_t>>, ShaderObject::ST_NumShaderTypes> shader_srvsrc_cache_;
		std::array<std::vector<ID3D11ShaderResourceView*>, ShaderObject::ST_NumShaderTypes> shader_srv_ptr_cache_;
		std::array<std::vector<ID3D11SamplerState*>, ShaderObject::ST_NumShaderTypes> shader_sampler_ptr_cache_;
		std::array<std::vector<ID3D11Buffer*>, ShaderObject::ST_NumShaderTypes> shader_cb_ptr_cache_;
		std::array<std::vector<ID3D11ShaderResourceView*>, ShaderObject::ST_NumShaderTypes> shader_srv_cache_;
		std::array<std::vector<ID3D11SamplerState*>, ShaderObject::ST_NumShaderTypes> shader_sampler_cache_;
		std::array<std::vector<ID3D11Buffer*>, ShaderObject::ST_NumShaderTypes> shader_cb_cache_;

		std::unordered_map<size_t, ID3D11InputLayoutPtr> input_layout_bank_;

		char const * vs_profile_;
		char const * ps_profile_;
		char const * gs_profile_;
		char const * cs_profile_;
		char const * hs_profile_;
		char const * ds_profile_;

		enum StereoMethod
		{
			SM_None,
			SM_DXGI,
			SM_NV3DVision,
			SM_AMDQuadBuffer
		};

		StereoMethod stereo_method_;
		FrameBufferPtr stereo_nv_3d_vision_fb_;
		TexturePtr stereo_nv_3d_vision_tex_;

		std::set<ElementFormat> vertex_format_;
		std::set<ElementFormat> texture_format_;
		std::map<ElementFormat, std::vector<std::pair<uint32_t, uint32_t>>> rendertarget_format_;

		ID3D11QueryPtr timestamp_disjoint_query_;
		double inv_timestamp_freq_;
	};
}

#endif			// _D3D11RENDERENGINE_HPP
