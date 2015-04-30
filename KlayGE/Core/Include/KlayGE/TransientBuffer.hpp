/**
 * @file TransientBuffer.hpp
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

#ifndef _TRANSIENTBUFFER_HPP
#define _TRANSIENTBUFFER_HPP

#pragma once

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Query.hpp>

namespace KlayGE
{
	enum BindFlag
	{
		BF_Vertex,
		BF_Index
	};

	//A part of transient buffer that is allocated
	struct SubAlloc
	{
		uint32_t offset_;
		uint32_t length_;
		void* data_;
		SubAlloc():offset_(0),length_(0),data_(0){};
		SubAlloc(int offset,int length):offset_(offset),length_(length),data_(0){};
	};

	//Frames that have ended
	struct RetiredFrame
	{
		//Sub allocs that are unuseful and will be freed at the end of the frame
		std::list<SubAlloc> pending_frees_;
		//QueryPtr frame_complete_query_;
		int frameID;
		RetiredFrame(int frameID):
		frameID(frameID)
		{
		
		};
	};

	class TransientBuffer
	{
	public:
		TransientBuffer(uint32_t size_in_byte,BindFlag bind_flag);
		~TransientBuffer();
		GraphicsBufferPtr CreateBuffer(BindFlag bind_flag);
		//Allocate a sub space from transient buffer
		SubAlloc Alloc(uint32_t size_in_byte, void* data);
		//Knowtify transient buffer that this alloc is unused and will be freed at the end of the frame.
		void KnowtifyUnused(SubAlloc const & alloc);
		//Free the sub alloc and return the space allocated back to transient buffer.
		void Free(SubAlloc const & alloc);
		//Feed data to transient buffer and cache them.
		void FeedData(SubAlloc const & alloc);
		//Bufferred Map/Unmap.
		void UploadData();
		//Do with retired frames
		void OnPresent();
		//QueryPtr CreateAndIssueFrameCompleteQuery() const;
		//bool IsQueryComplete(QueryPtr query) const;

	private:
		GraphicsBufferPtr buffer_;
		uint32_t buffer_size_;
		std::list<SubAlloc> free_list_;
		std::vector<SubAlloc> data_list_;
		std::list<RetiredFrame> retired_frames_;
		BindFlag bind_flag_;
		uint32_t frame_id_;
	};
}

#endif
