// BlockCompression.hpp
// KlayGE 纹理分块压缩 头文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2008-2011
// Homepage: http://www.klayge.org
//
// 3.10.0
// 增加了DecodeBC1/3/4 (2010.1.27)
//
// 3.8.0
// 初次建立 (2008.12.9)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _BLOCKCOMPRESSION_HPP
#define _BLOCKCOMPRESSION_HPP

#pragma once

namespace KlayGE
{
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(push, 1)
#endif
	struct BC1_layout
	{
		uint16_t clr_0, clr_1;
		uint16_t bitmap[2];
	};

	struct BC2_layout
	{
		uint16_t alpha[4];
		BC1_layout bc1;
	};

	struct BC4_layout
	{
		uint8_t alpha_0, alpha_1;
		uint8_t bitmap[6];
	};

	struct BC3_layout
	{
		BC4_layout alpha;
		BC1_layout bc1;
	};

	struct BC5_layout
	{
		BC4_layout red;
		BC4_layout green;
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

	KLAYGE_CORE_API void DecodeBC1(uint32_t* argb, uint8_t const * bc1);
	KLAYGE_CORE_API void DecodeBC2(uint32_t* argb, uint8_t const * bc2);
	KLAYGE_CORE_API void DecodeBC3(uint32_t* argb, uint8_t const * bc3);
	KLAYGE_CORE_API void DecodeBC4(uint8_t* r, uint8_t const * bc4);
	KLAYGE_CORE_API void DecodeBC5(uint8_t* r, uint8_t* g, uint8_t const * bc5);

	KLAYGE_CORE_API void DecodeBC1_sRGB(uint32_t* argb, uint8_t const * bc1);
	KLAYGE_CORE_API void DecodeBC2_sRGB(uint32_t* argb, uint8_t const * bc2);
	KLAYGE_CORE_API void DecodeBC3_sRGB(uint32_t* argb, uint8_t const * bc3);
	KLAYGE_CORE_API void DecodeBC4_sRGB(uint8_t* r, uint8_t const * bc4);
	KLAYGE_CORE_API void DecodeBC5_sRGB(uint8_t* r, uint8_t* g, uint8_t const * bc5);

	KLAYGE_CORE_API void DecodeBC1(void* argb, uint32_t pitch, void const * bc1, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC2(void* argb, uint32_t pitch, void const * bc2, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC3(void* argb, uint32_t pitch, void const * bc3, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC4(void* r, uint32_t pitch, void const * bc4, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC5(void* gr, uint32_t pitch, void const * bc5, uint32_t width, uint32_t height);

	KLAYGE_CORE_API void DecodeBC1_sRGB(void* argb, uint32_t pitch, void const * bc1, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC2_sRGB(void* argb, uint32_t pitch, void const * bc2, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC3_sRGB(void* argb, uint32_t pitch, void const * bc3, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC4_sRGB(void* r, uint32_t pitch, void const * bc4, uint32_t width, uint32_t height);
	KLAYGE_CORE_API void DecodeBC5_sRGB(void* gr, uint32_t pitch, void const * bc5, uint32_t width, uint32_t height);

	KLAYGE_CORE_API void DecodeBC1(TexturePtr const & dst_tex, TexturePtr const & bc_tex);
	KLAYGE_CORE_API void DecodeBC2(TexturePtr const & dst_tex, TexturePtr const & bc_tex);
	KLAYGE_CORE_API void DecodeBC3(TexturePtr const & dst_tex, TexturePtr const & bc_tex);
	KLAYGE_CORE_API void DecodeBC4(TexturePtr const & dst_tex, TexturePtr const & bc_tex);
	KLAYGE_CORE_API void DecodeBC5(TexturePtr const & dst_tex, TexturePtr const & bc_tex);


	KLAYGE_CORE_API void EncodeBC1(BC1_layout& bc1, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC2(BC2_layout& bc2, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC3(BC3_layout& bc3, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC4(BC4_layout& bc4, uint8_t const * r);
	KLAYGE_CORE_API void EncodeBC5(BC5_layout& bc5, uint8_t const * r, uint8_t const * g);

	KLAYGE_CORE_API void EncodeBC1_sRGB(BC1_layout& bc1, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC2_sRGB(BC2_layout& bc2, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC3_sRGB(BC3_layout& bc3, uint32_t const * argb, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC4_sRGB(BC4_layout& bc4, uint8_t const * r);
	KLAYGE_CORE_API void EncodeBC5_sRGB(BC5_layout& bc5, uint8_t const * r, uint8_t const * g);

	KLAYGE_CORE_API void EncodeBC1(void* bc1, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC2(void* bc2, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC3(void* bc3, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC4(void* bc4, uint32_t out_pitch, void const * r, uint32_t width, uint32_t height, uint32_t in_pitch);
	KLAYGE_CORE_API void EncodeBC5(void* bc5, uint32_t out_pitch, void const * gr, uint32_t width, uint32_t height, uint32_t in_pitch);

	KLAYGE_CORE_API void EncodeBC1_sRGB(void* bc1, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC2_sRGB(void* bc2, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC3_sRGB(void* bc3, uint32_t out_pitch, void const * argb, uint32_t width, uint32_t height, uint32_t in_pitch, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC4_sRGB(void* bc4, uint32_t out_pitch, void const * r, uint32_t width, uint32_t height, uint32_t in_pitch);
	KLAYGE_CORE_API void EncodeBC5_sRGB(void* bc5, uint32_t out_pitch, void const * gr, uint32_t width, uint32_t height, uint32_t in_pitch);

	KLAYGE_CORE_API void EncodeBC1(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC2(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC3(TexturePtr const & bc_tex, TexturePtr const & src_tex, EBCMethod method);
	KLAYGE_CORE_API void EncodeBC4(TexturePtr const & bc_tex, TexturePtr const & src_tex);
	KLAYGE_CORE_API void EncodeBC5(TexturePtr const & bc_tex, TexturePtr const & src_tex);

	KLAYGE_CORE_API void BC4ToBC1G(BC1_layout& bc1, BC4_layout const & bc4);
}

#endif		// _BLOCKCOMPRESSION_HPP
