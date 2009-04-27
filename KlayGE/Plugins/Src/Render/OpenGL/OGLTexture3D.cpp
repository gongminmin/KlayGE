// OGLTexture3D.hpp
// KlayGE OpenGL 3D纹理类 实现文件
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
	OGLTexture3D::OGLTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: OGLTexture(TT_3D, sample_count, sample_quality, access_hint)
	{
		if (!glloader_GL_EXT_texture_sRGB())
		{
			format = this->SRGBToRGB(format);
		}

		format_ = format;

		if (0 == numMipMaps)
		{
			uint32_t w = width;
			uint32_t h = height;
			uint32_t d = depth;
			while ((w > 1) && (h > 1) && (d > 1))
			{
				++ numMipMaps_;

				w = std::max(static_cast<uint32_t>(1), w / 2);
				h = std::max(static_cast<uint32_t>(1), h / 2);
				d = std::max(static_cast<uint32_t>(1), d / 2);
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
		glBindTexture(GL_TEXTURE_3D, texture_);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

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

				GLsizei const image_size = ((width + 3) / 4) * ((height + 3) / 4) * depth * block_size;

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

				glCompressedTexImage3D(GL_TEXTURE_3D, level, glinternalFormat,
					width, height, depth, 0, image_size, NULL);
			}
			else
			{
				GLsizei const image_size = width * height * depth * bpp_ / 8;

				glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
				uint8_t* p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
				if (NULL == init_data)
				{
					memset(p, 0, image_size);
				}
				else
				{
					for (uint32_t d = 0; d < depth; ++ d)
					{
						for (uint32_t h = 0; h < height; ++ h)
						{
							memcpy(p + (d * height + h) * width * bpp_ / 8,
								static_cast<uint8_t const *>(init_data[level].data) + d * init_data[level].slice_pitch + h * init_data[level].row_pitch, width * bpp_ / 8);
						}
					}
				}
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				glTexImage3D(GL_TEXTURE_3D, level, glinternalFormat, width, height, depth, 0, glformat, gltype, NULL);
			}

			width = std::max(static_cast<uint32_t>(1), width / 2);
			height = std::max(static_cast<uint32_t>(1), height / 2);
			depth = std::max(static_cast<uint32_t>(1), depth / 2);
		}

		this->UpdateParams();
	}

	uint32_t OGLTexture3D::Width(int level) const
	{
		return widthes_[level];
	}

	uint32_t OGLTexture3D::Height(int level) const
	{
		return heights_[level];
	}

	uint32_t OGLTexture3D::Depth(int level) const
	{
		return depthes_[level];
	}

	void OGLTexture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && (widthes_[0] == target.Width(0)) && (heights_[0] == target.Height(0)) && (depthes_[0] == target.Depth(0)))
		{
			GLint gl_internalFormat;
			GLenum gl_format;
			GLenum gl_type;
			OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

			for (int level = 0; level < numMipMaps_; ++ level)
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);

				glBindTexture(GL_TEXTURE_3D, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(GL_TEXTURE_3D, level, NULL);
				}
				else
				{
					glGetTexImage(GL_TEXTURE_3D, level, gl_format, gl_type, NULL);
				}

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
				glBindTexture(GL_TEXTURE_3D, checked_cast<OGLTexture*>(&target)->GLTexture());

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

					GLsizei const image_size = ((this->Width(level) + 3) / 4) * ((this->Height(level) + 3) / 4) * this->Depth(level) * block_size;

					glCompressedTexSubImage3D(GL_TEXTURE_3D, level, 0, 0, 0,
						this->Width(level), this->Height(level), this->Depth(level), gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_3D, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level),
							gl_format, gl_type, NULL);
				}
			}
		}
		else
		{
			for (int level = 0; level < numMipMaps_; ++ level)
			{
				this->CopyToTexture3D(target, level, target.Width(level), target.Height(level), target.Depth(level), 0, 0, 0,
					this->Width(level), this->Height(level), this->Depth(level), 0, 0, 0);
			}
		}
	}

	void OGLTexture3D::CopyToTexture3D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			uint32_t src_xOffset, uint32_t src_yOffset, uint32_t src_zOffset)
	{
		UNREF_PARAM(dst_depth);

		BOOST_ASSERT(type_ == target.Type());
		// fix me
		BOOST_ASSERT(src_depth == dst_depth);

		if (!IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))) && glloader_GL_EXT_framebuffer_blit())
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			GLuint fbo_src, fbo_dst;
			re.GetFBOForBlit(fbo_src, fbo_dst);

			GLuint old_fbo = re.BindFramebuffer();

			for (uint32_t depth = 0; depth < src_depth; ++ depth)
			{
				glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo_src);
				glFramebufferTexture3DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D, texture_, level, src_zOffset + depth);

				glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo_dst);
				glFramebufferTexture3DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D, checked_cast<OGLTexture*>(&target)->GLTexture(), level, dst_zOffset + depth);

				glBlitFramebufferEXT(src_xOffset, src_yOffset, src_xOffset + src_width, src_yOffset + src_height,
								dst_xOffset, dst_yOffset, dst_xOffset + dst_width, dst_yOffset + dst_height,
								GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			re.BindFramebuffer(old_fbo, true);
		}
		else
		{
			BOOST_ASSERT(format_ == target.Format());

			GLint gl_internalFormat;
			GLenum gl_format;
			GLenum gl_type;
			OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

			GLint gl_target_internal_format;
			GLenum gl_target_format;
			GLenum gl_target_type;
			OGLMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

			size_t const src_format_size = NumFormatBytes(format_);
			size_t const dst_format_size = NumFormatBytes(target.Format());

			std::vector<uint8_t> data_in(src_width * src_height * src_format_size);
			std::vector<uint8_t> data_out(dst_width * dst_height * dst_format_size);

			for (uint32_t z = 0; z < src_depth; ++ z)
			{
				{
					Texture::Mapper mapper(*this, level, TMA_Read_Only, src_xOffset, src_yOffset, src_zOffset + z,
						src_width, src_height, 1);
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
					Texture::Mapper mapper(target, level, TMA_Write_Only, dst_xOffset, dst_yOffset, dst_zOffset + z,
						dst_width, dst_height, 1);
					uint8_t const * s = &data_out[0];
					uint8_t* d = mapper.Pointer<uint8_t>();
					for (uint32_t y = 0; y < src_height; ++ y)
					{
						memcpy(d, s, dst_width * dst_format_size);

						s += src_width * src_format_size;
						d += mapper.RowPitch();
					}
				}
			}
		}
	}

	void OGLTexture3D::Map3D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		last_tma_ = tma;
		last_x_offset_ = x_offset;
		last_y_offset_ = y_offset;
		last_z_offset_ = z_offset;
		last_width_ = width;
		last_height_ = height;
		last_depth_ = depth;

		uint32_t const size_fmt = NumFormatBytes(format_);

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
				//glBufferData(GL_PIXEL_PACK_BUFFER, width * height * depth * size_fmt, NULL, GL_STREAM_READ);

				glBindTexture(GL_TEXTURE_3D, texture_);
				glGetTexImage(GL_TEXTURE_3D, level, gl_format, gl_type, NULL);

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
				//glBufferData(GL_PIXEL_PACK_BUFFER, width * height * depth * size_fmt, NULL, GL_STREAM_READ);

				glBindTexture(GL_TEXTURE_3D, texture_);
				glGetTexImage(GL_TEXTURE_3D, level, gl_format, gl_type, NULL);

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		row_pitch = width * size_fmt;
		slice_pitch = row_pitch * height;
	}

	void OGLTexture3D::Unmap3D(int level)
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

				glBindTexture(GL_TEXTURE_3D, texture_);

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

					GLsizei const image_size = ((last_width_ + 3) / 4) * ((last_height_ + 3) / 4) * last_depth_ * block_size;

					glCompressedTexSubImage3D(GL_TEXTURE_3D, level, last_x_offset_, last_y_offset_, last_z_offset_,
							last_width_, last_height_, last_depth_, gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_3D, level,
							last_x_offset_, last_y_offset_, last_z_offset_, last_width_, last_height_, last_depth_,
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

				glBindTexture(GL_TEXTURE_3D, texture_);

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

					GLsizei const image_size = ((last_width_ + 3) / 4) * ((last_height_ + 3) / 4) * last_depth_ * block_size;

					glCompressedTexSubImage3D(GL_TEXTURE_3D, level, last_x_offset_, last_y_offset_, last_z_offset_,
							last_width_, last_height_, last_depth_, gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage3D(GL_TEXTURE_3D, level, last_x_offset_, last_y_offset_, last_z_offset_,
							last_width_, last_height_, last_depth_,
							gl_format, gl_type, NULL);
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void OGLTexture3D::UpdateParams()
	{
		GLint w, h, d;

		widthes_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		depthes_.resize(numMipMaps_);

		glBindTexture(GL_TEXTURE_3D, texture_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glGetTexLevelParameteriv(GL_TEXTURE_3D, level, GL_TEXTURE_WIDTH, &w);
			widthes_[level] = w;

			glGetTexLevelParameteriv(GL_TEXTURE_3D, level, GL_TEXTURE_HEIGHT, &h);
			heights_[level] = h;

			glGetTexLevelParameteriv(GL_TEXTURE_3D, level, GL_TEXTURE_DEPTH, &d);
			depthes_[level] = d;
		}
	}
}
