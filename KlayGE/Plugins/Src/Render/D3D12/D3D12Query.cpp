/**
 * @file D3D12Query.cpp
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
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Query.hpp>

namespace KlayGE
{
	D3D12OcclusionQuery::D3D12OcclusionQuery()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&rf.RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		D3D12_QUERY_HEAP_DESC query_heap_desc;
		query_heap_desc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
		query_heap_desc.Count = 1;
		query_heap_desc.NodeMask = 0;

		ID3D12QueryHeap* query_heap;
		TIF(device->CreateQueryHeap(&query_heap_desc, IID_ID3D12QueryHeap,
			reinterpret_cast<void**>(&query_heap)));
		query_heap_ = MakeCOMPtr(query_heap);

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC res_desc;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = sizeof(uint64_t);
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* query_result;
		TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_PREDICATION, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&query_result)));
		query_result_ = MakeCOMPtr(query_result);

		heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
		ID3D12Resource* query_result_readback;
		TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&query_result_readback)));
		query_result_readback_ = MakeCOMPtr(query_result_readback);
	}

	void D3D12OcclusionQuery::Begin()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		cmd_list->BeginQuery(query_heap_.get(), D3D12_QUERY_TYPE_OCCLUSION, 0);
	}

	void D3D12OcclusionQuery::End()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(query_heap_.get(), D3D12_QUERY_TYPE_OCCLUSION, 0);

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = query_result_.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PREDICATION;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = 0;
		cmd_list->ResourceBarrier(1, &barrier);

		cmd_list->ResolveQueryData(query_heap_.get(), D3D12_QUERY_TYPE_OCCLUSION, 0, 1, query_result_.get(), 0);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		cmd_list->ResourceBarrier(1, &barrier);

		cmd_list->CopyResource(query_result_readback_.get(), query_result_.get());

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PREDICATION;
		cmd_list->ResourceBarrier(1, &barrier);
	}

	uint64_t D3D12OcclusionQuery::SamplesPassed()
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.ForceCPUGPUSync();

		uint64_t result;
		void* p;
		query_result_readback_->Map(0, nullptr, &p);
		memcpy(&result, p, sizeof(result));
		query_result_readback_->Unmap(0, nullptr);
		return result;
	}


	D3D12ConditionalRender::D3D12ConditionalRender()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&rf.RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		D3D12_QUERY_HEAP_DESC query_heap_desc;
		query_heap_desc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
		query_heap_desc.Count = 1;
		query_heap_desc.NodeMask = 0;

		ID3D12QueryHeap* predicate_heap;
		TIF(device->CreateQueryHeap(&query_heap_desc, IID_ID3D12QueryHeap,
			reinterpret_cast<void**>(&predicate_heap)));
		predicate_heap_ = MakeCOMPtr(predicate_heap);

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC res_desc;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = sizeof(uint64_t);
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* predicate_result;
		TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_PREDICATION, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&predicate_result)));
		predicate_result_ = MakeCOMPtr(predicate_result);

		heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
		ID3D12Resource* predicate_result_readback;
		TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&predicate_result_readback)));
		predicate_result_readback_ = MakeCOMPtr(predicate_result_readback);
	}

	void D3D12ConditionalRender::Begin()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		cmd_list->BeginQuery(predicate_heap_.get(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, 0);
	}

	void D3D12ConditionalRender::End()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(predicate_heap_.get(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, 0);

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = predicate_result_.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PREDICATION;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = 0;
		cmd_list->ResourceBarrier(1, &barrier);

		cmd_list->ResolveQueryData(predicate_heap_.get(), D3D12_QUERY_TYPE_OCCLUSION, 0, 1, predicate_result_.get(), 0);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		cmd_list->ResourceBarrier(1, &barrier);

		cmd_list->CopyResource(predicate_result_readback_.get(), predicate_result_.get());

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PREDICATION;
		cmd_list->ResourceBarrier(1, &barrier);
	}

	void D3D12ConditionalRender::BeginConditionalRender()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		cmd_list->SetPredication(predicate_result_.get(), 0, D3D12_PREDICATION_OP_NOT_EQUAL_ZERO);
	}

	void D3D12ConditionalRender::EndConditionalRender()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		cmd_list->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_NOT_EQUAL_ZERO);
	}

	bool D3D12ConditionalRender::AnySamplesPassed()
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.ForceCPUGPUSync();

		uint64_t result;
		void* p;
		predicate_result_readback_->Map(0, nullptr, &p);
		memcpy(&result, p, sizeof(result));
		predicate_result_readback_->Unmap(0, nullptr);
		return result ? true : false;
	}


	D3D12TimerQuery::D3D12TimerQuery()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12DevicePtr const & device = re.D3DDevice();

		D3D12_QUERY_HEAP_DESC timestamp_query_heap_desc;
		timestamp_query_heap_desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		timestamp_query_heap_desc.Count = 2;
		timestamp_query_heap_desc.NodeMask = 0;
		ID3D12QueryHeap* timestamp_heap;
		TIF(device->CreateQueryHeap(&timestamp_query_heap_desc, IID_ID3D12QueryHeap,
			reinterpret_cast<void**>(&timestamp_heap)));
		timestamp_heap_ = MakeCOMPtr(timestamp_heap);

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC res_desc;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = sizeof(uint64_t) * 2;
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* timestamp_result;
		TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_PREDICATION, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&timestamp_result)));
		timestamp_result_ = MakeCOMPtr(timestamp_result);

		heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
		ID3D12Resource* timestamp_result_readback;
		TIF(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&timestamp_result_readback)));
		timestamp_result_readback_ = MakeCOMPtr(timestamp_result_readback);
	}

	void D3D12TimerQuery::Begin()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(timestamp_heap_.get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
	}

	void D3D12TimerQuery::End()
	{
		D3D12RenderEngine const & re = *checked_cast<D3D12RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandListPtr const & cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(timestamp_heap_.get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = timestamp_result_.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PREDICATION;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = 0;
		cmd_list->ResourceBarrier(1, &barrier);

		cmd_list->ResolveQueryData(timestamp_heap_.get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, timestamp_result_.get(), 0);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		cmd_list->ResourceBarrier(1, &barrier);

		cmd_list->CopyResource(timestamp_result_readback_.get(), timestamp_result_.get());

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PREDICATION;
		cmd_list->ResourceBarrier(1, &barrier);
	}

	double D3D12TimerQuery::TimeElapsed()
	{
		D3D12RenderEngine& re = *checked_cast<D3D12RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.InvTimestampFreq() > 0)
		{
			re.ForceCPUGPUSync();

			uint64_t timestamp[2];
			void* p;
			timestamp_result_readback_->Map(0, nullptr, &p);
			memcpy(timestamp, p, sizeof(timestamp));
			timestamp_result_readback_->Unmap(0, nullptr);

			return (timestamp[1] - timestamp[0]) * re.InvTimestampFreq();
		}
		else
		{
			return -1;
		}
	}
}
