// Texture.cpp
// KlayGE 纹理类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 支持在线解压BC1-BC3 (2010.1.27)
//
// 3.9.0
// 支持Texture Array的读写 (2009.10.15)
//
// 3.8.0
// 支持BC4/BC5纹理压缩的读写和转换 (2008.12.8)
// 多线程纹理载入 (2009.1.22)
//
// 3.5.0
// 支持有符号格式 (2007.2.12)
//
// 3.3.0
// 支持GR16和ABGR16 (2006.6.7)
//
// 2.4.0
// 初次建立 (2005.3.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/filesystem.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/TexCompressionETC.hpp>
#include <KlayGE/DevHelper.hpp>
#include <KFL/Half.hpp>
#include <KFL/Hash.hpp>

#include <cstring>
#include <fstream>
#include <system_error>

#include <KlayGE/Texture.hpp>

namespace
{
	using namespace KlayGE;

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif

	enum
	{
		// The surface has alpha channel information in the pixel format.
		DDSPF_ALPHAPIXELS = 0x00000001,

		// The pixel format contains alpha only information
		DDSPF_ALPHA = 0x00000002,

		// The FourCC code is valid.
		DDSPF_FOURCC = 0x00000004,

		// The RGB data in the pixel format structure is valid.
		DDSPF_RGB = 0x00000040,

		// Luminance data in the pixel format is valid.
		// Use this flag for luminance-only or luminance+alpha surfaces,
		// the bit depth is then ddpf.dwLuminanceBitCount.
		DDSPF_LUMINANCE = 0x00020000,

		// Bump map dUdV data in the pixel format is valid.
		DDSPF_BUMPDUDV = 0x00080000
	};

	struct DDSPIXELFORMAT
	{
		uint32_t	size;				// size of structure
		uint32_t	flags;				// pixel format flags
		uint32_t	four_cc;			// (FOURCC code)
		uint32_t	rgb_bit_count;		// how many bits per pixel
		uint32_t	r_bit_mask;			// mask for red bit
		uint32_t	g_bit_mask;			// mask for green bits
		uint32_t	b_bit_mask;			// mask for blue bits
		uint32_t	rgb_alpha_bit_mask;	// mask for alpha channels
	};

	enum
	{
		// Indicates a complex surface structure is being described.  A
		// complex surface structure results in the creation of more than
		// one surface.  The additional surfaces are attached to the root
		// surface.  The complex structure can only be destroyed by
		// destroying the root.
		DDSCAPS_COMPLEX		= 0x00000008,

		// Indicates that this surface can be used as a 3D texture.  It does not
		// indicate whether or not the surface is being used for that purpose.
		DDSCAPS_TEXTURE		= 0x00001000,

		// Indicates surface is one level of a mip-map. This surface will
		// be attached to other DDSCAPS_MIPMAP surfaces to form the mip-map.
		// This can be done explicitly, by creating a number of surfaces and
		// attaching them with AddAttachedSurface or by implicitly by CreateSurface.
		// If this bit is set then DDSCAPS_TEXTURE must also be set.
		DDSCAPS_MIPMAP		= 0x00400000,
	};

	enum
	{
		// This flag is used at CreateSurface time to indicate that this set of
		// surfaces is a cubic environment map
		DDSCAPS2_CUBEMAP	= 0x00000200,

		// These flags preform two functions:
		// - At CreateSurface time, they define which of the six cube faces are
		//   required by the application.
		// - After creation, each face in the cubemap will have exactly one of these
		//   bits set.
		DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400,
		DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800,
		DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000,
		DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000,
		DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000,
		DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000,

		// Indicates that the surface is a volume.
		// Can be combined with DDSCAPS_MIPMAP to indicate a multi-level volume
		DDSCAPS2_VOLUME		= 0x00200000,
	};

	struct DDSCAPS2
	{
		uint32_t	caps1;			// capabilities of surface wanted
		uint32_t	caps2;
		uint32_t	reserved[2];
	};

	enum
	{
		DDSD_CAPS			= 0x00000001,	// default, dds_caps field is valid.
		DDSD_HEIGHT			= 0x00000002,	// height field is valid.
		DDSD_WIDTH			= 0x00000004,	// width field is valid.
		DDSD_PITCH			= 0x00000008,	// pitch is valid.
		DDSD_PIXELFORMAT	= 0x00001000,	// pixel_format is valid.
		DDSD_MIPMAPCOUNT	= 0x00020000,	// mip_map_count is valid.
		DDSD_LINEARSIZE		= 0x00080000,	// linear_size is valid
		DDSD_DEPTH			= 0x00800000,	// depth is valid
	};

	struct DDSSURFACEDESC2
	{
		uint32_t	size;					// size of the DDSURFACEDESC structure
		uint32_t	flags;					// determines what fields are valid
		uint32_t	height;					// height of surface to be created
		uint32_t	width;					// width of input surface
		union
		{
			int32_t		pitch;				// distance to start of next line (return value only)
			uint32_t	linear_size;		// Formless late-allocated optimized surface size
		};
		uint32_t		depth;				// the depth if this is a volume texture
		uint32_t		mip_map_count;		// number of mip-map levels requestde
		uint32_t		reserved1[11];		// reserved
		DDSPIXELFORMAT	pixel_format;		// pixel format description of the surface
		DDSCAPS2		dds_caps;			// direct draw surface capabilities
		uint32_t		reserved2;
	};

	enum D3D_RESOURCE_DIMENSION
	{
		D3D_RESOURCE_DIMENSION_UNKNOWN = 0,
		D3D_RESOURCE_DIMENSION_BUFFER = 1,
		D3D_RESOURCE_DIMENSION_TEXTURE1D = 2,
		D3D_RESOURCE_DIMENSION_TEXTURE2D = 3,
		D3D_RESOURCE_DIMENSION_TEXTURE3D = 4,
	};

	enum D3D_RESOURCE_MISC_FLAG
	{
		D3D_RESOURCE_MISC_GENERATE_MIPS = 0x1L,
		D3D_RESOURCE_MISC_SHARED = 0x2L,
		D3D_RESOURCE_MISC_TEXTURECUBE = 0x4L,
		D3D_RESOURCE_MISC_SHARED_KEYEDMUTEX = 0x10L,
		D3D_RESOURCE_MISC_GDI_COMPATIBLE = 0x20L
	};

	struct DDS_HEADER_DXT10
	{
		uint32_t dxgi_format;
		D3D_RESOURCE_DIMENSION resource_dim;
		uint32_t misc_flag;
		uint32_t array_size;
		uint32_t reserved;
	};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

#ifndef DXGI_FORMAT_DEFINED
	enum DXGI_FORMAT : uint32_t
	{
		DXGI_FORMAT_UNKNOWN	                    = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
		DXGI_FORMAT_R32G32B32A32_UINT           = 3,
		DXGI_FORMAT_R32G32B32A32_SINT           = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
		DXGI_FORMAT_R32G32B32_FLOAT             = 6,
		DXGI_FORMAT_R32G32B32_UINT              = 7,
		DXGI_FORMAT_R32G32B32_SINT              = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
		DXGI_FORMAT_R16G16B16A16_UINT           = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
		DXGI_FORMAT_R16G16B16A16_SINT           = 14,
		DXGI_FORMAT_R32G32_TYPELESS             = 15,
		DXGI_FORMAT_R32G32_FLOAT                = 16,
		DXGI_FORMAT_R32G32_UINT                 = 17,
		DXGI_FORMAT_R32G32_SINT                 = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
		DXGI_FORMAT_R10G10B10A2_UINT            = 25,
		DXGI_FORMAT_R11G11B10_FLOAT             = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
		DXGI_FORMAT_R8G8B8A8_UINT               = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
		DXGI_FORMAT_R8G8B8A8_SINT               = 32,
		DXGI_FORMAT_R16G16_TYPELESS             = 33,
		DXGI_FORMAT_R16G16_FLOAT                = 34,
		DXGI_FORMAT_R16G16_UNORM                = 35,
		DXGI_FORMAT_R16G16_UINT                 = 36,
		DXGI_FORMAT_R16G16_SNORM                = 37,
		DXGI_FORMAT_R16G16_SINT                 = 38,
		DXGI_FORMAT_R32_TYPELESS                = 39,
		DXGI_FORMAT_D32_FLOAT                   = 40,
		DXGI_FORMAT_R32_FLOAT                   = 41,
		DXGI_FORMAT_R32_UINT                    = 42,
		DXGI_FORMAT_R32_SINT                    = 43,
		DXGI_FORMAT_R24G8_TYPELESS              = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
		DXGI_FORMAT_R8G8_TYPELESS               = 48,
		DXGI_FORMAT_R8G8_UNORM                  = 49,
		DXGI_FORMAT_R8G8_UINT                   = 50,
		DXGI_FORMAT_R8G8_SNORM                  = 51,
		DXGI_FORMAT_R8G8_SINT                   = 52,
		DXGI_FORMAT_R16_TYPELESS                = 53,
		DXGI_FORMAT_R16_FLOAT                   = 54,
		DXGI_FORMAT_D16_UNORM                   = 55,
		DXGI_FORMAT_R16_UNORM                   = 56,
		DXGI_FORMAT_R16_UINT                    = 57,
		DXGI_FORMAT_R16_SNORM                   = 58,
		DXGI_FORMAT_R16_SINT                    = 59,
		DXGI_FORMAT_R8_TYPELESS                 = 60,
		DXGI_FORMAT_R8_UNORM                    = 61,
		DXGI_FORMAT_R8_UINT                     = 62,
		DXGI_FORMAT_R8_SNORM                    = 63,
		DXGI_FORMAT_R8_SINT                     = 64,
		DXGI_FORMAT_A8_UNORM                    = 65,
		DXGI_FORMAT_R1_UNORM                    = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
		DXGI_FORMAT_BC1_TYPELESS                = 70,
		DXGI_FORMAT_BC1_UNORM                   = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
		DXGI_FORMAT_BC2_TYPELESS                = 73,
		DXGI_FORMAT_BC2_UNORM                   = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
		DXGI_FORMAT_BC3_TYPELESS                = 76,
		DXGI_FORMAT_BC3_UNORM                   = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
		DXGI_FORMAT_BC4_TYPELESS                = 79,
		DXGI_FORMAT_BC4_UNORM                   = 80,
		DXGI_FORMAT_BC4_SNORM                   = 81,
		DXGI_FORMAT_BC5_TYPELESS                = 82,
		DXGI_FORMAT_BC5_UNORM                   = 83,
		DXGI_FORMAT_BC5_SNORM                   = 84,
		DXGI_FORMAT_B5G6R5_UNORM                = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
		DXGI_FORMAT_BC6H_TYPELESS               = 94,
		DXGI_FORMAT_BC6H_UF16                   = 95,
		DXGI_FORMAT_BC6H_SF16                   = 96,
		DXGI_FORMAT_BC7_TYPELESS                = 97,
		DXGI_FORMAT_BC7_UNORM                   = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
		DXGI_FORMAT_AYUV                        = 100,
		DXGI_FORMAT_Y410                        = 101,
		DXGI_FORMAT_Y416                        = 102,
		DXGI_FORMAT_NV12                        = 103,
		DXGI_FORMAT_P010                        = 104,
		DXGI_FORMAT_P016                        = 105,
		DXGI_FORMAT_420_OPAQUE                  = 106,
		DXGI_FORMAT_YUY2                        = 107,
		DXGI_FORMAT_Y210                        = 108,
		DXGI_FORMAT_Y216                        = 109,
		DXGI_FORMAT_NV11                        = 110,
		DXGI_FORMAT_AI44                        = 111,
		DXGI_FORMAT_IA44                        = 112,
		DXGI_FORMAT_P8                          = 113,
		DXGI_FORMAT_A8P8                        = 114,
		DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
		DXGI_FORMAT_FORCE_UINT                  = 0xffffffff
	};
#endif

	ElementFormat FromDXGIFormat(uint32_t format)
	{
		switch (format)
		{
		case DXGI_FORMAT_A8_UNORM:
			return EF_A8;

		case DXGI_FORMAT_B5G6R5_UNORM:
			return EF_R5G6B5;

		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return EF_A1RGB5;

		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return EF_ARGB4;

		case DXGI_FORMAT_R8_UNORM:
			return EF_R8;

		case DXGI_FORMAT_R8_SNORM:
			return EF_SIGNED_R8;

		case DXGI_FORMAT_R8G8_UNORM:
			return EF_GR8;

		case DXGI_FORMAT_R8G8_SNORM:
			return EF_SIGNED_GR8;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return EF_ARGB8;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return EF_ABGR8;

		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return EF_SIGNED_ABGR8;

		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return EF_A2BGR10;

		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return EF_SIGNED_A2BGR10;

		case DXGI_FORMAT_R8_UINT:
			return EF_R8UI;

		case DXGI_FORMAT_R8_SINT:
			return EF_R8I;

		case DXGI_FORMAT_R8G8_UINT:
			return EF_GR8UI;

		case DXGI_FORMAT_R8G8_SINT:
			return EF_GR8I;

		case DXGI_FORMAT_R8G8B8A8_UINT:
			return EF_ABGR8UI;

		case DXGI_FORMAT_R8G8B8A8_SINT:
			return EF_ABGR8I;

		case DXGI_FORMAT_R10G10B10A2_UINT:
			return EF_A2BGR10UI;

		case DXGI_FORMAT_R16_UNORM:
			return EF_R16;

		case DXGI_FORMAT_R16_SNORM:
			return EF_SIGNED_R16;

		case DXGI_FORMAT_R16G16_UNORM:
			return EF_GR16;

		case DXGI_FORMAT_R16G16_SNORM:
			return EF_SIGNED_GR16;

		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return EF_ABGR16;

		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return EF_SIGNED_ABGR16;

		case DXGI_FORMAT_R16_UINT:
			return EF_R16UI;

		case DXGI_FORMAT_R16_SINT:
			return EF_R16I;

		case DXGI_FORMAT_R16G16_UINT:
			return EF_GR16UI;

		case DXGI_FORMAT_R16G16_SINT:
			return EF_GR16I;

		case DXGI_FORMAT_R16G16B16A16_UINT:
			return EF_ABGR16UI;

		case DXGI_FORMAT_R16G16B16A16_SINT:
			return EF_ABGR16I;

		case DXGI_FORMAT_R32_UINT:
			return EF_R32UI;

		case DXGI_FORMAT_R32_SINT:
			return EF_R32I;

		case DXGI_FORMAT_R32G32_UINT:
			return EF_GR32UI;

		case DXGI_FORMAT_R32G32_SINT:
			return EF_GR32I;

		case DXGI_FORMAT_R32G32B32_UINT:
			return EF_BGR32UI;

		case DXGI_FORMAT_R32G32B32_SINT:
			return EF_BGR32I;

		case DXGI_FORMAT_R32G32B32A32_UINT:
			return EF_ABGR32UI;

		case DXGI_FORMAT_R32G32B32A32_SINT:
			return EF_ABGR32I;

		case DXGI_FORMAT_R16_FLOAT:
			return EF_R16F;

		case DXGI_FORMAT_R16G16_FLOAT:
			return EF_GR16F;

		case DXGI_FORMAT_R11G11B10_FLOAT:
			return EF_B10G11R11F;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return EF_ABGR16F;

		case DXGI_FORMAT_R32_FLOAT:
			return EF_R32F;

		case DXGI_FORMAT_R32G32_FLOAT:
			return EF_GR32F;

		case DXGI_FORMAT_R32G32B32_FLOAT:
			return EF_BGR32F;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return EF_ABGR32F;

		case DXGI_FORMAT_BC1_UNORM:
			return EF_BC1;

		case DXGI_FORMAT_BC2_UNORM:
			return EF_BC2;

		case DXGI_FORMAT_BC3_UNORM:
			return EF_BC3;

		case DXGI_FORMAT_BC4_UNORM:
			return EF_BC4;

		case DXGI_FORMAT_BC4_SNORM:
			return EF_SIGNED_BC4;

		case DXGI_FORMAT_BC5_UNORM:
			return EF_BC5;

		case DXGI_FORMAT_BC5_SNORM:
			return EF_SIGNED_BC5;

		case DXGI_FORMAT_BC6H_UF16:
			return EF_BC6;

		case DXGI_FORMAT_BC6H_SF16:
			return EF_SIGNED_BC6;

		case DXGI_FORMAT_BC7_UNORM:
			return EF_BC7;

		case DXGI_FORMAT_D16_UNORM:
			return EF_D16;

		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return EF_D24S8;

		case DXGI_FORMAT_D32_FLOAT:
			return EF_D32F;

		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return EF_ABGR8_SRGB;

		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return EF_BC1_SRGB;

		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return EF_BC2_SRGB;

		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return EF_BC3_SRGB;

		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return EF_BC7_SRGB;

			// My extensions for ETC

		case 0x80000000UL:
			return EF_ETC1;

		case 0x80000001UL:
			return EF_ETC2_R11;

		case 0x80000002UL:
			return EF_SIGNED_ETC2_R11;

		case 0x80000003UL:
			return EF_ETC2_GR11;

		case 0x80000004UL:
			return EF_SIGNED_ETC2_GR11;

		case 0x80000005UL:
			return EF_ETC2_BGR8;

		case 0x80000006UL:
			return EF_ETC2_BGR8_SRGB;

		case 0x80000007UL:
			return EF_ETC2_A1BGR8;

		case 0x80000008UL:
			return EF_ETC2_A1BGR8_SRGB;

		case 0x80000009UL:
			return EF_ETC2_ABGR8;

		case 0x8000000AUL:
			return EF_ETC2_ABGR8_SRGB;

		default:
			KFL_UNREACHABLE("Invalid format");
		}
	}

	DXGI_FORMAT ToDXGIFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_A8:
			return DXGI_FORMAT_A8_UNORM;

		case EF_R5G6B5:
			return DXGI_FORMAT_B5G6R5_UNORM;

		case EF_A1RGB5:
			return DXGI_FORMAT_B5G5R5A1_UNORM;

