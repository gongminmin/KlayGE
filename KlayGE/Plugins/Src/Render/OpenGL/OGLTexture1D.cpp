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
			uint32_t w = width;
			while (w > 1)
			{
				++ numMipMaps_;

				w /= 2;
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

		pbos_.resize(numMipMaps_);
		glGenBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);

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

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
				glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
				uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
				memset(p, 0, image_size);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				glCompressedTexImage1D(GL_TEXTURE_1D, level, glinternalFormat,
					width, 0, image_size, NULL);
			}
			else
			{
				GLsizei const image_size = width * bpp_ / 8;

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
				glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
				uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
				memset(p, 0, image_size);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				glTexImage1D(GL_TEXTURE_1D, level, glinternalFormat,
					width, 0, glformat, gltype, NULL);
			}

			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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

		for (int level = 0; level < numMipMaps_; ++ level)
		{
			this->CopyToTexture1D(target, level, target.Width(level), 0,
				this->Width(level), 0);
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

		std::vector<uint8_t> data_in(src_width * this->Bpp() / 8);
		std::vector<uint8_t> data_out(dst_width * other.Bpp() / 8);
		
		void* p;
		this->Map1D(level, TMA_Read_Only, src_xOffset, src_width, p);
		memcpy(&data_in[0], static_cast<uint8_t*>(p), data_in.size() * sizeof(data_in[0]));
		this->Unmap1D(level);

		gluScaleImage(gl_format, src_width, 1, gl_type, &data_in[0],
			dst_width, 1, gl_target_type, &data_out[0]);

		other.Map1D(level, TMA_Write_Only, dst_xOffset, dst_width, p);
		memcpy(static_cast<uint8_t*>(p), &data_out[0], data_out.size() * sizeof(data_out[0]));
		other.Unmap1D(level);
	}

	void OGLTexture1D::Map1D(int level, TextureMapAccess tma, uint32_t x_offset, uint32_t width, void*& data)
	{
		last_tma_ = tma;
		last_x_offset_ = x_offset;
		last_width_ = width;

		uint32_t const size_fmt = NumFormatBytes(format_);

		switch (tma)
		{
		case TMA_Read_Only:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

				glBufferData(GL_PIXEL_PACK_BUFFER, width * size_fmt, NULL, GL_STREAM_READ);

				glBindTexture(GL_TEXTURE_1D, texture_);
				glGetTexImage(GL_TEXTURE_1D, level, gl_format, gl_type, NULL);

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			}
			break;

		case TMA_Write_Only:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
				std::vector<uint8_t> zero(width * size_fmt);
				glBufferData(GL_PIXEL_UNPACK_BUFFER, width * size_fmt, &zero[0], GL_STREAM_DRAW);
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
				
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

				glBufferData(GL_PIXEL_PACK_BUFFER, width * size_fmt, NULL, GL_STREAM_READ);

				glBindTexture(GL_TEXTURE_1D, texture_);
				glGetTexImage(GL_TEXTURE_1D, level, gl_format, gl_type, NULL);

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void OGLTexture1D::Unmap1D(int level)
	{
		switch (last_tma_)
		{
		case TMA_Read_Only:
			{
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}
			break;

		case TMA_Write_Only:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindTexture(GL_TEXTURE_1D, texture_);

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

					GLsizei const image_size = ((last_width_ + 3) / 4) * block_size;
				
					glCompressedTexSubImage1D(GL_TEXTURE_1D, level, last_x_offset_,
						last_width_, gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage1D(GL_TEXTURE_1D, level, last_x_offset_, last_width_,
							gl_format, gl_type, NULL);
				}
			}
			break;

		case TMA_Read_Write:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);

				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindTexture(GL_TEXTURE_1D, texture_);

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

					GLsizei const image_size = ((last_width_ + 3) / 4) * block_size;
				
					glCompressedTexSubImage1D(GL_TEXTURE_1D, level, last_x_offset_,
						last_width_, gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage1D(GL_TEXTURE_1D, level, last_x_offset_, last_width_,
							gl_format, gl_type, NULL);
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
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
