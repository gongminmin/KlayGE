/**
 * @file TransientBuffer.cpp
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

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/App3D.hpp>

#include <cstring>

#include <KlayGE/TransientBuffer.hpp>

namespace KlayGE
{
	TransientBuffer::TransientBuffer(uint32_t size_in_byte, TransientBuffer::BindFlag bind_flag)
		: bind_flag_(bind_flag)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine const & re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		use_no_overwrite_ = caps.no_overwrite_support;

		buffer_ = this->DoCreateBuffer(bind_flag_, size_in_byte);
		if (use_no_overwrite_)
		{
			num_pre_frames_ = 3;
		}
		else
		{
			num_pre_frames_ = 1;
			simulate_buffer_.resize(buffer_->Size());
			valid_min_ = 0;
			valid_max_ = 0;
		}

		SubAlloc alloc(0, size_in_byte);
		free_list_.push_back(alloc);

		App3DFramework const & app = Context::Instance().AppInstance();
		retired_frames_.push_back(RetiredFrame(app.TotalNumFrames() + 1));
	}

	GraphicsBufferPtr TransientBuffer::DoCreateBuffer(TransientBuffer::BindFlag bind_flag, uint32_t size_in_byte)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		GraphicsBufferPtr buffer;
		switch (bind_flag)
		{
		case BF_Vertex:
			buffer = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, size_in_byte, nullptr);
			break;

		case BF_Index:
			buffer = rf.MakeIndexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, size_in_byte, nullptr);
			break;

		default:
			KFL_UNREACHABLE("Invalid bind flag");
		}
		return buffer;
	}

	SubAlloc TransientBuffer::Alloc(uint32_t size_in_byte, void const * data)
	{
		SubAlloc ret;

		// Use first fit method to find a free sub alloc
		bool found = false;
		auto iter = free_list_.begin();
		auto first_fit_iter = iter;
		for (; iter != free_list_.end(); ++ iter)
		{
			if (iter->length_ >= size_in_byte)
			{
				first_fit_iter = iter;
				found = true;
				break;
			}
		}

		// If there is not enough space, reallocate a larger buffer.
		if (!found)
		{
			uint32_t const old_buffer_size = buffer_->Size();
			uint32_t larger_buffer_size = std::max(old_buffer_size * 2, old_buffer_size + size_in_byte);
			GraphicsBufferPtr larger_buffer = this->DoCreateBuffer(bind_flag_, larger_buffer_size);
			SubAlloc alloc(old_buffer_size, larger_buffer_size - old_buffer_size);
			if (!free_list_.empty() && (free_list_.back().offset_ + free_list_.back().length_ == alloc.offset_))
			{
				free_list_.back().length_ += alloc.length_;
			}
			else
			{
				free_list_.push_back(alloc);
			}
			first_fit_iter = free_list_.end();
			-- first_fit_iter;
			if (use_no_overwrite_)
			{
				buffer_->CopyToBuffer(*larger_buffer);
			}
			else
			{
				simulate_buffer_.resize(larger_buffer_size);
			}
			buffer_ = larger_buffer;
		}

		uint32_t left_size = first_fit_iter->length_ - size_in_byte;
		ret.length_ = size_in_byte;
		ret.offset_ = first_fit_iter->offset_;
		if (0 == left_size)
		{
			free_list_.erase(first_fit_iter);
		}
		else
		{
			first_fit_iter->length_ = left_size;
			first_fit_iter->offset_ += size_in_byte;
		}

		if (use_no_overwrite_)
		{
			GraphicsBuffer::Mapper mapper(*buffer_, BA_Write_No_Overwrite);
			uint8_t* buffer_data = mapper.Pointer<uint8_t>();
			memcpy(buffer_data + ret.offset_, data, ret.length_);
		}
		else
		{
			memcpy(&simulate_buffer_[ret.offset_], data, ret.length_);
			valid_min_ = std::min(valid_min_, ret.offset_);
			valid_max_ = std::max(valid_max_, ret.offset_ + ret.length_);
		}

		return ret;
	}

	void TransientBuffer::Dealloc(SubAlloc const & alloc)
	{
		if ((alloc.length_ > 0) && !retired_frames_.empty())
		{
			RetiredFrame& frame = retired_frames_.back();
			frame.pending_frees_.push_back(alloc);

			if (!use_no_overwrite_)
			{
				if (alloc.offset_ == valid_min_)
				{
					valid_min_ += alloc.length_;
				}
				if (alloc.offset_ + alloc.length_ == valid_max_)
				{
					valid_max_ -= alloc.length_;
				}
			}
		}
	}

	void TransientBuffer::OnPresent()
	{
		if (!retired_frames_.empty())
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			uint32_t const frame_id = app.TotalNumFrames();

			// First, deal with deletes from this frame
			RetiredFrame& ret_frame = retired_frames_.back();
			if (!ret_frame.pending_frees_.empty()) 
			{
				// Append a new (empty) RetiredFrame to retired_frames_
				retired_frames_.push_back(RetiredFrame(frame_id + 1));
			}
			// Second, return pending frees to free_list
			auto frame_iter = retired_frames_.begin();
			for (; frame_iter != retired_frames_.end();)
			{
				if (frame_iter->frame_id + num_pre_frames_ <= frame_id)
				{
					auto iter = frame_iter->pending_frees_.begin();
					for (; iter != frame_iter->pending_frees_.end(); ++ iter)
					{
						this->DoFree(*iter);
					}
					frame_iter = retired_frames_.erase(frame_iter);
				}
				else
				{
					++ frame_iter;
				}
			}
		}
	}

	void TransientBuffer::DoFree(SubAlloc const & alloc)
	{
		if (free_list_.empty())
		{
			free_list_.push_back(alloc);
		}
		else
		{
			// Find where to insert the alloc
			auto insert_position = free_list_.begin();
			while (insert_position != free_list_.end())
			{
				if (insert_position->offset_ > alloc.offset_)
				{
					break;
				}
				++ insert_position;
			}

			bool left_merged = false;
			// If the alloc is adjacent to previous alloc, merge them.
			auto previous = insert_position;
			if (insert_position != free_list_.begin())
			{
				-- previous;
				if (previous->offset_ + previous->length_ == alloc.offset_)
				{
					previous->length_ += alloc.length_;
					left_merged = true;
				}
			}

			bool right_merged = false;
			// If the alloc is adjecent to next alloc, merge them.
			auto next = insert_position;
			if (insert_position != free_list_.end())
			{
				if (left_merged)
				{
					if (previous->offset_ + previous->length_ == next->offset_)
					{
						previous->length_ += next->length_;
						free_list_.erase(next);
						right_merged = true;
					}
				}
				else
				{
					if (alloc.offset_ + alloc.length_ == next->offset_)
					{
						next->offset_ -= alloc.length_;
						next->length_ += alloc.length_;
						right_merged = true;
					}
				}
			}
			if (!(left_merged || right_merged))
			{
				free_list_.emplace(insert_position, alloc);
			}
		}
	}

	void TransientBuffer::EnsureDataReady()
	{
		if (!use_no_overwrite_)
		{
			GraphicsBuffer::Mapper mapper(*buffer_, BA_Write_Only);
			memcpy(mapper.Pointer<uint8_t>() + valid_min_, &simulate_buffer_[valid_min_],
				valid_max_ - valid_min_);
		}
	}
}
