// OGLTextureCube.cpp
// KlayGE OpenGL Cube������ ʵ���ļ�
// Ver 3.9.0
// ��Ȩ����(C) ������, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// ֧��GL_NV_copy_image (2009.8.5)
//
// 3.8.0
// ͨ��GL_EXT_framebuffer_blit����CopyTexture (2008.10.12)
//
// 3.6.0
// ��pbo���� (2007.3.13)
//
// 3.2.0
// ���ν��� (2006.4.30)
//
// �޸ļ�¼
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

namespace KlayGE
{
	OGLTextureCube::OGLTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
								uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: OGLTexture(TT_Cube, array_size, sample_count, sample_quality, access_hint)
	{
		format_ = format;

		if (0 == numMipMaps)
		{
			num_mip_maps_ = 1;
			uint32_t s = size;
			while (s > 1)
			{
				++ num_mip_maps_;

				s = std::max<uint32_t>(1U, s / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		width_ = size;

		pbos_.resize(array_size * num_mip_maps_ * 6);
		glGenBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);
	}

	uint32_t OGLTextureCube::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t OGLTextureCube::Height(uint32_t level) const
	{
		return this->Width(level);
	}

	void OGLTextureCube::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
		{
			for (int face = 0; face < 6; ++ face)
			{
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					this->CopyToSubTextureCube(target,
						array_index, static_cast<CubeFaces>(face), level, 0, 0, target.Width(level), target.Height(level),
						array_index, static_cast<CubeFaces>(face), level, 0, 0, this->Width(level), this->Height(level));
				}
			}
		}
	}

