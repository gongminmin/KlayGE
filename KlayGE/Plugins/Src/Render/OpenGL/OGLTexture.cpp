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

namespace
{
	using namespace KlayGE;

	void Convert(GLint& internalFormat, GLenum& glformat, GLenum& gltype, KlayGE::PixelFormat pf)
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

		case PF_A4L4:
			internalFormat = GL_LUMINANCE4_ALPHA4;
			glformat = GL_LUMINANCE_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_A8L8:
			internalFormat = GL_LUMINANCE8_ALPHA8;
			glformat = GL_LUMINANCE_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_R5G6B5:
			internalFormat = GL_RGB5;
			glformat = GL_BGR;
			gltype = GL_UNSIGNED_SHORT_5_6_5_REV;
			break;

		case PF_A4R4G4B4:
			internalFormat = GL_RGBA4;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_SHORT_4_4_4_4_REV;
			break;

		case PF_X8R8G8B8:
			internalFormat = GL_RGB8;
			glformat = GL_BGR;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case PF_A8R8G8B8:
			internalFormat = GL_RGBA8;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_INT_8_8_8_8_REV;
			break;

		case PF_A2R10G10B10:
			internalFormat = GL_RGB10_A2;
			glformat = GL_BGRA;
			gltype = GL_UNSIGNED_INT_2_10_10_10_REV;
			break;

		case PF_DXT1:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			glformat = GL_BGR;
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
		}
	}
}

namespace KlayGE
{
	OGLTexture::OGLTexture(uint32_t width, uint16_t numMipMaps,
								PixelFormat format, TextureUsage usage)
					: Texture(usage, TT_1D)
	{
		format_		= format;

		if (0 == numMipMaps)
		{
			while (width > 1)
			{
				++ numMipMaps_;

				width /= 2;
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		bpp_ = PixelFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, format_);

		glGenTextures(1, &texture_[0]);
		glBindTexture(GL_TEXTURE_1D, texture_[0]);

		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glTexImage1D(GL_TEXTURE_1D, level, glinternalFormat,
				width, 0, glformat, gltype, NULL);

			width /= 2;
		}

		this->UpdateParams();
	}

