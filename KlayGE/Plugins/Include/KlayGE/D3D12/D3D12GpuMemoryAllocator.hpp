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

#include <map>
#include <memory>

#include <KlayGE/D3D12/D3D12Typedefs.hpp>

namespace KlayGE
{
	class D3D12GpuMemoryPage : boost::noncopyable
	{
	public:
		D3D12GpuMemoryPage(bool is_upload, ID3D12ResourcePtr resource, uint32_t size_in_bytes);
		~D3D12GpuMemoryPage();

		ID3D12Resource* Resource() const
		{
			return resource_.get();
		}

		uint32_t Size() const
		{
			return size_in_bytes_;
		}

	private:
		bool const is_upload_;
		ID3D12ResourcePtr const resource_;
		uint32_t const size_in_bytes_;
	};
	using D3D12GpuMemoryPagePtr = std::shared_ptr<D3D12GpuMemoryPage>;

	class D3D12GpuMemoryAllocator : boost::noncopyable
	{
	public:
		explicit D3D12GpuMemoryAllocator(bool is_upload);

		// TODO: Enable to allocate a block from a page
		D3D12GpuMemoryPagePtr Allocate(uint32_t size_in_bytes);
		void Deallocate(D3D12GpuMemoryPagePtr page);

		void ClearStallPages();

		void Clear();

	private:
		bool const is_upload_;

		struct FrameContext
		{
			std::vector<D3D12GpuMemoryPagePtr> stall_pages;
		};
		std::array<FrameContext, NUM_BACK_BUFFERS> frame_contexts_;

		std::multimap<uint32_t, D3D12GpuMemoryPagePtr> available_pages_;
	};
} // namespace KlayGE

#endif // KLAYGE_PLUGINS_D3D12_GPU_MEMORY_ALLOCATOR_HPP
