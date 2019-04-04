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
#include <KFL/ErrorHandling.hpp>
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
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		D3D12_QUERY_HEAP_DESC query_heap_desc;
		query_heap_desc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
		query_heap_desc.Count = 1;
		query_heap_desc.NodeMask = 0;

		ID3D12QueryHeap* query_heap;
		TIFHR(device->CreateQueryHeap(&query_heap_desc, IID_ID3D12QueryHeap,
			reinterpret_cast<void**>(&query_heap)));
		query_heap_ = MakeCOMPtr(query_heap);

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
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
		TIFHR(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&query_result)));
		query_result_ = MakeCOMPtr(query_result);
	}

	void D3D12OcclusionQuery::Begin()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->BeginQuery(query_heap_.get(), D3D12_QUERY_TYPE_OCCLUSION, 0);
	}

	void D3D12OcclusionQuery::End()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(query_heap_.get(), D3D12_QUERY_TYPE_OCCLUSION, 0);
	}

	uint64_t D3D12OcclusionQuery::SamplesPassed()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->ResolveQueryData(query_heap_.get(), D3D12_QUERY_TYPE_OCCLUSION, 0, 1, query_result_.get(), 0);

		re.ForceFinish();

		D3D12_RANGE range;
		range.Begin = 0;

		uint64_t* result;
		range.End = sizeof(uint64_t);
		query_result_->Map(0, &range, reinterpret_cast<void**>(&result));
		uint64_t ret = *result;
		range.End = 0;
		query_result_->Unmap(0, &range);
		return ret;
	}


	D3D12ConditionalRender::D3D12ConditionalRender()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		D3D12_QUERY_HEAP_DESC query_heap_desc;
		query_heap_desc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
		query_heap_desc.Count = 1;
		query_heap_desc.NodeMask = 0;

		ID3D12QueryHeap* predicate_heap;
		TIFHR(device->CreateQueryHeap(&query_heap_desc, IID_ID3D12QueryHeap,
			reinterpret_cast<void**>(&predicate_heap)));
		predicate_heap_ = MakeCOMPtr(predicate_heap);

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
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
		TIFHR(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&predicate_result)));
		predicate_result_ = MakeCOMPtr(predicate_result);
	}

	void D3D12ConditionalRender::Begin()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->BeginQuery(predicate_heap_.get(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, 0);
	}

	void D3D12ConditionalRender::End()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(predicate_heap_.get(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, 0);
	}

	void D3D12ConditionalRender::BeginConditionalRender()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->SetPredication(predicate_result_.get(), 0, D3D12_PREDICATION_OP_NOT_EQUAL_ZERO);
	}

	void D3D12ConditionalRender::EndConditionalRender()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_NOT_EQUAL_ZERO);
	}

	bool D3D12ConditionalRender::AnySamplesPassed()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->ResolveQueryData(predicate_heap_.get(), D3D12_QUERY_TYPE_OCCLUSION, 0, 1, predicate_result_.get(), 0);

		re.ForceFinish();

		D3D12_RANGE range;
		range.Begin = 0;

		uint64_t* result;
		range.End = sizeof(uint64_t);
		predicate_result_->Map(0, &range, reinterpret_cast<void**>(&result));
		bool ret = *result ? true : false;
		range.End = 0;
		predicate_result_->Unmap(0, &range);
		return ret;
	}


	D3D12TimerQuery::D3D12TimerQuery()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		D3D12_QUERY_HEAP_DESC timestamp_query_heap_desc;
		timestamp_query_heap_desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		timestamp_query_heap_desc.Count = 2;
		timestamp_query_heap_desc.NodeMask = 0;
		ID3D12QueryHeap* timestamp_heap;
		TIFHR(device->CreateQueryHeap(&timestamp_query_heap_desc, IID_ID3D12QueryHeap,
			reinterpret_cast<void**>(&timestamp_heap)));
		timestamp_heap_ = MakeCOMPtr(timestamp_heap);

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
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
		TIFHR(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&timestamp_result)));
		timestamp_result_ = MakeCOMPtr(timestamp_result);
	}

	void D3D12TimerQuery::Begin()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(timestamp_heap_.get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
	}

	void D3D12TimerQuery::End()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(timestamp_heap_.get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);
	}

	double D3D12TimerQuery::TimeElapsed()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->ResolveQueryData(timestamp_heap_.get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, timestamp_result_.get(), 0);

		re.ForceFinish();

		UINT64 freq;
		re.D3DRenderCmdQueue()->GetTimestampFrequency(&freq);

		D3D12_RANGE range;
		range.Begin = 0;

		uint64_t* timestamp;
		range.End = sizeof(uint64_t) * 2;
		timestamp_result_->Map(0, &range, reinterpret_cast<void**>(&timestamp));
		double ret = static_cast<double>(timestamp[1] - timestamp[0]) / freq;
		range.End = 0;
		timestamp_result_->Unmap(0, &range);

		return ret;
	}


	D3D12SOStatisticsQuery::D3D12SOStatisticsQuery()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12Device* device = re.D3DDevice();

		D3D12_QUERY_HEAP_DESC query_heap_desc;
		query_heap_desc.Type = D3D12_QUERY_HEAP_TYPE_SO_STATISTICS;
		query_heap_desc.Count = 1;
		query_heap_desc.NodeMask = 0;

		ID3D12QueryHeap* query_heap;
		TIFHR(device->CreateQueryHeap(&query_heap_desc, IID_ID3D12QueryHeap,
			reinterpret_cast<void**>(&query_heap)));
		so_stat_query_heap_ = MakeCOMPtr(query_heap);

		D3D12_HEAP_PROPERTIES heap_prop;
		heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC res_desc;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = sizeof(D3D12_QUERY_DATA_SO_STATISTICS);
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* query_result;
		TIFHR(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_ID3D12Resource, reinterpret_cast<void**>(&query_result)));
		so_stat_query_result_ = MakeCOMPtr(query_result);
	}

	void D3D12SOStatisticsQuery::Begin()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->BeginQuery(so_stat_query_heap_.get(), D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0);
	}

	void D3D12SOStatisticsQuery::End()
	{
		auto const& re = checked_cast<D3D12RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->EndQuery(so_stat_query_heap_.get(), D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0);
	}

	uint64_t D3D12SOStatisticsQuery::NumPrimitivesWritten()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->ResolveQueryData(so_stat_query_heap_.get(), D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0, 1, so_stat_query_result_.get(), 0);

		re.ForceFinish();

		D3D12_RANGE range;
		range.Begin = 0;

		D3D12_QUERY_DATA_SO_STATISTICS* result;
		range.End = sizeof(D3D12_QUERY_DATA_SO_STATISTICS);
		so_stat_query_result_->Map(0, &range, reinterpret_cast<void**>(&result));
		uint64_t ret = result->NumPrimitivesWritten;
		range.End = 0;
		so_stat_query_result_->Unmap(0, &range);
		return ret;
	}

	uint64_t D3D12SOStatisticsQuery::PrimitivesGenerated()
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D12GraphicsCommandList* cmd_list = re.D3DRenderCmdList();

		cmd_list->ResolveQueryData(so_stat_query_heap_.get(), D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0, 0, 1, so_stat_query_result_.get(), 0);

		re.ForceFinish();

		D3D12_RANGE range;
		range.Begin = 0;

		D3D12_QUERY_DATA_SO_STATISTICS* result;
		range.End = sizeof(D3D12_QUERY_DATA_SO_STATISTICS);
		so_stat_query_result_->Map(0, &range, reinterpret_cast<void**>(&result));
		uint64_t ret = result->PrimitivesStorageNeeded;
		range.End = 0;
		so_stat_query_result_->Unmap(0, &range);
		return ret;
	}
}
