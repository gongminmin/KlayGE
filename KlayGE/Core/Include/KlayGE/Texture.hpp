// Texture.hpp
// KlayGE 纹理类 实现文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.3.0
// 增加了对浮点纹理格式的支持 (2005.1.25)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <string>

#if defined(DEBUG) | defined(_DEBUG)
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
			return 16;
				
		case PF_X8R8G8B8:
		case PF_A8R8G8B8:
		case PF_A2R10G10B10:
		case PF_G16R16F:
		case PF_R32F:
			return 32;

		case PF_A16B16G16R16F:
		case PF_G32R32F:
			return 64;

		case PF_A32B32G32R32F:
			return 128;
		}

		return 0;
	}

	/** Abstract class representing a Texture resource.
	@remarks
	The actual concrete subclass which will exist for a texture
	is dependent on the rendering system in use (Direct3D, OpenGL etc).
	This class represents the commonalities, and is the one 'used'
	by programmers even though the real implementation could be
	different in reality. Texture objects are created through
	the 'create' method of the TextureManager concrete subclass.
	*/
	class Texture
	{
	public:
		// Enum identifying the texture usage
		enum TextureUsage
		{
			TU_Default		= 0,	// default usage
			TU_RenderTarget = 1		// this texture will be a render target, ie. used as a target for render to texture
		};

	public:
		virtual ~Texture()
			{ }

		// Gets the name of texture
		virtual std::wstring const & Name() const = 0;

		// Gets the number of mipmaps to be used for this texture.
		uint16_t NumMipMaps() const 
			{ return this->numMipMaps_; }

		// Sets the number of mipmaps to be used for this texture.
		// @note
		// Must be set before calling any 'load' method.
		void NumMipMaps(uint16_t num)
			{ this->numMipMaps_ = num; }

		// Returns the TextureUsage indentifier for this Texture
		TextureUsage Usage() const
			{ return this->usage_; }

        // Returns the width of the texture.
		uint32_t Width() const
			{ return this->width_; }

		// Returns the height of the texture.
		uint32_t Height() const
			{ return this->height_; }

		// Returns the bpp of the texture.
		uint32_t Bpp() const
			{ return this->bpp_; }

		// Returns the pixel format for the texture surface.
		virtual PixelFormat Format() const
			{ return this->format_; }

		// Copies (and maybe scales to fit) the contents of this texture to another texture.
		virtual void CopyToTexture(Texture& target) = 0;
		virtual void CopyMemoryToTexture(void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset) = 0;

	protected:
		uint32_t		height_;
		uint32_t		width_;
		uint32_t		bpp_;

		uint16_t		numMipMaps_;

		PixelFormat format_;
		TextureUsage usage_;
	};
}

#endif			// _TEXTURE_HPP
