/**
 * @file TransientBuffer.hpp
 * @author Shenghua Lin, Minmin Gong
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

#ifndef _TRANSIENTBUFFER_HPP
#define _TRANSIENTBUFFER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <vector>
#include <list>

namespace KlayGE
{
	// A part of transient buffer that is allocated
	struct SubAlloc
	{
		uint32_t offset_;
		uint32_t length_;
		
		SubAlloc()
		{
		}
		SubAlloc(int offset, int length)
			: offset_(offset), length_(length)
		{
		}
	};

	class KLAYGE_CORE_API TransientBuffer final : boost::noncopyable
	{
		// Frames that have ended
		struct RetiredFrame
		{
			// Sub allocs that are unuseful and will be freed at the end of the frame
			std::list<SubAlloc> pending_frees_;
			uint32_t frame_id;

			explicit RetiredFrame(uint32_t id)
				: frame_id(id)
			{
			}
		};

	public:
		enum BindFlag
		{
			BF_Vertex,
			BF_Index
		};

	public:
		TransientBuffer(uint32_t size_in_byte, BindFlag bind_flag);

		// Allocate a sub space from transient buffer
		SubAlloc Alloc(uint32_t size_in_byte, void const * data);
		// Knowtify transient buffer that this alloc is unused and will be freed at the end of the frame.
		void Dealloc(SubAlloc const & alloc);
		void EnsureDataReady();
		// Do with retired frames
		void OnPresent();

		GraphicsBufferPtr const & GetBuffer() const
		{
			return buffer_;
		}

	private:
		GraphicsBufferPtr DoCreateBuffer(BindFlag bind_flag, uint32_t size_in_byte);
		// Free the sub alloc and return the space allocated back to transient buffer.
		void DoFree(SubAlloc const & alloc);

	private:
		bool use_no_overwrite_;
		uint32_t num_pre_frames_;

		GraphicsBufferPtr buffer_;
		std::list<SubAlloc> free_list_;
		std::list<RetiredFrame> retired_frames_;
		BindFlag bind_flag_;

		std::vector<uint8_t> simulate_buffer_;
		uint32_t valid_min_;
		uint32_t valid_max_;
	};
}

#endif
