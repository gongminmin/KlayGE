// ElementFormat.hpp
// KlayGE 元素格式 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 增加了access_hint的标志 (2008.9.20)
// 增加了ElementInitData (2008.10.1)
//
// 3.5.0
// 支持有符号格式 (2007.2.12)
//
// 3.4.0
// 增加了MakeSRGB/MakeNonSRGB (2006.9.6)
//
// 3.3.0
// 初次建立 (2006.6.8)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _ELEMENTFORMAT_HPP
#define _ELEMENTFORMAT_HPP

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <vector>
#include <boost/assert.hpp>

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
		// 16-bit element format, 8 bits for red, green.
		EF_RG8,
		// 16-bit element format, 8 bits for signed red, green.
		EF_SIGNED_GR8,
		// 24-bit element format, 8 bits for red, green and blue.
		EF_RGB8,
		// 24-bit element format, 8 bits for signed red, green and blue.
		EF_SIGNED_BGR8,
		// 32-bit element format, 8 bits for alpha, red, green and blue.
		EF_ARGB8,
		// 32-bit element format, 8 bits for alpha, red, green and blue.
		EF_ABGR8,
		// 32-bit element format, 8 bits for signed alpha, red, green and blue.
		EF_SIGNED_ABGR8,
		// 32-bit element format, 2 bits for alpha, 10 bits for red, green and blue.
		EF_A2BGR10,
		// 32-bit element format, 2 bits for alpha, 10 bits for signed red, green and blue.
		EF_SIGNED_A2BGR10,

		// 16-bit element format, 16 bits for red.
		EF_R16,
		// 16-bit element format, 16 bits for signed red.
		EF_SIGNED_R16,
		// 32-bit element format, 16 bits for red and green.
		EF_GR16,
		// 32-bit element format, 16 bits for signed red and green.
		EF_SIGNED_GR16,
		// 48-bit element format, 16 bits for alpha, blue, green and red.
		EF_BGR16,
		// 48-bit element format, 16 bits for signed alpha, blue, green and red.
		EF_SIGNED_BGR16,
		// 64-bit element format, 16 bits for alpha, blue, green and red.
		EF_ABGR16,
		// 64-bit element format, 16 bits for signed alpha, blue, green and red.
		EF_SIGNED_ABGR16,
		// 32-bit element format, 32 bits for red.
		EF_R32,
		// 32-bit element format, 32 bits for signed red.
		EF_SIGNED_R32,
		// 64-bit element format, 16 bits for red and green.
		EF_GR32,
		// 64-bit element format, 16 bits for signed red and green.
		EF_SIGNED_GR32,
		// 96-bit element format, 16 bits for alpha, blue, green and red.
		EF_BGR32,
		// 96-bit element format, 16 bits for signed_alpha, blue, green and red.
		EF_SIGNED_BGR32,
		// 128-bit element format, 16 bits for alpha, blue, green and red.
		EF_ABGR32,
		// 128-bit element format, 16 bits for signed alpha, blue, green and red.
		EF_SIGNED_ABGR32,

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

		// BC1 compression element format, DXT1
		EF_BC1,
		// BC1 compression element format, signed DXT1
		EF_SIGNED_BC1,
		// BC2 compression element format, DXT3
		EF_BC2,
		// BC2 compression element format, signed DXT3
		EF_SIGNED_BC2,
		// BC3 compression element format, DXT5
		EF_BC3,
		// BC3 compression element format, signed DXT5
		EF_SIGNED_BC3,
		// BC4 compression element format, 1 channel
		EF_BC4,
		// BC4 compression element format, 1 channel signed
		EF_SIGNED_BC4,
		// BC5 compression element format, 2 channels
		EF_BC5,
		// BC5 compression element format, 2 channels signed
		EF_SIGNED_BC5,

		// 16-bit element format, 16 bits depth
		EF_D16,
		// 32-bit element format, 24 bits depth and 8 bits stencil
		EF_D24S8,
		// 32-bit element format, 32 bits depth
		EF_D32F,

		// 32-bit element format, 8 bits for alpha, red, green and blue. Standard RGB (gamma = 2.2).
		EF_ARGB8_SRGB,
		// 32-bit element format, 8 bits for alpha, red, green and blue. Standard RGB (gamma = 2.2).
		EF_ABGR8_SRGB,
		// BC1 compression element format. Standard RGB (gamma = 2.2).
		EF_BC1_SRGB,
		// BC2 compression element format. Standard RGB (gamma = 2.2).
		EF_BC2_SRGB,
		// BC3 compression element format. Standard RGB (gamma = 2.2).
		EF_BC3_SRGB,
		// BC4 compression element format. Standard RGB (gamma = 2.2).
		EF_BC4_SRGB,
		// BC5 compression element format. Standard RGB (gamma = 2.2).
		EF_BC5_SRGB,
	};

	inline uint8_t
	NumFormatBits(ElementFormat format)
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
		case EF_RG8:
		case EF_SIGNED_GR8:
		case EF_R16:
		case EF_SIGNED_R16:
		case EF_R16F:
		case EF_BC1:
		case EF_SIGNED_BC1:
		case EF_BC1_SRGB:
		case EF_BC4:
		case EF_SIGNED_BC4:
		case EF_BC4_SRGB:
		case EF_D16:
			return 16;

		case EF_RGB8:
		case EF_SIGNED_BGR8:
			return 24;

		case EF_AL16:
		case EF_ARGB8:
		case EF_ABGR8:
		case EF_SIGNED_ABGR8:
		case EF_ARGB8_SRGB:
		case EF_ABGR8_SRGB:
		case EF_A2BGR10:
		case EF_SIGNED_A2BGR10:
		case EF_GR16:
		case EF_SIGNED_GR16:
		case EF_GR16F:
		case EF_R32:
		case EF_SIGNED_R32:
		case EF_R32F:
		case EF_BC2:
		case EF_SIGNED_BC2:
		case EF_BC2_SRGB:
		case EF_BC3:
		case EF_SIGNED_BC3:
		case EF_BC3_SRGB:
		case EF_BC5:
		case EF_SIGNED_BC5:
		case EF_BC5_SRGB:
		case EF_D24S8:
		case EF_D32F:
			return 32;

		case EF_BGR16:
		case EF_SIGNED_BGR16:
		case EF_BGR16F:
			return 48;

		case EF_ABGR16:
		case EF_SIGNED_ABGR16:
		case EF_ABGR16F:
		case EF_GR32:
		case EF_SIGNED_GR32:
		case EF_GR32F:
			return 64;

		case EF_BGR32:
		case EF_SIGNED_BGR32:
		case EF_BGR32F:
			return 96;

		case EF_ABGR32:
		case EF_SIGNED_ABGR32:
		case EF_ABGR32F:
			return 128;

		default:
			BOOST_ASSERT(false);
			return 0;
		}
	}

	inline uint8_t
	NumFormatBytes(ElementFormat format)
	{
		return NumFormatBits(format) / 8;
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
		case EF_BC1:
		case EF_BC2:
		case EF_BC3:
		case EF_BC4:
		case EF_BC5:
		case EF_SIGNED_BC1:
		case EF_SIGNED_BC2:
		case EF_SIGNED_BC3:
		case EF_SIGNED_BC4:
		case EF_SIGNED_BC5:
		case EF_BC1_SRGB:
		case EF_BC2_SRGB:
		case EF_BC3_SRGB:
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
		case EF_D32F:
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
		case EF_ABGR8_SRGB:
		case EF_ARGB8_SRGB:
		case EF_BC1_SRGB:
		case EF_BC2_SRGB:
		case EF_BC3_SRGB:
			return true;

		default:
			return false;
		}
	}

	inline bool
	IsSigned(ElementFormat format)
	{
		switch (format)
		{
		case EF_SIGNED_GR8:
		case EF_SIGNED_BGR8:
		case EF_SIGNED_ABGR8:
		case EF_SIGNED_A2BGR10:
		case EF_SIGNED_R16:
		case EF_SIGNED_GR16:
		case EF_SIGNED_BGR16:
		case EF_SIGNED_ABGR16:
		case EF_SIGNED_R32:
		case EF_SIGNED_GR32:
		case EF_SIGNED_BGR32:
		case EF_SIGNED_ABGR32:
		case EF_SIGNED_BC1:
		case EF_SIGNED_BC2:
		case EF_SIGNED_BC3:
		case EF_SIGNED_BC4:
		case EF_SIGNED_BC5:
			return true;

		default:
			return false;
		}
	}

	inline ElementFormat
	MakeSRGB(ElementFormat format)
	{
		switch (format)
		{
		case EF_ARGB8:
			return EF_ARGB8_SRGB;

		case EF_ABGR8:
			return EF_ABGR8_SRGB;

		case EF_BC1:
			return EF_BC1_SRGB;

		case EF_BC2:
			return EF_BC2_SRGB;

		case EF_BC3:
			return EF_BC3_SRGB;

		default:
			return format;
		}
	}

	inline ElementFormat
	MakeNonSRGB(ElementFormat format)
	{
		switch (format)
		{
		case EF_ARGB8_SRGB:
			return EF_ARGB8;

		case EF_ABGR8_SRGB:
			return EF_ABGR8;

		case EF_BC1_SRGB:
			return EF_BC1;

		case EF_BC2_SRGB:
			return EF_BC2;

		case EF_BC3_SRGB:
			return EF_BC3;

		default:
			return format;
		}
	}

	inline ElementFormat
	MakeSigned(ElementFormat format)
	{
		switch (format)
		{
		case EF_RG8:
			return EF_SIGNED_GR8;

		case EF_RGB8:
			return EF_SIGNED_BGR8;

		case EF_ABGR8:
			return EF_SIGNED_ABGR8;

		case EF_A2BGR10:
			return EF_SIGNED_A2BGR10;

		case EF_R16:
			return EF_SIGNED_R16;

		case EF_GR16:
			return EF_SIGNED_GR16;

		case EF_BGR16:
			return EF_SIGNED_BGR16;

		case EF_ABGR16:
			return EF_SIGNED_ABGR16;

		case EF_R32:
			return EF_SIGNED_R32;

		case EF_GR32:
			return EF_SIGNED_GR32;

		case EF_BGR32:
			return EF_SIGNED_BGR32;

		case EF_ABGR32:
			return EF_SIGNED_ABGR32;

		case EF_BC1:
			return EF_SIGNED_BC1;

		case EF_BC2:
			return EF_SIGNED_BC2;

		case EF_BC3:
			return EF_SIGNED_BC3;

		case EF_BC4:
			return EF_SIGNED_BC4;

		case EF_BC5:
			return EF_SIGNED_BC5;

		default:
			return format;
		}
	}

	inline ElementFormat
	MakeUnsigned(ElementFormat format)
	{
		switch (format)
		{
		case EF_SIGNED_GR8:
			return EF_RG8;

		case EF_SIGNED_BGR8:
			return EF_RGB8;

		case EF_SIGNED_ABGR8:
			return EF_ARGB8;

		case EF_SIGNED_A2BGR10:
			return EF_A2BGR10;

		case EF_SIGNED_R16:
			return EF_R16;

		case EF_SIGNED_GR16:
			return EF_GR16;

		case EF_SIGNED_BGR16:
			return EF_BGR16;

		case EF_SIGNED_ABGR16:
			return EF_ABGR16;

		case EF_SIGNED_R32:
			return EF_R32;

		case EF_SIGNED_GR32:
			return EF_GR32;

		case EF_SIGNED_BGR32:
			return EF_BGR32;

		case EF_SIGNED_ABGR32:
			return EF_ABGR32;

		case EF_SIGNED_BC1:
			return EF_BC1;

		case EF_SIGNED_BC2:
			return EF_BC2;

		case EF_SIGNED_BC3:
			return EF_BC3;

		case EF_SIGNED_BC4:
			return EF_BC4;

		case EF_SIGNED_BC5:
			return EF_BC5;

		default:
			return format;
		}
	}

	inline uint8_t
	NumDepthBits(ElementFormat format)
	{
		switch (format)
		{
		case EF_D16:
			return 16;

		case EF_D24S8:
			return 24;

		case EF_D32F:
			return 32;

		default:
			return 0;
		}
	}

	inline uint8_t
	NumStencilBits(ElementFormat format)
	{
		switch (format)
		{
		case EF_D24S8:
			return 8;

		default:
			return 0;
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
		case EF_SIGNED_R16:
		case EF_R32:
		case EF_SIGNED_R32:
		case EF_R16F:
		case EF_R32F:
		case EF_D16:
		case EF_D32F:
			return 1;

		case EF_AL4:
		case EF_AL8:
		case EF_AL16:
		case EF_RG8:
		case EF_SIGNED_GR8:
		case EF_GR16:
		case EF_SIGNED_GR16:
		case EF_GR32:
		case EF_SIGNED_GR32:
		case EF_GR16F:
		case EF_GR32F:
		case EF_D24S8:
			return 2;

		case EF_R5G6B5:
		case EF_BGR16:
		case EF_SIGNED_BGR16:
		case EF_BGR32:
		case EF_SIGNED_BGR32:
		case EF_BGR16F:
		case EF_BGR32F:
			return 3;

		case EF_ARGB4:
		case EF_ARGB8:
		case EF_ABGR8:
		case EF_SIGNED_ABGR8:
		case EF_A2BGR10:
		case EF_SIGNED_A2BGR10:
		case EF_ABGR16:
		case EF_SIGNED_ABGR16:
		case EF_ABGR32:
		case EF_SIGNED_ABGR32:
		case EF_ABGR16F:
		case EF_ABGR32F:
		case EF_BC1:
		case EF_SIGNED_BC1:
		case EF_BC2:
		case EF_SIGNED_BC2:
		case EF_BC3:
		case EF_SIGNED_BC3:
		case EF_ARGB8_SRGB:
		case EF_ABGR8_SRGB:
		case EF_BC1_SRGB:
		case EF_BC2_SRGB:
		case EF_BC3_SRGB:
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
		case EF_RG8:
		case EF_SIGNED_GR8:
		case EF_ARGB8:
		case EF_ABGR8:
		case EF_SIGNED_ABGR8:
		case EF_ARGB8_SRGB:
		case EF_ABGR8_SRGB:
			return 8;

		case EF_L16:
		case EF_AL16:
		case EF_GR16:
		case EF_SIGNED_GR16:
		case EF_ABGR16:
		case EF_SIGNED_ABGR16:
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
		case EF_D32F:
			return 32;

		default:
			BOOST_ASSERT(false);
			return 0;
		}
	}


	enum ElementAccessHint
	{
		EAH_CPU_Read = 1UL << 0,
		EAH_CPU_Write = 1UL << 1,
		EAH_GPU_Read = 1UL << 2,
		EAH_GPU_Write = 1UL << 3,
	};

	struct ElementInitData
	{
		std::vector<uint8_t> data;
		uint32_t row_pitch;
		uint32_t slice_pitch;
	};
}

#endif			// _ELEMENTFORMAT_HPP
