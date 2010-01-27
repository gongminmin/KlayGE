// BlockCompression.cpp
// KlayGE 纹理分块压缩 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2008-2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 增加了DecodeBC1/3/4 (2010.1.27)
//
// 3.8.0
// 初次建立(2008.12.9)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Color.hpp>

#include <KlayGE/BlockCompression.hpp>

namespace KlayGE
{
	Color RGB565_to_Color(uint16_t rgb)
	{
		return Color(((rgb >> 11) & 0x1F) / 31.0f, ((rgb >> 5) & 0x3F) / 63.0f, ((rgb >> 0) & 0x1F) / 31.0f, 1);
	}

	void DecodeBC1Internal(uint32_t* argb, BC1_layout const & bc1)
	{
		boost::array<Color, 4> clr;
		clr[0] = RGB565_to_Color(bc1.clr_0);
		clr[1] = RGB565_to_Color(bc1.clr_1);
		if (bc1.clr_0 > bc1.clr_1)
		{
			clr[2] = MathLib::lerp(clr[0], clr[1], 1 / 3.0f);
			clr[3] = MathLib::lerp(clr[0], clr[1], 2 / 3.0f);
		}
		else
		{
			clr[2] = (clr[0] + clr[1]) / 2;
			clr[3] = Color(0, 0, 0, 0);
		}

		for (int i = 0; i < 2; ++ i)
		{
			for (int j = 0; j < 8; ++ j)
			{
				argb[i * 8 + j] = clr[(bc1.bitmap[i] >> (j * 2)) & 0x3].ARGB();
			}
		}
	}

