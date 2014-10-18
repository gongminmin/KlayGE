/**
* @file TexCompressionBC.hpp
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

#ifndef _TEXCOMPRESSIONBC_HPP
#define _TEXCOMPRESSIONBC_HPP

#pragma once

#include <KlayGE/TexCompression.hpp>

namespace KlayGE
{
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
	struct BC1Block
	{
		uint16_t clr_0, clr_1;
		uint16_t bitmap[2];
	};

	struct BC2Block
	{
		uint16_t alpha[4];
		BC1Block bc1;
	};

	struct BC4Block
	{
		uint8_t alpha_0, alpha_1;
		uint8_t bitmap[6];
	};

	struct BC3Block
	{
		BC4Block alpha;
		BC1Block bc1;
	};

	struct BC5Block
	{
		BC4Block red;
		BC4Block green;
	};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

	class KLAYGE_CORE_API TexCompressionBC1 : public TexCompression
	{
	public:
		TexCompressionBC1();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) KLAYGE_OVERRIDE;
		virtual void DecodeBlock(void* output, void const * input) KLAYGE_OVERRIDE;

		void EncodeBC1Internal(BC1Block& bc1, uint32_t const * argb, bool alpha, TexCompressionMethod method) const;

	private:
		void PrepareOptTable(uint8_t* table, uint8_t const * expand, int size) const;
		uint32_t RGB565To888(uint16_t rgb) const;
		uint16_t RGB888To565(uint32_t rgb) const;
		uint32_t MatchColorsBlock(uint32_t const * argb, uint32_t min_clr, uint32_t max_clr, bool alpha) const;
		void OptimizeColorsBlock(uint32_t const * argb, uint32_t& min_clr, uint32_t& max_clr, TexCompressionMethod method) const;
		bool RefineBlock(uint32_t const * argb, uint32_t& min_clr, uint32_t& max_clr, uint32_t mask) const;

	private:
		static uint8_t expand5_[32];
		static uint8_t expand6_[64];
		static uint8_t o_match5_[256][2];
		static uint8_t o_match6_[256][2];
		static uint8_t quant_rb_tab_[256 + 16];
		static uint8_t quant_g_tab_[256 + 16];
		static bool lut_inited_;
	};

	class KLAYGE_CORE_API TexCompressionBC2 : public TexCompression
	{
	public:
		TexCompressionBC2();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) KLAYGE_OVERRIDE;
		virtual void DecodeBlock(void* output, void const * input) KLAYGE_OVERRIDE;

	private:
		TexCompressionBC1Ptr bc1_codec_;
	};

	class KLAYGE_CORE_API TexCompressionBC3 : public TexCompression
	{
	public:
		TexCompressionBC3();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) KLAYGE_OVERRIDE;
		virtual void DecodeBlock(void* output, void const * input) KLAYGE_OVERRIDE;

	private:
		TexCompressionBC1Ptr bc1_codec_;
		TexCompressionBC4Ptr bc4_codec_;
	};

	class KLAYGE_CORE_API TexCompressionBC4 : public TexCompression
	{
	public:
		TexCompressionBC4();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) KLAYGE_OVERRIDE;
		virtual void DecodeBlock(void* output, void const * input) KLAYGE_OVERRIDE;
	};

	class KLAYGE_CORE_API TexCompressionBC5 : public TexCompression
	{
	public:
		TexCompressionBC5();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) KLAYGE_OVERRIDE;
		virtual void DecodeBlock(void* output, void const * input) KLAYGE_OVERRIDE;

	private:
		TexCompressionBC4Ptr bc4_codec_;
	};

	KLAYGE_CORE_API void BC4ToBC1G(BC1Block& bc1, BC4Block const & bc4);
}

#endif		// _TEXCOMPRESSIONBC_HPP
