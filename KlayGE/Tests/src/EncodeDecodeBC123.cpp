#include <KlayGE/KlayGE.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/Texture.hpp>

#define BOOST_TEST_MODULE EncodeDecodeBC123
#include <boost/test/unit_test.hpp>

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace KlayGE;

uint32_t xrgb[16] = 
{
	0xFFC55D5A,
	0xFFCC6E6D,
	0xFFCD7775,
	0xFFAB595F,
	0xFFBF5858,
	0xFFAF5B6A,
	0xFFC98F8F,
	0xFFB06B71,
	0xFFA75162,
	0xFF92445E,
	0xFFB35866,
	0xFFCD907F,
	0xFF914C60,
	0xFF8B3D5A,
	0xFFD2817B,
	0xFFAE6363
};

void TestEncodeDecodeBC1()
{
	uint32_t const width = 4;
	uint32_t const height = 4;

	TexCompressionPtr codec = MakeSharedPtr<TexCompressionBC1>();

	ElementFormat const decoded_fmt = codec->DecodedFormat();
	uint32_t const pixel_size = NumFormatBytes(decoded_fmt);
	BOOST_CHECK(4 == pixel_size);

	uint32_t const block_width = codec->BlockWidth();
	uint32_t const block_height = codec->BlockWidth();
	uint32_t const block_bytes = codec->BlockBytes();
	std::vector<uint8_t> bc_blocks((width + block_width - 1) / block_width
		* (height + block_height - 1) / block_height * block_bytes);
	std::vector<uint8_t> input_argb(block_width * block_height * pixel_size);
	for (uint32_t y = 0; y < block_height; ++ y)
	{
		for (uint32_t x = 0; x < block_width; ++ x)
		{
			uint8_t pixel[4];
			memcpy(pixel, &xrgb[y * block_width + x], pixel_size);
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

			memcpy(&input_argb[(y * block_width + x) * pixel_size], pixel, pixel_size);
		}
	}

	codec->EncodeBlock(&bc_blocks[0], &input_argb[0], TCM_Quality);

	std::vector<uint8_t> restored_argb(block_width * block_height * pixel_size);
	codec->DecodeBlock(&restored_argb[0], &bc_blocks[0]);

	float mse = 0;
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			int const a0 = input_argb[(y * width + x) * 4 + 3];
			int const r0 = input_argb[(y * width + x) * 4 + 2];
			int const g0 = input_argb[(y * width + x) * 4 + 1];
			int const b0 = input_argb[(y * width + x) * 4 + 0];
			int const a1 = restored_argb[(y * width + x) * 4 + 3];
			int const r1 = restored_argb[(y * width + x) * 4 + 2];
			int const g1 = restored_argb[(y * width + x) * 4 + 1];
			int const b1 = restored_argb[(y * width + x) * 4 + 0];

			int diff_a = a0 - a1;
			int diff_r = r0 - r1;
			int diff_g = g0 - g1;
			int diff_b = b0 - b1;

			mse += diff_a * diff_a + diff_r * diff_r + diff_g * diff_g + diff_b * diff_b;
		}
	}

	mse /= (width * height);
	BOOST_CHECK(sqrt(mse) < 20);
}

BOOST_AUTO_TEST_CASE(EncodeDecodeBC1)
{
	TestEncodeDecodeBC1();
}

