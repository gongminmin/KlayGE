/**
 * @file D3D12GpuDescriptorAllocator.cpp
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

#include <KlayGE/D3D12/D3D12GpuDescriptorAllocator.hpp>
#include <KlayGE/D3D12/D3D12RenderEngine.hpp>

namespace
{
	uint32_t descriptor_size[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]{};

	uint16_t const default_descriptor_page_size[] = {32 * 1024, 1 * 1024, 8 * 1024, 4 * 1024};
} // namespace

namespace KlayGE
{
	D3D12GpuDescriptorPage::D3D12GpuDescriptorPage(ID3D12DescriptorHeapPtr heap)
		: heap_(std::move(heap)), cpu_handle_(heap_->GetCPUDescriptorHandleForHeapStart()),
		  gpu_handle_(heap_->GetGPUDescriptorHandleForHeapStart())
	{
	}


	void D3D12GpuDescriptorBlock::Reset(D3D12GpuDescriptorPage const& page, uint32_t offset, uint32_t size) noexcept
	{
		heap_ = page.Heap();
		offset_ = offset;
		size_ = size;

		uint32_t const desc_size = descriptor_size[heap_->GetDesc().Type];

		cpu_handle_ = {page.CpuHandle().ptr + offset * desc_size};
		gpu_handle_ = {page.GpuHandle().ptr + offset * desc_size};
	}


	D3D12GpuDescriptorAllocator::D3D12GpuDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) noexcept
		: type_(type), flags_(flags)
	{
	}

	uint32_t D3D12GpuDescriptorAllocator::DescriptorSize() const
	{
		return descriptor_size[type_];
	}

	std::unique_ptr<D3D12GpuDescriptorBlock> D3D12GpuDescriptorAllocator::Allocate(uint32_t size)
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		std::unique_ptr<D3D12GpuDescriptorBlock> desc_block;
		this->Allocate(lock, desc_block, size);
		return desc_block;
	}

	void D3D12GpuDescriptorAllocator::Allocate(
		std::lock_guard<std::mutex>& proof_of_lock, std::unique_ptr<D3D12GpuDescriptorBlock>& desc_block, uint32_t size)
	{
		KFL_UNUSED(proof_of_lock);

		if (descriptor_size[type_] == 0)
		{
			auto& d3d12_re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto* device = d3d12_re.D3DDevice();
			descriptor_size[type_] = device->GetDescriptorHandleIncrementSize(type_);
		}

		if (!desc_block)
		{
			desc_block = MakeUniquePtr<D3D12GpuDescriptorBlock>();
		}

		uint16_t const default_page_size = default_descriptor_page_size[type_];
		BOOST_ASSERT(size <= default_page_size);

		for (auto& page_info : pages_)
		{
			auto const iter = std::lower_bound(
				page_info.free_list.begin(), page_info.free_list.end(), size, [](PageInfo::FreeRange const& free_range, uint32_t size) {
					return free_range.first_offset + size > free_range.last_offset;
				});
			if (iter != page_info.free_list.end())
			{
				desc_block->Reset(*page_info.page, iter->first_offset, size);
				iter->first_offset += static_cast<uint16_t>(size);
				if (iter->first_offset == iter->last_offset)
				{
					page_info.free_list.erase(iter);
				}

				return;
			}
		}

		PageInfo new_page_info;
		new_page_info.page = this->CreatePage(default_page_size);
		new_page_info.free_list.push_back({static_cast<uint16_t>(size), default_page_size});
		desc_block->Reset(*new_page_info.page, 0, size);
		pages_.emplace_back(std::move(new_page_info));
	}

	void D3D12GpuDescriptorAllocator::Deallocate(std::unique_ptr<D3D12GpuDescriptorBlock> desc_block, uint64_t fence_value)
	{
		if (desc_block)
		{
			std::lock_guard<std::mutex> lock(allocation_mutex_);
			this->Deallocate(lock, desc_block.get(), fence_value);
		}
	}

	void D3D12GpuDescriptorAllocator::Deallocate(
		std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuDescriptorBlock* desc_block, uint64_t fence_value)
	{
		KFL_UNUSED(proof_of_lock);
		BOOST_ASSERT(desc_block != nullptr);

		uint16_t const default_page_size = default_descriptor_page_size[type_];

		if (desc_block->Size() <= default_page_size)
		{
			for (auto& page : pages_)
			{
				if (page.page->Heap() == desc_block->Heap())
				{
					page.stall_list.push_back(
						{{static_cast<uint16_t>(desc_block->Offset()), static_cast<uint16_t>(desc_block->Offset() + desc_block->Size())},
							fence_value});
					return;
				}
			}

			KFL_UNREACHABLE("This descriptor block is not allocated by this allocator");
		}
	}

	void D3D12GpuDescriptorAllocator::Renew(std::unique_ptr<D3D12GpuDescriptorBlock>& desc_block, uint64_t fence_value, uint32_t size)
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		if (desc_block)
		{
			this->Deallocate(lock, desc_block.get(), fence_value);
		}
		this->Allocate(lock, desc_block, size);
	}

	void D3D12GpuDescriptorAllocator::ClearStallPages(uint64_t fence_value)
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
	}

	void D3D12GpuDescriptorAllocator::Clear()
	{
		std::lock_guard<std::mutex> lock(allocation_mutex_);

		pages_.clear();
	}

	D3D12GpuDescriptorPagePtr D3D12GpuDescriptorAllocator::CreatePage(uint32_t size) const
	{
		auto& d3d12_re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* device = d3d12_re.D3DDevice();

		D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
		cbv_srv_heap_desc.Type = type_;
		cbv_srv_heap_desc.NumDescriptors = size;
		cbv_srv_heap_desc.Flags = flags_;
		cbv_srv_heap_desc.NodeMask = 0;
		ID3D12DescriptorHeapPtr heap;
		TIFHR(device->CreateDescriptorHeap(&cbv_srv_heap_desc, UuidOf<ID3D12DescriptorHeap>(), heap.put_void()));

		return MakeSharedPtr<D3D12GpuDescriptorPage>(std::move(heap));
	}
} // namespace KlayGE