	void OGLTextureCube::CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && !IsCompressedFormat(format_) && (glloader_GL_VERSION_4_3() || glloader_GL_ARB_copy_image())
			&& (src_width == dst_width) && (src_height == dst_height) && (1 == sample_count_))
		{
			OGLTexture& ogl_target = *checked_cast<OGLTexture*>(&target);
			glCopyImageSubData(
				texture_, target_type_, src_level,
				src_x_offset, src_y_offset, src_array_index * 6 + src_face - CF_Positive_X,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, dst_y_offset, dst_array_index * 6 + dst_face - CF_Positive_X, src_width, src_height, 1);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (!IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))))
			{
				GLuint fbo_src, fbo_dst;
				re.GetFBOForBlit(fbo_src, fbo_dst);

				GLuint old_fbo = re.BindFramebuffer();

				glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_src);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, src_level, src_array_index * 6 + src_face - CF_Positive_X);
				}
				else
				{
					glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + src_face - CF_Positive_X, texture_, src_level);
				}

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_dst);
				if (target.ArraySize() > 1)
				{
					glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, checked_cast<OGLTexture*>(&target)->GLTexture(), dst_level, dst_array_index * 6 + dst_face - CF_Positive_X);
				}
				else
				{
					glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + dst_face - CF_Positive_X, checked_cast<OGLTexture*>(&target)->GLTexture(), dst_level);
				}

				glBlitFramebuffer(src_x_offset, src_y_offset, src_x_offset + src_width, src_y_offset + src_height,
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

						Texture::Mapper mapper_src(*this, src_array_index, src_face, src_level, TMA_Read_Only, 0, 0, this->Width(src_level), this->Height(src_level));
						Texture::Mapper mapper_dst(target, dst_array_index, dst_face, dst_level, TMA_Write_Only, 0, 0, target.Width(dst_level), target.Height(dst_level));

						uint32_t const block_size = NumFormatBytes(format_) * 4;
						uint8_t const * s = mapper_src.Pointer<uint8_t>() + (src_y_offset / 4) * mapper_src.RowPitch() + (src_x_offset / 4 * block_size);
						uint8_t* d = mapper_dst.Pointer<uint8_t>() + (dst_y_offset / 4) * mapper_dst.RowPitch() + (dst_x_offset / 4 * block_size);
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

						Texture::Mapper mapper_src(*this, src_array_index, src_face, src_level, TMA_Read_Only, src_x_offset, src_y_offset, src_width, src_height);
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
						src_array_index, src_face, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
				}
			}
		}
	}

	void OGLTextureCube::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
					uint32_t x_offset, uint32_t y_offset, uint32_t /*width*/, uint32_t /*height*/,
					void*& data, uint32_t& row_pitch)
	{
		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);
		uint32_t const w = this->Width(level);

		row_pitch = w * texel_size;

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

				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[(array_index * 6 + face) * num_mip_maps_ + level]);

				re.BindTexture(0, target_type_, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, nullptr);
					p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
				}
				else
				{
					glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, gl_format, gl_type, nullptr);
					p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
				}
			}
			break;

		case TMA_Write_Only:
			{
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[(array_index * 6 + face) * num_mip_maps_ + level]);
				p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
			}
			break;

		default:
			BOOST_ASSERT(false);
			p = nullptr;
			break;
		}

		if (IsCompressedFormat(format_))
		{
			uint32_t const block_size = NumFormatBytes(format_) * 4;
			data = p + (y_offset / 4) * row_pitch + (x_offset / 4 * block_size);
		}
		else
		{
			data = p + (y_offset * w + x_offset) * texel_size;
		}
	}

	void OGLTextureCube::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (last_tma_)
		{
		case TMA_Read_Only:
			re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbos_[(array_index * 6 + face) * num_mip_maps_ + level]);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			break;

		case TMA_Write_Only:
		case TMA_Read_Write:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				uint32_t const s = this->Width(level);

				re.BindTexture(0, target_type_, texture_);

				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[(array_index * 6 + face) * num_mip_maps_ + level]);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				if (IsCompressedFormat(format_))
				{
					uint32_t const block_size = NumFormatBytes(format_) * 4;
					GLsizei const image_size = ((s + 3) / 4) * ((s + 3) / 4) * block_size;

					if (array_size_ > 1)
					{
						glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
							0, 0, array_index, s, s, 1, gl_format, image_size, nullptr);
					}
					else
					{
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
							0, 0, s, s, gl_format, image_size, nullptr);
					}
				}
				else
				{
					if (array_size_ > 1)
					{
						glTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
							0, 0, array_index, s, s, 1,
							gl_format, gl_type, nullptr);
					}
					else
					{
						glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
							0, 0, s, s, gl_format, gl_type, nullptr);
					}
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void OGLTextureCube::CreateHWResource(ElementInitData const * init_data)
	{
		uint32_t texel_size = NumFormatBytes(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
		{
			for (int face = 0; face < 6; ++ face)
			{
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					uint32_t const s = this->Width(level);

					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[(array_index * 6 + face) * num_mip_maps_ + level]);
					if (IsCompressedFormat(format_))
					{
						uint32_t const block_size = NumFormatBytes(format_) * 4;
						GLsizei const image_size = ((s + 3) / 4) * ((s + 3) / 4) * block_size;

						glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, nullptr, GL_STREAM_DRAW);
						re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

						if (array_size_ > 1)
						{
							if (0 == array_index)
							{
								glCompressedTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
									s, s, array_size_, 0, image_size * array_size_, nullptr);
							}

							if (init_data != nullptr)
							{
								glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, array_index, s, s, 1,
									glformat, image_size, init_data[(array_index * 6 + face) * num_mip_maps_ + level].data);
							}
						}
						else
						{
							glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
								s, s, 0, image_size, (nullptr == init_data) ? nullptr : init_data[(array_index * 6 + face) * num_mip_maps_ + level].data);
						}
					}
					else
					{
						GLsizei const image_size = s * s * texel_size;

						glBufferData(GL_PIXEL_UNPACK_BUFFER, image_size, nullptr, GL_STREAM_DRAW);
						re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

						if (array_size_ > 1)
						{
							if (0 == array_index)
							{
								glTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat, s, s, array_size_, 0, glformat, gltype, nullptr);
							}

							if (init_data != nullptr)
							{
								glTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, array_index, s, s, 1,
									glformat, gltype, init_data[array_index * num_mip_maps_ + level].data);
							}
						}
						else
						{
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
								s, s, 0, glformat, gltype, (nullptr == init_data) ? nullptr : init_data[(array_index * 6 + face) * num_mip_maps_ + level].data);
						}
					}
				}
			}
		}

		hw_res_ready_ = true;
	}
}
