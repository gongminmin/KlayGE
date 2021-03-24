/**
 * @file D3D12GpuDescriptorAllocator.hpp
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

#ifndef KLAYGE_PLUGINS_D3D12_GPU_DESCRIPTOR_ALLOCATOR_HPP
#define KLAYGE_PLUGINS_D3D12_GPU_DESCRIPTOR_ALLOCATOR_HPP

#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include <KlayGE/D3D12/D3D12Util.hpp>

namespace KlayGE
{
	class D3D12GpuDescriptorPage final
	{
		D3D12GpuDescriptorPage(D3D12GpuDescriptorPage const& other) = delete;
		D3D12GpuDescriptorPage& operator=(D3D12GpuDescriptorPage const& other) = delete;

	public:
		D3D12GpuDescriptorPage(uint32_t size, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

		D3D12GpuDescriptorPage(D3D12GpuDescriptorPage&& other) noexcept;
		D3D12GpuDescriptorPage& operator=(D3D12GpuDescriptorPage&& other) noexcept;

		ID3D12DescriptorHeap* Heap() const noexcept
		{
			return heap_.get();
		}

		D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle() const noexcept
		{
			return cpu_handle_;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle() const noexcept
		{
			return gpu_handle_;
		}

	private:
		ID3D12DescriptorHeapPtr heap_;
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle_;
	};

	class D3D12GpuDescriptorBlock final
	{
		D3D12GpuDescriptorBlock(D3D12GpuDescriptorBlock const& other) = delete;
		D3D12GpuDescriptorBlock& operator=(D3D12GpuDescriptorBlock const& other) = delete;

	public:
		D3D12GpuDescriptorBlock() noexcept;
		D3D12GpuDescriptorBlock(D3D12GpuDescriptorBlock&& other) noexcept;
		D3D12GpuDescriptorBlock& operator=(D3D12GpuDescriptorBlock&& other) noexcept;

		void Reset() noexcept;
		void Reset(D3D12GpuDescriptorPage const& page, uint32_t offset, uint32_t size) noexcept;

		explicit operator bool() const noexcept
		{
			return heap_ != nullptr;
		}

		ID3D12DescriptorHeap* Heap() const noexcept
		{
			return heap_;
		}

		uint32_t Offset() const noexcept
		{
			return offset_;
		}

		uint32_t Size() const noexcept
		{
			return size_;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle() const noexcept
		{
			return cpu_handle_;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle() const noexcept
		{
			return gpu_handle_;
		}

	private:
		ID3D12DescriptorHeap* heap_ = nullptr;
		uint32_t offset_ = 0;
		uint32_t size_ = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle_{};
	};

	class D3D12GpuDescriptorAllocator final
	{
		D3D12GpuDescriptorAllocator(D3D12GpuDescriptorAllocator const& other) = delete;
		D3D12GpuDescriptorAllocator& operator=(D3D12GpuDescriptorAllocator const& other) = delete;

	public:
		D3D12GpuDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) noexcept;

		D3D12GpuDescriptorAllocator(D3D12GpuDescriptorAllocator&& other) noexcept;
		D3D12GpuDescriptorAllocator& operator=(D3D12GpuDescriptorAllocator&& other) noexcept;

		uint32_t DescriptorSize() const;

		D3D12GpuDescriptorBlock Allocate(uint32_t size);
		void Deallocate(D3D12GpuDescriptorBlock&& desc_block, uint64_t fence_value);
		void Renew(D3D12GpuDescriptorBlock& desc_block, uint64_t fence_value, uint32_t size);

		void ClearStallPages(uint64_t fence_value);
		void Clear();

	private:
		void Allocate(std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuDescriptorBlock& desc_block, uint32_t size);
		void Deallocate(std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuDescriptorBlock& desc_block, uint64_t fence_value);

	private:
		D3D12_DESCRIPTOR_HEAP_TYPE const type_;
		D3D12_DESCRIPTOR_HEAP_FLAGS const flags_;

		std::mutex allocation_mutex_;

		struct PageInfo
		{
			D3D12GpuDescriptorPage page;

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
			struct FreeRange
			{
				uint16_t first_offset;
				uint16_t last_offset;
			};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif
			std::vector<FreeRange> free_list;

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
			struct StallRange
			{
				FreeRange free_range;
				uint64_t fence_value;
			};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif
			std::vector<StallRange> stall_list;
		};
		std::vector<PageInfo> pages_;
	};
} // namespace KlayGE

#endif // KLAYGE_PLUGINS_D3D12_GPU_DESCRIPTOR_ALLOCATOR_HPP
