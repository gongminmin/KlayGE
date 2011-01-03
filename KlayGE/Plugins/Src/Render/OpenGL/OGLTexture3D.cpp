// OGLTexture3D.cpp
// KlayGE OpenGL 3D纹理类 实现文件
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
	OGLTexture3D::OGLTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: OGLTexture(TT_3D, array_size, sample_count, sample_quality, access_hint)
	{
		if (!glloader_GL_EXT_texture_sRGB())
		{
			format = this->SRGBToRGB(format);
		}
		BOOST_ASSERT(1 == array_size);

		format_ = format;

		if (0 == numMipMaps)
		{
			num_mip_maps_ = 1;
			uint32_t w = width;
			uint32_t h = height;
			uint32_t d = depth;
			while ((w > 1) && (h > 1) && (d > 1))
			{
				++ num_mip_maps_;

				w = std::max(1U, w / 2);
				h = std::max(1U, h / 2);
				d = std::max(1U, d / 2);
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

		pbos_.resize(num_mip_maps_);
		glGenBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);

		glGenTextures(1, &texture_);
		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target_type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
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

				GLsizei const image_size = ((width + 3) / 4) * ((height + 3) / 4) * depth * block_size;

				glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

				glCompressedTexImage3D(target_type_, level, glinternalFormat,
					width, height, depth, 0, image_size, (NULL == init_data) ? NULL : init_data[level].data);
			}
			else
			{
				GLsizei const image_size = width * height * depth * texel_size;

				glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, NULL, GL_STREAM_DRAW);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

				glTexImage3D(target_type_, level, glinternalFormat, width, height, depth, 0, glformat, gltype,
					(NULL == init_data) ? NULL : init_data[level].data);
			}

			width = std::max(1U, width / 2);
			height = std::max(1U, height / 2);
			depth = std::max(1U, depth / 2);
		}

		this->UpdateParams();
	}

	uint32_t OGLTexture3D::Width(uint32_t level) const
	{
		return widthes_[level];
	}

	uint32_t OGLTexture3D::Height(uint32_t level) const
	{
		return heights_[level];
	}

	uint32_t OGLTexture3D::Depth(uint32_t level) const
	{
		return depthes_[level];
	}

	void OGLTexture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && (widthes_[0] == target.Width(0)) && (heights_[0] == target.Height(0)) && (depthes_[0] == target.Depth(0)))
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
						0, 0, 0, widthes_[level], heights_[level], depthes_[level]);
				}
			}
			else
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);

					glBindTexture(target_type_, texture_);
					if (IsCompressedFormat(format_))
					{
						glGetCompressedTexImage(target_type_, level, NULL);
					}
					else
					{
						glGetTexImage(target_type_, level, gl_format, gl_type, NULL);
					}

					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
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

						GLsizei const image_size = ((this->Width(level) + 3) / 4) * ((this->Height(level) + 3) / 4) * this->Depth(level) * block_size;

						glCompressedTexSubImage3D(target_type_, level, 0, 0, 0,
							this->Width(level), this->Height(level), this->Depth(level), gl_format, image_size, NULL);
					}
					else
					{
						glTexSubImage3D(target_type_, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level),
								gl_format, gl_type, NULL);
					}
				}
			}
		}
		else
		{
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				this->CopyToSubTexture3D(target,
					0, level, 0, 0, 0, target.Width(level), target.Height(level), target.Depth(level),
					0, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level));
			}
		}
	}

	void OGLTexture3D::CopyToSubTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		UNREF_PARAM(dst_depth);

		BOOST_ASSERT(type_ == target.Type());
		BOOST_ASSERT(0 == src_array_index);
		BOOST_ASSERT(0 == dst_array_index);
		// fix me
		BOOST_ASSERT(src_depth == dst_depth);

		if ((format_ == target.Format()) && glloader_GL_NV_copy_image() && (src_width == dst_width) && (src_height == dst_height) && (src_depth == dst_depth))
		{
			OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
			glCopyImageSubDataNV(
				texture_, target_type_, src_level,
				src_x_offset, src_y_offset, src_z_offset,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, dst_y_offset, dst_z_offset, src_width, src_height, src_depth);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (!re.HackForATI() && !IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))) && glloader_GL_EXT_framebuffer_blit())
			{
				GLuint fbo_src, fbo_dst;
				re.GetFBOForBlit(fbo_src, fbo_dst);

				GLuint old_fbo = re.BindFramebuffer();

				for (uint32_t depth = 0; depth < src_depth; ++ depth)
				{
					glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo_src);
					glFramebufferTexture3DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target_type_, texture_, src_level, src_z_offset + depth);

					glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, fbo_dst);
					glFramebufferTexture3DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target_type_, checked_cast<OGLTexture*>(&target)->GLTexture(), dst_level, dst_z_offset + depth);

					glBlitFramebufferEXT(src_x_offset, src_y_offset, src_x_offset + src_width, src_y_offset + src_height,
									dst_x_offset, dst_y_offset, dst_x_offset + dst_width, dst_y_offset + dst_height,
									GL_COLOR_BUFFER_BIT, ((src_width == dst_width) && (src_height == dst_height) && (src_depth == dst_depth)) ? GL_NEAREST : GL_LINEAR);
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
						Texture::Mapper mapper(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_y_offset, src_z_offset + z,
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
						Texture::Mapper mapper(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_y_offset, dst_z_offset + z,
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
	}

	void OGLTexture3D::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		BOOST_ASSERT(0 == array_index);
		UNREF_PARAM(array_index);

		last_tma_ = tma;
		last_x_offset_ = x_offset;
		last_y_offset_ = y_offset;
		last_z_offset_ = z_offset;
		last_width_ = width;
		last_height_ = height;
		last_depth_ = depth;

		uint32_t const size_fmt = NumFormatBytes(format_);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (tma)
		{
		case TMA_Read_Only:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				//glBufferData(GL_PIXEL_PACK_BUFFER, width * height * depth * size_fmt, NULL, GL_STREAM_READ);

				glBindTexture(target_type_, texture_);
				glGetTexImage(target_type_, level, gl_format, gl_type, NULL);

				data = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			}
			break;

		case TMA_Write_Only:
			{
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
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

				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				//glBufferData(GL_PIXEL_PACK_BUFFER, width * height * depth * size_fmt, NULL, GL_STREAM_READ);

				glBindTexture(target_type_, texture_);
				glGetTexImage(target_type_, level, gl_format, gl_type, NULL);

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

	void OGLTexture3D::Unmap3D(uint32_t array_index, uint32_t level)
	{
		BOOST_ASSERT(0 == array_index);
		UNREF_PARAM(array_index);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (last_tma_)
		{
		case TMA_Read_Only:
			{
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}
			break;

		case TMA_Write_Only:
			{
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);
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

					GLsizei const image_size = ((last_width_ + 3) / 4) * ((last_height_ + 3) / 4) * last_depth_ * block_size;

					glCompressedTexSubImage3D(target_type_, level, last_x_offset_, last_y_offset_, last_z_offset_,
							last_width_, last_height_, last_depth_, gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage3D(target_type_, level,
							last_x_offset_, last_y_offset_, last_z_offset_, last_width_, last_height_, last_depth_,
							gl_format, gl_type, NULL);
				}
			}
			break;

		case TMA_Read_Write:
			{
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[level]);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[level]);

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

					GLsizei const image_size = ((last_width_ + 3) / 4) * ((last_height_ + 3) / 4) * last_depth_ * block_size;

					glCompressedTexSubImage3D(target_type_, level, last_x_offset_, last_y_offset_, last_z_offset_,
							last_width_, last_height_, last_depth_, gl_format, image_size, NULL);
				}
				else
				{
					glTexSubImage3D(target_type_, level, last_x_offset_, last_y_offset_, last_z_offset_,
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

		widthes_.resize(num_mip_maps_);
		heights_.resize(num_mip_maps_);
		depthes_.resize(num_mip_maps_);

		glBindTexture(target_type_, texture_);
		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			glGetTexLevelParameteriv(target_type_, level, GL_TEXTURE_WIDTH, &w);
			widthes_[level] = w;

			glGetTexLevelParameteriv(target_type_, level, GL_TEXTURE_HEIGHT, &h);
			heights_[level] = h;

			glGetTexLevelParameteriv(target_type_, level, GL_TEXTURE_DEPTH, &d);
			depthes_[level] = d;
		}
	}
}