		case EF_ARGB4:
			return DXGI_FORMAT_B4G4R4A4_UNORM;

		case EF_R8:
			return DXGI_FORMAT_R8_UNORM;

		case EF_SIGNED_R8:
			return DXGI_FORMAT_R8_SNORM;

		case EF_GR8:
			return DXGI_FORMAT_R8G8_UNORM;

		case EF_SIGNED_GR8:
			return DXGI_FORMAT_R8G8_SNORM;

		case EF_ARGB8:
		case EF_ARGB8_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case EF_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case EF_SIGNED_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_SNORM;

		case EF_A2BGR10:
			return DXGI_FORMAT_R10G10B10A2_UNORM;

		case EF_SIGNED_A2BGR10:
			return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

		case EF_R8UI:
			return DXGI_FORMAT_R8_UINT;

		case EF_R8I:
			return DXGI_FORMAT_R8_SINT;

		case EF_GR8UI:
			return DXGI_FORMAT_R8G8_UINT;

		case EF_GR8I:
			return DXGI_FORMAT_R8G8_SINT;

		case EF_ABGR8UI:
			return DXGI_FORMAT_R8G8B8A8_UINT;

		case EF_ABGR8I:
			return DXGI_FORMAT_R8G8B8A8_SINT;

		case EF_A2BGR10UI:
			return DXGI_FORMAT_R10G10B10A2_UINT;

		case EF_R16:
			return DXGI_FORMAT_R16_UNORM;

		case EF_SIGNED_R16:
			return DXGI_FORMAT_R16_SNORM;

		case EF_GR16:
			return DXGI_FORMAT_R16G16_UNORM;

		case EF_SIGNED_GR16:
			return DXGI_FORMAT_R16G16_SNORM;

		case EF_ABGR16:
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case EF_SIGNED_ABGR16:
			return DXGI_FORMAT_R16G16B16A16_SNORM;

		case EF_R16UI:
			return DXGI_FORMAT_R16_UINT;

		case EF_R16I:
			return DXGI_FORMAT_R16_SINT;

		case EF_GR16UI:
			return DXGI_FORMAT_R16G16_UINT;

		case EF_GR16I:
			return DXGI_FORMAT_R16G16_SINT;

		case EF_ABGR16UI:
			return DXGI_FORMAT_R16G16B16A16_UINT;

		case EF_ABGR16I:
			return DXGI_FORMAT_R16G16B16A16_SINT;

		case EF_R32UI:
			return DXGI_FORMAT_R32_UINT;

		case EF_R32I:
			return DXGI_FORMAT_R32_SINT;

		case EF_GR32UI:
			return DXGI_FORMAT_R32G32_UINT;

		case EF_GR32I:
			return DXGI_FORMAT_R32G32_SINT;

		case EF_BGR32UI:
			return DXGI_FORMAT_R32G32B32_UINT;

		case EF_BGR32I:
			return DXGI_FORMAT_R32G32B32_SINT;

		case EF_ABGR32UI:
			return DXGI_FORMAT_R32G32B32A32_UINT;

		case EF_ABGR32I:
			return DXGI_FORMAT_R32G32B32A32_SINT;

		case EF_R16F:
			return DXGI_FORMAT_R16_FLOAT;

		case EF_GR16F:
			return DXGI_FORMAT_R16G16_FLOAT;

		case EF_B10G11R11F:
			return DXGI_FORMAT_R11G11B10_FLOAT;

		case EF_ABGR16F:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case EF_R32F:
			return DXGI_FORMAT_R32_FLOAT;

		case EF_GR32F:
			return DXGI_FORMAT_R32G32_FLOAT;

		case EF_BGR32F:
			return DXGI_FORMAT_R32G32B32_FLOAT;

		case EF_ABGR32F:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case EF_BC1:
			return DXGI_FORMAT_BC1_UNORM;

		case EF_BC2:
			return DXGI_FORMAT_BC2_UNORM;

		case EF_BC3:
			return DXGI_FORMAT_BC3_UNORM;

		case EF_BC4:
			return DXGI_FORMAT_BC4_UNORM;

		case EF_SIGNED_BC4:
			return DXGI_FORMAT_BC4_SNORM;

		case EF_BC5:
			return DXGI_FORMAT_BC5_UNORM;

		case EF_SIGNED_BC5:
			return DXGI_FORMAT_BC5_SNORM;

		case EF_BC6:
			return DXGI_FORMAT_BC6H_UF16;

		case EF_SIGNED_BC6:
			return DXGI_FORMAT_BC6H_SF16;

		case EF_BC7:
			return DXGI_FORMAT_BC7_UNORM;

		case EF_D16:
			return DXGI_FORMAT_D16_UNORM;

		case EF_D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

		case EF_D32F:
			return DXGI_FORMAT_D32_FLOAT;

		case EF_ABGR8_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case EF_BC1_SRGB:
			return DXGI_FORMAT_BC1_UNORM_SRGB;

		case EF_BC2_SRGB:
			return DXGI_FORMAT_BC2_UNORM_SRGB;

		case EF_BC3_SRGB:
			return DXGI_FORMAT_BC3_UNORM_SRGB;

		case EF_BC7_SRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;

			// My extensions for ETC

		case EF_ETC1:
			return static_cast<DXGI_FORMAT>(0x80000000UL);

		case EF_ETC2_R11:
			return static_cast<DXGI_FORMAT>(0x80000001UL);

		case EF_SIGNED_ETC2_R11:
			return static_cast<DXGI_FORMAT>(0x80000002UL);

		case EF_ETC2_GR11:
			return static_cast<DXGI_FORMAT>(0x80000003UL);

		case EF_SIGNED_ETC2_GR11:
			return static_cast<DXGI_FORMAT>(0x80000004UL);

		case EF_ETC2_BGR8:
			return static_cast<DXGI_FORMAT>(0x80000005UL);

		case EF_ETC2_BGR8_SRGB:
			return static_cast<DXGI_FORMAT>(0x80000006UL);

		case EF_ETC2_A1BGR8:
			return static_cast<DXGI_FORMAT>(0x80000007UL);

		case EF_ETC2_A1BGR8_SRGB:
			return static_cast<DXGI_FORMAT>(0x80000008UL);

		case EF_ETC2_ABGR8:
			return static_cast<DXGI_FORMAT>(0x80000009UL);

		case EF_ETC2_ABGR8_SRGB:
			return static_cast<DXGI_FORMAT>(0x8000000AUL);

