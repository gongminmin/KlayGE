#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/TexCompressionETC.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/Half.hpp>

#include <vector>
#include <string>
#include <iostream>

#include "KlayGETests.hpp"

using namespace std;
using namespace KlayGE;

void TestEncodeDecodeTex(std::string const & input_name, std::string const & tc_name,
		ElementFormat bc_fmt, float threshold)
{
	std::vector<uint8_t> input_argb;
	std::vector<uint8_t> bc_blocks;
	uint32_t width, height;

	std::unique_ptr<TexCompression> codec;
	switch (bc_fmt)
	{
	case EF_BC1:
		codec = MakeUniquePtr<TexCompressionBC1>();
		break;

	case EF_BC2:
		codec = MakeUniquePtr<TexCompressionBC2>();
		break;

	case EF_BC3:
		codec = MakeUniquePtr<TexCompressionBC3>();
		break;

	case EF_BC6:
		codec = MakeUniquePtr<TexCompressionBC6U>();
		break;

	case EF_SIGNED_BC6:
		codec = MakeUniquePtr<TexCompressionBC6S>();
		break;

	case EF_BC7:
		codec = MakeUniquePtr<TexCompressionBC7>();
		break;

	case EF_ETC1:
		codec = MakeUniquePtr<TexCompressionETC1>();
		break;

	default:
		KFL_UNREACHABLE("Unsupported compression format");
	}

	ElementFormat const decoded_fmt = codec->DecodedFormat();
	uint32_t const pixel_size = NumFormatBytes(decoded_fmt);

	{
		Texture::TextureType type;
		uint32_t depth, num_mipmaps, array_size;
		ElementFormat format;
		std::vector<ElementInitData> init_data;
		std::vector<uint8_t> data_block;
		LoadTexture(input_name, type, width, height, depth, num_mipmaps, array_size,
			format, init_data, data_block);

		BOOST_ASSERT(pixel_size == NumFormatBytes(format));

		input_argb.resize(width * height * pixel_size);
		array<uint8_t, 16> pixel;

		uint8_t const * src = static_cast<uint8_t const *>(init_data[0].data);
		uint32_t const pitch = init_data[0].row_pitch;
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				memcpy(&pixel[0], &src[x * pixel_size], pixel_size);
				if (EF_BC1 == bc_fmt)
				{
					if (pixel[3] < 128)
					{
						pixel[0] = 0;
						pixel[1] = 0;
						pixel[2] = 0;
						pixel[3] = 0;
					}
					else
					{
						pixel[3] = 255;
					}
				}

				memcpy(&input_argb[(y * width + x) * pixel_size], &pixel[0], pixel_size);
			}

			src += pitch;
		}
	}

	uint32_t const block_width = codec->BlockWidth();
	uint32_t const block_height = codec->BlockWidth();
	uint32_t const block_bytes = codec->BlockBytes();
	bc_blocks.resize((width + block_width - 1) / block_width * (height + block_height - 1) / block_height * block_bytes);

	if (tc_name.empty())
	{
		for (uint32_t y_base = 0; y_base < height; y_base += block_height)
		{
			for (uint32_t x_base = 0; x_base < width; x_base += block_width)
			{
				std::vector<uint8_t> uncompressed(block_width * block_height * pixel_size);
				for (uint32_t y = 0; y < block_height; ++ y)
				{
					for (uint32_t x = 0; x < block_width; ++ x)
					{
						if ((x_base + x < width) && (y_base + y < height))
						{
							memcpy(&uncompressed[(y * block_width + x) * pixel_size],
								&input_argb[((y_base + y) * width + (x_base + x)) * pixel_size], pixel_size);
						}
						else
						{
							memset(&uncompressed[(y * block_width + x) * pixel_size], 0, pixel_size);
						}
					}
				}

				uint32_t index = ((y_base / block_height) * ((width + block_width - 1) / block_width) + (x_base / block_width)) * block_bytes;
				codec->EncodeBlock(&bc_blocks[index], &uncompressed[0], TCM_Balanced);
			}
		}
	}
	else
	{
		Texture::TextureType type;
		uint32_t depth, num_mipmaps, array_size;
		ElementFormat format;
		std::vector<ElementInitData> init_data;
		std::vector<uint8_t> data_block;
		LoadTexture(tc_name, type, width, height, depth, num_mipmaps, array_size,
			format, init_data, data_block);

		uint8_t const * src = static_cast<uint8_t const *>(init_data[0].data);
		uint32_t const pitch = init_data[0].row_pitch;
		for (uint32_t y = 0; y < (height + block_height - 1) / block_height; ++ y)
		{
			memcpy(&bc_blocks[y * ((width + block_width - 1) / block_width * block_bytes)], src,
				(width + block_width - 1) / block_width * block_bytes);
			src += pitch;
		}
	}

	std::vector<uint8_t> restored_argb(width * height * pixel_size);
	for (uint32_t y_base = 0; y_base < height; y_base += block_height)
	{
		for (uint32_t x_base = 0; x_base < width; x_base += block_width)
		{
			uint32_t index = ((y_base / block_height) * ((width + block_width - 1) / block_width) + (x_base / block_width)) * block_bytes;

			std::vector<uint8_t> argb_block(block_width * block_height * pixel_size);
			codec->DecodeBlock(&argb_block[0], &bc_blocks[index]);
			for (uint32_t y = 0; y < block_height; ++ y)
			{
				for (uint32_t x = 0; x < block_width; ++ x)
				{
					if ((x_base + x < width) && (y_base + y < height))
					{
						memcpy(&restored_argb[((y_base + y) * width + (x_base + x)) * pixel_size],
							&argb_block[(y * block_width + x) * pixel_size], pixel_size);
					}
				}
			}
		}
	}

	float mse = 0;
	if (EF_ABGR16F == decoded_fmt)
	{
		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				float const a0 = *reinterpret_cast<half const *>(&input_argb[((y * width + x) * 4 + 3) * 2]);
				float const r0 = *reinterpret_cast<half const *>(&input_argb[((y * width + x) * 4 + 2) * 2]);
				float const g0 = *reinterpret_cast<half const *>(&input_argb[((y * width + x) * 4 + 1) * 2]);
				float const b0 = *reinterpret_cast<half const *>(&input_argb[((y * width + x) * 4 + 0) * 2]);
				float const a1 = *reinterpret_cast<half const *>(&restored_argb[((y * width + x) * 4 + 3) * 2]);
				float const r1 = *reinterpret_cast<half const *>(&restored_argb[((y * width + x) * 4 + 2) * 2]);
				float const g1 = *reinterpret_cast<half const *>(&restored_argb[((y * width + x) * 4 + 1) * 2]);
				float const b1 = *reinterpret_cast<half const *>(&restored_argb[((y * width + x) * 4 + 0) * 2]);

				float diff_a = a0 - a1;
				float diff_r = r0 - r1;
				float diff_g = g0 - g1;
				float diff_b = b0 - b1;

				mse += (diff_a * diff_a + diff_r * diff_r + diff_g * diff_g + diff_b * diff_b);
			}
		}
	}
	else
	{
		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				float const a0 = input_argb[(y * width + x) * 4 + 3];
				float const r0 = input_argb[(y * width + x) * 4 + 2];
				float const g0 = input_argb[(y * width + x) * 4 + 1];
				float const b0 = input_argb[(y * width + x) * 4 + 0];
				float const a1 = restored_argb[(y * width + x) * 4 + 3];
				float const r1 = restored_argb[(y * width + x) * 4 + 2];
				float const g1 = restored_argb[(y * width + x) * 4 + 1];
				float const b1 = restored_argb[(y * width + x) * 4 + 0];

				float diff_a = a0 - a1;
				float diff_r = r0 - r1;
				float diff_g = g0 - g1;
				float diff_b = b0 - b1;

				mse += diff_a * diff_a + diff_r * diff_r + diff_g * diff_g + diff_b * diff_b;
			}
		}
	}

	mse = sqrt(mse / (width * height) / 4);
	EXPECT_LT(mse, threshold);
}

