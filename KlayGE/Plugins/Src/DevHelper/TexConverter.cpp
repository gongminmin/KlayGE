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
#include <KFL/CXX17/filesystem.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/TexCompression.hpp>

#include <cstring>

#include <FreeImage.h>

#include <KlayGE/DevHelper/TexConverter.hpp>
#include "ImagePlane.hpp"

using namespace KlayGE;

namespace
{
	class TexLoader
	{
	public:
		TexturePtr Load(std::string_view input_name, TexMetadata const & metadata);

		static void GetImageInfo(std::string_view input_name, TexMetadata const & metadata,
			Texture::TextureType& type,
			uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
			ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch);

		static bool IsSupported(std::string_view input_name);

	private:
		bool Load();
		TexturePtr StoreToTexture();

	private:
		std::string input_name_;

		TexMetadata metadata_;

		std::vector<std::vector<std::shared_ptr<ImagePlane>>> planes_;
		uint32_t width_;
		uint32_t height_;
		uint32_t array_size_;
		uint32_t num_mipmaps_;
		ElementFormat format_;
	};

	TexturePtr TexLoader::Load(std::string_view input_name, TexMetadata const & metadata)
	{
		TexturePtr ret;

		input_name_ = std::string(input_name);
		metadata_ = metadata;

		auto in_folder = FILESYSTEM_NS::path(ResLoader::Instance().Locate(input_name_)).parent_path().string();
		bool const in_path = ResLoader::Instance().IsInPath(in_folder);
		if (!in_path)
		{
			ResLoader::Instance().AddPath(in_folder);
		}

		if (this->Load())
		{
			ret = this->StoreToTexture();
		}

		if (!in_path)
		{
			ResLoader::Instance().DelPath(in_folder);
		}

		return ret;
	}

	void TexLoader::GetImageInfo(std::string_view input_name, TexMetadata const & metadata,
		Texture::TextureType& type,
		uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
		ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		ImagePlane image;
		image.Load(input_name, metadata);

		type = Texture::TT_2D;
		width = image.Width();
		height = image.Height();
		depth = 1;

		if (metadata.MipmapEnabled())
		{
			if (metadata.NumMipmaps() == 0)
			{
				num_mipmaps = 1;
				uint32_t w = width;
				uint32_t h = height;
				while ((w != 1) || (h != 1))
				{
					++ num_mipmaps;

					w = std::max<uint32_t>(1U, w / 2);
					h = std::max<uint32_t>(1U, h / 2);
				}
			}
			else
			{
				num_mipmaps = metadata.NumMipmaps();
			}
		}
		else
		{
			num_mipmaps = 1;
		}

		array_size = metadata.ArraySize();

		if (image.CompressedTex())
		{
			format = image.CompressedTex()->Format();
		}
		else
		{
			format = image.UncompressedTex()->Format();
		}
		BOOST_ASSERT(format != EF_Unknown);
		ElementFormat metadata_format = metadata.PreferedFormat();
		if (metadata_format == EF_Unknown)
		{
			metadata_format = format;
		}

		if ((num_mipmaps > 1) && metadata.AutoGenMipmap())
		{
			format = image.UncompressedTex()->Format();
		}

		if (format != metadata_format)
		{
			format = metadata_format;

			uint32_t const block_width = BlockWidth(format);
			uint32_t const block_height = BlockHeight(format);
			uint32_t const block_bytes = BlockBytes(format);

			row_pitch = (width + block_width - 1) / block_width * block_bytes;
			slice_pitch = (height + block_height - 1) / block_height * row_pitch;
		}
		else if (IsCompressedFormat(format))
		{
			Texture::Mapper mapper(*image.CompressedTex(), 0, 0, TMA_Read_Only, 0, 0,
				image.CompressedTex()->Width(0), image.CompressedTex()->Height(0));
			row_pitch = mapper.RowPitch();
			slice_pitch = mapper.SlicePitch();
		}
		else
		{
			Texture::Mapper mapper(*image.UncompressedTex(), 0, 0, TMA_Read_Only, 0, 0,
				image.UncompressedTex()->Width(0), image.UncompressedTex()->Height(0));
			row_pitch = mapper.RowPitch();
			slice_pitch = mapper.SlicePitch();
		}
	}

