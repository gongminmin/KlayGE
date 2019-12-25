/**
 * @file D3D12GpuMemoryAllocator.cpp
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

#include <KlayGE/Context.hpp>
#include <KlayGE/Fence.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12GraphicsBuffer.hpp>
#include <KlayGE/D3D12/D3D12RenderEngine.hpp>

#include <KlayGE/D3D12/D3D12GpuMemoryAllocator.hpp>

namespace KlayGE
{
	D3D12GpuMemoryPage::D3D12GpuMemoryPage(bool is_upload, ID3D12ResourcePtr resource, uint32_t size_in_bytes)
		: is_upload_(is_upload), resource_(std::move(resource)), size_in_bytes_(size_in_bytes)
	{
	}

	D3D12GpuMemoryPage::~D3D12GpuMemoryPage()
	{
	}


	D3D12GpuMemoryAllocator::D3D12GpuMemoryAllocator(bool is_upload) : is_upload_(is_upload)
	{
	}

	D3D12GpuMemoryPagePtr D3D12GpuMemoryAllocator::Allocate(uint32_t size_in_bytes)
	{
		D3D12GpuMemoryPagePtr ret;

		auto iter = available_pages_.lower_bound(size_in_bytes);
		if ((iter != available_pages_.end()) && (iter->first == size_in_bytes))
		{
			ret = iter->second;
			available_pages_.erase(iter);
		}
		else
		{
			D3D12_RESOURCE_STATES init_state;
			D3D12_HEAP_PROPERTIES heap_prop;
			if (is_upload_)
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
			res_desc.Width = size_in_bytes;
			res_desc.Height = 1;
			res_desc.DepthOrArraySize = 1;
			res_desc.MipLevels = 1;
			res_desc.Format = DXGI_FORMAT_UNKNOWN;
			res_desc.SampleDesc.Count = 1;
			res_desc.SampleDesc.Quality = 0;
			res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

			auto& d3d12_re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto* device = d3d12_re.D3DDevice();

			ID3D12ResourcePtr resource;
			TIFHR(device->CreateCommittedResource(
				&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, init_state, nullptr, IID_ID3D12Resource, resource.put_void()));
			ret = MakeSharedPtr<D3D12GpuMemoryPage>(is_upload_, resource, size_in_bytes);
		}

		return ret;
	}

	void D3D12GpuMemoryAllocator::Deallocate(D3D12GpuMemoryPagePtr page)
	{
		if (page)
		{
			auto& d3d12_re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			frame_contexts_[d3d12_re.FrameIndex()].stall_pages.emplace_back(std::move(page));
		}
	}

	void D3D12GpuMemoryAllocator::ClearStallPages()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& d3d12_re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto& stall_pages = frame_contexts_[d3d12_re.FrameIndex()].stall_pages;
			for (auto const& page : stall_pages)
			{
				available_pages_.emplace(page->Size(), page);
			}
			stall_pages.clear();
		}
	}

	void D3D12GpuMemoryAllocator::Clear()
	{
		for (auto& frame_context : frame_contexts_)
		{
			frame_context.stall_pages.clear();
		}
		available_pages_.clear();
	}
} // namespace KlayGE
