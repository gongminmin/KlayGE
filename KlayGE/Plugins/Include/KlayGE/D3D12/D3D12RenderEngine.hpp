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
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/D3D12/D3D12AdapterList.hpp>
#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12GpuMemoryAllocator.hpp>
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

	class D3D12RenderEngine final : public RenderEngine
	{
	public:
		D3D12RenderEngine();
		~D3D12RenderEngine() override;

		std::wstring const & Name() const override;

		bool RequiresFlipping() const override
		{
			return true;
		}

		void BeginFrame() override;
		void EndFrame() override;

		IDXGIFactory4* DXGIFactory4() const noexcept;
		IDXGIFactory5* DXGIFactory5() const noexcept;
		IDXGIFactory6* DXGIFactory6() const noexcept;
		uint8_t DXGISubVer() const noexcept;

		ID3D12Device* D3DDevice() const noexcept;
		ID3D12CommandQueue* D3DCmdQueue() const noexcept;
		D3D_FEATURE_LEVEL DeviceFeatureLevel() const noexcept;
		void D3DDevice(ID3D12Device* device, D3D_FEATURE_LEVEL feature_level);

		ID3D12CommandAllocator* D3DRenderCmdAllocator() const;
		ID3D12GraphicsCommandList* D3DRenderCmdList() const;
		void CommitRenderCmd();
		void SyncRenderCmd();
		void ResetRenderCmd();

		ID3D12CommandAllocator* D3DLoadCmdAllocator() const;
		ID3D12GraphicsCommandList* D3DLoadCmdList() const;
		void CommitLoadCmd();
		void SyncLoadCmd();
		void ResetLoadCmd();

		void ForceFlush() override;
		void ForceFinish();

		TexturePtr const & ScreenDepthStencilTexture() const override;

		void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		bool FullScreen() const override;
		void FullScreen(bool fs) override;

		char const* DefaultShaderProfile(ShaderStage stage) const noexcept
		{
			return shader_profiles_[static_cast<uint32_t>(stage)];
		}

		void OMSetStencilRef(ID3D12GraphicsCommandList* cmd_list, uint16_t stencil_ref);
		void OMSetBlendFactor(ID3D12GraphicsCommandList* cmd_list, Color const& blend_factor);
		void OMSetRenderTargets(ID3D12GraphicsCommandList* cmd_list, std::span<D3D12_CPU_DESCRIPTOR_HANDLE const> render_target_descriptors,
			bool rts_single_handle_to_descriptor_range, D3D12_CPU_DESCRIPTOR_HANDLE const* depth_stencil_descriptor);
		void RSSetViewports(ID3D12GraphicsCommandList* cmd_list, std::span<D3D12_VIEWPORT const> viewports);
		void SetPipelineState(ID3D12GraphicsCommandList* cmd_list, ID3D12PipelineState* pso);
		void SetGraphicsRootSignature(ID3D12GraphicsCommandList* cmd_list, ID3D12RootSignature* root_signature);
		void SetComputeRootSignature(ID3D12GraphicsCommandList* cmd_list, ID3D12RootSignature* root_signature);
		void RSSetScissorRects(ID3D12GraphicsCommandList* cmd_list, D3D12_RECT const& rect);
		void IASetPrimitiveTopology(ID3D12GraphicsCommandList* cmd_list, RenderLayout::topology_type primitive_topology);
		void SetDescriptorHeaps(ID3D12GraphicsCommandList* cmd_list, std::span<ID3D12DescriptorHeap* const> descriptor_heaps);
		void IASetVertexBuffers(ID3D12GraphicsCommandList* cmd_list, uint32_t start_slot, std::span<D3D12_VERTEX_BUFFER_VIEW const> views);
		void IASetIndexBuffer(ID3D12GraphicsCommandList* cmd_list, D3D12_INDEX_BUFFER_VIEW const& view);
		
		void ResetRenderStates();

		ID3D12DescriptorHeap* RTVDescHeap() const noexcept
		{
			return rtv_desc_heap_.get();
		}
		ID3D12DescriptorHeap* DSVDescHeap() const noexcept
		{
			return dsv_desc_heap_.get();
		}
		ID3D12DescriptorHeap* CBVSRVUAVDescHeap() const noexcept
		{
			return cbv_srv_uav_desc_heap_.get();
		}
		uint32_t RTVDescSize() const noexcept
		{
			return rtv_desc_size_;
		}
		uint32_t DSVDescSize() const noexcept
		{
			return dsv_desc_size_;
		}
		uint32_t CBVSRVUAVDescSize() const noexcept
		{
			return cbv_srv_uav_desc_size_;
		}
		uint32_t SamplerDescSize() const noexcept
		{
			return sampler_desc_size_;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE NullSrvHandle() const noexcept
		{
			return null_srv_handle_;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE NullUavHandle() const noexcept
		{
			return null_uav_handle_;
		}

		uint32_t AllocRTV();
		uint32_t AllocDSV();
		uint32_t AllocCBVSRVUAV();
		void DeallocRTV(uint32_t offset);
		void DeallocDSV(uint32_t offset);
		void DeallocCBVSRVUAV(uint32_t offset);

		ID3D12RootSignaturePtr const & CreateRootSignature(
			std::array<uint32_t, NumShaderStages * 4> const & num,
			bool has_vs, bool has_stream_output);
		ID3D12PipelineStatePtr const & CreateRenderPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC const & desc);
		ID3D12PipelineStatePtr const & CreateComputePSO(D3D12_COMPUTE_PIPELINE_STATE_DESC const & desc);

		std::unique_ptr<D3D12GpuMemoryBlock> AllocMemBlock(bool is_upload, uint32_t size_in_bytes);
		void DeallocMemBlock(bool is_upload, std::unique_ptr<D3D12GpuMemoryBlock> mem_block);
		void RenewMemBlock(bool is_upload, std::unique_ptr<D3D12GpuMemoryBlock>& mem_block, uint32_t size_in_bytes);

		void AddStallResource(ID3D12ResourcePtr const& resource);

		void AddResourceBarrier(ID3D12GraphicsCommandList* cmd_list, std::span<D3D12_RESOURCE_BARRIER const> barriers);
		void FlushResourceBarriers(ID3D12GraphicsCommandList* cmd_list);

		ID3D12DescriptorHeap* CreateDynamicCBVSRVUAVDescriptorHeap(uint32_t num);

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

		virtual void CheckConfig(RenderSettings& settings) override;

		void UpdateRenderPSO(ID3D12GraphicsCommandList* cmd_list, RenderEffect const& effect, RenderPass const& pass,
			RenderLayout const& rl, bool has_tessellation);
		void UpdateComputePSO(ID3D12GraphicsCommandList* cmd_list, RenderEffect const& effect, RenderPass const& pass);
		void UpdateCbvSrvUavSamplerHeaps(ID3D12GraphicsCommandList* cmd_list, RenderEffect const& effect, ShaderObject const& so);

		std::vector<D3D12_RESOURCE_BARRIER>* FindResourceBarriers(ID3D12GraphicsCommandList* cmd_list, bool allow_creation);

		void RestoreRenderCmdStates(ID3D12GraphicsCommandList* cmd_list);

		class PerThreadContext;
		PerThreadContext& CurrThreadContext(bool is_render_context) const;
		void CommitRenderCmd(PerThreadContext& context);
		void SyncRenderCmd(PerThreadContext& context);
		void ResetRenderCmd(PerThreadContext& context);

	private:
		// Direct3D rendering device
		// Only created after top-level window created
		IDXGIFactory4Ptr gi_factory_4_;
		IDXGIFactory5Ptr gi_factory_5_;
		IDXGIFactory6Ptr gi_factory_6_;
		uint8_t dxgi_sub_ver_;

		ID3D12DevicePtr d3d_device_;
		ID3D12CommandQueuePtr d3d_cmd_queue_;
		D3D_FEATURE_LEVEL d3d_feature_level_;

		class PerThreadContext : boost::noncopyable
		{
		public:
			PerThreadContext(ID3D12Device* d3d_device, FencePtr const& frame_fence);
			~PerThreadContext();

			void CommitCmd(ID3D12CommandQueue* d3d_cmd_queue, uint32_t frame_index);
			void SyncCmd(uint32_t frame_index);
			void ResetCmd(uint32_t frame_index);

			void Reset(uint32_t frame_index);

			std::thread::id ThreadID() const noexcept;
			ID3D12CommandAllocator* D3DCmdAllocator(uint32_t frame_index) const noexcept;
			ID3D12GraphicsCommandList* D3DCmdList() const noexcept;

			uint64_t FrameFenceValue(uint32_t frame_index) const noexcept;

		private:
			struct PerThreadPerFrameContext : boost::noncopyable
			{
				ID3D12CommandAllocatorPtr d3d_cmd_allocator;
				uint64_t fence_value = 0;
			};

		private:
			std::thread::id thread_id_;
			std::array<PerThreadPerFrameContext, NUM_BACK_BUFFERS> per_frame_contexts_;
			ID3D12GraphicsCommandListPtr d3d_cmd_list_;
			std::weak_ptr<Fence> frame_fence_;
		};
		mutable std::vector<std::unique_ptr<PerThreadContext>> render_thread_cmd_contexts_;
		mutable std::vector<std::unique_ptr<PerThreadContext>> load_thread_cmd_contexts_;

		struct PerFrameContext : boost::noncopyable
		{
		public:
			~PerFrameContext();

			void Destroy();

			ID3D12DescriptorHeap* CreateDynamicCBVSRVUAVDescriptorHeap(ID3D12Device* d3d_device, uint32_t num);

			void AddStallResource(ID3D12ResourcePtr const& resource);
			void ClearStallResources();

		private:
			struct CbvSrcUavHeaps
			{
				std::vector<ID3D12DescriptorHeapPtr> actives;
				std::vector<ID3D12DescriptorHeapPtr> stalls;
			};
			std::map<uint32_t, CbvSrcUavHeaps> cbv_srv_uav_heap_cache_;
			std::vector<ID3D12ResourcePtr> stall_resources_;
			std::mutex mutex_;
		};
		std::array<PerFrameContext, NUM_BACK_BUFFERS> per_frame_contexts_;
		uint32_t curr_frame_index_ = 0;

		FencePtr frame_fence_;
		uint64_t frame_fence_value_;

		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D12AdapterList adapterList_;

		RenderLayout::topology_type topology_type_cache_;
		D3D12_RECT scissor_rc_cache_;
		std::vector<GraphicsBufferPtr> so_buffs_;
		std::unordered_map<size_t, ID3D12RootSignaturePtr> root_signatures_;
		std::unordered_map<size_t, ID3D12PipelineStatePtr> graphics_psos_;
		std::unordered_map<size_t, ID3D12PipelineStatePtr> compute_psos_;

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
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> curr_render_targets_;
		uint32_t curr_num_render_targets_ = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE const* curr_depth_stencil_ = nullptr;

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

		char const* shader_profiles_[NumShaderStages];

		D3D12GpuMemoryAllocator upload_mem_allocator_{true};
		D3D12GpuMemoryAllocator readback_mem_allocator_{false};

		enum StereoMethod
		{
			SM_None,
			SM_DXGI
		};

		StereoMethod stereo_method_;

		ID3D12CommandSignaturePtr draw_indirect_signature_;
		ID3D12CommandSignaturePtr draw_indexed_indirect_signature_;
		ID3D12CommandSignaturePtr dispatch_indirect_signature_;

		std::vector<std::pair<ID3D12GraphicsCommandList*, std::vector<D3D12_RESOURCE_BARRIER>>> res_barriers_;
	};
}

#endif			// _D3D12RENDERENGINE_HPP