	bool TexLoader::IsSupported(std::string_view input_name)
	{
		std::string const input_name_str = ResLoader::Instance().Locate(input_name);
		if (input_name_str.empty())
		{
			LogError() << "Could NOT find " << input_name << '.' << std::endl;
			return false;
		}

		std::string const ext_name = FILESYSTEM_NS::path(input_name_str).extension().string();
		if (ext_name == ".dds")
		{
			return true;
		}
		else
		{
			FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(input_name_str.c_str(), 0);
			if (fif == FIF_UNKNOWN)
			{
				fif = FreeImage_GetFIFFromFilename(input_name_str.c_str());
			}
			return fif != FIF_UNKNOWN;
		}
	}

	bool TexLoader::Load()
	{
		array_size_ = metadata_.ArraySize();

		planes_.resize(array_size_);
		planes_[0].resize(1);
		planes_[0][0] = MakeSharedPtr<ImagePlane>();
		auto& first_image = *planes_[0][0];

		if (!first_image.Load(input_name_, metadata_))
		{
			LogError() << "Could NOT load " << input_name_ << '.' << std::endl;
			return false;
		}

		width_ = first_image.Width();
		height_ = first_image.Height();
		if (first_image.CompressedTex())
		{
			format_ = first_image.CompressedTex()->Format();
		}
		else
		{
			format_ = first_image.UncompressedTex()->Format();
		}
		BOOST_ASSERT(format_ != EF_Unknown);
		if (metadata_.PreferedFormat() == EF_Unknown)
		{
			metadata_.PreferedFormat(format_);
		}

		if (metadata_.MipmapEnabled())
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
			num_mipmaps_ = 1;
		}

