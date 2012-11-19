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

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "OpenGL32.lib")
#endif

namespace KlayGE
{
	OGLTexture3D::OGLTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
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

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
				d = std::max<uint32_t>(1U, d / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		widths_.resize(num_mip_maps_);
		heights_.resize(num_mip_maps_);
		depthes_.resize(num_mip_maps_);
		{
			uint32_t w = width;
			uint32_t h = height;
			uint32_t d = depth;
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				widths_[level] = w;
				heights_[level] = h;
				depthes_[level] = d;

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
				d = std::max<uint32_t>(1U, d / 2);
			}
		}

		uint32_t texel_size = NumFormatBytes(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		if (glloader_GL_ARB_pixel_buffer_object())
		{
			pbos_.resize(num_mip_maps_);
			glGenBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);
		}
		else
		{
			tex_data_.resize(num_mip_maps_);
		}

		glGenTextures(1, &texture_);
		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target_type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			uint32_t const w = widths_[level];
			uint32_t const h = heights_[level];
			uint32_t const d = depthes_[level];

			if (!pbos_.empty())
			{
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbos_[level]);
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

				GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * d * block_size;

				if (!pbos_.empty())
				{
					glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, image_size, nullptr, GL_STREAM_DRAW);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
				}

				glCompressedTexImage3D(target_type_, level, glinternalFormat,
					w, h, d, 0, image_size, (nullptr == init_data) ? nullptr : init_data[level].data);
			}
			else
			{
				GLsizei const image_size = w * h * d * texel_size;

				if (!pbos_.empty())
				{
					glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, image_size, nullptr, GL_STREAM_DRAW);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
				}

				glTexImage3D(target_type_, level, glinternalFormat, w, h, d, 0, glformat, gltype,
					(nullptr == init_data) ? nullptr : init_data[level].data);
			}
		}
	}

	uint32_t OGLTexture3D::Width(uint32_t level) const
	{
		return widths_[level];
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

		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			this->CopyToSubTexture3D(target,
				0, level, 0, 0, 0, target.Width(level), target.Height(level), target.Depth(level),
				0, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level));
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

		if ((format_ == target.Format()) && !IsCompressedFormat(format_) && glloader_GL_NV_copy_image()
			&& (src_width == dst_width) && (src_height == dst_height) && (src_depth == dst_depth) && (1 == sample_count_))
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
				if ((src_width == dst_width) && (src_height == dst_height) && (src_depth == dst_depth) && (format_ == target.Format()))
				{
					if (IsCompressedFormat(format_))
					{
						BOOST_ASSERT((0 == (src_x_offset & 0x3)) && (0 == (src_y_offset & 0x3)));
						BOOST_ASSERT((0 == (dst_x_offset & 0x3)) && (0 == (dst_y_offset & 0x3)));
						BOOST_ASSERT((0 == (src_width & 0x3)) && (0 == (src_height & 0x3)));
						BOOST_ASSERT((0 == (dst_width & 0x3)) && (0 == (dst_height & 0x3)));

						for (uint32_t z = 0; z < src_depth; ++ z)
						{
							Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only,
								src_x_offset, src_y_offset, src_z_offset + z, src_width, src_height, 1);
							Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only,
								dst_x_offset, dst_y_offset, dst_z_offset + z, dst_width, dst_height, 1);

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
								memcpy(d, s, src_width / 4 * block_size);

								s += mapper_src.RowPitch();
								d += mapper_dst.RowPitch();
							}
						}
					}
					else
					{
						for (uint32_t z = 0; z < src_depth; ++ z)
						{
							size_t const format_size = NumFormatBytes(format_);

							Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only,
								src_x_offset, src_y_offset, src_z_offset + z, src_width, src_height, 1);
							Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only,
								dst_x_offset, dst_y_offset, dst_z_offset + z, dst_width, dst_height, 1);
							uint8_t const * s = mapper_src.Pointer<uint8_t>();
							uint8_t* d = mapper_dst.Pointer<uint8_t>();
							for (uint32_t y = 0; y < src_height; ++ y)
							{
								memcpy(d, s, src_width * format_size);

								s += mapper_src.RowPitch();
								d += mapper_dst.RowPitch();
							}
						}
					}
				}
				else
				{
					this->ResizeTexture3D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_z_offset, dst_width, dst_height, dst_depth,
						src_array_index, src_level, src_x_offset, src_y_offset, src_z_offset, src_width, src_height, src_depth, true);
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
		UNREF_PARAM(width);
		UNREF_PARAM(height);
		UNREF_PARAM(depth);

		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);

		row_pitch = widths_[level] * texel_size;
		slice_pitch = row_pitch * heights_[level];

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
					re.BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbos_[level]);

					glBindTexture(target_type_, texture_);
					glGetTexImage(target_type_, level, gl_format, gl_type, nullptr);

					p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY));
				}
				else
				{
					glBindTexture(target_type_, texture_);
					glGetTexImage(target_type_, level, gl_format, gl_type, &tex_data_[level][0]);

					p = &tex_data_[level][0];
				}
			}
			break;

		case TMA_Write_Only:
			{
				if (!pbos_.empty())
				{
					re.BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbos_[level]);
					p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY));
				}
				else
				{
					p = &tex_data_[level][0];
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			p = nullptr;
			break;
		}

		data = p + ((z_offset * depthes_[level] + y_offset) * widths_[level] + x_offset) * texel_size;
	}

	void OGLTexture3D::Unmap3D(uint32_t array_index, uint32_t level)
	{
		BOOST_ASSERT(0 == array_index);
		UNREF_PARAM(array_index);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (last_tma_)
		{
		case TMA_Read_Only:
			if (!pbos_.empty())
			{
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
				re.BindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbos_[level]);
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
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbos_[level]);
					glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);

					p = nullptr;
				}
				else
				{
					p = &tex_data_[array_index * num_mip_maps_ + level][0];
				}

				if (IsCompressedFormat(format_))
				{
					glCompressedTexSubImage3D(target_type_, level, 0, 0, 0,
							widths_[level], heights_[level], depthes_[level], gl_format, image_size,
							p);
				}
				else
				{
					glTexSubImage3D(target_type_, level, 0, 0, 0, widths_[level], heights_[level], depthes_[level],
							gl_format, gl_type, p);
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
}
