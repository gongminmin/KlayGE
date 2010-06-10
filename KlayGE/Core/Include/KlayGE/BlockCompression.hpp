// BlockCompression.hpp
// KlayGE 纹理分块压缩 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2008-2010
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

#include <vector>
#include <boost/assert.hpp>

namespace KlayGE
{
#ifdef KLAYGE_PLATFORM_WINDOWS
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
#ifdef KLAYGE_PLATFORM_WINDOWS
	#pragma pack(pop)
#endif

	KLAYGE_CORE_API void DecodeBC1(uint32_t* argb, uint8_t const * bc1);
	KLAYGE_CORE_API void DecodeBC2(uint32_t* argb, uint8_t const * bc2);
	KLAYGE_CORE_API void DecodeBC3(uint32_t* argb, uint8_t const * bc3);
	KLAYGE_CORE_API void DecodeBC4(uint8_t* alpha_block, uint8_t const * bc4);
	KLAYGE_CORE_API void DecodeBC5(uint32_t* argb, uint8_t const * bc5);

	KLAYGE_CORE_API void EncodeBC4(BC4_layout& bc4, uint8_t const * alpha);

	KLAYGE_CORE_API void BC4ToBC1G(BC1_layout& bc1, BC4_layout const & bc4);
}

#endif		// _BLOCKCOMPRESSION_HPP
