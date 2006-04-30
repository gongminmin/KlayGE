// OGLTexture.hpp
// KlayGE OpenGL纹理类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 增加了AddressingMode, Filtering和Anisotropy (2005.6.27)
// 增加了MaxMipLevel和MipMapLodBias (2005.6.28)
//
// 2.3.0
// 增加了CopyToMemory (2005.2.6)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Context.hpp>

#include <cstring>

#include <glloader/glloader.h>
#include <gl/glu.h>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

namespace KlayGE
{
	OGLTexture::OGLTexture(TextureType type)
					: Texture(type)
	{
		switch (type_)
		{
		case TT_1D:
			target_type_ = GL_TEXTURE_1D;

		case TT_2D:
			target_type_ = GL_TEXTURE_2D;

		case TT_3D:
			target_type_ = GL_TEXTURE_3D;

		case TT_Cube:
			target_type_ = GL_TEXTURE_CUBE_MAP;

		default:
			BOOST_ASSERT(false);
			target_type_ = GL_TEXTURE_1D;
		}
	}

	OGLTexture::~OGLTexture()
	{
		glDeleteTextures(1, &texture_);
	}

	std::wstring const & OGLTexture::Name() const
	{
		static const std::wstring name(L"OpenGL Texture");
		return name;
	}

	uint32_t OGLTexture::Width(int level) const
	{
		BOOST_ASSERT(false);
		return 0;
	}
	
	uint32_t OGLTexture::Height(int level) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	uint32_t OGLTexture::Depth(int level) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	void OGLTexture::CopyToMemory1D(int level, void* data)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToMemory2D(int level, void* data)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToMemory3D(int level, void* data)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToMemoryCube(CubeFaces face, int level, void* data)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
		uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
		uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
		uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::BuildMipSubLevels()
	{
		if (glloader_GL_EXT_framebuffer_object())
		{
			this->GLBindTexture();
			glGenerateMipmapEXT(target_type_);
		}
		else
		{
			THR(E_FAIL);
		}
	}

	void OGLTexture::CustomAttribute(std::string const & /*name*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::GLBindTexture()
	{
		glBindTexture(target_type_, texture_);
	}

	void OGLTexture::Usage(Texture::TextureUsage usage)
	{
		usage_ = usage;
	}

	PixelFormat OGLTexture::SRGBToRGB(PixelFormat pf)
	{
		switch (pf)
		{
		case PF_ARGB8_SRGB:
			return PF_ARGB8;

		case PF_DXT1_SRGB:
			return PF_DXT1;

		case PF_DXT3_SRGB:
			return PF_DXT3;

		case PF_DXT5_SRGB:
			return PF_DXT5;

		default:
			return pf;
		}
	}

	void OGLTexture::Convert(GLint& internalFormat, GLenum& glformat, GLenum& gltype, PixelFormat pf)
	{
		switch (pf)
		{
		case PF_L8:
			internalFormat = GL_LUMINANCE8;
			glformat = GL_LUMINANCE;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_A8:
			internalFormat = GL_ALPHA8;
			glformat = GL_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_AL4:
			internalFormat = GL_LUMINANCE4_ALPHA4;
			glformat = GL_LUMINANCE_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_L16:
			internalFormat = GL_LUMINANCE16;
			glformat = GL_LUMINANCE;
			gltype = GL_UNSIGNED_SHORT;
			break;

		case PF_AL8:
			internalFormat = GL_LUMINANCE8_ALPHA8;
			glformat = GL_LUMINANCE_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_R5G6B5:
			internalFormat = GL_RGB5;
			glformat = GL_BGR;
			gltype = GL_UNSIGNED_SHORT_5_6_5_REV;
			break;

		case PF_ARGB4:
			internalFormat = GL_RGBA4;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_SHORT_4_4_4_4_REV;
			break;

		case PF_XRGB8:
			internalFormat = GL_RGB8;
			glformat = GL_BGR;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_ARGB8:
			internalFormat = GL_RGBA8;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_INT_8_8_8_8_REV;
			break;

		case PF_A2RGB10:
			internalFormat = GL_RGB10_A2;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_INT_2_10_10_10_REV;
			break;

		case PF_DXT1:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_DXT3:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_DXT5:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_ARGB8_SRGB:
			internalFormat = GL_SRGB8_ALPHA8_EXT;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_INT_8_8_8_8_REV;
			break;

		case PF_DXT1_SRGB:
			internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_DXT3_SRGB:
			internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_DXT5_SRGB:
			internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
}
