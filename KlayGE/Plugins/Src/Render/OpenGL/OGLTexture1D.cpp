// OGLTexture1D.hpp
// KlayGE OpenGL 1D纹理类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 通过EXT_framebuffer_blit加速CopyTexture (2008.10.12)
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
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>
#include <GL/glu.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	OGLTexture1D::OGLTexture1D(uint32_t width, uint16_t numMipMaps, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: OGLTexture(TT_1D, sample_count, sample_quality, access_hint)
	{
		if (!glloader_GL_EXT_texture_sRGB())
		{
			format = this->SRGBToRGB(format);
		}

		format_ = format;

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
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
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

				glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
				uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
				if (NULL == init_data)
				{
					memset(p, 0, image_size);
				}
				else
				{
					memcpy(p, init_data[level].data, image_size);
				}
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				glCompressedTexImage1D(GL_TEXTURE_1D, level, glinternalFormat,
					width, 0, image_size, NULL);
			}
			else
			{
				GLsizei const image_size = width * bpp_ / 8;

				glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
				uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
				if (NULL == init_data)
				{
					memset(p, 0, image_size);
				}
				else
				{
					memcpy(p, init_data[level].data, image_size);
				}
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				glTexImage1D(GL_TEXTURE_1D, level, glinternalFormat, width, 0, glformat, gltype, NULL);
			}

			width /= 2;
		}

		this->UpdateParams();
	}

	uint32_t OGLTexture1D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return widthes_[level];
	}

	uint32_t OGLTexture1D::Height(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(level < numMipMaps_);

		return 1;
	}

	uint32_t OGLTexture1D::Depth(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(level < numMipMaps_);

		return 1;
	}

	void OGLTexture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && (widthes_[0] == target.Width(0)))
		{
			GLint gl_internalFormat;
			GLenum gl_format;
			GLenum gl_type;
			OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

			for (int level = 0; level < numMipMaps_; ++ level)
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);

				glBindTexture(GL_TEXTURE_1D, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(GL_TEXTURE_1D, level, NULL);
				}
				else
				{
					glGetTexImage(GL_TEXTURE_1D, level, gl_format, gl_type, NULL);
				}

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
				glBindTexture(GL_TEXTURE_1D, checked_cast<OGLTexture*>(&target)->GLTexture());

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

					GLsizei const image_size = ((this->Width(level) + 3) / 4) * block_size;

					glCompressedTexSubImage1D(GL_TEXTURE_1D, level, 0,
						this->Width(level), gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage1D(GL_TEXTURE_1D, level, 0, this->Width(level), gl_format, gl_type, NULL);
				}
			}
		}
		else
		{
			for (int level = 0; level < numMipMaps_; ++ level)
			{
				this->CopyToTexture1D(target, level, target.Width(level), 0, this->Width(level), 0);
			}
		}
	}

	void OGLTexture1D::CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset)
	{
		BOOST_ASSERT(type_ == target.Type());
		
		if (!IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))) && glloader_GL_EXT_framebuffer_blit())
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			GLuint fbo_src, fbo_dst;
			re.GetFBOForBlit(fbo_src, fbo_dst);

			GLuint old_fbo = re.BindFramebuffer();

			glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo_src);
			glFramebufferTexture1DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_1D, texture_, level);

			glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo_dst);
			glFramebufferTexture1DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_1D, checked_cast<OGLTexture*>(&target)->GLTexture(), level);

			glBlitFramebufferEXT(src_xOffset, 0, src_xOffset + src_width, 1,
							dst_xOffset, 0, dst_xOffset + dst_width, 1,
							GL_COLOR_BUFFER_BIT, GL_LINEAR);

			re.BindFramebuffer(old_fbo, true);
		}
		else
		{
			BOOST_ASSERT(format_ == target.Format());

			OGLTexture1D& other = static_cast<OGLTexture1D&>(target);

			GLint gl_internalFormat;
			GLenum gl_format;
			GLenum gl_type;
			OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

			GLint gl_target_internal_format;
			GLenum gl_target_format;
			GLenum gl_target_type;
			OGLMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

			if (IsCompressedFormat(format_))
			{
				BOOST_ASSERT((0 == src_xOffset) && (0 == dst_xOffset));
				BOOST_ASSERT((src_width == dst_width));

				Texture::Mapper mapper_src(*this, level, TMA_Read_Only, src_xOffset, src_width);
				Texture::Mapper mapper_dst(target, level, TMA_Write_Only, dst_xOffset, dst_width);

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

				memcpy(mapper_dst.Pointer<uint8_t>(), mapper_src.Pointer<uint8_t>(), image_size);
			}
			else
			{
				size_t const src_format_size = NumFormatBytes(format_);
				size_t const dst_format_size = NumFormatBytes(target.Format());

				if (src_width != dst_width)
				{
					std::vector<uint8_t> data_in(src_width * src_format_size);
					std::vector<uint8_t> data_out(dst_width * dst_format_size);

					{
						Texture::Mapper mapper(*this, level, TMA_Read_Only, src_xOffset, src_width);
						memcpy(&data_in[0], mapper.Pointer<uint8_t*>(), data_in.size() * sizeof(data_in[0]));
					}

					gluScaleImage(gl_format, src_width, 1, gl_type, &data_in[0],
						dst_width, 1, gl_target_type, &data_out[0]);

					{
						Texture::Mapper mapper(other, level, TMA_Write_Only, dst_xOffset, dst_width);
						memcpy(mapper.Pointer<uint8_t*>(), &data_out[0], data_out.size() * sizeof(data_out[0]));
					}
				}
				else
				{
					Texture::Mapper mapper_src(*this, level, TMA_Read_Only, src_xOffset, src_width);
					Texture::Mapper mapper_dst(target, level, TMA_Write_Only, dst_xOffset, dst_width);
					uint8_t const * s = mapper_src.Pointer<uint8_t>();
					uint8_t* d = mapper_dst.Pointer<uint8_t>();
					
					memcpy(d, s, src_width * src_format_size);
				}
			}
		}
	}

	void OGLTexture1D::Map1D(int level, TextureMapAccess tma, uint32_t x_offset, uint32_t width, void*& data)
	{
		last_tma_ = tma;
		last_x_offset_ = x_offset;
		last_width_ = width;

		uint32_t const size_fmt = NumFormatBytes(format_);
		GLsizei image_size;
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

			image_size = ((width + 3) / 4) * block_size;
		}
		else
		{
			image_size = width * size_fmt;
		}

		switch (tma)
		{
		case TMA_Read_Only:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				//glBufferData(GL_PIXEL_PACK_BUFFER, image_size, NULL, GL_STREAM_READ);

				glBindTexture(GL_TEXTURE_1D, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(GL_TEXTURE_1D, level, NULL);
				}
				else
				{
					glGetTexImage(GL_TEXTURE_1D, level, gl_format, gl_type, NULL);
				}

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			}
			break;

		case TMA_Write_Only:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
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

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				//glBufferData(GL_PIXEL_PACK_BUFFER, image_size, NULL, GL_STREAM_READ);

				glBindTexture(GL_TEXTURE_1D, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(GL_TEXTURE_1D, level, NULL);
				}
				else
				{
					glGetTexImage(GL_TEXTURE_1D, level, gl_format, gl_type, NULL);
				}

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

		widthes_.resize(numMipMaps_);

		glBindTexture(GL_TEXTURE_1D, texture_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glGetTexLevelParameteriv(GL_TEXTURE_1D, level, GL_TEXTURE_WIDTH, &w);
			widthes_[level] = w;
		}
	}
}
