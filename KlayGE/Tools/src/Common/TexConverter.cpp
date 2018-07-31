/**
 * @file TexConverter.cpp
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

#include <cstring>

#include <KlayGE/TexConverter.hpp>
#include "ImagePlane.hpp"

namespace KlayGE
{
	bool TexConverter::Convert(std::string_view input_name, TexMetadata const & metadata,
		Texture::TextureType& output_type, uint32_t& output_width, uint32_t& output_height, uint32_t& output_depth,
		uint32_t& output_num_mipmaps, uint32_t& output_array_size,
		ElementFormat& output_format, std::vector<ElementInitData>& output_init_data, std::vector<uint8_t>& output_data_block)
	{
		input_name_ = std::string(input_name);
		metadata_ = metadata;

		if (!this->Load())
		{
			return false;
		}

		this->Save(output_type, output_width, output_height, output_depth, output_num_mipmaps, output_array_size,
			output_format, output_init_data, output_data_block);

		return true;
	}

	bool TexConverter::Load()
	{
		array_size_ = metadata_.ArraySize();

		planes_.resize(array_size_);
		planes_[0].resize(1);
		planes_[0][0] = MakeSharedPtr<ImagePlane>();
		auto& first_image = *planes_[0][0];

		first_image.Load(input_name_, metadata_);

		width_ = first_image.Width();
		height_ = first_image.Height();
		if (first_image.CompressedFormat() != EF_Unknown)
		{
			format_ = first_image.CompressedFormat();
		}
		else
		{
			format_ = first_image.UncompressedFormat();
		}
		BOOST_ASSERT(format_ != EF_Unknown);
		if (metadata_.PreferedFormat() == EF_Unknown)
		{
			metadata_.PreferedFormat(format_);
		}

		if (metadata_.MipmapEnabled())
		{
			if (metadata_.AutoGenMipmap())
			{
				if (metadata_.NumMipmaps() == 0)
				{
					num_mipmaps_ = 1;
					uint32_t w = width_;
					uint32_t h = height_;
					while ((w != 1) || (h != 1))
					{
						++ num_mipmaps_;

						w = std::max<uint32_t>(1U, w / 2);
						h = std::max<uint32_t>(1U, h / 2);
					}
				}
				else
				{
					num_mipmaps_ = metadata_.NumMipmaps();
				}
			}
			else
			{
				num_mipmaps_ = metadata_.NumMipmaps();
			}
		}
		else
		{
			num_mipmaps_ = 1;
		}

		if ((num_mipmaps_ > 1) && metadata_.AutoGenMipmap())
		{
			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				planes_[arr].resize(num_mipmaps_);

				if (arr > 0)
				{
					planes_[arr][0] = MakeSharedPtr<ImagePlane>();
					planes_[arr][0]->Load(metadata_.PlaneFileName(arr, 0), metadata_);
				}

				uint32_t w = width_;
				uint32_t h = height_;
				for (uint32_t m = 0; m < num_mipmaps_ - 1; ++ m)
				{
					w = std::max<uint32_t>(1U, w / 2);
					h = std::max<uint32_t>(1U, h / 2);

					planes_[arr][m + 1] = MakeSharedPtr<ImagePlane>();
					*planes_[arr][m + 1] = planes_[arr][m]->ResizeTo(w, h, metadata_.LinearMipmap());
				}
			}
		}
		else
		{
			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				planes_[arr].resize(num_mipmaps_);

				for (uint32_t m = 0; m < num_mipmaps_; ++ m)
				{
					if ((arr != 0) || (m != 0))
					{
						planes_[arr][m] = MakeSharedPtr<ImagePlane>();
						planes_[arr][m]->Load(metadata_.PlaneFileName(arr, m), metadata_);
					}
				}
			}
		}

		if (format_ != metadata_.PreferedFormat())
		{
			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				for (uint32_t m = 0; m < num_mipmaps_; ++ m)
				{
					planes_[arr][m]->FormatConversion(metadata_.PreferedFormat());
				}
			}

			format_ = metadata_.PreferedFormat();
		}

		return true;
	}

	void TexConverter::Save(Texture::TextureType& output_type, uint32_t& output_width, uint32_t& output_height, uint32_t& output_depth,
		uint32_t& output_num_mipmaps, uint32_t& output_array_size,
		ElementFormat& output_format, std::vector<ElementInitData>& output_init_data, std::vector<uint8_t>& output_data_block)
	{
		output_type = Texture::TT_2D;
		output_width = width_;
		output_height = height_;
		output_depth = 1;
		output_num_mipmaps = num_mipmaps_;
		output_array_size = array_size_;
		output_format = format_;

		output_init_data.resize(array_size_ * num_mipmaps_);
		uint32_t data_size = 0;
		for (uint32_t arr = 0; arr < array_size_; ++ arr)
		{
			for (uint32_t m = 0; m < num_mipmaps_; ++ m)
			{
				auto const & plane = *planes_[arr][m];
				auto& out_data = output_init_data[arr * num_mipmaps_ + m];

				if (IsCompressedFormat(format_))
				{
					out_data.row_pitch = plane.CompressedRowPitch();
					out_data.slice_pitch = plane.CompressedSlicePitch();
				}
				else
				{
					out_data.row_pitch = plane.UncompressedRowPitch();
					out_data.slice_pitch = plane.UncompressedSlicePitch();
				}
				data_size += out_data.slice_pitch;
			}
		}

		output_data_block.resize(data_size);
		uint32_t start_index = 0;
		for (uint32_t arr = 0; arr < array_size_; ++ arr)
		{
			for (uint32_t m = 0; m < num_mipmaps_; ++ m)
			{
				auto const & plane = *planes_[arr][m];
				auto& out_data = output_init_data[arr * num_mipmaps_ + m];

				uint8_t* dst = &output_data_block[start_index];
				uint8_t const * src;
				if (IsCompressedFormat(format_))
				{
					src = plane.CompressedData().data();
				}
				else
				{
					src = plane.UncompressedData().data();
				}
				std::memcpy(dst, src, out_data.slice_pitch);
				out_data.data = dst;
				start_index += out_data.slice_pitch;
			}
		}
	}
}
