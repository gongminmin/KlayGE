#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <gl/gl.h>

#include <KlayGE/OpenGL/OGLTexture.hpp>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

namespace
{
	using namespace KlayGE;

	void Convert(GLint& intenalFormat, GLenum& glformat, GLenum& gltype, KlayGE::PixelFormat pf)
	{
		switch (pf)
		{
		case PF_L8:
			intenalFormat = GL_LUMINANCE8;
			glformat = GL_LUMINANCE;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_A8:
			intenalFormat = GL_ALPHA8;
			glformat = GL_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_A4L4:
			intenalFormat = GL_LUMINANCE4_ALPHA4;
			glformat = GL_LUMINANCE_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_R5G6B5:
			intenalFormat = GL_RGB5;
			glformat = GL_RGB;
			gltype = GL_UNSIGNED_SHORT;
			break;

		case PF_A4R4G4B4:
			intenalFormat = GL_RGBA4;
			glformat = GL_RGBA;
			gltype = GL_UNSIGNED_SHORT;
			break;

		case PF_X8R8G8B8:
			intenalFormat = GL_RGB8;
			glformat = GL_RGB;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_A8R8G8B8:
			intenalFormat = GL_RGBA8;
			glformat = GL_RGBA;
			gltype = GL_UNSIGNED_INT;
			break;

		case PF_A2R10G10B10:
			intenalFormat = GL_RGB10_A2;
			glformat = GL_RGBA;
			gltype = GL_UNSIGNED_INT;
			break;
		}
	}
}

namespace KlayGE
{
	OGLTexture::OGLTexture(U32 width, U32 height,
								U16 mipMapsNum, PixelFormat format, TextureUsage usage)
	{
		mipMapsNum_ = (0 == mipMapsNum) ? mipMapsNum : 1;
		format_		= format;
		width_		= width;
		height_		= height;

		bpp_ = PixelFormatBits(format);

		usage_ = usage;

		glGenTextures(1, &texture_);
		glBindTexture(GL_TEXTURE_2D, texture_);

		GLint glinternalFormat;
		GLenum glformat, gltype;
		Convert(glinternalFormat, glformat, gltype, format);
		glTexImage2D(GL_TEXTURE_2D, 0, glformat, width_, height_, 0, glformat, gltype, NULL);
	}

	OGLTexture::~OGLTexture()
	{
	}

	const WString& OGLTexture::Name() const
	{
		static WString name(L"OpenGL Texture");
		return name;
	}

	void OGLTexture::CopyToTexture(Texture& target)
	{
		OGLTexture& other(reinterpret_cast<OGLTexture&>(target));
		glCopyTexImage2D(other.GLTexture(), 0, 3, 0, 0, other.Width(), other.Height(), 0);
	}

	void OGLTexture::CopyMemoryToTexture(void* pData, PixelFormat pf,
		U32 width, U32 height, U32 pitch, U32 xOffset, U32 yOffset)
	{
		U16 bpp(PixelFormatBits(pf));

		if (0 == width)
		{
			width = width_;
		}
		if (0 == height)
		{
			height = height_;
		}
		if (0 == pitch)
		{
			pitch = width * bpp / 8;
		}

		GLint glinternalFormat;
		GLenum glformat, gltype;
		Convert(glinternalFormat, glformat, gltype, pf);
		glTexSubImage2D(texture_, 0, xOffset, yOffset, width, height, glformat, gltype, pData);
	}

	void OGLTexture::CustomAttribute(const String& name, void* pData)
	{
		if ("IsTexture" == name)
		{
			bool* b = reinterpret_cast<bool*>(pData);
			*b = true;

			return;
		}
	}
}
