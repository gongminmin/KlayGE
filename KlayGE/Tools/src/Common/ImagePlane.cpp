/**
 * @file ImagePlane.cpp
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
#include <KFL/CXX17/filesystem.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/TexCompression.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/TexCompressionETC.hpp>
#include <KlayGE/Texture.hpp>

#include <FreeImage.h>

#include "ImagePlane.hpp"
#include <KlayGE/TexMetadata.hpp>

namespace KlayGE
{
	bool ImagePlane::Load(std::string_view name, TexMetadata const & metadata)
	{
		std::string const name_str = ResLoader::Instance().Locate(name);
		if (name_str.empty())
		{
			return false;
		}

		std::string const ext_name = std::filesystem::path(name_str).extension().string();
		if (ext_name == ".dds")
		{
			Texture::TextureType type;
			uint32_t depth;
			uint32_t num_mipmaps;
			uint32_t array_size;
			ElementFormat format;
			std::vector<ElementInitData> init_data;
			std::vector<uint8_t> data_block;

			LoadTexture(name_str, type, width_, height_, depth, num_mipmaps, array_size, format, init_data, data_block);
			if (type != Texture::TT_2D)
			{
				LogWarn() << "Only 2D texture are supported." << std::endl;
			}
			if (depth != 1)
			{
				LogWarn() << "Only first slice in the 3D texture is used." << std::endl;
			}
			if (num_mipmaps != 1)
			{
				LogWarn() << "Only first mip level in the texture is used." << std::endl;
			}
			if (array_size != 1)
			{
				LogWarn() << "Only first slice in the texture array is used." << std::endl;
			}

			if (IsCompressedFormat(format))
			{
				compressed_format_ = format;
				compressed_data_ = std::move(data_block);
				compressed_row_pitch_ = init_data[0].row_pitch;
				compressed_slice_pitch_ = init_data[0].slice_pitch;

				if ((format == EF_BC6) || (format == EF_SIGNED_BC6))
				{
					uncompressed_format_ = EF_ABGR16F;
				}
				else if (IsSigned(format))
				{
					uncompressed_format_ = EF_SIGNED_ABGR8;
				}
				else if (IsSRGB(format))
				{
					uncompressed_format_ = EF_ARGB8_SRGB;
				}
				else
				{
					uncompressed_format_ = EF_ARGB8;
				}

				uncompressed_row_pitch_ = width_ * NumFormatBytes(uncompressed_format_);
				uncompressed_slice_pitch_ = uncompressed_row_pitch_ * height_;
				uncompressed_data_.resize(uncompressed_slice_pitch_);

				ResizeTexture(uncompressed_data_.data(), uncompressed_row_pitch_, uncompressed_slice_pitch_,
					uncompressed_format_, width_, height_, 1,
					compressed_data_.data(), compressed_row_pitch_, compressed_slice_pitch_,
					compressed_format_, width_, height_, 1, false);
			}
			else
			{
				uncompressed_data_ = std::move(data_block);
				uncompressed_row_pitch_ = init_data[0].row_pitch;
				uncompressed_slice_pitch_ = init_data[0].slice_pitch;
				uncompressed_format_ = format;

				compressed_format_ = EF_Unknown;
			}
		}
		else
		{
			FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(name_str.c_str(), 0);
			if (fif == FIF_UNKNOWN)
			{
				fif = FreeImage_GetFIFFromFilename(name_str.c_str());
			}
			if (fif == FIF_UNKNOWN)
			{
				return false;
			}

			auto fi_bitmap_deleter = [](FIBITMAP* dib)
			{
				FreeImage_Unload(dib);
			};

			std::unique_ptr<FIBITMAP, decltype(fi_bitmap_deleter)> dib(nullptr, fi_bitmap_deleter);
			if (FreeImage_FIFSupportsReading(fif))
			{
				dib.reset(FreeImage_Load(fif, name_str.c_str()));
			}
			if (!dib)
			{
				return false;
			}

			width_ = FreeImage_GetWidth(dib.get());
			height_ = FreeImage_GetHeight(dib.get());
			if ((width_ == 0) || (height_ == 0))
			{
				return false;
			}

			FreeImage_FlipVertical(dib.get());

			uncompressed_format_ = EF_ABGR8;
			FREE_IMAGE_TYPE const image_type = FreeImage_GetImageType(dib.get());
			switch (image_type)
			{
			case FIT_BITMAP:
				{
					uint32_t const bpp = FreeImage_GetBPP(dib.get());
					uint32_t const r_mask = FreeImage_GetRedMask(dib.get());
					uint32_t const g_mask = FreeImage_GetGreenMask(dib.get());
					uint32_t const b_mask = FreeImage_GetBlueMask(dib.get());
					switch (bpp)
					{
					case 1:
					case 4:
					case 8:
					case 24:
						if (bpp == 24)
						{
							if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
							{
								uncompressed_format_ = EF_ARGB8;
							}
							else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
							{
								uncompressed_format_ = EF_ABGR8;
							}
						}
						else
						{
							uncompressed_format_ = EF_ARGB8;
						}
						dib.reset(FreeImage_ConvertTo32Bits(dib.get()));
						break;

					case 16:
						if ((r_mask == (0x1F << 10)) && (g_mask == (0x1F << 5)) && (b_mask == 0x1F))
						{
							uncompressed_format_ = EF_A1RGB5;
						}
						else if ((r_mask == (0x1F << 11)) && (g_mask == (0x3F << 5)) && (b_mask == 0x1F))
						{
							uncompressed_format_ = EF_R5G6B5;
						}
						break;

					case 32:
						if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
						{
							uncompressed_format_ = EF_ARGB8;
						}
						else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
						{
							uncompressed_format_ = EF_ABGR8;
						}
						break;

					default:
						KFL_UNREACHABLE("Unsupported bpp.");
					}
				}
				break;

			case FIT_UINT16:
				uncompressed_format_ = EF_R16UI;
				break;

			case FIT_INT16:
				uncompressed_format_ = EF_R16I;
				break;

			case FIT_UINT32:
				uncompressed_format_ = EF_R32UI;
				break;

			case FIT_INT32:
				uncompressed_format_ = EF_R32I;
				break;

			case FIT_FLOAT:
				uncompressed_format_ = EF_R32F;
				break;

			case FIT_COMPLEX:
				uncompressed_format_ = EF_GR32F;
				break;

			case FIT_RGB16:
				uncompressed_format_ = EF_ABGR16;
				dib.reset(FreeImage_ConvertToRGBA16(dib.get()));
				break;

			case FIT_RGBA16:
				uncompressed_format_ = EF_ABGR16;
				break;

			case FIT_RGBF:
				uncompressed_format_ = EF_ABGR32F;
				dib.reset(FreeImage_ConvertToRGBAF(dib.get()));
				break;

			case FIT_RGBAF:
				uncompressed_format_ = EF_ABGR32F;
				break;

			default:
				KFL_UNREACHABLE("Unsupported image type.");
			}

			uint8_t const * src = FreeImage_GetBits(dib.get());
			if (src == nullptr)
			{
				return false;
			}

			uncompressed_row_pitch_ = FreeImage_GetPitch(dib.get());
			uncompressed_slice_pitch_ = uncompressed_row_pitch_ * height_;
			uncompressed_data_.assign(src, src + uncompressed_slice_pitch_);
		}

		if (metadata.ForceSRGB())
		{
			uncompressed_format_ = MakeSRGB(uncompressed_format_);
			compressed_format_ = MakeSRGB(compressed_format_);
		}

		uint32_t const num_channels = NumComponents(uncompressed_format_);
		uint32_t channel_mapping[4];
		for (uint32_t ch = 0; ch < num_channels; ++ ch)
		{
			channel_mapping[ch] = metadata.ChannelMapping(ch);
		}

		bool need_swizzle = false;
		for (uint32_t ch = 0; ch < num_channels; ++ ch)
		{
			if (channel_mapping[ch] != ch)
			{
				need_swizzle = true;
				break;
			}
		}
		if (need_swizzle)
		{
			compressed_data_.clear();
			compressed_row_pitch_ = 0;
			compressed_slice_pitch_ = 0;
			compressed_format_ = EF_Unknown;

			uint8_t* ptr = uncompressed_data_.data();
			std::vector<Color> line_32f(width_);
			for (uint32_t y = 0; y < height_; ++ y)
			{
				ConvertToABGR32F(uncompressed_format_, ptr, width_, line_32f.data());

				Color original_clr;
				Color swizzled_clr(0, 0, 0, 0);
				for (uint32_t x = 0; x < width_; ++ x)
				{
					original_clr = line_32f[x];
					for (uint32_t ch = 0; ch < num_channels; ++ ch)
					{
						swizzled_clr[ch] = original_clr[channel_mapping[ch]];
					}
					line_32f[x] = swizzled_clr;
				}

				ConvertFromABGR32F(uncompressed_format_, line_32f.data(), width_, ptr);

				ptr += uncompressed_row_pitch_;
			}
		}

		return true;
	}

	void ImagePlane::FormatConversion(ElementFormat format)
	{
		uint32_t new_row_pitch;
		uint32_t new_slice_pitch;
		if (IsCompressedFormat(format))
		{
			std::unique_ptr<TexCompression> out_codec;
			switch (format)
			{
			case EF_BC1:
			case EF_BC1_SRGB:
			case EF_SIGNED_BC1:
				out_codec = MakeUniquePtr<TexCompressionBC1>();
				break;

			case EF_BC2:
			case EF_BC2_SRGB:
			case EF_SIGNED_BC2:
				out_codec = MakeUniquePtr<TexCompressionBC2>();
				break;

			case EF_BC3:
			case EF_BC3_SRGB:
			case EF_SIGNED_BC3:
				out_codec = MakeUniquePtr<TexCompressionBC3>();
				break;

			case EF_BC4:
			case EF_BC4_SRGB:
			case EF_SIGNED_BC4:
				out_codec = MakeUniquePtr<TexCompressionBC4>();
				break;

			case EF_BC5:
			case EF_BC5_SRGB:
			case EF_SIGNED_BC5:
				out_codec = MakeUniquePtr<TexCompressionBC5>();
				break;

			case EF_BC6:
				out_codec = MakeUniquePtr<TexCompressionBC6U>();
				break;

			case EF_SIGNED_BC6:
				out_codec = MakeUniquePtr<TexCompressionBC6S>();
				break;

			case EF_BC7:
			case EF_BC7_SRGB:
				out_codec = MakeUniquePtr<TexCompressionBC7>();
				break;

			case EF_ETC1:
				out_codec = MakeUniquePtr<TexCompressionETC1>();
				break;

			case EF_ETC2_BGR8:
			case EF_ETC2_BGR8_SRGB:
				out_codec = MakeUniquePtr<TexCompressionETC2RGB8>();
				break;

			case EF_ETC2_A1BGR8:
			case EF_ETC2_A1BGR8_SRGB:
				out_codec = MakeUniquePtr<TexCompressionETC2RGB8A1>();
				break;

			case EF_ETC2_ABGR8:
			case EF_ETC2_ABGR8_SRGB:
				// TODO
				KFL_UNREACHABLE("Not implemented");
				break;

			case EF_ETC2_R11:
			case EF_SIGNED_ETC2_R11:
				// TODO
				KFL_UNREACHABLE("Not implemented");
				break;

			case EF_ETC2_GR11:
			case EF_SIGNED_ETC2_GR11:
				// TODO
				KFL_UNREACHABLE("Not implemented");
				break;

			default:
				KFL_UNREACHABLE("Invalid compression format");
			}

			uint32_t const block_width = out_codec->BlockWidth();
			uint32_t const block_height = out_codec->BlockHeight();
			uint32_t const block_bytes = out_codec->BlockBytes();

			new_row_pitch = ((width_ + block_width - 1) / block_width) * block_bytes;
			new_slice_pitch = new_row_pitch * ((height_ + block_height - 1) / block_height);
		}
		else
		{
			new_row_pitch = width_ * NumFormatBytes(format);
			new_slice_pitch = new_row_pitch * height_;
		}

		std::vector<uint8_t> new_data(new_slice_pitch);
		ResizeTexture(new_data.data(), new_row_pitch, new_slice_pitch, format,
			width_, height_, 1,
			uncompressed_data_.data(), uncompressed_row_pitch_, uncompressed_slice_pitch_, uncompressed_format_,
			width_, height_, 1,
			false);

		if (IsCompressedFormat(format))
		{
			compressed_data_ = std::move(new_data);
			compressed_row_pitch_ = new_row_pitch;
			compressed_slice_pitch_ = new_slice_pitch;
			compressed_format_ = format;
		}
		else
		{
			uncompressed_data_ = std::move(new_data);
			uncompressed_row_pitch_ = new_row_pitch;
			uncompressed_slice_pitch_ = new_slice_pitch;
			uncompressed_format_ = format;
		}
	}

	ImagePlane ImagePlane::ResizeTo(uint32_t width, uint32_t height, bool linear)
	{
		ImagePlane target;
		target.width_ = width;
		target.height_ = height;
		target.uncompressed_row_pitch_ = target.width_ * NumFormatBytes(uncompressed_format_);
		target.uncompressed_slice_pitch_ = target.uncompressed_row_pitch_ * target.height_;
		target.uncompressed_data_.resize(target.uncompressed_slice_pitch_);
		target.uncompressed_format_ = uncompressed_format_;

		ResizeTexture(target.uncompressed_data_.data(), target.uncompressed_row_pitch_, target.uncompressed_slice_pitch_,
			uncompressed_format_, width, height, 1,
			uncompressed_data_.data(), uncompressed_row_pitch_, uncompressed_slice_pitch_, uncompressed_format_,
			width_, height_, 1,
			linear);

		return target;
	}
}
