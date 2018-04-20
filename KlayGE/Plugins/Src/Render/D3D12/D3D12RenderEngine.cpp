/**
 * @file D3D12RenderEngine.cpp
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

#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Fence.hpp>
#include <KFL/Hash.hpp>

#include <KlayGE/D3D12/D3D12RenderWindow.hpp>
#include <KlayGE/D3D12/D3D12FrameBuffer.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12GraphicsBuffer.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12RenderLayout.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>
#include <KlayGE/D3D12/D3D12ShaderObject.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>
#include <KlayGE/D3D12/D3D12InterfaceLoader.hpp>
#include <KlayGE/D3D12/D3D12Fence.hpp>

#include <algorithm>
#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D12RenderEngine::D3D12RenderEngine()
		: inv_timestamp_freq_(0),
			render_cmd_fence_val_(0), res_cmd_fence_val_(0)
	{
		UINT dxgi_factory_flags = 0;

#ifdef KLAYGE_DEBUG
		{
			ID3D12Debug* debug_ctrl = nullptr;
			if (SUCCEEDED(D3D12InterfaceLoader::Instance().D3D12GetDebugInterface(IID_ID3D12Debug, reinterpret_cast<void**>(&debug_ctrl))))
			{
				BOOST_ASSERT(debug_ctrl);
				debug_ctrl->EnableDebugLayer();
				debug_ctrl->Release();

				dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif

		native_shader_fourcc_ = MakeFourCC<'D', 'X', 'B', 'C'>::value;
		native_shader_version_ = 5;

		IDXGIFactory4* gi_factory;
		TIFHR(D3D12InterfaceLoader::Instance().CreateDXGIFactory2(dxgi_factory_flags,
			IID_IDXGIFactory4, reinterpret_cast<void**>(&gi_factory)));
		gi_factory_4_ = MakeCOMPtr(gi_factory);
		dxgi_sub_ver_ = 4;

		IDXGIFactory5* gi_factory5;
		gi_factory->QueryInterface(IID_IDXGIFactory5, reinterpret_cast<void**>(&gi_factory5));
		if (gi_factory5 != nullptr)
		{
			gi_factory_5_ = MakeCOMPtr(gi_factory5);
			dxgi_sub_ver_ = 5;
		}

		adapterList_.Enumerate(gi_factory_4_);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D12RenderEngine::~D3D12RenderEngine()
	{
		this->Destroy();
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & D3D12RenderEngine::Name() const
	{
		static std::wstring const name(L"Direct3D12 Render Engine");
		return name;
	}

	void D3D12RenderEngine::BeginFrame()
	{
		this->BindFrameBuffer(this->DefaultFrameBuffer());

		RenderEngine::BeginFrame();
	}

	void D3D12RenderEngine::EndFrame()
	{
		RenderEngine::EndFrame();

		render_cmd_fence_val_ = checked_cast<D3D12Fence*>(render_cmd_fence_.get())->Signal(d3d_render_cmd_queue_.get());
		this->RecycleCmdAllocator(curr_render_cmd_allocator_, render_cmd_fence_val_);
		curr_render_cmd_allocator_ = this->AllocCmdAllocator();

		this->ResetRenderCmd();
		this->ClearTempObjs();
	}

	void D3D12RenderEngine::UpdateGPUTimestampsFrequency()
	{
		inv_timestamp_freq_ = 0;
		if (d3d_render_cmd_queue_)
		{
			UINT64 freq;
			if (SUCCEEDED(d3d_render_cmd_queue_->GetTimestampFrequency(&freq)))
			{
				inv_timestamp_freq_ = 1.0 / freq;
			}
		}
	}

	IDXGIFactory4* D3D12RenderEngine::DXGIFactory4() const
	{
		return gi_factory_4_.get();
	}

	IDXGIFactory5* D3D12RenderEngine::DXGIFactory5() const
	{
		return gi_factory_5_.get();
	}

	uint8_t D3D12RenderEngine::DXGISubVer() const
	{
		return dxgi_sub_ver_;
	}

	ID3D12Device* D3D12RenderEngine::D3DDevice() const
	{
		return d3d_device_.get();
	}

	ID3D12CommandQueue* D3D12RenderEngine::D3DRenderCmdQueue() const
	{
		return d3d_render_cmd_queue_.get();
	}

	ID3D12CommandAllocator* D3D12RenderEngine::D3DRenderCmdAllocator() const
	{
		return curr_render_cmd_allocator_->cmd_allocator.get();
	}

	ID3D12GraphicsCommandList* D3D12RenderEngine::D3DRenderCmdList() const
	{
		return d3d_render_cmd_list_.get();
	}

	ID3D12CommandAllocator* D3D12RenderEngine::D3DResCmdAllocator() const
	{
		return d3d_res_cmd_allocator_.get();
	}

	ID3D12GraphicsCommandList* D3D12RenderEngine::D3DResCmdList() const
	{
		return d3d_res_cmd_list_.get();
	}

	D3D_FEATURE_LEVEL D3D12RenderEngine::DeviceFeatureLevel() const
	{
		return d3d_feature_level_;
	}

	// 获取D3D适配器列表
	/////////////////////////////////////////////////////////////////////////////////
	D3D12AdapterList const & D3D12RenderEngine::D3DAdapters() const
	{
		return adapterList_;
	}

	// 获取当前适配器
	/////////////////////////////////////////////////////////////////////////////////
	D3D12Adapter& D3D12RenderEngine::ActiveAdapter() const
	{
		return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		D3D12RenderWindowPtr win = MakeSharedPtr<D3D12RenderWindow>(&this->ActiveAdapter(), name, settings);

		native_shader_platform_name_ = "d3d_12_0";
		switch (d3d_feature_level_)
		{
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			vs_profile_ = "vs_5_1";
			ps_profile_ = "ps_5_1";
			gs_profile_ = "gs_5_1";
			cs_profile_ = "cs_5_1";
			hs_profile_ = "hs_5_1";
			ds_profile_ = "ds_5_1";
			break;

		default:
			KFL_UNREACHABLE("Invalid feature level");
		}

		this->ResetRenderStates();
		this->BindFrameBuffer(win);

		if (STM_LCDShutter == settings.stereo_method)
		{
			stereo_method_ = SM_None;

			if (gi_factory_4_->IsWindowedStereoEnabled())
			{
				stereo_method_ = SM_DXGI;
			}
		}

		blit_effect_ = SyncLoadRenderEffect("Copy.fxml");
		bilinear_blit_tech_ = blit_effect_->TechniqueByName("BilinearCopy");
	}

	void D3D12RenderEngine::CheckConfig(RenderSettings& settings)
	{
		KFL_UNUSED(settings);
	}

	void D3D12RenderEngine::D3DDevice(ID3D12Device* device, ID3D12CommandQueue* cmd_queue, D3D_FEATURE_LEVEL feature_level)
	{
		d3d_device_ = MakeCOMPtr(device);
		d3d_render_cmd_queue_ = MakeCOMPtr(cmd_queue);
		d3d_feature_level_ = feature_level;

		Verify(!!d3d_render_cmd_queue_);
		Verify(!!d3d_device_);

		curr_render_cmd_allocator_ = this->AllocCmdAllocator();

		ID3D12GraphicsCommandList* d3d_render_cmd_list;
		TIFHR(d3d_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, curr_render_cmd_allocator_->cmd_allocator.get(), nullptr,
			IID_ID3D12GraphicsCommandList, reinterpret_cast<void**>(&d3d_render_cmd_list)));
		d3d_render_cmd_list_ = MakeCOMPtr(d3d_render_cmd_list);

		ID3D12CommandAllocator* d3d_res_cmd_allocator;
		TIFHR(d3d_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_ID3D12CommandAllocator, reinterpret_cast<void**>(&d3d_res_cmd_allocator)));
		d3d_res_cmd_allocator_ = MakeCOMPtr(d3d_res_cmd_allocator);

		ID3D12GraphicsCommandList* d3d_res_cmd_list;
		TIFHR(d3d_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3d_res_cmd_allocator_.get(), nullptr,
			IID_ID3D12GraphicsCommandList, reinterpret_cast<void**>(&d3d_res_cmd_list)));
		d3d_res_cmd_list_ = MakeCOMPtr(d3d_res_cmd_list);

		D3D12_DESCRIPTOR_HEAP_DESC rtv_desc_heap;
		rtv_desc_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtv_desc_heap.NumDescriptors = NUM_BACK_BUFFERS * 2 + NUM_MAX_RENDER_TARGET_VIEWS;
		rtv_desc_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtv_desc_heap.NodeMask = 0;
		ID3D12DescriptorHeap* rtv_descriptor_heap;
		TIFHR(d3d_device_->CreateDescriptorHeap(&rtv_desc_heap, IID_ID3D12DescriptorHeap,
			reinterpret_cast<void**>(&rtv_descriptor_heap)));
		rtv_desc_heap_ = MakeCOMPtr(rtv_descriptor_heap);
		rtv_desc_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsv_desc_heap;
		dsv_desc_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsv_desc_heap.NumDescriptors = 2 + NUM_MAX_DEPTH_STENCIL_VIEWS;
		dsv_desc_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsv_desc_heap.NodeMask = 0;
		ID3D12DescriptorHeap* dsv_descriptor_heap;
		TIFHR(d3d_device_->CreateDescriptorHeap(&dsv_desc_heap, IID_ID3D12DescriptorHeap,
			reinterpret_cast<void**>(&dsv_descriptor_heap)));
		dsv_desc_heap_ = MakeCOMPtr(dsv_descriptor_heap);
		dsv_desc_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_uav_desc_heap;
		cbv_srv_uav_desc_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_srv_uav_desc_heap.NumDescriptors = NUM_MAX_CBV_SRV_UAVS;
		cbv_srv_uav_desc_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		cbv_srv_uav_desc_heap.NodeMask = 0;
		ID3D12DescriptorHeap* cbv_srv_uav_descriptor_heap;
		TIFHR(d3d_device_->CreateDescriptorHeap(&cbv_srv_uav_desc_heap, IID_ID3D12DescriptorHeap,
			reinterpret_cast<void**>(&cbv_srv_uav_descriptor_heap)));
		cbv_srv_uav_desc_heap_ = MakeCOMPtr(cbv_srv_uav_descriptor_heap);
		cbv_srv_uav_desc_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		sampler_desc_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		rtv_heap_occupied_.reset();
		dsv_heap_occupied_.reset();
		cbv_srv_uav_heap_occupied_.reset();

		D3D12_SHADER_RESOURCE_VIEW_DESC null_srv_desc;
		null_srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		null_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		null_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		null_srv_desc.Texture2D.MipLevels = 1;
		null_srv_desc.Texture2D.MostDetailedMip = 0;
		null_srv_desc.Texture2D.PlaneSlice = 0;
		null_srv_desc.Texture2D.ResourceMinLODClamp = 0;
		null_srv_handle_ = cbv_srv_uav_desc_heap_->GetCPUDescriptorHandleForHeapStart();
		null_srv_handle_.ptr += this->AllocCBVSRVUAV();
		d3d_device_->CreateShaderResourceView(nullptr, &null_srv_desc, null_srv_handle_);

		D3D12_UNORDERED_ACCESS_VIEW_DESC null_uav_desc;
		null_uav_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		null_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		null_uav_desc.Texture2D.MipSlice = 0;
		null_uav_desc.Texture2D.PlaneSlice = 0;
		null_uav_handle_ = cbv_srv_uav_desc_heap_->GetCPUDescriptorHandleForHeapStart();
		null_uav_handle_.ptr += this->AllocCBVSRVUAV();
		d3d_device_->CreateUnorderedAccessView(nullptr, nullptr, &null_uav_desc, null_uav_handle_);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		res_cmd_fence_ = rf.MakeFence();
		render_cmd_fence_ = rf.MakeFence();

		{
			D3D12_INDIRECT_ARGUMENT_DESC indirect_param;
			indirect_param.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

			D3D12_COMMAND_SIGNATURE_DESC cmd_signature_desc;
			cmd_signature_desc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
			cmd_signature_desc.NumArgumentDescs = 1;
			cmd_signature_desc.pArgumentDescs = &indirect_param;
			cmd_signature_desc.NodeMask = 1;

			ID3D12CommandSignature* cmd_signature;
			TIFHR(d3d_device_->CreateCommandSignature(&cmd_signature_desc, nullptr,
				IID_ID3D12CommandSignature, reinterpret_cast<void**>(&cmd_signature)));
			draw_indirect_signature_ = MakeCOMPtr(cmd_signature);
		}
		{
			D3D12_INDIRECT_ARGUMENT_DESC indirect_param;
			indirect_param.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

			D3D12_COMMAND_SIGNATURE_DESC cmd_signature_desc;
			cmd_signature_desc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
			cmd_signature_desc.NumArgumentDescs = 1;
			cmd_signature_desc.pArgumentDescs = &indirect_param;
			cmd_signature_desc.NodeMask = 1;

			ID3D12CommandSignature* cmd_signature;
			TIFHR(d3d_device_->CreateCommandSignature(&cmd_signature_desc, nullptr,
				IID_ID3D12CommandSignature, reinterpret_cast<void**>(&cmd_signature)));
			draw_indexed_indirect_signature_ = MakeCOMPtr(cmd_signature);
		}
		{
			D3D12_INDIRECT_ARGUMENT_DESC indirect_param;
			indirect_param.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

			D3D12_COMMAND_SIGNATURE_DESC cmd_signature_desc;
			cmd_signature_desc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
			cmd_signature_desc.NumArgumentDescs = 1;
			cmd_signature_desc.pArgumentDescs = &indirect_param;
			cmd_signature_desc.NodeMask = 1;

			ID3D12CommandSignature* cmd_signature;
			TIFHR(d3d_device_->CreateCommandSignature(&cmd_signature_desc, nullptr,
				IID_ID3D12CommandSignature, reinterpret_cast<void**>(&cmd_signature)));
			dispatch_indirect_signature_ = MakeCOMPtr(cmd_signature);
		}

		this->FillRenderDeviceCaps();
	}

	void D3D12RenderEngine::ClearTempObjs()
	{
		curr_render_cmd_allocator_->cbv_srv_uav_heap_cache_.clear();
		curr_render_cmd_allocator_->release_after_sync_buffs_.clear();

		for (auto const & item : curr_render_cmd_allocator_->recycle_after_sync_upload_buffs_)
		{
			temp_upload_free_buffs_.emplace(item.second, item.first);
		}
		curr_render_cmd_allocator_->recycle_after_sync_upload_buffs_.clear();
		for (auto const & item : curr_render_cmd_allocator_->recycle_after_sync_readback_buffs_)
		{
			temp_readback_free_buffs_.emplace(item.second, item.first);
		}
		curr_render_cmd_allocator_->recycle_after_sync_readback_buffs_.clear();
	}

	void D3D12RenderEngine::CommitResCmd()
	{
		TIFHR(d3d_res_cmd_list_->Close());
		ID3D12CommandList* cmd_lists[] = { d3d_res_cmd_list_.get() };
		d3d_render_cmd_queue_->ExecuteCommandLists(static_cast<uint32_t>(std::size(cmd_lists)), cmd_lists);

		res_cmd_fence_val_ = res_cmd_fence_->Signal(Fence::FT_Render);
		res_cmd_fence_->Wait(res_cmd_fence_val_);

		d3d_res_cmd_allocator_->Reset();
		d3d_res_cmd_list_->Reset(d3d_res_cmd_allocator_.get(), nullptr);
	}

	void D3D12RenderEngine::CommitRenderCmd()
	{
		TIFHR(d3d_render_cmd_list_->Close());
		ID3D12CommandList* cmd_lists[] = { d3d_render_cmd_list_.get() };
		d3d_render_cmd_queue_->ExecuteCommandLists(static_cast<uint32_t>(std::size(cmd_lists)), cmd_lists);
	}

	void D3D12RenderEngine::SyncRenderCmd()
	{
		render_cmd_fence_val_ = checked_cast<D3D12Fence*>(render_cmd_fence_.get())->Signal(d3d_render_cmd_queue_.get());
		render_cmd_fence_->Wait(render_cmd_fence_val_);
	}

	void D3D12RenderEngine::ResetRenderCmd()
	{
		d3d_render_cmd_list_->Reset(this->D3DRenderCmdAllocator(), curr_pso_);
		d3d_render_cmd_list_->OMSetStencilRef(curr_stencil_ref_);
		d3d_render_cmd_list_->OMSetBlendFactor(&curr_blend_factor_.r());
		d3d_render_cmd_list_->RSSetViewports(1, &curr_viewport_);
		if (curr_graphics_root_signature_ != nullptr)
		{
			d3d_render_cmd_list_->SetGraphicsRootSignature(curr_graphics_root_signature_);
		}
		if (curr_compute_root_signature_ != nullptr)
		{
			d3d_render_cmd_list_->SetComputeRootSignature(curr_compute_root_signature_);
		}
		d3d_render_cmd_list_->IASetPrimitiveTopology(curr_topology_);
		d3d_render_cmd_list_->RSSetScissorRects(1, &curr_scissor_rc_);
		d3d_render_cmd_list_->SetDescriptorHeaps(curr_num_desc_heaps_, curr_desc_heaps_.data());
		d3d_render_cmd_list_->IASetVertexBuffers(0, static_cast<uint32_t>(curr_vbvs_.size()), curr_vbvs_.data());
		d3d_render_cmd_list_->IASetIndexBuffer(&curr_ibv_);

		auto fb = checked_cast<D3D12FrameBuffer*>(this->CurFrameBuffer().get());
		if (fb)
		{
			fb->SetRenderTargets();
		}
	}

	void D3D12RenderEngine::ResetRenderStates()
	{
		RasterizerStateDesc default_rs_desc;
		DepthStencilStateDesc default_dss_desc;
		BlendStateDesc default_bs_desc;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRenderStateObject(default_rs_desc, default_dss_desc, default_bs_desc);

		topology_type_cache_ = RenderLayout::TT_PointList;

		memset(&scissor_rc_cache_, 0, sizeof(scissor_rc_cache_));

		curr_stencil_ref_ = 0;
		curr_blend_factor_ = Color(1, 1, 1, 1);
		memset(&curr_viewport_, 0, sizeof(curr_viewport_));
		curr_pso_ = nullptr;
		curr_graphics_root_signature_ = nullptr;
		curr_compute_root_signature_ = nullptr;
		curr_scissor_rc_ = { 0, 0, 0, 0 };
		curr_topology_ = D3D12Mapping::Mapping(topology_type_cache_);
		curr_num_desc_heaps_ = 0;
		curr_vbvs_.clear();
		curr_ibv_ = { 0, 0, DXGI_FORMAT_UNKNOWN };

		d3d_render_cmd_list_->IASetPrimitiveTopology(curr_topology_);

		this->ClearTempObjs();
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderEngine::DoBindFrameBuffer(FrameBufferPtr const & fb)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(d3d_device_);
		BOOST_ASSERT(fb);
	}

	// 设置当前Stream output目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderEngine::DoBindSOBuffers(RenderLayoutPtr const & rl)
	{
		uint32_t num_buffs = rl ? rl->NumVertexStreams() : 0;
		if (num_buffs > 0)
		{
			so_buffs_.resize(num_buffs);
			std::vector<D3D12_STREAM_OUTPUT_BUFFER_VIEW> sobv(num_buffs);
			for (uint32_t i = 0; i < num_buffs; ++ i)
			{
				D3D12GraphicsBufferPtr d3d12_buf = checked_pointer_cast<D3D12GraphicsBuffer>(rl->GetVertexStream(i));

				so_buffs_[i] = d3d12_buf;
				sobv[i].BufferLocation = d3d12_buf->GPUVirtualAddress();
				sobv[i].SizeInBytes = d3d12_buf->Size();
				sobv[i].BufferFilledSizeLocation = sobv[i].BufferLocation + d3d12_buf->CounterOffset();
			}

			d3d_render_cmd_list_->SOSetTargets(0, static_cast<UINT>(num_buffs), &sobv[0]);
		}
		else if (so_buffs_.size() > 0)
		{
			d3d_render_cmd_list_->SOSetTargets(0, 0, nullptr);

			so_buffs_.clear();
		}
	}

	void D3D12RenderEngine::UpdateRenderPSO(RenderEffect const & effect, RenderPass const & pass, RenderLayout const & rl,
		bool has_tessellation)
	{
		auto const & so = *checked_cast<D3D12ShaderObject const *>(pass.GetShaderObject(effect).get());
		auto const & rso = *checked_cast<D3D12RenderStateObject const *>(pass.GetRenderStateObject().get());

		auto pso = rso.RetrieveGraphicsPSO(rl, so, *this->CurFrameBuffer(), has_tessellation);
		this->SetPipelineState(pso);

		auto root_signature = so.RootSignature();
		this->SetGraphicsRootSignature(root_signature);

		D3D12_RECT scissor_rc;
		if (pass.GetRenderStateObject()->GetRasterizerStateDesc().scissor_enable)
		{
			scissor_rc = scissor_rc_cache_;
		}
		else
		{
			scissor_rc =
			{
				static_cast<LONG>(curr_viewport_.TopLeftX),
				static_cast<LONG>(curr_viewport_.TopLeftY),
				static_cast<LONG>(curr_viewport_.TopLeftX + curr_viewport_.Width),
				static_cast<LONG>(curr_viewport_.TopLeftY + curr_viewport_.Height)
			};
		}
		this->RSSetScissorRects(scissor_rc);

		this->UpdateCbvSrvUavSamplerHeaps(so);
	}

	void D3D12RenderEngine::UpdateComputePSO(RenderEffect const & effect, RenderPass const & pass)
	{
		auto const & so = *checked_cast<D3D12ShaderObject const *>(pass.GetShaderObject(effect).get());
		auto const & rso = *checked_cast<D3D12RenderStateObject const *>(pass.GetRenderStateObject().get());

		auto pso = rso.RetrieveComputePSO(so);
		this->SetPipelineState(pso);

		auto root_signature = so.RootSignature();
		this->SetComputeRootSignature(root_signature);

		this->UpdateCbvSrvUavSamplerHeaps(so);
	}

	void D3D12RenderEngine::UpdateCbvSrvUavSamplerHeaps(ShaderObject const & so)
	{
		auto const & d3d12_so = *checked_cast<D3D12ShaderObject const *>(&so);

		uint32_t const num_handles = d3d12_so.NumHandles();

		std::array<ID3D12DescriptorHeap*, 2> heaps;
		uint32_t num_heaps = 0;
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap;
		auto sampler_heap = d3d12_so.SamplerHeap();
		if (num_handles > 0)
		{
			size_t hash_val = 0;
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
				HashCombine(hash_val, st);
				HashCombine(hash_val, d3d12_so.SRVs(st).size());
				if (!d3d12_so.SRVs(st).empty())
				{
					HashRange(hash_val, d3d12_so.SRVs(st).begin(), d3d12_so.SRVs(st).end());
				}
			}
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
				HashCombine(hash_val, st);
				HashCombine(hash_val, d3d12_so.UAVs(st).size());
				if (!d3d12_so.UAVs(st).empty())
				{
					HashRange(hash_val, d3d12_so.UAVs(st).begin(), d3d12_so.UAVs(st).end());
				}
			}

			auto iter = cbv_srv_uav_heaps_.find(hash_val);
			if (iter == cbv_srv_uav_heaps_.end())
			{
				cbv_srv_uav_heap = this->CreateDynamicCBVSRVUAVDescriptorHeap(num_handles);
				cbv_srv_uav_heaps_.emplace(hash_val, cbv_srv_uav_heap);
			}
			else
			{
				cbv_srv_uav_heap = iter->second;
			}
			heaps[num_heaps] = cbv_srv_uav_heap.get();
			++ num_heaps;
		}
		if (sampler_heap)
		{
			heaps[num_heaps] = sampler_heap;
			++ num_heaps;
		}

		this->SetDescriptorHeaps(ArrayRef<ID3D12DescriptorHeap*>(heaps.data(), num_heaps));

		uint32_t root_param_index = 0;
		for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
			auto const & cbuffers = d3d12_so.CBuffers(st);
			if (!cbuffers.empty())
			{
				for (auto cbuffer : cbuffers)
				{
					D3D12_GPU_VIRTUAL_ADDRESS gpu_vaddr;
					if (cbuffer != nullptr)
					{
						gpu_vaddr = checked_cast<D3D12GraphicsBuffer*>(cbuffer)->GPUVirtualAddress();
					}
					else
					{
						gpu_vaddr = 0;
					}

					if (st != ShaderObject::ST_ComputeShader)
					{
						d3d_render_cmd_list_->SetGraphicsRootConstantBufferView(root_param_index, gpu_vaddr);
					}
					else
					{
						d3d_render_cmd_list_->SetComputeRootConstantBufferView(root_param_index, gpu_vaddr);
					}
					++ root_param_index;
				}
			}
		}

		uint32_t const HANDLES_PER_COPY = 16;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, HANDLES_PER_COPY> src_handles;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, HANDLES_PER_COPY> dst_handles;
		std::array<uint32_t, HANDLES_PER_COPY> handle_sizes;
		handle_sizes.fill(1);
		if (cbv_srv_uav_heap)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE cpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE gpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();

			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType const st = static_cast<ShaderObject::ShaderType>(i);
				auto const & srvs = d3d12_so.SRVs(st);
				if (!srvs.empty())
				{
					if (st != ShaderObject::ST_ComputeShader)
					{
						d3d_render_cmd_list_->SetGraphicsRootDescriptorTable(root_param_index, gpu_cbv_srv_uav_handle);
					}
					else
					{
						d3d_render_cmd_list_->SetComputeRootDescriptorTable(root_param_index, gpu_cbv_srv_uav_handle);
					}

					uint32_t const num_srvs = static_cast<uint32_t>(srvs.size());
					for (uint32_t j = 0; j < num_srvs; j += HANDLES_PER_COPY)
					{
						uint32_t const n = std::min(HANDLES_PER_COPY, num_srvs - j);
						for (uint32_t k = 0; k < n; ++ k)
						{
							auto srv = srvs[j + k];
							src_handles[k] = srv ? srv->Handle() : null_srv_handle_;
							dst_handles[k] = cpu_cbv_srv_uav_handle;
							cpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size_;
						}
						d3d_device_->CopyDescriptors(n, dst_handles.data(), handle_sizes.data(),
							n, src_handles.data(), handle_sizes.data(),
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					}

					gpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size_ * num_srvs;

					++ root_param_index;
				}
			}
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType const st = static_cast<ShaderObject::ShaderType>(i);
				auto const & uavs = d3d12_so.UAVs(st);
				if (!uavs.empty())
				{
					if (st != ShaderObject::ST_ComputeShader)
					{
						d3d_render_cmd_list_->SetGraphicsRootDescriptorTable(root_param_index, gpu_cbv_srv_uav_handle);
					}
					else
					{
						d3d_render_cmd_list_->SetComputeRootDescriptorTable(root_param_index, gpu_cbv_srv_uav_handle);
					}

					uint32_t const num_uavs = static_cast<uint32_t>(uavs.size());
					for (uint32_t j = 0; j < num_uavs; j += HANDLES_PER_COPY)
					{
						uint32_t const n = std::min(HANDLES_PER_COPY, num_uavs - j);
						for (uint32_t k = 0; k < n; ++ k)
						{
							auto uav = uavs[j + k];
							src_handles[k] = uav ? uav->Handle() : null_uav_handle_;
							dst_handles[k] = cpu_cbv_srv_uav_handle;
							cpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size_;
						}
						d3d_device_->CopyDescriptors(n, dst_handles.data(), handle_sizes.data(),
							n, src_handles.data(), handle_sizes.data(),
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					}

					gpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size_ * num_uavs;

					++ root_param_index;
				}
			}
		}

		if (sampler_heap)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE gpu_sampler_handle = sampler_heap->GetGPUDescriptorHandleForHeapStart();

			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType const st = static_cast<ShaderObject::ShaderType>(i);
				auto const & samplers = d3d12_so.Samplers(st);
				if (!samplers.empty())
				{
					if (st != ShaderObject::ST_ComputeShader)
					{
						d3d_render_cmd_list_->SetGraphicsRootDescriptorTable(root_param_index, gpu_sampler_handle);
					}
					else
					{
						d3d_render_cmd_list_->SetComputeRootDescriptorTable(root_param_index, gpu_sampler_handle);
					}

					gpu_sampler_handle.ptr += sampler_desc_size_ * samplers.size();

					++ root_param_index;
				}
			}
		}
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderEngine::DoRender(RenderEffect const & effect, RenderTechnique const & tech, RenderLayout const & rl)
	{
		D3D12FrameBuffer& fb = *checked_cast<D3D12FrameBuffer*>(this->CurFrameBuffer().get());
		fb.BindBarrier();

		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		for (uint32_t i = 0; i < so_buffs_.size(); ++ i)
		{
			D3D12GraphicsBuffer& d3dvb = *checked_cast<D3D12GraphicsBuffer*>(so_buffs_[i].get());

			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			if (d3dvb.UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_STREAM_OUT))
			{
				barriers.push_back(barrier);
			}
		}

		uint32_t const num_vertex_streams = rl.NumVertexStreams();

		for (uint32_t i = 0; i < num_vertex_streams; ++ i)
		{
			GraphicsBufferPtr const & stream = rl.GetVertexStream(i);

			D3D12GraphicsBuffer& d3dvb = *checked_cast<D3D12GraphicsBuffer*>(stream.get());
			if (!(d3dvb.AccessHint() & (EAH_CPU_Read | EAH_CPU_Write)))
			{
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				if (d3dvb.UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
				{
					barriers.push_back(barrier);
				}
			}
		}
		if (rl.InstanceStream())
		{
			GraphicsBufferPtr const & stream = rl.InstanceStream();

			D3D12GraphicsBuffer& d3dvb = *checked_cast<D3D12GraphicsBuffer*>(stream.get());
			if (!(d3dvb.AccessHint() & (EAH_CPU_Read | EAH_CPU_Write)))
			{
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				if (d3dvb.UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
				{
					barriers.push_back(barrier);
				}
			}
		}

		if (rl.UseIndices())
		{
			D3D12GraphicsBuffer& ib = *checked_cast<D3D12GraphicsBuffer*>(rl.GetIndexStream().get());
			if (!(ib.AccessHint() & (EAH_CPU_Read | EAH_CPU_Write)))
			{
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				if (ib.UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_INDEX_BUFFER))
				{
					barriers.push_back(barrier);
				}
			}
		}

		if (rl.GetIndirectArgs())
		{
			auto& arg_buff = *checked_cast<D3D12GraphicsBuffer*>(rl.GetIndirectArgs().get());
			if (!(arg_buff.AccessHint() & (EAH_CPU_Read | EAH_CPU_Write)))
			{
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				if (arg_buff.UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT))
				{
					barriers.push_back(barrier);
				}
			}
		}

		if (!barriers.empty())
		{
			d3d_render_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
		}

		checked_cast<D3D12RenderLayout const *>(&rl)->Active();

		uint32_t const vertex_count = static_cast<uint32_t>(rl.UseIndices() ? rl.NumIndices() : rl.NumVertices());

		RenderLayout::topology_type tt = rl.TopologyType();
		if (tech.HasTessellation())
		{
			switch (tt)
			{
			case RenderLayout::TT_PointList:
				tt = RenderLayout::TT_1_Ctrl_Pt_PatchList;
				break;

			case RenderLayout::TT_LineList:
				tt = RenderLayout::TT_2_Ctrl_Pt_PatchList;
				break;

			case RenderLayout::TT_TriangleList:
				tt = RenderLayout::TT_3_Ctrl_Pt_PatchList;
				break;

			default:
				break;
			}
		}
		this->IASetPrimitiveTopology(tt);

		uint32_t prim_count;
		switch (tt)
		{
		case RenderLayout::TT_PointList:
			prim_count = vertex_count;
			break;

		case RenderLayout::TT_LineList:
		case RenderLayout::TT_LineList_Adj:
			prim_count = vertex_count / 2;
			break;

		case RenderLayout::TT_LineStrip:
		case RenderLayout::TT_LineStrip_Adj:
			prim_count = vertex_count - 1;
			break;

		case RenderLayout::TT_TriangleList:
		case RenderLayout::TT_TriangleList_Adj:
			prim_count = vertex_count / 3;
			break;

		case RenderLayout::TT_TriangleStrip:
		case RenderLayout::TT_TriangleStrip_Adj:
			prim_count = vertex_count - 2;
			break;

		default:
			if ((tt >= RenderLayout::TT_1_Ctrl_Pt_PatchList)
				&& (tt <= RenderLayout::TT_32_Ctrl_Pt_PatchList))
			{
				prim_count = vertex_count / (tt - RenderLayout::TT_1_Ctrl_Pt_PatchList + 1);
			}
			else
			{
				KFL_UNREACHABLE("Invalid topology type");
			}
			break;
		}

		uint32_t const num_instances = rl.NumInstances();

		num_primitives_just_rendered_ += num_instances * prim_count;
		num_vertices_just_rendered_ += num_instances * vertex_count;

		uint32_t const num_passes = tech.NumPasses();
		bool const has_tessellation = tech.HasTessellation();
		GraphicsBufferPtr const & indirect_buff = rl.GetIndirectArgs();
		if (indirect_buff)
		{
			auto* arg_buff = checked_cast<D3D12GraphicsBuffer const *>(indirect_buff.get())->D3DResource().get();

			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);
					this->UpdateRenderPSO(effect, pass, rl, has_tessellation);
					d3d_render_cmd_list_->ExecuteIndirect(draw_indexed_indirect_signature_.get(), 1,
						arg_buff, rl.IndirectArgsOffset(), nullptr, 0);
					pass.Unbind(effect);
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);
					this->UpdateRenderPSO(effect, pass, rl, has_tessellation);
					d3d_render_cmd_list_->ExecuteIndirect(draw_indirect_signature_.get(), 1,
						arg_buff, rl.IndirectArgsOffset(), nullptr, 0);
					pass.Unbind(effect);
				}
			}
		}
		else
		{
			if (rl.UseIndices())
			{
				uint32_t const num_indices = rl.NumIndices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);
					this->UpdateRenderPSO(effect, pass, rl, has_tessellation);
					d3d_render_cmd_list_->DrawIndexedInstanced(num_indices, num_instances, rl.StartIndexLocation(),
						rl.StartVertexLocation(), rl.StartInstanceLocation());
					pass.Unbind(effect);
				}
			}
			else
			{
				uint32_t const num_vertices = rl.NumVertices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					auto& pass = tech.Pass(i);

					pass.Bind(effect);
					this->UpdateRenderPSO(effect, pass, rl, has_tessellation);
					d3d_render_cmd_list_->DrawInstanced(num_vertices, num_instances,
						rl.StartVertexLocation(), rl.StartInstanceLocation());
					pass.Unbind(effect);
				}
			}
		}

		num_draws_just_called_ += num_passes;
	}

	void D3D12RenderEngine::DoDispatch(RenderEffect const & effect, RenderTechnique const & tech,
		uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++ i)
		{
			auto& pass = tech.Pass(i);

			pass.Bind(effect);
			this->UpdateComputePSO(effect, pass);
			d3d_render_cmd_list_->Dispatch(tgx, tgy, tgz);
			pass.Unbind(effect);
		}

		num_dispatches_just_called_ += num_passes;
	}

	void D3D12RenderEngine::DoDispatchIndirect(RenderEffect const & effect, RenderTechnique const & tech,
		GraphicsBufferPtr const & buff_args, uint32_t offset)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		auto& arg_buff = *checked_cast<D3D12GraphicsBuffer*>(buff_args.get());
		if (!(arg_buff.AccessHint() & (EAH_CPU_Read | EAH_CPU_Write)))
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			if (arg_buff.UpdateResourceBarrier(0, barrier, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT))
			{
				barriers.push_back(barrier);
			}
		}

		if (!barriers.empty())
		{
			d3d_render_cmd_list_->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
		}

		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++ i)
		{
			auto& pass = tech.Pass(i);

			pass.Bind(effect);
			this->UpdateComputePSO(effect, pass);
			d3d_render_cmd_list_->ExecuteIndirect(dispatch_indirect_signature_.get(), 1,
				checked_cast<D3D12GraphicsBuffer*>(buff_args.get())->D3DResource().get(), offset, nullptr, 0);
			pass.Unbind(effect);
		}

		num_dispatches_just_called_ += num_passes;
	}

	void D3D12RenderEngine::ForceFlush()
	{
		this->CommitRenderCmd();
		this->ResetRenderCmd();
	}

	void D3D12RenderEngine::ForceFinish()
	{
		curr_vbvs_.clear();
		curr_ibv_ = { 0, 0, DXGI_FORMAT_UNKNOWN };

		this->ForceFlush();
		this->SyncRenderCmd();

		this->ClearTempObjs();
	}

	TexturePtr const & D3D12RenderEngine::ScreenDepthStencilTexture() const
	{
		return checked_cast<D3D12RenderWindow*>(screen_frame_buffer_.get())->D3DDepthStencilBuffer();
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		scissor_rc_cache_.left = static_cast<LONG>(x);
		scissor_rc_cache_.top = static_cast<LONG>(y);
		scissor_rc_cache_.right = static_cast<LONG>(x + width);
		scissor_rc_cache_.bottom = static_cast<LONG>(y + height);
	}

	void D3D12RenderEngine::DoResize(uint32_t width, uint32_t height)
	{
		checked_cast<D3D12RenderWindow*>(screen_frame_buffer_.get())->Resize(width, height);
	}

	void D3D12RenderEngine::DoDestroy()
	{
		adapterList_.Destroy();

		uint64_t max_fence_val = 0;
		for (auto const & item : d3d_render_cmd_allocators_)
		{
			max_fence_val = std::max(max_fence_val, item.second);
		}
		render_cmd_fence_->Wait(max_fence_val);

		res_cmd_fence_.reset();
		render_cmd_fence_.reset();

		this->ClearTempObjs();

		temp_upload_free_buffs_.clear();
		temp_readback_free_buffs_.clear();

		so_buffs_.clear();
		root_signatures_.clear();
		graphics_psos_.clear();
		compute_psos_.clear();
		cbv_srv_uav_heaps_.clear();

		bilinear_blit_tech_ = nullptr;
		blit_effect_.reset();

		draw_indirect_signature_.reset();
		draw_indexed_indirect_signature_.reset();
		dispatch_indirect_signature_.reset();

		cbv_srv_uav_desc_heap_.reset();
		dsv_desc_heap_.reset();
		rtv_desc_heap_.reset();

		d3d_res_cmd_list_.reset();
		d3d_res_cmd_allocator_.reset();
		d3d_render_cmd_list_.reset();
		curr_render_cmd_allocator_.reset();
		d3d_render_cmd_allocators_.clear();
		d3d_render_cmd_queue_.reset();
		d3d_device_.reset();

		gi_factory_4_.reset();
		gi_factory_5_.reset();

		D3D12InterfaceLoader::Instance().Destroy();
	}

	void D3D12RenderEngine::DoSuspend()
	{
		IDXGIDevice3* dxgi_device = nullptr;
		d3d_device_->QueryInterface(IID_IDXGIDevice3, reinterpret_cast<void**>(&dxgi_device));
		if (dxgi_device != nullptr)
		{
			dxgi_device->Trim();
			dxgi_device->Release();
		}
	}

	void D3D12RenderEngine::DoResume()
	{
		// TODO
	}

	bool D3D12RenderEngine::FullScreen() const
	{
		return checked_cast<D3D12RenderWindow*>(screen_frame_buffer_.get())->FullScreen();
	}

	void D3D12RenderEngine::FullScreen(bool fs)
	{
		checked_cast<D3D12RenderWindow*>(screen_frame_buffer_.get())->FullScreen(fs);
	}

	bool D3D12RenderEngine::VertexFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(vertex_format_.begin(), vertex_format_.end(), elem_fmt);
		return (iter != vertex_format_.end()) && (*iter == elem_fmt);
	}

	bool D3D12RenderEngine::TextureFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(texture_format_.begin(), texture_format_.end(), elem_fmt);
		return (iter != texture_format_.end()) && (*iter == elem_fmt);
	}

	bool D3D12RenderEngine::RenderTargetFormatSupport(ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality)
	{
		auto iter = rendertarget_format_.find(elem_fmt);
		if (iter != rendertarget_format_.end())
		{
			for (auto const & p : iter->second)
			{
				if ((sample_count == p.first) && (sample_quality < p.second))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool D3D12RenderEngine::UAVFormatSupport(ElementFormat elem_fmt)
	{
		auto iter = std::lower_bound(uav_format_.begin(), uav_format_.end(), elem_fmt);
		return (iter != uav_format_.end()) && (*iter == elem_fmt);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderEngine::FillRenderDeviceCaps()
	{
		BOOST_ASSERT(d3d_device_);

		switch (d3d_feature_level_)
		{
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			caps_.max_shader_model
				= (d3d_feature_level_ >= D3D_FEATURE_LEVEL_12_0) ? ShaderModel(5, 1) : ShaderModel(5, 0);
			caps_.max_texture_width = caps_.max_texture_height = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
			caps_.max_texture_depth = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			caps_.max_texture_cube_size = D3D12_REQ_TEXTURECUBE_DIMENSION;
			caps_.max_texture_array_length = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
			caps_.max_vertex_texture_units = D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps_.max_pixel_texture_units = D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps_.max_geometry_texture_units = D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT;
			caps_.max_simultaneous_rts = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
			caps_.max_simultaneous_uavs = D3D12_PS_CS_UAV_REGISTER_COUNT;
			caps_.cs_support = true;
			caps_.tess_method = TM_Hardware;
			caps_.max_vertex_streams = D3D12_STANDARD_VERTEX_ELEMENT_COUNT;
			caps_.max_texture_anisotropy = D3D12_MAX_MAXANISOTROPY;
			break;

		default:
			KFL_UNREACHABLE("Invalid feature level");
		}

		{
			D3D12_FEATURE_DATA_ARCHITECTURE arch_feature;
			arch_feature.NodeIndex = 0;
			d3d_device_->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &arch_feature, sizeof(arch_feature));
			caps_.is_tbdr = arch_feature.TileBasedRenderer ? true : false;
		}
		caps_.hw_instancing_support = true;
		caps_.instance_id_support = true;
		caps_.stream_output_support = true;
		caps_.alpha_to_coverage_support = true;
		caps_.primitive_restart_support = true;
		caps_.multithread_rendering_support = true;
		caps_.multithread_res_creating_support = true;
		caps_.arbitrary_multithread_rendering_support = false;
		caps_.mrt_independent_bit_depths_support = true;
		caps_.independent_blend_support = true;
		caps_.draw_indirect_support = true;
		caps_.no_overwrite_support = true;
		caps_.full_npot_texture_support = true;
		caps_.render_to_texture_array_support = true;
		caps_.explicit_multi_sample_support = true;
		caps_.load_from_buffer_support = true;
		caps_.uavs_at_every_stage_support = (d3d_feature_level_ >= D3D_FEATURE_LEVEL_11_1);
		caps_.gs_support = true;
		caps_.hs_support = true;
		caps_.ds_support = true;

		bool check_uav_fmts = false;
		{
			D3D12_FEATURE_DATA_D3D12_OPTIONS feature_data;
			if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &feature_data, sizeof(feature_data))))
			{
				caps_.logic_op_support = feature_data.OutputMergerLogicOp ? true : false;
				caps_.rovs_support = feature_data.ROVsSupported ? true : false;

				if (feature_data.TypedUAVLoadAdditionalFormats)
				{
					check_uav_fmts = true;
				}
			}
			else
			{
				caps_.logic_op_support = false;
				caps_.rovs_support = false;
			}
		}

		if (!check_uav_fmts)
		{
			uav_format_.push_back(EF_R32F);
			uav_format_.push_back(EF_R32UI);
			uav_format_.push_back(EF_R32I);
		}

		std::pair<ElementFormat, DXGI_FORMAT> const fmts[] = 
		{
			std::make_pair(EF_A8, DXGI_FORMAT_A8_UNORM),
			std::make_pair(EF_R5G6B5, DXGI_FORMAT_B5G6R5_UNORM),
			std::make_pair(EF_A1RGB5, DXGI_FORMAT_B5G5R5A1_UNORM),
			std::make_pair(EF_ARGB4, DXGI_FORMAT_B4G4R4A4_UNORM),
			std::make_pair(EF_R8, DXGI_FORMAT_R8_UNORM),
			std::make_pair(EF_SIGNED_R8, DXGI_FORMAT_R8_SNORM),
			std::make_pair(EF_GR8, DXGI_FORMAT_R8G8_UNORM),
			std::make_pair(EF_SIGNED_GR8, DXGI_FORMAT_R8G8_SNORM),
			std::make_pair(EF_ARGB8, DXGI_FORMAT_B8G8R8A8_UNORM),
			std::make_pair(EF_ABGR8, DXGI_FORMAT_R8G8B8A8_UNORM),
			std::make_pair(EF_SIGNED_ABGR8, DXGI_FORMAT_R8G8B8A8_SNORM),
			std::make_pair(EF_A2BGR10, DXGI_FORMAT_R10G10B10A2_UNORM),
			std::make_pair(EF_SIGNED_A2BGR10, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM),
			std::make_pair(EF_R8UI, DXGI_FORMAT_R8_UINT),
			std::make_pair(EF_R8I, DXGI_FORMAT_R8_SINT),
			std::make_pair(EF_GR8UI, DXGI_FORMAT_R8G8_UINT),
			std::make_pair(EF_GR8I, DXGI_FORMAT_R8G8_SINT),
			std::make_pair(EF_ABGR8UI, DXGI_FORMAT_R8G8B8A8_UINT),
			std::make_pair(EF_ABGR8I, DXGI_FORMAT_R8G8B8A8_SINT),
			std::make_pair(EF_A2BGR10UI, DXGI_FORMAT_R10G10B10A2_UINT),
			std::make_pair(EF_R16, DXGI_FORMAT_R16_UNORM),
			std::make_pair(EF_SIGNED_R16, DXGI_FORMAT_R16_SNORM),
			std::make_pair(EF_GR16, DXGI_FORMAT_R16G16_UNORM),
			std::make_pair(EF_SIGNED_GR16, DXGI_FORMAT_R16G16_SNORM),
			std::make_pair(EF_ABGR16, DXGI_FORMAT_R16G16B16A16_UNORM),
			std::make_pair(EF_SIGNED_ABGR16, DXGI_FORMAT_R16G16B16A16_SNORM),
			std::make_pair(EF_R16UI, DXGI_FORMAT_R16_UINT),
			std::make_pair(EF_R16I, DXGI_FORMAT_R16_SINT),
			std::make_pair(EF_GR16UI, DXGI_FORMAT_R16G16_UINT),
			std::make_pair(EF_GR16I, DXGI_FORMAT_R16G16_SINT),
			std::make_pair(EF_ABGR16UI, DXGI_FORMAT_R16G16B16A16_UINT),
			std::make_pair(EF_ABGR16I, DXGI_FORMAT_R16G16B16A16_SINT),
			std::make_pair(EF_R32UI, DXGI_FORMAT_R32_UINT),
			std::make_pair(EF_R32I, DXGI_FORMAT_R32_SINT),
			std::make_pair(EF_GR32UI, DXGI_FORMAT_R32G32_UINT),
			std::make_pair(EF_GR32I, DXGI_FORMAT_R32G32_SINT),
			std::make_pair(EF_BGR32UI, DXGI_FORMAT_R32G32B32_UINT),
			std::make_pair(EF_BGR32I, DXGI_FORMAT_R32G32B32_SINT),
			std::make_pair(EF_ABGR32UI, DXGI_FORMAT_R32G32B32A32_UINT),
			std::make_pair(EF_ABGR32I, DXGI_FORMAT_R32G32B32A32_SINT),
			std::make_pair(EF_R16F, DXGI_FORMAT_R16_FLOAT),
			std::make_pair(EF_GR16F, DXGI_FORMAT_R16G16_FLOAT),
			std::make_pair(EF_B10G11R11F, DXGI_FORMAT_R11G11B10_FLOAT),
			std::make_pair(EF_ABGR16F, DXGI_FORMAT_R16G16B16A16_FLOAT),
			std::make_pair(EF_R32F, DXGI_FORMAT_R32_FLOAT),
			std::make_pair(EF_GR32F, DXGI_FORMAT_R32G32_FLOAT),
			std::make_pair(EF_BGR32F, DXGI_FORMAT_R32G32B32_FLOAT),
			std::make_pair(EF_ABGR32F, DXGI_FORMAT_R32G32B32A32_FLOAT),
			std::make_pair(EF_BC1, DXGI_FORMAT_BC1_UNORM),
			std::make_pair(EF_BC2, DXGI_FORMAT_BC2_UNORM),
			std::make_pair(EF_BC3, DXGI_FORMAT_BC3_UNORM),
			std::make_pair(EF_BC4, DXGI_FORMAT_BC4_UNORM),
			std::make_pair(EF_SIGNED_BC4, DXGI_FORMAT_BC4_SNORM),
			std::make_pair(EF_BC5, DXGI_FORMAT_BC5_UNORM),
			std::make_pair(EF_SIGNED_BC5, DXGI_FORMAT_BC5_SNORM),
			std::make_pair(EF_BC6, DXGI_FORMAT_BC6H_UF16),
			std::make_pair(EF_SIGNED_BC6, DXGI_FORMAT_BC6H_SF16),
			std::make_pair(EF_BC7, DXGI_FORMAT_BC7_UNORM),
			std::make_pair(EF_D16, DXGI_FORMAT_D16_UNORM),
			std::make_pair(EF_D24S8, DXGI_FORMAT_D24_UNORM_S8_UINT),
			std::make_pair(EF_D32F, DXGI_FORMAT_D32_FLOAT),
			std::make_pair(EF_ARGB8_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB),
			std::make_pair(EF_ABGR8_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
			std::make_pair(EF_BC1_SRGB, DXGI_FORMAT_BC1_UNORM_SRGB),
			std::make_pair(EF_BC2_SRGB, DXGI_FORMAT_BC2_UNORM_SRGB),
			std::make_pair(EF_BC3_SRGB, DXGI_FORMAT_BC3_UNORM_SRGB),
			std::make_pair(EF_BC7_SRGB, DXGI_FORMAT_BC7_UNORM_SRGB)
		};

		D3D12_FEATURE_DATA_FORMAT_SUPPORT fmt_support;
		for (auto const & fmt : fmts)
		{
			DXGI_FORMAT dxgi_fmt;
			if (IsDepthFormat(fmt.first))
			{
				switch (fmt.first)
				{
				case EF_D16:
					dxgi_fmt = DXGI_FORMAT_R16_TYPELESS;
					break;

				case EF_D24S8:
					dxgi_fmt = DXGI_FORMAT_R24G8_TYPELESS;
					break;

				case EF_D32F:
				default:
					dxgi_fmt = DXGI_FORMAT_R32_TYPELESS;
					break;
				}

				fmt_support.Format = dxgi_fmt;
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_NONE;
				fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					if (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
					{
						vertex_format_.push_back(fmt.first);
					}

					if ((fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE1D)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURECUBE)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE))
					{
						texture_format_.push_back(fmt.first);
					}
				}
			}
			else
			{
				dxgi_fmt = fmt.second;

				fmt_support.Format = dxgi_fmt;
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_NONE;
				fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					if (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
					{
						vertex_format_.push_back(fmt.first);
					}

					if ((fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE1D)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURECUBE)
						|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE))
					{
						texture_format_.push_back(fmt.first);
					}

					if (check_uav_fmts
						&& ((fmt_support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
						&& ((fmt_support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0))
					{
						uav_format_.push_back(fmt.first);
					}
				}
			}

			fmt_support.Format = fmt.second;
			fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_NONE;
			fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
			if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
			{
				if ((fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
					|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET)
					|| (fmt_support.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL))
				{
					D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaa_quality_levels;
					msaa_quality_levels.Format = dxgi_fmt;

					UINT count = 1;
					while (count <= D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT)
					{
						msaa_quality_levels.SampleCount = count;
						if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
							&msaa_quality_levels, sizeof(msaa_quality_levels))))
						{
							if (msaa_quality_levels.NumQualityLevels > 0)
							{
								rendertarget_format_[fmt.first].emplace_back(count, msaa_quality_levels.NumQualityLevels);
								count <<= 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
				}
			}
		}

		std::sort(vertex_format_.begin(), vertex_format_.end());
		vertex_format_.erase(std::unique(vertex_format_.begin(), vertex_format_.end()), vertex_format_.end());
		std::sort(texture_format_.begin(), texture_format_.end());
		texture_format_.erase(std::unique(texture_format_.begin(), texture_format_.end()), texture_format_.end());
		std::sort(uav_format_.begin(), uav_format_.end());
		uav_format_.erase(std::unique(uav_format_.begin(), uav_format_.end()), uav_format_.end());

		caps_.vertex_format_support = [this](ElementFormat elem_fmt)
			{
				return this->VertexFormatSupport(elem_fmt);
			};
		caps_.texture_format_support = [this](ElementFormat elem_fmt)
			{
				return this->TextureFormatSupport(elem_fmt);
			};
		caps_.rendertarget_format_support = [this](ElementFormat elem_fmt, uint32_t sample_count, uint32_t sample_quality)
			{
				return this->RenderTargetFormatSupport(elem_fmt, sample_count, sample_quality);
			};
		caps_.uav_format_support = [this](ElementFormat elem_fmt)
			{
				return this->UAVFormatSupport(elem_fmt);
			};

		caps_.depth_texture_support = (caps_.texture_format_support(EF_D24S8) || caps_.texture_format_support(EF_D16));
		caps_.fp_color_support = ((caps_.texture_format_support(EF_B10G11R11F) && caps_.rendertarget_format_support(EF_B10G11R11F, 1, 0))
			|| (caps_.texture_format_support(EF_ABGR16F) && caps_.rendertarget_format_support(EF_ABGR16F, 1, 0)));
		caps_.pack_to_rgba_required = !(caps_.texture_format_support(EF_R16F) && caps_.rendertarget_format_support(EF_R16F, 1, 0)
			&& caps_.texture_format_support(EF_R32F) && caps_.rendertarget_format_support(EF_R32F, 1, 0));
	}

	void D3D12RenderEngine::StereoscopicForLCDShutter(int32_t eye)
	{
		uint32_t const width = mono_tex_->Width(0);
		uint32_t const height = mono_tex_->Height(0);
		D3D12RenderWindowPtr win = checked_pointer_cast<D3D12RenderWindow>(screen_frame_buffer_);

		switch (stereo_method_)
		{
		case SM_DXGI:
			{
				D3D12RenderTargetRenderView* rtv = checked_cast<D3D12RenderTargetRenderView*>(
					(0 == eye) ? win->D3DBackBufferRTV().get() : win->D3DBackBufferRightEyeRTV().get());

				D3D12_CPU_DESCRIPTOR_HANDLE rt_handle = rtv->D3DRenderTargetView()->Handle();
				d3d_render_cmd_list_->OMSetRenderTargets(1, &rt_handle, false, nullptr);

				D3D12_VIEWPORT vp;
				vp.TopLeftX = 0;
				vp.TopLeftY = 0;
				vp.Width = static_cast<float>(width);
				vp.Height = static_cast<float>(height);
				vp.MinDepth = 0;
				vp.MaxDepth = 1;

				d3d_render_cmd_list_->RSSetViewports(1, &vp);
				stereoscopic_pp_->SetParam(3, eye);
				stereoscopic_pp_->Render();
			}
			break;

		default:
			break;
		}
	}

	void D3D12RenderEngine::OMSetStencilRef(uint16_t stencil_ref)
	{
		if (curr_stencil_ref_ != stencil_ref)
		{
			d3d_render_cmd_list_->OMSetStencilRef(stencil_ref);
			curr_stencil_ref_ = stencil_ref;
		}
	}

	void D3D12RenderEngine::OMSetBlendFactor(Color const & blend_factor)
	{
		if (curr_blend_factor_ != blend_factor)
		{
			d3d_render_cmd_list_->OMSetBlendFactor(&blend_factor.r());
			curr_blend_factor_ = blend_factor;
		}
	}

	void D3D12RenderEngine::RSSetViewports(UINT NumViewports, D3D12_VIEWPORT const * pViewports)
	{
		if (NumViewports == 1)
		{
			if (memcmp(&curr_viewport_, pViewports, sizeof(pViewports[0])) != 0)
			{
				d3d_render_cmd_list_->RSSetViewports(NumViewports, pViewports);
				curr_viewport_ = pViewports[0];
			}
		}
		else
		{
			d3d_render_cmd_list_->RSSetViewports(NumViewports, pViewports);
			curr_viewport_ = pViewports[0];
		}
	}

	void D3D12RenderEngine::SetPipelineState(ID3D12PipelineState* pso)
	{
		if (pso != curr_pso_)
		{
			d3d_render_cmd_list_->SetPipelineState(pso);
			curr_pso_ = pso;
		}
	}

	void D3D12RenderEngine::SetGraphicsRootSignature(ID3D12RootSignature* root_signature)
	{
		if (root_signature != curr_graphics_root_signature_)
		{
			d3d_render_cmd_list_->SetGraphicsRootSignature(root_signature);
			curr_graphics_root_signature_ = root_signature;
		}
	}

	void D3D12RenderEngine::SetComputeRootSignature(ID3D12RootSignature* root_signature)
	{
		if (root_signature != curr_compute_root_signature_)
		{
			d3d_render_cmd_list_->SetComputeRootSignature(root_signature);
			curr_compute_root_signature_ = root_signature;
		}
	}

	void D3D12RenderEngine::RSSetScissorRects(D3D12_RECT const & rect)
	{
		if (memcmp(&rect, &curr_scissor_rc_, sizeof(rect)) != 0)
		{
			d3d_render_cmd_list_->RSSetScissorRects(1, &rect);
			curr_scissor_rc_ = rect;
		}
	}

	void D3D12RenderEngine::IASetPrimitiveTopology(RenderLayout::topology_type primitive_topology)
	{
		if (topology_type_cache_ != primitive_topology)
		{
			topology_type_cache_ = primitive_topology;
			curr_topology_ = D3D12Mapping::Mapping(primitive_topology);
			d3d_render_cmd_list_->IASetPrimitiveTopology(curr_topology_);
		}
	}

	void D3D12RenderEngine::SetDescriptorHeaps(ArrayRef<ID3D12DescriptorHeap*> descriptor_heaps)
	{
		if ((descriptor_heaps.size() != curr_num_desc_heaps_)
			|| (descriptor_heaps != ArrayRef<ID3D12DescriptorHeap*>(curr_desc_heaps_.data(), curr_num_desc_heaps_)))
		{
			BOOST_ASSERT(descriptor_heaps.size() <= curr_desc_heaps_.size());
			curr_num_desc_heaps_ = static_cast<uint32_t>(descriptor_heaps.size());
			for (uint32_t i = 0; i < curr_num_desc_heaps_; ++ i)
			{
				curr_desc_heaps_[i] = descriptor_heaps[i];
			}

			d3d_render_cmd_list_->SetDescriptorHeaps(curr_num_desc_heaps_, curr_desc_heaps_.data());
		}
	}

	void D3D12RenderEngine::IASetVertexBuffers(uint32_t start_slot, ArrayRef<D3D12_VERTEX_BUFFER_VIEW> views)
	{
		if ((start_slot + views.size() > curr_vbvs_.size())
			|| (memcmp(&curr_vbvs_[start_slot], views.data(), views.size() * sizeof(views[0])) != 0))
		{
			curr_vbvs_.resize(std::max(curr_vbvs_.size(), start_slot + views.size()));
			memcpy(&curr_vbvs_[start_slot], views.data(), views.size() * sizeof(views[0]));
			d3d_render_cmd_list_->IASetVertexBuffers(start_slot, static_cast<uint32_t>(views.size()), views.data());
		}
	}

	void D3D12RenderEngine::IASetIndexBuffer(D3D12_INDEX_BUFFER_VIEW const & view)
	{
		if ((curr_ibv_.BufferLocation != view.BufferLocation)
			|| (curr_ibv_.SizeInBytes != view.SizeInBytes)
			|| (curr_ibv_.Format != view.Format))
		{
			d3d_render_cmd_list_->IASetIndexBuffer(&view);
			curr_ibv_ = view;
		}
	}

	uint32_t D3D12RenderEngine::AllocRTV()
	{
		for (size_t i = 0; i < rtv_heap_occupied_.size(); ++ i)
		{
			if (!rtv_heap_occupied_[i])
			{
				rtv_heap_occupied_[i] = true;
				return static_cast<uint32_t>(NUM_BACK_BUFFERS * 2 + i) * rtv_desc_size_;
			}
		}

		KFL_UNREACHABLE("Can't allocate more RTVs");
	}

	uint32_t D3D12RenderEngine::AllocDSV()
	{
		for (size_t i = 0; i < dsv_heap_occupied_.size(); ++ i)
		{
			if (!dsv_heap_occupied_[i])
			{
				dsv_heap_occupied_[i] = true;
				return static_cast<uint32_t>(2 + i) * dsv_desc_size_;
			}
		}

		KFL_UNREACHABLE("Can't allocate more DSVs");
	}

	uint32_t D3D12RenderEngine::AllocCBVSRVUAV()
	{
		for (size_t i = 0; i < cbv_srv_uav_heap_occupied_.size(); ++ i)
		{
			if (!cbv_srv_uav_heap_occupied_[i])
			{
				cbv_srv_uav_heap_occupied_[i] = true;
				return static_cast<uint32_t>(i) * cbv_srv_uav_desc_size_;
			}
		}
		KFL_UNREACHABLE("Can't allocate more CBVs/SRVs/UAVs");
	}

	void D3D12RenderEngine::DeallocRTV(uint32_t offset)
	{
		uint32_t const index = offset / rtv_desc_size_;
		if (index >= NUM_BACK_BUFFERS * 2)
		{
			rtv_heap_occupied_[index - NUM_BACK_BUFFERS * 2] = false;
		}
	}

	void D3D12RenderEngine::DeallocDSV(uint32_t offset)
	{
		uint32_t const index = offset / dsv_desc_size_;
		if (index >= 2)
		{
			dsv_heap_occupied_[index - 2] = false;
		}
	}

	void D3D12RenderEngine::DeallocCBVSRVUAV(uint32_t offset)
	{
		uint32_t const index = offset / cbv_srv_uav_desc_size_;
		cbv_srv_uav_heap_occupied_[index] = false;
	}

	ID3D12RootSignaturePtr const & D3D12RenderEngine::CreateRootSignature(
			std::array<uint32_t, ShaderObject::ST_NumShaderTypes * 4> const & num,
			bool has_vs, bool has_stream_output)
	{
		ID3D12RootSignaturePtr ret;

		size_t hash_val = 0;
		HashRange(hash_val, num.begin(), num.end());
		HashCombine(hash_val, has_vs);
		HashCombine(hash_val, has_stream_output);
		auto iter = root_signatures_.find(hash_val);
		if (iter == root_signatures_.end())
		{
			uint32_t num_cbv = 0;
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				num_cbv += num[i * 4 + 0];
			}

			std::vector<D3D12_ROOT_PARAMETER> root_params;
			std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
			ranges.reserve(num_cbv + ShaderObject::ST_NumShaderTypes * 3);
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				if (num[i * 4 + 0] != 0)
				{
					D3D12_ROOT_PARAMETER root_param;
					root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					switch (i)
					{
					case ShaderObject::ST_VertexShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
						break;

					case ShaderObject::ST_PixelShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
						break;

					case ShaderObject::ST_GeometryShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
						break;

					case ShaderObject::ST_HullShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_HULL;
						break;

					case ShaderObject::ST_DomainShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
						break;

					default:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
						break;
					}
					root_param.Descriptor.RegisterSpace = 0;
					for (uint32_t j = 0; j < num[i * 4 + 0]; ++ j)
					{
						root_param.Descriptor.ShaderRegister = j;
						root_params.push_back(root_param);
					}
				}
			}
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				if (num[i * 4 + 1] != 0)
				{
					D3D12_DESCRIPTOR_RANGE range;
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					range.NumDescriptors = num[i * 4 + 1];
					range.BaseShaderRegister = 0;
					range.RegisterSpace = 0;
					range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					ranges.push_back(range);

					D3D12_ROOT_PARAMETER root_param;
					root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					switch (i)
					{
					case ShaderObject::ST_VertexShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
						break;

					case ShaderObject::ST_PixelShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
						break;

					case ShaderObject::ST_GeometryShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
						break;

					case ShaderObject::ST_HullShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_HULL;
						break;

					case ShaderObject::ST_DomainShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
						break;

					default:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
						break;
					}
					root_param.DescriptorTable.NumDescriptorRanges = 1;
					root_param.DescriptorTable.pDescriptorRanges = &ranges.back();
					root_params.push_back(root_param);
				}
			}
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				if (num[i * 4 + 2] != 0)
				{
					D3D12_DESCRIPTOR_RANGE range;
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					range.NumDescriptors = num[i * 4 + 2];
					range.BaseShaderRegister = 0;
					range.RegisterSpace = 0;
					range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					ranges.push_back(range);

					D3D12_ROOT_PARAMETER root_param;
					root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					switch (i)
					{
					case ShaderObject::ST_VertexShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
						break;

					case ShaderObject::ST_PixelShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
						break;

					case ShaderObject::ST_GeometryShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
						break;

					case ShaderObject::ST_HullShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_HULL;
						break;

					case ShaderObject::ST_DomainShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
						break;

					default:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
						break;
					}
					root_param.DescriptorTable.NumDescriptorRanges = 1;
					root_param.DescriptorTable.pDescriptorRanges = &ranges.back();
					root_params.push_back(root_param);
				}
			}
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				if (num[i * 4 + 3] != 0)
				{
					D3D12_DESCRIPTOR_RANGE range;
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					range.NumDescriptors = num[i * 4 + 3];
					range.BaseShaderRegister = 0;
					range.RegisterSpace = 0;
					range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					ranges.push_back(range);

					D3D12_ROOT_PARAMETER root_param;
					root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					switch (i)
					{
					case ShaderObject::ST_VertexShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
						break;

					case ShaderObject::ST_PixelShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
						break;

					case ShaderObject::ST_GeometryShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
						break;

					case ShaderObject::ST_HullShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_HULL;
						break;

					case ShaderObject::ST_DomainShader:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
						break;

					default:
						root_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
						break;
					}
					root_param.DescriptorTable.NumDescriptorRanges = 1;
					root_param.DescriptorTable.pDescriptorRanges = &ranges.back();
					root_params.push_back(root_param);
				}
			}

			D3D12_ROOT_SIGNATURE_DESC root_signature_desc;
			root_signature_desc.NumParameters = static_cast<UINT>(root_params.size());
			root_signature_desc.pParameters = root_params.empty() ? nullptr : &root_params[0];
			root_signature_desc.NumStaticSamplers = 0;
			root_signature_desc.pStaticSamplers = nullptr;
			root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
			if (has_vs)
			{
				root_signature_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			}
			if (has_stream_output)
			{
				root_signature_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
			}

			ID3DBlob* signature = nullptr;
			ID3DBlob* error = nullptr;
			TIFHR(D3D12InterfaceLoader::Instance().D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
			ID3D12RootSignature* rs;
			TIFHR(d3d_device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
				IID_ID3D12RootSignature, reinterpret_cast<void**>(&rs)));
			signature->Release();
			if (error)
			{
				error->Release();
			}
			
			iter = root_signatures_.emplace(hash_val, MakeCOMPtr(rs)).first;
		}

		return iter->second;
	}

	ID3D12PipelineStatePtr const & D3D12RenderEngine::CreateRenderPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC const & desc)
	{
		char const * p = reinterpret_cast<char const *>(&desc);
		size_t hash_val = 0;
		HashRange(hash_val, p, p + sizeof(desc));

		auto iter = graphics_psos_.find(hash_val);
		if (iter == graphics_psos_.end())
		{
			ID3D12PipelineState* d3d_pso;
			TIFHR(d3d_device_->CreateGraphicsPipelineState(&desc, IID_ID3D12PipelineState, reinterpret_cast<void**>(&d3d_pso)));
			iter = graphics_psos_.emplace(hash_val, MakeCOMPtr(d3d_pso)).first;
		}

		return iter->second;
	}

	ID3D12PipelineStatePtr const & D3D12RenderEngine::CreateComputePSO(D3D12_COMPUTE_PIPELINE_STATE_DESC const & desc)
	{
		char const * p = reinterpret_cast<char const *>(&desc);
		size_t hash_val = 0;
		HashRange(hash_val, p, p + sizeof(desc));

		auto iter = compute_psos_.find(hash_val);
		if (iter == compute_psos_.end())
		{
			ID3D12PipelineState* d3d_pso;
			TIFHR(d3d_device_->CreateComputePipelineState(&desc, IID_ID3D12PipelineState, reinterpret_cast<void**>(&d3d_pso)));
			iter = compute_psos_.emplace(hash_val, MakeCOMPtr(d3d_pso)).first;
		}

		return iter->second;
	}

	ID3D12DescriptorHeapPtr D3D12RenderEngine::CreateDynamicCBVSRVUAVDescriptorHeap(uint32_t num)
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
		cbv_srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_srv_heap_desc.NumDescriptors = num;
		cbv_srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbv_srv_heap_desc.NodeMask = 0;
		ID3D12DescriptorHeap* csu_heap;
		TIFHR(d3d_device_->CreateDescriptorHeap(&cbv_srv_heap_desc, IID_ID3D12DescriptorHeap, reinterpret_cast<void**>(&csu_heap)));
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap = MakeCOMPtr(csu_heap);
		curr_render_cmd_allocator_->cbv_srv_uav_heap_cache_.push_back(cbv_srv_uav_heap);
		return cbv_srv_uav_heap;
	}

	ID3D12ResourcePtr D3D12RenderEngine::AllocTempBuffer(bool is_upload, uint32_t size_in_byte)
	{
		ID3D12ResourcePtr ret;

		auto& buffs = (is_upload) ? temp_upload_free_buffs_ : temp_readback_free_buffs_;

		auto iter = buffs.lower_bound(size_in_byte);
		if ((iter != buffs.end()) && (iter->first == size_in_byte))
		{
			ret = iter->second;

			buffs.erase(iter);
		}
		else
		{
			D3D12_RESOURCE_STATES init_state;
			D3D12_HEAP_PROPERTIES heap_prop;
			if (is_upload)
			{
				init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
				heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
			}
			else
			{
				init_state = D3D12_RESOURCE_STATE_COPY_DEST;
				heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
			}
			heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_prop.CreationNodeMask = 0;
			heap_prop.VisibleNodeMask = 0;

			D3D12_RESOURCE_DESC res_desc;
			res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			res_desc.Alignment = 0;
			res_desc.Width = size_in_byte;
			res_desc.Height = 1;
			res_desc.DepthOrArraySize = 1;
			res_desc.MipLevels = 1;
			res_desc.Format = DXGI_FORMAT_UNKNOWN;
			res_desc.SampleDesc.Count = 1;
			res_desc.SampleDesc.Quality = 0;
			res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

			ID3D12Resource* buffer;
			TIFHR(d3d_device_->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
				&res_desc, init_state, nullptr,
				IID_ID3D12Resource, reinterpret_cast<void**>(&buffer)));
			ret = MakeCOMPtr(buffer);
		}

		return ret;
	}

	void D3D12RenderEngine::RecycleTempBuffer(ID3D12ResourcePtr const & buff, bool is_upload, uint32_t size_in_byte)
	{
		if (buff)
		{
			auto& buffs = is_upload ? curr_render_cmd_allocator_->recycle_after_sync_upload_buffs_
				: curr_render_cmd_allocator_->recycle_after_sync_readback_buffs_;
			buffs.emplace_back(buff, size_in_byte);
		}
	}
	
	void D3D12RenderEngine::ReleaseAfterSync(ID3D12ResourcePtr const & buff)
	{
		if (buff)
		{
			curr_render_cmd_allocator_->release_after_sync_buffs_.push_back(buff);
		}
	}

	std::shared_ptr<D3D12RenderEngine::CmdAllocatorDependencies> D3D12RenderEngine::AllocCmdAllocator()
	{
		std::shared_ptr<CmdAllocatorDependencies> ret;
		for (auto iter = d3d_render_cmd_allocators_.begin(); iter != d3d_render_cmd_allocators_.end(); ++ iter)
		{
			if (render_cmd_fence_->Completed(iter->second))
			{
				ret = iter->first;
				d3d_render_cmd_allocators_.erase(iter);
				ret->cmd_allocator->Reset();
				break;
			}
		}

		if (!ret)
		{
			ret = MakeSharedPtr<CmdAllocatorDependencies>();

			ID3D12CommandAllocator* d3d_render_cmd_allocator;
			TIFHR(d3d_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_ID3D12CommandAllocator, reinterpret_cast<void**>(&d3d_render_cmd_allocator)));
			ret->cmd_allocator = MakeCOMPtr(d3d_render_cmd_allocator);
		}

		return ret;
	}

	void D3D12RenderEngine::RecycleCmdAllocator(std::shared_ptr<CmdAllocatorDependencies> const & cmd_allocator, uint64_t fence_val)
	{
		if (cmd_allocator)
		{
			d3d_render_cmd_allocators_.emplace_back(cmd_allocator, fence_val);
		}
	}
}
