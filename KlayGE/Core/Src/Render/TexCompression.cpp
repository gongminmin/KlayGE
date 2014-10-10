/**
* @file TexCompression.cpp
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
#include <KlayGE/Texture.hpp>

#include <vector>
#include <cstring>

#include <KlayGE/TexCompression.hpp>

namespace KlayGE
{
	void TexCompression::EncodeMem(uint32_t width, uint32_t height,
		void* output, uint32_t out_row_pitch, uint32_t out_slice_pitch,
		void const * input, uint32_t in_row_pitch, uint32_t in_slice_pitch,
		TexCompressionMethod method)
	{
		UNREF_PARAM(out_slice_pitch);
		UNREF_PARAM(in_slice_pitch);

		uint32_t const elem_size = NumFormatBytes(decoded_fmt_);

		uint8_t const * src = static_cast<uint8_t const *>(input);

		std::vector<uint8_t> uncompressed(block_width_ * block_height_ * elem_size);
		for (uint32_t y_base = 0; y_base < height; y_base += block_height_)
		{
			uint8_t* dst = static_cast<uint8_t*>(output) + (y_base / block_height_) * out_row_pitch;

			for (uint32_t x_base = 0; x_base < width; x_base += block_width_)
			{
				for (uint32_t y = 0; y < block_height_; ++ y)
				{
					for (uint32_t x = 0; x < block_width_; ++ x)
					{
						if ((x_base + x < width) && (y_base + y < height))
						{
							memcpy(&uncompressed[(y * block_width_ + x) * elem_size],
								&src[(y_base + y) * in_row_pitch + (x_base + x) * elem_size],
								elem_size);
						}
						else
						{
							memset(&uncompressed[(y * block_width_ + x) * elem_size],
								0, elem_size);
						}
					}
				}

				this->EncodeBlock(dst, &uncompressed[0], method);
				dst += block_bytes_;
			}
		}
	}

	void TexCompression::DecodeMem(uint32_t width, uint32_t height,
		void* output, uint32_t out_row_pitch, uint32_t out_slice_pitch,
		void const * input, uint32_t in_row_pitch, uint32_t in_slice_pitch)
	{
		UNREF_PARAM(out_slice_pitch);
		UNREF_PARAM(in_slice_pitch);

		uint32_t const elem_size = NumFormatBytes(decoded_fmt_);

		uint8_t * dst = static_cast<uint8_t*>(output);

		std::vector<uint8_t> uncompressed(block_width_ * block_height_ * elem_size);
		for (uint32_t y_base = 0; y_base < height; y_base += block_height_)
		{
			uint8_t const * src = static_cast<uint8_t const *>(input) + in_row_pitch * (y_base / block_height_);

			uint32_t const block_h = std::min(block_height_, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += block_width_)
			{
				uint32_t const block_w = std::min(block_width_, width - x_base);

				this->DecodeBlock(&uncompressed[0], src);
				src += block_bytes_;

				for (uint32_t y = 0; y < block_h; ++ y)
				{
					for (uint32_t x = 0; x < block_w; ++ x)
					{
						memcpy(&dst[(y_base + y) * out_row_pitch + (x_base + x) * elem_size],
							&uncompressed[(y * block_width_ + x) * elem_size], elem_size);
					}
				}
			}
		}
	}

	void TexCompression::EncodeTex(TexturePtr const & out_tex, TexturePtr const & in_tex, TexCompressionMethod method)
	{
		uint32_t width = in_tex->Width(0);
		uint32_t height = in_tex->Height(0);

		TexturePtr uncompressed_tex;
		if (uncompressed_tex->Format() != decoded_fmt_)
		{
			uncompressed_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height,
				1, 1, decoded_fmt_, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
			in_tex->CopyToTexture(*uncompressed_tex);
		}
		else
		{
			uncompressed_tex = in_tex;
		}

		Texture::Mapper mapper_src(*uncompressed_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
		Texture::Mapper mapper_dst(*out_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
		this->EncodeMem(width, height, mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_dst.SlicePitch(),
			mapper_src.Pointer<void>(), mapper_src.RowPitch(), mapper_src.SlicePitch(), method);
	}

	void TexCompression::DecodeTex(TexturePtr const & out_tex, TexturePtr const & in_tex)
	{
		uint32_t width = in_tex->Width(0);
		uint32_t height = in_tex->Height(0);

		TexturePtr decoded_tex;
		if (out_tex->Format() != decoded_fmt_)
		{
			decoded_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height,
				1, 1, decoded_fmt_, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
		}
		else
		{
			decoded_tex = out_tex;
		}

		{
			Texture::Mapper mapper_src(*in_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*decoded_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			this->DecodeMem(width, height, mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_dst.SlicePitch(),
				mapper_src.Pointer<void>(), mapper_src.RowPitch(), mapper_src.SlicePitch());
		}

		if (out_tex->Format() != decoded_fmt_)
		{
			decoded_tex->CopyToTexture(*out_tex);
		}
	}
}
