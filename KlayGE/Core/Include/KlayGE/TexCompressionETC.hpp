/**
* @file TexCompressionETC.hpp
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

#ifndef _TEXCOMPRESSIONETC_HPP
#define _TEXCOMPRESSIONETC_HPP

#pragma once

namespace KlayGE
{
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(push, 1)
#endif
	struct ETC1Block
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t cw_diff_flip;
		uint16_t msb;
		uint16_t lsb;
	};

	struct ETC2TModeBlock
	{
		uint8_t r1;
		uint8_t g1_b1;
		uint8_t r2_g2;
		uint8_t b2_d;
		uint16_t msb;
		uint16_t lsb;
	};

	struct ETC2HModeBlock
	{
		uint8_t r1_g1;
		uint8_t g1_b1;
		uint8_t b1_r2_g2;
		uint8_t g2_b2_d;
		uint16_t msb;
		uint16_t lsb;
	};

	struct ETC2PlanarModeBlock
	{
		uint8_t ro_go;
		uint8_t go_bo;
		uint8_t bo;
		uint8_t bo_rh;
		uint8_t gh_bh;
		uint8_t bh_rv;
		uint8_t rv_gv;
		uint8_t gv_bv;
	};

	union ETC2Block
	{
		ETC1Block etc1;
		ETC2TModeBlock etc2_t_mode;
		ETC2HModeBlock etc2_h_mode;
		ETC2PlanarModeBlock etc2_planar_mode;
	};
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(pop)
#endif

	KLAYGE_CORE_API void DecodeETC1(uint32_t* argb, void const * etc1);
	KLAYGE_CORE_API void DecodeRGB8ETC2(uint32_t* argb, void const * etc2);
	KLAYGE_CORE_API void DecodeRGB8A1ETC2(uint32_t* argb, void const * etc2);

	KLAYGE_CORE_API void DecodeETC1(void* argb, uint32_t pitch, void const * etc1, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeRGB8ETC2(void* argb, uint32_t pitch, void const * etc2, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeRGB8A1ETC2(void* argb, uint32_t pitch, void const * etc2, uint32_t width, uint32_t height);

	KLAYGE_CORE_API void DecodeETC1(TexturePtr const & dst_tex, TexturePtr const & etc_tex);
	KLAYGE_CORE_API void DecodeRGB8ETC2(TexturePtr const & dst_tex, TexturePtr const & etc_tex);
	KLAYGE_CORE_API void DecodeRGB8A1ETC2(TexturePtr const & dst_tex, TexturePtr const & etc_tex);
}

#endif		// _TEXCOMPRESSIONETC_HPP
