// Texture.hpp
// KlayGE 纹理类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 去掉了构造函数的usage (2005.10.5)
//
// 2.7.0
// 可以获取Mipmap中每层的宽高深 (2005.6.8)
// 增加了AddressingMode, Filtering和Anisotropy (2005.6.27)
// 增加了MaxMipLevel和MipMapLodBias (2005.6.28)
//
// 2.4.0
// 增加了DXTn的支持 (2005.3.6)
// 增加了1D/2D/3D/cube的支持 (2005.3.8)
//
// 2.3.0
// 增加了对浮点纹理格式的支持 (2005.1.25)
// 增加了CopyToMemory (2005.2.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <KlayGE/PreDeclare.hpp>

#include <string>
#include <boost/assert.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	enum PixelFormat
	{
		// Unknown pixel format.
		PF_Unknown,
		// 8-bit pixel format, all bits luminace.
		PF_L8,
		// 8-bit pixel format, all bits alpha.
		PF_A8,
		// 8-bit pixel format, 4 bits alpha, 4 bits luminace.
		PF_A4L4,
		// 16-bit pixel format, 8 bits alpha, 8 bits luminace.
		PF_A8L8,
		// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
		PF_R5G6B5,
		// 16-bit pixel format, 4 bits for alpha, red, green and blue.
		PF_A4R4G4B4,
		// 32-bit pixel format, 8 bits no used, 8 bits for red, green and blue.
		PF_X8R8G8B8,
		// 32-bit pixel format, 8 bits for alpha, red, green and blue.
		PF_A8R8G8B8,
		// 32-bit pixel format, 2 bits for alpha, 10 bits for red, green and blue.
		PF_A2R10G10B10,

		// 16-bit pixel format, 16 bits floating-point for red.
		PF_R16F,
		// 32-bit pixel format, 16 bits floating-point for green and red.
		PF_G16R16F,
		// 64-bit pixel format, 16 bits floating-point for alpha, blue, green and red.
		PF_A16B16G16R16F,
		// 32-bit pixel format, 32 bits floating-point for red.
		PF_R32F,
		// 64-bit pixel format, 32 bits floating-point for green and red.
		PF_G32R32F,
		// 128-bit pixel format, 32 bits floating-point for alpha, blue, green and red.
		PF_A32B32G32R32F,

		// DXT1 compression texture format 
		PF_DXT1,
		// DXT3 compression texture format
		PF_DXT3,
		// DXT5 compression texture format
		PF_DXT5,

		// 16-bit pixel format, 16 bits depth
		PF_D16,
		// 32-bit pixel format, 24 bits depth
		PF_D24X8,
		// 32-bit pixel format, 24 bits depth and 8 bits stencil
		PF_D24S8,
	};

	inline uint8_t
	PixelFormatBits(PixelFormat format)
	{
		switch (format)
		{
		case PF_L8:
		case PF_A8:
		case PF_A4L4:
			return 8;

		case PF_R5G6B5:
		case PF_A4R4G4B4:
		case PF_R16F:
		case PF_DXT1:
		case PF_D16:
			return 16;
				
		case PF_X8R8G8B8:
		case PF_A8R8G8B8:
		case PF_A2R10G10B10:
		case PF_G16R16F:
		case PF_R32F:
		case PF_DXT3:
		case PF_DXT5:
		case PF_D24X8:
		case PF_D24S8:
			return 32;

		case PF_A16B16G16R16F:
		case PF_G32R32F:
			return 64;

		case PF_A32B32G32R32F:
			return 128;
		}

		BOOST_ASSERT(false);
		return 0;
	}

	inline bool
	IsFloatFormat(PixelFormat format)
	{
		switch (format)
		{
		case PF_L8:
		case PF_A8:
		case PF_A4L4:
		case PF_R5G6B5:
		case PF_A4R4G4B4:
		case PF_X8R8G8B8:
		case PF_A8R8G8B8:
		case PF_A2R10G10B10:
		case PF_A16B16G16R16F:
		case PF_DXT1:
		case PF_DXT3:
		case PF_DXT5:
		case PF_D16:
		case PF_D24X8:
		case PF_D24S8:
			return false;
				
		case PF_R16F:
		case PF_G16R16F:
		case PF_R32F:
		case PF_G32R32F:
		case PF_A32B32G32R32F:
			return true;
		}

		BOOST_ASSERT(false);
		return false;
	}

	inline bool
	IsCompressedFormat(PixelFormat format)
	{
		switch (format)
		{
		case PF_DXT1:
		case PF_DXT3:
		case PF_DXT5:
			return true;

		case PF_L8:
		case PF_A8:
		case PF_A4L4:
		case PF_R5G6B5:
		case PF_A4R4G4B4:
		case PF_X8R8G8B8:
		case PF_A8R8G8B8:
		case PF_A2R10G10B10:
		case PF_A16B16G16R16F:
		case PF_R16F:
		case PF_G16R16F:
		case PF_R32F:
		case PF_G32R32F:
		case PF_A32B32G32R32F:
		case PF_D16:
		case PF_D24X8:
		case PF_D24S8:
			return false;
		}

		BOOST_ASSERT(false);
		return false;
	}

	inline bool
	IsDepthFormat(PixelFormat format)
	{
		switch (format)
		{
		case PF_D16:
		case PF_D24X8:
		case PF_D24S8:
			return true;

		default:
			return false;
		}
	}

	// Abstract class representing a Texture resource.
	// @remarks
	// The actual concrete subclass which will exist for a texture
	// is dependent on the rendering system in use (Direct3D, OpenGL etc).
	// This class represents the commonalities, and is the one 'used'
	// by programmers even though the real implementation could be
	// different in reality.
	class Texture
	{
	public:
		// Enum identifying the texture usage
		enum TextureUsage
		{
			TU_Default		= 0,	// default usage
			TU_RenderTarget = 1,	// this texture will be a render target, ie. used as a target for render to texture
		};

		// Enum identifying the texture type
		enum TextureType
		{
			// 1D texture, used in combination with 1D texture coordinates
			TT_1D,
			// 2D texture, used in combination with 2D texture coordinates
			TT_2D,
			// 3D texture, used in combination with 3D texture coordinates
			TT_3D,
			// cube map, used in combination with 3D texture coordinates
			TT_Cube,
		};

		enum CubeFaces
		{
			CF_Positive_X = 0,
			CF_Negative_X = 1,
			CF_Positive_Y = 2,
			CF_Negative_Y = 3,
			CF_Positive_Z = 4,
			CF_Negative_Z = 5,
		};

	public:
		explicit Texture(TextureType type);
		virtual ~Texture();

		// Gets the name of texture
		virtual std::wstring const & Name() const = 0;

		// Gets the number of mipmaps to be used for this texture.
		uint16_t NumMipMaps() const;

		// Returns the TextureUsage indentifier for this Texture
		TextureUsage Usage() const;
		virtual void Usage(TextureUsage usage) = 0;

        // Returns the width of the texture.
		virtual uint32_t Width(int level) const = 0;
		// Returns the height of the texture.
		virtual uint32_t Height(int level) const = 0;
		// Returns the depth of the texture (only for 3D texture).
		virtual uint32_t Depth(int level) const = 0;

		// Returns the bpp of the texture.
		uint32_t Bpp() const;
		// Returns the pixel format for the texture surface.
		PixelFormat Format() const;

		// Returns the texture type of the texture.
		TextureType Type() const;

		// Copies (and maybe scales to fit) the contents of this texture to another texture.
		virtual void CopyToTexture(Texture& target) = 0;

		virtual void CopyToMemory1D(int level, void* data) = 0;
		virtual void CopyToMemory2D(int level, void* data) = 0;
		virtual void CopyToMemory3D(int level, void* data) = 0;
		virtual void CopyToMemoryCube(CubeFaces face, int level, void* data) = 0;

		virtual void CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t xOffset) = 0;
		virtual void CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset) = 0;
		virtual void CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t depth,
			uint32_t xOffset, uint32_t yOffset, uint32_t zOffset) = 0;
		virtual void CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t size, uint32_t xOffset) = 0;

		virtual void BuildMipSubLevels() = 0;

	protected:
		uint32_t		bpp_;

		uint16_t		numMipMaps_;

		PixelFormat		format_;
		TextureUsage	usage_;
		TextureType		type_;
	};

	TexturePtr LoadTexture(std::string const & tex_name);
	void SaveToFile(TexturePtr texture, std::string const & tex_name);
}

#endif			// _TEXTURE_HPP
