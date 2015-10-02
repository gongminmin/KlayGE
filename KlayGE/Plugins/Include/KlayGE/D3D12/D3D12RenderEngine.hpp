/**
 * @file D3D12RenderEngine.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _D3D12RENDERENGINE_HPP
#define _D3D12RENDERENGINE_HPP

#pragma once

#include <KFL/Vector.hpp>
#include <KFL/Color.hpp>
#include <KFL/Thread.hpp>

#include <KlayGE/D3D12/D3D12MinGWDefs.hpp>
#include <D3D11Shader.h>

#include <vector>
#include <set>
#include <map>
#include <unordered_map>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/D3D12/D3D12AdapterList.hpp>
#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12GraphicsBuffer.hpp>

namespace KlayGE
{
	class D3D12AdapterList;
	class D3D12Adapter;

	static uint32_t const NUM_BACK_BUFFERS = 3;
	static uint32_t const NUM_MAX_RENDER_TARGET_VIEWS = 8 * 1024;
	static uint32_t const NUM_MAX_DEPTH_STENCIL_VIEWS = 4 * 1024;
	static uint32_t const NUM_MAX_CBV_SRV_UAVS = 32 * 1024;

	inline uint32_t CalcSubresource(uint32_t MipSlice, uint32_t ArraySlice, uint32_t PlaneSlice, uint32_t MipLevels, uint32_t ArraySize)
	{
		return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
	}

	class D3D12RenderEngine : public RenderEngine
	{
	public:
		D3D12RenderEngine();
		~D3D12RenderEngine();

		std::wstring const & Name() const;

		bool RequiresFlipping() const
		{
			return true;
		}

		void BeginFrame() KLAYGE_OVERRIDE;
		void UpdateGPUTimestampsFrequency() KLAYGE_OVERRIDE;

		IDXGIFactory4Ptr const & DXGIFactory() const;
		ID3D12DevicePtr const & D3D12Device() const;
		ID3D12CommandQueuePtr const & D3D12GraphicsCmdQueue() const;
		ID3D12CommandAllocatorPtr const & D3D12GraphicsCmdAllocator() const;
		ID3D12GraphicsCommandListPtr const & D3D12GraphicsCmdList() const;
		ID3D12CommandQueuePtr const & D3D12ComputeCmdQueue() const;
		ID3D12CommandAllocatorPtr const & D3D12ComputeCmdAllocator() const;
		ID3D12GraphicsCommandListPtr const & D3D12ComputeCmdList() const;
		ID3D12CommandAllocatorPtr const & D3D12ResCmdAllocator() const;
		ID3D12GraphicsCommandListPtr const & D3D12ResCmdList() const;
		std::mutex& D3D12ResCmdListMutex()
		{
			return res_cmd_list_mutex_;
		}
		D3D_FEATURE_LEVEL DeviceFeatureLevel() const;
		void D3DDevice(ID3D12DevicePtr const & device_12, ID3D12CommandQueuePtr const & cmd_queue, D3D_FEATURE_LEVEL feature_level);
		void ClearPSOCache();
		void CommitResCmd();
		void CommitGraphicsCmd();
		void CommitComputeCmd();
		void ResetGraphicsCmd();
		void ResetComputeCmd();

		void ForceFlush();
		void ForceCPUGPUSync();

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		bool FullScreen() const;
		void FullScreen(bool fs);

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

		double InvTimestampFreq() const
		{
			return inv_timestamp_freq_;
		}

		void OMSetStencilRef(uint16_t stencil_ref);
		void OMSetBlendState(Color const & blend_factor, uint32_t sample_mask);
		void RSSetViewports(UINT NumViewports, D3D12_VIEWPORT const * pViewports);
		
		void ResetRenderStates();

		D3D12_SHADER_RESOURCE_VIEW_DESC const & NullSRVDesc() const;
		D3D12_UNORDERED_ACCESS_VIEW_DESC const & NullUAVDesc() const;
		D3D12_RENDER_TARGET_VIEW_DESC const & NullRTVDesc() const;
		D3D12_DEPTH_STENCIL_VIEW_DESC const & NullDSVDesc() const;

		ID3D12DescriptorHeapPtr const & RTVDescHeap() const
		{
			return rtv_desc_heap_;
		}
		ID3D12DescriptorHeapPtr const & DSVDescHeap() const
		{
			return dsv_desc_heap_;
		}
		ID3D12DescriptorHeapPtr const & CBVSRVUAVDescHeap() const
		{
			return cbv_srv_uav_desc_heap_;
		}
		uint32_t RTVDescSize() const
		{
			return rtv_desc_size_;
		}
		uint32_t DSVDescSize() const
		{
			return dsv_desc_size_;
		}
		uint32_t CBVSRVUAVDescSize() const
		{
			return cbv_srv_uav_desc_size_;
		}

		uint32_t AllocRTV();
		uint32_t AllocDSV();
		uint32_t AllocCBVSRVUAV();
		void DeallocRTV(uint32_t offset);
		void DeallocDSV(uint32_t offset);
		void DeallocCBVSRVUAV(uint32_t offset);

		RenderTechniquePtr BilinearBlitTech() const
		{
			return bilinear_blit_tech_;
		}
		RenderLayoutPtr BlitRL() const
		{
			return blit_rl_;
		}

		ID3D12RootSignaturePtr const & CreateRootSignature(
			std::array<size_t, ShaderObject::ST_NumShaderTypes * 4> const & num,
			bool has_vs, bool has_stream_output);
		ID3D12PipelineStatePtr const & CreateGraphicsPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC const & desc);
		ID3D12PipelineStatePtr const & CreateComputePSO(D3D12_COMPUTE_PIPELINE_STATE_DESC const & desc);
		ID3D12DescriptorHeapPtr CreateDynamicCBVSRVUAVDescriptorHeap(uint32_t num);

	private:
		virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) KLAYGE_OVERRIDE;
		virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) KLAYGE_OVERRIDE;
		virtual void DoBindSOBuffers(RenderLayoutPtr const & rl) KLAYGE_OVERRIDE;
		virtual void DoRender(RenderTechnique const & tech, RenderLayout const & rl) KLAYGE_OVERRIDE;
		virtual void DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz) KLAYGE_OVERRIDE;
		virtual void DoDispatchIndirect(RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) KLAYGE_OVERRIDE;
		virtual void DoResize(uint32_t width, uint32_t height) KLAYGE_OVERRIDE;
		virtual void DoDestroy() KLAYGE_OVERRIDE;
		virtual void DoSuspend() KLAYGE_OVERRIDE;
		virtual void DoResume() KLAYGE_OVERRIDE;

		void FillRenderDeviceCaps();

		virtual void StereoscopicForLCDShutter(int32_t eye) KLAYGE_OVERRIDE;

		bool VertexFormatSupport(ElementFormat elem_fmt);
		bool TextureFormatSupport(ElementFormat elem_fmt);
		bool RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality);

		virtual void CheckConfig(RenderSettings& settings) KLAYGE_OVERRIDE;

		void UpdateGraphicsPSO(RenderTechnique const & tech, RenderPassPtr const & pass, RenderLayout const & rl);
		void UpdateComputePSO(RenderPassPtr const & pass);

	private:
		D3D12AdapterList const & D3DAdapters() const;
		D3D12AdapterPtr const & ActiveAdapter() const;

		// Direct3D rendering device
		// Only created after top-level window created
		IDXGIFactory4Ptr	gi_factory_;
		ID3D12DevicePtr		d3d_12_device_;
		ID3D12CommandQueuePtr d3d_graphics_cmd_queue_;
		ID3D12CommandAllocatorPtr d3d_graphics_cmd_allocator_;
		ID3D12GraphicsCommandListPtr d3d_graphics_cmd_list_;
		ID3D12CommandQueuePtr d3d_compute_cmd_queue_;
		ID3D12CommandAllocatorPtr d3d_compute_cmd_allocator_;
		ID3D12GraphicsCommandListPtr d3d_compute_cmd_list_;
		ID3D12CommandAllocatorPtr d3d_res_cmd_allocator_;
		ID3D12GraphicsCommandListPtr d3d_res_cmd_list_;
		std::mutex res_cmd_list_mutex_;
		D3D_FEATURE_LEVEL d3d_feature_level_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D12AdapterList adapterList_;

		uint16_t stencil_ref_cache_;
		Color blend_factor_cache_;
		uint32_t sample_mask_cache_;
		RenderLayout::topology_type topology_type_cache_;
		D3D12_VIEWPORT viewport_cache_;
		D3D12_RECT scissor_rc_cache_;
		std::vector<ID3D12ResourcePtr> so_buffs_;
		std::set<ID3D12ResourcePtr> buff_cache_;
		std::vector<ID3D12PipelineStatePtr> pso_cache_;
		std::vector<ID3D12DescriptorHeapPtr> cbv_srv_uav_heap_cache_;
		std::unordered_map<size_t, ID3D12RootSignaturePtr> root_signatures_;
		std::unordered_map<size_t, ID3D12PipelineStatePtr> graphics_psos_;
		std::unordered_map<size_t, ID3D12PipelineStatePtr> compute_psos_;
		std::unordered_map<size_t, ID3D12DescriptorHeapPtr> cbv_srv_uav_heaps_;

		ID3D12DescriptorHeapPtr rtv_desc_heap_;
		uint32_t rtv_desc_size_;
		ID3D12DescriptorHeapPtr dsv_desc_heap_;
		uint32_t dsv_desc_size_;
		ID3D12DescriptorHeapPtr cbv_srv_uav_desc_heap_;
		uint32_t cbv_srv_uav_desc_size_;
		std::vector<bool> rtv_heap_occupied_;
		std::vector<bool> dsv_heap_occupied_;
		std::vector<bool> cbv_srv_uav_heap_occupied_;

		D3D12_CPU_DESCRIPTOR_HANDLE null_srv_handle_;
		D3D12_CPU_DESCRIPTOR_HANDLE null_uav_handle_;

		std::string vs_profile_, ps_profile_, gs_profile_, cs_profile_, hs_profile_, ds_profile_;

		enum StereoMethod
		{
			SM_None,
			SM_DXGI
		};

		StereoMethod stereo_method_;

		std::set<ElementFormat> vertex_format_;
		std::set<ElementFormat> texture_format_;
		std::map<ElementFormat, std::vector<std::pair<uint32_t, uint32_t>>> rendertarget_format_;

		double inv_timestamp_freq_;

		ID3D12FencePtr res_cmd_fence_;
		uint64_t res_cmd_fence_val_;
		HANDLE res_cmd_fence_event_;

		ID3D12FencePtr graphics_cmd_fence_;
		uint64_t graphics_cmd_fence_val_;
		HANDLE graphics_cmd_fence_event_;

		ID3D12FencePtr compute_cmd_fence_;
		uint64_t compute_cmd_fence_val_;
		HANDLE compute_cmd_fence_event_;

		RenderEffectPtr blit_effect_;
		RenderTechniquePtr bilinear_blit_tech_;
		RenderLayoutPtr blit_rl_;
		GraphicsBufferPtr blit_vb_;
	};

	typedef std::shared_ptr<D3D12RenderEngine> D3D12RenderEnginePtr;
}

#endif			// _D3D12RENDERENGINE_HPP
