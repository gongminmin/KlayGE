// OGLTexture.hpp
// KlayGE OpenGL纹理类 实现文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

#include <cstring>

#include <GLLoader/GLLoader.h>

#include <KlayGE/OpenGL/OGLTexture.hpp>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

namespace
{
	using namespace KlayGE;

	void Convert(GLint& internalFormat, GLenum& glformat, KlayGE::PixelFormat pf)
	{
		switch (pf)
		{
		case PF_L8:
			internalFormat = GL_LUMINANCE8;
			glformat = GL_LUMINANCE;
			break;

		case PF_A8:
			internalFormat = GL_ALPHA8;
			glformat = GL_ALPHA;
			break;

		case PF_A4L4:
			internalFormat = GL_LUMINANCE4_ALPHA4;
			glformat = GL_LUMINANCE_ALPHA;
			break;

		case PF_A8L8:
			internalFormat = GL_LUMINANCE8_ALPHA8;
			glformat = GL_LUMINANCE_ALPHA;
			break;

		case PF_R5G6B5:
			internalFormat = GL_RGB5;
			glformat = GL_BGR;
			break;

		case PF_A4R4G4B4:
			internalFormat = GL_RGBA4;
			glformat = GL_BGRA;
			break;

		case PF_X8R8G8B8:
			internalFormat = GL_RGB8;
			glformat = GL_BGR;
			break;

		case PF_A8R8G8B8:
			internalFormat = GL_RGBA8;
			glformat = GL_BGRA;
			break;

		case PF_A2R10G10B10:
			internalFormat = GL_RGB10_A2;
			glformat = GL_BGRA;
			break;
		}
	}
}

namespace KlayGE
{
	OGLTexture::OGLTexture(uint32_t width, uint32_t height,
								uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
								: texture_(0)
	{
		numMipMaps_ = numMipMaps;
		format_		= format;
		width_		= width;
		height_		= height;

		bpp_ = PixelFormatBits(format);

		usage_ = usage;
	}

	OGLTexture::~OGLTexture()
	{
	}

	std::wstring const & OGLTexture::Name() const
	{
		static const std::wstring name(L"OpenGL Texture");
		return name;
	}

	void OGLTexture::CopyToTexture(Texture& target)
	{
		OGLTexture& other(static_cast<OGLTexture&>(target));
		glCopyTexImage2D(other.GLTexture(), 0, 3, 0, 0, other.Width(), other.Height(), 0);
	}

	void OGLTexture::CopyToMemory(int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, format_);

		glGetTexImage(texture_, level, glformat, GL_UNSIGNED_BYTE, data);
	}

	void OGLTexture::CopyMemoryToTexture(int level, void* data, PixelFormat pf,
		uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset)
	{
		if (0 == width)
		{
			width = width_;
		}
		if (0 == height)
		{
			height = height_;
		}

		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, pf);

		if ((0 == texture_) || (width > width_) || (height > height_) || (pf != format_))
		{
			if (texture_ != 0)
			{
				glDeleteTextures(1, &texture_);
			}

			glGenTextures(1, &texture_);
			glBindTexture(GL_TEXTURE_2D, texture_);

			glTexImage2D(GL_TEXTURE_2D, level, glinternalFormat, width, height, 0, glformat, GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, texture_);
			glTexSubImage2D(GL_TEXTURE_2D, level, xOffset, yOffset, width, height, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::GenerateMipSubLevels()
	{
	}

	void OGLTexture::CustomAttribute(std::string const & name, void* pData)
	{
		if ("IsTexture" == name)
		{
			bool* b = reinterpret_cast<bool*>(pData);
			*b = true;

			return;
		}
	}
}
