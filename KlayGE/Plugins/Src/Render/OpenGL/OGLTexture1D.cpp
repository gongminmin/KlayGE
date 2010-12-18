// OGLTexture1D.cpp
// KlayGE OpenGL 1D纹理类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 支持GL_NV_copy_image (2009.8.5)
//
// 3.8.0
// 通过GL_EXT_framebuffer_blit加速CopyTexture (2008.10.12)
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
	OGLTexture1D::OGLTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: OGLTexture(TT_1D, array_size, sample_count, sample_quality, access_hint)
	{
		if (!glloader_GL_EXT_texture_sRGB())
		{
			format = this->SRGBToRGB(format);
		}

		format_ = format;

		if (0 == numMipMaps)
		{
			num_mip_maps_ = 1;
			uint32_t w = width;
			while (w > 1)
			{
				++ num_mip_maps_;

				w = std::max(1U, w / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		uint32_t texel_size = NumFormatBytes(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		pbos_.resize(array_size * num_mip_maps_);
		glGenBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);

		glGenTextures(1, &texture_);
		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target_type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

		for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
		{
			uint32_t w = width;
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
				if (IsCompressedFormat(format_))
				{
					int block_size;
					if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
						|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					GLsizei const image_size = ((w + 3) / 4) * block_size;

					glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

					if (array_size > 1)
					{
						if (0 == array_index)
						{
							glCompressedTexImage2D(target_type_, level, glinternalFormat,
								w, array_size, 0, image_size, NULL);
						}

						glCompressedTexSubImage2D(target_type_, level, 0, array_index, w, 1,
							glformat, gltype, (NULL == init_data) ? NULL : init_data[array_index * num_mip_maps_ + level].data);
					}
					else
					{
						glCompressedTexImage1D(target_type_, level, glinternalFormat,
							w, 0, image_size, (NULL == init_data) ? NULL : init_data[array_index * num_mip_maps_ + level].data);
					}
				}
				else
				{
					GLsizei const image_size = w * texel_size;

					glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
					glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

					if (array_size > 1)
					{
						if (0 == array_index)
						{
							glTexImage2D(target_type_, level, glinternalFormat, w, array_size, 0, glformat, gltype, NULL);
						}

						glTexSubImage2D(target_type_, level, 0, array_index, w, 1,
							glformat, gltype, (NULL == init_data) ? NULL : init_data[array_index * num_mip_maps_ + level].data);
					}
					else
					{
						glTexImage1D(target_type_, level, glinternalFormat, w, 0, glformat, gltype,
							(NULL == init_data) ? NULL : init_data[array_index * num_mip_maps_ + level].data);
					}
				}

				width = std::max(1U, width / 2);
			}
		}

		this->UpdateParams();
	}

	uint32_t OGLTexture1D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return widthes_[level];
	}

	void OGLTexture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && (widthes_[0] == target.Width(0)))
		{
			if (glloader_GL_NV_copy_image())
			{
				OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					glCopyImageSubDataNV(
						texture_, target_type_, level,
						0, 0, 0,
						ogl_target.GLTexture(), ogl_target.GLType(), level,
						0, 0, 0, widthes_[level], 1, 1);
				}
			}
			else
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
				{
					for (uint32_t level = 0; level < num_mip_maps_; ++ level)
					{
						glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);

						glBindTexture(target_type_, texture_);
						if (IsCompressedFormat(format_))
						{
							glGetCompressedTexImage(target_type_, level, NULL);
						}
						else
						{
							glGetTexImage(target_type_, level, gl_format, gl_type, NULL);
						}

						glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
						glBindTexture(target_type_, checked_cast<OGLTexture*>(&target)->GLTexture());

						if (IsCompressedFormat(format_))
						{
							int block_size;
							if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
								|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
							{
								block_size = 8;
							}
							else
							{
								block_size = 16;
							}

							GLsizei const image_size = ((this->Width(level) + 3) / 4) * block_size;

							if (array_size_ > 1)
							{
								glCompressedTexSubImage2D(target_type_, level, 0, array_index,
									this->Width(level), 1, gl_format, image_size, NULL);
							}
							else
							{
								glCompressedTexSubImage1D(target_type_, level, 0,
									this->Width(level), gl_format, image_size, NULL);
							}
						}
						else
						{
							if (array_size_ > 1)
							{
								glTexSubImage2D(target_type_, level, 0, array_index, this->Width(level), 1,
									gl_format, gl_type, NULL);
							}
							else
							{
								glTexSubImage1D(target_type_, level, 0, this->Width(level), gl_format, gl_type, NULL);
							}
						}
					}
				}
			}
		}
		else
		{
			for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
			{
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					this->CopyToSubTexture1D(target,
						array_index, level, 0, target.Width(level),
						array_index, level, 0, this->Width(level));
				}
			}
		}
	}

	void OGLTexture1D::CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		BOOST_ASSERT(type_ == target.Type());
		
		if ((format_ == target.Format()) && glloader_GL_NV_copy_image() && (src_width == dst_width))
		{
			OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
			glCopyImageSubDataNV(
				texture_, target_type_, src_level,
				src_x_offset, 0, src_array_index,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, 0, dst_array_index, src_width, 1, 1);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (!re.HackForATI() && !IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))) && glloader_GL_EXT_framebuffer_blit())
			{
				GLuint fbo_src, fbo_dst;
				re.GetFBOForBlit(fbo_src, fbo_dst);

				GLuint old_fbo = re.BindFramebuffer();

				glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo_src);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayerEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, texture_, src_level, src_array_index);
				}
				else
				{
					glFramebufferTexture1DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target_type_, texture_, src_level);
				}

				glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo_dst);
				if (target.ArraySize() > 1)
				{
					glFramebufferTextureLayerEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, checked_cast<OGLTexture*>(&target)->GLTexture(), dst_level, dst_array_index);
				}
				else
				{
					glFramebufferTexture1DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target_type_, checked_cast<OGLTexture*>(&target)->GLTexture(), dst_level);
				}

				glBlitFramebufferEXT(src_x_offset, 0, src_x_offset + src_width, 1,
								dst_x_offset, 0, dst_x_offset + dst_width, 1,
								GL_COLOR_BUFFER_BIT, (src_width == dst_width) ? GL_NEAREST : GL_LINEAR);

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
					BOOST_ASSERT((0 == src_x_offset) && (0 == dst_x_offset));
					BOOST_ASSERT((src_width == dst_width));

					Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_width);
					Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_width);

					int block_size;
					if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
						|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
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
							Texture::Mapper mapper(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_width);
							memcpy(&data_in[0], mapper.Pointer<uint8_t*>(), data_in.size() * sizeof(data_in[0]));
						}

						gluScaleImage(gl_format, src_width, 1, gl_type, &data_in[0],
							dst_width, 1, gl_target_type, &data_out[0]);

						{
							Texture::Mapper mapper(other, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_width);
							memcpy(mapper.Pointer<uint8_t*>(), &data_out[0], data_out.size() * sizeof(data_out[0]));
						}
					}
					else
					{
						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_width);
						Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_width);
						uint8_t const * s = mapper_src.Pointer<uint8_t>();
						uint8_t* d = mapper_dst.Pointer<uint8_t>();
						
						memcpy(d, s, src_width * src_format_size);
					}
				}
			}
		}
	}

	void OGLTexture1D::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t width, void*& data)
	{
		last_tma_ = tma;
		last_x_offset_ = x_offset;
		last_width_ = width;

		uint32_t const size_fmt = NumFormatBytes(format_);
		GLsizei image_size;
		if (IsCompressedFormat(format_))
		{
			int block_size;
			if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
				|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
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
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
				//glBufferData(GL_PIXEL_PACK_BUFFER, image_size, NULL, GL_STREAM_READ);

				glBindTexture(target_type_, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(target_type_, level, NULL);
				}
				else
				{
					glGetTexImage(target_type_, level, gl_format, gl_type, NULL);
				}

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			}
			break;

		case TMA_Write_Only:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
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
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
				//glBufferData(GL_PIXEL_PACK_BUFFER, image_size, NULL, GL_STREAM_READ);

				glBindTexture(target_type_, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(target_type_, level, NULL);
				}
				else
				{
					glGetTexImage(target_type_, level, gl_format, gl_type, NULL);
				}

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void OGLTexture1D::Unmap1D(uint32_t array_index, uint32_t level)
	{
		switch (last_tma_)
		{
		case TMA_Read_Only:
			{
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}
			break;

		case TMA_Write_Only:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindTexture(target_type_, texture_);

				if (IsCompressedFormat(format_))
				{
					int block_size;
					if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
						|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					GLsizei const image_size = ((last_width_ + 3) / 4) * block_size;

					if (array_size_ > 1)
					{
						glCompressedTexSubImage2D(target_type_, level, last_x_offset_, array_index, last_width_, 1,
							gl_format, gl_type, NULL);
					}
					else
					{
						glCompressedTexSubImage1D(target_type_, level, last_x_offset_,
							last_width_, gl_format, image_size, NULL);
					}
				}
				else
				{
					if (array_size_ > 1)
					{
						glTexSubImage2D(target_type_, level, last_x_offset_, array_index, last_width_, 1,
							gl_format, gl_type, NULL);
					}
					else
					{
						glTexSubImage1D(target_type_, level, last_x_offset_, last_width_,
							gl_format, gl_type, NULL);
					}
				}
			}
			break;

		case TMA_Read_Write:
			{
				glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[array_index * num_mip_maps_ + level]);

				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindTexture(target_type_, texture_);

				if (IsCompressedFormat(format_))
				{
					int block_size;
					if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
						|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					GLsizei const image_size = ((last_width_ + 3) / 4) * block_size;

					if (array_size_ > 1)
					{
						glCompressedTexSubImage2D(target_type_, level, last_x_offset_, array_index, last_width_, 1,
							gl_format, gl_type, NULL);
					}
					else
					{
						glCompressedTexSubImage1D(target_type_, level, last_x_offset_,
							last_width_, gl_format, image_size, NULL);
					}
				}
				else
				{
					if (array_size_ > 1)
					{
						glTexSubImage2D(target_type_, level, last_x_offset_, array_index, last_width_, 1,
							gl_format, gl_type, NULL);
					}
					else
					{
						glTexSubImage1D(target_type_, level, last_x_offset_, last_width_,
							gl_format, gl_type, NULL);
					}
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

		widthes_.resize(num_mip_maps_);

		glBindTexture(target_type_, texture_);
		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			glGetTexLevelParameteriv(target_type_, level, GL_TEXTURE_WIDTH, &w);
			widthes_[level] = w;
		}
	}
}
