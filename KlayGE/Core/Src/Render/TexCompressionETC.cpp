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
	void DecodeETCIndividualModeInternal(uint32_t* argb, ETC1Block const & etc1)
	{
		// GLES 3.1 spec Table C.6
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

	template <bool ALPHA>
	void DecodeETCDifferentialModeInternal(uint32_t* argb, ETC1Block const & etc1)
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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
				if (ALPHA)
				{
					modifier = (mod & 0x1) ? modifier_table[cw][1] * ((mod & 0x2) ? -1 : 1) : 0;
				}
				else
				{
					modifier = modifier_table[cw][mod & 0x1] * ((mod & 0x2) ? -1 : 1);
				}
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
				if (ALPHA && (2 == pixel_index))
				{
					argb[y * 4 + x] = 0;
				}
				else
				{
					argb[y * 4 + x] = modified_clr[sub_block][pixel_index];
				}
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
			}
		}
	}

	void DecodeETC1Internal(uint32_t* argb, ETC1Block const & etc1)
	{
		if (etc1.cw_diff_flip & 0x2)
		{
			DecodeETCDifferentialModeInternal<false>(argb, etc1);
		}
		else
		{
			DecodeETCIndividualModeInternal(argb, etc1);
		}
	}

	template <bool ALPHA>
	void DecodeETCTModeInternal(uint32_t* argb, ETC2TModeBlock const & etc2)
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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
				if (ALPHA && (2 == pixel_index))
				{
					argb[y * 4 + x] = 0;
				}
				else
				{
					argb[y * 4 + x] = modified_clr[pixel_index];
				}
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
			}
		}
	}

	template <bool ALPHA>
	void DecodeETCHModeInternal(uint32_t* argb, ETC2HModeBlock const & etc2)
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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
				if (ALPHA && (2 == pixel_index))
				{
					argb[y * 4 + x] = 0;
				}
				else
				{
					argb[y * 4 + x] = modified_clr[pixel_index];
				}
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
			}
		}
	}

	void DecodeETCPlanarModeInternal(uint32_t* argb, ETC2PlanarModeBlock const & etc2)
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

	void DecodeRGB8ETC2Internal(uint32_t* argb, ETC2Block const & etc2)
	{
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
				DecodeETCTModeInternal<false>(argb, etc2.etc2_t_mode);
			}
			else if (g & 0xFFE0)
			{
				DecodeETCHModeInternal<false>(argb, etc2.etc2_h_mode);
			}
			else if (b & 0xFFE0)
			{
				DecodeETCPlanarModeInternal(argb, etc2.etc2_planar_mode);
			}
			else
			{
				DecodeETCDifferentialModeInternal<false>(argb, etc2.etc1);
			}
		}
		else
		{
			DecodeETCIndividualModeInternal(argb, etc2.etc1);
		}
	}

	void DecodeRGB8A1ETC2Internal(uint32_t* argb, ETC2Block const & etc2)
	{
		int const dr = etc2.etc1.r & 0x7;
		int const r = (etc2.etc1.r >> 3) - (dr & 0x4) + (dr & 0x3);
		int const dg = etc2.etc1.g & 0x7;
		int const g = (etc2.etc1.g >> 3) - (dg & 0x4) + (dg & 0x3);
		int const db = etc2.etc1.b & 0x7;
		int const b = (etc2.etc1.b >> 3) - (db & 0x4) + (db & 0x3);
		int op = etc2.etc1.cw_diff_flip & 0x2;

		if (r & 0xFFE0)
		{
			if (op)
			{
				DecodeETCTModeInternal<false>(argb, etc2.etc2_t_mode);
			}
			else
			{
				DecodeETCTModeInternal<true>(argb, etc2.etc2_t_mode);
			}
		}
		else if (g & 0xFFE0)
		{
			if (op)
			{
				DecodeETCHModeInternal<false>(argb, etc2.etc2_h_mode);
			}
			else
			{
				DecodeETCHModeInternal<true>(argb, etc2.etc2_h_mode);
			}
		}
		else if (b & 0xFFE0)
		{
			DecodeETCPlanarModeInternal(argb, etc2.etc2_planar_mode);
		}
		else
		{
			if (op)
			{
				DecodeETCDifferentialModeInternal<false>(argb, etc2.etc1);
			}
			else
			{
				DecodeETCDifferentialModeInternal<true>(argb, etc2.etc1);
			}
		}
	}

	void DecodeETC1(uint32_t* argb, void const * etc1)
	{
		DecodeETC1Internal(argb, *static_cast<ETC1Block const *>(etc1));
	}

	void DecodeRGB8ETC2(uint32_t* argb, void const * etc2)
	{
		DecodeRGB8ETC2Internal(argb, *static_cast<ETC2Block const *>(etc2));
	}

	void DecodeRGB8A1ETC2(uint32_t* argb, void const * etc2)
	{
		DecodeRGB8A1ETC2Internal(argb, *static_cast<ETC2Block const *>(etc2));
	}

	void DecodeETC1(void* argb, uint32_t pitch, void const * etc1, uint32_t width, uint32_t height)
	{
		ETC1Block const * src = static_cast<ETC1Block const *>(etc1);
		uint32_t * dst = static_cast<uint32_t*>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			uint32_t const block_h = std::min(4U, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t const block_w = std::min(4U, width - x_base);

				DecodeETC1(&uncompressed[0], src);
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

	void DecodeRGB8ETC2(void* argb, uint32_t pitch, void const * etc2, uint32_t width, uint32_t height)
	{
		ETC2Block const * src = static_cast<ETC2Block const *>(etc2);
		uint32_t * dst = static_cast<uint32_t*>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			uint32_t const block_h = std::min(4U, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t const block_w = std::min(4U, width - x_base);

				DecodeRGB8ETC2(&uncompressed[0], src);
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

	void DecodeRGB8A1ETC2(void* argb, uint32_t pitch, void const * etc2, uint32_t width, uint32_t height)
	{
		ETC2Block const * src = static_cast<ETC2Block const *>(etc2);
		uint32_t * dst = static_cast<uint32_t*>(argb);

		uint32_t uncompressed[16];
		for (uint32_t y_base = 0; y_base < height; y_base += 4)
		{
			uint32_t const block_h = std::min(4U, height - y_base);
			for (uint32_t x_base = 0; x_base < width; x_base += 4)
			{
				uint32_t const block_w = std::min(4U, width - x_base);

				DecodeRGB8A1ETC2(&uncompressed[0], src);
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

	void DecodeETC1(TexturePtr const & dst_tex, TexturePtr const & etc_tex)
	{
		uint32_t width = etc_tex->Width(0);
		uint32_t height = etc_tex->Height(0);

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
			Texture::Mapper mapper_src(*etc_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*argb8_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			DecodeETC1(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height);
		}

		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex->CopyToTexture(*dst_tex);
		}
	}

	void DecodeRGB8ETC2(TexturePtr const & dst_tex, TexturePtr const & etc_tex)
	{
		uint32_t width = etc_tex->Width(0);
		uint32_t height = etc_tex->Height(0);

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
			Texture::Mapper mapper_src(*etc_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*argb8_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			DecodeRGB8ETC2(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height);
		}

		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex->CopyToTexture(*dst_tex);
		}
	}

	void DecodeRGB8A1ETC2(TexturePtr const & dst_tex, TexturePtr const & etc_tex)
	{
		uint32_t width = etc_tex->Width(0);
		uint32_t height = etc_tex->Height(0);

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
			Texture::Mapper mapper_src(*etc_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
			Texture::Mapper mapper_dst(*argb8_tex, 0, 0, TMA_Write_Only, 0, 0, width, height);
			DecodeRGB8A1ETC2(mapper_dst.Pointer<void>(), mapper_dst.RowPitch(), mapper_src.Pointer<void>(), width, height);
		}

		if (dst_tex->Format() != EF_ARGB8)
		{
			argb8_tex->CopyToTexture(*dst_tex);
		}
	}
}
