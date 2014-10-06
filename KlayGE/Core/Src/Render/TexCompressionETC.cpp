/**
* @file TexCompressionETC.cpp
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

#include <KlayGE/TexCompressionETC.hpp>

namespace
{
	using namespace KlayGE;

	uint32_t ARGB(int a, int r, int g, int b)
	{
		return (MathLib::clamp(b, 0, 255) << 0)
			| (MathLib::clamp(g, 0, 255) << 8)
			| (MathLib::clamp(r, 0, 255) << 16)
			| (MathLib::clamp(a, 0, 255) << 24);
	}

	uint8_t Extend4To8Bits(int input)
	{
		return static_cast<uint8_t>(input | (input << 4));
	}

	uint8_t Extend5To8Bits(int input)
	{
		return static_cast<uint8_t>((input >> 2) | (input << 3));
	}

	uint8_t Extend6To8Bits(int input)
	{
		return static_cast<uint8_t>((input >> 4) | (input << 2));
	}

	uint8_t Extend7To8Bits(int input)
	{
		return static_cast<uint8_t>((input >> 6) | (input << 1));
	}
}

namespace KlayGE
{
	TexCompressionETC1::TexCompressionETC1()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_ETC1) * 4;
		decoded_fmt_ = EF_ARGB8;
	}

	void TexCompressionETC1::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		UNREF_PARAM(output);
		UNREF_PARAM(input);
		UNREF_PARAM(method);

		// TODO
	}

	void TexCompressionETC1::DecodeBlock(void* output, void const * input)
	{
		uint32_t* argb = static_cast<uint32_t*>(output);
		ETC1Block const & etc1 = *static_cast<ETC1Block const *>(input);

		if (etc1.cw_diff_flip & 0x2)
		{
			this->DecodeETCDifferentialModeInternal(argb, etc1, false);
		}
		else
		{
			this->DecodeETCIndividualModeInternal(argb, etc1);
		}
	}

	void TexCompressionETC1::DecodeETCIndividualModeInternal(uint32_t* argb, ETC1Block const & etc1)
	{
		static int const modifier_table[8][2] =
		{
			{ 2, 8 },
			{ 5, 17 },
			{ 9, 29 },
			{ 13, 42 },
			{ 18, 60 },
			{ 24, 80 },
			{ 33, 106 },
			{ 47, 183 }
		};

		int r1 = etc1.r >> 4;
		int r2 = etc1.r & 0xF;
		int g1 = etc1.g >> 4;
		int g2 = etc1.g & 0xF;
		int b1 = etc1.b >> 4;
		int b2 = etc1.b & 0xF;

		uint8_t base_clr[2][3];
		base_clr[0][0] = Extend4To8Bits(r1);
		base_clr[0][1] = Extend4To8Bits(g1);
		base_clr[0][2] = Extend4To8Bits(b1);
		base_clr[1][0] = Extend4To8Bits(r2);
		base_clr[1][1] = Extend4To8Bits(g2);
		base_clr[1][2] = Extend4To8Bits(b2);

		uint32_t modified_clr[2][4];
		for (int sub = 0; sub < 2; ++ sub)
		{
			int const cw = (etc1.cw_diff_flip >> (2 + (!sub * 3))) & 0x7;
			for (int mod = 0; mod < 4; ++ mod)
			{
				int const modifier = modifier_table[cw][mod & 0x1] * ((mod & 0x2) ? -1 : 1);
				modified_clr[sub][mod] = ARGB(255, base_clr[sub][0] + modifier,
					base_clr[sub][1] + modifier, base_clr[sub][2] + modifier);
			}
		}

		bool const flip = etc1.cw_diff_flip & 0x1;
		for (int x = 0; x < 4; ++ x)
		{
			for (int y = 0; y < 4; ++ y)
			{
				int sub_block = ((flip ? y : x) >> 1);
				int bit_index = (x * 4 + y) ^ 0x8;
				int msb = (etc1.msb >> bit_index) & 0x1;
				int lsb = (etc1.lsb >> bit_index) & 0x1;
				int pixel_index = msb * 2 + lsb;
				argb[y * 4 + x] = modified_clr[sub_block][pixel_index];
			}
		}
	}

	void TexCompressionETC1::DecodeETCDifferentialModeInternal(uint32_t* argb, ETC1Block const & etc1, bool alpha)
	{
		static int const modifier_table[8][2] =
		{
			{ 2, 8 },
			{ 5, 17 },
			{ 9, 29 },
			{ 13, 42 },
			{ 18, 60 },
			{ 24, 80 },
			{ 33, 106 },
			{ 47, 183 }
		};

		int const r = etc1.r >> 3;
		int const dr = etc1.r & 0x7;
		int const g = etc1.g >> 3;
		int const dg = etc1.g & 0x7;
		int const b = etc1.b >> 3;
		int const db = etc1.b & 0x7;

		uint8_t base_clr[2][3];
		base_clr[0][0] = Extend5To8Bits(r);
		base_clr[0][1] = Extend5To8Bits(g);
		base_clr[0][2] = Extend5To8Bits(b);
		base_clr[1][0] = Extend5To8Bits(r - (dr & 0x4) + (dr & 0x3));
		base_clr[1][1] = Extend5To8Bits(g - (dg & 0x4) + (dg & 0x3));
		base_clr[1][2] = Extend5To8Bits(b - (db & 0x4) + (db & 0x3));

		uint32_t modified_clr[2][4];
		for (int sub = 0; sub < 2; ++ sub)
		{
			int const cw = (etc1.cw_diff_flip >> (2 + (!sub * 3))) & 0x7;
			for (int mod = 0; mod < 4; ++ mod)
			{
				int modifier;
				if (alpha)
				{
					modifier = (mod & 0x1) ? modifier_table[cw][1] * ((mod & 0x2) ? -1 : 1) : 0;
				}
				else
				{
					modifier = modifier_table[cw][mod & 0x1] * ((mod & 0x2) ? -1 : 1);
				}
				modified_clr[sub][mod] = ARGB(255, base_clr[sub][0] + modifier,
					base_clr[sub][1] + modifier, base_clr[sub][2] + modifier);
			}
		}

		bool const flip = etc1.cw_diff_flip & 0x1;
		for (int x = 0; x < 4; ++ x)
		{
			for (int y = 0; y < 4; ++ y)
			{
				int sub_block = ((flip ? y : x) >> 1);
				int bit_index = (x * 4 + y) ^ 0x8;
				int msb = (etc1.msb >> bit_index) & 0x1;
				int lsb = (etc1.lsb >> bit_index) & 0x1;
				int pixel_index = msb * 2 + lsb;
				if (alpha && (2 == pixel_index))
				{
					argb[y * 4 + x] = 0;
				}
				else
				{
					argb[y * 4 + x] = modified_clr[sub_block][pixel_index];
				}
			}
		}
	}


	TexCompressionETC2RGB8::TexCompressionETC2RGB8()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_ETC2_BGR8) * 4;
		decoded_fmt_ = EF_ARGB8;

		etc1_codec_ = MakeSharedPtr<TexCompressionETC1>();
	}

	void TexCompressionETC2RGB8::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		UNREF_PARAM(output);
		UNREF_PARAM(input);
		UNREF_PARAM(method);

		// TODO
	}

	void TexCompressionETC2RGB8::DecodeBlock(void* output, void const * input)
	{
		uint32_t* argb = static_cast<uint32_t*>(output);
		ETC2Block const & etc2 = *static_cast<ETC2Block const *>(input);

		if (etc2.etc1.cw_diff_flip & 0x2)
		{
			int const dr = etc2.etc1.r & 0x7;
			int const r = (etc2.etc1.r >> 3) - (dr & 0x4) + (dr & 0x3);
			int const dg = etc2.etc1.g & 0x7;
			int const g = (etc2.etc1.g >> 3) - (dg & 0x4) + (dg & 0x3);
			int const db = etc2.etc1.b & 0x7;
			int const b = (etc2.etc1.b >> 3) - (db & 0x4) + (db & 0x3);

			if (r & 0xFFE0)
			{
				this->DecodeETCTModeInternal(argb, etc2.etc2_t_mode, false);
			}
			else if (g & 0xFFE0)
			{
				this->DecodeETCHModeInternal(argb, etc2.etc2_h_mode, false);
			}
			else if (b & 0xFFE0)
			{
				this->DecodeETCPlanarModeInternal(argb, etc2.etc2_planar_mode);
			}
			else
			{
				etc1_codec_->DecodeETCDifferentialModeInternal(argb, etc2.etc1, false);
			}
		}
		else
		{
			etc1_codec_->DecodeETCIndividualModeInternal(argb, etc2.etc1);
		}
	}

	void TexCompressionETC2RGB8::DecodeETCTModeInternal(uint32_t* argb, ETC2TModeBlock const & etc2, bool alpha)
	{
		static int const distance_table[8] = { 3, 6, 11, 16, 23, 32, 41, 64 };

		int const r1 = ((etc2.r1 >> 1) & 0xC) | (etc2.r1 & 0x3);
		int const g1 = etc2.g1_b1 >> 4;
		int const b1 = etc2.g1_b1 & 0xF;
		int const r2 = etc2.r2_g2 >> 4;
		int const g2 = etc2.r2_g2 & 0xF;
		int const b2 = etc2.b2_d >> 4;
		int const da = (etc2.b2_d & 0xC) >> 1;
		int const db = etc2.b2_d & 0x1;

		int const base_clr1[] =
		{
			Extend4To8Bits(r1),
			Extend4To8Bits(g1),
			Extend4To8Bits(b1)
		};

		int const base_clr2[] =
		{
			Extend4To8Bits(r2),
			Extend4To8Bits(g2),
			Extend4To8Bits(b2)
		};

		int const distance = distance_table[da | db];
		uint32_t const modified_clr[] =
		{
			ARGB(255, base_clr1[0], base_clr1[1], base_clr1[2]),
			ARGB(255, base_clr2[0] + distance, base_clr2[1] + distance, base_clr2[2] + distance),
			ARGB(255, base_clr2[0], base_clr2[1], base_clr2[2]),
			ARGB(255, base_clr2[0] - distance, base_clr2[1] - distance, base_clr2[2] - distance)
		};

		for (int x = 0; x < 4; ++ x)
		{
			for (int y = 0; y < 4; ++ y)
			{
				int bit_index = (x * 4 + y) ^ 0x8;
				int msb = (etc2.msb >> bit_index) & 0x1;
				int lsb = (etc2.lsb >> bit_index) & 0x1;
				int pixel_index = msb * 2 + lsb;
				if (alpha && (2 == pixel_index))
				{
					argb[y * 4 + x] = 0;
				}
				else
				{
					argb[y * 4 + x] = modified_clr[pixel_index];
				}
			}
		}
	}

	void TexCompressionETC2RGB8::DecodeETCHModeInternal(uint32_t* argb, ETC2HModeBlock const & etc2, bool alpha)
	{
		static int const distance_table[8] = { 3, 6, 11, 16, 23, 32, 41, 64 };

		int const r1 = (etc2.r1_g1 >> 3) & 0xF;
		int const g1 = ((etc2.r1_g1 & 0x7) << 1) | ((etc2.g1_b1 >> 4) & 0x1);
		int const b1 = (etc2.g1_b1 & 0x8) | ((etc2.g1_b1 & 0x3) << 1) | ((etc2.b1_r2_g2 >> 7) & 0x1);
		int const r2 = (etc2.b1_r2_g2 >> 3) & 0xF;
		int const g2 = ((etc2.b1_r2_g2 & 0x7) << 1) | ((etc2.g2_b2_d >> 7) & 0x1);
		int const b2 = (etc2.g2_b2_d >> 3) & 0xF;
		int const da = etc2.g2_b2_d & 0x04;
		int const db = etc2.g2_b2_d & 0x01;

		int const base_clr1[] =
		{
			Extend4To8Bits(r1),
			Extend4To8Bits(g1),
			Extend4To8Bits(b1)
		};

		int const base_clr2[] =
		{
			Extend4To8Bits(r2),
			Extend4To8Bits(g2),
			Extend4To8Bits(b2)
		};

		int const ordering = ARGB(0, base_clr1[0], base_clr1[1], base_clr1[2])
			>= ARGB(0, base_clr2[0], base_clr2[1], base_clr2[2]);
		int distance = distance_table[da | (db << 1) | ordering];
		uint32_t const modified_clr[] =
		{
			ARGB(255, base_clr1[0] + distance, base_clr1[1] + distance, base_clr1[2] + distance),
			ARGB(255, base_clr1[0] - distance, base_clr1[1] - distance, base_clr1[2] - distance),
			ARGB(255, base_clr2[0] + distance, base_clr2[1] + distance, base_clr2[2] + distance),
			ARGB(255, base_clr2[0] - distance, base_clr2[1] - distance, base_clr2[2] - distance)
		};

		for (int x = 0; x < 4; ++ x)
		{
			for (int y = 0; y < 4; ++ y)
			{
				int bit_index = (x * 4 + y) ^ 0x8;
				int msb = (etc2.msb >> bit_index) & 0x1;
				int lsb = (etc2.lsb >> bit_index) & 0x1;
				int pixel_index = msb * 2 + lsb;
				if (alpha && (2 == pixel_index))
				{
					argb[y * 4 + x] = 0;
				}
				else
				{
					argb[y * 4 + x] = modified_clr[pixel_index];
				}
			}
		}
	}

	void TexCompressionETC2RGB8::DecodeETCPlanarModeInternal(uint32_t* argb, ETC2PlanarModeBlock const & etc2)
	{
		int const ro = (etc2.ro_go >> 1) & 0x3F;
		int const go = ((etc2.ro_go & 0x1) << 6) | ((etc2.go_bo >> 1) & 0x3F);
		int const bo = ((etc2.go_bo & 0x1) << 5) | (etc2.bo & 0x18) | ((etc2.bo & 0x3) << 1) | ((etc2.bo_rh >> 7) & 0x1);
		int const rh = (((etc2.bo_rh >> 2) & 0x1F) << 1) | (etc2.bo_rh & 0x1);
		int const gh = etc2.gh_bh >> 1;
		int const bh = ((etc2.gh_bh & 0x1) << 5) | ((etc2.bh_rv >> 3) & 0x1F);
		int const rv = ((etc2.bh_rv & 0x7) << 3) | ((etc2.rv_gv >> 5) & 0x7);
		int const gv = ((etc2.rv_gv & 0x1F) << 2) | ((etc2.gv_bv >> 6) & 0x3);
		int const bv = etc2.gv_bv & 0x3F;

		int const o[] =
		{
			Extend6To8Bits(ro),
			Extend7To8Bits(go),
			Extend6To8Bits(bo)
		};

		int const h[] =
		{
			Extend6To8Bits(rh),
			Extend7To8Bits(gh),
			Extend6To8Bits(bh)
		};

		int const v[] =
		{
			Extend6To8Bits(rv),
			Extend7To8Bits(gv),
			Extend6To8Bits(bv)
		};

		for (int y = 0; y < 4; ++ y)
		{
			for (int x = 0; x < 4; ++ x)
			{
				argb[y * 4 + x] = ARGB(255,
					(x * (h[0] - o[0]) + y * (v[0] - o[0]) + 4 * o[0] + 2) >> 2,
					(x * (h[1] - o[1]) + y * (v[1] - o[1]) + 4 * o[1] + 2) >> 2,
					(x * (h[2] - o[2]) + y * (v[2] - o[2]) + 4 * o[2] + 2) >> 2);
			}
		}
	}


	TexCompressionETC2RGB8A1::TexCompressionETC2RGB8A1()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_ETC2_A1BGR8) * 4;
		decoded_fmt_ = EF_ARGB8;

		etc1_codec_ = MakeSharedPtr<TexCompressionETC1>();
		etc2_rgb8_codec_ = MakeSharedPtr<TexCompressionETC2RGB8>();
	}

	void TexCompressionETC2RGB8A1::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		UNREF_PARAM(output);
		UNREF_PARAM(input);
		UNREF_PARAM(method);

		// TODO
	}

	void TexCompressionETC2RGB8A1::DecodeBlock(void* output, void const * input)
	{
		uint32_t* argb = static_cast<uint32_t*>(output);
		ETC2Block const & etc2 = *static_cast<ETC2Block const *>(input);

		int const dr = etc2.etc1.r & 0x7;
		int const r = (etc2.etc1.r >> 3) - (dr & 0x4) + (dr & 0x3);
		int const dg = etc2.etc1.g & 0x7;
		int const g = (etc2.etc1.g >> 3) - (dg & 0x4) + (dg & 0x3);
		int const db = etc2.etc1.b & 0x7;
		int const b = (etc2.etc1.b >> 3) - (db & 0x4) + (db & 0x3);
		int op = etc2.etc1.cw_diff_flip & 0x2;

		if (r & 0xFFE0)
		{
			etc2_rgb8_codec_->DecodeETCTModeInternal(argb, etc2.etc2_t_mode, !op);
		}
		else if (g & 0xFFE0)
		{
			etc2_rgb8_codec_->DecodeETCHModeInternal(argb, etc2.etc2_h_mode, !op);
		}
		else if (b & 0xFFE0)
		{
			etc2_rgb8_codec_->DecodeETCPlanarModeInternal(argb, etc2.etc2_planar_mode);
		}
		else
		{
			etc1_codec_->DecodeETCDifferentialModeInternal(argb, etc2.etc1, !op);
		}
	}
}
