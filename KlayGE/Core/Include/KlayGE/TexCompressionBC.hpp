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

	enum EBCMethod
	{
		EBCM_Quality,
		EBCM_Balanced,
		EBCM_Speed
	};

	KLAYGE_CORE_API void DecodeBC1(uint32_t* argb, void const * bc1);
	KLAYGE_CORE_API void DecodeBC2(uint32_t* argb, void const * bc2);
	KLAYGE_CORE_API void DecodeBC3(uint32_t* argb, void const * bc3);
	KLAYGE_CORE_API void DecodeBC4(uint8_t* r, void const * bc4);
	KLAYGE_CORE_API void DecodeBC5(uint8_t* r, uint8_t* g, void const * bc5);

	KLAYGE_CORE_API void DecodeBC1(void* argb, uint32_t pitch, void const * bc1, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC2(void* argb, uint32_t pitch, void const * bc2, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC3(void* argb, uint32_t pitch, void const * bc3, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC4(void* r, uint32_t pitch, void const * bc4, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC5(void* gr, uint32_t pitch, void const * bc5, uint32_t width, uint32_t height);

	KLAYGE_CORE_API void DecodeBC1(TexturePtr const & dst_tex, TexturePtr const & bc_tex);
	KLAYGE_CORE_API void DecodeBC2(TexturePtr const & dst_tex, TexturePtr const & bc_tex);
	KLAYGE_CORE_API void DecodeBC3(TexturePtr const & dst_tex, TexturePtr const & bc_tex);
	KLAYGE_CORE_API void DecodeBC4(TexturePtr const & dst_tex, TexturePtr const & bc_tex);
	KLAYGE_CORE_API void DecodeBC5(TexturePtr const & dst_tex, TexturePtr const & bc_tex);


	KLAYGE_CORE_API void EncodeBC1(BC1Block& bc1, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC2(BC2Block& bc2, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC3(BC3Block& bc3, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC4(BC4Block& bc4, uint8_t const * r);
	KLAYGE_CORE_API void EncodeBC5(BC5Block& bc5, uint8_t const * r, uint8_t const * g);

	KLAYGE_CORE_API void EncodeBC1(void* bc1, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC2(void* bc2, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC3(void* bc3, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC4(void* bc4, uint32_t out_pitch, void const * r, uint32_t width, uint32_t height, uint32_t in_pitch);
	KLAYGE_CORE_API void EncodeBC5(void* bc5, uint32_t out_pitch, void const * gr, uint32_t width, uint32_t height, uint32_t in_pitch);

	KLAYGE_CORE_API void EncodeBC1(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC2(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC3(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC4(TexturePtr const & bc_tex, TexturePtr const & src_tex);
	KLAYGE_CORE_API void EncodeBC5(TexturePtr const & bc_tex, TexturePtr const & src_tex);

	KLAYGE_CORE_API void BC4ToBC1G(BC1Block& bc1, BC4Block const & bc4);
}

#endif		// _TEXCOMPRESSIONBC_HPP