	void DecodeBC4Internal(uint8_t* alpha_block, BC4_layout const & bc4)
	{
		boost::array<float, 8> alpha;
		alpha[0] = bc4.alpha_0 / 255.0f;
		alpha[1] = bc4.alpha_1 / 255.0f;
		if (alpha[0] > alpha[1])
		{
			alpha[2] = MathLib::lerp(alpha[0], alpha[1], 1 / 7.0f);
			alpha[3] = MathLib::lerp(alpha[0], alpha[1], 2 / 7.0f);
			alpha[4] = MathLib::lerp(alpha[0], alpha[1], 3 / 7.0f);
			alpha[5] = MathLib::lerp(alpha[0], alpha[1], 4 / 7.0f);
			alpha[6] = MathLib::lerp(alpha[0], alpha[1], 5 / 7.0f);
			alpha[7] = MathLib::lerp(alpha[0], alpha[1], 6 / 7.0f);
		}
		else
		{
			alpha[2] = MathLib::lerp(alpha[0], alpha[1], 1 / 5.0f);
			alpha[3] = MathLib::lerp(alpha[0], alpha[1], 2 / 5.0f);
			alpha[4] = MathLib::lerp(alpha[0], alpha[1], 3 / 5.0f);
			alpha[5] = MathLib::lerp(alpha[0], alpha[1], 4 / 5.0f);
			alpha[6] = 0;
			alpha[7] = 1;
		}

		for (int i = 0; i < 2; ++ i)
		{
			uint32_t alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0] << 0);
			for (int j = 0; j < 8; ++ j)
			{
				alpha_block[i * 8 + j] = static_cast<uint8_t>(alpha[(alpha32 >> (j * 3)) & 0x7] * 255.0f + 0.5f);
			}
		}
	}

	void DecodeBC1(uint32_t* argb, uint8_t const * bc1)
	{
		DecodeBC1Internal(argb, *reinterpret_cast<BC1_layout const *>(bc1));
	}
	
	void DecodeBC2(uint32_t* argb, uint8_t const * bc2)
	{
		BC2_layout const * bc2_layout = reinterpret_cast<BC2_layout const *>(bc2);

		DecodeBC1Internal(argb, bc2_layout->bc1);

		for (int i = 0; i < 16; ++ i)
		{
			argb[i] &= 0x00FFFFFF;
		}

		for (int i = 0; i < 4; ++ i)
		{
			for (int j = 0; j < 4; ++ j)
			{
				argb[i * 4 + j] |= (((bc2_layout->alpha[i] >> (4 * j)) & 0xF) << 4) << 24;
			}
		}
	}

	void DecodeBC3(uint32_t* argb, uint8_t const * bc3)
	{
		BC3_layout const * bc3_layout = reinterpret_cast<BC3_layout const *>(bc3);

		DecodeBC1Internal(argb, bc3_layout->bc1);

		boost::array<uint8_t, 16> alpha_block;
		DecodeBC4Internal(&alpha_block[0], bc3_layout->alpha);

		for (size_t i = 0; i < alpha_block.size(); ++ i)
		{
			argb[i] &= 0x00FFFFFF;
			argb[i] |= alpha_block[i] << 24;
		}
	}

	void DecodeBC4(uint8_t* alpha_block, uint8_t const * bc4)
	{
		DecodeBC4Internal(alpha_block, *reinterpret_cast<BC4_layout const *>(bc4));
	}

	void DecodeBC5(uint32_t* argb, uint8_t const * bc5)
	{
		BC5_layout const * bc5_layout = reinterpret_cast<BC5_layout const *>(bc5);

		boost::array<uint8_t, 16> block_0;
		DecodeBC4Internal(&block_0[0], bc5_layout->red);
		boost::array<uint8_t, 16> block_1;
		DecodeBC4Internal(&block_1[0], bc5_layout->green);

		for (size_t i = 0; i < block_0.size(); ++ i)
		{
			argb[i] = (block_0[i] << 16) | (block_1[i] << 8);
		}
	}

	void EncodeBC4(BC4_layout& bc4, uint8_t const * alpha)
	{
		float max_value = 0;
		float min_value = 1;
		boost::array<float, 16> float_alpha;
		for (size_t i = 0; i < float_alpha.size(); ++ i)
		{
			float value = alpha[i] / 255.0f;

			float_alpha[i] = value;

			min_value = std::min(min_value, value);
			max_value = std::max(max_value, value);
		}

		uint32_t const L = ((0.0f == min_value) || (1.0f == max_value)) ? 6 : 8;

		if ((6 == L) && (min_value == max_value))
		{
			max_value = 1;
		}

		// Newton's Method
		// x_{n+1} = x_n-\frac{f(x_n)}{f'(x_n)}\,
		// f_min(x) = \int (1-f)(src-(x(1-f)-bf))\, dx
		// f_min'(x) = (1-f)(src-(x(1-f)-bf))
		// f_min''(x) = (1-fs)^2
		// f_max(x) = \int f(src-(a(1-f)-xf))\, dx
		// f_max'(x) = f(src-(a(1-f)-xf))\
		// f_max''(x) = fs^2
		float const num_steps = L - 1.0f;
		for (size_t i = 0; i < 8; ++ i)
		{
			if ((max_value - min_value) < 1.0f / 256.0f)
			{
				break;
			}

			float const inv_scale = num_steps / (max_value - min_value);

			float steps[8];
			for (size_t j = 0; j < L; ++ j)
			{
				steps[j] = MathLib::lerp(min_value, max_value, j / num_steps);
			}

			if (6 == L)
			{
				steps[6] = 0.0f;
				steps[7] = 1.0f;
			}

			float d_min = 0;
			float d_max = 0;
			float d_2_min = 0;
			float d_2_max = 0;
			for (size_t j = 0; j < float_alpha.size(); ++ j)
			{
				float d = (float_alpha[j] - min_value) * inv_scale;

				uint32_t s;
				if (d <= 0.0f)
				{
					s = ((6 == L) && (float_alpha[j] <= min_value * 0.5f)) ? 6 : 0;
				}
				else
				{
					if (d >= num_steps)
					{
						s = ((6 == L) && (float_alpha[j] >= (max_value + 1.0f) * 0.5f)) ? 7 : (L - 1);
					}
					else
					{
						s = static_cast<int>(d + 0.5f);
					}
				}

				if (s < L)
				{
					float const diff = float_alpha[j] - steps[s];
					float const fs = s / num_steps;

					d_min += (1 - fs) * diff;
					d_2_min += (1 - fs) * (1 - fs);

					d_max += fs * diff;
					d_2_max += fs * fs;
				}
			}

			if (d_2_min > 0.0f)
			{
				min_value -= d_min / d_2_min;
			}
			if (d_2_max > 0.0f)
			{
				max_value -= d_max / d_2_max;
			}

			if (min_value > max_value)
			{
				std::swap(min_value, max_value);
			}

			if ((d_min * d_min < 1.0f / 64) && (d_max * d_max < 1.0f / 64))
			{
				break;
			}
		}

		min_value = (min_value < 0.0f) ? 0.0f : (min_value > 1.0f) ? 1.0f : min_value;
		max_value = (max_value < 0.0f) ? 0.0f : (max_value > 1.0f) ? 1.0f : max_value;

		float ref_alpha0, ref_alpha1;
		if (6 == L)
		{
			ref_alpha0 = min_value;
			ref_alpha1 = max_value;
		}
		else
		{
			ref_alpha0 = max_value;
			ref_alpha1 = min_value;
		}

		bc4.alpha_0 = static_cast<uint8_t>(ref_alpha0 * 255 + 0.5f);
		bc4.alpha_1 = static_cast<uint8_t>(ref_alpha1 * 255 + 0.5f);

		boost::array<float, 8> steps;
		steps[0] = ref_alpha0;
		steps[1] = ref_alpha1;
		for (uint32_t i = 1; i < L - 1; ++ i)
		{
			steps[i + 1] = MathLib::lerp(steps[0], steps[1], i / num_steps);
		}

		if (6 == L)
		{
			steps[6] = 0;
			steps[7] = 1;
		}

		float inv_scale = (steps[0] != steps[1]) ? (num_steps / (steps[1] - steps[0])) : 0.0f;

		boost::array<uint8_t, 16> bit_code;
		for (uint8_t i = 0; i < float_alpha.size(); ++ i)
		{
			bit_code[i] = 0;

			float d = (float_alpha[i] - steps[0]) * inv_scale;

			uint8_t s;
			if (d <= 0.0f)
			{
				s = ((6 == L) && (float_alpha[i] <= steps[0] * 0.5f)) ? 6 : 0;
			}
			else
			{
				if (d >= num_steps)
				{
					s = ((6 == L) && (float_alpha[i] >= (steps[1] + 1.0f) * 0.5f)) ? 7 : 1;
				}
				else
				{
					int id = static_cast<int>(d + 0.5f);
					if (0 == id)
					{
						s = 0;
					}
					else
					{
						if (num_steps == id)
						{
							s = 1;
						}
						else
						{
							s = static_cast<uint8_t>(id + 1);
						}
					}
				}
			}

			bit_code[i] = s;
		}

		bc4.bitmap[0] = ((bit_code[2] & 0x3) << 6) | (bit_code[1] << 3) | (bit_code[0] << 0);
		bc4.bitmap[1] = ((bit_code[5] & 0x1) << 7) | (bit_code[4] << 4) | (bit_code[3] << 1) | ((bit_code[2] >> 2) << 0);
		bc4.bitmap[2] = (bit_code[7] << 5) | (bit_code[6] << 2) | ((bit_code[5] >> 1) << 0);
		bc4.bitmap[3] = ((bit_code[10] & 0x3) << 6) | (bit_code[9] << 3) | (bit_code[8] << 0);
		bc4.bitmap[4] = ((bit_code[13] & 0x1) << 7) | (bit_code[12] << 4) | (bit_code[11] << 1) | ((bit_code[10] >> 2) << 0);
		bc4.bitmap[5] = (bit_code[15] << 5) | (bit_code[14] << 2) | ((bit_code[13] >> 1) << 0);
	}

	void BC4ToBC1G(BC1_layout& bc1, BC4_layout const & bc4)
	{
		bc1.clr_0 = (bc4.alpha_0 >> 2) << 5;
		bc1.clr_1 = (bc4.alpha_1 >> 2) << 5;
		bool swap_clr = false;
		if (bc4.alpha_0 < bc4.alpha_1)
		{
			std::swap(bc1.clr_0, bc1.clr_1);
			swap_clr = true;
		}
		for (int i = 0; i < 2; ++ i)
		{
			uint32_t alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0] << 0);
			uint16_t mask = 0;
			for (int j = 0; j < 8; ++ j)
			{
				uint16_t bit = (alpha32 >> (j * 3)) & 0x7;
				if (swap_clr)
				{
					switch (bit)
					{
					case 0:
					case 6:
						bit = 0;
						break;

					case 1:
					case 7:
						bit = 1;
						break;

					case 2:
					case 3:
						bit = 2;
						break;

					case 4:
					case 5:
						bit = 3;
						break;
					}
				}
				else
				{
					switch (bit)
					{
					case 0:
					case 2:
						bit = 0;
						break;

					case 1:
					case 5:
					case 7:
						bit = 1;
						break;

					case 3:
					case 4:
					case 6:
						bit = 2;
						break;
					}
				}

				mask |= bit << (j * 2);
			}

			bc1.bitmap[i] = mask;
		}
	}
}
