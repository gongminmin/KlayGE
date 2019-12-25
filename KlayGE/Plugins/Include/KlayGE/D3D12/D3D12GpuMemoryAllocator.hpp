/**
 * @file D3D12GpuMemoryAllocator.hpp
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

#ifndef KLAYGE_PLUGINS_D3D12_GPU_MEMORY_ALLOCATOR_HPP
#define KLAYGE_PLUGINS_D3D12_GPU_MEMORY_ALLOCATOR_HPP

#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include <KlayGE/D3D12/D3D12Typedefs.hpp>

namespace KlayGE
{
	class D3D12GpuMemoryPage : boost::noncopyable
	{
	public:
		D3D12GpuMemoryPage(bool is_upload, ID3D12ResourcePtr resource);
		~D3D12GpuMemoryPage();

		ID3D12Resource* Resource() const
		{
			return resource_.get();
		}

		void* CpuAddress() const
		{
			return cpu_addr_;
		}

		D3D12_GPU_VIRTUAL_ADDRESS GpuAddress() const
		{
			return gpu_addr_;
		}

	private:
		bool const is_upload_;
		ID3D12ResourcePtr const resource_;
		void* cpu_addr_;
		D3D12_GPU_VIRTUAL_ADDRESS const gpu_addr_;
	};
	using D3D12GpuMemoryPagePtr = std::shared_ptr<D3D12GpuMemoryPage>;

	class D3D12GpuMemoryBlock : boost::noncopyable
	{
	public:
		D3D12GpuMemoryBlock(D3D12GpuMemoryPage const& page, uint32_t offset, uint32_t size);

		ID3D12Resource* Resource() const
		{
			return resource_;
		}

		uint32_t Offset() const
		{
			return offset_;
		}

		void* CpuAddress() const
		{
			return cpu_addr_;
		}

		D3D12_GPU_VIRTUAL_ADDRESS GpuAddress() const
		{
			return gpu_addr_;
		}

	private:
		ID3D12Resource* const resource_;
		uint32_t const offset_;
		uint32_t const size_;
		void* const cpu_addr_;
		D3D12_GPU_VIRTUAL_ADDRESS const gpu_addr_;
	};
	using D3D12GpuMemoryBlockPtr = std::shared_ptr<D3D12GpuMemoryBlock>;

	class D3D12GpuMemoryAllocator : boost::noncopyable
	{
	public:
		static constexpr uint32_t DefaultAligment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

	public:
		explicit D3D12GpuMemoryAllocator(bool is_upload);

		D3D12GpuMemoryBlockPtr Allocate(uint32_t size_in_bytes, uint32_t alignment = DefaultAligment);
		void Deallocate(D3D12GpuMemoryBlockPtr mem_block);

		void ClearStallPages();

		void Clear();

	private:
		D3D12GpuMemoryPagePtr CreatePage(uint32_t size_in_bytes);
		D3D12GpuMemoryPagePtr CreateLargePage(uint32_t size_in_bytes);

	private:
		static constexpr uint32_t DefaultPageSize = 2 * 1024 * 1024;

		bool const is_upload_;
		uint32_t page_size_ = DefaultPageSize;

		std::mutex allocation_mutex_;
		D3D12GpuMemoryPagePtr curr_page_;
		uint32_t curr_offset_ = ~0U;

		struct FrameContext
		{
			std::vector<D3D12GpuMemoryPagePtr> stall_pages;
			std::vector<D3D12GpuMemoryPagePtr> large_pages;
		};
		std::array<FrameContext, NUM_BACK_BUFFERS> frame_contexts_;
	};
} // namespace KlayGE

#endif // KLAYGE_PLUGINS_D3D12_GPU_MEMORY_ALLOCATOR_HPP