	OGLTexture::OGLTexture(uint32_t width, uint32_t height,
								uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
					: Texture(usage, TT_2D)
	{
		format_		= format;

		if (0 == numMipMaps)
		{
			while ((width > 1) && (height > 1))
			{
				++ numMipMaps_;

				width /= 2;
				height /= 2;
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		bpp_ = PixelFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, format_);

		glGenTextures(1, &texture_[0]);
		glBindTexture(GL_TEXTURE_2D, texture_[0]);

		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glTexImage2D(GL_TEXTURE_2D, level, glinternalFormat,
				width, height, 0, glformat, gltype, NULL);

			width /= 2;
			height /= 2;
		}

		this->UpdateParams();
	}

	OGLTexture::OGLTexture(uint32_t width, uint32_t height, uint32_t depth,
								uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
					: Texture(usage, TT_3D)
	{
		format_		= format;

		if (0 == numMipMaps)
		{
			while ((width > 1) && (height > 1) && (depth > 1))
			{
				++ numMipMaps_;

				width /= 2;
				height /= 2;
				depth /= 2;
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		bpp_ = PixelFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, format_);

		glGenTextures(1, &texture_[0]);
		glBindTexture(GL_TEXTURE_3D, texture_[0]);

		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glTexImage3D(GL_TEXTURE_3D, level, glinternalFormat,
				width, height, depth, 0, glformat, gltype, NULL);

			width /= 2;
			height /= 2;
			depth /= 2;
		}

		this->UpdateParams();
	}

	OGLTexture::OGLTexture(uint32_t size, bool /*cube*/, uint16_t numMipMaps,
								PixelFormat format, TextureUsage usage)
					: Texture(usage, TT_Cube)
	{
		format_		= format;

		if (0 == numMipMaps)
		{
			while (size > 1)
			{
				++ numMipMaps_;

				size /= 2;
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		bpp_ = PixelFormatBits(format_);

		usage_ = usage;

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, format_);

		glGenTextures(6, &texture_[0]);

		for (int face = 0; face < 6; ++ face)
		{
			for (uint16_t level = 0; level < numMipMaps_; ++ level)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
					size, size, 0, glformat, gltype, NULL);

				size /= 2;
			}
		}

		this->UpdateParams();
	}

	OGLTexture::~OGLTexture()
	{
		if (TT_Cube == type_)
		{
			glDeleteTextures(6, &texture_[0]);
		}
		else
		{
			glDeleteTextures(1, &texture_[0]);
		}
	}

	std::wstring const & OGLTexture::Name() const
	{
		static const std::wstring name(L"OpenGL Texture");
		return name;
	}

	uint32_t OGLTexture::Width(int level) const
	{
		return static_cast<GLint>(widths_[level]);
	}
	
	uint32_t OGLTexture::Height(int level) const
	{
		return static_cast<GLint>(heights_[level]);
	}

	uint32_t OGLTexture::Depth(int level) const
	{
		return static_cast<GLint>(depths_[level]);
	}

	void OGLTexture::CopyToTexture(Texture& target)
	{
		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		Convert(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		Convert(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		switch (type_)
		{
		case TT_2D:
			{
				std::vector<uint8_t> data_in;
				std::vector<uint8_t> data_out;
				for (int level = 0; level < numMipMaps_; ++ level)
				{
					data_in.resize(this->Width(level) * this->Height(level) * bpp_ / 8);
					data_out.resize(target.Width(level) * target.Height(level) * target.Bpp() / 8);

					this->CopyToMemory2D(level, &data_in[0]);

					gluScaleImage(gl_format, this->Width(level), this->Height(level), GL_UNSIGNED_BYTE, &data_in[0],
						target.Width(0), target.Height(0), gl_type, &data_out[0]);

					target.CopyMemoryToTexture2D(level, &data_out[0], format_,
						target.Width(level), target.Height(level), 0, 0);
				}
			}
			break;
		}
	}

	void OGLTexture::CopyToMemory1D(int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, format_);

		glBindTexture(GL_TEXTURE_1D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImageARB(GL_TEXTURE_1D, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_1D, level, glformat, gltype, data);
		}
	}

	void OGLTexture::CopyToMemory2D(int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, format_);

		glBindTexture(GL_TEXTURE_2D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImageARB(GL_TEXTURE_2D, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_2D, level, glformat, gltype, data);
		}
	}

	void OGLTexture::CopyToMemory3D(int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, format_);

		glBindTexture(GL_TEXTURE_3D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImageARB(GL_TEXTURE_3D, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_3D, level, glformat, gltype, data);
		}
	}

	void OGLTexture::CopyToMemoryCube(CubeFaces face, int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, format_);

		glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImageARB(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glformat, gltype, data);
		}
	}

	void OGLTexture::CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
		uint32_t width, uint32_t xOffset)
	{
		BOOST_ASSERT(width != 0);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, pf);

		glBindTexture(GL_TEXTURE_2D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (PF_DXT1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			GLsizei const image_size = ((width + 3) / 4) * block_size;

			glCompressedTexSubImage1D(GL_TEXTURE_1D, level, xOffset,
				width, glformat, image_size, data);
		}
		else
		{
			glTexSubImage1D(GL_TEXTURE_1D, level, xOffset,
				width, glformat, gltype, data);
		}
	}

	void OGLTexture::CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
		uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset)
	{
		BOOST_ASSERT(width != 0);
		BOOST_ASSERT(height != 0);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, pf);

		glBindTexture(GL_TEXTURE_2D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (PF_DXT1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			GLsizei const image_size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;

			glCompressedTexSubImage2D(GL_TEXTURE_2D, level, xOffset, yOffset,
				width, height, glformat, image_size, data);
		}
		else
		{
			glTexSubImage2D(GL_TEXTURE_2D, level, xOffset, yOffset,
				width, height, glformat, gltype, data);
		}
	}

	void OGLTexture::CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t depth,
			uint32_t xOffset, uint32_t yOffset, uint32_t zOffset)
	{
		BOOST_ASSERT(width != 0);
		BOOST_ASSERT(height != 0);
		BOOST_ASSERT(depth != 0);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, pf);

		glBindTexture(GL_TEXTURE_3D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (PF_DXT1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			GLsizei const image_size = ((width + 3) / 4) * ((height + 3) / 4) * ((depth + 3) / 4) * block_size;

			glCompressedTexSubImage3D(GL_TEXTURE_3D, level, xOffset, yOffset, zOffset,
				width, height, depth, glformat, image_size, data);
		}
		else
		{
			glTexSubImage3D(GL_TEXTURE_3D, level, xOffset, yOffset, zOffset,
				width, height, depth, glformat, gltype, data);
		}
	}

	void OGLTexture::CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t size, uint32_t xOffset)
	{
		BOOST_ASSERT(size != 0);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		Convert(glinternalFormat, glformat, gltype, pf);

		glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + face, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (PF_DXT1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			GLsizei const image_size = ((size + 3) / 4) * ((size + 3) / 4) * block_size;

			glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + face, level, xOffset, xOffset,
				size, size, glformat, image_size, data);
		}
		else
		{
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + face, level, xOffset, xOffset,
				size, size, glformat, gltype, data);
		}
	}

	void OGLTexture::BuildMipSubLevels()
	{
	}

	void OGLTexture::CustomAttribute(std::string const & /*name*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::UpdateParams()
	{
		GLint w, h, d;

		widths_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		depths_.resize(numMipMaps_);
		switch (type_)
		{
		case TT_1D:
			glBindTexture(GL_TEXTURE_1D, texture_[0]);
			for (uint16_t level = 0; level < numMipMaps_; ++ level)
			{
				glGetTexLevelParameteriv(GL_TEXTURE_1D, level, GL_TEXTURE_WIDTH, &w);
				widths_[level] = w;

				glGetTexLevelParameteriv(GL_TEXTURE_1D, level, GL_TEXTURE_HEIGHT, &h);
				heights_[level] = h;

				glGetTexLevelParameteriv(GL_TEXTURE_1D, level, GL_TEXTURE_DEPTH, &d);
				depths_[level] = d;
			}
			break;

		case TT_2D:
			glBindTexture(GL_TEXTURE_2D, texture_[0]);
			for (uint16_t level = 0; level < numMipMaps_; ++ level)
			{
				glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &w);
				widths_[level] = w;

				glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &h);
				heights_[level] = h;

				glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_DEPTH, &d);
				depths_[level] = d;
			}
			break;

		case TT_3D:
			glBindTexture(GL_TEXTURE_3D, texture_[0]);
			for (uint16_t level = 0; level < numMipMaps_; ++ level)
			{
				glGetTexLevelParameteriv(GL_TEXTURE_3D, level, GL_TEXTURE_WIDTH, &w);
				widths_[level] = w;

				glGetTexLevelParameteriv(GL_TEXTURE_3D, level, GL_TEXTURE_HEIGHT, &h);
				heights_[level] = h;

				glGetTexLevelParameteriv(GL_TEXTURE_3D, level, GL_TEXTURE_DEPTH, &d);
				depths_[level] = d;
			}
			break;

		case TT_Cube:
			glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X, texture_[0]);
			for (uint16_t level = 0; level < numMipMaps_; ++ level)
			{
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, level, GL_TEXTURE_WIDTH, &w);
				widths_[level] = w;

				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, level, GL_TEXTURE_HEIGHT, &h);
				heights_[level] = h;

				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, level, GL_TEXTURE_DEPTH, &d);
				depths_[level] = d;
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void OGLTexture::GLBindTexture()
	{
		switch (type_)
		{
		case TT_1D:
			glBindTexture(GL_TEXTURE_1D, texture_[0]);
			break;

		case TT_2D:
			glBindTexture(GL_TEXTURE_2D, texture_[0]);
			break;

		case TT_3D:
			glBindTexture(GL_TEXTURE_3D, texture_[0]);
			break;

		case TT_Cube:
			for (int face = 0; face < 6; ++ face)
			{
				glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture_[face]);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	GLenum OGLTexture::GLType() const
	{
		switch (type_)
		{
		case TT_1D:
			return GL_TEXTURE_1D;

		case TT_2D:
			return GL_TEXTURE_2D;

		case TT_3D:
			return GL_TEXTURE_3D;

		case TT_Cube:
			return GL_TEXTURE_CUBE_MAP;

		default:
			BOOST_ASSERT(false);
			return GL_TEXTURE_1D;
		}
	}
}