		default:
			KFL_UNREACHABLE("Invalid format");
		}
	}


	void EncodeTexture(void* dst_data, uint32_t dst_row_pitch, uint32_t dst_slice_pitch, ElementFormat dst_format,
		void const * src_data, uint32_t src_row_pitch, uint32_t src_slice_pitch, ElementFormat src_format,
		uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		BOOST_ASSERT(IsCompressedFormat(dst_format) && !IsCompressedFormat(src_format));
		KFL_UNUSED(src_format);

		std::unique_ptr<TexCompression> codec;
		switch (dst_format)
		{
		case EF_BC1:
		case EF_BC1_SRGB:
		case EF_SIGNED_BC1:
			codec = MakeUniquePtr<TexCompressionBC1>();
			break;

		case EF_BC2:
		case EF_BC2_SRGB:
		case EF_SIGNED_BC2:
			codec = MakeUniquePtr<TexCompressionBC2>();
			break;

		case EF_BC3:
		case EF_BC3_SRGB:
		case EF_SIGNED_BC3:
			codec = MakeUniquePtr<TexCompressionBC3>();
			break;

		case EF_BC4:
		case EF_BC4_SRGB:
		case EF_SIGNED_BC4:
			codec = MakeUniquePtr<TexCompressionBC4>();
			break;

		case EF_BC5:
		case EF_BC5_SRGB:
		case EF_SIGNED_BC5:
			codec = MakeUniquePtr<TexCompressionBC5>();
			break;

		case EF_BC6:
			codec = MakeUniquePtr<TexCompressionBC6U>();
			break;

		case EF_SIGNED_BC6:
			codec = MakeUniquePtr<TexCompressionBC6S>();
			break;

		case EF_BC7:
		case EF_BC7_SRGB:
			codec = MakeUniquePtr<TexCompressionBC7>();
			break;

		case EF_ETC1:
			codec = MakeUniquePtr<TexCompressionETC1>();
			break;

		case EF_ETC2_BGR8:
		case EF_ETC2_BGR8_SRGB:
			codec = MakeUniquePtr<TexCompressionETC2RGB8>();
			break;

		case EF_ETC2_A1BGR8:
		case EF_ETC2_A1BGR8_SRGB:
			codec = MakeUniquePtr<TexCompressionETC2RGB8A1>();
			break;

		case EF_ETC2_ABGR8:
		case EF_ETC2_ABGR8_SRGB:
			// TODO
			KFL_UNREACHABLE("Not implemented");
			break;

		case EF_ETC2_R11:
		case EF_SIGNED_ETC2_R11:
			// TODO
			KFL_UNREACHABLE("Not implemented");
			break;

		case EF_ETC2_GR11:
		case EF_SIGNED_ETC2_GR11:
			// TODO
			KFL_UNREACHABLE("Not implemented");
			break;

		default:
			KFL_UNREACHABLE("Invalid compression format");
		}

		uint8_t const * src = static_cast<uint8_t const *>(src_data);
		uint8_t* dst = static_cast<uint8_t*>(dst_data);
		for (uint32_t z = 0; z < src_depth; ++ z)
		{
			codec->EncodeMem(src_width, src_height, dst, dst_row_pitch, dst_slice_pitch,
				src, src_row_pitch, src_slice_pitch, TCM_Quality);

			src += src_slice_pitch;
			dst += dst_slice_pitch;
		}
	}
	
	void DecodeTexture(std::vector<uint8_t>& dst_data_block, uint32_t& dst_row_pitch, uint32_t& dst_slice_pitch, ElementFormat& dst_format,
		void const * src_data, uint32_t src_row_pitch, uint32_t src_slice_pitch, ElementFormat src_format,
		uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		BOOST_ASSERT(IsCompressedFormat(src_format));

		switch (src_format)
		{				
		case EF_BC1:
		case EF_BC2:
		case EF_BC3:
		case EF_BC7:
		case EF_ETC1:
		case EF_ETC2_BGR8:
		case EF_ETC2_A1BGR8:
		case EF_ETC2_ABGR8:
			dst_format = EF_ARGB8;
			break;
				
		case EF_BC4:
		case EF_ETC2_R11:
			dst_format = EF_R8;
			break;

		case EF_BC5:
		case EF_ETC2_GR11:
			dst_format = EF_GR8;
			break;

		case EF_SIGNED_BC1:
		case EF_SIGNED_BC2:
		case EF_SIGNED_BC3:
		case EF_SIGNED_ETC2_R11:
			dst_format = EF_SIGNED_ABGR8;
			break;

		case EF_SIGNED_BC4:
			dst_format = EF_SIGNED_R8;
			break;

		case EF_SIGNED_BC5:
			dst_format = EF_SIGNED_GR8;
			break;

		case EF_BC1_SRGB:
		case EF_BC2_SRGB:
		case EF_BC3_SRGB:
		case EF_BC4_SRGB:
		case EF_BC5_SRGB:
		case EF_BC7_SRGB:
		case EF_ETC2_BGR8_SRGB:
		case EF_ETC2_A1BGR8_SRGB:
		case EF_ETC2_ABGR8_SRGB:
			dst_format = EF_ARGB8_SRGB;
			break;

		case EF_BC6:
		case EF_SIGNED_BC6:
			dst_format = EF_ABGR16F;
			break;

		default:
			KFL_UNREACHABLE("Invalid destination format");
		}

		std::unique_ptr<TexCompression> codec;
		switch (src_format)
		{
		case EF_BC1:
		case EF_BC1_SRGB:
		case EF_SIGNED_BC1:
			codec = MakeUniquePtr<TexCompressionBC1>();
			break;

		case EF_BC2:
		case EF_BC2_SRGB:
		case EF_SIGNED_BC2:
			codec = MakeUniquePtr<TexCompressionBC2>();
			break;

		case EF_BC3:
		case EF_BC3_SRGB:
		case EF_SIGNED_BC3:
			codec = MakeUniquePtr<TexCompressionBC3>();
			break;

		case EF_BC4:
		case EF_BC4_SRGB:
		case EF_SIGNED_BC4:
			codec = MakeUniquePtr<TexCompressionBC4>();
			break;

		case EF_BC5:
		case EF_BC5_SRGB:
		case EF_SIGNED_BC5:
			codec = MakeUniquePtr<TexCompressionBC5>();
			break;

		case EF_BC6:
			codec = MakeUniquePtr<TexCompressionBC6U>();
			break;

		case EF_SIGNED_BC6:
			codec = MakeUniquePtr<TexCompressionBC6S>();
			break;

		case EF_BC7:
		case EF_BC7_SRGB:
			codec = MakeUniquePtr<TexCompressionBC7>();
			break;

		case EF_ETC1:
			codec = MakeUniquePtr<TexCompressionETC1>();
			break;

		case EF_ETC2_BGR8:
		case EF_ETC2_BGR8_SRGB:
			codec = MakeUniquePtr<TexCompressionETC2RGB8>();
			break;

		case EF_ETC2_A1BGR8:
		case EF_ETC2_A1BGR8_SRGB:
			codec = MakeUniquePtr<TexCompressionETC2RGB8A1>();
			break;

		case EF_ETC2_ABGR8:
		case EF_ETC2_ABGR8_SRGB:
			// TODO
			KFL_UNREACHABLE("Not implemented");
			break;

		case EF_ETC2_R11:
		case EF_SIGNED_ETC2_R11:
			// TODO
			KFL_UNREACHABLE("Not implemented");
			break;

		case EF_ETC2_GR11:
		case EF_SIGNED_ETC2_GR11:
			// TODO
			KFL_UNREACHABLE("Not implemented");
			break;

		default:
			KFL_UNREACHABLE("Invalid source format");
		}

		dst_row_pitch = src_width * NumFormatBytes(dst_format);
		dst_slice_pitch = dst_row_pitch * src_height;
		dst_data_block.resize(src_depth * dst_slice_pitch);

		uint8_t const * src = static_cast<uint8_t const *>(src_data);
		uint8_t* dst = static_cast<uint8_t*>(&dst_data_block[0]);
		for (uint32_t z = 0; z < src_depth; ++ z)
		{
			codec->DecodeMem(src_width, src_height, dst, dst_row_pitch, dst_slice_pitch,
				src, src_row_pitch, src_slice_pitch);

			src += src_slice_pitch;
			dst += dst_slice_pitch;
		}
	}


	class TextureLoadingDesc : public ResLoadingDesc
	{
	private:
		struct TexDesc
		{
			std::string res_name;
			std::string metadata_name;
			std::string runtime_name;
			uint32_t access_hint;

			struct TexData
			{
				Texture::TextureType type;
				uint32_t width, height, depth;
				uint32_t num_mipmaps;
				uint32_t array_size;
				ElementFormat format;
				std::vector<ElementInitData> init_data;
				std::vector<uint8_t> data_block;
			};
			std::shared_ptr<TexData> tex_data;

			std::shared_ptr<TexturePtr> tex;
		};

	public:
		TextureLoadingDesc(std::string_view res_name, uint32_t access_hint)
		{
			tex_desc_.res_name = std::string(res_name);
			tex_desc_.access_hint = access_hint;
			tex_desc_.tex_data = MakeSharedPtr<TexDesc::TexData>();
			tex_desc_.tex = MakeSharedPtr<TexturePtr>();

			std::filesystem::path res_path(tex_desc_.res_name);
			bool const dds_ext = (res_path.extension().string() == ".dds");
			tex_desc_.metadata_name = tex_desc_.res_name + ".kmeta";
			tex_desc_.runtime_name = tex_desc_.res_name;
			if (!dds_ext || !ResLoader::Instance().Locate(tex_desc_.metadata_name).empty())
			{
				// Texture's runtime format is dds, for now
				tex_desc_.runtime_name += ".dds";
			}
		}

		uint64_t Type() const override
		{
			static uint64_t const type = CT_HASH("TextureLoadingDesc");
			return type;
		}

		bool StateLess() const override
		{
			return true;
		}

		virtual std::shared_ptr<void> CreateResource() override
		{
			TexDesc::TexData& tex_data = *tex_desc_.tex_data;

			{
				uint32_t row_pitch, slice_pitch;
				std::string_view image_info_name;
				if (ResLoader::Instance().Locate(tex_desc_.runtime_name).empty())
				{
					image_info_name = tex_desc_.res_name;
				}
				else
				{
					image_info_name = tex_desc_.runtime_name;
				}
				GetImageInfo(image_info_name, tex_data.type, tex_data.width, tex_data.height, tex_data.depth,
					tex_data.num_mipmaps, tex_data.array_size, tex_data.format,
					row_pitch, slice_pitch);
			}

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			if ((Texture::TT_3D == tex_data.type) && (caps.max_texture_depth < tex_data.depth))
			{
				tex_data.type = Texture::TT_2D;
				tex_data.height *= tex_data.depth;
				tex_data.depth = 1;
				tex_data.num_mipmaps = 1;
				tex_data.init_data.resize(1);
			}

			uint32_t array_size = tex_data.array_size;
			if (Texture::TT_Cube == tex_data.type)
			{
				array_size *= 6;
			}

			if (((EF_BC5 == tex_data.format) && !caps.TextureFormatSupport(EF_BC5))
				|| ((EF_BC5_SRGB == tex_data.format) && !caps.TextureFormatSupport(EF_BC5_SRGB)))
			{
				if (IsSRGB(tex_data.format))
				{
					tex_data.format = EF_BC3_SRGB;
				}
				else
				{
					tex_data.format = EF_BC3;
				}
			}
			if (((EF_BC4 == tex_data.format) && !caps.TextureFormatSupport(EF_BC4))
				|| ((EF_BC4_SRGB == tex_data.format) && !caps.TextureFormatSupport(EF_BC4_SRGB)))
			{
				if (IsSRGB(tex_data.format))
				{
					tex_data.format = EF_BC1_SRGB;
				}
				else
				{
					tex_data.format = EF_BC1;
				}
			}

			static ElementFormat const convert_fmts[][2] =
			{
				{ EF_BC1, EF_ARGB8 },
				{ EF_BC1_SRGB, EF_ARGB8_SRGB },
				{ EF_BC2, EF_ARGB8 },
				{ EF_BC2_SRGB, EF_ARGB8_SRGB },
				{ EF_BC3, EF_ARGB8 },
				{ EF_BC3_SRGB, EF_ARGB8_SRGB },
				{ EF_BC4, EF_R8 },
				{ EF_BC4_SRGB, EF_R8 },
				{ EF_SIGNED_BC4, EF_SIGNED_R8 },
				{ EF_BC5, EF_GR8 },
				{ EF_BC5_SRGB, EF_GR8 },
				{ EF_SIGNED_BC5, EF_SIGNED_GR8 },
				{ EF_BC6, EF_ABGR16F },
				{ EF_SIGNED_BC6, EF_ABGR16F },
				{ EF_BC7, EF_ARGB8 },
				{ EF_BC7_SRGB, EF_ARGB8 },
				{ EF_ETC1, EF_ARGB8 },
				{ EF_ETC2_BGR8, EF_ARGB8 },
				{ EF_ETC2_BGR8_SRGB, EF_ARGB8_SRGB },
				{ EF_ETC2_A1BGR8, EF_ARGB8 },
				{ EF_ETC2_A1BGR8_SRGB, EF_ARGB8_SRGB },
				{ EF_ETC2_ABGR8, EF_ARGB8 },
				{ EF_ETC2_ABGR8_SRGB, EF_ARGB8_SRGB },
				{ EF_R8, EF_ARGB8 },
				{ EF_SIGNED_R8, EF_SIGNED_ABGR8 },
				{ EF_GR8, EF_ARGB8 },
				{ EF_SIGNED_GR8, EF_SIGNED_ABGR8 },
				{ EF_ARGB8_SRGB, EF_ARGB8 },
				{ EF_ARGB8, EF_ABGR8 },
				{ EF_R16, EF_R16F },
				{ EF_R16F, EF_R8 },
			};
			while (!caps.TextureFormatSupport(tex_data.format))
			{
				bool found = false;
				for (size_t i = 0; i < std::size(convert_fmts); ++ i)
				{
					if (convert_fmts[i][0] == tex_data.format)
					{
						tex_data.format = convert_fmts[i][1];
						found = true;
						break;
					}
				}

				if (!found)
				{
					LogError() << tex_desc_.res_name << "'s format (0x" << std::hex << static_cast<uint64_t>(tex_data.format)
						<< ") is not supported." << std::endl;
					break;
				}
			}

			*tex_desc_.tex = this->CreateTexture();
			return *tex_desc_.tex;
		}

		void SubThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);

			TexturePtr const & tex = *tex_desc_.tex;
			if (tex && tex->HWResourceReady())
			{
				return;
			}

			bool jit = false;
			if (ResLoader::Instance().Locate(tex_desc_.runtime_name).empty())
			{
				jit = true;
			}
			else
			{
				uint64_t const runtime_file_timestamp = ResLoader::Instance().Timestamp(tex_desc_.runtime_name);
				uint64_t const input_file_timestamp = ResLoader::Instance().Timestamp(tex_desc_.res_name);
				uint64_t const metadata_timestamp = ResLoader::Instance().Timestamp(tex_desc_.metadata_name);
				if (((input_file_timestamp > 0) && (runtime_file_timestamp < input_file_timestamp))
					|| (((metadata_timestamp > 0) && (runtime_file_timestamp < metadata_timestamp))))
				{
					jit = true;
				}
			}
			if (jit)
			{
#if KLAYGE_IS_DEV_PLATFORM
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

				Context::Instance().DevHelperInstance().ConvertTexture(tex_desc_.res_name, tex_desc_.metadata_name,
					tex_desc_.runtime_name, &caps);
#else
				LogError() << "Could NOT locate " << tex_desc_.runtime_name << std::endl;
#endif
			}

			this->LoadDDS();
		}

		void MainThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
			this->MainThreadStageNoLock();
		}

		bool HasSubThreadStage() const override
		{
			return true;
		}

		bool Match(ResLoadingDesc const & rhs) const override
		{
			if (this->Type() == rhs.Type())
			{
				TextureLoadingDesc const & tld = static_cast<TextureLoadingDesc const &>(rhs);
				return (tex_desc_.res_name == tld.tex_desc_.res_name)
					&& (tex_desc_.access_hint == tld.tex_desc_.access_hint);
			}
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs) override
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			TextureLoadingDesc const & tld = static_cast<TextureLoadingDesc const &>(rhs);
			tex_desc_.res_name = tld.tex_desc_.res_name;
			tex_desc_.access_hint = tld.tex_desc_.access_hint;
			tex_desc_.tex_data = tld.tex_desc_.tex_data;
			tex_desc_.tex = tld.tex_desc_.tex;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
		{
			return resource;
		}

		std::shared_ptr<void> Resource() const override
		{
			return *tex_desc_.tex;
		}

	private:
		void LoadDDS()
		{
			TexDesc::TexData& tex_data = *tex_desc_.tex_data;

			{
				TexturePtr tex = LoadSoftwareTexture(tex_desc_.runtime_name);
				tex_data.type = tex->Type();
				tex_data.width = tex->Width(0);
				tex_data.height = tex->Height(0);
				tex_data.depth = tex->Depth(0);
				tex_data.num_mipmaps = tex->NumMipMaps();
				tex_data.array_size = tex->ArraySize();
				tex_data.format = tex->Format();

				auto& sw_tex = checked_cast<SoftwareTexture&>(*tex);
				tex_data.init_data = sw_tex.SubresourceData();
				tex_data.data_block = sw_tex.DataBlock();

				for (size_t i = 0; i < tex_data.init_data.size(); ++ i)
				{
					size_t const offset = static_cast<uint8_t const *>(tex_data.init_data[i].data) - sw_tex.DataBlock().data();
					tex_data.init_data[i].data = tex_data.data_block.data() + offset;
				}
			}

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if ((Texture::TT_3D == tex_data.type) && (caps.max_texture_depth < tex_data.depth))
			{
				tex_data.type = Texture::TT_2D;
				tex_data.height *= tex_data.depth;
				tex_data.depth = 1;
				tex_data.num_mipmaps = 1;
				tex_data.init_data.resize(1);
			}

			uint32_t array_size = tex_data.array_size;
			if (Texture::TT_Cube == tex_data.type)
			{
				array_size *= 6;
			}

			if (((EF_BC5 == tex_data.format) && !caps.TextureFormatSupport(EF_BC5))
				|| ((EF_BC5_SRGB == tex_data.format) && !caps.TextureFormatSupport(EF_BC5_SRGB)))
			{
				BC1Block tmp;
				for (size_t i = 0; i < tex_data.init_data.size(); ++ i)
				{
					for (size_t j = 0; j < tex_data.init_data[i].slice_pitch; j += sizeof(BC4Block) * 2)
					{
						char* p = static_cast<char*>(const_cast<void*>(tex_data.init_data[i].data)) + j;

						BC4ToBC1G(tmp, *reinterpret_cast<BC4Block const *>(p + sizeof(BC4Block)));
						std::memcpy(p + sizeof(BC4Block), &tmp, sizeof(BC1Block));
					}
				}

				if (IsSRGB(tex_data.format))
				{
					tex_data.format = EF_BC3_SRGB;
				}
				else
				{
					tex_data.format = EF_BC3;
				}
			}
			if (((EF_BC4 == tex_data.format) && !caps.TextureFormatSupport(EF_BC4))
				|| ((EF_BC4_SRGB == tex_data.format) && !caps.TextureFormatSupport(EF_BC4_SRGB)))
			{
				BC1Block tmp;
				for (size_t i = 0; i < tex_data.init_data.size(); ++ i)
				{
					for (size_t j = 0; j < tex_data.init_data[i].slice_pitch; j += sizeof(BC4Block))
					{
						char* p = static_cast<char*>(const_cast<void*>(tex_data.init_data[i].data)) + j;

						BC4ToBC1G(tmp, *reinterpret_cast<BC4Block const *>(p));
						std::memcpy(p, &tmp, sizeof(BC1Block));
					}
				}

				if (IsSRGB(tex_data.format))
				{
					tex_data.format = EF_BC1_SRGB;
				}
				else
				{
					tex_data.format = EF_BC1;
				}
			}

			static ElementFormat const convert_fmts[][2] =
			{
				{ EF_BC1, EF_ARGB8 },
				{ EF_BC1_SRGB, EF_ARGB8_SRGB },
				{ EF_BC2, EF_ARGB8 },
				{ EF_BC2_SRGB, EF_ARGB8_SRGB },
				{ EF_BC3, EF_ARGB8 },
				{ EF_BC3_SRGB, EF_ARGB8_SRGB },
				{ EF_BC4, EF_R8 },
				{ EF_BC4_SRGB, EF_R8 },
				{ EF_SIGNED_BC4, EF_SIGNED_R8 },
				{ EF_BC5, EF_GR8 },
				{ EF_BC5_SRGB, EF_GR8 },
				{ EF_SIGNED_BC5, EF_SIGNED_GR8 },
				{ EF_BC6, EF_ABGR16F },
				{ EF_SIGNED_BC6, EF_ABGR16F },
				{ EF_BC7, EF_ARGB8 },
				{ EF_BC7_SRGB, EF_ARGB8 },
				{ EF_ETC1, EF_ARGB8 },
				{ EF_ETC2_BGR8, EF_ARGB8 },
				{ EF_ETC2_BGR8_SRGB, EF_ARGB8_SRGB },
				{ EF_ETC2_A1BGR8, EF_ARGB8 },
				{ EF_ETC2_A1BGR8_SRGB, EF_ARGB8_SRGB },
				{ EF_ETC2_ABGR8, EF_ARGB8 },
				{ EF_ETC2_ABGR8_SRGB, EF_ARGB8_SRGB },
				{ EF_R8, EF_ARGB8 },
				{ EF_SIGNED_R8, EF_SIGNED_ABGR8 },
				{ EF_GR8, EF_ARGB8 },
				{ EF_SIGNED_GR8, EF_SIGNED_ABGR8 },
				{ EF_ARGB8_SRGB, EF_ARGB8 },
				{ EF_ARGB8, EF_ABGR8 },
				{ EF_R16, EF_R16F },
				{ EF_R16F, EF_R8 },
			};
			while (!caps.TextureFormatSupport(tex_data.format))
			{
				bool found = false;
				for (size_t i = 0; i < std::size(convert_fmts); ++ i)
				{
					if (convert_fmts[i][0] == tex_data.format)
					{
						uint32_t const src_elem_size = NumFormatBytes(convert_fmts[i][0]);
						uint32_t const dst_elem_size = NumFormatBytes(convert_fmts[i][1]);

						bool needs_new_data_block = (src_elem_size < dst_elem_size)
							|| (IsCompressedFormat(convert_fmts[i][0]) && !IsCompressedFormat(convert_fmts[i][1]));

						std::vector<uint8_t> new_data_block;
						std::vector<uint32_t> new_sub_res_start;
						if (needs_new_data_block)
						{
							uint32_t new_data_block_size = 0;
							new_sub_res_start.resize(array_size * tex_data.num_mipmaps);

							for (size_t index = 0; index < array_size; ++ index)
							{
								uint32_t width = tex_data.width;
								uint32_t height = tex_data.height;
								for (size_t level = 0; level < tex_data.num_mipmaps; ++ level)
								{
									uint32_t slice_pitch;
									if (IsCompressedFormat(convert_fmts[i][1]))
									{
										slice_pitch = ((width + 3) & ~3) * (height + 3) / 4 * dst_elem_size;
									}
									else
									{
										slice_pitch = width * height * dst_elem_size;
									}

									size_t sub_res = index * tex_data.num_mipmaps + level;
									new_sub_res_start[sub_res] = new_data_block_size;
									new_data_block_size += slice_pitch;

									width = std::max<uint32_t>(1U, width / 2);
									height = std::max<uint32_t>(1U, height / 2);
								}
							}

							new_data_block.resize(new_data_block_size);
						}

						for (size_t index = 0; index < array_size; ++ index)
						{
							uint32_t width = tex_data.width;
							uint32_t height = tex_data.height;
							uint32_t depth = tex_data.depth;
							for (size_t level = 0; level < tex_data.num_mipmaps; ++ level)
							{
								uint32_t row_pitch, slice_pitch;
								if (IsCompressedFormat(convert_fmts[i][1]))
								{
									row_pitch = ((width + 3) & ~3) * dst_elem_size;
									slice_pitch = (height + 3) / 4 * row_pitch;
								}
								else
								{
									row_pitch = width * dst_elem_size;
									slice_pitch = height * row_pitch;
								}

								size_t sub_res = index * tex_data.num_mipmaps + level;
								uint8_t* sub_data_block;
								if (needs_new_data_block)
								{
									sub_data_block = &new_data_block[new_sub_res_start[sub_res]];
								}
								else
								{
									sub_data_block = static_cast<uint8_t*>(
										const_cast<void*>(tex_data.init_data[sub_res].data));
								}
								ResizeTexture(sub_data_block, row_pitch, slice_pitch,
									convert_fmts[i][1], width, height, depth,
									tex_data.init_data[sub_res].data,
									tex_data.init_data[sub_res].row_pitch,
									tex_data.init_data[sub_res].slice_pitch,
									convert_fmts[i][0], width, height, depth, false);

								width = std::max<uint32_t>(1U, width / 2);
								height = std::max<uint32_t>(1U, height / 2);
								depth = std::max<uint32_t>(1U, depth / 2);

								tex_data.init_data[sub_res].row_pitch = row_pitch;
								tex_data.init_data[sub_res].slice_pitch = slice_pitch;
								tex_data.init_data[sub_res].data = sub_data_block;
							}
						}

						if (needs_new_data_block)
						{
							tex_data.data_block.swap(new_data_block);
						}

						tex_data.format = convert_fmts[i][1];
						found = true;
						break;
					}
				}

				if (!found)
				{
					break;
				}
			}

			if (caps.multithread_res_creating_support)
			{
				this->MainThreadStageNoLock();
			}
		}

		TexturePtr CreateTexture()
		{
			TexDesc::TexData const & tex_data = *tex_desc_.tex_data;

			TexturePtr texture;
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			switch (tex_data.type)
			{
			case Texture::TT_1D:
				texture = rf.MakeDelayCreationTexture1D(tex_data.width, tex_data.num_mipmaps, tex_data.array_size,
					tex_data.format, 1, 0, tex_desc_.access_hint);
				break;

			case Texture::TT_2D:
				texture = rf.MakeDelayCreationTexture2D(tex_data.width, tex_data.height, tex_data.num_mipmaps, tex_data.array_size,
					tex_data.format, 1, 0, tex_desc_.access_hint);
				break;

			case Texture::TT_3D:
				texture = rf.MakeDelayCreationTexture3D(tex_data.width, tex_data.height, tex_data.depth, tex_data.num_mipmaps,
					tex_data.array_size, tex_data.format, 1, 0, tex_desc_.access_hint);
				break;

			case Texture::TT_Cube:
				texture = rf.MakeDelayCreationTextureCube(tex_data.width, tex_data.num_mipmaps, tex_data.array_size,
					tex_data.format, 1, 0, tex_desc_.access_hint);
				break;

			default:
				KFL_UNREACHABLE("Invalid texture type");
			}

			return texture;
		}

		void MainThreadStageNoLock()
		{
			TexturePtr const & tex = *tex_desc_.tex;
			if (!tex || !tex->HWResourceReady())
			{
				tex->CreateHWResource(tex_desc_.tex_data->init_data, nullptr);
				tex_desc_.tex_data.reset();
			}
		}

	private:
		TexDesc tex_desc_;
		std::mutex main_thread_stage_mutex_;
	};
}

namespace KlayGE
{
	Texture::Mapper::Mapper(Texture& tex, uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t width)
		: tex_(tex),
			mapped_array_index_(array_index),
			mapped_level_(level)
	{
		tex_.Map1D(array_index, level, tma, x_offset, width, data_);
		row_pitch_ = slice_pitch_ = width * NumFormatBytes(tex.Format());
	}

	Texture::Mapper::Mapper(Texture& tex, uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset,
		uint32_t width, uint32_t height)
		: tex_(tex),
			mapped_array_index_(array_index),
			mapped_level_(level)
	{
		tex_.Map2D(array_index, level, tma, x_offset, y_offset, width, height, data_, row_pitch_);

		uint32_t const block_height = BlockHeight(tex.Format());
		slice_pitch_ = (height + block_height - 1) / block_height * row_pitch_;
	}

	Texture::Mapper::Mapper(Texture& tex, uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth)
		: tex_(tex),
			mapped_array_index_(array_index),
			mapped_level_(level)
	{
		tex_.Map3D(array_index, level, tma, x_offset, y_offset, z_offset, width, height, depth, data_, row_pitch_, slice_pitch_);
	}

	Texture::Mapper::Mapper(Texture& tex, uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset,
		uint32_t width, uint32_t height)
		: tex_(tex),
			mapped_array_index_(array_index),
			mapped_face_(face),
			mapped_level_(level)
	{
		tex_.MapCube(array_index, face, level, tma, x_offset, y_offset, width, height, data_, row_pitch_);

		uint32_t const block_height = BlockHeight(tex.Format());
		slice_pitch_ = (height + block_height - 1) / block_height * row_pitch_;
	}

	Texture::Mapper::~Mapper()
	{
		switch (tex_.Type())
		{
		case TT_1D:
			tex_.Unmap1D(mapped_array_index_, mapped_level_);
			break;

		case TT_2D:
			tex_.Unmap2D(mapped_array_index_, mapped_level_);
			break;

		case TT_3D:
			tex_.Unmap3D(mapped_array_index_, mapped_level_);
			break;

		case TT_Cube:
			tex_.UnmapCube(mapped_array_index_, mapped_face_, mapped_level_);
			break;
		}
	}


