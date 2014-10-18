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
#include <KFL/Thread.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/TexCompressionBC.hpp>

namespace
{
	KlayGE::mutex singleton_mutex;
}

namespace KlayGE
{
	uint8_t TexCompressionBC1::expand5_[32];
	uint8_t TexCompressionBC1::expand6_[64];
	uint8_t TexCompressionBC1::o_match5_[256][2];
	uint8_t TexCompressionBC1::o_match6_[256][2];
	uint8_t TexCompressionBC1::quant_rb_tab_[256 + 16];
	uint8_t TexCompressionBC1::quant_g_tab_[256 + 16];
	bool TexCompressionBC1::lut_inited_ = false;

	TexCompressionBC1::TexCompressionBC1()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC1) * 4;
		decoded_fmt_ = EF_ARGB8;

		if (!lut_inited_)
		{
			unique_lock<mutex> lock(singleton_mutex);
			if (!lut_inited_)
			{
				for (int i = 0; i < 32; ++ i)
				{
					expand5_[i] = Extend5To8Bits(i);
				}

				for (int i = 0; i < 64; ++ i)
				{
					expand6_[i] = Extend6To8Bits(i);
				}

				for (int i = 0; i < 256 + 16; ++ i)
				{
					int v = MathLib::clamp(i - 8, 0, 255);
					quant_rb_tab_[i] = expand5_[Mul8Bit(v, 31)];
					quant_g_tab_[i] = expand6_[Mul8Bit(v, 63)];
				}

				this->PrepareOptTable(&o_match5_[0][0], expand5_, 32);
				this->PrepareOptTable(&o_match6_[0][0], expand6_, 64);

				lut_inited_ = true;
			}
		}
	}

	void TexCompressionBC1::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BC1Block& bc1 = *static_cast<BC1Block*>(output);
		uint32_t const * argb = static_cast<uint32_t const *>(input);

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

		this->EncodeBC1Internal(bc1, argb, alpha, method);
	}

	void TexCompressionBC1::DecodeBlock(void* output, void const * input)
	{
		uint32_t* argb = static_cast<uint32_t*>(output);
		BC1Block const & bc1 = *static_cast<BC1Block const *>(input);

		uint32_t max_clr = this->RGB565To888(bc1.clr_0);
		uint32_t min_clr = this->RGB565To888(bc1.clr_1);
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

	void TexCompressionBC1::PrepareOptTable(uint8_t* table, uint8_t const * expand, int size) const
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
						table[i * 2 + 0] = static_cast<uint8_t>(max);
						table[i * 2 + 1] = static_cast<uint8_t>(min);
						best_err = err;
					}
				}
			}
		}
	}

	uint32_t TexCompressionBC1::RGB565To888(uint16_t rgb) const
	{
		return ARGB(255, expand5_[(rgb >> 11) & 0x1F], expand6_[(rgb >> 5) & 0x3F],
			expand5_[(rgb >> 0) & 0x1F]);
	}

	uint16_t TexCompressionBC1::RGB888To565(uint32_t rgb) const
	{
		return (((rgb >> 19) & 0x1F) << 11) | (((rgb >> 10) & 0x3F) << 5) | (((rgb >> 3) & 0x1F) << 0);
	}

	// The color matching function
	uint32_t TexCompressionBC1::MatchColorsBlock(uint32_t const * argb, uint32_t min_clr, uint32_t max_clr, bool alpha) const
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
	void TexCompressionBC1::OptimizeColorsBlock(uint32_t const * argb, uint32_t& min_clr, uint32_t& max_clr, TexCompressionMethod method) const
	{
		if (method != TCM_Quality)
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
	bool TexCompressionBC1::RefineBlock(uint32_t const * argb, uint32_t& min_clr, uint32_t& max_clr, uint32_t mask) const
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

			akku += prods[step];
			At1_r += w1 * r;
			At1_g += w1 * g;
			At1_b += w1 * b;
			At2_r += r;
			At2_g += g;
			At2_b += b;
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

		uint16_t old_min = this->RGB888To565(min_clr);
		uint16_t old_max = this->RGB888To565(max_clr);

		// solve.
		int max_r = MathLib::clamp<int>(static_cast<int>((At1_r * yy - At2_r * xy) * frb + 0.5f), 0, 31);
		int max_g = MathLib::clamp<int>(static_cast<int>((At1_g * yy - At2_g * xy) * fg + 0.5f), 0, 63);
		int max_b = MathLib::clamp<int>(static_cast<int>((At1_b * yy - At2_b * xy) * frb + 0.5f), 0, 31);

		int min_r = MathLib::clamp<int>(static_cast<int>((At2_r * xx - At1_r * xy) * frb + 0.5f), 0, 31);
		int min_g = MathLib::clamp<int>(static_cast<int>((At2_g * xx - At1_g * xy) * fg + 0.5f), 0, 63);
		int min_b = MathLib::clamp<int>(static_cast<int>((At2_b * xx - At1_b * xy) * frb + 0.5f), 0, 31);

		uint16_t max16 = static_cast<uint16_t>((max_r << 11) | (max_g << 5) | (max_b << 0));
		uint16_t min16 = static_cast<uint16_t>((min_r << 11) | (min_g << 5) | (min_b << 0));

		if ((old_min != min16) || (old_max != max16))
		{
			min_clr = this->RGB565To888(min16);
			max_clr = this->RGB565To888(max16);

			return true;
		}
		else
		{
			return false;
		}
	}

	void TexCompressionBC1::EncodeBC1Internal(BC1Block& bc1, uint32_t const * argb, bool alpha, TexCompressionMethod method) const
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
			this->OptimizeColorsBlock(argb, min_clr, max_clr, method);
			max16 = this->RGB888To565(max_clr);
			min16 = this->RGB888To565(min_clr);
			if (max16 != min16)
			{
				mask = this->MatchColorsBlock(argb, min_clr, max_clr, alpha);
			}
			else
			{
				mask = 0;
			}
			if (!alpha && (method != TCM_Speed))
			{
				if (this->RefineBlock(argb, min_clr, max_clr, mask))
				{
					max16 = this->RGB888To565(max_clr);
					min16 = this->RGB888To565(min_clr);
					if (max16 != min16)
					{
						mask = this->MatchColorsBlock(argb, min_clr, max_clr, alpha);
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
				int r = GetR(argb[0]) & 0xFF;
				int g = GetG(argb[0]) & 0xFF;
				int b = GetB(argb[0]) & 0xFF;

				mask = 0xAAAAAAAA;
				max16 = (o_match5_[r][0] << 11) | (o_match6_[g][0] << 5) | o_match5_[b][0];
				min16 = (o_match5_[r][1] << 11) | (o_match6_[g][1] << 5) | o_match5_[b][1];
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


	TexCompressionBC2::TexCompressionBC2()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC2) * 4;
		decoded_fmt_ = EF_ARGB8;

		bc1_codec_ = MakeSharedPtr<TexCompressionBC1>();
	}

	void TexCompressionBC2::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BC2Block& bc2 = *static_cast<BC2Block*>(output);
		uint32_t const * argb = static_cast<uint32_t const *>(input);

		array<uint8_t, 16> alpha;
		array<uint32_t, 16> xrgb;
		for (size_t i = 0; i < xrgb.size(); ++ i)
		{
			xrgb[i] = argb[i] | 0xFF000000;
			alpha[i] = static_cast<uint8_t>(argb[i] >> 28);
		}

		bc1_codec_->EncodeBC1Internal(bc2.bc1, &xrgb[0], false, method);
		
		for (int i = 0; i < 4; ++ i)
		{
			bc2.alpha[i] = (alpha[i * 4 + 0] << 0) | (alpha[i * 4 + 1] << 4)
				| (alpha[i * 4 + 2] << 8) | (alpha[i * 4 + 3] << 12);
		}
	}

	void TexCompressionBC2::DecodeBlock(void* output, void const * input)
	{
		uint32_t* argb = static_cast<uint32_t*>(output);
		BC2Block const * bc2_block = static_cast<BC2Block const *>(input);

		bc1_codec_->DecodeBlock(argb, &bc2_block->bc1);

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


	TexCompressionBC3::TexCompressionBC3()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC3) * 4;
		decoded_fmt_ = EF_ARGB8;

		bc1_codec_ = MakeSharedPtr<TexCompressionBC1>();
		bc4_codec_ = MakeSharedPtr<TexCompressionBC4>();
	}

	void TexCompressionBC3::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BC3Block& bc3 = *static_cast<BC3Block*>(output);
		uint32_t const * argb = static_cast<uint32_t const *>(input);

		array<uint8_t, 16> alpha;
		array<uint32_t, 16> xrgb;
		for (size_t i = 0; i < xrgb.size(); ++ i)
		{
			xrgb[i] = argb[i] | 0xFF000000;
			alpha[i] = static_cast<uint8_t>(argb[i] >> 24);
		}

		bc1_codec_->EncodeBC1Internal(bc3.bc1, &xrgb[0], false, method);
		bc4_codec_->EncodeBlock(&bc3.alpha, &alpha[0], method);
	}

	void TexCompressionBC3::DecodeBlock(void* output, void const * input)
	{
		uint32_t* argb = static_cast<uint32_t*>(output);
		BC3Block const * bc3_block = static_cast<BC3Block const *>(input);

		bc1_codec_->DecodeBlock(argb, &bc3_block->bc1);

		array<uint8_t, 16> alpha_block;
		bc4_codec_->DecodeBlock(&alpha_block[0], &bc3_block->alpha);

		for (size_t i = 0; i < alpha_block.size(); ++ i)
		{
			argb[i] &= 0x00FFFFFF;
			argb[i] |= alpha_block[i] << 24;
		}
	}


	TexCompressionBC4::TexCompressionBC4()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC4) * 4;
		decoded_fmt_ = EF_R8;
	}

	// Alpha block compression (this is easy for a change)
	void TexCompressionBC4::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		UNREF_PARAM(method);

		BC4Block& bc4 = *static_cast<BC4Block*>(output);
		uint8_t const * r = static_cast<uint8_t const *>(input);

		// find min/max color
		int min, max;
		min = max = r[0];

		for (int i = 1; i < 16; ++ i)
		{
			min = std::min<int>(min, r[i]);
			max = std::max<int>(max, r[i]);
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
			int a = r[i] * 7 - bias;
			int ind, t;

			// select index (hooray for bit magic)
			t = (dist4 - a) >> 31;  ind = t & 4; a -= dist4 & t;
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

	void TexCompressionBC4::DecodeBlock(void* output, void const * input)
	{
		uint8_t* alpha_block = static_cast<uint8_t*>(output);
		BC4Block const & bc4 = *static_cast<BC4Block const *>(input);

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


	TexCompressionBC5::TexCompressionBC5()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_BC5) * 4;
		decoded_fmt_ = EF_GR8;

		bc4_codec_ = MakeSharedPtr<TexCompressionBC4>();
	}

	void TexCompressionBC5::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BC5Block& bc5 = *static_cast<BC5Block*>(output);
		uint16_t const * gr = static_cast<uint16_t const *>(input);

		array<uint8_t, 16> r;
		array<uint8_t, 16> g;
		for (size_t i = 0; i < r.size(); ++ i)
		{
			r[i] = gr[i] & 0xFF;
			g[i] = gr[i] >> 8;
		}

		bc4_codec_->EncodeBlock(&bc5.red, &r[0], method);
		bc4_codec_->EncodeBlock(&bc5.green, &g[0], method);
	}

	void TexCompressionBC5::DecodeBlock(void* output, void const * input)
	{
		uint16_t* gr = static_cast<uint16_t*>(output);
		BC5Block const * bc5_block = static_cast<BC5Block const *>(input);

		array<uint8_t, 16> r;
		bc4_codec_->DecodeBlock(&r[0], &bc5_block->red);
		array<uint8_t, 16> g;
		bc4_codec_->DecodeBlock(&g[0], &bc5_block->green);

		for (size_t i = 0; i < r.size(); ++ i)
		{
			gr[i] = r[i] | (g[i] << 8);
		}
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
