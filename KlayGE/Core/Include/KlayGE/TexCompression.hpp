/**
* @file TexCompression.hpp
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

#ifndef _TEXCOMPRESSION_HPP
#define _TEXCOMPRESSION_HPP

#pragma once

namespace KlayGE
{
	enum TexCompressionMethod
	{
		TCM_Speed,
		TCM_Balanced,
		TCM_Quality
	};

	class KLAYGE_CORE_API TexCompression
	{
	public:
		virtual ~TexCompression()
		{
		}

		uint32_t BlockWidth() const
		{
			return block_width_;
		}
		uint32_t BlockHeight() const
		{
			return block_height_;
		}
		uint32_t BlockDepth() const
		{
			return block_depth_;
		}
		uint32_t BlockBytes() const
		{
			return block_bytes_;
		}
		ElementFormat DecodedFormat() const
		{
			return decoded_fmt_;
		}

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) = 0;
		virtual void DecodeBlock(void* output, void const * input) = 0;

		virtual void EncodeMem(uint32_t width, uint32_t height, 
			void* output, uint32_t out_row_pitch, uint32_t out_slice_pitch,
			void const * input, uint32_t in_row_pitch, uint32_t in_slice_pitch,
			TexCompressionMethod method);
		virtual void DecodeMem(uint32_t width, uint32_t height,
			void* output, uint32_t out_row_pitch, uint32_t out_slice_pitch,
			void const * input, uint32_t in_row_pitch, uint32_t in_slice_pitch);

		virtual void EncodeTex(TexturePtr const & out_tex, TexturePtr const & in_tex, TexCompressionMethod method);
		virtual void DecodeTex(TexturePtr const & out_tex, TexturePtr const & in_tex);

	protected:
		uint32_t block_width_;
		uint32_t block_height_;
		uint32_t block_depth_;
		uint32_t block_bytes_;
		ElementFormat decoded_fmt_;
	};
}

#endif		// _TEXCOMPRESSIONBC_HPP
