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
#include <KFL/ThrowErr.hpp>
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

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/functional/hash.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D12RenderEngine::D3D12RenderEngine()
		: last_engine_type_(ET_Render), inv_timestamp_freq_(0),
			render_cmd_fence_val_(0), compute_cmd_fence_val_(0), copy_cmd_fence_val_(0),
			res_cmd_fence_val_(0)
	{
		native_shader_fourcc_ = MakeFourCC<'D', 'X', 'B', 'C'>::value;
		native_shader_version_ = 5;

		IDXGIFactory4* gi_factory;
		TIF(D3D12InterfaceLoader::Instance().CreateDXGIFactory1(IID_IDXGIFactory4, reinterpret_cast<void**>(&gi_factory)));
		gi_factory_ = MakeCOMPtr(gi_factory);

		adapterList_.Enumerate(gi_factory_);
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

	// 获取D3D接口
	/////////////////////////////////////////////////////////////////////////////////
	IDXGIFactory4Ptr const & D3D12RenderEngine::DXGIFactory() const
	{
		return gi_factory_;
	}

	ID3D12DevicePtr const & D3D12RenderEngine::D3DDevice() const
	{
		return d3d_device_;
	}

	ID3D12CommandQueuePtr const & D3D12RenderEngine::D3DRenderCmdQueue() const
	{
		return d3d_render_cmd_queue_;
	}

	ID3D12CommandAllocatorPtr const & D3D12RenderEngine::D3DRenderCmdAllocator() const
	{
		return d3d_render_cmd_allocator_;
	}

	ID3D12GraphicsCommandListPtr const & D3D12RenderEngine::D3DRenderCmdList() const
	{
		return d3d_render_cmd_list_;
	}

	ID3D12CommandQueuePtr const & D3D12RenderEngine::D3DComputeCmdQueue() const
	{
		return d3d_compute_cmd_queue_;
	}

	ID3D12CommandAllocatorPtr const & D3D12RenderEngine::D3DComputeCmdAllocator() const
	{
		return d3d_compute_cmd_allocator_;
	}

	ID3D12GraphicsCommandListPtr const & D3D12RenderEngine::D3DComputeCmdList() const
	{
		return d3d_compute_cmd_list_;
	}

	ID3D12CommandQueuePtr const & D3D12RenderEngine::D3DCopyCmdQueue() const
	{
		return d3d_copy_cmd_queue_;
	}

	ID3D12CommandAllocatorPtr const & D3D12RenderEngine::D3DCopyCmdAllocator() const
	{
		return d3d_copy_cmd_allocator_;
	}

	ID3D12GraphicsCommandListPtr const & D3D12RenderEngine::D3DCopyCmdList() const
	{
		return d3d_copy_cmd_list_;
	}

	ID3D12CommandAllocatorPtr const & D3D12RenderEngine::D3DResCmdAllocator() const
	{
		return d3d_res_cmd_allocator_;
	}

	ID3D12GraphicsCommandListPtr const & D3D12RenderEngine::D3DResCmdList() const
	{
		return d3d_res_cmd_list_;
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
	D3D12AdapterPtr const & D3D12RenderEngine::ActiveAdapter() const
	{
		return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderEngine::DoCreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		motion_frames_ = settings.motion_frames;

		D3D12RenderWindowPtr win = MakeSharedPtr<D3D12RenderWindow>(gi_factory_, this->ActiveAdapter(),
			name, settings);

		switch (d3d_feature_level_)
		{
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			vs_profile_ = "vs_5_0";
			ps_profile_ = "ps_5_0";
			gs_profile_ = "gs_5_0";
			cs_profile_ = "cs_5_0";
			hs_profile_ = "hs_5_0";
			ds_profile_ = "ds_5_0";
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		this->ResetRenderStates();
		this->BindFrameBuffer(win);

		if (STM_LCDShutter == settings.stereo_method)
		{
			stereo_method_ = SM_None;

			IDXGIFactory2* factory;
			gi_factory_->QueryInterface(IID_IDXGIFactory2, reinterpret_cast<void**>(&factory));
			if (factory != nullptr)
			{
				if (factory->IsWindowedStereoEnabled())
				{
					stereo_method_ = SM_DXGI;
				}
				factory->Release();
			}
		}

		blit_effect_ = SyncLoadRenderEffect("Copy.fxml");
		bilinear_blit_tech_ = blit_effect_->TechniqueByName("BilinearCopy");
	}

	void D3D12RenderEngine::CheckConfig(RenderSettings& settings)
	{
		KFL_UNUSED(settings);
	}

	void D3D12RenderEngine::D3DDevice(ID3D12DevicePtr const & device, ID3D12CommandQueuePtr const & cmd_queue, D3D_FEATURE_LEVEL feature_level)
	{
		d3d_device_ = device;
		d3d_render_cmd_queue_ = cmd_queue;
		d3d_feature_level_ = feature_level;

		Verify(!!d3d_render_cmd_queue_);
		Verify(!!d3d_device_);

		ID3D12CommandAllocator* d3d_render_cmd_allocator;
		TIF(d3d_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_ID3D12CommandAllocator, reinterpret_cast<void**>(&d3d_render_cmd_allocator)));
		d3d_render_cmd_allocator_ = MakeCOMPtr(d3d_render_cmd_allocator);

		ID3D12GraphicsCommandList* d3d_render_cmd_list;
		TIF(d3d_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3d_render_cmd_allocator_.get(), nullptr,
			IID_ID3D12GraphicsCommandList, reinterpret_cast<void**>(&d3d_render_cmd_list)));
		d3d_render_cmd_list_ = MakeCOMPtr(d3d_render_cmd_list);

		D3D12_COMMAND_QUEUE_DESC queue_desc;
		queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		queue_desc.Priority = 0;
		queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queue_desc.NodeMask = 0;

		ID3D12CommandQueue* d3d_compute_cmd_queue;
		TIF(d3d_device_->CreateCommandQueue(&queue_desc,
			IID_ID3D12CommandQueue, reinterpret_cast<void**>(&d3d_compute_cmd_queue)));
		d3d_compute_cmd_queue_ = MakeCOMPtr(d3d_compute_cmd_queue);

		ID3D12CommandAllocator* d3d_compute_cmd_allocator;
		TIF(d3d_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
			IID_ID3D12CommandAllocator, reinterpret_cast<void**>(&d3d_compute_cmd_allocator)));
		d3d_compute_cmd_allocator_ = MakeCOMPtr(d3d_compute_cmd_allocator);

		ID3D12GraphicsCommandList* d3d_compute_cmd_list;
		TIF(d3d_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, d3d_compute_cmd_allocator_.get(), nullptr,
			IID_ID3D12GraphicsCommandList, reinterpret_cast<void**>(&d3d_compute_cmd_list)));
		d3d_compute_cmd_list_ = MakeCOMPtr(d3d_compute_cmd_list);

		queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

		ID3D12CommandQueue* d3d_copy_cmd_queue;
		TIF(d3d_device_->CreateCommandQueue(&queue_desc,
			IID_ID3D12CommandQueue, reinterpret_cast<void**>(&d3d_copy_cmd_queue)));
		d3d_copy_cmd_queue_ = MakeCOMPtr(d3d_copy_cmd_queue);

		ID3D12CommandAllocator* d3d_copy_cmd_allocator;
		TIF(d3d_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY,
			IID_ID3D12CommandAllocator, reinterpret_cast<void**>(&d3d_copy_cmd_allocator)));
		d3d_copy_cmd_allocator_ = MakeCOMPtr(d3d_copy_cmd_allocator);

		ID3D12GraphicsCommandList* d3d_copy_cmd_list;
		TIF(d3d_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, d3d_copy_cmd_allocator_.get(), nullptr,
			IID_ID3D12GraphicsCommandList, reinterpret_cast<void**>(&d3d_copy_cmd_list)));
		d3d_copy_cmd_list_ = MakeCOMPtr(d3d_copy_cmd_list);

		ID3D12CommandAllocator* d3d_res_cmd_allocator;
		TIF(d3d_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_ID3D12CommandAllocator, reinterpret_cast<void**>(&d3d_res_cmd_allocator)));
		d3d_res_cmd_allocator_ = MakeCOMPtr(d3d_res_cmd_allocator);

		ID3D12GraphicsCommandList* d3d_res_cmd_list;
		TIF(d3d_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3d_res_cmd_allocator_.get(), nullptr,
			IID_ID3D12GraphicsCommandList, reinterpret_cast<void**>(&d3d_res_cmd_list)));
		d3d_res_cmd_list_ = MakeCOMPtr(d3d_res_cmd_list);

		D3D12_DESCRIPTOR_HEAP_DESC rtv_desc_heap;
		rtv_desc_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtv_desc_heap.NumDescriptors = NUM_BACK_BUFFERS * 2 + NUM_MAX_RENDER_TARGET_VIEWS;
		rtv_desc_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtv_desc_heap.NodeMask = 0;
		ID3D12DescriptorHeap* rtv_descriptor_heap;
		TIF(d3d_device_->CreateDescriptorHeap(&rtv_desc_heap, IID_ID3D12DescriptorHeap,
			reinterpret_cast<void**>(&rtv_descriptor_heap)));
		rtv_desc_heap_ = MakeCOMPtr(rtv_descriptor_heap);
		rtv_desc_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsv_desc_heap;
		dsv_desc_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsv_desc_heap.NumDescriptors = 2 + NUM_MAX_DEPTH_STENCIL_VIEWS;
		dsv_desc_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsv_desc_heap.NodeMask = 0;
		ID3D12DescriptorHeap* dsv_descriptor_heap;
		TIF(d3d_device_->CreateDescriptorHeap(&dsv_desc_heap, IID_ID3D12DescriptorHeap,
			reinterpret_cast<void**>(&dsv_descriptor_heap)));
		dsv_desc_heap_ = MakeCOMPtr(dsv_descriptor_heap);
		dsv_desc_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_uav_desc_heap;
		cbv_srv_uav_desc_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_srv_uav_desc_heap.NumDescriptors = NUM_MAX_CBV_SRV_UAVS;
		cbv_srv_uav_desc_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		cbv_srv_uav_desc_heap.NodeMask = 0;
		ID3D12DescriptorHeap* cbv_srv_uav_descriptor_heap;
		TIF(d3d_device_->CreateDescriptorHeap(&cbv_srv_uav_desc_heap, IID_ID3D12DescriptorHeap,
			reinterpret_cast<void**>(&cbv_srv_uav_descriptor_heap)));
		cbv_srv_uav_desc_heap_ = MakeCOMPtr(cbv_srv_uav_descriptor_heap);
		cbv_srv_uav_desc_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		rtv_heap_occupied_.assign(NUM_MAX_RENDER_TARGET_VIEWS, false);
		dsv_heap_occupied_.assign(NUM_MAX_DEPTH_STENCIL_VIEWS, false);
		cbv_srv_uav_heap_occupied_.assign(NUM_MAX_CBV_SRV_UAVS, false);

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
		compute_cmd_fence_ = rf.MakeFence();
		copy_cmd_fence_ = rf.MakeFence();

		this->FillRenderDeviceCaps();
	}

	void D3D12RenderEngine::ClearPSOCache()
	{
		cbv_srv_uav_heap_cache_.clear();
		pso_cache_.clear();
		remove_res_after_sync_.clear();
	}

	void D3D12RenderEngine::CommitResCmd()
	{
		TIF(d3d_res_cmd_list_->Close());
		ID3D12CommandList* cmd_lists[] = { d3d_res_cmd_list_.get() };
		d3d_render_cmd_queue_->ExecuteCommandLists(sizeof(cmd_lists) / sizeof(cmd_lists[0]), cmd_lists);

		res_cmd_fence_val_ = res_cmd_fence_->Signal(Fence::FT_Render);
		res_cmd_fence_->Wait(res_cmd_fence_val_);

		d3d_res_cmd_allocator_->Reset();
		d3d_res_cmd_list_->Reset(d3d_res_cmd_allocator_.get(), nullptr);
	}

	void D3D12RenderEngine::CommitRenderCmd()
	{
		TIF(d3d_render_cmd_list_->Close());
		ID3D12CommandList* cmd_lists[] = { d3d_render_cmd_list_.get() };
		d3d_render_cmd_queue_->ExecuteCommandLists(sizeof(cmd_lists) / sizeof(cmd_lists[0]), cmd_lists);
	}

	void D3D12RenderEngine::CommitComputeCmd()
	{
		TIF(d3d_compute_cmd_list_->Close());
		ID3D12CommandList* cmd_lists[] = { d3d_compute_cmd_list_.get() };
		d3d_compute_cmd_queue_->ExecuteCommandLists(sizeof(cmd_lists) / sizeof(cmd_lists[0]), cmd_lists);
	}

	void D3D12RenderEngine::CommitCopyCmd()
	{
		TIF(d3d_copy_cmd_list_->Close());
		ID3D12CommandList* cmd_lists[] = { d3d_copy_cmd_list_.get() };
		d3d_copy_cmd_queue_->ExecuteCommandLists(sizeof(cmd_lists) / sizeof(cmd_lists[0]), cmd_lists);
	}

	void D3D12RenderEngine::SyncRenderCmd()
	{
		render_cmd_fence_val_ = render_cmd_fence_->Signal(Fence::FT_Render);
		render_cmd_fence_->Wait(render_cmd_fence_val_);
	}

	void D3D12RenderEngine::SyncComputeCmd()
	{
		compute_cmd_fence_val_ = compute_cmd_fence_->Signal(Fence::FT_Compute);
		compute_cmd_fence_->Wait(compute_cmd_fence_val_);
	}

	void D3D12RenderEngine::SyncCopyCmd()
	{
		copy_cmd_fence_val_ = copy_cmd_fence_->Signal(Fence::FT_Copy);
		copy_cmd_fence_->Wait(copy_cmd_fence_val_);
	}

	void D3D12RenderEngine::ResetRenderCmd()
	{
		d3d_render_cmd_allocator_->Reset();
		d3d_render_cmd_list_->Reset(d3d_render_cmd_allocator_.get(), nullptr);
	}

	void D3D12RenderEngine::ResetComputeCmd()
	{
		d3d_compute_cmd_allocator_->Reset();
		d3d_compute_cmd_list_->Reset(d3d_compute_cmd_allocator_.get(), nullptr);
	}

	void D3D12RenderEngine::ResetCopyCmd()
	{
		d3d_copy_cmd_allocator_->Reset();
		d3d_copy_cmd_list_->Reset(d3d_copy_cmd_allocator_.get(), nullptr);
	}

	void D3D12RenderEngine::ResetRenderStates()
	{
		RasterizerStateDesc default_rs_desc;
		DepthStencilStateDesc default_dss_desc;
		BlendStateDesc default_bs_desc;

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		cur_rs_obj_ = rf.MakeRasterizerStateObject(default_rs_desc);
		cur_dss_obj_ = rf.MakeDepthStencilStateObject(default_dss_desc);
		cur_bs_obj_ = rf.MakeBlendStateObject(default_bs_desc);

		stencil_ref_cache_ = 0;
		blend_factor_cache_ = Color(1, 1, 1, 1);
		sample_mask_cache_ = 0xFFFFFFFF;

		topology_type_cache_ = RenderLayout::TT_PointList;
		d3d_render_cmd_list_->IASetPrimitiveTopology(D3D12Mapping::Mapping(topology_type_cache_));

		memset(&viewport_cache_, 0, sizeof(viewport_cache_));
		memset(&scissor_rc_cache_, 0, sizeof(scissor_rc_cache_));

		this->ClearPSOCache();
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
				sobv[i].BufferLocation = d3d12_buf->D3DBuffer()->GetGPUVirtualAddress();
				sobv[i].SizeInBytes = d3d12_buf->Size();
				sobv[i].BufferFilledSizeLocation = sobv[i].BufferLocation + d3d12_buf->CounterOffset();
			}

			d3d_render_cmd_list_->SOSetTargets(0, static_cast<UINT>(num_buffs), &sobv[0]);
		}
		else
		{
			d3d_render_cmd_list_->SOSetTargets(0, 0, nullptr);

			so_buffs_.clear();
		}
	}

	void D3D12RenderEngine::UpdateRenderPSO(RenderTechnique const & tech, RenderPassPtr const & pass, RenderLayout const & rl)
	{
		D3D12RenderLayout const & d3d12_rl = *checked_cast<D3D12RenderLayout const *>(&rl);

		D3D12ShaderObjectPtr so = checked_pointer_cast<D3D12ShaderObject>(pass->GetShaderObject());

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;
		pso_desc.pRootSignature = so->RootSignature().get();
		{
			auto const & blob = so->ShaderBlob(ShaderObject::ST_VertexShader);
			if (blob && !blob->empty())
			{
				pso_desc.VS.pShaderBytecode = blob->data();
				pso_desc.VS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.VS.pShaderBytecode = nullptr;
				pso_desc.VS.BytecodeLength = 0;
			}
		}
		{
			auto const & blob = so->ShaderBlob(ShaderObject::ST_PixelShader);
			if (blob && !blob->empty())
			{
				pso_desc.PS.pShaderBytecode = blob->data();
				pso_desc.PS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.PS.pShaderBytecode = nullptr;
				pso_desc.PS.BytecodeLength = 0;
			}
		}
		{
			auto const & blob = so->ShaderBlob(ShaderObject::ST_DomainShader);
			if (blob && !blob->empty())
			{
				pso_desc.DS.pShaderBytecode = blob->data();
				pso_desc.DS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.DS.pShaderBytecode = nullptr;
				pso_desc.DS.BytecodeLength = 0;
			}
		}
		{
			auto const & blob = so->ShaderBlob(ShaderObject::ST_HullShader);
			if (blob && !blob->empty())
			{
				pso_desc.HS.pShaderBytecode = blob->data();
				pso_desc.HS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.HS.pShaderBytecode = nullptr;
				pso_desc.HS.BytecodeLength = 0;
			}
		}
		{
			auto const & blob = so->ShaderBlob(ShaderObject::ST_GeometryShader);
			if (blob && !blob->empty())
			{
				pso_desc.GS.pShaderBytecode = blob->data();
				pso_desc.GS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.GS.pShaderBytecode = nullptr;
				pso_desc.GS.BytecodeLength = 0;
			}
		}

		auto const & so_decls = so->SODecl();
		std::vector<UINT> so_strides(so_decls.size());
		for (size_t i = 0; i < so_decls.size(); ++ i)
		{
			so_strides[i] = so_decls[i].ComponentCount * sizeof(float);
		}
		pso_desc.StreamOutput.pSODeclaration = so_decls.empty() ? nullptr : &so_decls[0];
		pso_desc.StreamOutput.NumEntries = static_cast<UINT>(so_decls.size());
		pso_desc.StreamOutput.pBufferStrides = so_strides.empty() ? nullptr : &so_strides[0];
		pso_desc.StreamOutput.NumStrides = static_cast<UINT>(so_strides.size());
		pso_desc.StreamOutput.RasterizedStream = so->RasterizedStream();

		pso_desc.BlendState = checked_pointer_cast<D3D12BlendStateObject>(pass->GetBlendStateObject())->D3DDesc();
		pso_desc.SampleMask = sample_mask_cache_;
		pso_desc.RasterizerState = checked_pointer_cast<D3D12RasterizerStateObject>(pass->GetRasterizerStateObject())->D3DDesc();
		pso_desc.DepthStencilState = checked_pointer_cast<D3D12DepthStencilStateObject>(pass->GetDepthStencilStateObject())->D3DDesc();
		pso_desc.InputLayout.pInputElementDescs = d3d12_rl.InputElementDesc().empty() ? nullptr : &d3d12_rl.InputElementDesc()[0];
		pso_desc.InputLayout.NumElements = static_cast<UINT>(d3d12_rl.InputElementDesc().size());
		pso_desc.IBStripCutValue = (EF_R16UI == rl.IndexStreamFormat())
			? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;

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
		pso_desc.PrimitiveTopologyType = D3D12Mapping::MappingPriTopoType(tt);

		pso_desc.NumRenderTargets = 0;
		FrameBufferPtr const & fb = this->CurFrameBuffer();
		for (int i = sizeof(pso_desc.RTVFormats) / sizeof(pso_desc.RTVFormats[0]) - 1; i >= 0; -- i)
		{
			if (fb->Attached(FrameBuffer::ATT_Color0 + i))
			{
				pso_desc.NumRenderTargets = i + 1;
				break;
			}
		}
		for (uint32_t i = 0; i < pso_desc.NumRenderTargets; ++ i)
		{
			pso_desc.RTVFormats[i] = D3D12Mapping::MappingFormat(fb->Attached(FrameBuffer::ATT_Color0 + i)->Format());
		}
		for (uint32_t i = pso_desc.NumRenderTargets; i < sizeof(pso_desc.RTVFormats) / sizeof(pso_desc.RTVFormats[0]); ++ i)
		{
			pso_desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}
		if (fb->Attached(FrameBuffer::ATT_DepthStencil))
		{
			pso_desc.DSVFormat = D3D12Mapping::MappingFormat(fb->Attached(FrameBuffer::ATT_DepthStencil)->Format());
		}
		else
		{
			pso_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		}
		pso_desc.SampleDesc.Count = 1;
		pso_desc.SampleDesc.Quality = 0;
		pso_desc.NodeMask = 0;
		pso_desc.CachedPSO.pCachedBlob = nullptr;
		pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
		pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		ID3D12PipelineStatePtr pso = this->CreateRenderPSO(pso_desc);
		pso_cache_.push_back(pso);

		d3d_render_cmd_list_->SetPipelineState(pso.get());
		d3d_render_cmd_list_->SetGraphicsRootSignature(so->RootSignature().get());

		if (pass->GetRasterizerStateObject()->GetDesc().scissor_enable)
		{
			d3d_render_cmd_list_->RSSetScissorRects(1, &scissor_rc_cache_);
		}
		else
		{
			D3D12_RECT rc =
			{
				static_cast<LONG>(viewport_cache_.TopLeftX),
				static_cast<LONG>(viewport_cache_.TopLeftY),
				static_cast<LONG>(viewport_cache_.TopLeftX + viewport_cache_.Width),
				static_cast<LONG>(viewport_cache_.TopLeftY + viewport_cache_.Height)
			};
			d3d_render_cmd_list_->RSSetScissorRects(1, &rc);
		}

		size_t num_handle = 0;
		for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
			num_handle += so->SRVs(st).size() + so->UAVs(st).size();
		}

		std::array<ID3D12DescriptorHeap*, 2> heaps;
		uint32_t num_heaps = 0;
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap;
		ID3D12DescriptorHeapPtr sampler_heap = so->SamplerHeap();
		if (num_handle > 0)
		{
			size_t hash_val = 0;
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
				boost::hash_combine(hash_val, st);
				boost::hash_combine(hash_val, so->SRVs(st).size());
				if (!so->SRVs(st).empty())
				{
					boost::hash_range(hash_val, so->SRVs(st).begin(), so->SRVs(st).end());
				}
			}
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
				boost::hash_combine(hash_val, st);
				boost::hash_combine(hash_val, so->UAVs(st).size());
				if (!so->UAVs(st).empty())
				{
					boost::hash_range(hash_val, so->UAVs(st).begin(), so->UAVs(st).end());
				}
			}

			auto iter = cbv_srv_uav_heaps_.find(hash_val);
			if (iter == cbv_srv_uav_heaps_.end())
			{
				cbv_srv_uav_heap = this->CreateDynamicCBVSRVUAVDescriptorHeap(static_cast<uint32_t>(num_handle));
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
			heaps[num_heaps] = sampler_heap.get();
			++ num_heaps;
		}

		if (num_heaps > 0)
		{
			d3d_render_cmd_list_->SetDescriptorHeaps(num_heaps, &heaps[0]);
		}

		uint32_t root_param_index = 0;
		for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
			if (!so->CBuffers(st).empty())
			{
				for (uint32_t j = 0; j < so->CBuffers(st).size(); ++ j)
				{
					ID3D12ResourcePtr const & buff = checked_cast<D3D12GraphicsBuffer*>(so->CBuffers(st)[j])->D3DBuffer();
					if (buff)
					{
						d3d_render_cmd_list_->SetGraphicsRootConstantBufferView(root_param_index, buff->GetGPUVirtualAddress());

						++ root_param_index;
					}
					else
					{
						d3d_render_cmd_list_->SetGraphicsRootConstantBufferView(root_param_index, 0);
					}
				}
			}
		}
		if (cbv_srv_uav_heap)
		{
			UINT const cbv_srv_uav_desc_size = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			D3D12_CPU_DESCRIPTOR_HANDLE cpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE gpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();

			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
				if (!so->SRVs(st).empty())
				{
					d3d_render_cmd_list_->SetGraphicsRootDescriptorTable(root_param_index, gpu_cbv_srv_uav_handle);

					for (uint32_t j = 0; j < so->SRVs(st).size(); ++ j)
					{
						d3d_device_->CopyDescriptorsSimple(1, cpu_cbv_srv_uav_handle,
							std::get<0>(so->SRVSrcs(st)[j]) ? so->SRVs(st)[j]->Handle() : null_srv_handle_,
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						cpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size;
						gpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size;
					}

					++ root_param_index;
				}
			}
			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
				if (!so->UAVs(st).empty())
				{
					d3d_render_cmd_list_->SetGraphicsRootDescriptorTable(root_param_index, gpu_cbv_srv_uav_handle);

					for (uint32_t j = 0; j < so->UAVs(st).size(); ++ j)
					{
						d3d_device_->CopyDescriptorsSimple(1, cpu_cbv_srv_uav_handle,
							so->UAVSrcs(st)[j].first ? so->UAVs(st)[j]->Handle() : null_uav_handle_,
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						cpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size;
						gpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size;
					}

					++ root_param_index;
				}
			}
		}

		if (sampler_heap)
		{
			UINT const sampler_desc_size = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

			D3D12_GPU_DESCRIPTOR_HANDLE gpu_sampler_handle = sampler_heap->GetGPUDescriptorHandleForHeapStart();

			for (uint32_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
			{
				ShaderObject::ShaderType st = static_cast<ShaderObject::ShaderType>(i);
				if (!so->Samplers(st).empty())
				{
					d3d_render_cmd_list_->SetGraphicsRootDescriptorTable(root_param_index, gpu_sampler_handle);

					gpu_sampler_handle.ptr += sampler_desc_size * so->Samplers(st).size();

					++ root_param_index;
				}
			}
		}
	}

	void D3D12RenderEngine::UpdateComputePSO(RenderPassPtr const & pass)
	{
		D3D12ShaderObjectPtr so = checked_pointer_cast<D3D12ShaderObject>(pass->GetShaderObject());

		D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc;
		pso_desc.pRootSignature = so->RootSignature().get();
		{
			auto const & blob = so->ShaderBlob(ShaderObject::ST_ComputeShader);
			if (blob && !blob->empty())
			{
				pso_desc.CS.pShaderBytecode = blob->data();
				pso_desc.CS.BytecodeLength = static_cast<UINT>(blob->size());
			}
			else
			{
				pso_desc.CS.pShaderBytecode = nullptr;
				pso_desc.CS.BytecodeLength = 0;
			}
		}
		pso_desc.NodeMask = 0;
		pso_desc.CachedPSO.pCachedBlob = nullptr;
		pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
		pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		ID3D12PipelineStatePtr pso = this->CreateComputePSO(pso_desc);
		pso_cache_.push_back(pso);

		d3d_compute_cmd_list_->SetPipelineState(pso.get());
		d3d_compute_cmd_list_->SetComputeRootSignature(so->RootSignature().get());

		ShaderObject::ShaderType const st = ShaderObject::ST_ComputeShader;
		size_t const num_handle = so->SRVs(st).size() + so->UAVs(st).size();

		std::array<ID3D12DescriptorHeap*, 2> heaps;
		uint32_t num_heaps = 0;
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap;
		ID3D12DescriptorHeapPtr sampler_heap = so->SamplerHeap();
		if (num_handle > 0)
		{
			size_t hash_val = 0;
			boost::hash_combine(hash_val, st);
			boost::hash_combine(hash_val, so->SRVs(st).size());
			if (!so->SRVs(st).empty())
			{
				boost::hash_range(hash_val, so->SRVs(st).begin(), so->SRVs(st).end());
			}
			boost::hash_combine(hash_val, st);
			boost::hash_combine(hash_val, so->UAVs(st).size());
			if (!so->UAVs(st).empty())
			{
				boost::hash_range(hash_val, so->UAVs(st).begin(), so->UAVs(st).end());
			}

			auto iter = cbv_srv_uav_heaps_.find(hash_val);
			if (iter == cbv_srv_uav_heaps_.end())
			{
				cbv_srv_uav_heap = this->CreateDynamicCBVSRVUAVDescriptorHeap(static_cast<uint32_t>(num_handle));
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
			heaps[num_heaps] = sampler_heap.get();
			++ num_heaps;
		}

		if (num_heaps > 0)
		{
			d3d_compute_cmd_list_->SetDescriptorHeaps(num_heaps, &heaps[0]);
		}

		uint32_t root_param_index = 0;
		if (!so->CBuffers(st).empty())
		{
			for (uint32_t j = 0; j < so->CBuffers(st).size(); ++ j)
			{
				ID3D12ResourcePtr const & buff = checked_cast<D3D12GraphicsBuffer*>(so->CBuffers(st)[j])->D3DBuffer();
				if (buff)
				{
					d3d_compute_cmd_list_->SetComputeRootConstantBufferView(root_param_index, buff->GetGPUVirtualAddress());

					++ root_param_index;
				}
				else
				{
					d3d_compute_cmd_list_->SetComputeRootConstantBufferView(root_param_index, 0);
				}
			}
		}
		if (cbv_srv_uav_heap)
		{
			UINT const cbv_srv_uav_desc_size = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			D3D12_CPU_DESCRIPTOR_HANDLE cpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE gpu_cbv_srv_uav_handle = cbv_srv_uav_heap->GetGPUDescriptorHandleForHeapStart();

			if (!so->SRVs(st).empty())
			{
				d3d_compute_cmd_list_->SetComputeRootDescriptorTable(root_param_index, gpu_cbv_srv_uav_handle);

				for (uint32_t j = 0; j < so->SRVs(st).size(); ++ j)
				{
					d3d_device_->CopyDescriptorsSimple(1, cpu_cbv_srv_uav_handle,
						std::get<0>(so->SRVSrcs(st)[j]) ? so->SRVs(st)[j]->Handle() : null_srv_handle_,
						D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					cpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size;
					gpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size;
				}

				++ root_param_index;
			}
			if (!so->UAVs(st).empty())
			{
				d3d_compute_cmd_list_->SetComputeRootDescriptorTable(root_param_index, gpu_cbv_srv_uav_handle);

				for (uint32_t j = 0; j < so->UAVs(st).size(); ++ j)
				{
					d3d_device_->CopyDescriptorsSimple(1, cpu_cbv_srv_uav_handle, 
						so->UAVSrcs(st)[j].first ? so->UAVs(st)[j]->Handle() : null_uav_handle_,
						D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					cpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size;
					gpu_cbv_srv_uav_handle.ptr += cbv_srv_uav_desc_size;
				}

				++ root_param_index;
			}
		}

		if (sampler_heap)
		{
			UINT const sampler_desc_size = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

			D3D12_GPU_DESCRIPTOR_HANDLE gpu_sampler_handle = sampler_heap->GetGPUDescriptorHandleForHeapStart();

			if (!so->Samplers(st).empty())
			{
				d3d_compute_cmd_list_->SetComputeRootDescriptorTable(root_param_index, gpu_sampler_handle);

				gpu_sampler_handle.ptr += sampler_desc_size * so->Samplers(st).size();

				++ root_param_index;
			}
		}
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D12RenderEngine::DoRender(RenderTechnique const & tech, RenderLayout const & rl)
	{
		if (last_engine_type_ != ET_Render)
		{
			this->ForceCPUGPUSync();
		}

		D3D12FrameBuffer& fb = *checked_cast<D3D12FrameBuffer*>(this->CurFrameBuffer().get());
		fb.SetRenderTargets();
		fb.BindBarrier();

		d3d_render_cmd_list_->OMSetStencilRef(stencil_ref_cache_);
		d3d_render_cmd_list_->OMSetBlendFactor(&blend_factor_cache_.r());

		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		for (uint32_t i = 0; i < so_buffs_.size(); ++ i)
		{
			D3D12GraphicsBuffer& d3dvb = *checked_cast<D3D12GraphicsBuffer*>(so_buffs_[i].get());

			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.Subresource = 0;
			if (d3dvb.UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_STREAM_OUT))
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
				barrier.Transition.Subresource = 0;
				if (d3dvb.UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
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
				barrier.Transition.Subresource = 0;
				if (d3dvb.UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
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
				barrier.Transition.Subresource = 0;
				if (ib.UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_INDEX_BUFFER))
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
		if (topology_type_cache_ != tt)
		{
			topology_type_cache_ = tt;
		}
		d3d_render_cmd_list_->IASetPrimitiveTopology(D3D12Mapping::Mapping(tt));

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
				BOOST_ASSERT(false);
				prim_count = 0;
			}
			break;
		}

		uint32_t const num_instances = rl.NumInstances();

		num_primitives_just_rendered_ += num_instances * prim_count;
		num_vertices_just_rendered_ += num_instances * vertex_count;

		uint32_t const num_passes = tech.NumPasses();
		GraphicsBufferPtr const & indirect_buff = rl.GetIndirectArgs();
		if (indirect_buff)
		{
			// TODO: ExecuteIndirect's first 2 parameters can't be right
			if (rl.UseIndices())
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					this->UpdateRenderPSO(tech, pass, rl);
					d3d_render_cmd_list_->ExecuteIndirect(nullptr, 0,
						checked_cast<D3D12GraphicsBuffer const *>(indirect_buff.get())->D3DBuffer().get(),
						rl.IndirectArgsOffset(), nullptr, 0);
					pass->Unbind();
				}
			}
			else
			{
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					this->UpdateRenderPSO(tech, pass, rl);
					d3d_render_cmd_list_->ExecuteIndirect(nullptr, 0,
						checked_cast<D3D12GraphicsBuffer const *>(indirect_buff.get())->D3DBuffer().get(),
						rl.IndirectArgsOffset(), nullptr, 0);
					pass->Unbind();
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
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					this->UpdateRenderPSO(tech, pass, rl);
					d3d_render_cmd_list_->DrawIndexedInstanced(num_indices, num_instances, rl.StartIndexLocation(),
						rl.StartVertexLocation(), rl.StartInstanceLocation());
					pass->Unbind();
				}
			}
			else
			{
				uint32_t const num_vertices = rl.NumVertices();
				for (uint32_t i = 0; i < num_passes; ++ i)
				{
					RenderPassPtr const & pass = tech.Pass(i);

					pass->Bind();
					this->UpdateRenderPSO(tech, pass, rl);
					d3d_render_cmd_list_->DrawInstanced(num_vertices, num_instances,
						rl.StartVertexLocation(), rl.StartInstanceLocation());
					pass->Unbind();
				}
			}
		}

		num_draws_just_called_ += num_passes;

		fb.UnbindBarrier();

		last_engine_type_ = ET_Render;
	}

	void D3D12RenderEngine::DoDispatch(RenderTechnique const & tech, uint32_t tgx, uint32_t tgy, uint32_t tgz)
	{
		if (last_engine_type_ != ET_Compute)
		{
			this->ForceCPUGPUSync();
		}

		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++ i)
		{
			RenderPassPtr const & pass = tech.Pass(i);

			pass->Bind();
			this->UpdateComputePSO(pass);
			d3d_compute_cmd_list_->Dispatch(tgx, tgy, tgz);
			pass->Unbind();
		}

		num_dispatches_just_called_ += num_passes;
		last_engine_type_ = ET_Compute;
	}

	void D3D12RenderEngine::DoDispatchIndirect(RenderTechnique const & tech, GraphicsBufferPtr const & buff_args,
			uint32_t offset)
	{
		if (last_engine_type_ != ET_Compute)
		{
			this->ForceCPUGPUSync();
		}

		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++ i)
		{
			RenderPassPtr const & pass = tech.Pass(i);

			pass->Bind();
			this->UpdateComputePSO(pass);
			// TODO: ExecuteIndirect's first 2 parameters can't be right
			d3d_compute_cmd_list_->ExecuteIndirect(nullptr, 0, checked_cast<D3D12GraphicsBuffer*>(buff_args.get())->D3DBuffer().get(),
				offset, nullptr, 0);
			pass->Unbind();
		}

		num_dispatches_just_called_ += num_passes;
		last_engine_type_ = ET_Compute;
	}

	void D3D12RenderEngine::ForceFlush()
	{
		TIF(d3d_render_cmd_list_->Close());
		TIF(d3d_compute_cmd_list_->Close());
		TIF(d3d_copy_cmd_list_->Close());

		ID3D12CommandList* cmd_lists[1];

		cmd_lists[0] = d3d_render_cmd_list_.get();
		d3d_render_cmd_queue_->ExecuteCommandLists(sizeof(cmd_lists) / sizeof(cmd_lists[0]), cmd_lists);

		cmd_lists[0] = d3d_compute_cmd_list_.get();
		d3d_compute_cmd_queue_->ExecuteCommandLists(sizeof(cmd_lists) / sizeof(cmd_lists[0]), cmd_lists);

		cmd_lists[0] = d3d_copy_cmd_list_.get();
		d3d_copy_cmd_queue_->ExecuteCommandLists(sizeof(cmd_lists) / sizeof(cmd_lists[0]), cmd_lists);
	}

	void D3D12RenderEngine::ForceCPUGPUSync()
	{
		this->CommitRenderCmd();
		this->CommitComputeCmd();
		this->CommitCopyCmd();
		this->SyncRenderCmd();
		this->SyncComputeCmd();
		this->SyncCopyCmd();
		this->ResetRenderCmd();
		this->ResetComputeCmd();
		this->ResetCopyCmd();

		this->ClearPSOCache();
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

		res_cmd_fence_.reset();
		render_cmd_fence_.reset();
		compute_cmd_fence_.reset();

		copy_cmd_fence_.reset();

		this->ClearPSOCache();

		compute_psos_.clear();
		graphics_psos_.clear();
		root_signatures_.clear();

		bilinear_blit_tech_.reset();
		blit_effect_.reset();

		cbv_srv_uav_desc_heap_.reset();
		dsv_desc_heap_.reset();
		rtv_desc_heap_.reset();

		d3d_res_cmd_list_.reset();
		d3d_res_cmd_allocator_.reset();
		d3d_render_cmd_list_.reset();
		d3d_render_cmd_allocator_.reset();
		d3d_render_cmd_queue_.reset();
		d3d_compute_cmd_list_.reset();
		d3d_compute_cmd_allocator_.reset();
		d3d_compute_cmd_queue_.reset();
		d3d_copy_cmd_list_.reset();
		d3d_copy_cmd_allocator_.reset();
		d3d_copy_cmd_queue_.reset();
		d3d_device_.reset();

		gi_factory_.reset();
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
		return vertex_format_.find(elem_fmt) != vertex_format_.end();
	}

	bool D3D12RenderEngine::TextureFormatSupport(ElementFormat elem_fmt)
	{
		return texture_format_.find(elem_fmt) != texture_format_.end();
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
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		switch (d3d_feature_level_)
		{
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			caps_.max_vertex_streams = D3D12_STANDARD_VERTEX_ELEMENT_COUNT;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		switch (d3d_feature_level_)
		{
		case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
			caps_.max_texture_anisotropy = D3D12_MAX_MAXANISOTROPY;
			break;

		default:
			BOOST_ASSERT(false);
			break;
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
		caps_.mrt_independent_bit_depths_support = true;
		caps_.standard_derivatives_support = true;
		caps_.shader_texture_lod_support = true;
		caps_.logic_op_support = true;
		caps_.independent_blend_support = true;
		caps_.draw_indirect_support = true;
		caps_.no_overwrite_support = true;
		caps_.full_npot_texture_support = true;
		caps_.render_to_texture_array_support = true;
		caps_.gs_support = true;
		caps_.hs_support = true;
		caps_.ds_support = true;

		std::pair<ElementFormat, DXGI_FORMAT> fmts[] = 
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
		for (size_t i = 0; i < sizeof(fmts) / sizeof(fmts[0]); ++ i)
		{
			DXGI_FORMAT dxgi_fmt;
			if (IsDepthFormat(fmts[i].first))
			{
				switch (fmts[i].first)
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
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER;
				fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					vertex_format_.insert(fmts[i].first);
				}

				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE1D;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE2D;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE3D;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURECUBE;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_SHADER_LOAD;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
			}
			else
			{
				dxgi_fmt = fmts[i].second;

				fmt_support.Format = dxgi_fmt;
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER;
				fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					vertex_format_.insert(fmts[i].first);
				}

				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE1D;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE2D;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE3D;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_TEXTURECUBE;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
				fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE;
				if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
				{
					texture_format_.insert(fmts[i].first);
				}
			}

			bool rt_supported = false;
			fmt_support.Format = dxgi_fmt;
			fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
			fmt_support.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
			if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
			{
				rt_supported = true;
			}
			fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET;
			if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
			{
				rt_supported = true;
			}
			fmt_support.Support1 = D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL;
			if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support, sizeof(fmt_support))))
			{
				rt_supported = true;
			}

			if (rt_supported)
			{
				D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaa_quality_levels;
				msaa_quality_levels.Format = dxgi_fmt;

				UINT count = 1;
				while (count <= D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT)
				{
					msaa_quality_levels.SampleCount = count;
					if (SUCCEEDED(d3d_device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaa_quality_levels, sizeof(msaa_quality_levels))))
					{
						if (msaa_quality_levels.NumQualityLevels > 0)
						{
							rendertarget_format_[fmts[i].first].emplace_back(count, msaa_quality_levels.NumQualityLevels);
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

		caps_.vertex_format_support = std::bind<bool>(&D3D12RenderEngine::VertexFormatSupport, this,
			std::placeholders::_1);
		caps_.texture_format_support = std::bind<bool>(&D3D12RenderEngine::TextureFormatSupport, this,
			std::placeholders::_1);
		caps_.rendertarget_format_support = std::bind<bool>(&D3D12RenderEngine::RenderTargetFormatSupport, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

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
		if (stencil_ref_cache_ != stencil_ref)
		{
			stencil_ref_cache_ = stencil_ref;
			d3d_render_cmd_list_->OMSetStencilRef(stencil_ref_cache_);
		}
	}

	void D3D12RenderEngine::OMSetBlendState(Color const & blend_factor, uint32_t sample_mask)
	{
		if (blend_factor_cache_ != blend_factor)
		{
			blend_factor_cache_ = blend_factor;
			d3d_render_cmd_list_->OMSetBlendFactor(&blend_factor_cache_.r());
		}
		sample_mask_cache_ = sample_mask;
	}

	void D3D12RenderEngine::RSSetViewports(UINT NumViewports, D3D12_VIEWPORT const * pViewports)
	{
		viewport_cache_ = pViewports[0];
		d3d_render_cmd_list_->RSSetViewports(NumViewports, reinterpret_cast<D3D12_VIEWPORT const *>(pViewports));
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

		BOOST_ASSERT(false);
		return 0;
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

		BOOST_ASSERT(false);
		return 0;
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

		BOOST_ASSERT(false);
		return 0;
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
			std::array<size_t, ShaderObject::ST_NumShaderTypes * 4> const & num,
			bool has_vs, bool has_stream_output)
	{
		ID3D12RootSignaturePtr ret;

		size_t hash_val = 0;
		boost::hash_range(hash_val, num.begin(), num.end());
		boost::hash_combine(hash_val, has_vs);
		boost::hash_combine(hash_val, has_stream_output);
		auto iter = root_signatures_.find(hash_val);
		if (iter == root_signatures_.end())
		{
			size_t num_cbv = 0;
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
					range.NumDescriptors = static_cast<UINT>(num[i * 4 + 1]);
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
					range.NumDescriptors = static_cast<UINT>(num[i * 4 + 2]);
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
					range.NumDescriptors = static_cast<UINT>(num[i * 4 + 3]);
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

			ID3DBlob* signature;
			ID3DBlob* error;
			TIF(D3D12InterfaceLoader::Instance().D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
			ID3D12RootSignature* rs;
			TIF(d3d_device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
				IID_ID3D12RootSignature, reinterpret_cast<void**>(&rs)));
			signature->Release();
			if (error)
			{
				error->Release();
			}
			
			return root_signatures_.emplace(hash_val, MakeCOMPtr(rs)).first->second;
		}
		else
		{
			return iter->second;
		}
	}

	ID3D12PipelineStatePtr const & D3D12RenderEngine::CreateRenderPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC const & desc)
	{
		char const * p = reinterpret_cast<char const *>(&desc);
		size_t hash_val = 0;
		boost::hash_range(hash_val, p, p + sizeof(desc));

		auto iter = graphics_psos_.find(hash_val);
		if (iter == graphics_psos_.end())
		{
			ID3D12PipelineState* d3d_pso;
			TIF(d3d_device_->CreateGraphicsPipelineState(&desc, IID_ID3D12PipelineState, reinterpret_cast<void**>(&d3d_pso)));
			return graphics_psos_.emplace(hash_val, MakeCOMPtr(d3d_pso)).first->second;
		}
		else
		{
			return iter->second;
		}
	}

	ID3D12PipelineStatePtr const & D3D12RenderEngine::CreateComputePSO(D3D12_COMPUTE_PIPELINE_STATE_DESC const & desc)
	{
		char const * p = reinterpret_cast<char const *>(&desc);
		size_t hash_val = 0;
		boost::hash_range(hash_val, p, p + sizeof(desc));

		auto iter = compute_psos_.find(hash_val);
		if (iter == compute_psos_.end())
		{
			ID3D12PipelineState* d3d_pso;
			TIF(d3d_device_->CreateComputePipelineState(&desc, IID_ID3D12PipelineState, reinterpret_cast<void**>(&d3d_pso)));
			return compute_psos_.emplace(hash_val, MakeCOMPtr(d3d_pso)).first->second;
		}
		else
		{
			return iter->second;
		}
	}

	ID3D12DescriptorHeapPtr D3D12RenderEngine::CreateDynamicCBVSRVUAVDescriptorHeap(uint32_t num)
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
		cbv_srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_srv_heap_desc.NumDescriptors = num;
		cbv_srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbv_srv_heap_desc.NodeMask = 0;
		ID3D12DescriptorHeap* csu_heap;
		TIF(d3d_device_->CreateDescriptorHeap(&cbv_srv_heap_desc, IID_ID3D12DescriptorHeap, reinterpret_cast<void**>(&csu_heap)));
		ID3D12DescriptorHeapPtr cbv_srv_uav_heap = MakeCOMPtr(csu_heap);
		cbv_srv_uav_heap_cache_.push_back(cbv_srv_uav_heap);
		return cbv_srv_uav_heap;
	}
}
