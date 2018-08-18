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
			TexturePtr in_tex = LoadSoftwareTexture(name_str);
			auto const type = in_tex->Type();
			auto const depth = in_tex->Depth(0);
			auto const num_mipmaps = in_tex->NumMipMaps();
			auto const array_size = in_tex->ArraySize();
			auto const format = in_tex->Format();

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
				compressed_tex_ = in_tex;

				ElementFormat uncompressed_format;
				if ((format == EF_BC6) || (format == EF_SIGNED_BC6))
				{
					uncompressed_format = EF_ABGR16F;
				}
				else if (IsSigned(format))
				{
					uncompressed_format = EF_SIGNED_ABGR8;
				}
				else if (IsSRGB(format))
				{
					uncompressed_format = EF_ARGB8_SRGB;
				}
				else
				{
					uncompressed_format = EF_ARGB8;
				}

				uncompressed_tex_ = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, in_tex->Width(0), in_tex->Height(0),
					in_tex->Depth(0), in_tex->NumMipMaps(), in_tex->ArraySize(), uncompressed_format, false);
				uncompressed_tex_->CreateHWResource({}, nullptr);
				compressed_tex_->CopyToTexture(*uncompressed_tex_);
			}
			else
			{
				uncompressed_tex_ = in_tex;

				compressed_tex_.reset();
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

			uint32_t const width = FreeImage_GetWidth(dib.get());
			uint32_t const height = FreeImage_GetHeight(dib.get());
			if ((width == 0) || (height == 0))
			{
				return false;
			}

			FreeImage_FlipVertical(dib.get());

			ElementFormat uncompressed_format = EF_ABGR8;
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
								uncompressed_format = EF_ARGB8;
							}
							else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
							{
								uncompressed_format = EF_ABGR8;
							}
						}
						else
						{
							uncompressed_format = EF_ARGB8;
						}
						dib.reset(FreeImage_ConvertTo32Bits(dib.get()));
						break;

					case 16:
						if ((r_mask == (0x1F << 10)) && (g_mask == (0x1F << 5)) && (b_mask == 0x1F))
						{
							uncompressed_format = EF_A1RGB5;
						}
						else if ((r_mask == (0x1F << 11)) && (g_mask == (0x3F << 5)) && (b_mask == 0x1F))
						{
							uncompressed_format = EF_R5G6B5;
						}
						break;

					case 32:
						if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
						{
							uncompressed_format = EF_ARGB8;
						}
						else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
						{
							uncompressed_format = EF_ABGR8;
						}
						break;

					default:
						KFL_UNREACHABLE("Unsupported bpp.");
					}
				}
				break;

			case FIT_UINT16:
				uncompressed_format = EF_R16UI;
				break;

			case FIT_INT16:
				uncompressed_format = EF_R16I;
				break;

			case FIT_UINT32:
				uncompressed_format = EF_R32UI;
				break;

			case FIT_INT32:
				uncompressed_format = EF_R32I;
				break;

			case FIT_FLOAT:
				uncompressed_format = EF_R32F;
				break;

			case FIT_COMPLEX:
				uncompressed_format = EF_GR32F;
				break;

			case FIT_RGB16:
				uncompressed_format = EF_ABGR16;
				dib.reset(FreeImage_ConvertToRGBA16(dib.get()));
				break;

			case FIT_RGBA16:
				uncompressed_format = EF_ABGR16;
				break;

			case FIT_RGBF:
				uncompressed_format = EF_ABGR32F;
				dib.reset(FreeImage_ConvertToRGBAF(dib.get()));
				break;

			case FIT_RGBAF:
				uncompressed_format = EF_ABGR32F;
				break;

			default:
				KFL_UNREACHABLE("Unsupported image type.");
			}

			uint8_t const * src = FreeImage_GetBits(dib.get());
			if (src == nullptr)
			{
				return false;
			}

			ElementInitData uncompressed_init_data;
			uncompressed_init_data.data = src;
			uncompressed_init_data.row_pitch = FreeImage_GetPitch(dib.get());
			uncompressed_init_data.slice_pitch = uncompressed_init_data.row_pitch * height;

			uncompressed_tex_ = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, width, height,
				1, 1, 1, uncompressed_format, false);
			uncompressed_tex_->CreateHWResource(uncompressed_init_data, nullptr);
		}

		if (metadata.ForceSRGB())
		{
			if (!IsSRGB(uncompressed_tex_->Format()))
			{
				uint32_t const width = uncompressed_tex_->Width(0);
				uint32_t const height = uncompressed_tex_->Height(0);

				TexturePtr srgb_uncompressed_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D,
					width, height, 1, 1, 1, MakeSRGB(uncompressed_tex_->Format()), false);
				{
					Texture::Mapper ori_mapper(*uncompressed_tex_, 0, 0, TMA_Read_Only, 0, 0, width, height);

					ElementInitData init_data;
					init_data.data = ori_mapper.Pointer<void>();
					init_data.row_pitch = ori_mapper.RowPitch();
					init_data.slice_pitch = ori_mapper.SlicePitch();

					srgb_uncompressed_tex->CreateHWResource(init_data, nullptr);
				}

				uncompressed_tex_ = srgb_uncompressed_tex;
			}

			if (compressed_tex_ && !IsSRGB(compressed_tex_->Format()))
			{
				uint32_t const width = compressed_tex_->Width(0);
				uint32_t const height = compressed_tex_->Height(0);

				TexturePtr srgb_compressed_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D,
					width, height, 1, 1, 1, MakeSRGB(compressed_tex_->Format()), false);
				{
					Texture::Mapper ori_mapper(*compressed_tex_, 0, 0, TMA_Read_Only, 0, 0, width, height);

					ElementInitData init_data;
					init_data.data = ori_mapper.Pointer<void>();
					init_data.row_pitch = ori_mapper.RowPitch();
					init_data.slice_pitch = ori_mapper.SlicePitch();

					srgb_compressed_tex->CreateHWResource(init_data, nullptr);
				}

				compressed_tex_ = srgb_compressed_tex;
			}
		}

		uint32_t const num_channels = NumComponents(uncompressed_tex_->Format());
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
			compressed_tex_.reset();

			uint32_t const width = uncompressed_tex_->Width(0);
			uint32_t const height = uncompressed_tex_->Height(0);
			ElementFormat const format = uncompressed_tex_->Format();

			Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Write, 0, 0,
				uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
			uint8_t* ptr = mapper.Pointer<uint8_t>();
			std::vector<Color> line_32f(width);
			for (uint32_t y = 0; y < height; ++ y)
			{
				ConvertToABGR32F(format, ptr, width, line_32f.data());

				Color original_clr;
				Color swizzled_clr(0, 0, 0, 0);
				for (uint32_t x = 0; x < width; ++ x)
				{
					original_clr = line_32f[x];
					for (uint32_t ch = 0; ch < num_channels; ++ ch)
					{
						swizzled_clr[ch] = original_clr[channel_mapping[ch]];
					}
					line_32f[x] = swizzled_clr;
				}

				ConvertFromABGR32F(format, line_32f.data(), width, ptr);

				ptr += mapper.RowPitch();
			}
		}

		return true;
	}

	void ImagePlane::FormatConversion(ElementFormat format)
	{
		TexturePtr new_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, uncompressed_tex_->Width(0), uncompressed_tex_->Height(0),
			1, 1, 1, format, false);
		new_tex->CreateHWResource({}, nullptr);
		uncompressed_tex_->CopyToTexture(*new_tex);

		if (IsCompressedFormat(format))
		{
			compressed_tex_ = new_tex;
		}
		else
		{
			uncompressed_tex_ = new_tex;
			compressed_tex_.reset();
		}
	}

	ImagePlane ImagePlane::ResizeTo(uint32_t width, uint32_t height, bool linear)
	{
		BOOST_ASSERT(uncompressed_tex_);

		auto const format = uncompressed_tex_->Format();

		ImagePlane target;
		target.uncompressed_tex_ = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, width, height, 1, 1, 1,
			format, false);

		ElementInitData target_init_data;
		target_init_data.row_pitch = width * NumFormatBytes(uncompressed_tex_->Format());
		target_init_data.slice_pitch = target_init_data.row_pitch * height;
		std::vector<uint8_t> target_data(target_init_data.slice_pitch);
		target_init_data.data = target_data.data();

		{
			Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Only, 0, 0,
				uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
			ResizeTexture(target_data.data(), target_init_data.row_pitch, target_init_data.slice_pitch,
				format, width, height, 1,
				mapper.Pointer<void>(), mapper.RowPitch(), mapper.SlicePitch(), format,
				uncompressed_tex_->Width(0), uncompressed_tex_->Height(0), 1,
				linear);
		}

		target.uncompressed_tex_->CreateHWResource(target_init_data, nullptr);

		return target;
	}
}