TEST(EncodeDecodeTexTest, DecodeBC1)
{
	TestEncodeDecodeTex("Lenna.dds", "Lenna_bc1.dds", EF_BC1, 4.7f);
}

TEST(EncodeDecodeTexTest, DecodeBC2)
{
	TestEncodeDecodeTex("leaf_v3_green_tex.dds", "leaf_v3_green_tex_bc2.dds", EF_BC2, 9.0f);
}

TEST(EncodeDecodeTexTest, DecodeBC3)
{
	TestEncodeDecodeTex("leaf_v3_green_tex.dds", "leaf_v3_green_tex_bc3.dds", EF_BC3, 8.8f);
}

TEST(EncodeDecodeTexTest, DecodeBC6U)
{
	TestEncodeDecodeTex("memorial.dds", "memorial_bc6u.dds", EF_BC6, 0.1f);
}

TEST(EncodeDecodeTexTest, DecodeBC6S)
{
	TestEncodeDecodeTex("uffizi_probe.dds", "uffizi_probe_bc6s.dds", EF_SIGNED_BC6, 0.1f);
}

TEST(EncodeDecodeTexTest, DecodeBC7XRGB)
{
	TestEncodeDecodeTex("Lenna.dds", "Lenna_bc7.dds", EF_BC7, 2.1f);
}

TEST(EncodeDecodeTexTest, DecodeBC7ARGB)
{
	TestEncodeDecodeTex("leaf_v3_green_tex.dds", "leaf_v3_green_tex_bc7.dds", EF_BC7, 8.6f);
}

TEST(EncodeDecodeTexTest, EncodeDecodeBC1)
{
	TestEncodeDecodeTex("Lenna.dds", "", EF_BC1, 4.6f);
}

TEST(EncodeDecodeTexTest, EncodeDecodeBC2)
{
	TestEncodeDecodeTex("leaf_v3_green_tex.dds", "", EF_BC2, 9.1f);
}

TEST(EncodeDecodeTexTest, EncodeDecodeBC3)
{
	TestEncodeDecodeTex("leaf_v3_green_tex.dds", "", EF_BC3, 8.9f);
}

TEST(EncodeDecodeTexTest, EncodeDecodeBC7XRGB)
{
	TestEncodeDecodeTex("Lenna.dds", "", EF_BC7, 1.8f);
}

TEST(EncodeDecodeTexTest, EncodeDecodeBC7ARGB)
{
	TestEncodeDecodeTex("leaf_v3_green_tex.dds", "", EF_BC7, 11.0f);
}

TEST(EncodeDecodeTexTest, EncodeDecodeETC1)
{
	TestEncodeDecodeTex("Lenna.dds", "", EF_ETC1, 4.8f);
}