	void ReadDdsFileHeader(ResIdentifierPtr const & tex_res, Texture::TextureType& type,
		uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
		ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		uint32_t magic;
		tex_res->read(&magic, sizeof(magic));
		magic = LE2Native(magic);
		BOOST_ASSERT((MakeFourCC<'D', 'D', 'S', ' '>::value) == magic);

		DDSSURFACEDESC2 desc;
		tex_res->read(&desc, sizeof(desc));
		desc.size = LE2Native(desc.size);
		desc.flags = LE2Native(desc.flags);
		desc.height = LE2Native(desc.height);
		desc.width = LE2Native(desc.width);
		desc.pitch = LE2Native(desc.pitch);
		desc.depth = LE2Native(desc.depth);
		desc.mip_map_count = LE2Native(desc.mip_map_count);
		for (uint32_t i = 0; i < std::size(desc.reserved1); ++ i)
		{
			desc.reserved1[i] = LE2Native(desc.reserved1[i]);
		}
		desc.pixel_format.size = LE2Native(desc.pixel_format.size);
		desc.pixel_format.flags = LE2Native(desc.pixel_format.flags);
		desc.pixel_format.four_cc = LE2Native(desc.pixel_format.four_cc);
		desc.pixel_format.rgb_bit_count = LE2Native(desc.pixel_format.rgb_bit_count);
		desc.pixel_format.r_bit_mask = LE2Native(desc.pixel_format.r_bit_mask);
		desc.pixel_format.g_bit_mask = LE2Native(desc.pixel_format.g_bit_mask);
		desc.pixel_format.b_bit_mask = LE2Native(desc.pixel_format.b_bit_mask);
		desc.pixel_format.rgb_alpha_bit_mask = LE2Native(desc.pixel_format.rgb_alpha_bit_mask);
		desc.dds_caps.caps1 = LE2Native(desc.dds_caps.caps1);
		desc.dds_caps.caps2 = LE2Native(desc.dds_caps.caps2);
		for (uint32_t i = 0; i < std::size(desc.dds_caps.reserved); ++ i)
		{
			desc.dds_caps.reserved[i] = LE2Native(desc.dds_caps.reserved[i]);
		}
		desc.reserved2 = LE2Native(desc.reserved2);

		DDS_HEADER_DXT10 desc10;
		if (MakeFourCC<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc)
		{
			tex_res->read(&desc10, sizeof(desc10));
			desc10.dxgi_format = LE2Native(desc10.dxgi_format);
			desc10.resource_dim = LE2Native(desc10.resource_dim);
			desc10.misc_flag = LE2Native(desc10.misc_flag);
			desc10.array_size = LE2Native(desc10.array_size);
			desc10.reserved = LE2Native(desc10.reserved);
			desc10.array_size = LE2Native(desc10.array_size);
			array_size = desc10.array_size;
		}
		else
		{
			std::memset(&desc10, 0, sizeof(desc10));
			array_size = 1;

			BOOST_ASSERT((desc.flags & DDSD_CAPS) != 0);
			BOOST_ASSERT((desc.flags & DDSD_PIXELFORMAT) != 0);
		}

		BOOST_ASSERT((desc.flags & DDSD_WIDTH) != 0);
		BOOST_ASSERT((desc.flags & DDSD_HEIGHT) != 0);

		if (0 == (desc.flags & DDSD_MIPMAPCOUNT))
		{
			desc.mip_map_count = 1;
		}

		format = EF_ARGB8;
		if ((desc.pixel_format.flags & DDSPF_FOURCC) != 0)
		{
			// From "Programming Guide for DDS", http://msdn.microsoft.com/en-us/library/bb943991.aspx
			switch (desc.pixel_format.four_cc)
			{
			case 36:
				format = EF_ABGR16;
				break;

			case 110:
				format = EF_SIGNED_ABGR16;
				break;

			case 111:
				format = EF_R16F;
				break;

			case 112:
				format = EF_GR16F;
				break;

			case 113:
				format = EF_ABGR16F;
				break;

			case 114:
				format = EF_R32F;
				break;

			case 115:
				format = EF_GR32F;
				break;

			case 116:
				format = EF_ABGR32F;
				break;

			case MakeFourCC<'D', 'X', 'T', '1'>::value:
				format = EF_BC1;
				break;

			case MakeFourCC<'D', 'X', 'T', '3'>::value:
				format = EF_BC2;
				break;

			case MakeFourCC<'D', 'X', 'T', '5'>::value:
				format = EF_BC3;
				break;

			case MakeFourCC<'B', 'C', '4', 'U'>::value:
			case MakeFourCC<'A', 'T', 'I', '1'>::value:
				format = EF_BC4;
				break;

			case MakeFourCC<'B', 'C', '4', 'S'>::value:
				format = EF_SIGNED_BC4;
				break;

			case MakeFourCC<'B', 'C', '5', 'U'>::value:
			case MakeFourCC<'A', 'T', 'I', '2'>::value:
				format = EF_BC5;
				break;

			case MakeFourCC<'B', 'C', '5', 'S'>::value:
				format = EF_SIGNED_BC5;
				break;

			case MakeFourCC<'D', 'X', '1', '0'>::value:
				format = FromDXGIFormat(desc10.dxgi_format);
				break;
			}
		}
		else
		{
			if ((desc.pixel_format.flags & DDSPF_RGB) != 0)
			{
				switch (desc.pixel_format.rgb_bit_count)
				{
				case 16:
					if ((0xF000 == desc.pixel_format.rgb_alpha_bit_mask)
						&& (0x0F00 == desc.pixel_format.r_bit_mask)
						&& (0x00F0 == desc.pixel_format.g_bit_mask)
						&& (0x000F == desc.pixel_format.b_bit_mask))
					{
						format = EF_ARGB4;
					}
					else
					{
						KFL_UNREACHABLE("Invalid format");
					}
					break;

				case 32:
					if ((0x00FF0000 == desc.pixel_format.r_bit_mask)
						&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
						&& (0x000000FF == desc.pixel_format.b_bit_mask))
					{
						format = EF_ARGB8;
					}
					else
					{
						if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
							&& (0x000003FF == desc.pixel_format.r_bit_mask)
							&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
							&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
						{
							format = EF_A2BGR10;
						}
						else
						{
							if ((0xFF000000 == desc.pixel_format.rgb_alpha_bit_mask)
								&& (0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
								&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								format = EF_ABGR8;
							}
							else
							{
								if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
									&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
									&& (0x00000000 == desc.pixel_format.b_bit_mask))
								{
									format = EF_GR16;
								}
								else
								{
									KFL_UNREACHABLE("Invalid format");
								}
							}
						}
					}
					break;

				default:
					KFL_UNREACHABLE("Invalid rgb bit count");
				}
			}
			else
			{
				if ((desc.pixel_format.flags & DDSPF_LUMINANCE) != 0)
				{
					switch (desc.pixel_format.rgb_bit_count)
					{
					case 8:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							format = EF_R8;
						}
						else
						{
							KFL_UNREACHABLE("Invalid format");
						}
						break;

					case 16:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							format = EF_R16;
						}
						else
						{
							KFL_UNREACHABLE("Invalid format");
						}
						break;

					default:
						KFL_UNREACHABLE("Invalid rgb bit count");
					}
				}
				else
				{
					if ((desc.pixel_format.flags & DDSPF_BUMPDUDV) != 0)
					{
						switch (desc.pixel_format.rgb_bit_count)
						{
						case 16:
							if ((0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask))
							{
								format = EF_SIGNED_GR8;
							}
							else
							{
								if (0x0000FFFF == desc.pixel_format.r_bit_mask)
								{
									format = EF_SIGNED_R16;
								}
								else
								{
									KFL_UNREACHABLE("Invalid format");
								}
							}
							break;

						case 32:
							if ((0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
								&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								format = EF_SIGNED_ABGR8;
							}
							else
							{
								if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x000003FF == desc.pixel_format.r_bit_mask)
									&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
									&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
								{
									format = EF_SIGNED_A2BGR10;
								}
								else
								{
									if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
										&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
										&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
										&& (0x00000000 == desc.pixel_format.b_bit_mask))
									{
										format = EF_SIGNED_GR16;
									}
									else
									{
										KFL_UNREACHABLE("Invalid format");
									}
								}
							}
							break;

						default:
							KFL_UNREACHABLE("Invalid rgb bit count");
							break;
						}
					}
					else
					{
						if ((desc.pixel_format.flags & DDSPF_ALPHA) != 0)
						{
							format = EF_A8;
						}
						else
						{
							KFL_UNREACHABLE("Invalid alpha format");
						}
					}
				}
			}
		}

		if ((desc.flags & DDSD_PITCH) != 0)
		{
			row_pitch = desc.pitch;
		}
		else
		{
			if ((desc.flags & desc.pixel_format.flags & 0x00000040) != 0)
			{
				row_pitch = desc.width * desc.pixel_format.rgb_bit_count / 8;
			}
			else
			{
				row_pitch = desc.width * NumFormatBytes(format);
			}
		}
		slice_pitch = row_pitch * desc.height;

		if (desc.reserved1[0] != 0)
		{
			format = MakeSRGB(format);
		}

		width = desc.width;
		num_mipmaps = desc.mip_map_count;

		if ((MakeFourCC<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc))
		{
			if (D3D_RESOURCE_MISC_TEXTURECUBE == desc10.misc_flag)
			{
				type = Texture::TT_Cube;
				array_size /= 6;
				height = desc.width;
				depth = 1;
			}
			else
			{
				switch (desc10.resource_dim)
				{
				case D3D_RESOURCE_DIMENSION_TEXTURE1D:
					type = Texture::TT_1D;
					height = 1;
					depth = 1;
					break;

				case D3D_RESOURCE_DIMENSION_TEXTURE2D:
					type = Texture::TT_2D;
					height = desc.height;
					depth = 1;
					break;

				case D3D_RESOURCE_DIMENSION_TEXTURE3D:
					type = Texture::TT_3D;
					height = desc.height;
					depth = desc.depth;
					break;

				default:
					KFL_UNREACHABLE("Invalid resource dimension");
				}
			}	
		}
		else
		{
			if ((desc.dds_caps.caps2 & DDSCAPS2_CUBEMAP) != 0)
			{
				type = Texture::TT_Cube;
				height = desc.width;
				depth = 1;
			}
			else
			{
				if ((desc.dds_caps.caps2 & DDSCAPS2_VOLUME) != 0)
				{
					type = Texture::TT_3D;
					height = desc.height;
					depth = desc.depth;
				}
				else
				{
					type = Texture::TT_2D;
					height = desc.height;
					depth = 1;
				}
			}
		}
	}

	void GetImageInfo(std::string_view tex_name, Texture::TextureType& type,
		uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
		ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		std::filesystem::path res_path(tex_name.begin(), tex_name.end());
		if (res_path.extension().string() != ".dds")
		{
#if KLAYGE_IS_DEV_PLATFORM
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

			std::string const metadata_name = std::string(tex_name) + ".kmeta";
			Context::Instance().DevHelperInstance().GetImageInfo(ResLoader::Instance().Locate(tex_name), metadata_name, &caps,
				type, width, height, depth, num_mipmaps, array_size, format, row_pitch, slice_pitch);
#else
			LogError() << "The file type is not supported: " << tex_name << std::endl;
#endif
		}
		else
		{
			ResIdentifierPtr tex_res = ResLoader::Instance().Open(tex_name);
			ReadDdsFileHeader(tex_res, type, width, height, depth, num_mipmaps, array_size, format,
				row_pitch, slice_pitch);
		}
	}

	TexturePtr LoadSoftwareTexture(std::string_view tex_name)
	{
		if (ResLoader::Instance().Locate(tex_name).empty())
		{
			return TexturePtr();
		}

		ResIdentifierPtr tex_res = ResLoader::Instance().Open(tex_name);

		Texture::TextureType type;
		uint32_t width, height, depth;
		uint32_t num_mipmaps;
		uint32_t array_size;
		ElementFormat format;
		std::vector<ElementInitData> init_data;
		std::vector<uint8_t> data_block;

		uint32_t row_pitch, slice_pitch;
		ReadDdsFileHeader(tex_res, type, width, height, depth, num_mipmaps, array_size, format,
			row_pitch, slice_pitch);

		uint32_t const fmt_size = NumFormatBytes(format);
		bool padding = false;
		if (!IsCompressedFormat(format))
		{
			if (row_pitch != width * fmt_size)
			{
				BOOST_ASSERT(row_pitch == ((width + 3) & ~3) * fmt_size);
				padding = true;
			}
		}

		std::vector<size_t> base;
		switch (type)
		{
		case Texture::TT_1D:
			{
				init_data.resize(array_size * num_mipmaps);
				base.resize(array_size * num_mipmaps);
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					uint32_t the_width = width;
					for (uint32_t level = 0; level < num_mipmaps; ++ level)
					{
						size_t const index = array_index * num_mipmaps + level;
						uint32_t image_size;
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							image_size = ((the_width + 3) / 4) * block_size;
						}
						else
						{
							image_size = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
						}

						base[index] = data_block.size();
						data_block.resize(base[index] + image_size);
						init_data[index].row_pitch = image_size;
						init_data[index].slice_pitch = image_size;

						tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(image_size));
						BOOST_ASSERT(tex_res->gcount() == static_cast<int>(image_size));

						the_width = std::max<uint32_t>(the_width / 2, 1);
					}
				}
			}
			break;

		case Texture::TT_2D:
			{
				init_data.resize(array_size * num_mipmaps);
				base.resize(array_size * num_mipmaps);
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					uint32_t the_width = width;
					uint32_t the_height = height;
					for (uint32_t level = 0; level < num_mipmaps; ++ level)
					{
						size_t const index = array_index * num_mipmaps + level;
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

							base[index] = data_block.size();
							data_block.resize(base[index] + image_size);
							init_data[index].row_pitch = (the_width + 3) / 4 * block_size;
							init_data[index].slice_pitch = image_size;

							tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(image_size));
							BOOST_ASSERT(tex_res->gcount() == static_cast<int>(image_size));
						}
						else
						{
							init_data[index].row_pitch = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
							init_data[index].slice_pitch = init_data[index].row_pitch * the_height;
							base[index] = data_block.size();
							data_block.resize(base[index] + init_data[index].slice_pitch);

							tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(init_data[index].slice_pitch));
							BOOST_ASSERT(tex_res->gcount() == static_cast<int>(init_data[index].slice_pitch));
						}

						the_width = std::max<uint32_t>(the_width / 2, 1);
						the_height = std::max<uint32_t>(the_height / 2, 1);
					}
				}
			}
			break;

		case Texture::TT_3D:
			{
				init_data.resize(array_size * num_mipmaps);
				base.resize(array_size * num_mipmaps);
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					uint32_t the_width = width;
					uint32_t the_height = height;
					uint32_t the_depth = depth;
					for (uint32_t level = 0; level < num_mipmaps; ++ level)
					{
						size_t const index = array_index * num_mipmaps + level;
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * the_depth * block_size;

							base[index] = data_block.size();
							data_block.resize(base[index] + image_size);
							init_data[index].row_pitch = (the_width + 3) / 4 * block_size;
							init_data[index].slice_pitch = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

							tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(image_size));
							BOOST_ASSERT(tex_res->gcount() == static_cast<int>(image_size));
						}
						else
						{
							init_data[index].row_pitch = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
							init_data[index].slice_pitch = init_data[index].row_pitch * the_height;
							base[index] = data_block.size();
							data_block.resize(base[index] + init_data[index].slice_pitch * the_depth);

							tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(init_data[index].slice_pitch * the_depth));
							BOOST_ASSERT(tex_res->gcount() == static_cast<int>(init_data[index].slice_pitch * the_depth));
						}

						the_width = std::max<uint32_t>(the_width / 2, 1);
						the_height = std::max<uint32_t>(the_height / 2, 1);
						the_depth = std::max<uint32_t>(the_depth / 2, 1);
					}
				}
			}
			break;

		case Texture::TT_Cube:
			{
				init_data.resize(array_size * 6 * num_mipmaps);
				base.resize(array_size * 6 * num_mipmaps);
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					for (uint32_t face = Texture::CF_Positive_X; face <= Texture::CF_Negative_Z; ++ face)
					{
						uint32_t the_width = width;
						uint32_t the_height = height;
						for (uint32_t level = 0; level < num_mipmaps; ++ level)
						{
							size_t const index = (array_index * 6 + face - Texture::CF_Positive_X) * num_mipmaps + level;
							if (IsCompressedFormat(format))
							{
								uint32_t const block_size = NumFormatBytes(format) * 4;
								uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

								base[index] = data_block.size();
								data_block.resize(base[index] + image_size);
								init_data[index].row_pitch = (the_width + 3) / 4 * block_size;
								init_data[index].slice_pitch = image_size;

								tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(image_size));
								BOOST_ASSERT(tex_res->gcount() == static_cast<int>(image_size));
							}
							else
							{
								init_data[index].row_pitch = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
								init_data[index].slice_pitch = init_data[index].row_pitch * the_width;
								base[index] = data_block.size();
								data_block.resize(base[index] + init_data[index].slice_pitch);

								tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(init_data[index].slice_pitch));
								BOOST_ASSERT(tex_res->gcount() == static_cast<int>(init_data[index].slice_pitch));
							}

							the_width = std::max<uint32_t>(the_width / 2, 1);
							the_height = std::max<uint32_t>(the_height / 2, 1);
						}
					}
				}
			}
			break;
		}

		for (size_t i = 0; i < base.size(); ++ i)
		{
			init_data[i].data = &data_block[base[i]];
		}

		auto ret = MakeSharedPtr<SoftwareTexture>(type, width, height, depth,
			num_mipmaps, array_size, format, false);
		ret->CreateHWResource(init_data, nullptr);
		return ret;
	}

	TexturePtr SyncLoadTexture(std::string_view tex_name, uint32_t access_hint)
	{
		return ResLoader::Instance().SyncQueryT<Texture>(MakeSharedPtr<TextureLoadingDesc>(tex_name, access_hint));
	}

	TexturePtr ASyncLoadTexture(std::string_view tex_name, uint32_t access_hint)
	{
		return ResLoader::Instance().ASyncQueryT<Texture>(MakeSharedPtr<TextureLoadingDesc>(tex_name, access_hint));
	}

	void SaveTexture(std::string const & tex_name, Texture::TextureType type,
		uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size,
		ElementFormat format, std::span<ElementInitData const> init_data)
	{
		std::ofstream file(tex_name.c_str(), std::ios_base::binary);
		if (!file)
		{
			file.open((ResLoader::Instance().LocalFolder() + tex_name).c_str(), std::ios_base::binary);
		}

		uint32_t magic = Native2LE(MakeFourCC<'D', 'D', 'S', ' '>::value);
		file.write(reinterpret_cast<char*>(&magic), sizeof(magic));

		DDSSURFACEDESC2 desc;
		std::memset(&desc, 0, sizeof(desc));

		desc.size = sizeof(desc);

		desc.flags |= DDSD_WIDTH;
		desc.flags |= DDSD_HEIGHT;

		desc.width = width;
		desc.height = height;

		if (numMipMaps != 0)
		{
			desc.flags |= DDSD_MIPMAPCOUNT;
			desc.mip_map_count = numMipMaps;
		}

		desc.pixel_format.size = sizeof(desc.pixel_format);

		if (IsSRGB(format))
		{
			desc.reserved1[0] = 1;
		}

		if (array_size != 1)
		{
			desc.pixel_format.flags |= DDSPF_FOURCC;
			desc.pixel_format.four_cc = MakeFourCC<'D', 'X', '1', '0'>::value;
		}
		else
		{
			if ((EF_ABGR16 == format)
				|| IsFloatFormat(format) || IsCompressedFormat(format))
			{
				desc.pixel_format.flags |= DDSPF_FOURCC;

				switch (format)
				{
				case EF_ABGR16:
					desc.pixel_format.four_cc = 36;
					break;

				case EF_SIGNED_ABGR16:
					desc.pixel_format.four_cc = 110;
					break;

				case EF_R16F:
					desc.pixel_format.four_cc = 111;
					break;

				case EF_GR16F:
					desc.pixel_format.four_cc = 112;
					break;

				case EF_ABGR16F:
					desc.pixel_format.four_cc = 113;
					break;

				case EF_R32F:
					desc.pixel_format.four_cc = 114;
					break;

				case EF_GR32F:
					desc.pixel_format.four_cc = 115;
					break;

				case EF_ABGR32F:
					desc.pixel_format.four_cc = 116;
					break;

				case EF_BC1:
				case EF_BC1_SRGB:
					desc.pixel_format.four_cc = MakeFourCC<'D', 'X', 'T', '1'>::value;
					break;

				case EF_BC2:
				case EF_BC2_SRGB:
					desc.pixel_format.four_cc = MakeFourCC<'D', 'X', 'T', '3'>::value;
					break;

				case EF_BC3:
				case EF_BC3_SRGB:
					desc.pixel_format.four_cc = MakeFourCC<'D', 'X', 'T', '5'>::value;
					break;

				case EF_BC4:
				case EF_BC4_SRGB:
					desc.pixel_format.four_cc = MakeFourCC<'B', 'C', '4', 'U'>::value;
					break;

				case EF_SIGNED_BC4:
					desc.pixel_format.four_cc = MakeFourCC<'B', 'C', '4', 'S'>::value;
					break;

				case EF_BC5:
				case EF_BC5_SRGB:
					desc.pixel_format.four_cc = MakeFourCC<'B', 'C', '5', 'U'>::value;
					break;

				case EF_SIGNED_BC5:
					desc.pixel_format.four_cc = MakeFourCC<'B', 'C', '5', 'S'>::value;
					break;

				case EF_B10G11R11F:
				case EF_BC6:
				case EF_SIGNED_BC6:
				case EF_BC7:
				case EF_BC7_SRGB:
				case EF_ETC1:
				case EF_ETC2_R11:
				case EF_SIGNED_ETC2_R11:
				case EF_ETC2_GR11:
				case EF_SIGNED_ETC2_GR11:
				case EF_ETC2_BGR8:
				case EF_ETC2_BGR8_SRGB:
				case EF_ETC2_A1BGR8:
				case EF_ETC2_A1BGR8_SRGB:
				case EF_ETC2_ABGR8:
				case EF_ETC2_ABGR8_SRGB:
					desc.pixel_format.four_cc = MakeFourCC<'D', 'X', '1', '0'>::value;
					break;

				default:
					KFL_UNREACHABLE("Invalid element format");
				}
			}
			else
			{
				switch (format)
				{
				case EF_ARGB4:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
					desc.pixel_format.rgb_bit_count = 16;

					desc.pixel_format.rgb_alpha_bit_mask = 0x0000F000;
					desc.pixel_format.r_bit_mask = 0x00000F00;
					desc.pixel_format.g_bit_mask = 0x000000F0;
					desc.pixel_format.b_bit_mask = 0x0000000F;
					break;

				case EF_GR8:
					desc.pixel_format.flags |= DDSPF_FOURCC;
					desc.pixel_format.four_cc = MakeFourCC<'D', 'X', '1', '0'>::value;
					break;

				case EF_SIGNED_GR8:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 16;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x000000FF;
					desc.pixel_format.g_bit_mask = 0x0000FF00;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_SIGNED_R16:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 16;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x0000FFFF;
					desc.pixel_format.g_bit_mask = 0x00000000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_ARGB8:
				case EF_ARGB8_SRGB:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
					desc.pixel_format.r_bit_mask = 0x00FF0000;
					desc.pixel_format.g_bit_mask = 0x0000FF00;
					desc.pixel_format.b_bit_mask = 0x000000FF;
					break;

				case EF_ABGR8:
				case EF_ABGR8_SRGB:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
					desc.pixel_format.r_bit_mask = 0x000000FF;
					desc.pixel_format.g_bit_mask = 0x0000FF00;
					desc.pixel_format.b_bit_mask = 0x00FF0000;
					break;

				case EF_SIGNED_ABGR8:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xFF000000;
					desc.pixel_format.r_bit_mask = 0x000000FF;
					desc.pixel_format.g_bit_mask = 0x0000FF00;
					desc.pixel_format.b_bit_mask = 0x00FF0000;
					break;

				case EF_A2BGR10:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.flags |= DDSPF_ALPHAPIXELS;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xC0000000;
					desc.pixel_format.r_bit_mask = 0x000003FF;
					desc.pixel_format.g_bit_mask = 0x000FFC00;
					desc.pixel_format.b_bit_mask = 0x3FF00000;
					break;

				case EF_SIGNED_A2BGR10:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0xC0000000;
					desc.pixel_format.r_bit_mask = 0x000003FF;
					desc.pixel_format.g_bit_mask = 0x000FFC00;
					desc.pixel_format.b_bit_mask = 0x3FF00000;
					break;

				case EF_GR16:
					desc.pixel_format.flags |= DDSPF_RGB;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x0000FFFF;
					desc.pixel_format.g_bit_mask = 0xFFFF0000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_SIGNED_GR16:
					desc.pixel_format.flags |= DDSPF_BUMPDUDV;
					desc.pixel_format.rgb_bit_count = 32;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x0000FFFF;
					desc.pixel_format.g_bit_mask = 0xFFFF0000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_R8:
					desc.pixel_format.flags |= DDSPF_LUMINANCE;
					desc.pixel_format.rgb_bit_count = 8;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x000000FF;
					desc.pixel_format.g_bit_mask = 0x00000000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_R16:
					desc.pixel_format.flags |= DDSPF_LUMINANCE;
					desc.pixel_format.rgb_bit_count = 16;

					desc.pixel_format.rgb_alpha_bit_mask = 0x00000000;
					desc.pixel_format.r_bit_mask = 0x0000FFFF;
					desc.pixel_format.g_bit_mask = 0x00000000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				case EF_A8:
					desc.pixel_format.flags |= DDSPF_ALPHA;
					desc.pixel_format.rgb_bit_count = 8;

					desc.pixel_format.rgb_alpha_bit_mask = 0x000000FF;
					desc.pixel_format.r_bit_mask = 0x00000000;
					desc.pixel_format.g_bit_mask = 0x00000000;
					desc.pixel_format.b_bit_mask = 0x00000000;
					break;

				default:
					KFL_UNREACHABLE("Invalid element format");
				}
			}
		}
		
		if (desc.pixel_format.four_cc != MakeFourCC<'D', 'X', '1', '0'>::value)
		{
			desc.flags |= DDSD_CAPS;
			desc.flags |= DDSD_PIXELFORMAT;

			desc.dds_caps.caps1 = DDSCAPS_TEXTURE;
			if (numMipMaps != 1)
			{
				desc.dds_caps.caps1 |= DDSCAPS_MIPMAP;
				desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
			}
			if (Texture::TT_3D == type)
			{
				desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
				desc.dds_caps.caps2 |= DDSCAPS2_VOLUME;
				desc.flags |= DDSD_DEPTH;
				desc.depth = depth;
			}
			if (Texture::TT_Cube == type)
			{
				desc.dds_caps.caps1 |= DDSCAPS_COMPLEX;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEX;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEX;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEY;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEY;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_POSITIVEZ;
				desc.dds_caps.caps2 |= DDSCAPS2_CUBEMAP_NEGATIVEZ;
			}
		}

		uint32_t format_size = NumFormatBytes(format);
		if (IsCompressedFormat(format))
		{
			uint32_t const block_size = NumFormatBytes(format) * 4;
			uint32_t image_size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;

			desc.flags |= DDSD_LINEARSIZE;
			desc.linear_size = image_size;
		}

		DDSSURFACEDESC2 desc_le = desc;
		desc_le.size = Native2LE(desc_le.size);
		desc_le.flags = Native2LE(desc_le.flags);
		desc_le.height = Native2LE(desc_le.height);
		desc_le.width = Native2LE(desc_le.width);
		desc_le.pitch = Native2LE(desc_le.pitch);
		desc_le.depth = Native2LE(desc_le.depth);
		desc_le.mip_map_count = Native2LE(desc_le.mip_map_count);
		for (uint32_t i = 0; i < std::size(desc_le.reserved1); ++ i)
		{
			desc_le.reserved1[i] = Native2LE(desc_le.reserved1[i]);
		}
		desc_le.pixel_format.size = Native2LE(desc_le.pixel_format.size);
		desc_le.pixel_format.flags = Native2LE(desc_le.pixel_format.flags);
		desc_le.pixel_format.four_cc = Native2LE(desc_le.pixel_format.four_cc);
		desc_le.pixel_format.rgb_bit_count = Native2LE(desc_le.pixel_format.rgb_bit_count);
		desc_le.pixel_format.r_bit_mask = Native2LE(desc_le.pixel_format.r_bit_mask);
		desc_le.pixel_format.g_bit_mask = Native2LE(desc_le.pixel_format.g_bit_mask);
		desc_le.pixel_format.b_bit_mask = Native2LE(desc_le.pixel_format.b_bit_mask);
		desc_le.pixel_format.rgb_alpha_bit_mask = Native2LE(desc_le.pixel_format.rgb_alpha_bit_mask);
		desc_le.dds_caps.caps1 = Native2LE(desc_le.dds_caps.caps1);
		desc_le.dds_caps.caps2 = Native2LE(desc_le.dds_caps.caps2);
		for (uint32_t i = 0; i < std::size(desc_le.dds_caps.reserved); ++ i)
		{
			desc_le.dds_caps.reserved[i] = Native2LE(desc_le.dds_caps.reserved[i]);
		}
		desc_le.reserved2 = Native2LE(desc_le.reserved2);
		file.write(reinterpret_cast<char*>(&desc_le), sizeof(desc_le));

		if (MakeFourCC<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc)
		{
			DDS_HEADER_DXT10 desc10;
			desc10.dxgi_format = ToDXGIFormat(format);
			desc10.misc_flag = 0;
			switch (type)
			{
			case Texture::TT_1D:
				desc10.resource_dim = D3D_RESOURCE_DIMENSION_TEXTURE1D;
				break;
			
			case Texture::TT_2D:
				desc10.resource_dim = D3D_RESOURCE_DIMENSION_TEXTURE2D;
				break;
			
			case Texture::TT_3D:
				desc10.resource_dim = D3D_RESOURCE_DIMENSION_TEXTURE3D;
				break;
			
			case Texture::TT_Cube:
				desc10.resource_dim = D3D_RESOURCE_DIMENSION_TEXTURE2D;
				desc10.misc_flag = D3D_RESOURCE_MISC_TEXTURECUBE;
				break;

			default:
				KFL_UNREACHABLE("Invalid texture type");
			}
			desc10.array_size = array_size;
			if (Texture::TT_Cube == type)
			{
				desc10.array_size *= 6;
			}
			desc10.reserved = 0;

			desc10.dxgi_format = Native2LE(desc10.dxgi_format);
			desc10.resource_dim = Native2LE(desc10.resource_dim);
			desc10.misc_flag = Native2LE(desc10.misc_flag);
			desc10.array_size = Native2LE(desc10.array_size);
			desc10.reserved = Native2LE(desc10.reserved);
			desc10.array_size = Native2LE(desc10.array_size);
			file.write(reinterpret_cast<char*>(&desc10), sizeof(desc10));
		}

		switch (type)
		{
		case Texture::TT_1D:
			{
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					uint32_t the_width = width;
					for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
					{
						size_t const index = array_index * desc.mip_map_count + level;
						uint32_t image_size;
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							image_size = ((the_width + 3) / 4) * block_size;
						}
						else
						{
							image_size = the_width * format_size;
						}

						file.write(reinterpret_cast<char const *>(init_data[index].data), static_cast<std::streamsize>(image_size));

						the_width = std::max<uint32_t>(the_width / 2, 1);
					}
				}
			}
			break;

		case Texture::TT_2D:
			{
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					uint32_t the_width = width;
					uint32_t the_height = height;
					for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
					{
						size_t const index = array_index * desc.mip_map_count + level;
						uint32_t image_size;
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;
						}
						else
						{
							image_size = the_width * the_height * format_size;
						}

						file.write(reinterpret_cast<char const *>(init_data[index].data), static_cast<std::streamsize>(image_size));

						the_width = std::max<uint32_t>(the_width / 2, 1);
						the_height = std::max<uint32_t>(the_height / 2, 1);
					}
				}
			}
			break;

		case Texture::TT_3D:
			{
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					uint32_t the_width = width;
					uint32_t the_height = height;
					uint32_t the_depth = depth;
					for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
					{
						size_t const index = array_index * desc.mip_map_count + level;
						uint32_t image_size;
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * the_depth * block_size;
						}
						else
						{
							image_size = the_width * the_height * the_depth * format_size;
						}

						file.write(reinterpret_cast<char const *>(init_data[index].data), static_cast<std::streamsize>(image_size));

						the_width = std::max<uint32_t>(the_width / 2, 1);
						the_height = std::max<uint32_t>(the_height / 2, 1);
						the_depth = std::max<uint32_t>(the_depth / 2, 1);
					}
				}
			}
			break;

		case Texture::TT_Cube:
			{
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					for (uint32_t face = Texture::CF_Positive_X; face <= Texture::CF_Negative_Z; ++ face)
					{
						uint32_t the_width = width;
						for (uint32_t level = 0; level < desc.mip_map_count; ++ level)
						{
							size_t const index = (array_index * 6 + face - Texture::CF_Positive_X) * numMipMaps + level;
							uint32_t image_size;
							if (IsCompressedFormat(format))
							{
								uint32_t const block_size = NumFormatBytes(format) * 4;
								image_size = ((the_width + 3) / 4) * ((the_width + 3) / 4) * block_size;
							}
							else
							{
								image_size = the_width * the_width * format_size;
							}

							file.write(reinterpret_cast<char const *>(init_data[index].data), static_cast<std::streamsize>(image_size));

							the_width = std::max<uint32_t>(the_width / 2, 1);
						}
					}
				}
			}
			break;
		}
	}

	// 把纹理保存入DDS文件
	void SaveTexture(TexturePtr const & texture, std::string const & tex_name)
	{
		ElementFormat format = texture->Format();
		uint32_t numMipMaps = texture->NumMipMaps();
		uint32_t array_size = texture->ArraySize();

		TexturePtr texture_sys_mem;
		if (texture->AccessHint() & EAH_CPU_Read)
		{
			texture_sys_mem = texture;
		}
		else
		{
			RenderFactory& renderFactory = Context::Instance().RenderFactoryInstance();

			switch (texture->Type())
			{
			case Texture::TT_1D:
				texture_sys_mem = renderFactory.MakeTexture1D(texture->Width(0),
					numMipMaps, array_size, format, 1, 0, EAH_CPU_Read);
				break;

			case Texture::TT_2D:
				texture_sys_mem = renderFactory.MakeTexture2D(texture->Width(0), texture->Height(0),
					numMipMaps, array_size, format, 1, 0, EAH_CPU_Read);
				break;

			case Texture::TT_3D:
				texture_sys_mem = renderFactory.MakeTexture3D(texture->Width(0), texture->Height(0),
					texture->Depth(0), numMipMaps, array_size, format, 1, 0, EAH_CPU_Read);
				break;

			case Texture::TT_Cube:
				texture_sys_mem = renderFactory.MakeTextureCube(texture->Width(0),
					numMipMaps, array_size, format, 1, 0, EAH_CPU_Read);
				break;

			default:
				KFL_UNREACHABLE("Invalid texture type");
			}
			texture->CopyToTexture(*texture_sys_mem);
		}

		uint32_t const format_size = NumFormatBytes(format);

		std::vector<ElementInitData> init_data;
		std::vector<size_t> base;
		std::vector<uint8_t> data_block;
		switch (texture_sys_mem->Type())
		{
		case Texture::TT_1D:
			{
				init_data.resize(array_size * numMipMaps);
				base.resize(array_size * numMipMaps);
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					for (uint32_t level = 0; level < numMipMaps; ++ level)
					{
						size_t const index = array_index * numMipMaps + level;
						uint32_t const width = texture_sys_mem->Width(level);
						uint32_t image_size;
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							image_size = ((width + 3) / 4) * block_size;
						}
						else
						{
							image_size = width * format_size;
						}

						{
							Texture::Mapper mapper(*texture_sys_mem, array_index, level, TMA_Read_Only, 0, width);
							base[index] = data_block.size();
							data_block.resize(data_block.size() + image_size);
							std::memcpy(&data_block[base[index]], mapper.Pointer<char>(), image_size);
						}
					}
				}
			}
			break;

		case Texture::TT_2D:
			{
				init_data.resize(array_size * numMipMaps);
				base.resize(array_size * numMipMaps);
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					for (uint32_t level = 0; level < numMipMaps; ++ level)
					{
						size_t const index = array_index * numMipMaps + level;
						uint32_t const width = texture_sys_mem->Width(level);
						uint32_t const height = texture_sys_mem->Height(level);
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							uint32_t image_size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;

							{
								Texture::Mapper mapper(*texture_sys_mem, array_index, level, TMA_Read_Only, 0, 0, width, height);
								char const * data = mapper.Pointer<char>();

								base[index] = data_block.size();
								data_block.resize(data_block.size() + image_size);
								for (uint32_t y = 0; y < (height + 3) / 4; ++ y)
								{
									std::memcpy(&data_block[base[index] + y * ((width + 3) / 4) * block_size], data, (width + 3) / 4 * block_size);
									data += mapper.RowPitch();
								}
							}
						}
						else
						{
							Texture::Mapper mapper(*texture_sys_mem, array_index, level, TMA_Read_Only, 0, 0, width, height);
							char const * data = mapper.Pointer<char>();

							base[index] = data_block.size();
							data_block.resize(data_block.size() + width * height * format_size);
							for (uint32_t y = 0; y < height; ++ y)
							{
								std::memcpy(&data_block[base[index] + y * width * format_size], data, width * format_size);
								data += mapper.RowPitch();
							}
						}
					}
				}
			}
			break;

		case Texture::TT_3D:
			{
				init_data.resize(array_size * numMipMaps);
				base.resize(array_size * numMipMaps);
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					for (uint32_t level = 0; level < numMipMaps; ++ level)
					{
						size_t const index = array_index * numMipMaps + level;
						uint32_t const width = texture_sys_mem->Width(level);
						uint32_t const height = texture_sys_mem->Height(level);
						uint32_t const depth = texture_sys_mem->Depth(level);
						if (IsCompressedFormat(format))
						{
							uint32_t const block_size = NumFormatBytes(format) * 4;
							uint32_t image_size = ((width + 3) / 4) * ((height + 3) / 4) * depth * block_size;

							{
								Texture::Mapper mapper(*texture_sys_mem, array_index, level, TMA_Read_Only, 0, 0, width, height);
								char const * data = mapper.Pointer<char>();

								base[index] = data_block.size();
								data_block.resize(data_block.size() + image_size);
								for (uint32_t z = 0; z < (depth + 3) / 4; ++ z)
								{
									for (uint32_t y = 0; y < (height + 3) / 4; ++ y)
									{
										std::memcpy(&data_block[base[index] + (z * ((height + 3) / 4) + y) * ((width + 3) / 4) * block_size], data, (width + 3) / 4 * block_size);
										data += mapper.RowPitch();
									}

									data += mapper.SlicePitch() - mapper.RowPitch() * ((height + 3) / 4);
								}
							}
						}
						else
						{
							Texture::Mapper mapper(*texture_sys_mem, array_index, level, TMA_Read_Only, 0, 0, 0, width, height, depth);
							char const * data = mapper.Pointer<char>();

							base[index] = data_block.size();
							data_block.resize(data_block.size() + width * height * depth * format_size);
							for (uint32_t z = 0; z < depth; ++ z)
							{
								for (uint32_t y = 0; y < height; ++ y)
								{
									std::memcpy(&data_block[base[index] + (z * height + y) * width * format_size], data, width * format_size);
									data += mapper.RowPitch();
								}

								data += mapper.SlicePitch() - mapper.RowPitch() * height;
							}
						}
					}
				}
			}
			break;

		case Texture::TT_Cube:
			{
				init_data.resize(array_size * 6 * numMipMaps);
				base.resize(array_size * 6 * numMipMaps);
				for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
				{
					for (uint32_t face = Texture::CF_Positive_X; face <= Texture::CF_Negative_Z; ++ face)
					{
						for (uint32_t level = 0; level < numMipMaps; ++ level)
						{
							size_t const index = (array_index * 6 + (face - Texture::CF_Positive_X)) * numMipMaps + level;
							uint32_t const width = texture_sys_mem->Width(level);
							uint32_t const height = texture_sys_mem->Height(level);
							if (IsCompressedFormat(format))
							{
								uint32_t const block_size = NumFormatBytes(format) * 4;
								uint32_t image_size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;

								{
									Texture::Mapper mapper(*texture_sys_mem, array_index, static_cast<Texture::CubeFaces>(face), level, TMA_Read_Only, 0, 0, width, height);
									char const * data = mapper.Pointer<char>();

									base[index] = data_block.size();
									data_block.resize(data_block.size() + image_size);
									for (uint32_t y = 0; y < (height + 3) / 4; ++ y)
									{
										std::memcpy(&data_block[base[index] + y * ((width + 3) / 4) * block_size], data, (width + 3) / 4 * block_size);
										data += mapper.RowPitch();
									}
								}
							}
							else
							{
								Texture::Mapper mapper(*texture_sys_mem, array_index, static_cast<Texture::CubeFaces>(face), level, TMA_Read_Only, 0, 0, width, height);
								char const * data = mapper.Pointer<char>();

								base[index] = data_block.size();
								data_block.resize(data_block.size() + width * height * format_size);
								for (uint32_t y = 0; y < height; ++ y)
								{
									std::memcpy(&data_block[base[index] + y * width * format_size], data, width * format_size);
									data += mapper.RowPitch();
								}
							}
						}
					}
				}
			}
			break;
		}

		for (size_t i = 0; i < base.size(); ++ i)
		{
			init_data[i].data = &data_block[base[i]];
		}

		SaveTexture(tex_name, texture_sys_mem->Type(),
			texture_sys_mem->Width(0), texture_sys_mem->Height(0), texture_sys_mem->Depth(0),
			numMipMaps, array_size, format, init_data);
	}


	Texture::Texture(Texture::TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
			: type_(type), sample_count_(sample_count), sample_quality_(sample_quality), access_hint_(access_hint)
	{
	}

	Texture::~Texture()
	{
	}

	uint32_t Texture::NumMipMaps() const
	{
		return num_mip_maps_;
	}

	uint32_t Texture::ArraySize() const
	{
		return array_size_;
	}

	ElementFormat Texture::Format() const
	{
		return format_;
	}

	Texture::TextureType Texture::Type() const
	{
		return type_;
	}

	uint32_t Texture::SampleCount() const
	{
		return sample_count_;
	}

	uint32_t Texture::SampleQuality() const
	{
		return sample_quality_;
	}

	uint32_t Texture::AccessHint() const
	{
		return access_hint_;
	}

	void Texture::ResizeTexture1D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width,
		bool linear)
	{
		BOOST_ASSERT(TT_1D == this->Type());
		BOOST_ASSERT(TT_1D == target.Type());

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		TexturePtr src_cpu;
		Texture* src_cpu_ptr;
		uint32_t src_cpu_array_index;
		uint32_t src_cpu_level;
		uint32_t src_cpu_x_offset;
		if (this->AccessHint() & EAH_CPU_Read)
		{
			src_cpu_ptr = this;
			src_cpu_array_index = src_array_index;
			src_cpu_level = src_level;
			src_cpu_x_offset = src_x_offset;
		}
		else
		{
			src_cpu = rf.MakeTexture1D(src_width, 1, 1,
				this->Format(), this->SampleCount(), this->SampleQuality(), EAH_CPU_Read);
			src_cpu_ptr = src_cpu.get();

			this->CopyToSubTexture1D(*src_cpu, 0, 0, 0, src_width, src_array_index, src_level, src_x_offset, src_width);

			src_cpu_array_index = 0;
			src_cpu_level = 0;
			src_cpu_x_offset = 0;
		}

		TexturePtr dst_cpu;
		Texture* dst_cpu_ptr;
		uint32_t dst_cpu_array_index;
		uint32_t dst_cpu_level;
		uint32_t dst_cpu_x_offset;
		if (target.AccessHint() & EAH_CPU_Write)
		{
			dst_cpu_ptr = &target;
			dst_cpu_array_index = dst_array_index;
			dst_cpu_level = dst_level;
			dst_cpu_x_offset = dst_x_offset;
		}
		else
		{
			dst_cpu = rf.MakeTexture1D(dst_width, 1, 1,
				target.Format(), target.SampleCount(), target.SampleQuality(), EAH_CPU_Write);
			dst_cpu_ptr = dst_cpu.get();

			dst_cpu_array_index = 0;
			dst_cpu_level = 0;
			dst_cpu_x_offset = 0;
		}

		{
			Mapper src_cpu_mapper(*src_cpu_ptr, src_cpu_array_index, src_cpu_level, TMA_Read_Only, src_cpu_x_offset, src_width);
			Mapper dst_cpu_mapper(*dst_cpu_ptr, dst_cpu_array_index, dst_cpu_level, TMA_Write_Only, dst_cpu_x_offset, dst_width);
			ResizeTexture(dst_cpu_mapper.Pointer<uint8_t>(), dst_cpu_mapper.RowPitch(), dst_cpu_mapper.SlicePitch(), target.Format(),
				dst_width, 1, 1,
				src_cpu_mapper.Pointer<uint8_t>(), src_cpu_mapper.RowPitch(), src_cpu_mapper.SlicePitch(), this->Format(),
				src_width, 1, 1,
				linear);
		}

		if (dst_cpu_ptr != &target)
		{
			dst_cpu_ptr->CopyToSubTexture1D(target, dst_array_index, dst_level, dst_x_offset, dst_width, 0, 0, 0, dst_width);
		}
	}

	void Texture::ResizeTexture2D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
		bool linear)
	{
		BOOST_ASSERT((TT_2D == this->Type()) || (TT_Cube == this->Type()));
		BOOST_ASSERT((TT_2D == target.Type()) || (TT_Cube == target.Type()));

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		TexturePtr src_cpu;
		Texture* src_cpu_ptr;
		uint32_t src_cpu_array_index;
		uint32_t src_cpu_level;
		uint32_t src_cpu_x_offset;
		uint32_t src_cpu_y_offset;
		if ((this->AccessHint() & EAH_CPU_Read))
		{
			src_cpu_ptr = this;
			src_cpu_array_index = src_array_index;
			src_cpu_level = src_level;
			src_cpu_x_offset = src_x_offset;
			src_cpu_y_offset = src_y_offset;
		}
		else
		{
			src_cpu = rf.MakeTexture2D(src_width, src_height, 1, 1,
				this->Format(), this->SampleCount(), this->SampleQuality(), EAH_CPU_Read | EAH_CPU_Write);
			src_cpu_ptr = src_cpu.get();

			this->CopyToSubTexture2D(*src_cpu, 0, 0, 0, 0, src_width, src_height, 
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height);

			src_cpu_array_index = 0;
			src_cpu_level = 0;
			src_cpu_x_offset = 0;
			src_cpu_y_offset = 0;
		}

		TexturePtr dst_cpu;
		Texture* dst_cpu_ptr;
		uint32_t dst_cpu_array_index;
		uint32_t dst_cpu_level;
		uint32_t dst_cpu_x_offset;
		uint32_t dst_cpu_y_offset;
		if ((target.AccessHint() & EAH_CPU_Write) && !IsCompressedFormat(target.Format()))
		{
			dst_cpu_ptr = &target;
			dst_cpu_array_index = dst_array_index;
			dst_cpu_level = dst_level;
			dst_cpu_x_offset = dst_x_offset;
			dst_cpu_y_offset = dst_y_offset;
		}
		else
		{
			dst_cpu = rf.MakeTexture2D(dst_width, dst_height, 1, 1,
				target.Format(), target.SampleCount(), target.SampleQuality(), EAH_CPU_Read | EAH_CPU_Write);
			dst_cpu_ptr = dst_cpu.get();

			dst_cpu_array_index = 0;
			dst_cpu_level = 0;
			dst_cpu_x_offset = 0;
			dst_cpu_y_offset = 0;
		}

		{
			Mapper src_cpu_mapper(*src_cpu_ptr, src_cpu_array_index, src_cpu_level, TMA_Read_Only, src_cpu_x_offset, src_cpu_y_offset, src_width, src_height);
			Mapper dst_cpu_mapper(*dst_cpu_ptr, dst_cpu_array_index, dst_cpu_level, TMA_Write_Only, dst_cpu_x_offset, dst_cpu_y_offset, dst_width, dst_height);
			ResizeTexture(dst_cpu_mapper.Pointer<uint8_t>(), dst_cpu_mapper.RowPitch(), dst_cpu_mapper.SlicePitch(), target.Format(),
				dst_width, dst_height, 1,
				src_cpu_mapper.Pointer<uint8_t>(), src_cpu_mapper.RowPitch(), src_cpu_mapper.SlicePitch(), this->Format(),
				src_width, src_height, 1,
				linear);
		}

		if (dst_cpu_ptr != &target)
		{
			dst_cpu_ptr->CopyToSubTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset,
				dst_width, dst_height, 0, 0, 0, 0, dst_width, dst_height);
		}
	}

	void Texture::ResizeTexture3D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth,
		bool linear)
	{
		BOOST_ASSERT(TT_3D == this->Type());
		BOOST_ASSERT(TT_3D == target.Type());

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		TexturePtr src_cpu;
		Texture* src_cpu_ptr;
		uint32_t src_cpu_array_index;
		uint32_t src_cpu_level;
		uint32_t src_cpu_x_offset;
		uint32_t src_cpu_y_offset;
		uint32_t src_cpu_z_offset;
		if (this->AccessHint() & EAH_CPU_Read)
		{
			src_cpu_ptr = this;
			src_cpu_array_index = src_array_index;
			src_cpu_level = src_level;
			src_cpu_x_offset = src_x_offset;
			src_cpu_y_offset = src_y_offset;
			src_cpu_z_offset = src_z_offset;
		}
		else
		{
			src_cpu = rf.MakeTexture3D(src_width, src_height, src_depth, 1, 1,
				this->Format(), this->SampleCount(), this->SampleQuality(), EAH_CPU_Read);
			src_cpu_ptr = src_cpu.get();

			this->CopyToSubTexture3D(*src_cpu, 0, 0, 0, 0, 0, src_width, src_height, src_depth, 
				src_array_index, src_level, src_x_offset, src_y_offset, src_z_offset, src_width, src_height, src_depth);

			src_cpu_array_index = 0;
			src_cpu_level = 0;
			src_cpu_x_offset = 0;
			src_cpu_y_offset = 0;
			src_cpu_z_offset = 0;
		}

		TexturePtr dst_cpu;
		Texture* dst_cpu_ptr;
		uint32_t dst_cpu_array_index;
		uint32_t dst_cpu_level;
		uint32_t dst_cpu_x_offset;
		uint32_t dst_cpu_y_offset;
		uint32_t dst_cpu_z_offset;
		if (target.AccessHint() & EAH_CPU_Write)
		{
			dst_cpu_ptr = &target;
			dst_cpu_array_index = dst_array_index;
			dst_cpu_level = dst_level;
			dst_cpu_x_offset = dst_x_offset;
			dst_cpu_y_offset = dst_y_offset;
			dst_cpu_z_offset = dst_z_offset;
		}
		else
		{
			dst_cpu = rf.MakeTexture3D(dst_width, dst_height, dst_depth, 1, 1,
				target.Format(), target.SampleCount(), target.SampleQuality(), EAH_CPU_Write);
			dst_cpu_ptr = dst_cpu.get();

			dst_cpu_array_index = 0;
			dst_cpu_level = 0;
			dst_cpu_x_offset = 0;
			dst_cpu_y_offset = 0;
			dst_cpu_z_offset = 0;
		}

		{
			Mapper src_cpu_mapper(*src_cpu_ptr, src_cpu_array_index, src_cpu_level, TMA_Read_Only, src_cpu_x_offset, src_cpu_y_offset, src_cpu_z_offset, src_width, src_height, src_depth);
			Mapper dst_cpu_mapper(*dst_cpu_ptr, dst_cpu_array_index, dst_cpu_level, TMA_Write_Only, dst_cpu_x_offset, dst_cpu_y_offset, dst_cpu_z_offset, dst_width, dst_height, dst_depth);
			ResizeTexture(dst_cpu_mapper.Pointer<uint8_t>(), dst_cpu_mapper.RowPitch(), dst_cpu_mapper.SlicePitch(), target.Format(),
				dst_width, dst_height, dst_depth,
				src_cpu_mapper.Pointer<uint8_t>(), src_cpu_mapper.RowPitch(), src_cpu_mapper.SlicePitch(), this->Format(),
				src_width, src_height, src_depth,
				linear);
		}

		if (dst_cpu_ptr != &target)
		{
			dst_cpu_ptr->CopyToSubTexture3D(target, dst_array_index, dst_level,
				dst_x_offset, dst_y_offset, dst_z_offset, dst_width, dst_height, dst_depth, 0, 0, 0, 0, 0, dst_width, dst_height, dst_height);
		}
	}

	void Texture::ResizeTextureCube(Texture& target,
		uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
		bool linear)
	{
		BOOST_ASSERT((TT_2D == this->Type()) || (TT_Cube == this->Type()));
		BOOST_ASSERT((TT_2D == target.Type()) || (TT_Cube == target.Type()));

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		TexturePtr src_cpu;
		Texture* src_cpu_ptr;
		uint32_t src_cpu_array_index;
		uint32_t src_cpu_level;
		uint32_t src_cpu_x_offset;
		uint32_t src_cpu_y_offset;
		{
			src_cpu = rf.MakeTexture2D(src_width, src_height, 1, 1,
				this->Format(), this->SampleCount(), this->SampleQuality(), EAH_CPU_Read);
			src_cpu_ptr = src_cpu.get();

			this->CopyToSubTexture2D(*src_cpu, 0, 0, 0, 0, src_width, src_height, 
				src_array_index * 6 + src_face - CF_Positive_X, src_level, src_x_offset, src_y_offset, src_width, src_height);

			src_cpu_array_index = 0;
			src_cpu_level = 0;
			src_cpu_x_offset = 0;
			src_cpu_y_offset = 0;
		}

		TexturePtr dst_cpu;
		Texture* dst_cpu_ptr;
		uint32_t dst_cpu_array_index;
		uint32_t dst_cpu_level;
		uint32_t dst_cpu_x_offset;
		uint32_t dst_cpu_y_offset;
		{
			dst_cpu = rf.MakeTexture2D(dst_width, dst_height, 1, 1,
				target.Format(), target.SampleCount(), target.SampleQuality(), EAH_CPU_Write);
			dst_cpu_ptr = dst_cpu.get();

			dst_cpu_array_index = 0;
			dst_cpu_level = 0;
			dst_cpu_x_offset = 0;
			dst_cpu_y_offset = 0;
		}

		{
			Mapper src_cpu_mapper(*src_cpu_ptr, src_cpu_array_index, src_face, src_cpu_level, TMA_Read_Only, src_cpu_x_offset, src_cpu_y_offset, src_width, src_height);
			Mapper dst_cpu_mapper(*dst_cpu_ptr, dst_cpu_array_index, dst_face, dst_cpu_level, TMA_Write_Only, dst_cpu_x_offset, dst_cpu_y_offset, dst_width, dst_height);
			ResizeTexture(dst_cpu_mapper.Pointer<uint8_t>(), dst_cpu_mapper.RowPitch(), dst_cpu_mapper.SlicePitch(), target.Format(),
				dst_width, dst_height, 1,
				src_cpu_mapper.Pointer<uint8_t>(), src_cpu_mapper.RowPitch(), src_cpu_mapper.SlicePitch(), this->Format(),
				src_width, src_height, 1,
				linear);
		}

		{
			dst_cpu_ptr->CopyToSubTextureCube(target, dst_array_index, dst_face, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				0, CF_Positive_X, 0, 0, 0, dst_width, dst_height);
		}
	}


	void ResizeTexture(void* dst_data, uint32_t dst_row_pitch, uint32_t dst_slice_pitch, ElementFormat dst_format,
		uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
		void const * src_data, uint32_t src_row_pitch, uint32_t src_slice_pitch, ElementFormat src_format,
		uint32_t src_width, uint32_t src_height, uint32_t src_depth,
		bool linear)
	{
		std::vector<uint8_t> src_cpu_data_block;
		void* src_cpu_data;
		uint32_t src_cpu_row_pitch;
		uint32_t src_cpu_slice_pitch;
		ElementFormat src_cpu_format;
		if (IsCompressedFormat(src_format))
		{
			DecodeTexture(src_cpu_data_block, src_cpu_row_pitch, src_cpu_slice_pitch, src_cpu_format,
				src_data, src_row_pitch, src_slice_pitch, src_format, src_width, src_height, src_depth);
			src_cpu_data = static_cast<void*>(&src_cpu_data_block[0]);
		}
		else
		{
			src_cpu_data = const_cast<void*>(src_data);
			src_cpu_row_pitch = src_row_pitch;
			src_cpu_slice_pitch = src_slice_pitch;
			src_cpu_format = src_format;
		}

		std::vector<uint8_t> dst_cpu_data_block;
		void* dst_cpu_data;
		uint32_t dst_cpu_row_pitch;
		uint32_t dst_cpu_slice_pitch;
		ElementFormat dst_cpu_format;
		if (IsCompressedFormat(dst_format))
		{
			switch (dst_format)
			{				
			case EF_BC1:
			case EF_BC2:
			case EF_BC3:
			case EF_BC7:
			case EF_ETC1:
			case EF_ETC2_BGR8:
			case EF_ETC2_A1BGR8:
			case EF_ETC2_ABGR8:
				dst_cpu_format = EF_ARGB8;
				break;
				
			case EF_BC4:
			case EF_ETC2_R11:
				dst_cpu_format = EF_R8;
				break;

			case EF_BC5:
			case EF_ETC2_GR11:
				dst_cpu_format = EF_GR8;
				break;

			case EF_SIGNED_BC1:
			case EF_SIGNED_BC2:
			case EF_SIGNED_BC3:
				dst_cpu_format = EF_SIGNED_ABGR8;
				break;

			case EF_SIGNED_BC4:
			case EF_SIGNED_ETC2_R11:
				dst_cpu_format = EF_SIGNED_R8;
				break;

			case EF_SIGNED_BC5:
				dst_cpu_format = EF_SIGNED_GR8;
				break;

			case EF_BC1_SRGB:
			case EF_BC2_SRGB:
			case EF_BC3_SRGB:
			case EF_BC4_SRGB:
			case EF_BC5_SRGB:
			case EF_BC7_SRGB:
			case EF_ETC2_BGR8_SRGB:
			case EF_ETC2_A1BGR8_SRGB:
			case EF_ETC2_ABGR8_SRGB:
				dst_cpu_format = EF_ARGB8_SRGB;
				break;

			case EF_BC6:
			case EF_SIGNED_BC6:
				dst_cpu_format = EF_ABGR16F;
				break;

			default:
				KFL_UNREACHABLE("Invalid destination format");
			}

			dst_cpu_row_pitch = dst_width * NumFormatBytes(src_cpu_format);
			dst_cpu_slice_pitch = dst_cpu_row_pitch * dst_height;
			dst_cpu_data_block.resize(dst_depth * dst_cpu_slice_pitch);
			dst_cpu_data = &dst_cpu_data_block[0];
		}
		else
		{
			dst_cpu_data = const_cast<void*>(dst_data);
			dst_cpu_row_pitch = dst_row_pitch;
			dst_cpu_slice_pitch = dst_slice_pitch;
			dst_cpu_format = dst_format;
		}

		uint8_t const * src_ptr = static_cast<uint8_t const *>(src_cpu_data);
		uint8_t* dst_ptr = static_cast<uint8_t*>(dst_cpu_data);
		uint32_t const src_elem_size = NumFormatBytes(src_cpu_format);
		uint32_t const dst_elem_size = NumFormatBytes(dst_cpu_format);

		if ((!linear || ((src_width == dst_width) && (src_height == dst_height) && (src_depth == dst_depth)))
			&& (src_cpu_format == dst_cpu_format))
		{
			for (uint32_t z = 0; z < dst_depth; ++ z)
			{
				float fz = static_cast<float>(z + 0.5f) / dst_depth * src_depth;
				uint32_t sz = std::min(static_cast<uint32_t>(fz), src_depth - 1);

				for (uint32_t y = 0; y < dst_height; ++ y)
				{
					float fy = static_cast<float>(y + 0.5f) / dst_height * src_height;
					uint32_t sy = std::min(static_cast<uint32_t>(fy), src_height - 1);

					uint8_t const * src_p = src_ptr + sz * src_cpu_slice_pitch + sy * src_cpu_row_pitch;
					uint8_t* dst_p = dst_ptr + z * dst_cpu_slice_pitch + y * dst_cpu_row_pitch;

					if (src_width == dst_width)
					{
						std::memcpy(dst_p, src_p, src_width * src_elem_size);
					}
					else
					{
						for (uint32_t x = 0; x < dst_width; ++ x, dst_p += dst_elem_size)
						{
							float fx = static_cast<float>(x + 0.5f) / dst_width * src_width;
							uint32_t sx = std::min(static_cast<uint32_t>(fx), src_width - 1);
							std::memcpy(dst_p, src_p + sx * src_elem_size, src_elem_size);
						}
					}
				}
			}
		}
		else
		{
			std::vector<Color> src_32f(src_width * src_height * src_depth);
			{
				for (uint32_t z = 0; z < src_depth; ++ z)
				{
					for (uint32_t y = 0; y < src_height; ++ y)
					{
						ConvertToABGR32F(src_cpu_format, src_ptr + z * src_cpu_slice_pitch + y * src_cpu_row_pitch,
							src_width, &src_32f[(z * src_height + y) * src_width]);
					}
				}
			}

			std::vector<Color> dst_32f(dst_width * dst_height * dst_depth);
			if (linear)
			{
				for (uint32_t z = 0; z < dst_depth; ++ z)
				{
					float fz = static_cast<float>(z + 0.5f) / dst_depth * src_depth;
					uint32_t sz0 = static_cast<uint32_t>(fz - 0.5f);
					uint32_t sz1 = MathLib::clamp<uint32_t>(sz0 + 1, 0, src_depth - 1);
					float weight_z = fz - sz0 - 0.5f;
							
					for (uint32_t y = 0; y < dst_height; ++ y)
					{
						float fy = static_cast<float>(y + 0.5f) / dst_height * src_height;
						uint32_t sy0 = static_cast<uint32_t>(fy - 0.5f);
						uint32_t sy1 = MathLib::clamp<uint32_t>(sy0 + 1, 0, src_height - 1);
						float weight_y = fy - sy0 - 0.5f;
							
						for (uint32_t x = 0; x < dst_width; ++ x)
						{
							float fx = static_cast<float>(x + 0.5f) / dst_width * src_width;
							uint32_t sx0 = static_cast<uint32_t>(fx - 0.5f);
							uint32_t sx1 = MathLib::clamp<uint32_t>(sx0 + 1, 0, src_width - 1);
							float weight_x = fx - sx0 - 0.5f;
							Color clr_x00 = MathLib::lerp(src_32f[(sz0 * src_height + sy0) * src_width + sx0],
								src_32f[(sz0 * src_height + sy0) * src_width + sx1], weight_x);
							Color clr_x01 = MathLib::lerp(src_32f[(sz0 * src_height + sy1) * src_width + sx0],
								src_32f[(sz0 * src_height + sy1) * src_width + sx1], weight_x);
							Color clr_y0 = MathLib::lerp(clr_x00, clr_x01, weight_y);
							Color clr_x10 = MathLib::lerp(src_32f[(sz1 * src_height + sy0) * src_width + sx0],
								src_32f[(sz1 * src_height + sy0) * src_width + sx1], weight_x);
							Color clr_x11 = MathLib::lerp(src_32f[(sz1 * src_height + sy1) * src_width + sx0],
								src_32f[(sz1 * src_height + sy1) * src_width + sx1], weight_x);
							Color clr_y1 = MathLib::lerp(clr_x10, clr_x11, weight_y);
							dst_32f[(z * dst_height + y) * dst_width + x] = MathLib::lerp(clr_y0, clr_y1, weight_z);
						}
					}
				}
			}
			else
			{
				for (uint32_t z = 0; z < dst_depth; ++ z)
				{
					float fz = static_cast<float>(z + 0.5f) / dst_depth * src_depth;
					uint32_t sz = std::min(static_cast<uint32_t>(fz), src_depth - 1);

					for (uint32_t y = 0; y < dst_height; ++ y)
					{
						float fy = static_cast<float>(y + 0.5f) / dst_height * src_height;
						uint32_t sy = std::min(static_cast<uint32_t>(fy), src_height - 1);

						for (uint32_t x = 0; x < dst_width; ++ x)
						{
							float fx = static_cast<float>(x + 0.5f) / dst_width * src_width;
							uint32_t sx = std::min(static_cast<uint32_t>(fx), src_width - 1);
							dst_32f[(z * dst_height + y) * dst_width + x] = src_32f[(sz * src_height + sy) * src_width + sx];
						}
					}
				}
			}

			{
				for (uint32_t z = 0; z < dst_depth; ++ z)
				{
					for (uint32_t y = 0; y < dst_height; ++ y)
					{
						ConvertFromABGR32F(dst_cpu_format, &dst_32f[(z * dst_height + y) * dst_width], dst_width, dst_ptr + z * dst_cpu_slice_pitch + y * dst_cpu_row_pitch);
					}
				}
			}
		}

		if (IsCompressedFormat(dst_format))
		{
			EncodeTexture(dst_data, dst_row_pitch, dst_slice_pitch, dst_format,
				dst_cpu_data, dst_cpu_row_pitch, dst_cpu_slice_pitch, dst_cpu_format,
				dst_width, dst_height, dst_depth);
		}
	}


	template KLAYGE_CORE_API std::pair<float3, float3> CubeMapViewVector(Texture::CubeFaces face);

	template <typename T>
	std::pair<Vector_T<T, 3>, Vector_T<T, 3>> CubeMapViewVector(Texture::CubeFaces face)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		Vector_T<T, 3> look_dir;
		Vector_T<T, 3> up_dir;

		switch (face)
		{
		case Texture::CF_Positive_X:
			look_dir	= Vector_T<T, 3>(1, 0, 0);
			up_dir		= Vector_T<T, 3>(0, 1, 0);
			break;

		case Texture::CF_Negative_X:
			look_dir	= Vector_T<T, 3>(-1, 0, 0);
			up_dir		= Vector_T<T, 3>(0, 1, 0);
			break;

		case Texture::CF_Positive_Y:
			if (re.RequiresFlipping())
			{
				look_dir	= Vector_T<T, 3>(0, 1, 0);
				up_dir		= Vector_T<T, 3>(0, 0, -1);
			}
			else
			{
				look_dir	= Vector_T<T, 3>(0, -1, 0);
				up_dir		= Vector_T<T, 3>(0, 0, 1);
			}
			break;

		case Texture::CF_Negative_Y:
			if (re.RequiresFlipping())
			{
				look_dir	= Vector_T<T, 3>(0, -1, 0);
				up_dir		= Vector_T<T, 3>(0, 0, 1);
			}
			else
			{
				look_dir	= Vector_T<T, 3>(0, 1, 0);
				up_dir		= Vector_T<T, 3>(0, 0, -1);
			}
			break;

		case Texture::CF_Positive_Z:
			look_dir	= Vector_T<T, 3>(0, 0, 1);
			up_dir		= Vector_T<T, 3>(0, 1, 0);
			break;

		case Texture::CF_Negative_Z:
		default:
			look_dir	= Vector_T<T, 3>(0, 0, -1);
			up_dir		= Vector_T<T, 3>(0, 1, 0);
			break;
		}

		return std::make_pair(look_dir, up_dir);
	}


	SoftwareTexture::SoftwareTexture(TextureType type, uint32_t width, uint32_t height, uint32_t depth,
		uint32_t num_mipmaps, uint32_t array_size, ElementFormat format, bool ref_only)
		: Texture(type, 1, 0, EAH_CPU_Read | EAH_CPU_Write),
			ref_only_(ref_only), width_(width), height_(height), depth_(depth)
	{
		if (0 == num_mipmaps)
		{
			num_mipmaps = 1;
			uint32_t w = width;
			uint32_t h = height;
			while ((w != 1) || (h != 1))
			{
				++ num_mipmaps;

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
			}
		}
		num_mip_maps_ = num_mipmaps;

		array_size_ = array_size;
		format_ = format;
	}

	std::wstring const & SoftwareTexture::Name() const
	{
		static const std::wstring name(L"Software Texture");
		return name;
	}

	uint32_t SoftwareTexture::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t SoftwareTexture::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, height_ >> level);
	}

	uint32_t SoftwareTexture::Depth(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, depth_ >> level);
	}

	void SoftwareTexture::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		uint32_t const num_faces = (type_ == TT_Cube) ? 6 : 1;
		uint32_t const array_size = std::min(array_size_, target.ArraySize());
		uint32_t const num_mip_maps = std::min(num_mip_maps_, target.NumMipMaps());
		for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
		{
			for (uint32_t face = 0; face < num_faces; ++ face)
			{
				for (uint32_t mip = 0; mip < num_mip_maps; ++ mip)
				{
					switch (type_)
					{
					case TT_1D:
						this->CopyToSubTexture1D(target, array_index, mip, 0, target.Width(mip),
							array_index, mip, 0, this->Width(mip));
						break;

					case TT_2D:
						this->CopyToSubTexture2D(target, array_index, mip, 0, 0, target.Width(mip), target.Height(mip),
							array_index, mip, 0, 0, this->Width(mip), this->Height(mip));
						break;

					case TT_3D:
						this->CopyToSubTexture3D(target, array_index, mip,
							0, 0, 0, target.Width(mip), target.Height(mip), target.Depth(mip),
							array_index, mip,
							0, 0, 0, this->Width(mip), this->Height(mip), this->Depth(mip));
						break;

					case TT_Cube:
						this->CopyToSubTextureCube(target, array_index, static_cast<CubeFaces>(face), mip,
							0, 0, target.Width(mip), target.Height(mip),
							array_index, static_cast<CubeFaces>(face), mip,
							0, 0, this->Width(mip), this->Height(mip));
						break;
					}
				}
			}
		}
	}

	void SoftwareTexture::CopyToSubTexture1D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		auto const & src_data = subres_data_[src_array_index * num_mip_maps_ + src_level];
		void const * src_ptr = static_cast<uint8_t const *>(src_data.data)
			+ src_x_offset * NumFormatBytes(format_);

		uint32_t resized_row_pitch;
		uint32_t resized_slice_pitch;
		void const * resized_ptr;
		std::vector<uint8_t> resized_data;
		if ((src_width == dst_width) && (format_ == target.Format()))
		{
			resized_row_pitch = src_data.row_pitch;
			resized_slice_pitch = src_data.slice_pitch;
			resized_ptr = src_ptr;
		}
		else
		{
			auto const target_format = target.Format();
			uint32_t const block_width = BlockWidth(target_format);
			uint32_t const block_bytes = BlockBytes(target_format);

			resized_row_pitch = (dst_width + block_width - 1) / block_width * block_bytes;
			resized_slice_pitch = resized_row_pitch;
			resized_data.resize(resized_slice_pitch);
			resized_ptr = resized_data.data();

			ResizeTexture(resized_data.data(), resized_row_pitch, resized_slice_pitch, target.Format(),
				dst_width, 1, 1,
				src_ptr, src_data.row_pitch, src_data.slice_pitch, format_,
				src_width, 1, 1,
				true);
		}

		target.UpdateSubresource1D(dst_array_index, dst_level, dst_x_offset, dst_width, resized_ptr);
	}

	void SoftwareTexture::CopyToSubTexture2D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_width, uint32_t src_height)
	{
		auto const & src_data = subres_data_[src_array_index * num_mip_maps_ + src_level];
		void const * src_ptr = static_cast<uint8_t const *>(src_data.data)
			+ src_y_offset / BlockHeight(format_) * subres_data_[src_array_index * num_mip_maps_ + src_level].row_pitch
			+ src_x_offset * NumFormatBytes(format_);

		uint32_t resized_row_pitch;
		uint32_t resized_slice_pitch;
		void const * resized_ptr;
		std::vector<uint8_t> resized_data;
		if ((src_width == dst_width) && (src_height == dst_height) && (format_ == target.Format()))
		{
			resized_row_pitch = src_data.row_pitch;
			resized_slice_pitch = src_data.slice_pitch;
			resized_ptr = src_ptr;
		}
		else
		{
			auto const target_format = target.Format();
			uint32_t const block_width = BlockWidth(target_format);
			uint32_t const block_height = BlockHeight(target_format);
			uint32_t const block_bytes = BlockBytes(target_format);

			resized_row_pitch = (dst_width + block_width - 1) / block_width * block_bytes;
			resized_slice_pitch = (dst_height + block_height - 1) / block_height * resized_row_pitch;
			resized_data.resize(resized_slice_pitch);
			resized_ptr = resized_data.data();

			ResizeTexture(resized_data.data(), resized_row_pitch, resized_slice_pitch, target_format,
				dst_width, dst_height, 1,
				src_ptr, src_data.row_pitch, src_data.slice_pitch, format_,
				src_width, src_height, 1,
				true);
		}

		target.UpdateSubresource2D(dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
			resized_ptr, resized_row_pitch);
	}

	void SoftwareTexture::CopyToSubTexture3D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset,
		uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset,
		uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		auto const & src_data = subres_data_[src_array_index * num_mip_maps_ + src_level];
		void const * src_ptr = static_cast<uint8_t const *>(src_data.data)
			+ src_z_offset * src_data.slice_pitch + src_y_offset * src_data.row_pitch + src_x_offset * NumFormatBytes(format_);

		uint32_t resized_row_pitch;
		uint32_t resized_slice_pitch;
		void const * resized_ptr;
		std::vector<uint8_t> resized_data;
		if ((src_width == dst_width) && (format_ == target.Format()))
		{
			resized_row_pitch = src_data.row_pitch;
			resized_slice_pitch = src_data.slice_pitch;
			resized_ptr = src_ptr;
		}
		else
		{
			auto const target_format = target.Format();
			uint32_t const block_width = BlockWidth(target_format);
			uint32_t const block_height = BlockHeight(target_format);
			uint32_t const block_bytes = BlockBytes(target_format);

			resized_row_pitch = (dst_width + block_width - 1) / block_width * block_bytes;
			resized_slice_pitch = (dst_height + block_height - 1) / block_height * resized_row_pitch;
			resized_data.resize(resized_slice_pitch * dst_depth);
			resized_ptr = resized_data.data();

			ResizeTexture(resized_data.data(), resized_row_pitch, resized_slice_pitch, target.Format(),
				dst_width, dst_height, dst_depth,
				src_ptr, src_data.row_pitch, src_data.slice_pitch, format_,
				src_width, src_height, src_depth,
				true);
		}

		target.UpdateSubresource3D(dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_z_offset, dst_width, dst_height, dst_depth,
			resized_ptr, resized_row_pitch, resized_slice_pitch);
	}

	void SoftwareTexture::CopyToSubTextureCube(Texture& target,
		uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_width, uint32_t src_height)
	{
		auto const & src_data = subres_data_[(src_array_index * 6 + src_face) * num_mip_maps_ + src_level];
		uint8_t const * src_ptr = static_cast<uint8_t const *>(src_data.data)
			+ src_y_offset * src_data.row_pitch + src_x_offset * NumFormatBytes(format_);

		uint32_t resized_row_pitch;
		uint32_t resized_slice_pitch;
		void const * resized_ptr;
		std::vector<uint8_t> resized_data;
		if ((src_width == dst_width) && (format_ == target.Format()))
		{
			resized_row_pitch = src_data.row_pitch;
			resized_slice_pitch = src_data.slice_pitch;
			resized_ptr = src_ptr;
		}
		else
		{
			auto const target_format = target.Format();
			uint32_t const block_width = BlockWidth(target_format);
			uint32_t const block_height = BlockHeight(target_format);
			uint32_t const block_bytes = BlockBytes(target_format);

			resized_row_pitch = (dst_width + block_width - 1) / block_width * block_bytes;
			resized_slice_pitch = (dst_height + block_height - 1) / block_height * resized_row_pitch;
			resized_data.resize(resized_slice_pitch);
			resized_ptr = resized_data.data();

			ResizeTexture(resized_data.data(), resized_row_pitch, resized_slice_pitch, target.Format(),
				dst_width, dst_height, 1,
				src_ptr, src_data.row_pitch, src_data.slice_pitch, format_,
				src_width, src_height, 1,
				true);
		}

		target.UpdateSubresourceCube(dst_array_index, dst_face, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
			resized_ptr, resized_row_pitch);
	}

	void SoftwareTexture::BuildMipSubLevels()
	{
		for (uint32_t index = 0; index < this->ArraySize(); ++ index)
		{
			for (uint32_t level = 1; level < this->NumMipMaps(); ++ level)
			{
				switch (type_)
				{
				case TT_1D:
					this->CopyToSubTexture1D(*this, index, level, 0, this->Width(level),
						index, level - 1, 0, this->Width(level - 1));
					break;

				case TT_2D:
					this->CopyToSubTexture2D(*this, index, level, 0, 0, this->Width(level), this->Height(level),
						index, level - 1, 0, 0, this->Width(level - 1), this->Height(level - 1));
					break;

				case TT_3D:
					this->CopyToSubTexture3D(*this, index, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level),
						index, level - 1, 0, 0, 0, this->Width(level - 1), this->Height(level - 1), this->Depth(level - 1));
					break;

				case TT_Cube:
					for (uint32_t face = 0; face < 6; ++ face)
					{
						this->CopyToSubTextureCube(*this, index, static_cast<CubeFaces>(face), level, 0, 0,
							this->Width(level), this->Height(level),
							index, static_cast<CubeFaces>(face), level - 1, 0, 0,
							this->Width(level - 1), this->Height(level - 1));
					}
					break;
				}
			}
		}
	}

	void SoftwareTexture::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t width,
		void*& data)
	{
		KFL_UNUSED(tma);
		KFL_UNUSED(width);

		size_t const subres = array_index * num_mip_maps_ + level;

		bool already_mapped = false;
		if (mapped_[subres]->compare_exchange_strong(already_mapped, true))
		{
			auto const & init_data = subres_data_[subres];
			uint8_t const * ptr = static_cast<uint8_t const *>(init_data.data);
			data = const_cast<uint8_t*>(ptr + x_offset * NumFormatBytes(format_));
		}
		else
		{
			data = nullptr;
			TERRC(std::errc::device_or_resource_busy);
		}
	}

	void SoftwareTexture::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void*& data, uint32_t& row_pitch)
	{
		KFL_UNUSED(tma);
		KFL_UNUSED(width);
		KFL_UNUSED(height);

		size_t const subres = array_index * num_mip_maps_ + level;

		bool already_mapped = false;
		if (mapped_[subres]->compare_exchange_strong(already_mapped, true))
		{
			auto const & init_data = subres_data_[subres];
			uint8_t const * ptr = static_cast<uint8_t const *>(init_data.data);
			row_pitch = init_data.row_pitch;
			data = const_cast<uint8_t*>(ptr + y_offset * row_pitch + x_offset * NumFormatBytes(format_));
		}
		else
		{
			data = nullptr;
			row_pitch = 0;
			TERRC(std::errc::device_or_resource_busy);
		}
	}

	void SoftwareTexture::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		KFL_UNUSED(tma);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);

		size_t const subres = array_index * num_mip_maps_ + level;

		bool already_mapped = false;
		if (mapped_[subres]->compare_exchange_strong(already_mapped, true))
		{
			auto const & init_data = subres_data_[subres];
			uint8_t const * ptr = static_cast<uint8_t const *>(init_data.data);
			row_pitch = init_data.row_pitch;
			slice_pitch = init_data.slice_pitch;
			data = const_cast<uint8_t*>(ptr + z_offset * slice_pitch + y_offset * row_pitch + x_offset * NumFormatBytes(format_));
		}
		else
		{
			data = nullptr;
			row_pitch = 0;
			slice_pitch = 0;
			TERRC(std::errc::device_or_resource_busy);
		}
	}

	void SoftwareTexture::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void*& data, uint32_t& row_pitch)
	{
		KFL_UNUSED(tma);
		KFL_UNUSED(width);
		KFL_UNUSED(height);

		size_t const subres = (array_index * 6 + face) * num_mip_maps_ + level;

		bool already_mapped = false;
		if (mapped_[subres]->compare_exchange_strong(already_mapped, true))
		{
			auto const & init_data = subres_data_[subres];
			uint8_t const * ptr = static_cast<uint8_t const *>(init_data.data);
			row_pitch = init_data.row_pitch;
			data = const_cast<uint8_t*>(ptr + y_offset * row_pitch + x_offset * NumFormatBytes(format_));
		}
		else
		{
			data = nullptr;
			row_pitch = 0;
			TERRC(std::errc::device_or_resource_busy);
		}
	}

	void SoftwareTexture::Unmap1D(uint32_t array_index, uint32_t level)
	{
		size_t const subres = array_index * num_mip_maps_ + level;

		bool already_mapped = true;
		if (!mapped_[subres]->compare_exchange_strong(already_mapped, false))
		{
			TERRC(std::errc::device_or_resource_busy);
		}
	}

	void SoftwareTexture::Unmap2D(uint32_t array_index, uint32_t level)
	{
		size_t const subres = array_index * num_mip_maps_ + level;

		bool already_mapped = true;
		if (!mapped_[subres]->compare_exchange_strong(already_mapped, false))
		{
			TERRC(std::errc::device_or_resource_busy);
		}
	}

	void SoftwareTexture::Unmap3D(uint32_t array_index, uint32_t level)
	{
		size_t const subres = array_index * num_mip_maps_ + level;

		bool already_mapped = true;
		if (!mapped_[subres]->compare_exchange_strong(already_mapped, false))
		{
			TERRC(std::errc::device_or_resource_busy);
		}
	}

	void SoftwareTexture::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
	{
		size_t const subres = (array_index * 6 + face) * num_mip_maps_ + level;

		bool already_mapped = true;
		if (!mapped_[subres]->compare_exchange_strong(already_mapped, false))
		{
			TERRC(std::errc::device_or_resource_busy);
		}
	}

	void SoftwareTexture::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		KFL_UNUSED(clear_value_hint);

		uint32_t const num_faces = (type_ == TT_Cube) ? 6 : 1;
		uint32_t const num_subres = num_mip_maps_ * array_size_ * num_faces;

		if (init_data.empty())
		{
			BOOST_ASSERT(!ref_only_);

			subres_data_.resize(num_subres);

			uint32_t const block_width = BlockWidth(format_);
			uint32_t const block_height = BlockHeight(format_);
			uint32_t const block_bytes = BlockBytes(format_);
			for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
			{
				for (uint32_t face = 0; face < num_faces; ++ face)
				{
					for (uint32_t level = 0; level < num_mip_maps_; ++ level)
					{
						size_t const subres = (array_index * num_faces + face) * num_mip_maps_ + level;

						subres_data_[subres].row_pitch = (this->Width(level) + block_width - 1) / block_width * block_bytes;
						subres_data_[subres].slice_pitch
							= (this->Height(level) + block_height - 1) / block_height * subres_data_[subres].row_pitch;
					}
				}
			}
		}
		else
		{
			BOOST_ASSERT(init_data.size() >= num_subres);

			subres_data_.assign(init_data.begin(), init_data.begin() + num_subres);
		}
		mapped_.resize(num_subres);

		for (auto& item : mapped_)
		{
			item = MakeUniquePtr<std::atomic<bool>>(false);
		}

		data_block_.clear();
		if (!ref_only_)
		{
			uint32_t size = 0;
			for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
			{
				for (uint32_t face = 0; face < num_faces; ++ face)
				{
					for (uint32_t level = 0; level < num_mip_maps_; ++ level)
					{
						size_t const subres = (array_index * num_faces + face) * num_mip_maps_ + level;
						size += subres_data_[subres].slice_pitch * this->Depth(level);
					}
				}
			}

			data_block_.reserve(size);
			for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
			{
				for (uint32_t face = 0; face < num_faces; ++ face)
				{
					for (uint32_t level = 0; level < num_mip_maps_; ++ level)
					{
						size_t const subres = (array_index * num_faces + face) * num_mip_maps_ + level;

						subres_data_[subres].data = data_block_.data() + data_block_.size();
						uint32_t const subres_size = subres_data_[subres].slice_pitch * this->Depth(level);
						if (init_data.empty())
						{
							data_block_.resize(data_block_.size() + subres_size);
						}
						else
						{
							uint8_t const * p = static_cast<uint8_t const *>(init_data[subres].data);
							data_block_.insert(data_block_.end(), p, p + subres_size);
						}
					}
				}
			}

			if (init_data.empty())
			{
				std::memset(data_block_.data(), 0, data_block_.size());
			}
		}
	}

	void SoftwareTexture::DeleteHWResource()
	{
		subres_data_.clear();
		data_block_.clear();
		mapped_.clear();
	}

	bool SoftwareTexture::HWResourceReady() const
	{
		return !subres_data_.empty();
	}

	void SoftwareTexture::UpdateSubresource1D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t width,
		void const * data)
	{
		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_bytes = BlockBytes(format_);
		size_t const subres = array_index * num_mip_maps_ + level;
		auto const & init_data = subres_data_[subres];
		uint8_t* dst_ptr = static_cast<uint8_t*>(const_cast<void*>(init_data.data))
			+ (x_offset + block_width - 1) / block_width * block_bytes;

		uint32_t const bytes_per_row = (width + block_width - 1) / block_width * block_bytes;
		std::memcpy(dst_ptr, data, bytes_per_row);
	}

	void SoftwareTexture::UpdateSubresource2D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);
		uint32_t const block_bytes = BlockBytes(format_);
		size_t const subres = array_index * num_mip_maps_ + level;
		auto const & init_data = subres_data_[subres];
		uint8_t* dst_ptr = static_cast<uint8_t*>(const_cast<void*>(init_data.data))
			+ (y_offset + block_height - 1) / block_height * init_data.row_pitch
			+ (x_offset + block_width - 1) / block_width * block_bytes;
		uint8_t const * src_ptr = static_cast<uint8_t const *>(data);

		uint32_t const bytes_per_row = (width + block_width - 1) / block_width * block_bytes;
		for (uint32_t y = 0; y < height; y += block_height)
		{
			std::memcpy(dst_ptr, src_ptr, bytes_per_row);

			src_ptr += row_pitch;
			dst_ptr += init_data.row_pitch;
		}
	}

	void SoftwareTexture::UpdateSubresource3D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void const * data, uint32_t row_pitch, uint32_t slice_pitch)
	{
		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);
		uint32_t const block_depth = BlockDepth(format_);
		uint32_t const block_bytes = BlockBytes(format_);
		size_t const subres = array_index * num_mip_maps_ + level;
		auto const & init_data = subres_data_[subres];
		uint8_t* dst0_ptr = static_cast<uint8_t*>(const_cast<void*>(init_data.data))
			+ (z_offset + block_depth - 1) / block_depth * init_data.slice_pitch
			+ (y_offset + block_height - 1) / block_height * init_data.row_pitch
			+ (x_offset + block_width - 1) / block_width * block_bytes;
		uint8_t const * src0_ptr = static_cast<uint8_t const *>(data);

		uint32_t const bytes_per_row = (width + block_width - 1) / block_width * block_bytes;
		for (uint32_t z = 0; z < depth; z += block_depth)
		{
			uint8_t const * src_ptr = src0_ptr;
			uint8_t* dst_ptr = dst0_ptr;

			for (uint32_t y = 0; y < height; y += block_height)
			{
				std::memcpy(dst_ptr, src_ptr, bytes_per_row);

				src_ptr += row_pitch;
				dst_ptr += init_data.row_pitch;
			}

			src0_ptr += slice_pitch;
			dst_ptr += init_data.slice_pitch;
		}
	}

	void SoftwareTexture::UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		uint32_t const block_width = BlockWidth(format_);
		uint32_t const block_height = BlockHeight(format_);
		uint32_t const block_bytes = BlockBytes(format_);
		size_t const subres = (array_index * 6 + face) * num_mip_maps_ + level;
		auto const & init_data = subres_data_[subres];
		uint8_t* dst_ptr = static_cast<uint8_t*>(const_cast<void*>(init_data.data))
			+ (y_offset + block_height - 1) / block_height * init_data.row_pitch
			+ (x_offset + block_width - 1) / block_width * block_bytes;
		uint8_t const * src_ptr = static_cast<uint8_t const *>(data);

		uint32_t const bytes_per_row = (width + block_width - 1) / block_width * block_bytes;
		for (uint32_t y = 0; y < height; y += block_height)
		{
			std::memcpy(dst_ptr, src_ptr, bytes_per_row);

			src_ptr += row_pitch;
			dst_ptr += init_data.row_pitch;
		}
	}
}
