// OGLTexture2D.cpp
// KlayGE OpenGL 2D纹理类 实现文件
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
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "OpenGL32.lib")
#endif

namespace KlayGE
{
	OGLTexture2D::OGLTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
					: OGLTexture(TT_2D, array_size, sample_count, sample_quality, access_hint)
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
			uint32_t h = height;
			while ((w != 1) || (h != 1))
			{
				++ num_mip_maps_;

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		widths_.resize(num_mip_maps_);
		heights_.resize(num_mip_maps_);
		{
			uint32_t w = width;
			uint32_t h = height;
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				widths_[level] = w;
				heights_[level] = h;

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
			}
		}

		uint32_t texel_size = NumFormatBytes(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		if (glloader_GL_ARB_pixel_buffer_object())
		{
			pbos_.resize(array_size * num_mip_maps_);
			glGenBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);
		}
		else
		{
			tex_data_.resize(array_size * num_mip_maps_);
		}

		if (sample_count <= 1)
		{
			glGenTextures(1, &texture_);
			glBindTexture(target_type_, texture_);
			glTexParameteri(target_type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(target_type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
			{
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					uint32_t const w = widths_[level];
					uint32_t const h = heights_[level];

					if (!pbos_.empty())
					{
						re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbos_[array_index * num_mip_maps_ + level]);
					}
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

						GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * block_size;

						if (!pbos_.empty())
						{
							glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, image_size, nullptr, GL_STREAM_DRAW);
							re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
						}
						else
						{
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size);
						}

						if (array_size > 1)
						{
							if (0 == array_index)
							{
								glCompressedTexImage3D(target_type_, level, glinternalFormat,
									w, h, array_size, 0, image_size, nullptr);
							}
					
							glCompressedTexSubImage3D(target_type_, level, 0, 0, array_index, w, h, 1,
								glformat, gltype, (nullptr == init_data) ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
						}
						else
						{
							glCompressedTexImage2D(target_type_, level, glinternalFormat,
								w, h, 0, image_size, (nullptr == init_data) ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
						}
					}
					else
					{
						GLsizei const image_size = w * h * texel_size;

						if (!pbos_.empty())
						{
							glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, image_size, nullptr, GL_STREAM_DRAW);
							re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
						}
						else
						{
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size);
						}

						if (array_size > 1)
						{
							if (0 == array_index)
							{
								glTexImage3D(target_type_, level, glinternalFormat, w, h, array_size, 0, glformat, gltype, nullptr);
							}

							glTexSubImage3D(target_type_, level, 0, 0, array_index, w, h, 1,
								glformat, gltype, (nullptr == init_data) ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
						}
						else
						{
							glTexImage2D(target_type_, level, glinternalFormat, w, h, 0, glformat, gltype,
								(nullptr == init_data) ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
						}
					}
				}
			}
		}
		else
		{
			glGenRenderbuffersEXT(1, &texture_);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, texture_);
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, sample_count, glinternalFormat, width, height);
		}
	}

	uint32_t OGLTexture2D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return widths_[level];
	}

	uint32_t OGLTexture2D::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return heights_[level];
	}

	void OGLTexture2D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
		{
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				this->CopyToSubTexture2D(target,
					array_index, level, 0, 0, target.Width(level), target.Height(level),
					array_index, level, 0, 0, this->Width(level), this->Height(level));
			}
		}
	}

	void OGLTexture2D::CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(type_ == target.Type());

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			
		if (!re.HackForATI() && !re.HackForIntel() && ((format_ == target.Format()) && !IsCompressedFormat(format_) && glloader_GL_NV_copy_image()
			&& (src_width == dst_width) && (src_height == dst_height) && (1 == sample_count_)))
		{
			OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
			glCopyImageSubDataNV(
				texture_, target_type_, src_level,
				src_x_offset, src_y_offset, src_array_index,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, dst_y_offset, dst_array_index, src_width, src_height, 1);
		}
		else
		{
			if (((sample_count_ > 1) || (!re.HackForATI() && !re.HackForIntel())) && !IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))) && glloader_GL_EXT_framebuffer_blit())
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
					if (sample_count_ <= 1)
					{
						glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target_type_, texture_, src_level);
					}
					else
					{
						glFramebufferRenderbufferEXT(GL_READ_FRAMEBUFFER_EXT,
											GL_COLOR_ATTACHMENT0_EXT,
											GL_RENDERBUFFER_EXT, texture_);
					}
				}

				OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
				glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo_dst);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayerEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, ogl_target.GLTexture(), dst_level, dst_array_index);
				}
				else
				{
					glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, ogl_target.GLType(), ogl_target.GLTexture(), dst_level);
				}

				glBlitFramebufferEXT(src_x_offset, src_y_offset, src_x_offset + src_width, src_y_offset + src_height,
								dst_x_offset, dst_y_offset, dst_x_offset + dst_width, dst_y_offset + dst_height,
								GL_COLOR_BUFFER_BIT, ((src_width == dst_width) && (src_height == dst_height)) ? GL_NEAREST : GL_LINEAR);

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

				if (IsCompressedFormat(format_))
				{
					BOOST_ASSERT((src_width == dst_width) && (src_height == dst_height));
					BOOST_ASSERT((0 == (src_x_offset & 0x3)) && (0 == (src_y_offset & 0x3)));
					BOOST_ASSERT((0 == (dst_x_offset & 0x3)) && (0 == (dst_y_offset & 0x3)));
					BOOST_ASSERT((0 == (src_width & 0x3)) && (0 == (src_height & 0x3)));
					BOOST_ASSERT((0 == (dst_width & 0x3)) && (0 == (dst_height & 0x3)));

					Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_y_offset, src_width, src_height);
					Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_y_offset, dst_width, dst_height);

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

					uint8_t const * s = mapper_src.Pointer<uint8_t>();
					uint8_t* d = mapper_dst.Pointer<uint8_t>();
					for (uint32_t y = 0; y < src_height; y += 4)
					{
						std::memcpy(d, s, src_width / 4 * block_size);

						s += mapper_src.RowPitch();
						d += mapper_dst.RowPitch();
					}
				}
				else
				{
					if ((src_width == dst_width) && (src_height == dst_height))
					{
						size_t const format_size = NumFormatBytes(format_);

						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_y_offset, src_width, src_height);
						Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_y_offset, dst_width, dst_height);
						uint8_t const * s = mapper_src.Pointer<uint8_t>();
						uint8_t* d = mapper_dst.Pointer<uint8_t>();
						for (uint32_t y = 0; y < src_height; ++ y)
						{
							std::memcpy(d, s, src_width * format_size);

							s += mapper_src.RowPitch();
							d += mapper_dst.RowPitch();
						}
					}
					else
					{
						this->ResizeTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
							src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
					}
				}
			}
		}
	}

	void OGLTexture2D::CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		UNREF_PARAM(src_face);
		BOOST_ASSERT(TT_Cube == target.Type());

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		if (!re.HackForATI() && !re.HackForIntel() && ((format_ == target.Format()) && !IsCompressedFormat(format_) && glloader_GL_NV_copy_image()
			&& (src_width == dst_width) && (src_height == dst_height) && (1 == sample_count_)))
		{
			OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
			glCopyImageSubDataNV(
				texture_, target_type_, src_level,
				src_x_offset, src_y_offset, src_array_index,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, dst_y_offset, dst_array_index * 6 + dst_face - CF_Positive_X, src_width, src_height, 1);
		}
		else
		{
			if (((sample_count_ > 1) || (!re.HackForATI() && !re.HackForIntel())) && !IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))) && glloader_GL_EXT_framebuffer_blit())
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
					if (sample_count_ <= 1)
					{
						glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target_type_, texture_, src_level);
					}
					else
					{
						glFramebufferRenderbufferEXT(GL_READ_FRAMEBUFFER_EXT,
											GL_COLOR_ATTACHMENT0_EXT,
											GL_RENDERBUFFER_EXT, texture_);
					}
				}

				glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo_dst);
				if (target.ArraySize() > 1)
				{
					glFramebufferTextureLayerEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, texture_, dst_level, dst_array_index * 6 + dst_face - CF_Positive_X);
				}
				else
				{
					glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + dst_face, checked_cast<OGLTexture*>(&target)->GLTexture(), dst_level);
				}

				glBlitFramebufferEXT(src_x_offset, src_y_offset, src_x_offset + src_width, src_y_offset + src_height,
								dst_x_offset, dst_y_offset, dst_x_offset + dst_width, dst_y_offset + dst_height,
								GL_COLOR_BUFFER_BIT, ((src_width == dst_width) && (src_height == dst_height)) ? GL_NEAREST : GL_LINEAR);

				re.BindFramebuffer(old_fbo, true);
			}
			else
			{
				if ((src_width == dst_width) && (src_height == dst_height) && (format_ == target.Format()))
				{
					if (IsCompressedFormat(format_))
					{
						BOOST_ASSERT((0 == (src_x_offset & 0x3)) && (0 == (src_y_offset & 0x3)));
						BOOST_ASSERT((0 == (dst_x_offset & 0x3)) && (0 == (dst_y_offset & 0x3)));
						BOOST_ASSERT((0 == (src_width & 0x3)) && (0 == (src_height & 0x3)));
						BOOST_ASSERT((0 == (dst_width & 0x3)) && (0 == (dst_height & 0x3)));

						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_y_offset, src_width, src_height);
						Texture::Mapper mapper_dst(target, dst_array_index, dst_face, dst_level, TMA_Write_Only, dst_x_offset, dst_y_offset, dst_width, dst_height);

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

						uint8_t const * s = mapper_src.Pointer<uint8_t>();
						uint8_t* d = mapper_dst.Pointer<uint8_t>();
						for (uint32_t y = 0; y < src_height; y += 4)
						{
							std::memcpy(d, s, src_width / 4 * block_size);

							s += mapper_src.RowPitch();
							d += mapper_dst.RowPitch();
						}
					}
					else
					{
						size_t const format_size = NumFormatBytes(format_);

						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_y_offset, src_width, src_height);
						Texture::Mapper mapper_dst(target, dst_array_index, dst_face, dst_level, TMA_Write_Only, dst_x_offset, dst_y_offset, dst_width, dst_height);
						uint8_t const * s = mapper_src.Pointer<uint8_t>();
						uint8_t* d = mapper_dst.Pointer<uint8_t>();
						for (uint32_t y = 0; y < src_height; ++ y)
						{
							std::memcpy(d, s, src_width * format_size);

							s += mapper_src.RowPitch();
							d += mapper_dst.RowPitch();
						}
					}
				}
				else
				{
					this->ResizeTextureCube(target, dst_array_index, dst_face, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
						src_array_index, CF_Positive_X, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
				}
			}
		}
	}

	void OGLTexture2D::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
					uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
					void*& data, uint32_t& row_pitch)
	{
		UNREF_PARAM(width);
		UNREF_PARAM(height);

		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);
		int block_size;
		if (IsCompressedFormat(format_))
		{
			if ((EF_BC1 == format_) || (EF_SIGNED_BC1 == format_) || (EF_BC1_SRGB == format_)
				|| (EF_BC4 == format_) || (EF_SIGNED_BC4 == format_) || (EF_BC4_SRGB == format_))
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}
		}
		else
		{
			block_size = 0;
		}

		row_pitch = widths_[level] * texel_size;

		uint8_t* p;
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (tma)
		{
		case TMA_Read_Only:
		case TMA_Read_Write:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				if (!pbos_.empty())
				{
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
					re.BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbos_[array_index * num_mip_maps_ + level]);

					glBindTexture(target_type_, texture_);
					if (IsCompressedFormat(format_))
					{
						glGetCompressedTexImage(target_type_, level, nullptr);
						p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY));
					}
					else
					{
						glGetTexImage(target_type_, level, gl_format, gl_type, nullptr);
						p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY));
					}
				}
				else
				{
					glBindTexture(target_type_, texture_);
					if (IsCompressedFormat(format_))
					{
						glGetCompressedTexImage(target_type_, level, &tex_data_[array_index * num_mip_maps_ + level][0]);
						p = &tex_data_[array_index * num_mip_maps_ + level][0];
					}
					else
					{
						glGetTexImage(target_type_, level, gl_format, gl_type, &tex_data_[array_index * num_mip_maps_ + level][0]);
						p = &tex_data_[array_index * num_mip_maps_ + level][0];
					}
				}
			}
			break;

		case TMA_Write_Only:
			{
				if (!pbos_.empty())
				{
					re.BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbos_[array_index * num_mip_maps_ + level]);
					p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY));
				}
				else
				{
					p = &tex_data_[array_index * num_mip_maps_ + level][0];
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			p = nullptr;
			break;
		}

		if (IsCompressedFormat(format_))
		{
			data = p + (y_offset / 4) * row_pitch + (x_offset / 4 * block_size);
		}
		else
		{
			data = p + (y_offset * widths_[level] + x_offset) * texel_size;
		}
	}

	void OGLTexture2D::Unmap2D(uint32_t array_index, uint32_t level)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (last_tma_)
		{
		case TMA_Read_Only:
			if (!pbos_.empty())
			{
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
				re.BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbos_[array_index * num_mip_maps_ + level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
			}
			break;

		case TMA_Write_Only:
		case TMA_Read_Write:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				GLsizei image_size = 0;
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

					image_size = ((widths_[level] + 3) / 4) * ((heights_[level] + 3) / 4) * block_size;
				}

				glBindTexture(target_type_, texture_);

				uint8_t* p;
				if (!pbos_.empty())
				{
					re.BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbos_[array_index * num_mip_maps_ + level]);
					glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);

					p = nullptr;
				}
				else
				{
					p = &tex_data_[array_index * num_mip_maps_ + level][0];
				}

				if (IsCompressedFormat(format_))
				{
					if (array_size_ > 1)
					{
						glCompressedTexSubImage3D(target_type_, level, 0, 0, array_index,
							widths_[level], heights_[level], 1, gl_format, image_size, p);
					}
					else
					{
						glCompressedTexSubImage2D(target_type_, level, 0, 0,
							widths_[level], heights_[level], gl_format, image_size, p);
					}
				}
				else
				{
					if (array_size_ > 1)
					{
						glTexSubImage3D(target_type_, level, 0, 0, array_index, widths_[level], heights_[level], 1,
							gl_format, gl_type, p);
					}
					else
					{
						glTexSubImage2D(target_type_, level, 0, 0, widths_[level], heights_[level],
							gl_format, gl_type, p);
					}
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
}
