// OGLTextureCube.hpp
// KlayGE OpenGL Cube纹理类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 用pbo加速 (2007.3.13)
//
// 3.2.0
// 初次建立 (2006.4.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>
#include <gl/glu.h>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	OGLTextureCube::OGLTextureCube(uint32_t size, uint16_t numMipMaps,
								ElementFormat format)
					: OGLTexture(TT_Cube)
	{
		if (!glloader_GL_EXT_texture_sRGB())
		{
			format = this->SRGBToRGB(format);
		}

		format_		= format;

		if (0 == numMipMaps)
		{
			uint32_t s = size;
			while (s > 1)
			{
				++ numMipMaps_;

				size /= 2;
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		bpp_ = NumFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		pbos_.resize(numMipMaps_ * 6);
		glGenBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);

		glGenTextures(1, &texture_);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);

		for (int face = 0; face < 6; ++ face)
		{
			for (uint16_t level = 0; level < numMipMaps_; ++ level)
			{
				if (IsCompressedFormat(format_))
				{
					int block_size;
					if (EF_BC1 == format_)
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					GLsizei const image_size = ((size + 3) / 4) * ((size + 3) / 4) * block_size;

					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[face * numMipMaps_ + level]);
					glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
					uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
					memset(p, 0, image_size);
					glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

					glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
						size, size, 0, image_size, NULL);
				}
				else
				{
					GLsizei const image_size = size * size * bpp_ / 8;

					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[face * numMipMaps_ + level]);
					glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
					uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
					memset(p, 0, image_size);
					glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
						size, size, 0, glformat, gltype, NULL);
				}

				size /= 2;
			}
		}

		this->UpdateParams();
	}

	uint32_t OGLTextureCube::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return static_cast<GLint>(widths_[level]);
	}

	uint32_t OGLTextureCube::Height(int level) const
	{
		return this->Width(level);
	}

	void OGLTextureCube::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		for (int face = 0; face < 6; ++ face)
		{
			for (int level = 0; level < numMipMaps_; ++ level)
			{
				this->CopyToTextureCube(target, static_cast<CubeFaces>(face),
					level, target.Width(level), target.Height(level), 0, 0,
					this->Width(level), this->Height(level), 0, 0);
			}
		}
	}

	void OGLTextureCube::CopyToTextureCube(Texture& target, CubeFaces face, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset)
	{
		BOOST_ASSERT(type_ == target.Type());

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		OGLMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		size_t const src_format_size = this->Bpp() / 8;
		size_t const dst_format_size = target.Bpp() / 8;

		std::vector<uint8_t> data_in(src_width * src_height * src_format_size);
		std::vector<uint8_t> data_out(dst_width * dst_height * dst_format_size);

		{
			Texture::Mapper mapper(*this, face, level, TMA_Read_Only, src_xOffset, src_yOffset, src_width, src_height);
			uint8_t const * s = mapper.Pointer<uint8_t>();
			uint8_t* d = &data_in[0];
			for (uint32_t y = 0; y < src_height; ++ y)
			{
				memcpy(d, s, src_width * src_format_size);

				s += mapper.RowPitch();
				d += src_width * src_format_size;
			}
		}

		gluScaleImage(gl_format, src_width, src_height, gl_type, &data_in[0],
			dst_width, dst_height, gl_target_type, &data_out[0]);

		{
			Texture::Mapper mapper(target, face, level, TMA_Write_Only, dst_xOffset, dst_yOffset, dst_width, dst_height);
			uint8_t const * s = &data_out[0];
			uint8_t* d = mapper.Pointer<uint8_t>();
			for (uint32_t y = 0; y < src_height; ++ y)
			{
				memcpy(d, s, src_width * src_format_size);

				s += src_width * src_format_size;
				d += mapper.RowPitch();
			}
		}
	}

	void OGLTextureCube::MapCube(CubeFaces face, int level, TextureMapAccess tma,
					uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
					void*& data, uint32_t& row_pitch)
	{
		last_tma_ = tma;
		last_x_offset_ = x_offset;
		last_y_offset_ = y_offset;
		last_width_ = width;
		last_height_ = height;

		uint32_t const size_fmt = NumFormatBytes(format_);

		switch (tma)
		{
		case TMA_Read_Only:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[face * numMipMaps_ + level]);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

				glBufferData(GL_PIXEL_PACK_BUFFER, width * height * size_fmt, NULL, GL_STREAM_READ);

				glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, gl_format, gl_type, NULL);

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			}
			break;

		case TMA_Write_Only:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[face * numMipMaps_ + level]);
				std::vector<uint8_t> zero(width * height * size_fmt);
				glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, width * height * size_fmt, &zero[0]);
				data = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
			}
			break;

		case TMA_Read_Write:
			// fix me
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[face * numMipMaps_ + level]);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

				glBufferData(GL_PIXEL_PACK_BUFFER, width * height * size_fmt, NULL, GL_STREAM_READ);

				glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, gl_format, gl_type, NULL);

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
		row_pitch = width * bpp_ / 8;
	}

	void OGLTextureCube::UnmapCube(CubeFaces face, int level)
	{
		switch (last_tma_)
		{
		case TMA_Read_Only:
			{
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[face * numMipMaps_ + level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}
			break;

		case TMA_Write_Only:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[face * numMipMaps_ + level]);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);

				if (IsCompressedFormat(format_))
				{
					int block_size;
					if (EF_BC1 == format_)
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					GLsizei const image_size = ((this->Width(level) + 3) / 4) * ((this->Height(level) + 3) / 4) * block_size;

					glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
						last_x_offset_, last_y_offset_, last_width_, last_height_, gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
						last_x_offset_, last_y_offset_, last_width_, last_height_,
						gl_format, gl_type, NULL);
				}
			}
			break;

		case TMA_Read_Write:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[face * numMipMaps_ + level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[face * numMipMaps_ + level]);

				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);

				if (IsCompressedFormat(format_))
				{
					int block_size;
					if (EF_BC1 == format_)
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					GLsizei const image_size = ((this->Width(level) + 3) / 4) * ((this->Height(level) + 3) / 4) * block_size;

					glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
						last_x_offset_, last_y_offset_, last_width_, last_height_, gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
						last_x_offset_, last_y_offset_, last_width_, last_height_,
						gl_format, gl_type, NULL);
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void OGLTextureCube::UpdateParams()
	{
		GLint w;

		widths_.resize(numMipMaps_);

		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, GL_TEXTURE_WIDTH, &w);
			widths_[level] = w;
		}
	}
}