		bool need_gen_mipmaps = false;
		if ((num_mipmaps_ > 1) && metadata_.AutoGenMipmap())
		{
			need_gen_mipmaps = true;

			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				planes_[arr].resize(num_mipmaps_);

				if (arr > 0)
				{
					std::string_view const plane_file_name = metadata_.PlaneFileName(arr, 0);
					planes_[arr][0] = MakeSharedPtr<ImagePlane>();
					if (!planes_[arr][0]->Load(plane_file_name, metadata_))
					{
						LogError() << "Could NOT load " << plane_file_name << '.' << std::endl;
						return false;
					}
				}

				for (uint32_t m = 1; m < num_mipmaps_; ++ m)
				{
					planes_[arr][m] = MakeSharedPtr<ImagePlane>();
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
						std::string_view const plane_file_name = metadata_.PlaneFileName(arr, m);
						planes_[arr][m] = MakeSharedPtr<ImagePlane>();
						if (plane_file_name.empty())
						{
							if (m == 0)
							{
								*planes_[arr][m] = planes_[0][0]->ResizeTo(width_, height_, false);
							}
							else
							{
								*planes_[arr][m] = planes_[arr][0]->ResizeTo(
									std::max(1U, width_ >> m), std::max(1U, height_ >> m), metadata_.LinearMipmap());
							}
						}
						else
						{
							if (!planes_[arr][m]->Load(plane_file_name, metadata_))
							{
								LogError() << "Could NOT load " << plane_file_name << '.' << std::endl;
								return false;
							}
						}
					}
				}
			}
		}

		if (metadata_.RgbToLum())
		{
			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				uint32_t const num = need_gen_mipmaps ? 1 : num_mipmaps_;
				for (uint32_t m = 0; m < num; ++ m)
				{
					planes_[arr][m]->RgbToLum();
				}
			}
		}

		if (((metadata_.Slot() == RenderMaterial::TS_Normal) || (metadata_.Slot() == RenderMaterial::TS_Occlusion)) &&
			(metadata_.BumpToNormal() || metadata_.BumpToOcclusion()))
		{
			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				uint32_t const num = need_gen_mipmaps ? 1 : num_mipmaps_;
				for (uint32_t m = 0; m < num; ++ m)
				{
					planes_[arr][m]->BumpToNormal(metadata_.BumpScale(), metadata_.BumpToOcclusion() ? metadata_.OcclusionAmplitude() : 0);

					if (metadata_.Slot() == RenderMaterial::TS_Occlusion)
					{
						planes_[arr][m]->AlphaToLum();
					}
				}
			}
		}

		if ((metadata_.Slot() == RenderMaterial::TS_Height) && metadata_.NormalToHeight())
		{
			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				uint32_t const num = need_gen_mipmaps ? 1 : num_mipmaps_;
				for (uint32_t m = 0; m < num; ++ m)
				{
					planes_[arr][m]->NormalToHeight(metadata_.HeightMinZ());
				}
			}
		}

		if (need_gen_mipmaps)
		{
			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				uint32_t w = width_;
				uint32_t h = height_;
				for (uint32_t m = 0; m < num_mipmaps_ - 1; ++ m)
				{
					w = std::max<uint32_t>(1U, w / 2);
					h = std::max<uint32_t>(1U, h / 2);

					*planes_[arr][m + 1] = planes_[arr][m]->ResizeTo(w, h, metadata_.LinearMipmap());
				}
			}

			format_ = planes_[0][0]->UncompressedTex()->Format();
		}
	
		bool need_normal_compression = false;
		if (metadata_.Slot() == RenderMaterial::TS_Normal)
		{
			switch (metadata_.PreferedFormat())
			{
			case EF_BC3:
			case EF_BC5:
			case EF_GR8:
				need_normal_compression = true;
				break;

			default:
				break;
			}
		}
		if (need_normal_compression)
		{
			for (uint32_t arr = 0; arr < array_size_; ++ arr)
			{
				for (uint32_t m = 0; m < num_mipmaps_; ++ m)
				{
					planes_[arr][m]->PrepareNormalCompression(metadata_.PreferedFormat());
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

	TexturePtr TexLoader::StoreToTexture()
	{
		Texture::TextureType output_type = Texture::TT_2D;
		uint32_t output_width = width_;
		uint32_t output_height = height_;
		uint32_t output_depth = 1;
		uint32_t output_num_mipmaps = num_mipmaps_;
		uint32_t output_array_size = array_size_;
		ElementFormat output_format = format_;

		std::vector<ElementInitData> output_init_data(array_size_ * num_mipmaps_);
		uint32_t data_size = 0;
		for (uint32_t arr = 0; arr < array_size_; ++ arr)
		{
			for (uint32_t m = 0; m < num_mipmaps_; ++ m)
			{
				auto const & plane = *planes_[arr][m];
				auto& out_data = output_init_data[arr * num_mipmaps_ + m];

				if (IsCompressedFormat(format_))
				{
					Texture::Mapper mapper(*plane.CompressedTex(), 0, 0, TMA_Read_Only, 0, 0,
						plane.CompressedTex()->Width(0), plane.CompressedTex()->Height(0));
					out_data.row_pitch = mapper.RowPitch();
					out_data.slice_pitch = mapper.SlicePitch();
				}
				else
				{
					Texture::Mapper mapper(*plane.UncompressedTex(), 0, 0, TMA_Read_Only, 0, 0,
						plane.UncompressedTex()->Width(0), plane.UncompressedTex()->Height(0));
					out_data.row_pitch = mapper.RowPitch();
					out_data.slice_pitch = mapper.SlicePitch();
				}
				data_size += out_data.slice_pitch;
			}
		}

		std::vector<uint8_t> output_data_block(data_size);
		uint32_t start_index = 0;
		for (uint32_t arr = 0; arr < array_size_; ++ arr)
		{
			for (uint32_t m = 0; m < num_mipmaps_; ++ m)
			{
				auto const & plane = *planes_[arr][m];
				auto& out_data = output_init_data[arr * num_mipmaps_ + m];

				uint8_t* dst = &output_data_block[start_index];
				if (IsCompressedFormat(format_))
				{
					Texture::Mapper mapper(*plane.CompressedTex(), 0, 0, TMA_Read_Only, 0, 0,
						plane.CompressedTex()->Width(0), plane.CompressedTex()->Height(0));
					std::memcpy(dst, mapper.Pointer<uint8_t>(), out_data.slice_pitch);
				}
				else
				{
					Texture::Mapper mapper(*plane.UncompressedTex(), 0, 0, TMA_Read_Only, 0, 0,
						plane.UncompressedTex()->Width(0), plane.UncompressedTex()->Height(0));
					std::memcpy(dst, mapper.Pointer<uint8_t>(), out_data.slice_pitch);
				}
				out_data.data = dst;
				start_index += out_data.slice_pitch;
			}
		}

		TexturePtr ret = MakeSharedPtr<SoftwareTexture>(output_type, output_width, output_height, output_depth,
			output_num_mipmaps, output_array_size, output_format, false);
		ret->CreateHWResource(output_init_data, nullptr);
		return ret;
	}
}

namespace KlayGE
{
	TexturePtr TexConverter::Load(std::string_view input_name, TexMetadata const & metadata)
	{
		TexLoader tl;
		return tl.Load(input_name, metadata);
	}

	void TexConverter::GetImageInfo(std::string_view input_name, TexMetadata const & metadata,
		Texture::TextureType& type,
		uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
		ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		return TexLoader::GetImageInfo(input_name, metadata,
			type, width, height, depth, num_mipmaps, array_size, format, row_pitch, slice_pitch);
	}

	bool TexConverter::IsSupported(std::string_view input_name)
	{
		return TexLoader::IsSupported(input_name);
	}
}
