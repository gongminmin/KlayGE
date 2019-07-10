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
#include <KFL/CXX2a/span.hpp>
#include <KFL/CpuInfo.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/TexCompression.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/TexCompressionETC.hpp>
#include <KlayGE/Texture.hpp>

#include <FreeImage.h>

#include "ImagePlane.hpp"
#include <KlayGE/DevHelper/TexMetadata.hpp>

namespace
{
	using namespace KlayGE;

	void CreateDDM(std::vector<float2>& ddm, std::vector<float3> const & normal_map, float min_z)
	{
		ddm.resize(normal_map.size());
		for (size_t i = 0; i < normal_map.size(); ++ i)
		{
			float3 n = normal_map[i];
			n.z() = std::max(n.z(), min_z);
			ddm[i].x() = n.x() / n.z();
			ddm[i].y() = n.y() / n.z();
		}
	}

	void AccumulateDDM(std::vector<float>& height_map, std::vector<float2> const & ddm, uint32_t width, uint32_t height,
		int directions, int rings)
	{
		float const step = 2 * PI / directions;
		std::vector<float2> dxdy(directions);
		for (int i = 0; i < directions; ++ i)
		{
			MathLib::sincos(-i * step, dxdy[i].y(), dxdy[i].x());
		}

		std::vector<float2> tmp_hm[2];
		tmp_hm[0].resize(ddm.size(), float2(0, 0));
		tmp_hm[1].resize(ddm.size(), float2(0, 0));
		int active = 0;
		for (int i = 1; i < rings; ++ i)
		{
			for (size_t j = 0; j < ddm.size(); ++ j)
			{
				int y = static_cast<int>(j / width);
				int x = static_cast<int>(j - y * width);

				for (int k = 0; k < directions; ++ k)
				{
					float2 delta = dxdy[k] * static_cast<float>(i);
					float sample_x = x + delta.x();
					float sample_y = y + delta.y();
					int sample_x0 = static_cast<int>(floor(sample_x));
					int sample_y0 = static_cast<int>(floor(sample_y));
					int sample_x1 = sample_x0 + 1;
					int sample_y1 = sample_y0 + 1;
					float weight_x = sample_x - sample_x0;
					float weight_y = sample_y - sample_y0;

					sample_x0 %= width;
					sample_y0 %= height;
					sample_x1 %= width;
					sample_y1 %= height;

					float2 hl0 = MathLib::lerp(tmp_hm[active][sample_y0 * width + sample_x0], tmp_hm[active][sample_y0 * width + sample_x1], weight_x);
					float2 hl1 = MathLib::lerp(tmp_hm[active][sample_y1 * width + sample_x0], tmp_hm[active][sample_y1 * width + sample_x1], weight_x);
					float2 h = MathLib::lerp(hl0, hl1, weight_y);
					float2 ddl0 = MathLib::lerp(ddm[sample_y0 * width + sample_x0], ddm[sample_y0 * width + sample_x1], weight_x);
					float2 ddl1 = MathLib::lerp(ddm[sample_y1 * width + sample_x0], ddm[sample_y1 * width + sample_x1], weight_x);
					float2 dd = MathLib::lerp(ddl0, ddl1, weight_y);

					tmp_hm[!active][j] += h + dd * delta;
				}
			}

			active = !active;
		}

		float const scale = 0.5f / (directions * rings);

		height_map.resize(ddm.size());
		for (size_t i = 0; i < ddm.size(); ++ i)
		{
			float2 const & h = tmp_hm[active][i];
			height_map[i] = (h.x() + h.y()) * scale;
		}
	}
}

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
				int flag = 0;
				if (fif == FIF_JPEG)
				{
					flag = JPEG_ACCURATE;
				}

				dib.reset(FreeImage_Load(fif, name_str.c_str(), flag));
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
			uncompressed_tex_->CreateHWResource(MakeSpan<1>(uncompressed_init_data), nullptr);
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

					srgb_uncompressed_tex->CreateHWResource(MakeSpan<1>(init_data), nullptr);
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

					srgb_compressed_tex->CreateHWResource(MakeSpan<1>(init_data), nullptr);
				}

				compressed_tex_ = srgb_compressed_tex;
			}
		}

		uint32_t const num_channels = NumComponents(uncompressed_tex_->Format());
		int32_t channel_mapping[4];
		for (uint32_t ch = 0; ch < num_channels; ++ ch)
		{
			channel_mapping[ch] = metadata.ChannelMapping(ch);
		}

		bool need_swizzle = false;
		for (uint32_t ch = 0; ch < num_channels; ++ ch)
		{
			if (channel_mapping[ch] != static_cast<int32_t>(ch))
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
						if (channel_mapping[ch] >= 0)
						{
							swizzled_clr[ch] = original_clr[channel_mapping[ch]];
						}
						else
						{
							swizzled_clr[ch] = 0;
						}
					}
					line_32f[x] = swizzled_clr;
				}

				ConvertFromABGR32F(format, line_32f.data(), width, ptr);

				ptr += mapper.RowPitch();
			}
		}

		auto const preferred_fmt = metadata.PreferedFormat();
		if ((preferred_fmt != EF_Unknown) && IsCompressedFormat(preferred_fmt))
		{
			uint32_t const block_width = BlockWidth(preferred_fmt);
			uint32_t const block_height = BlockHeight(preferred_fmt);

			uint32_t const width = uncompressed_tex_->Width(0);
			uint32_t const height = uncompressed_tex_->Height(0);

			uint32_t const aligned_width = (width + block_width - 1) & ~(block_width - 1);
			uint32_t const aligned_height = (height + block_height - 1) & ~(block_height - 1);

			if ((width != aligned_width) || (height != aligned_height))
			{
				*this = this->ResizeTo(aligned_width, aligned_height, true);
			}
		}

		return true;
	}

	void ImagePlane::PrepareNormalCompression(ElementFormat normal_compression_format)
	{
		compressed_tex_.reset();

		uint32_t const width = uncompressed_tex_->Width(0);
		uint32_t const height = uncompressed_tex_->Height(0);
		ElementFormat const format = uncompressed_tex_->Format();
		uint32_t const elem_size = NumFormatBytes(format);

		Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Write, 0, 0,
			uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
		uint8_t* ptr = mapper.Pointer<uint8_t>();

		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				Color color_32f;
				ConvertToABGR32F(format, ptr + x * elem_size, 1, &color_32f);

				switch (normal_compression_format)
				{
				case EF_BC3:
					color_32f.a() = color_32f.r();
					color_32f.r() = 0;
					color_32f.b() = 0;
					break;

				case EF_BC5:
				case EF_GR8:
					color_32f.b() = 0;
					color_32f.a() = 0;
					break;

				default:
					KFL_UNREACHABLE("Invalid normal compression format.");
				}

				ConvertFromABGR32F(format, &color_32f, 1, ptr + x * elem_size);
			}

			ptr += mapper.RowPitch();
		}
	}

	void ImagePlane::RgbToLum()
	{
		compressed_tex_.reset();

		uint32_t const width = uncompressed_tex_->Width(0);
		uint32_t const height = uncompressed_tex_->Height(0);
		ElementFormat const format = uncompressed_tex_->Format();
		uint32_t const elem_size = NumFormatBytes(format);

		Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Write, 0, 0,
			uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
		uint8_t* ptr = mapper.Pointer<uint8_t>();

		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				Color color_32f;
				ConvertToABGR32F(format, ptr + x * elem_size, 1, &color_32f);

				float const lum = this->RgbToLum(color_32f);

				Color const color_f32(lum, lum, lum, 1);
				ConvertFromABGR32F(format, &color_f32, 1, ptr + x * elem_size);
			}

			ptr += mapper.RowPitch();
		}
	}

	void ImagePlane::AlphaToLum()
	{
		compressed_tex_.reset();

		uint32_t const width = uncompressed_tex_->Width(0);
		uint32_t const height = uncompressed_tex_->Height(0);
		ElementFormat const format = uncompressed_tex_->Format();
		uint32_t const elem_size = NumFormatBytes(format);

		Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Write, 0, 0, uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
		uint8_t* ptr = mapper.Pointer<uint8_t>();

		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				Color color_32f;
				ConvertToABGR32F(format, ptr + x * elem_size, 1, &color_32f);

				float const lum = color_32f.a();

				Color const color_f32(lum, lum, lum, 1);
				ConvertFromABGR32F(format, &color_f32, 1, ptr + x * elem_size);
			}

			ptr += mapper.RowPitch();
		}
	}

	void ImagePlane::BumpToNormal(float scale, float amplitude)
	{
		compressed_tex_.reset();

		uint32_t const width = uncompressed_tex_->Width(0);
		uint32_t const height = uncompressed_tex_->Height(0);
		ElementFormat const format = uncompressed_tex_->Format();
		uint32_t const elem_size = NumFormatBytes(format);
		uint32_t const num_comp = NumComponents(format);

		Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Write, 0, 0,
			uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
		uint8_t* ptr = mapper.Pointer<uint8_t>();

		std::vector<float> height_map(width * height);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				Color color_32f;
				ConvertToABGR32F(format, ptr + x * elem_size, 1, &color_32f);

				float h;
				if (num_comp == 1)
				{
					h = color_32f.r();
				}
				else
				{
					h = this->RgbToLum(color_32f);
				}
				height_map[y * width + x] = h;
			}

			ptr += mapper.RowPitch();
		}

		ptr = mapper.Pointer<uint8_t>();
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				uint32_t const x0 = x;
				uint32_t const x1 = (x0 + 1) % width;
				float const dx = height_map[y * width + x1] - height_map[y * width + x0];

				uint32_t const y0 = y;
				uint32_t const y1 = (y0 + 1) % height;
				float const dy = height_map[y1 * width + x] - height_map[y0 * width + x];

				float3 normal = MathLib::normalize(float3(-dx, -dy, scale));
				normal = normal * 0.5f + float3(0.5f, 0.5f, 0.5f);

				float occlusion = 1;
				if (amplitude > 0)
				{
					float delta = 0;
					float const c = height_map[y * width + x];
					for (int oy = -1; oy < 2; ++oy)
					{
						int const sy = (y + oy) % height;
						for (int ox = -1; ox < 2; ++ox)
						{
							int const sx = (x + ox) % width;
							if ((ox != 0) && (oy != 0))
							{
								float const t = height_map[sy * width + sx] - c;
								if (t > 0)
								{
									delta += t;
								}
							}
						}
					}

					delta *= amplitude / 8;
					if (delta > 0)
					{
						float const r = MathLib::sqrt(1 + delta * delta);
						occlusion = (r - delta) / r;
					}
				}

				Color const color_f32(normal.x(), normal.y(), normal.z(), occlusion);
				ConvertFromABGR32F(format, &color_f32, 1, ptr + x * elem_size);
			}

			ptr += mapper.RowPitch();
		}
	}

	void ImagePlane::NormalToHeight(float min_z)
	{
		ElementFormat const com_format = compressed_tex_ ? compressed_tex_->Format() : EF_Unknown;
		compressed_tex_.reset();

		uint32_t const width = uncompressed_tex_->Width(0);
		uint32_t const height = uncompressed_tex_->Height(0);
		ElementFormat const format = uncompressed_tex_->Format();
		uint32_t const elem_size = NumFormatBytes(format);

		Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Write, 0, 0,
			uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
		uint8_t* ptr = mapper.Pointer<uint8_t>();

		std::vector<float3> normal_map(width * height);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				Color color_32f;
				ConvertToABGR32F(format, ptr + x * elem_size, 1, &color_32f);

				float3 normal;
				if ((com_format == EF_BC5) || (com_format == EF_BC3) || (format == EF_GR8))
				{
					if (com_format == EF_BC3)
					{
						normal.x() = color_32f.a() * 2 - 1;
					}
					else
					{
						normal.x() = color_32f.r() * 2 - 1;
					}
					normal.y() = color_32f.g() * 2 - 1;
					normal.z() = sqrt(std::max(0.0f, 1 - normal.x() * normal.x() - normal.y() * normal.y()));
				}
				else
				{
					normal = MathLib::normalize(float3(color_32f.r(), color_32f.g(), color_32f.b()) * 2 - 1);
				}

				normal_map[y * width + x] = normal;
			}

			ptr += mapper.RowPitch();
		}

		std::vector<float2> ddm;
		CreateDDM(ddm, normal_map, min_z);

		std::vector<float> height_map(width * height);
		AccumulateDDM(height_map, ddm, width, height, 4, 9);

		float min_height = +1e10f;
		float max_height = -1e10f;
		for (size_t i = 0; i < height_map.size(); ++ i)
		{
			min_height = std::min(min_height, height_map[i]);
			max_height = std::max(max_height, height_map[i]);
		}
		if (max_height - min_height > 1e-6f)
		{
			for (size_t i = 0; i < height_map.size(); ++ i)
			{
				height_map[i] = (height_map[i] - min_height) / (max_height - min_height);
			}
		}

		ptr = mapper.Pointer<uint8_t>();
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float const h = height_map[y * width + x] * 0.5f + 0.5f;

				Color const color_f32(h, h, h, 1);
				ConvertFromABGR32F(format, &color_f32, 1, ptr + x * elem_size);
			}

			ptr += mapper.RowPitch();
		}
	}

	void ImagePlane::FormatConversion(ElementFormat format)
	{
		uint32_t const tex_width = uncompressed_tex_->Width(0);
		uint32_t const tex_height = uncompressed_tex_->Height(0);

		uint32_t const block_width = BlockWidth(format);
		uint32_t const block_height = BlockHeight(format);
		uint32_t const block_bytes = BlockBytes(format);

		uint32_t const row_pitch = (tex_width + block_width - 1) / block_width * block_bytes;
		uint32_t const slice_pitch = (tex_height + block_height - 1) / block_height * row_pitch;

		std::vector<uint8_t> new_tex_data(slice_pitch);

		CPUInfo cpu;
		uint32_t const num_threads = cpu.NumHWThreads();
		thread_pool tp(1, num_threads);
		std::vector<joiner<void>> joiners(num_threads);

		uint32_t const tex_region_height = ((tex_height + num_threads - 1) / num_threads + block_height - 1) & ~(block_height - 1);
		std::vector<TexturePtr> new_tex_regions(num_threads);
		for (uint32_t i = 0; i < num_threads; ++ i)
		{
			joiners[i] = tp(
				[block_height, tex_width, tex_height, tex_region_height, i, format, row_pitch,
					&new_tex_data, &new_tex_regions, this]
				{
					uint32_t const this_tex_region_height = MathLib::clamp(static_cast<int>(tex_height - i * tex_region_height),
						0, static_cast<int>(tex_region_height));
					if (this_tex_region_height > 0)
					{
						new_tex_regions[i] = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, tex_width, this_tex_region_height,
							1, 1, 1, format, true);

						ElementInitData init_data;
						init_data.data = new_tex_data.data() + i * tex_region_height / block_height * row_pitch;
						init_data.row_pitch = row_pitch;
						init_data.slice_pitch = (this_tex_region_height + block_height - 1) / block_height * row_pitch;

						new_tex_regions[i]->CreateHWResource(MakeSpan<1>(init_data), nullptr);

						uncompressed_tex_->CopyToSubTexture2D(*new_tex_regions[i], 0, 0, 0, 0, tex_width, this_tex_region_height,
							0, 0, 0, i * tex_region_height, tex_width, this_tex_region_height);
					}
				});
		}

		TexturePtr new_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, uncompressed_tex_->Width(0), uncompressed_tex_->Height(0),
			1, 1, 1, format, false);
		ElementInitData init_data;
		init_data.data = new_tex_data.data();
		init_data.row_pitch = row_pitch;
		init_data.slice_pitch = slice_pitch;

		for (uint32_t i = 0; i < num_threads; ++ i)
		{
			joiners[i]();
		}

		new_tex->CreateHWResource(MakeSpan<1>(init_data), nullptr);

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

		compressed_tex_.reset();

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

		target.uncompressed_tex_->CreateHWResource(MakeSpan<1>(target_init_data), nullptr);

		return target;
	}

	float ImagePlane::RgbToLum(Color const & clr)
	{
		float3 constexpr RGB_TO_LUM(0.2126f, 0.7152f, 0.0722f);
		return MathLib::dot(float3(clr.r(), clr.g(), clr.b()), RGB_TO_LUM);
	}
}
