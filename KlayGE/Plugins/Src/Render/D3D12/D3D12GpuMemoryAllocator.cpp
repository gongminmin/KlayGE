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
#include <KlayGE/RenderFactory.hpp>

#include <boost/assert.hpp>

#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12GpuMemoryAllocator.hpp>

namespace KlayGE
{
	D3D12GpuMemoryPage::D3D12GpuMemoryPage(bool is_upload, ID3D12ResourcePtr resource)
		: is_upload_(is_upload), resource_(std::move(resource)), gpu_addr_(resource_->GetGPUVirtualAddress())
	{
		D3D12_RANGE const read_range{0, 0};
		resource_->Map(0, &read_range, &cpu_addr_);
	}

	D3D12GpuMemoryPage::~D3D12GpuMemoryPage()
	{
		D3D12_RANGE const write_range{0, 0};
		resource_->Unmap(0, is_upload_ ? nullptr : &write_range);
	}


	D3D12GpuMemoryBlock::D3D12GpuMemoryBlock() noexcept = default;

	void D3D12GpuMemoryBlock::Reset(D3D12GpuMemoryPage const& page, uint32_t offset, uint32_t size) noexcept
	{
		resource_ = page.Resource();
		offset_ = offset;
		size_ = size;
		cpu_addr_ = reinterpret_cast<uint8_t*>(page.CpuAddress()) + offset;
		gpu_addr_ = page.GpuAddress() + offset;
	}


	D3D12GpuMemoryAllocator::D3D12GpuMemoryAllocator(bool is_upload) noexcept : is_upload_(is_upload)
	{
	}

	std::unique_ptr<D3D12GpuMemoryBlock> D3D12GpuMemoryAllocator::Allocate(uint32_t size_in_bytes, uint32_t alignment)
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		std::unique_ptr<D3D12GpuMemoryBlock> mem_block;
		this->Allocate(lock, mem_block, size_in_bytes, alignment);
		return mem_block;
	}

	void D3D12GpuMemoryAllocator::Allocate(std::lock_guard<std::mutex>& proof_of_lock, std::unique_ptr<D3D12GpuMemoryBlock>& mem_block,
		uint32_t size_in_bytes, uint32_t alignment)
	{
		KFL_UNUSED(proof_of_lock);

		uint32_t const alignment_mask = alignment - 1;
		BOOST_ASSERT((alignment & alignment_mask) == 0);

		uint32_t const aligned_size = (size_in_bytes + alignment_mask) & ~alignment_mask;

		if (!mem_block)
		{
			mem_block = MakeUniquePtr<D3D12GpuMemoryBlock>();
		}

		if (aligned_size > DefaultPageSize)
		{
			auto large_page = this->CreatePage(aligned_size);
			large_pages_.push_back(large_page);
			mem_block->Reset(*large_page, 0, size_in_bytes);
			return;
		}

		for (auto& page_info : pages_)
		{
			for (auto iter = page_info.free_list.begin(); iter != page_info.free_list.end(); ++iter)
			{
				if (iter->first_offset + aligned_size <= iter->last_offset)
				{
					mem_block->Reset(*page_info.page, iter->first_offset, aligned_size);
					iter->first_offset += aligned_size;
					if (iter->first_offset == iter->last_offset)
					{
						page_info.free_list.erase(iter);
					}

					return;
				}
			}
		}

		PageInfo new_page_info;
		new_page_info.page = this->CreatePage(DefaultPageSize);
		new_page_info.free_list.push_back({aligned_size, DefaultPageSize});
		mem_block->Reset(*new_page_info.page, 0, aligned_size);
		pages_.push_back(std::move(new_page_info));
	}

	void D3D12GpuMemoryAllocator::Deallocate(std::unique_ptr<D3D12GpuMemoryBlock> mem_block, uint64_t fence_value)
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);
		this->Deallocate(lock, mem_block, fence_value);
	}

	void D3D12GpuMemoryAllocator::Deallocate(
		std::lock_guard<std::mutex>& proof_of_lock, std::unique_ptr<D3D12GpuMemoryBlock>& mem_block, uint64_t fence_value)
	{
		KFL_UNUSED(proof_of_lock);

		if (!mem_block)
		{
			return;
		}		

		if (mem_block->Size() <= DefaultPageSize)
		{
			for (auto& page : pages_)
			{
				if (page.page->Resource() == mem_block->Resource())
				{
					page.stall_list.push_back({{mem_block->Offset(), mem_block->Offset() + mem_block->Size()}, fence_value});
					return;
				}
			}

			KFL_UNREACHABLE("This memory block is not allocated by this allocator");
		}
	}

	void D3D12GpuMemoryAllocator::Renew(
		std::unique_ptr<D3D12GpuMemoryBlock>& mem_block, uint64_t fence_value, uint32_t size_in_bytes, uint32_t alignment)
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		this->Deallocate(lock, mem_block, fence_value);
		this->Allocate(lock, mem_block, size_in_bytes, alignment);
	}

	void D3D12GpuMemoryAllocator::ClearStallPages(uint64_t fence_value)
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		for (auto& page : pages_)
		{
			for (auto stall_iter = page.stall_list.begin(); stall_iter != page.stall_list.end();)
			{
				if (stall_iter->fence_value <= fence_value)
				{
					auto const free_iter = std::lower_bound(page.free_list.begin(), page.free_list.end(), stall_iter->free_range.first_offset,
						[](PageInfo::FreeRange& free_range, uint32_t first_offset) { return free_range.first_offset < first_offset; });
					if ((free_iter == page.free_list.end()) || (free_iter->first_offset != stall_iter->free_range.last_offset))
					{
						page.free_list.insert(free_iter, stall_iter->free_range);
					}
					else
					{
						free_iter->first_offset = stall_iter->free_range.first_offset;
						if (free_iter != page.free_list.begin())
						{
							auto const prev_free_iter = std::prev(free_iter);
							if (prev_free_iter->last_offset == free_iter->first_offset)
							{
								prev_free_iter->last_offset = free_iter->last_offset;
								page.free_list.erase(free_iter);
							}
						}
					}

					stall_iter = page.stall_list.erase(stall_iter);
				}
				else
				{
					++stall_iter;
				}
			}
		}

		large_pages_.clear();
	}

	void D3D12GpuMemoryAllocator::Clear()
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		pages_.clear();
		large_pages_.clear();
	}

	D3D12GpuMemoryPagePtr D3D12GpuMemoryAllocator::CreatePage(uint32_t size_in_bytes) const
	{
		auto& d3d12_re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* device = d3d12_re.D3DDevice();

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

		ID3D12ResourcePtr resource;
		TIFHR(device->CreateCommittedResource(
			&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, init_state, nullptr, IID_ID3D12Resource, resource.put_void()));

		return MakeSharedPtr<D3D12GpuMemoryPage>(is_upload_, std::move(resource));
	}
} // namespace KlayGE
