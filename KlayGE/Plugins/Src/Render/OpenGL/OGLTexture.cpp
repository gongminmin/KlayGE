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

		case PF_DXT1:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			glformat = GL_BGR;
			break;

		case PF_DXT3:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			glformat = GL_BGRA;
			break;

		case PF_DXT5:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			glformat = GL_BGRA;
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
		this->InitParams();

		format_		= format;
		width_		= width;
		height_		= 1;
		depth_		= 1;

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
		Convert(glinternalFormat, glformat, format_);

		glGenTextures(1, &texture_[0]);
		glBindTexture(GL_TEXTURE_1D, texture_[0]);

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

			GLsizei const image_size = ((width_ + 3) / 4) * ((height_ + 3) / 4) * block_size;

			glCompressedTexImage1D(GL_TEXTURE_1D, numMipMaps_, glinternalFormat,
				width_, 0, image_size, NULL);
		}
		else
		{
			glTexImage1D(GL_TEXTURE_1D, numMipMaps_, glinternalFormat,
				width_, 0, glformat, GL_UNSIGNED_BYTE, NULL);
		}

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		this->UpdateParams();
	}

	OGLTexture::OGLTexture(uint32_t width, uint32_t height,
								uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
					: Texture(usage, TT_2D)
	{
		this->InitParams();

		format_		= format;
		width_		= width;
		height_		= height;
		depth_		= 1;

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
		Convert(glinternalFormat, glformat, format_);

		glGenTextures(1, &texture_[0]);
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

			GLsizei const image_size = ((width_ + 3) / 4) * ((height_ + 3) / 4) * block_size;

			glCompressedTexImage2D(GL_TEXTURE_2D, numMipMaps_, glinternalFormat,
				width_, height_, 0, image_size, NULL);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, numMipMaps_, glinternalFormat,
				width_, height_, 0, glformat, GL_UNSIGNED_BYTE, NULL);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		this->UpdateParams();
	}

	OGLTexture::OGLTexture(uint32_t width, uint32_t height, uint32_t depth,
								uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
					: Texture(usage, TT_3D)
	{
		this->InitParams();

		format_		= format;
		width_		= width;
		height_		= height;
		depth_		= depth;

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
		Convert(glinternalFormat, glformat, format_);

		glGenTextures(1, &texture_[0]);
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

			GLsizei const image_size = ((width_ + 3) / 4) * ((height_ + 3) / 4) * block_size;

			glCompressedTexImage3D(GL_TEXTURE_3D, numMipMaps_, glinternalFormat,
				width_, height_, depth_, 0, image_size, NULL);
		}
		else
		{
			glTexImage3D(GL_TEXTURE_3D, numMipMaps_, glinternalFormat,
				width_, height_, depth_, 0, glformat, GL_UNSIGNED_BYTE, NULL);
		}

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		this->UpdateParams();
	}

	OGLTexture::OGLTexture(uint32_t size, bool /*cube*/, uint16_t numMipMaps,
								PixelFormat format, TextureUsage usage)
					: Texture(usage, TT_Cube)
	{
		this->InitParams();

		format_		= format;
		width_		= size;
		height_		= size;
		depth_		= 1;

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
		Convert(glinternalFormat, glformat, format_);

		glGenTextures(6, &texture_[0]);

		for (int face = 0; face < 6; ++ face)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture_[face]);

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

				GLsizei const image_size = ((width_ + 3) / 4) * ((height_ + 3) / 4) * block_size;

				glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, numMipMaps_, glinternalFormat,
					width_, height_, 0, image_size, NULL);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, numMipMaps_, glinternalFormat,
					width_, height_, 0, glformat, GL_UNSIGNED_BYTE, NULL);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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

	void OGLTexture::InitParams()
	{
		addr_mode_u_ = TAM_Wrap;
		addr_mode_v_ = TAM_Wrap;
		addr_mode_w_ = TAM_Wrap;
		
		min_filter_ = TFO_Point;
		mag_filter_ = TFO_Point;
		mip_filter_ = TFO_Point;

		anisotropy_ = 0;

		max_mip_level_ = 0;
		mip_map_lod_bias_ = 0;
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
		assert(dynamic_cast<OGLTexture*>(&target) != NULL);

		GLint gl_internal_format;
		GLenum gl_format;
		Convert(gl_internal_format, gl_format, format_);
		
		GLint gl_target_internal_format;
		GLenum gl_target_format;
		Convert(gl_target_internal_format, gl_target_format, target.Format());

		switch (type_)
		{
		case TT_2D:
			{
				std::vector<uint8_t> data_in(width_ * height_ * bpp_ / 8);
				std::vector<uint8_t> data_out(target.Width(0) * target.Height(0) * target.Bpp() / 8);
				for (int level = 0; level < numMipMaps_; ++ level)
				{
					this->CopyToMemory2D(level, &data_in[0]);

					gluScaleImage(gl_format, this->Width(level), this->Height(level), GL_UNSIGNED_BYTE, &data_in[0],
						target.Width(0), target.Height(0), GL_UNSIGNED_BYTE, &data_out[0]);

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
		Convert(glinternalFormat, glformat, format_);

		glBindTexture(GL_TEXTURE_1D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImageARB(GL_TEXTURE_1D, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_1D, level, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::CopyToMemory2D(int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, format_);

		glBindTexture(GL_TEXTURE_2D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImageARB(GL_TEXTURE_2D, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_2D, level, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::CopyToMemory3D(int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, format_);

		glBindTexture(GL_TEXTURE_3D, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImageARB(GL_TEXTURE_3D, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_3D, level, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::CopyToMemoryCube(CubeFaces face, int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, format_);

		glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture_[0]);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImageARB(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
		uint32_t width, uint32_t xOffset)
	{
		assert(width != 0);

		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, pf);

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
				width, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
		uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset)
	{
		assert(width != 0);
		assert(height != 0);

		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, pf);

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
				width, height, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t depth,
			uint32_t xOffset, uint32_t yOffset, uint32_t zOffset)
	{
		assert(width != 0);
		assert(height != 0);
		assert(depth != 0);

		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, pf);

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
				width, height, depth, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t size, uint32_t xOffset)
	{
		assert(size != 0);

		GLint glinternalFormat;
		GLenum glformat;
		Convert(glinternalFormat, glformat, pf);

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
				size, size, glformat, GL_UNSIGNED_BYTE, data);
		}
	}

	void OGLTexture::BuildMipSubLevels()
	{
	}

	void OGLTexture::CustomAttribute(std::string const & /*name*/, void* /*data*/)
	{
		assert(false);
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
			assert(false);
			break;
		}

		width_ = widths_[0];
		height_ = heights_[0];
		depth_ = depths_[0];
	}

	void OGLTexture::AddressingMode(TexAddressingType type, TexAddressingMode tam)
	{
		this->GLBindTexture();

		GLint mode = OGLMapping::Mapping(tam);

		switch (type)
		{
		case TAT_Addr_U:
			addr_mode_u_ = tam;
			glTexParameteri(this->GLType(), GL_TEXTURE_WRAP_S, mode);
			break;

		case TAT_Addr_V:
			addr_mode_v_ = tam;
			glTexParameteri(this->GLType(), GL_TEXTURE_WRAP_T, mode);
			break;

		case TAT_Addr_W:
			addr_mode_w_ = tam;
			glTexParameteri(this->GLType(), GL_TEXTURE_WRAP_R, mode);
			break;

		default:
			assert(false);
			break;
		}
	}

	Texture::TexAddressingMode OGLTexture::AddressingMode(TexAddressingType type) const
	{
		switch (type)
		{
		case TAT_Addr_U:
			return addr_mode_u_;

		case TAT_Addr_V:
			return addr_mode_v_;

		case TAT_Addr_W:
			return addr_mode_w_;

		default:
			assert(false);
			return addr_mode_u_;
		}
	}

	void OGLTexture::Filtering(TexFilterType type, TexFilterOp op)
	{
		this->GLBindTexture();

		GLint mode = GL_NEAREST;
		if (type != TFT_Min)
		{
			switch (op)
			{
			case TFO_None:
			case TFO_Point:
				mode = GL_NEAREST;
				break;

			case TFO_Bilinear:
			case TFO_Trilinear:
			case TFO_Anisotropic:
				mode = GL_LINEAR;
				break;

			default:
				assert(false);
				break;
			}
		}
		else
		{
			switch (op)
			{
			case TFO_None:
			case TFO_Point:
				switch (mip_filter_)
				{
				case TFO_None:
					// nearest min, no mip
					mode = GL_NEAREST;
					break;

				case TFO_Point:
					// nearest min, nearest mip
					mode = GL_NEAREST_MIPMAP_NEAREST;
					break;

				case TFO_Bilinear:
				case TFO_Trilinear:
				case TFO_Anisotropic:
					// nearest min, linear mip
					mode = GL_NEAREST_MIPMAP_LINEAR;
					break;
				}
				break;

			case TFO_Bilinear:
			case TFO_Trilinear:
			case TFO_Anisotropic:
				switch (mip_filter_)
				{
				case TFO_None:
					// linear min, no mip
					mode = GL_LINEAR;
					break;

				case TFO_Point:
					// linear min, point mip
					mode = GL_LINEAR_MIPMAP_NEAREST;
					break;

				case TFO_Bilinear:
				case TFO_Trilinear:
				case TFO_Anisotropic:
					// linear min, linear mip
					mode = GL_LINEAR_MIPMAP_LINEAR;
					break;
				}
				break;

			default:
				assert(false);
				break;
			}
		}

		switch (type)
		{
		case TFT_Min:
			min_filter_ = op;
			glTexParameteri(this->GLType(), GL_TEXTURE_MIN_FILTER, mode);
			break;

		case TFT_Mag:
			mag_filter_ = op;
			glTexParameteri(this->GLType(), GL_TEXTURE_MAG_FILTER, mode);
			break;

		case TFT_Mip:
			mip_filter_ = op;
			glTexParameteri(this->GLType(), GL_TEXTURE_MIN_FILTER, mode);
			break;

		default:
			assert(false);
			break;
		}
	}

	Texture::TexFilterOp OGLTexture::Filtering(TexFilterType type) const
	{
		switch (type)
		{
		case TFT_Min:
			return min_filter_;

		case TFT_Mag:
			return mag_filter_;

		case TFT_Mip:
			return mip_filter_;

		default:
			assert(false);
			return min_filter_;
		}
	}

	void OGLTexture::Anisotropy(uint32_t maxAnisotropy)
	{
		RenderEngine const & renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		uint32_t max_texture_anisotropy = renderEngine.DeviceCaps().max_texture_anisotropy;

		if (max_texture_anisotropy != 0)
		{
			anisotropy_ = std::min(max_texture_anisotropy, maxAnisotropy);
			glTexParameteri(this->GLType(), GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy_);
		}
	}

	uint32_t OGLTexture::Anisotropy() const
	{
		return anisotropy_;
	}

	// 设置最大的mip等级
	/////////////////////////////////////////////////////////////////////////////////
	void OGLTexture::MaxMipLevel(uint32_t level)
	{
		if (glloader_is_supported("GL_VERSION_1_2")
			|| glloader_is_supported("GL_SGIS_texture_lod"))
		{
			this->GLBindTexture();

			max_mip_level_ = level;
			glTexParameteri(this->GLType(), GL_TEXTURE_MAX_LEVEL, max_mip_level_);
		}
	}
	
	// 获取最大的mip等级
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t OGLTexture::MaxMipLevel() const
	{
		return max_mip_level_;
	}

	// 设置mip map偏移量
	/////////////////////////////////////////////////////////////////////////////////
	void OGLTexture::MipMapLodBias(float bias)
	{
		this->GLBindTexture();

		if (glloader_is_supported("GL_VERSION_1_4")
			|| glloader_is_supported("GL_EXT_texture_lod_bias"))
		{
			GLfloat max_bias;
			glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &max_bias);
			mip_map_lod_bias_ = std::min(bias, max_bias);
			glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, mip_map_lod_bias_);
		}
		else
		{
			if (glloader_is_supported("GL_SGIX_texture_lod_bias"))
			{
				mip_map_lod_bias_ = bias;
				glTexParameterf(this->GLType(), GL_TEXTURE_LOD_BIAS_S_SGIX, mip_map_lod_bias_);
				glTexParameterf(this->GLType(), GL_TEXTURE_LOD_BIAS_T_SGIX, mip_map_lod_bias_);
				glTexParameterf(this->GLType(), GL_TEXTURE_LOD_BIAS_R_SGIX, mip_map_lod_bias_);
			}
		}
	}

	// 获取mip map偏移量
	/////////////////////////////////////////////////////////////////////////////////
	float OGLTexture::MipMapLodBias() const
	{
		return mip_map_lod_bias_;
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
			assert(false);
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
			assert(false);
			return GL_TEXTURE_1D;
		}
	}
}
