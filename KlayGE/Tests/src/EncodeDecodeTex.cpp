#include <KlayGE/KlayGE.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/TexCompressionETC.hpp>
#include <KlayGE/Texture.hpp>

#include <boost/assert.hpp>

#define BOOST_TEST_MODULE EncodeDecodeTex
#include <boost/test/unit_test.hpp>

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace KlayGE;

uint32_t lena_xrgb[16] = 
{
	0xFFC55D5A, 0xFFCC6E6D, 0xFFCD7775, 0xFFAB595F,
	0xFFBF5858, 0xFFAF5B6A, 0xFFC98F8F, 0xFFB06B71,
	0xFFA75162, 0xFF92445E, 0xFFB35866, 0xFFCD907F,
	0xFF914C60, 0xFF8B3D5A, 0xFFD2817B, 0xFFAE6363
};

uint32_t leaf_argb[16] = 
{
	0xFE5E854A, 0xF65A8A44, 0xD4548542, 0x905D7F4C,
	0xFF648550, 0xFA5C854A, 0xE3588745, 0xA8587F47,
	0xFF5E854C, 0xFC60834E, 0xED578449, 0xBE558344,
	0xFF62874F, 0xFE60844E, 0xF55E844E, 0xD15B8849
};

void TestEncodeDecodeTex(uint32_t const * argb, ElementFormat bc_fmt, float threshold)
{
	TexCompressionPtr codec;
	switch (bc_fmt)
	{
	case EF_BC1:
		codec = MakeSharedPtr<TexCompressionBC1>();
		break;

	case EF_BC2:
		codec = MakeSharedPtr<TexCompressionBC2>();
		break;

	case EF_BC3:
		codec = MakeSharedPtr<TexCompressionBC3>();
		break;

	case EF_ETC1:
		codec = MakeSharedPtr<TexCompressionETC1>();
		break;

	default:
		BOOST_ASSERT(false);
		break;
	}

	ElementFormat const decoded_fmt = codec->DecodedFormat();
	uint32_t const pixel_size = NumFormatBytes(decoded_fmt);
	BOOST_CHECK(4 == pixel_size);

	uint32_t const block_width = codec->BlockWidth();
	uint32_t const block_height = codec->BlockWidth();
	uint32_t const block_bytes = codec->BlockBytes();
	std::vector<uint8_t> bc_blocks(block_bytes);
	std::vector<uint8_t> input_argb(block_width * block_height * pixel_size);
	memcpy(&input_argb[0], argb, input_argb.size());
	if (EF_BC1 == bc_fmt)
	{
		for (uint32_t i = 0; i < block_width * block_height; ++ i)
		{
			if ((argb[i] >> 24) < 128)
			{
				input_argb[i * 4 + 0] = 0;
				input_argb[i * 4 + 1] = 0;
				input_argb[i * 4 + 2] = 0;
				input_argb[i * 4 + 3] = 0;
			}
			else
			{
				input_argb[i * 4 + 3] = 255;
			}
		}
	}

	codec->EncodeBlock(&bc_blocks[0], &input_argb[0], TCM_Quality);

	std::vector<uint8_t> restored_argb(block_width * block_height * pixel_size);
	codec->DecodeBlock(&restored_argb[0], &bc_blocks[0]);

	float mse = 0;
	for (uint32_t i = 0; i < block_width * block_height; ++i)
	{
		int const a0 = input_argb[i * 4 + 3];
		int const r0 = input_argb[i * 4 + 2];
		int const g0 = input_argb[i * 4 + 1];
		int const b0 = input_argb[i * 4 + 0];
		int const a1 = restored_argb[i * 4 + 3];
		int const r1 = restored_argb[i * 4 + 2];
		int const g1 = restored_argb[i * 4 + 1];
		int const b1 = restored_argb[i * 4 + 0];

		int diff_a = a0 - a1;
		int diff_r = r0 - r1;
		int diff_g = g0 - g1;
		int diff_b = b0 - b1;

		mse += diff_a * diff_a + diff_r * diff_r + diff_g * diff_g + diff_b * diff_b;
	}

	mse = sqrt(mse / (block_width * block_height) / 4);
	BOOST_CHECK(mse < threshold);
}

BOOST_AUTO_TEST_CASE(EncodeDecodeBC1XRGB)
{
	TestEncodeDecodeTex(lena_xrgb, EF_BC1, 8);
}

BOOST_AUTO_TEST_CASE(EncodeDecodeBC2XRGB)
{
	TestEncodeDecodeTex(lena_xrgb, EF_BC2, 11);
}

BOOST_AUTO_TEST_CASE(EncodeDecodeBC2ARGB)
{
	TestEncodeDecodeTex(leaf_argb, EF_BC2, 6);
}

BOOST_AUTO_TEST_CASE(EncodeDecodeBC3XRGB)
{
	TestEncodeDecodeTex(lena_xrgb, EF_BC3, 8);
}

BOOST_AUTO_TEST_CASE(EncodeDecodeBC3ARGB)
{
	TestEncodeDecodeTex(leaf_argb, EF_BC3, 3);
}

BOOST_AUTO_TEST_CASE(EncodeDecodeETC1XRGB)
{
	TestEncodeDecodeTex(lena_xrgb, EF_ETC1, 8);
}
