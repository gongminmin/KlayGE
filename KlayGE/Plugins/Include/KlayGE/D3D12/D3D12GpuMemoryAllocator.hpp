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
	class D3D12GpuMemoryPage final : boost::noncopyable
	{
	public:
		D3D12GpuMemoryPage(bool is_upload, ID3D12ResourcePtr resource);
		~D3D12GpuMemoryPage();

		ID3D12Resource* Resource() const noexcept
		{
			return resource_.get();
		}

		void* CpuAddress() const noexcept
		{
			return cpu_addr_;
		}

		D3D12_GPU_VIRTUAL_ADDRESS GpuAddress() const noexcept
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

	class D3D12GpuMemoryBlock final : boost::noncopyable
	{
	public:
		D3D12GpuMemoryBlock() noexcept;

		void Reset(D3D12GpuMemoryPage const& page, uint32_t offset, uint32_t size) noexcept;

		ID3D12Resource* Resource() const noexcept
		{
			return resource_;
		}

		uint32_t Offset() const noexcept
		{
			return offset_;
		}

		uint32_t Size() const noexcept
		{
			return size_;
		}

		void* CpuAddress() const noexcept
		{
			return cpu_addr_;
		}

		D3D12_GPU_VIRTUAL_ADDRESS GpuAddress() const noexcept
		{
			return gpu_addr_;
		}

	private:
		ID3D12Resource* resource_ = nullptr;
		uint32_t offset_ = 0;
		uint32_t size_ = 0;
		void* cpu_addr_ = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS gpu_addr_ = 0;
	};

	class D3D12GpuMemoryAllocator final : boost::noncopyable
	{
	public:
		static constexpr uint32_t DefaultAligment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

	public:
		explicit D3D12GpuMemoryAllocator(bool is_upload) noexcept;

		std::unique_ptr<D3D12GpuMemoryBlock> Allocate(uint32_t size_in_bytes, uint32_t alignment = DefaultAligment);
		void Deallocate(std::unique_ptr<D3D12GpuMemoryBlock> mem_block, uint64_t fence_value);
		void Renew(std::unique_ptr<D3D12GpuMemoryBlock>& mem_block, uint64_t fence_value, uint32_t size_in_bytes,
			uint32_t alignment = DefaultAligment);

		void ClearStallPages(uint64_t fence_value);
		void Clear();

	private:
		void Allocate(std::lock_guard<std::mutex>& proof_of_lock, std::unique_ptr<D3D12GpuMemoryBlock>& mem_block, uint32_t size_in_bytes,
			uint32_t alignment);
		void Deallocate(std::lock_guard<std::mutex>& proof_of_lock, std::unique_ptr<D3D12GpuMemoryBlock>& mem_block, uint64_t fence_value);
		D3D12GpuMemoryPagePtr CreatePage(uint32_t size_in_bytes) const;

	private:
		static constexpr uint32_t DefaultPageSize = 2 * 1024 * 1024;

		bool const is_upload_;

		std::mutex allocation_mutex_;

		struct PageInfo
		{
			D3D12GpuMemoryPagePtr page;

			struct FreeRange
			{
				uint32_t first_offset;
				uint32_t last_offset;
			};
			std::vector<FreeRange> free_list;

			struct StallRange
			{
				FreeRange free_range;
				uint64_t fence_value;
			};
			std::vector<StallRange> stall_list;
		};
		std::vector<PageInfo> pages_;

		std::vector<D3D12GpuMemoryPagePtr> large_pages_;
	};
} // namespace KlayGE

#endif // KLAYGE_PLUGINS_D3D12_GPU_MEMORY_ALLOCATOR_HPP
