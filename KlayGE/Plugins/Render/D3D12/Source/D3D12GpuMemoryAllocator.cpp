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

#include "D3D12GpuMemoryAllocator.hpp"
#include "D3D12RenderEngine.hpp"

namespace
{
	uint32_t constexpr PageSize = 2 * 1024 * 1024;
	uint32_t constexpr SegmentSize = 512;
	uint32_t constexpr SegmentMask = SegmentSize - 1;
}

namespace KlayGE
{
	D3D12GpuMemoryPage::D3D12GpuMemoryPage(bool is_upload, uint32_t size_in_bytes) : is_upload_(is_upload)
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

		TIFHR(device->CreateCommittedResource(
			&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, init_state, nullptr, UuidOf<ID3D12Resource>(), resource_.put_void()));

		D3D12_RANGE const read_range{0, 0};
		TIFHR(resource_->Map(0, &read_range, &cpu_addr_));

		gpu_addr_ = resource_->GetGPUVirtualAddress();
	}

	D3D12GpuMemoryPage::~D3D12GpuMemoryPage() noexcept
	{
		if (resource_)
		{
			D3D12_RANGE const write_range{0, 0};
			resource_->Unmap(0, is_upload_ ? nullptr : &write_range);
		}
	}

	D3D12GpuMemoryPage::D3D12GpuMemoryPage(D3D12GpuMemoryPage&& other) noexcept
		: is_upload_(other.is_upload_), resource_(std::move(other.resource_)), cpu_addr_(std::move(other.cpu_addr_)),
		  gpu_addr_(std::move(other.gpu_addr_))
	{
	}

	D3D12GpuMemoryPage& D3D12GpuMemoryPage::operator=(D3D12GpuMemoryPage&& other) noexcept
	{
		if (this != &other)
		{
			BOOST_ASSERT(is_upload_ == other.is_upload_);

			resource_ = std::move(other.resource_);
			cpu_addr_ = std::move(other.cpu_addr_);
			gpu_addr_ = std::move(other.gpu_addr_);
		}
		return *this;
	}


	D3D12GpuMemoryBlock::D3D12GpuMemoryBlock() noexcept = default;
	D3D12GpuMemoryBlock::D3D12GpuMemoryBlock(D3D12GpuMemoryBlock&& other) noexcept = default;
	D3D12GpuMemoryBlock& D3D12GpuMemoryBlock::operator=(D3D12GpuMemoryBlock&& other) noexcept = default;

	void D3D12GpuMemoryBlock::Reset() noexcept
	{
		resource_ = nullptr;
		offset_ = 0;
		size_ = 0;
		cpu_addr_ = nullptr;
		gpu_addr_ = {};
	}

	void D3D12GpuMemoryBlock::Reset(D3D12GpuMemoryPage const& page, uint32_t offset, uint32_t size) noexcept
	{
		resource_ = page.Resource();
		offset_ = offset;
		size_ = size;
		cpu_addr_ = page.CpuAddress<uint8_t>() + offset;
		gpu_addr_ = page.GpuAddress() + offset;
	}


	D3D12GpuMemoryAllocator::D3D12GpuMemoryAllocator(bool is_upload) noexcept : is_upload_(is_upload)
	{
	}

	D3D12GpuMemoryAllocator::D3D12GpuMemoryAllocator(D3D12GpuMemoryAllocator&& other) noexcept
		: is_upload_(other.is_upload_), pages_(std::move(other.pages_)), large_pages_(std::move(other.large_pages_))
	{
	}

	D3D12GpuMemoryAllocator& D3D12GpuMemoryAllocator::operator=(D3D12GpuMemoryAllocator&& other) noexcept
	{
		if (this != &other)
		{
			BOOST_ASSERT(is_upload_ == other.is_upload_);

			pages_ = std::move(other.pages_);
			large_pages_ = std::move(other.large_pages_);
		}
		return *this;
	}

	D3D12GpuMemoryBlock D3D12GpuMemoryAllocator::Allocate(uint32_t size_in_bytes, uint32_t alignment)
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		D3D12GpuMemoryBlock mem_block;
		this->Allocate(lock, mem_block, size_in_bytes, alignment);
		return mem_block;
	}

	void D3D12GpuMemoryAllocator::Allocate([[maybe_unused]] std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuMemoryBlock& mem_block,
		uint32_t size_in_bytes, uint32_t alignment)
	{
		BOOST_ASSERT(alignment <= SegmentSize);
		uint32_t const aligned_size = ((size_in_bytes + alignment - 1) / alignment * alignment + SegmentMask) & ~SegmentMask;

		if (aligned_size > PageSize)
		{
			auto& large_page = large_pages_.emplace_back(D3D12GpuMemoryPage(is_upload_, aligned_size));
			mem_block.Reset(large_page, 0, size_in_bytes);
			return;
		}

		for (auto& page_info : pages_)
		{
			auto const iter = std::lower_bound(page_info.free_list.begin(), page_info.free_list.end(), aligned_size,
				[](PageInfo::FreeRange const& free_range, uint32_t s) { return free_range.first_offset + s > free_range.last_offset; });
			if (iter != page_info.free_list.end())
			{
				uint32_t const aligned_offset = (iter->first_offset + alignment - 1) / alignment * alignment;
				mem_block.Reset(page_info.page, aligned_offset, size_in_bytes);
				iter->first_offset += aligned_size;
				if (iter->first_offset == iter->last_offset)
				{
					page_info.free_list.erase(iter);
				}

				return;
			}
		}

		D3D12GpuMemoryPage new_page(is_upload_, PageSize);
		mem_block.Reset(new_page, 0, size_in_bytes);
		pages_.emplace_back(PageInfo{std::move(new_page), {{aligned_size, PageSize}}, {}});
	}

	void D3D12GpuMemoryAllocator::Deallocate(D3D12GpuMemoryBlock&& mem_block, uint64_t fence_value)
	{
		if (mem_block)
		{
			std::lock_guard<std::mutex> lock(allocation_mutex_);
			this->Deallocate(lock, mem_block, fence_value);
		}
	}

	void D3D12GpuMemoryAllocator::Deallocate(
		[[maybe_unused]] std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuMemoryBlock& mem_block, uint64_t fence_value)
	{
		BOOST_ASSERT(mem_block);

		if (mem_block.Size() <= PageSize)
		{
			for (auto& page : pages_)
			{
				if (page.page.Resource() == mem_block.Resource())
				{
					uint32_t const offset = mem_block.Offset() & ~SegmentMask;
					uint32_t const size = (mem_block.Offset() + mem_block.Size() - offset + SegmentMask) & ~SegmentMask;
					page.stall_list.push_back({{offset, offset + size}, fence_value});
					return;
				}
			}

			KFL_UNREACHABLE("This memory block is not allocated by this allocator");
		}
	}

	void D3D12GpuMemoryAllocator::Renew(D3D12GpuMemoryBlock& mem_block, uint64_t fence_value, uint32_t size_in_bytes, uint32_t alignment)
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		if (mem_block)
		{
			this->Deallocate(lock, mem_block, fence_value);
		}
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
					auto const free_iter = std::lower_bound(page.free_list.begin(), page.free_list.end(),
						stall_iter->free_range.first_offset, [](PageInfo::FreeRange const& free_range, uint32_t first_offset) {
							return free_range.first_offset < first_offset;
						});
					if (free_iter == page.free_list.end())
					{
						if (page.free_list.empty() || (page.free_list.back().last_offset != stall_iter->free_range.first_offset))
						{
							page.free_list.emplace_back(std::move(stall_iter->free_range));
						}
						else
						{
							page.free_list.back().last_offset = stall_iter->free_range.last_offset;
						}
					}
					else if (free_iter->first_offset != stall_iter->free_range.last_offset)
					{
						bool merge_with_prev = false;
						if (free_iter != page.free_list.begin())
						{
							auto const prev_free_iter = std::prev(free_iter);
							if (prev_free_iter->last_offset == stall_iter->free_range.first_offset)
							{
								prev_free_iter->last_offset = stall_iter->free_range.last_offset;
								merge_with_prev = true;
							}
						}

						if (!merge_with_prev)
						{
							page.free_list.emplace(free_iter, std::move(stall_iter->free_range));
						}
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
} // namespace KlayGE
