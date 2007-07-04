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
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Context.hpp>

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

		bpp_ = NumFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

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

					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
					glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW_ARB);
					uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
					memset(p, 0, image_size);
					glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

					glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
						size, size, 0, image_size, NULL);
				}
				else
				{
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					
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
		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		OGLMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		size_t const size = NumFormatBytes(format_);
		std::vector<uint8_t> data;
		for (int level = 0; level < numMipMaps_; ++ level)
		{
			data.resize(this->Width(level) * this->Height(level) * bpp_ / 8);

			for (int face = 0; face < 6; ++ face)
			{
				this->CopyToMemoryCube(static_cast<CubeFaces>(face), level, &data[0]);
				target.CopyMemoryToTextureCube(static_cast<CubeFaces>(face), level, &data[0], format_,
					target.Width(level), target.Height(level), 0, 0,
					this->Width(level), this->Height(level), 0, 0,
					this->Width(level) * size);
			}
		}
	}

	void OGLTextureCube::CopyToTextureCube(Texture& target, CubeFaces face, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset)
	{
		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		OGLMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		std::vector<uint8_t> data(src_width * src_height * bpp_ / 8);
		this->CopyToMemoryCube(static_cast<CubeFaces>(face), level, &data[0]);
		target.CopyMemoryToTextureCube(static_cast<CubeFaces>(face), level, &data[0], format_,
			dst_width, dst_height, dst_xOffset, dst_yOffset,
			src_width, src_height, src_xOffset, src_yOffset,
			this->Width(level) * NumFormatBytes(format_));
	}

	void OGLTextureCube::CopyToMemoryCube(CubeFaces face, int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glformat, gltype, data);
		}
	}

	void OGLTextureCube::CopyMemoryToTextureCube(CubeFaces face, int level, void const * data, ElementFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset,
			uint32_t src_row_pitch)
	{
		BOOST_ASSERT(src_width != 0);
		BOOST_ASSERT(src_height != 0);
		BOOST_ASSERT(dst_width != 0);
		BOOST_ASSERT(dst_height != 0);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, pf);

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

			GLsizei const image_size = ((dst_width + 3) / 4) * ((dst_height + 3) / 4) * block_size;

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW_ARB);
			uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
			memcpy(p, data, image_size);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

			glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, dst_xOffset, dst_yOffset,
				dst_width, dst_height, glformat, image_size, NULL);
		}
		else
		{
			size_t num_bytes_pf = NumFormatBytes(pf);

			std::vector<uint8_t> resized_data(src_width * src_height * num_bytes_pf);
			{
				uint8_t const * src = static_cast<uint8_t const *>(data) + (src_yOffset * src_width + src_xOffset) * num_bytes_pf;
				uint8_t* dst = &resized_data[0];
				for (size_t i = 0; i < src_height; ++ i)
				{
					memcpy(dst, src, src_width * num_bytes_pf);
					src += src_row_pitch;
					dst += src_width * num_bytes_pf;
				}
			}
			uint32_t data_size = dst_width * dst_height * num_bytes_pf;
			if ((dst_width != src_width) || (dst_height != src_height))
			{
				std::vector<uint8_t> tmp(data_size);
				gluScaleImage(glformat, src_width, src_height, gltype, &resized_data[0],
					dst_width, dst_height, gltype, &tmp[0]);
				resized_data = tmp;
			}

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, data_size, NULL, GL_STREAM_DRAW_ARB);
			uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
			memcpy(p, &resized_data[0], data_size);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, dst_xOffset, dst_yOffset,
				dst_width, dst_height, glformat, gltype, NULL);
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
