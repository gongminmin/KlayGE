/**
* @file TexCompressionBC.cpp
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
#include <KFL/Color.hpp>
#include <KlayGE/Texture.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/TexCompressionBC.hpp>

namespace
{
	using namespace KlayGE;

	uint8_t Expand5[32];
	uint8_t Expand6[64];
	uint8_t OMatch5[256][2];
	uint8_t OMatch6[256][2];
	uint8_t QuantRBTab[256 + 16];
	uint8_t QuantGTab[256 + 16];

	int Mul8Bit(int a, int b)
	{
		int t = a * b + 128;
		return (t + (t >> 8)) >> 8;
	}

	void PrepareOptTable(uint8_t* Table, uint8_t const * expand, int size)
	{
		for (int i = 0; i < 256; ++ i)
		{
			int best_err = 256;

			for (int min = 0; min < size; ++ min)
			{
				for (int max = 0; max < size; ++ max)
				{
					int min_e = expand[min];
					int max_e = expand[max];
					int err = abs(max_e + Mul8Bit(min_e - max_e, 0x55) - i);
					if (err < best_err)
					{
						Table[i * 2 + 0] = static_cast<uint8_t>(max);
						Table[i * 2 + 1] = static_cast<uint8_t>(min);
						best_err = err;
					}
				}
			}
		}
	}

	class BCIniter
	{
	public:
		BCIniter()
		{
			for (int i = 0; i < 32; ++ i)
			{
				Expand5[i] = static_cast<uint8_t>((i << 3) | (i >> 2));
			}

			for (int i = 0; i < 64; ++ i)
			{
				Expand6[i] = static_cast<uint8_t>((i << 2) | (i >> 4));
			}

			for (int i = 0; i < 256 + 16; ++ i)
			{
				int v = MathLib::clamp(i - 8, 0, 255);
				QuantRBTab[i] = Expand5[Mul8Bit(v, 31)];
				QuantGTab[i] = Expand6[Mul8Bit(v, 63)];
			}

			PrepareOptTable(&OMatch5[0][0], Expand5, 32);
			PrepareOptTable(&OMatch6[0][0], Expand6, 64);
		}
	};
	BCIniter bc_initer;

	uint32_t RGB565To888(uint16_t rgb)
	{
		return 0xFF000000 | (Expand5[(rgb >> 11) & 0x1F] << 16) | (Expand6[(rgb >> 5) & 0x3F] << 8)
			| (Expand5[(rgb >> 0) & 0x1F] << 0);
	}

	uint16_t RGB888To565(uint32_t rgb)
	{
		return (((rgb >> 19) & 0x1F) << 11) | (((rgb >> 10) & 0x3F) << 5) | (((rgb >> 3) & 0x1F) << 0);
	}
}

namespace KlayGE
{
	void DecodeBC1Internal(uint32_t* argb, BC1Block const & bc1)
	{
		uint32_t max_clr = RGB565To888(bc1.clr_0);
		uint32_t min_clr = RGB565To888(bc1.clr_1);
		uint8_t const * max_u8 = reinterpret_cast<uint8_t const *>(&max_clr);
		uint8_t const * min_u8 = reinterpret_cast<uint8_t const *>(&min_clr);

		array<uint32_t, 4> clr;
		clr[0] = max_clr;
		clr[1] = min_clr;
		if (bc1.clr_0 > bc1.clr_1)
		{
			uint8_t* clr2_u8 = reinterpret_cast<uint8_t*>(&clr[2]);
			uint8_t* clr3_u8 = reinterpret_cast<uint8_t*>(&clr[3]);
			clr2_u8[0] = (max_u8[0] * 2 + min_u8[0]) / 3;
			clr2_u8[1] = (max_u8[1] * 2 + min_u8[1]) / 3;
			clr2_u8[2] = (max_u8[2] * 2 + min_u8[2]) / 3;
			clr2_u8[3] = 255;
			clr3_u8[0] = (max_u8[0] + min_u8[0] * 2) / 3;
			clr3_u8[1] = (max_u8[1] + min_u8[1] * 2) / 3;
			clr3_u8[2] = (max_u8[2] + min_u8[2] * 2) / 3;
			clr3_u8[3] = 255;
		}
		else
		{
			uint8_t* clr2_u8 = reinterpret_cast<uint8_t*>(&clr[2]);
			clr2_u8[0] = (max_u8[0] + min_u8[0]) / 2;
			clr2_u8[1] = (max_u8[1] + min_u8[1]) / 2;
			clr2_u8[2] = (max_u8[2] + min_u8[2]) / 2;
			clr2_u8[3] = 255;
			clr[3] = 0;
		}

		for (int i = 0; i < 2; ++ i)
		{
			for (int j = 0; j < 8; ++ j)
			{
				argb[i * 8 + j] = clr[(bc1.bitmap[i] >> (j * 2)) & 0x3];
			}
		}
	}

	void DecodeBC4Internal(uint8_t* alpha_block, BC4Block const & bc4)
	{
		array<uint8_t, 8> alpha;
		float falpha0 = bc4.alpha_0 / 255.0f;
		float falpha1 = bc4.alpha_1 / 255.0f;
		alpha[0] = bc4.alpha_0;
		alpha[1] = bc4.alpha_1;
		if (alpha[0] > alpha[1])
		{
			alpha[2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 1 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[3] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 2 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[4] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 3 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[5] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 4 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[6] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 5 / 7.0f) * 255 + 0.5f), 0, 255));
			alpha[7] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 6 / 7.0f) * 255 + 0.5f), 0, 255));
		}
		else
		{
			alpha[2] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 1 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[3] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 2 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[4] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 3 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[5] = static_cast<uint8_t>(MathLib::clamp(static_cast<int>(MathLib::lerp(falpha0, falpha1, 4 / 5.0f) * 255 + 0.5f), 0, 255));
			alpha[6] = 0;
			alpha[7] = 255;
		}

		for (int i = 0; i < 2; ++ i)
		{
			uint32_t alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0] << 0);
			for (int j = 0; j < 8; ++ j)
			{
				alpha_block[i * 8 + j] = alpha[(alpha32 >> (j * 3)) & 0x7];
			}
		}
	}

	// The color matching function
	uint32_t MatchColorsBlock(uint32_t const * argb, uint32_t min_clr, uint32_t max_clr, bool alpha)
	{
		uint8_t const * block = reinterpret_cast<uint8_t const *>(argb);
		uint8_t const * max_u8 = reinterpret_cast<uint8_t const *>(&max_clr);
		uint8_t const * min_u8 = reinterpret_cast<uint8_t const *>(&min_clr);

		array<uint32_t, 4> color;
		color[0] = max_clr;
		color[1] = min_clr;
		if (!alpha)
		{
			uint8_t* clr2_u8 = reinterpret_cast<uint8_t*>(&color[2]);
			uint8_t* clr3_u8 = reinterpret_cast<uint8_t*>(&color[3]);
			clr2_u8[0] = (max_u8[0] * 2 + min_u8[0]) / 3;
			clr2_u8[1] = (max_u8[1] * 2 + min_u8[1]) / 3;
			clr2_u8[2] = (max_u8[2] * 2 + min_u8[2]) / 3;
			clr2_u8[3] = 255;
			clr3_u8[0] = (max_u8[0] + min_u8[0] * 2) / 3;
			clr3_u8[1] = (max_u8[1] + min_u8[1] * 2) / 3;
			clr3_u8[2] = (max_u8[2] + min_u8[2] * 2) / 3;
			clr3_u8[3] = 255;
		}

		array<uint8_t*, 4> color_u8;
		color_u8[0] = reinterpret_cast<uint8_t*>(&color[0]);
		color_u8[1] = reinterpret_cast<uint8_t*>(&color[1]);
		color_u8[2] = reinterpret_cast<uint8_t*>(&color[2]);
		color_u8[3] = reinterpret_cast<uint8_t*>(&color[3]);

		uint32_t mask = 0;
		int dirr = color_u8[0][2] - color_u8[1][2];
		int dirg = color_u8[0][1] - color_u8[1][1];
		int dirb = color_u8[0][0] - color_u8[1][0];

		int dots[16];
		for (int i = 0; i < 16; ++ i)
		{
			dots[i] = block[i * 4 + 2] * dirr + block[i * 4 + 1] * dirg + block[i * 4 + 0] * dirb;
		}

		if (alpha)
		{
			array<int, 2> stops;
			for (int i = 0; i < 2; ++ i)
			{
				stops[i] = color_u8[i][2] * dirr + color_u8[i][1] * dirg + color_u8[i][0] * dirb;
			}

			int c0_point = (stops[0] + stops[1] * 2) / 3;
			int c3_point = (stops[0] * 2 + stops[1]) / 3;

			for (int i = 15; i >= 0; -- i)
			{
				mask <<= 2;
				int dot = dots[i];
				if (0 == block[i * 4 + 3])
				{
					mask |= 3;
				}
				else
				{
					if (dot >= c0_point)
					{
						mask |= (dot < c3_point) ? 2 : 1;
					}
				}
			}
		}
		else
		{
			array<int, 4> stops;
			for (int i = 0; i < 4; ++ i)
			{
				stops[i] = color_u8[i][2] * dirr + color_u8[i][1] * dirg + color_u8[i][0] * dirb;
			}
  
			int c0_point = (stops[1] + stops[3]) >> 1;
			int half_point = (stops[3] + stops[2]) >> 1;
			int c3_point = (stops[2] + stops[0]) >> 1;

			for (int i = 15; i >= 0; -- i)
			{
				mask <<= 2;
				int dot = dots[i];

				if (dot < half_point)
				{
					mask |= (dot < c0_point) ? 1 : 3;
				}
				else
				{
					mask |= (dot < c3_point) ? 2 : 0;
				}
			}
		}

		return mask;
	}

	// The color optimization function. (Clever code, part 1)
	void OptimizeColorsBlock(uint32_t const * argb, uint32_t& min_clr, uint32_t& max_clr, EBCMethod method)
	{
		if (method != EBCM_Quality)
		{
			Color const LUM_WEIGHT(0.2126f, 0.7152f, 0.0722f, 0);

			max_clr = min_clr = argb[0];
			float min_lum = MathLib::dot(Color(min_clr), LUM_WEIGHT);
			float max_lum = min_lum;
			for (size_t i = 1; i < 16; ++ i)
			{
				float lum = MathLib::dot(Color(argb[i]), LUM_WEIGHT);
				if (lum < min_lum)
				{
					min_lum = lum;
					min_clr = argb[i];
				}
				if (lum > max_lum)
				{
					max_lum = lum;
					max_clr = argb[i];
				}
			}
		}
		else
		{
			static int const ITER_POWER = 4;

			uint8_t const * block = reinterpret_cast<uint8_t const *>(argb);

			// determine color distribution
			int mu[3], min[3], max[3];

			for (int ch = 0; ch < 3; ++ ch)
			{
				uint8_t const * bp = block + ch;
				int muv, minv, maxv;

				muv = minv = maxv = bp[0];
				for (int i = 4; i < 64; i += 4)
				{
					muv += bp[i];
					minv = std::min<int>(minv, bp[i]);
					maxv = std::max<int>(maxv, bp[i]);
				}

				mu[ch] = (muv + 8) >> 4;
				min[ch] = minv;
				max[ch] = maxv;
			}

			// determine covariance matrix
			int cov[6];
			for (int i = 0; i < 6; ++ i)
			{
				cov[i] = 0;
			}

			for (int i = 0; i < 16; ++ i)
			{
				int r = block[i * 4 + 2] - mu[2];
				int g = block[i * 4 + 1] - mu[1];
				int b = block[i * 4 + 0] - mu[0];

				cov[0] += r * r;
				cov[1] += r * g;
				cov[2] += r * b;
				cov[3] += g * g;
				cov[4] += g * b;
				cov[5] += b * b;
			}

			// convert covariance matrix to float, find principal axis via power iter
			float covf[6], vfr, vfg, vfb;
			for (int i = 0; i < 6; ++ i)
			{
				covf[i] = cov[i] / 255.0f;
			}

			vfr = static_cast<float>(max[2] - min[2]);
			vfg = static_cast<float>(max[1] - min[1]);
			vfb = static_cast<float>(max[0] - min[0]);

			for (int iter = 0; iter < ITER_POWER; ++ iter)
			{
				float r = vfr * covf[0] + vfg * covf[1] + vfb * covf[2];
				float g = vfr * covf[1] + vfg * covf[3] + vfb * covf[4];
				float b = vfr * covf[2] + vfg * covf[4] + vfb * covf[5];

				vfr = r;
				vfg = g;
				vfb = b;
			}

			float magn = std::max(std::max(abs(vfr), abs(vfg)), abs(vfb));
			int v_r, v_g, v_b;

			if (magn < 4.0f) // too small, default to luminance
			{
				v_r = 148;
				v_g = 300;
				v_b = 58;
			}
			else
			{
				magn = 512.0f / magn;
				v_r = static_cast<int>(vfr * magn);
				v_g = static_cast<int>(vfg * magn);
				v_b = static_cast<int>(vfb * magn);
			}

			// Pick colors at extreme points
			int min_d = 0x7FFFFFFF, max_d = -min_d;
			min_clr = max_clr = 0;
			for (int i = 0; i < 16; ++ i)
			{
				int dot = block[i * 4 + 2] * v_r + block[i * 4 + 1] * v_g + block[i * 4 + 0] * v_b;
				if (dot < min_d)
				{
					min_d = dot;
					min_clr = argb[i];
				}
				if (dot > max_d)
				{
					max_d = dot;
					max_clr = argb[i];
				}
			}
		}
	}

	// The refinement function. (Clever code, part 2)
	// Tries to optimize colors to suit block contents better.
	// (By solving a least squares system via normal equations+Cramer's rule)
	bool RefineBlock(uint32_t const * argb, uint32_t& min_clr, uint32_t& max_clr, uint32_t mask)
	{
		static int const w1Tab[4] = { 3, 0, 2, 1 };
		static int const prods[4] = { 0x090000, 0x000900, 0x040102, 0x010402 };
		// ^some magic to save a lot of multiplies in the accumulating loop...

		uint8_t const * block = reinterpret_cast<uint8_t const *>(argb);

		int akku = 0;
		int At1_r, At1_g, At1_b;
		int At2_r, At2_g, At2_b;

		At1_r = At1_g = At1_b = 0;
		At2_r = At2_g = At2_b = 0;
		for (int i = 0; i < 16; ++ i, mask >>= 2)
		{
			int step = mask & 3;
			int w1 = w1Tab[step];
			int r = block[i * 4 + 2];
			int g = block[i * 4 + 1];
			int b = block[i * 4 + 0];

			akku    += prods[step];
			At1_r   += w1 * r;
			At1_g   += w1 * g;
			At1_b   += w1 * b;
			At2_r   += r;
			At2_g   += g;
			At2_b   += b;
		}

		At2_r = 3 * At2_r - At1_r;
		At2_g = 3 * At2_g - At1_g;
		At2_b = 3 * At2_b - At1_b;

		// extract solutions and decide solvability
		int xx = akku >> 16;
		int yy = (akku >> 8) & 0xFF;
		int xy = (akku >> 0) & 0xFF;

		if (!yy || !xx || (xx * yy == xy * xy))
		{
			return false;
		}

		float const f = 3.0f / 255.0f / (xx * yy - xy * xy);
		float const frb = f * 31.0f;
		float const fg = f * 63.0f;

		uint16_t old_min = RGB888To565(min_clr);
		uint16_t old_max = RGB888To565(max_clr);

		// solve.
		int max_r = MathLib::clamp<int>(static_cast<int>((At1_r * yy - At2_r * xy) * frb + 0.5f), 0, 31);
		int max_g = MathLib::clamp<int>(static_cast<int>((At1_g * yy - At2_g * xy) * fg  + 0.5f), 0, 63);
		int max_b = MathLib::clamp<int>(static_cast<int>((At1_b * yy - At2_b * xy) * frb + 0.5f), 0, 31);

		int min_r = MathLib::clamp<int>(static_cast<int>((At2_r * xx - At1_r * xy) * frb + 0.5f), 0, 31);
		int min_g = MathLib::clamp<int>(static_cast<int>((At2_g * xx - At1_g * xy) * fg  + 0.5f), 0, 63);
		int min_b = MathLib::clamp<int>(static_cast<int>((At2_b * xx - At1_b * xy) * frb + 0.5f), 0, 31);

		uint16_t max16 = static_cast<uint16_t>((max_r << 11) | (max_g << 5) | (max_b << 0));
		uint16_t min16 = static_cast<uint16_t>((min_r << 11) | (min_g << 5) | (min_b << 0));

		if ((old_min != min16) || (old_max != max16))
		{
			min_clr = RGB565To888(min16);
			max_clr = RGB565To888(max16);

			return true;
		}
		else
		{
			return false;
		}
	}

	void EncodeBC1Internal(BC1Block& bc1, uint32_t const * argb, bool alpha, EBCMethod method)
	{
		// check if block is constant
		uint32_t min32, max32;
		min32 = max32 = argb[0];
		for (int i = 1; i < 16; ++ i)
		{
			min32 = std::min(min32, argb[i]);
			max32 = std::max(max32, argb[i]);
		}

		uint32_t mask;
		uint16_t max16, min16;
		if (min32 != max32) // no constant color
		{
			uint32_t max_clr, min_clr;
			OptimizeColorsBlock(argb, min_clr, max_clr, method);
			max16 = RGB888To565(max_clr);
			min16 = RGB888To565(min_clr);
			if (max16 != min16)
			{
				mask = MatchColorsBlock(argb, min_clr, max_clr, alpha);
			}
			else
			{
				mask = 0;
			}
			if (!alpha && (method != EBCM_Speed))
			{
				if (RefineBlock(argb, min_clr, max_clr, mask))
				{
					max16 = RGB888To565(max_clr);
					min16 = RGB888To565(min_clr);
					if (max16 != min16)
					{
						mask = MatchColorsBlock(argb, min_clr, max_clr, alpha);
					}
					else
					{
						mask = 0;
					}
				}
			}
		}
		else // constant color
		{
			if (alpha && (0 == argb[0]))
			{
				mask = 0xFFFFFFFF;
				max16 = min16 = 0;
			}
			else
			{
				int r = (argb[0] >> 16) & 0xFF;
				int g = (argb[0] >> 8) & 0xFF;
				int b = (argb[0] >> 0) & 0xFF;

				mask  = 0xAAAAAAAA;
				max16 = (OMatch5[r][0] << 11) | (OMatch6[g][0] << 5) | OMatch5[b][0];
				min16 = (OMatch5[r][1] << 11) | (OMatch6[g][1] << 5) | OMatch5[b][1];
			}
		}

		if (alpha)
		{
			if (max16 < min16)
			{
				std::swap(max16, min16);

				uint32_t xor_mask = 0;
				for (int i = 15; i >= 0; -- i)
				{
					xor_mask <<= 2;

					uint32_t pixel_mask = (mask >> (i * 2)) & 0x3;
					if (pixel_mask <= 1)
					{
						xor_mask |= 0x1;
					}
				}
				mask ^= xor_mask;
			}

			bc1.clr_0 = min16;
			bc1.clr_1 = max16;
		}
		else
		{
			if (max16 < min16)
			{
				std::swap(max16, min16);
				mask ^= 0x55555555;
			}

			bc1.clr_0 = max16;
			bc1.clr_1 = min16;
		}
		std::memcpy(bc1.bitmap, &mask, sizeof(mask));
	}

	// Alpha block compression (this is easy for a change)
	void EncodeBC4Internal(BC4Block& bc4, uint8_t const * alpha)
	{
		// find min/max color
		int min, max;
		min = max = alpha[0];

		for (int i = 1; i < 16; ++ i)
		{
			min = std::min<int>(min, alpha[i]);
			max = std::max<int>(max, alpha[i]);
		}

		// encode them
		bc4.alpha_0 = static_cast<uint8_t>(max);
		bc4.alpha_1 = static_cast<uint8_t>(min);

		// determine bias and emit color indices
		int dist = max - min;
		int bias = min * 7 - (dist >> 1);
		int dist4 = dist * 4;
		int dist2 = dist * 2;
		int bits = 0, mask = 0;

		int dest = 0;
		for (int i = 0; i < 16; ++ i)
		{
			int a = alpha[i] * 7 - bias;
			int ind, t;

			// select index (hooray for bit magic)
			t = (dist4 - a) >> 31;  ind =  t & 4; a -= dist4 & t;
			t = (dist2 - a) >> 31;  ind += t & 2; a -= dist2 & t;
			t = (dist - a) >> 31;   ind += t & 1;

			ind = -ind & 7;
			ind ^= (2 > ind);

			// write index
			mask |= ind << bits;
			if ((bits += 3) >= 8)
			{
				bc4.bitmap[dest] = static_cast<uint8_t>(mask);
				++ dest;
				mask >>= 8;
				bits -= 8;
			}
		}
	}

	void DecodeBC1(uint32_t* argb, void const * bc1)
	{
		DecodeBC1Internal(argb, *static_cast<BC1Block const *>(bc1));
	}
	
	void DecodeBC2(uint32_t* argb, void const * bc2)
	{
		BC2Block const * bc2_block = static_cast<BC2Block const *>(bc2);

		DecodeBC1(argb, &bc2_block->bc1);

		for (int i = 0; i < 16; ++ i)
		{
			argb[i] &= 0x00FFFFFF;
		}

		for (int i = 0; i < 4; ++ i)
		{
			for (int j = 0; j < 4; ++ j)
			{
				argb[i * 4 + j] |= (((bc2_block->alpha[i] >> (4 * j)) & 0xF) << 4) << 24;
			}
		}
	}

	void DecodeBC3(uint32_t* argb, void const * bc3)
	{
		BC3Block const * bc3_block = static_cast<BC3Block const *>(bc3);

		DecodeBC1(argb, &bc3_block->bc1);

		array<uint8_t, 16> alpha_block;
		DecodeBC4Internal(&alpha_block[0], bc3_block->alpha);

		for (size_t i = 0; i < alpha_block.size(); ++ i)
		{
			argb[i] &= 0x00FFFFFF;
			argb[i] |= alpha_block[i] << 24;
		}
	}

	void DecodeBC4(uint8_t* r, void const * bc4)
	{
		DecodeBC4Internal(r, *static_cast<BC4Block const *>(bc4));
	}

	void DecodeBC5(uint8_t* r, uint8_t* g, void const * bc5)
	{
		BC5Block const * bc5_block = static_cast<BC5Block const *>(bc5);

		array<uint8_t, 16> block_0;
		DecodeBC4(&block_0[0], &bc5_block->red);
		array<uint8_t, 16> block_1;
		DecodeBC4(&block_1[0], &bc5_block->green);

		for (size_t i = 0; i < block_0.size(); ++ i)
		{
			r[i] = block_0[i];
			g[i] = block_1[i];
		}
	}

	void DecodeBC1(void* argb, uint32_t pitch, void const * bc1, uint32_t width, uint32_t height)
	{
		BC1Block const * src = static_cast<BC1Block const *>(bc1);
		uint32_t * dst = static_cast<uint32_t*>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			uint32_t const block_h = std::min(4U, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t const block_w = std::min(4U, width - x_base);

				DecodeBC1(&uncompressed[0], src);
				++ src;

				for (uint32_t y = 0; y < block_h; ++ y)
				{
					for (uint32_t x = 0; x < block_w; ++ x)
					{
						dst[(y_base + y) * pitch / 4 + (x_base + x)] = uncompressed[y * 4 + x];
					}
				}
			}
		}
	}

	void DecodeBC2(void* argb, uint32_t pitch, void const * bc2, uint32_t width, uint32_t height)
	{
		BC2Block const * src = static_cast<BC2Block const *>(bc2);
		uint32_t * dst = static_cast<uint32_t*>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			uint32_t const block_h = std::min(4U, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t const block_w = std::min(4U, width - x_base);

				DecodeBC2(&uncompressed[0], src);
				++ src;

				for (uint32_t y = 0; y < block_h; ++ y)
				{
					for (uint32_t x = 0; x < block_w; ++ x)
					{
						dst[(y_base + y) * pitch / 4 + (x_base + x)] = uncompressed[y * 4 + x];
					}
				}
			}
		}
	}

	void DecodeBC3(void* argb, uint32_t pitch, void const * bc3, uint32_t width, uint32_t height)
	{
		BC3Block const * src = static_cast<BC3Block const *>(bc3);
		uint32_t * dst = static_cast<uint32_t*>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			uint32_t const block_h = std::min(4U, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t const block_w = std::min(4U, width - x_base);

				DecodeBC3(&uncompressed[0], src);
				++ src;

				for (uint32_t y = 0; y < block_h; ++ y)
				{
					for (uint32_t x = 0; x < block_w; ++ x)
					{
						dst[(y_base + y) * pitch / 4 + (x_base + x)] = uncompressed[y * 4 + x];
					}
				}
			}
		}
	}

	void DecodeBC4(void* r, uint32_t pitch, void const * bc4, uint32_t width, uint32_t height)
	{
		BC4Block const * src = static_cast<BC4Block const *>(bc4);
		uint8_t * dst = static_cast<uint8_t*>(r);

		uint8_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			uint32_t const block_h = std::min(4U, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t const block_w = std::min(4U, width - x_base);

				DecodeBC4(&uncompressed[0], src);
				++ src;

				for (uint32_t y = 0; y < block_h; ++ y)
				{
					for (uint32_t x = 0; x < block_w; ++ x)
					{
						dst[(y_base + y) * pitch + (x_base + x)] = uncompressed[y * 4 + x];
					}
				}
			}
		}
	}

	void DecodeBC5(void* gr, uint32_t pitch, void const * bc5, uint32_t width, uint32_t height)
	{
		BC5Block const * src = static_cast<BC5Block const *>(bc5);
		uint8_t * dst = static_cast<uint8_t*>(gr);

		uint8_t uncompressed_r[16];
		uint8_t uncompressed_g[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			uint32_t const block_h = std::min(4U, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t const block_w = std::min(4U, width - x_base);

				DecodeBC5(&uncompressed_r[0], &uncompressed_g[0], src);
				++ src;

				for (uint32_t y = 0; y < block_h; ++ y)
				{
					for (uint32_t x = 0; x < block_w; ++ x)
					{
						dst[(y_base + y) * pitch + (x_base + x) * 2 + 0] = uncompressed_r[y * 4 + x];
						dst[(y_base + y) * pitch + (x_base + x) * 2 + 1] = uncompressed_g[y * 4 + x];
					}
				}
			}
		}
	}

	void DecodeBC1(TexturePtr const & dst_tex, TexturePtr const & bc_tex)
	{
		uint32_t width = bc_tex->Width(0);
		uint32_t height = bc_tex->Height(0);

		TexturePtr argb8_tex;
		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_ARGB8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
		}
		else
		{
			argb8_tex = dst_tex;
		}

		{
			Texture::Mapper mapper_src(*bc_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*argb8_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			DecodeBC1(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height);
		}

		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex->CopyToTexture(*dst_tex);
		}
	}
	
	void DecodeBC2(TexturePtr const & dst_tex, TexturePtr const & bc_tex)
	{
		uint32_t width = bc_tex->Width(0);
		uint32_t height = bc_tex->Height(0);

		TexturePtr argb8_tex;
		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_ARGB8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
		}
		else
		{
			argb8_tex = dst_tex;
		}

		{
			Texture::Mapper mapper_src(*bc_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*argb8_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			DecodeBC2(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height);
		}

		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex->CopyToTexture(*dst_tex);
		}
	}

	void DecodeBC3(TexturePtr const & dst_tex, TexturePtr const & bc_tex)
	{
		uint32_t width = bc_tex->Width(0);
		uint32_t height = bc_tex->Height(0);

		TexturePtr argb8_tex;
		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_ARGB8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
		}
		else
		{
			argb8_tex = dst_tex;
		}

		{
			Texture::Mapper mapper_src(*bc_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*argb8_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			DecodeBC3(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height);
		}

		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex->CopyToTexture(*dst_tex);
		}
	}

	void DecodeBC4(TexturePtr const & dst_tex, TexturePtr const & bc_tex)
	{
		uint32_t width = bc_tex->Width(0);
		uint32_t height = bc_tex->Height(0);

		TexturePtr r8_tex;
		if (dst_tex->Format() != EF_ARGB8)
		{
			r8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_ARGB8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
		}
		else
		{
			r8_tex = dst_tex;
		}

		{
			Texture::Mapper mapper_src(*bc_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*r8_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			DecodeBC1(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height);
		}

		if (dst_tex->Format() != EF_ARGB8)
		{
			r8_tex->CopyToTexture(*dst_tex);
		}
	}

	void DecodeBC5(TexturePtr const & dst_tex, TexturePtr const & bc_tex)
	{
		uint32_t width = bc_tex->Width(0);
		uint32_t height = bc_tex->Height(0);

		TexturePtr gr8_tex;
		if (dst_tex->Format() != EF_ARGB8)
		{
			gr8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_ARGB8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
		}
		else
		{
			gr8_tex = dst_tex;
		}

		{
			Texture::Mapper mapper_src(*bc_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*gr8_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			DecodeBC5(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height);
		}

		if (dst_tex->Format() != EF_ARGB8)
		{
			gr8_tex->CopyToTexture(*dst_tex);
		}
	}

	void EncodeBC1(BC1Block& bc1, uint32_t const * argb, EBCMethod method)
	{
		array<uint32_t, 16> tmp_argb;
		bool alpha = false;
		for (size_t i = 0; i < tmp_argb.size(); ++ i)
		{
			if (((argb[i] >> 24) & 0xFF) < 0x80)
			{
				tmp_argb[i] = 0;
				alpha = true;
			}
			else
			{
				tmp_argb[i] = argb[i];
			}
		}

		EncodeBC1Internal(bc1, &tmp_argb[0], alpha, method);
	}

	void EncodeBC2(BC2Block& bc2, uint32_t const * argb, EBCMethod method)
	{
		array<uint8_t, 16> alpha;
		array<uint32_t, 16> xrgb;
		for (size_t i = 0; i < xrgb.size(); ++ i)
		{
			xrgb[i] = argb[i] | 0xFF000000;
			alpha[i] = static_cast<uint8_t>(argb[i] >> 28);
		}

		EncodeBC1Internal(bc2.bc1, &xrgb[0], false, method);

		for (int i = 0; i < 4; ++ i)
		{
			bc2.alpha[i] = (alpha[i * 4 + 0] << 0) | (alpha[i * 4 + 1] << 4)
				| (alpha[i * 4 + 2] << 8) | (alpha[i * 4 + 3] << 12);
		}
	}

	void EncodeBC3(BC3Block& bc3, uint32_t const * argb, EBCMethod method)
	{
		array<uint8_t, 16> alpha;
		array<uint32_t, 16> xrgb;
		for (size_t i = 0; i < xrgb.size(); ++ i)
		{
			xrgb[i] = argb[i] | 0xFF000000;
			alpha[i] = static_cast<uint8_t>(argb[i] >> 24);
		}

		EncodeBC1Internal(bc3.bc1, &xrgb[0], false, method);
		EncodeBC4Internal(bc3.alpha, &alpha[0]);
	}

	void EncodeBC4(BC4Block& bc4, uint8_t const * r)
	{
		EncodeBC4Internal(bc4, r);
	}

	void EncodeBC5(BC5Block& bc5, uint8_t const * r, uint8_t const * g)
	{
		EncodeBC4(bc5.red, r);
		EncodeBC4(bc5.green, g);
	}

	void EncodeBC1(void* bc1, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method)
	{
		uint32_t const * src = static_cast<uint32_t const *>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			BC1Block* dst = reinterpret_cast<BC1Block*>(static_cast<uint8_t*>(bc1) + y_base / 4 * out_pitch);

			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						uint32_t pixel = 0;
						if ((x_base + x < width) && (y_base + y < height))
						{
							pixel = src[(y_base + y) * in_pitch / 4 + (x_base + x)];
						}
						uncompressed[y * 4 + x] = pixel;
					}
				}

				EncodeBC1(*dst, &uncompressed[0], method);
				++ dst;
			}
		}
	}

	void EncodeBC2(void* bc2, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method)
	{
		uint32_t const * src = static_cast<uint32_t const *>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			BC2Block* dst = reinterpret_cast<BC2Block*>(static_cast<uint8_t*>(bc2) + y_base / 4 * out_pitch);

			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						uint32_t pixel = 0;
						if ((x_base + x < width) && (y_base + y < height))
						{
							pixel = src[(y_base + y) * in_pitch / 4 + (x_base + x)];
						}
						uncompressed[y * 4 + x] = pixel;
					}
				}

				EncodeBC2(*dst, &uncompressed[0], method);
				++ dst;
			}
		}
	}

	void EncodeBC3(void* bc3, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method)
	{
		uint32_t const * src = static_cast<uint32_t const *>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			BC3Block* dst = reinterpret_cast<BC3Block*>(static_cast<uint8_t*>(bc3) + y_base / 4 * out_pitch);

			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						uint32_t pixel = 0;
						if ((x_base + x < width) && (y_base + y < height))
						{
							pixel = src[(y_base + y) * in_pitch / 4 + (x_base + x)];
						}
						uncompressed[y * 4 + x] = pixel;
					}
				}

				EncodeBC3(*dst, &uncompressed[0], method);
				++ dst;
			}
		}
	}

	void EncodeBC4(void* bc4, uint32_t out_pitch, void const * r, uint32_t width, uint32_t height, uint32_t in_pitch)
	{
		uint8_t const * src = static_cast<uint8_t const *>(r);

		uint8_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			BC4Block* dst = reinterpret_cast<BC4Block*>(static_cast<uint8_t*>(bc4) + y_base / 4 * out_pitch);

			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						uint8_t pixel = 0;
						if ((x_base + x < width) && (y_base + y < height))
						{
							pixel = src[(y_base + y) * in_pitch + (x_base + x)];
						}
						uncompressed[y * 4 + x] = pixel;
					}
				}

				EncodeBC4(*dst, &uncompressed[0]);
				++ dst;
			}
		}
	}

	void EncodeBC5(void* bc5, uint32_t out_pitch, void const * gr, uint32_t width, uint32_t height, uint32_t in_pitch)
	{
		uint8_t const * src = static_cast<uint8_t const *>(gr);

		uint8_t uncompressed_r[16];
		uint8_t uncompressed_g[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			BC5Block* dst = reinterpret_cast<BC5Block*>(static_cast<uint8_t*>(bc5) + y_base / 4 * out_pitch);

			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				for (int y = 0; y < 4; ++ y)
				{
					for (int x = 0; x < 4; ++ x)
					{
						uint8_t r = 0, g = 0;
						if ((x_base + x < width) && (y_base + y < height))
						{
							r = src[(y_base + y) * in_pitch + (x_base + x) * 2 + 0];
							g = src[(y_base + y) * in_pitch + (x_base + x) * 2 + 1];
						}
						uncompressed_r[y * 4 + x] = r;
						uncompressed_g[y * 4 + x] = g;
					}
				}

				EncodeBC5(*dst, &uncompressed_r[0], &uncompressed_g[0]);
				++ dst;
			}
		}
	}

	void EncodeBC1(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method)
	{
		uint32_t width = src_tex->Width(0);
		uint32_t height = src_tex->Height(0);

		TexturePtr argb8_tex;
		if (src_tex->Format() != EF_ARGB8)
		{
			argb8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_ARGB8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
			src_tex->CopyToTexture(*argb8_tex);
		}
		else
		{
			argb8_tex = src_tex;
		}

		Texture::Mapper mapper_src(*argb8_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
		Texture::Mapper mapper_dst(*bc_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);

		EncodeBC1(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height, mapper_src.RowPitch(), method);
	}

	void EncodeBC2(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method)
	{
		uint32_t width = src_tex->Width(0);
		uint32_t height = src_tex->Height(0);

		TexturePtr argb8_tex;
		if (src_tex->Format() != EF_ARGB8)
		{
			argb8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_ARGB8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
			src_tex->CopyToTexture(*argb8_tex);
		}
		else
		{
			argb8_tex = src_tex;
		}

		Texture::Mapper mapper_src(*argb8_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
		Texture::Mapper mapper_dst(*bc_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);

		EncodeBC2(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height, mapper_src.RowPitch(), method);
	}

	void EncodeBC3(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method)
	{
		uint32_t width = src_tex->Width(0);
		uint32_t height = src_tex->Height(0);

		TexturePtr argb8_tex;
		if (src_tex->Format() != EF_ARGB8)
		{
			argb8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_ARGB8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
			src_tex->CopyToTexture(*argb8_tex);
		}
		else
		{
			argb8_tex = src_tex;
		}

		Texture::Mapper mapper_src(*argb8_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
		Texture::Mapper mapper_dst(*bc_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);

		EncodeBC3(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height, mapper_src.RowPitch(), method);
	}

	void EncodeBC4(TexturePtr const & bc_tex, TexturePtr const & src_tex)
	{
		uint32_t width = src_tex->Width(0);
		uint32_t height = src_tex->Height(0);

		TexturePtr r8_tex;
		if (src_tex->Format() != EF_R8)
		{
			r8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_R8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
			src_tex->CopyToTexture(*r8_tex);
		}
		else
		{
			r8_tex = src_tex;
		}

		Texture::Mapper mapper_src(*r8_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
		Texture::Mapper mapper_dst(*bc_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);

		EncodeBC4(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height, mapper_src.RowPitch());
	}

	void EncodeBC5(TexturePtr const & bc_tex, TexturePtr const & src_tex)
	{
		uint32_t width = src_tex->Width(0);
		uint32_t height = src_tex->Height(0);

		TexturePtr gr8_tex;
		if (src_tex->Format() != EF_GR8)
		{
			gr8_tex = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1, EF_GR8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, nullptr);
			src_tex->CopyToTexture(*gr8_tex);
		}
		else
		{
			gr8_tex = src_tex;
		}

		Texture::Mapper mapper_src(*gr8_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
		Texture::Mapper mapper_dst(*bc_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);

		EncodeBC5(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height, mapper_src.RowPitch());
	}

	void BC4ToBC1G(BC1Block& bc1, BC4Block const & bc4)
	{
		bc1.clr_0 = (bc4.alpha_0 >> 2) << 5;
		bc1.clr_1 = (bc4.alpha_1 >> 2) << 5;
		bool swap_clr = false;
		if (bc4.alpha_0 < bc4.alpha_1)
		{
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
