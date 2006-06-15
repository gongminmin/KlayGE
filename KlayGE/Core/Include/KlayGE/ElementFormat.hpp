// ElementFormat.hpp
// KlayGE 元素格式 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.6.8)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _ELEMENTFORMAT_HPP
#define _ELEMENTFORMAT_HPP

#include <boost/assert.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	enum ElementFormat
	{
		// Unknown element format.
		EF_Unknown,

		// 8-bit element format, all bits alpha.
		EF_A8,
		// 8-bit element format, 4 bits alpha, 4 bits luminace.
		EF_AL4,
		// 16-bit element format, 8 bits alpha, 8 bits luminace.
		EF_AL8,
		// 32-bit element format, 16 bits alpha, 16 bits luminace.
		EF_AL16,

		// 8-bit element format, all bits luminace.
		EF_L8,
		// 16-bit element format, all bits for luminace.
		EF_L16,

		// 16-bit element format, 5 bits red, 6 bits green, 5 bits blue.
		EF_R5G6B5,
		// 16-bit element format, 4 bits for alpha, red, green and blue.
		EF_ARGB4,
		// 24-bit element format, 8 bits for red, green and blue.
		EF_RGB8,
		// 32-bit element format, 8 bits for alpha, red, green and blue.
		EF_ARGB8,
		// 32-bit element format, 2 bits for alpha, 10 bits for red, green and blue.
		EF_A2RGB10,

		// 32-bit element format, 16 bits for red and green.
		EF_R16,
		// 32-bit element format, 16 bits for red and green.
		EF_GR16,
		// 48-bit element format, 16 bits for alpha, blue, green and red.
		EF_BGR16,
		// 64-bit element format, 16 bits for alpha, blue, green and red.
		EF_ABGR16,
		// 32-bit element format, 32 bits for red.
		EF_R32,
		// 64-bit element format, 16 bits for red and green.
		EF_GR32,
		// 96-bit element format, 16 bits for alpha, blue, green and red.
		EF_BGR32,
		// 128-bit element format, 16 bits for alpha, blue, green and red.
		EF_ABGR32,

		// 16-bit element format, 16 bits floating-point for red.
		EF_R16F,
		// 32-bit element format, 16 bits floating-point for green and red.
		EF_GR16F,
		// 48-bit element format, 16 bits floating-point for blue, green and red.
		EF_BGR16F,
		// 64-bit element format, 16 bits floating-point for alpha, blue, green and red.
		EF_ABGR16F,
		// 32-bit element format, 32 bits floating-point for red.
		EF_R32F,
		// 64-bit element format, 32 bits floating-point for green and red.
		EF_GR32F,
		// 96-bit element format, 32 bits floating-point for blue, green and red.
		EF_BGR32F,
		// 128-bit element format, 32 bits floating-point for alpha, blue, green and red.
		EF_ABGR32F,

		// DXT1 compression element format 
		EF_DXT1,
		// DXT3 compression element format
		EF_DXT3,
		// DXT5 compression element format
		EF_DXT5,

		// 16-bit element format, 16 bits depth
		EF_D16,
		// 32-bit element format, 24 bits depth and 8 bits stencil
		EF_D24S8,
		// 32-bit element format, 32 bits depth
		EF_D32,

		// 32-bit element format, 8 bits for alpha, red, green and blue. Standard RGB (gamma = 2.2).
		EF_ARGB8_SRGB,
		// DXT1 compression element format. Standard RGB (gamma = 2.2).
		EF_DXT1_SRGB,
		// DXT3 compression element format. Standard RGB (gamma = 2.2).
		EF_DXT3_SRGB,
		// DXT5 compression element format. Standard RGB (gamma = 2.2).
		EF_DXT5_SRGB,
	};

	inline uint8_t
	ElementFormatBits(ElementFormat format)
	{
		switch (format)
		{
		case EF_L8:
		case EF_A8:
		case EF_AL4:
			return 8;

		case EF_L16:
		case EF_AL8:
		case EF_R5G6B5:
		case EF_ARGB4:
		case EF_R16:
		case EF_R16F:
		case EF_DXT1:
		case EF_DXT1_SRGB:
		case EF_D16:
			return 16;

		case EF_RGB8:
			return 24;

		case EF_AL16:
		case EF_ARGB8:
		case EF_ARGB8_SRGB:
		case EF_A2RGB10:
		case EF_GR16:
		case EF_GR16F:
		case EF_R32:
		case EF_R32F:
		case EF_DXT3:
		case EF_DXT5:
		case EF_DXT3_SRGB:
		case EF_DXT5_SRGB:
		case EF_D24S8:
		case EF_D32:
			return 32;

		case EF_BGR16:
		case EF_BGR16F:
			return 48;

		case EF_ABGR16:
		case EF_ABGR16F:
		case EF_GR32:
		case EF_GR32F:
			return 64;

		case EF_BGR32:
		case EF_BGR32F:
			return 96;

		case EF_ABGR32:
		case EF_ABGR32F:
			return 128;

		default:
			BOOST_ASSERT(false);
			return 0;
		}
	}

	inline uint8_t
	ElementFormatBytes(ElementFormat format)
	{
		return ElementFormatBits(format) / 8;
	}

	inline bool
	IsFloatFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_R16F:
		case EF_GR16F:
		case EF_BGR16F:
		case EF_ABGR16F:
		case EF_R32F:
		case EF_GR32F:
		case EF_BGR32F:
		case EF_ABGR32F:
			return true;

		default:
			return false;
		}
	}

	inline bool
	IsCompressedFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_DXT1:
		case EF_DXT3:
		case EF_DXT5:
		case EF_DXT1_SRGB:
		case EF_DXT3_SRGB:
		case EF_DXT5_SRGB:
			return true;

		default:
			return false;
		}
	}

	inline bool
	IsDepthFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_D16:
		case EF_D24S8:
		case EF_D32:
			return true;

		default:
			return false;
		}
	}

	inline bool
	IsStencilFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_D24S8:
			return true;

		default:
			return false;
		}
	}

	inline bool
	IsSRGB(ElementFormat format)
	{
		switch (format)
		{
		case EF_ARGB8_SRGB:
		case EF_DXT1_SRGB:
		case EF_DXT3_SRGB:
		case EF_DXT5_SRGB:
			return true;

		default:
			return false;
		}
	}

	inline uint32_t
	NumComponents(ElementFormat format)
	{
		switch (format)
		{
		case EF_L8:
		case EF_A8:
		case EF_L16:
		case EF_R16:
		case EF_R32:
		case EF_R16F:
		case EF_R32F:
		case EF_D16:
		case EF_D32:
			return 1;

		case EF_AL4:
		case EF_AL8:
		case EF_AL16:
		case EF_GR16:
		case EF_GR32:
		case EF_GR16F:
		case EF_GR32F:
		case EF_D24S8:
			return 2;

		case EF_R5G6B5:
		case EF_BGR16:
		case EF_BGR32:
		case EF_BGR16F:
		case EF_BGR32F:
			return 3;

		case EF_ARGB4:
		case EF_ARGB8:
		case EF_A2RGB10:
		case EF_ABGR16:
		case EF_ABGR32:
		case EF_ABGR16F:
		case EF_ABGR32F:
		case EF_DXT1:
		case EF_DXT3:
		case EF_DXT5:
		case EF_ARGB8_SRGB:
		case EF_DXT1_SRGB:
		case EF_DXT3_SRGB:
		case EF_DXT5_SRGB:
			return 4;

		default:
			BOOST_ASSERT(false);
			return 0;
		}
	}

	inline uint32_t
	ComponentBpps(ElementFormat format)
	{
		switch (format)
		{
		case EF_AL4:
		case EF_ARGB4:
			return 4;

		case EF_L8:
		case EF_A8:
		case EF_AL8:
		case EF_ARGB8:
		case EF_ARGB8_SRGB:
			return 8;

		case EF_L16:
		case EF_AL16:
		case EF_GR16:
		case EF_ABGR16:
		case EF_R16F:
		case EF_GR16F:
		case EF_BGR16F:
		case EF_ABGR16F:
		case EF_D16:
			return 16;

		case EF_R32F:
		case EF_GR32F:
		case EF_BGR32F:
		case EF_ABGR32F:
		case EF_D32:
			return 32;

		default:
			BOOST_ASSERT(false);
			return 0;
		}
	}
}

#endif			// _ELEMENTFORMAT_HPP
