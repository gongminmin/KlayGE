/**
 * @file TransientBuffer.cpp
 * @author Shenghua Lin,Minmin Gong
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
#include <KlayGE/TransientBuffer.hpp>

namespace KlayGE
{
	TransientBuffer::TransientBuffer(uint32_t size_in_byte,BindFlag bind_flag):
	buffer_size_(size_in_byte),
	bind_flag_(bind_flag),
	frame_id_(1)
	{
		//Create buffer
		buffer_=CreateBuffer(bind_flag_);
		buffer_->Resize(buffer_size_);

		//Initialize free list
		SubAlloc alloc(0,size_in_byte);
		free_list_.push_back(alloc);

		//Initialize retired frame list
		retired_frames_.push_back(RetiredFrame(frame_id_));
	}

	GraphicsBufferPtr TransientBuffer::CreateBuffer(BindFlag bind_flag)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		GraphicsBufferPtr buffer;
		switch(bind_flag)
		{
		case BF_Vertex:
			buffer = rf.MakeVertexBuffer(BU_Dynamic,BA_Write_No_Overwrite,nullptr);
			break;
		case BF_Index:
			buffer = rf.MakeIndexBuffer(BU_Dynamic,BA_Write_No_Overwrite,nullptr);
			break;
		default:
			BOOST_ASSERT(false);
		}
		return buffer;
	}

	SubAlloc TransientBuffer::Alloc(uint32_t size_in_byte, void* data)
	{
		SubAlloc ret;

		//Use first fit method to find a free sub alloc
		bool find = false;
		std::list<SubAlloc>::iterator itr = free_list_.begin();
		std::list<SubAlloc>::iterator first_fit_itr = itr;
		for (;itr != free_list_.end();++itr)
		{
			if (itr->length_ == size_in_byte || itr->length_ - size_in_byte >=4)
			{
				first_fit_itr = itr;
				find = true;
				break;
			}
		}

		//If there is not enough space,reallocate a larger buffer.
		if(!find)
		{
			GraphicsBufferPtr larger_buffer = CreateBuffer(bind_flag_);
			uint32_t larger_buffer_size = buffer_size_ * 2 > buffer_size_ + size_in_byte ? buffer_size_ * 2 : buffer_size_ + size_in_byte;
			larger_buffer->Resize(larger_buffer_size);
			SubAlloc alloc(buffer_size_,larger_buffer_size - buffer_size_);
			buffer_size_ = larger_buffer_size;
			free_list_.push_back(alloc);
			first_fit_itr = free_list_.end();
			--first_fit_itr;
			buffer_->CopyToBuffer(*larger_buffer);
			buffer_=larger_buffer;
		}
		uint32_t left_size = first_fit_itr->length_ - size_in_byte;
		ret.length_ = size_in_byte;
		ret.offset_ = first_fit_itr->offset_;
		ret.data_ = data;
		if (0 == left_size)
		{
			free_list_.erase(first_fit_itr);
		}else
		{
			first_fit_itr->length_ = left_size;
			first_fit_itr->offset_ += size_in_byte;
		}

		//Submit data ,it will be cached,UploadData() will submit all the data.
		FeedData(ret);

		return ret;
	}

	void TransientBuffer::KnowtifyUnused(SubAlloc const & alloc)
	{
		RetiredFrame& frame = retired_frames_.back();
		frame.pending_frees_.push_back(alloc);
	}

	void TransientBuffer::OnPresent()
	{
		//First, deal with deletes from this frame
		RetiredFrame& retFrame = retired_frames_.back();
		if (!retFrame.pending_frees_.empty()) 
		{
			// Append a new (empty) RetiredFrame to retired_frames_
			//retFrame.frame_complete_query_ = CreateAndIssueFrameCompleteQuery();        
			retired_frames_.push_back(RetiredFrame(frame_id_+1));
		}
		// Second, return pending frees to free_list
		std::list<RetiredFrame>::iterator frameIt = retired_frames_.begin();
		for (;frameIt != retired_frames_.end();)
		{
			//auto query = frameIt->frame_complete_query_;
			if (frameIt->frameID == (frame_id_-3)) 
			{
				std::list<SubAlloc>::iterator itr = frameIt->pending_frees_.begin();
				for (;itr != frameIt->pending_frees_.end();++itr)
				{
					Free(*itr);
				}
				frameIt = retired_frames_.erase(frameIt);
			}else
			{
				++frameIt;
			}
		}
		++frame_id_;
		
	}

	/*QueryPtr TransientBuffer::CreateAndIssueFrameCompleteQuery()const
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		QueryPtr query = rf.MakeFrameCompleteQuery();
		query->End();
		return query;
	}

	bool TransientBuffer::IsQueryComplete(QueryPtr query)const 
	{
		FrameCompleteQuery* frame_complete_query = checked_cast<FrameCompleteQuery*>(query.get());
		bool ret = frame_complete_query->IsQueryComplete();
		return ret;
	}*/

	void TransientBuffer::Free(SubAlloc const & alloc)
	{
		if(free_list_.empty())
		{
			free_list_.push_back(alloc);
			return;
		}

		//Find where to insert the alloc
		std::list<SubAlloc>::iterator insert_position = free_list_.begin();
		while(insert_position != free_list_.end())
		{
			if(insert_position->offset_ > alloc.offset_)
			{
				break;
			}
			++insert_position;
		}

		bool left_merged = false;
		//If the alloc is adjacent to previous alloc,merge them.
		std::list<SubAlloc>::iterator previous = insert_position;
		if(insert_position != free_list_.begin())
		{
			--previous;
			if(previous->offset_ + previous->length_ == alloc.offset_)
			{
				previous->length_ += alloc.length_;
				left_merged = true;
			}
		}

		bool right_merged = false;
		//If the alloc is adjecent to next alloc,merge them.
		std::list<SubAlloc>::iterator next = insert_position;
		if(insert_position != free_list_.end())
		{
			if(left_merged)
			{
				if(previous->offset_ + previous->length_ == next->offset_)
				{
					previous->length_ += next->length_;
					free_list_.erase(next);
					right_merged = true;
				}
			}
			else
			{
				if(alloc.offset_ + alloc.length_ == next->offset_)
				{
					next->offset_ -= alloc.length_;
					next->length_ += alloc.length_;
					right_merged = true;
				}
			}
		}
		if(!left_merged && !right_merged)
		{
			free_list_.insert(insert_position,alloc);
		}
	}

	void TransientBuffer::FeedData(SubAlloc const & alloc)
	{
		data_list_.push_back(alloc);
	}

	void TransientBuffer::UploadData()
	{
		//feed data to suballoc
		{
			GraphicsBuffer::Mapper mapper(*buffer_, BA_Write_No_Overwrite);
			char* buffer_data = mapper.Pointer<char>();

			std::vector<SubAlloc>::iterator itr = data_list_.begin();
			for (;itr != data_list_.end();++itr)
			{
				char* p = buffer_data+itr->offset_;
				memcpy(p,itr->data_,itr->length_);
			}
		}

		//clear data list
		data_list_.clear();
	}
}
