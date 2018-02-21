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

#include <bitset>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

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

		void BeginFrame() override;
		void EndFrame() override;
		void UpdateGPUTimestampsFrequency() override;

		IDXGIFactory4* DXGIFactory4() const;
		IDXGIFactory5* DXGIFactory5() const;
		uint8_t DXGISubVer() const;

		ID3D12Device* D3DDevice() const;
		ID3D12CommandQueue* D3DRenderCmdQueue() const;
		ID3D12CommandAllocator* D3DRenderCmdAllocator() const;
		ID3D12GraphicsCommandList* D3DRenderCmdList() const;
		ID3D12CommandAllocator* D3DResCmdAllocator() const;
		ID3D12GraphicsCommandList* D3DResCmdList() const;
		std::mutex& D3DResCmdListMutex()
		{
			return res_cmd_list_mutex_;
		}
		D3D_FEATURE_LEVEL DeviceFeatureLevel() const;
		void D3DDevice(ID3D12Device* device, ID3D12CommandQueue* cmd_queue, D3D_FEATURE_LEVEL feature_level);
		void ClearTempObjs();
		void CommitRenderCmd();
		void SyncRenderCmd();
		void ResetRenderCmd();
		void CommitResCmd();

		void ForceFlush();
		void ForceFinish();

		virtual TexturePtr const & ScreenDepthStencilTexture() const override;

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

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

		void OMSetStencilRef(uint16_t stencil_ref);
		void OMSetBlendFactor(Color const & blend_factor);
		void RSSetViewports(UINT NumViewports, D3D12_VIEWPORT const * pViewports);
		void SetPipelineState(ID3D12PipelineState* pso);
		void SetGraphicsRootSignature(ID3D12RootSignature* root_signature);
		void SetComputeRootSignature(ID3D12RootSignature* root_signature);
		void RSSetScissorRects(D3D12_RECT const & rect);
		void IASetPrimitiveTopology(RenderLayout::topology_type primitive_topology);
		void SetDescriptorHeaps(ArrayRef<ID3D12DescriptorHeap*> descriptor_heaps);
		void IASetVertexBuffers(uint32_t start_slot, ArrayRef<D3D12_VERTEX_BUFFER_VIEW> views);
		void IASetIndexBuffer(D3D12_INDEX_BUFFER_VIEW const & view);
		
		void ResetRenderStates();

		ID3D12DescriptorHeap* RTVDescHeap() const
		{
			return rtv_desc_heap_.get();
		}
		ID3D12DescriptorHeap* DSVDescHeap() const
		{
			return dsv_desc_heap_.get();
		}
		ID3D12DescriptorHeap* CBVSRVUAVDescHeap() const
		{
			return cbv_srv_uav_desc_heap_.get();
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
		uint32_t SamplerDescSize() const
		{
			return sampler_desc_size_;
		}

		uint32_t AllocRTV();
		uint32_t AllocDSV();
		uint32_t AllocCBVSRVUAV();
		void DeallocRTV(uint32_t offset);
		void DeallocDSV(uint32_t offset);
		void DeallocCBVSRVUAV(uint32_t offset);

		RenderEffectPtr const & BlitEffect() const
		{
			return blit_effect_;
		}
		RenderTechnique* BilinearBlitTech() const
		{
			return bilinear_blit_tech_;
		}

		ID3D12RootSignaturePtr const & CreateRootSignature(
			std::array<uint32_t, ShaderObject::ST_NumShaderTypes * 4> const & num,
			bool has_vs, bool has_stream_output);
		ID3D12PipelineStatePtr const & CreateRenderPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC const & desc);
		ID3D12PipelineStatePtr const & CreateComputePSO(D3D12_COMPUTE_PIPELINE_STATE_DESC const & desc);
		ID3D12DescriptorHeapPtr CreateDynamicCBVSRVUAVDescriptorHeap(uint32_t num);

		ID3D12ResourcePtr AllocTempBuffer(bool is_upload, uint32_t size_in_byte);
		void RecycleTempBuffer(ID3D12ResourcePtr const & buff, bool is_upload, uint32_t size_in_byte);

		void ReleaseAfterSync(ID3D12ResourcePtr const & buff);

	private:
		struct CmdAllocatorDependencies
		{
			ID3D12CommandAllocatorPtr cmd_allocator;
			std::vector<ID3D12DescriptorHeapPtr> cbv_srv_uav_heap_cache_;
			std::vector<std::pair<ID3D12ResourcePtr, uint32_t>> recycle_after_sync_upload_buffs_;
			std::vector<std::pair<ID3D12ResourcePtr, uint32_t>> recycle_after_sync_readback_buffs_;
			std::vector<ID3D12ResourcePtr> release_after_sync_buffs_;
		};

	private:
		D3D12AdapterList const & D3DAdapters() const;
		D3D12Adapter& ActiveAdapter() const;

		virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) override;
		virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) override;
		virtual void DoBindSOBuffers(RenderLayoutPtr const & rl) override;
		virtual void DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl) override;
		virtual void DoDispatch(RenderEffect const & effect, RenderTechnique const & tech,
			uint32_t tgx, uint32_t tgy, uint32_t tgz) override;
		virtual void DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
			GraphicsBufferPtr const & buff_args, uint32_t offset) override;
		virtual void DoResize(uint32_t width, uint32_t height) override;
		virtual void DoDestroy() override;
		virtual void DoSuspend() override;
		virtual void DoResume() override;

		void FillRenderDeviceCaps();

		virtual void StereoscopicForLCDShutter(int32_t eye) override;

		bool VertexFormatSupport(ElementFormat elem_fmt);
		bool TextureFormatSupport(ElementFormat elem_fmt);
		bool RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality);
		bool UAVFormatSupport(ElementFormat elem_fmt);

		virtual void CheckConfig(RenderSettings& settings) override;

		void UpdateRenderPSO(RenderEffect const & effect, RenderPass const & pass, RenderLayout const & rl,
			bool has_tessellation);
		void UpdateComputePSO(RenderEffect const & effect, RenderPass const & pass);
		void UpdateCbvSrvUavSamplerHeaps(ShaderObject const & so);

		std::shared_ptr<CmdAllocatorDependencies> AllocCmdAllocator();
		void RecycleCmdAllocator(std::shared_ptr<CmdAllocatorDependencies> const & cmd_allocator, uint64_t fence_val);

	private:
		// Direct3D rendering device
		// Only created after top-level window created
		IDXGIFactory4Ptr gi_factory_4_;
		IDXGIFactory5Ptr gi_factory_5_;
		uint8_t dxgi_sub_ver_;

		ID3D12DevicePtr d3d_device_;
		ID3D12CommandQueuePtr d3d_render_cmd_queue_;
		std::list<std::pair<std::shared_ptr<CmdAllocatorDependencies>, uint64_t>> d3d_render_cmd_allocators_;
		std::shared_ptr<CmdAllocatorDependencies> curr_render_cmd_allocator_;
		ID3D12GraphicsCommandListPtr d3d_render_cmd_list_;
		ID3D12CommandAllocatorPtr d3d_res_cmd_allocator_;
		ID3D12GraphicsCommandListPtr d3d_res_cmd_list_;
		std::mutex res_cmd_list_mutex_;
		D3D_FEATURE_LEVEL d3d_feature_level_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D12AdapterList adapterList_;

		RenderLayout::topology_type topology_type_cache_;
		D3D12_RECT scissor_rc_cache_;
		std::vector<GraphicsBufferPtr> so_buffs_;
		std::unordered_map<size_t, ID3D12RootSignaturePtr> root_signatures_;
		std::unordered_map<size_t, ID3D12PipelineStatePtr> graphics_psos_;
		std::unordered_map<size_t, ID3D12PipelineStatePtr> compute_psos_;
		std::unordered_map<size_t, ID3D12DescriptorHeapPtr> cbv_srv_uav_heaps_;

		uint16_t curr_stencil_ref_;
		Color curr_blend_factor_;
		D3D12_VIEWPORT curr_viewport_;
		ID3D12PipelineState* curr_pso_;
		ID3D12RootSignature* curr_graphics_root_signature_;
		ID3D12RootSignature* curr_compute_root_signature_;
		D3D12_RECT curr_scissor_rc_;
		D3D12_PRIMITIVE_TOPOLOGY curr_topology_;
		std::array<ID3D12DescriptorHeap*, 2> curr_desc_heaps_;
		uint32_t curr_num_desc_heaps_;
		std::vector<D3D12_VERTEX_BUFFER_VIEW> curr_vbvs_;
		D3D12_INDEX_BUFFER_VIEW curr_ibv_;

		std::multimap<uint32_t, ID3D12ResourcePtr> temp_upload_free_buffs_;
		std::multimap<uint32_t, ID3D12ResourcePtr> temp_readback_free_buffs_;

		ID3D12DescriptorHeapPtr rtv_desc_heap_;
		uint32_t rtv_desc_size_;
		ID3D12DescriptorHeapPtr dsv_desc_heap_;
		uint32_t dsv_desc_size_;
		ID3D12DescriptorHeapPtr cbv_srv_uav_desc_heap_;
		uint32_t cbv_srv_uav_desc_size_;
		uint32_t sampler_desc_size_;
		std::bitset<NUM_MAX_RENDER_TARGET_VIEWS> rtv_heap_occupied_;
		std::bitset<NUM_MAX_DEPTH_STENCIL_VIEWS> dsv_heap_occupied_;
		std::bitset<NUM_MAX_CBV_SRV_UAVS> cbv_srv_uav_heap_occupied_;

		D3D12_CPU_DESCRIPTOR_HANDLE null_srv_handle_;
		D3D12_CPU_DESCRIPTOR_HANDLE null_uav_handle_;

		char const * vs_profile_;
		char const * ps_profile_;
		char const * gs_profile_;
		char const * cs_profile_;
		char const * hs_profile_;
		char const * ds_profile_;

		enum StereoMethod
		{
			SM_None,
			SM_DXGI
		};

		StereoMethod stereo_method_;

		std::vector<ElementFormat> vertex_format_;
		std::vector<ElementFormat> texture_format_;
		std::map<ElementFormat, std::vector<std::pair<uint32_t, uint32_t>>> rendertarget_format_;
		std::vector<ElementFormat> uav_format_;

		double inv_timestamp_freq_;

		FencePtr render_cmd_fence_;
		uint64_t render_cmd_fence_val_;

		FencePtr res_cmd_fence_;
		uint64_t res_cmd_fence_val_;

		RenderEffectPtr blit_effect_;
		RenderTechnique* bilinear_blit_tech_;
	};
}

#endif			// _D3D12RENDERENGINE_HPP
