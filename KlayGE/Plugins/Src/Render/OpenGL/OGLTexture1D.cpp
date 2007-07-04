// OGLTexture1D.hpp
// KlayGE OpenGL 1D纹理类 实现文件
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
	OGLTexture1D::OGLTexture1D(uint32_t width, uint16_t numMipMaps,
								ElementFormat format)
					: OGLTexture(TT_1D)
	{
		if (!glloader_GL_EXT_texture_sRGB())
		{
			format = this->SRGBToRGB(format);
		}

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

		bpp_ = NumFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		glGenTextures(1, &texture_);
		glBindTexture(GL_TEXTURE_1D, texture_);

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

				GLsizei const image_size = ((width + 3) / 4) * block_size;

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
				glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW_ARB);
				uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
				memset(p, 0, image_size);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				glCompressedTexImage1D(GL_TEXTURE_1D, level, glinternalFormat,
					width, 0, image_size, NULL);
			}
			else
			{
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				
				glTexImage1D(GL_TEXTURE_1D, level, glinternalFormat,
					width, 0, glformat, gltype, NULL);
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			width /= 2;
		}

		this->UpdateParams();
	}

	uint32_t OGLTexture1D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return static_cast<GLint>(widths_[level]);
	}

	void OGLTexture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		OGLTexture1D& other = static_cast<OGLTexture1D&>(target);

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		OGLMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		std::vector<uint8_t> data;
		for (int level = 0; level < numMipMaps_; ++ level)
		{
			data.resize(this->Width(level) * bpp_ / 8);

			this->CopyToMemory1D(level, &data[0]);
			other.CopyMemoryToTexture1D(level, &data[0], format_,
				target.Width(level), 0, this->Width(level), 0);
		}
	}

	void OGLTexture1D::CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset)
	{
		BOOST_ASSERT(type_ == target.Type());

		OGLTexture1D& other = static_cast<OGLTexture1D&>(target);

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		OGLMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		std::vector<uint8_t> data(src_width * bpp_ / 8);

		this->CopyToMemory1D(level, &data[0]);
		other.CopyMemoryToTexture1D(level, &data[0], format_,
			dst_width, dst_xOffset, src_width, src_xOffset);
	}

	void OGLTexture1D::CopyToMemory1D(int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		glBindTexture(GL_TEXTURE_1D, texture_);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImage(GL_TEXTURE_1D, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_1D, level, glformat, gltype, data);
		}
	}

	void OGLTexture1D::CopyMemoryToTexture1D(int level, void const * data, ElementFormat pf,
		uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset)
	{
		BOOST_ASSERT(src_width != 0);
		BOOST_ASSERT(dst_width != 0);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, pf);

		glBindTexture(GL_TEXTURE_1D, texture_);

		uint32_t data_size = dst_width * NumFormatBytes(pf);
		uint8_t const * resized_data = static_cast<uint8_t const *>(data) + src_xOffset * NumFormatBytes(pf);
		std::vector<uint8_t> tmp;
		if (dst_width != src_width)
		{
			tmp.resize(data_size);
			gluScaleImage(glformat, src_width, 1, gltype, resized_data,
				dst_width, 1, gltype, &tmp[0]);
			resized_data = &tmp[0];
		}

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

			GLsizei const image_size = ((dst_width + 3) / 4) * block_size;

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW_ARB);
			uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
			memcpy(p, resized_data, image_size);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

			glCompressedTexSubImage1D(GL_TEXTURE_1D, level, dst_xOffset,
				dst_width, glformat, image_size, NULL);
		}
		else
		{
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, data_size, NULL, GL_STREAM_DRAW_ARB);
			uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
			memcpy(p, resized_data, data_size);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

			glTexSubImage1D(GL_TEXTURE_1D, level, dst_xOffset,
				dst_width, glformat, gltype, NULL);
		}
	}

	void OGLTexture1D::UpdateParams()
	{
		GLint w;

		widths_.resize(numMipMaps_);
		
		glBindTexture(GL_TEXTURE_1D, texture_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glGetTexLevelParameteriv(GL_TEXTURE_1D, level, GL_TEXTURE_WIDTH, &w);
			widths_[level] = w;
		}
	}
}
