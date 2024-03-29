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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>

#include "OGLRenderEngine.hpp"
#include "OGLUtil.hpp"
#include "OGLTexture.hpp"

namespace KlayGE
{
	OGLTexture2D::OGLTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: OGLTexture(TT_2D, array_size, sample_count, sample_quality, access_hint)
	{
		format_ = format;

		if (0 == numMipMaps)
		{
			num_mip_maps_ = 1;
			uint32_t w = width;
			uint32_t h = height;
			while ((w != 1) || (h != 1))
			{
				++ num_mip_maps_;

				w = std::max(1U, w / 2);
				h = std::max(1U, h / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		width_ = width;
		height_ = height;

		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glCreateBuffers(1, &pbo_);
		}
		else
		{
			glGenBuffers(1, &pbo_);
		}

		mipmap_start_offset_.resize(num_mip_maps_ + 1);
		mipmap_start_offset_[0] = 0;
		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			uint32_t const w = this->Width(level);
			uint32_t const h = this->Height(level);

			GLsizei image_size;
			if (IsCompressedFormat(format_))
			{
				uint32_t const block_size = NumFormatBytes(format_) * 4;
				image_size = ((w + 3) / 4) * ((h + 3) / 4) * block_size;
			}
			else
			{
				uint32_t const texel_size = NumFormatBytes(format_);
				image_size = w * h * texel_size;
			}

			mipmap_start_offset_[level + 1] = mipmap_start_offset_[level] + image_size;
		}
	}

	uint32_t OGLTexture2D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max(1U, width_ >> level);
	}

	uint32_t OGLTexture2D::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max(1U, height_ >> level);
	}

	void OGLTexture2D::CopyToTexture(Texture& target, TextureFilter filter)
	{
		BOOST_ASSERT(type_ == target.Type());

		for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
		{
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				this->CopyToSubTexture2D(target, array_index, level, 0, 0, target.Width(level), target.Height(level), array_index, level, 0,
					0, this->Width(level), this->Height(level), filter);
			}
		}
	}

	void OGLTexture2D::CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
		uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset,
		uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && !IsCompressedFormat(format_) && (glloader_GL_VERSION_4_3() || glloader_GL_ARB_copy_image())
			&& (src_width == dst_width) && (src_height == dst_height) && (1 == sample_count_))
		{
			OGLTexture& ogl_target = checked_cast<OGLTexture&>(target);
			glCopyImageSubData(
				texture_, target_type_, src_level,
				src_x_offset, src_y_offset, src_array_index,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, dst_y_offset, dst_array_index, src_width, src_height, 1);
		}
		else
		{
			if ((sample_count_ > 1) && !IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))))
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				GLuint fbo_src, fbo_dst;
				re.GetFBOForBlit(fbo_src, fbo_dst);

				GLuint old_fbo = re.BindFramebuffer();

				glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_src);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, src_level, src_array_index);
				}
				else
				{
					if (sample_count_ <= 1)
					{
						glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target_type_, texture_, src_level);
					}
					else
					{
						glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0,
											GL_RENDERBUFFER, texture_);
					}
				}

				OGLTexture& ogl_target = checked_cast<OGLTexture&>(target);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_dst);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ogl_target.GLTexture(), dst_level, dst_array_index);
				}
				else
				{
					glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ogl_target.GLType(), ogl_target.GLTexture(), dst_level);
				}

				glBlitFramebuffer(src_x_offset, src_y_offset, src_x_offset + src_width, src_y_offset + src_height,
								dst_x_offset, dst_y_offset, dst_x_offset + dst_width, dst_y_offset + dst_height,
								GL_COLOR_BUFFER_BIT, ((src_width == dst_width) && (src_height == dst_height)) ? GL_NEAREST : GL_LINEAR);

				re.BindFramebuffer(old_fbo, true);
			}
			else if ((src_width == dst_width) && (src_height == dst_height) && (format_ == target.Format()))
			{
				if (IsCompressedFormat(format_))
				{
					BOOST_ASSERT((src_width == dst_width) && (src_height == dst_height));
					BOOST_ASSERT((0 == (src_x_offset & 0x3)) && (0 == (src_y_offset & 0x3)));
					BOOST_ASSERT((0 == (dst_x_offset & 0x3)) && (0 == (dst_y_offset & 0x3)));
					BOOST_ASSERT((0 == (src_width & 0x3)) && (0 == (src_height & 0x3)));
					BOOST_ASSERT((0 == (dst_width & 0x3)) && (0 == (dst_height & 0x3)));

					Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_y_offset, src_width, src_height);
					Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_y_offset, dst_width, dst_height);

					uint32_t const block_size = NumFormatBytes(format_) * 4;
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
			}
			else
			{
				this->ResizeTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
					src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
			}
		}
	}

	void OGLTexture2D::CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level,
		uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, [[maybe_unused]] CubeFaces src_face,
		uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter)
	{
		BOOST_ASSERT(TT_Cube == target.Type());

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		if ((format_ == target.Format()) && !IsCompressedFormat(format_) && (glloader_GL_VERSION_4_3() || glloader_GL_ARB_copy_image())
			&& (src_width == dst_width) && (src_height == dst_height) && (1 == sample_count_))
		{
			OGLTexture& ogl_target = checked_cast<OGLTexture&>(target);
			glCopyImageSubData(
				texture_, target_type_, src_level,
				src_x_offset, src_y_offset, src_array_index,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, dst_y_offset, dst_array_index * 6 + dst_face - CF_Positive_X, src_width, src_height, 1);
		}
		else
		{
			if ((sample_count_ > 1) && !IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))))
			{
				GLuint fbo_src, fbo_dst;
				re.GetFBOForBlit(fbo_src, fbo_dst);

				GLuint old_fbo = re.BindFramebuffer();

				glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_src);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, src_level, src_array_index);
				}
				else
				{
					if (sample_count_ <= 1)
					{
						glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target_type_, texture_, src_level);
					}
					else
					{
						glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0,
											GL_RENDERBUFFER, texture_);
					}
				}

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_dst);
				if (target.ArraySize() > 1)
				{
					glFramebufferTextureLayer(
						GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, dst_level, dst_array_index * 6 + dst_face - CF_Positive_X);
				}
				else
				{
					glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + dst_face,
						checked_cast<OGLTexture&>(target).GLTexture(), dst_level);
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

						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_y_offset, src_width, src_height);
						Texture::Mapper mapper_dst(target, dst_array_index, dst_face, dst_level, TMA_Write_Only, dst_x_offset, dst_y_offset, dst_width, dst_height);

						uint32_t const block_size = NumFormatBytes(format_) * 4;
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
						src_array_index, CF_Positive_X, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
				}
			}
		}
	}

	void OGLTexture2D::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
					uint32_t x_offset, uint32_t y_offset, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height,
					void*& data, uint32_t& row_pitch)
	{
		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);
		uint32_t const w = this->Width(level);

		row_pitch = w * texel_size;

		uint8_t* p;
		GLintptr const subres_offset = array_index * mipmap_start_offset_.back() + mipmap_start_offset_[level];
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);

				uint32_t const slice_size = (mipmap_start_offset_[level + 1] - mipmap_start_offset_[level]);
				std::vector<uint8_t> array_data(this->ArraySize() * slice_size);

				re.BindTexture(0, target_type_, texture_);
				if (IsCompressedFormat(format_))
				{
					glGetCompressedTexImage(target_type_, level, array_data.data());
				}
				else
				{
					glGetTexImage(target_type_, level, gl_format, gl_type, array_data.data());
				}

				re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbo_);
				p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
				memcpy(p + subres_offset, &array_data[array_index * slice_size], slice_size);
			}
			break;

		case TMA_Write_Only:
			re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
			re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
			p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
			break;

		default:
			KFL_UNREACHABLE("Invalid texture map access mode");
		}

		p += subres_offset;
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

	void OGLTexture2D::Unmap2D(uint32_t array_index, uint32_t level)
	{
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		switch (last_tma_)
		{
		case TMA_Read_Only:
			re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbo_);
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			break;

		case TMA_Write_Only:
		case TMA_Read_Write:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				uint32_t const w = this->Width(level);
				uint32_t const h = this->Height(level);

				re.BindTexture(0, target_type_, texture_);

				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				GLvoid* offset = reinterpret_cast<GLvoid*>(
					static_cast<GLintptr>(array_index * mipmap_start_offset_.back() + mipmap_start_offset_[level]));
				if (IsCompressedFormat(format_))
				{
					uint32_t const block_size = NumFormatBytes(format_) * 4;
					GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * block_size;

					if (array_size_ > 1)
					{
						glCompressedTexSubImage3D(target_type_, level, 0, 0, array_index,
							w, h, 1, gl_format, image_size, offset);
					}
					else
					{
						glCompressedTexSubImage2D(target_type_, level, 0, 0,
							w, h, gl_format, image_size, offset);
					}
				}
				else
				{
					if (array_size_ > 1)
					{
						glTexSubImage3D(target_type_, level, 0, 0, array_index, w, h, 1,
							gl_format, gl_type, offset);
					}
					else
					{
						glTexSubImage2D(target_type_, level, 0, 0, w, h, gl_format, gl_type, offset);
					}
				}
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid texture map access mode");
		}
	}
	
	void OGLTexture2D::CreateHWResource(std::span<ElementInitData const> init_data, [[maybe_unused]] float4 const * clear_value_hint)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		if (sample_count_ <= 1)
		{
			uint32_t const pbo_size = mipmap_start_offset_.back() * array_size_;
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glTextureParameteri(texture_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

				glNamedBufferStorage(pbo_, pbo_size, nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

				uint32_t const w0 = this->Width(0);
				uint32_t const h0 = this->Height(0);

				if (array_size_ > 1)
				{
					glTextureStorage3D(texture_, num_mip_maps_, glinternalFormat, w0, h0, array_size_);
				}
				else
				{
					glTextureStorage2D(texture_, num_mip_maps_, glinternalFormat, w0, h0);
				}

				if (!init_data.empty())
				{
					for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
					{
						for (uint32_t level = 0; level < num_mip_maps_; ++ level)
						{
							uint32_t const w = this->Width(level);
							uint32_t const h = this->Height(level);
							GLvoid const * data = init_data[array_index * num_mip_maps_ + level].data;

							if (IsCompressedFormat(format_))
							{
								uint32_t const block_size = NumFormatBytes(format_) * 4;
								GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * block_size;

								if (array_size_ > 1)
								{
									glCompressedTextureSubImage3D(texture_, level, 0, 0, array_index,
										w, h, 1, glformat, image_size, data);
								}
								else
								{
									glCompressedTextureSubImage2D(texture_, level, 0, 0,
										w, h, glformat, image_size, data);
								}
							}
							else
							{
								if (array_size_ > 1)
								{
									glTextureSubImage3D(texture_, level, 0, 0, array_index, w, h, 1,
										glformat, gltype, data);
								}
								else
								{
									glTextureSubImage2D(texture_, level, 0, 0, w, h, glformat, gltype, data);
								}
							}
						}
					}
				}
			}
			else
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				re.BindTexture(0, target_type_, texture_);
				glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
				if (glloader_GL_VERSION_4_4() || glloader_GL_ARB_buffer_storage())
				{
					glBufferStorage(GL_PIXEL_UNPACK_BUFFER, pbo_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
				}
				else
				{
					glBufferData(GL_PIXEL_UNPACK_BUFFER, pbo_size, nullptr, GL_STREAM_COPY);
				}
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

				if (glloader_GL_VERSION_4_2() || glloader_GL_ARB_texture_storage())
				{
					uint32_t const w0 = this->Width(0);
					uint32_t const h0 = this->Height(0);

					if (array_size_ > 1)
					{
						glTexStorage3D(target_type_, num_mip_maps_, glinternalFormat, w0, h0, array_size_);
					}
					else
					{
						glTexStorage2D(target_type_, num_mip_maps_, glinternalFormat, w0, h0);
					}

					if (!init_data.empty())
					{
						for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
						{
							for (uint32_t level = 0; level < num_mip_maps_; ++ level)
							{
								uint32_t const w = this->Width(level);
								uint32_t const h = this->Height(level);
								GLvoid const * data = init_data[array_index * num_mip_maps_ + level].data;

								if (IsCompressedFormat(format_))
								{
									uint32_t const block_size = NumFormatBytes(format_) * 4;
									GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * block_size;

									if (array_size_ > 1)
									{
										glCompressedTexSubImage3D(target_type_, level, 0, 0, array_index,
											w, h, 1, glformat, image_size, data);
									}
									else
									{
										glCompressedTexSubImage2D(target_type_, level, 0, 0,
											w, h, glformat, image_size, data);
									}
								}
								else
								{
									if (array_size_ > 1)
									{
										glTexSubImage3D(target_type_, level, 0, 0, array_index, w, h, 1,
											glformat, gltype, data);
									}
									else
									{
										glTexSubImage2D(target_type_, level, 0, 0, w, h, glformat, gltype, data);
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
							uint32_t const w = this->Width(level);
							uint32_t const h = this->Height(level);

							if (IsCompressedFormat(format_))
							{
								uint32_t const block_size = NumFormatBytes(format_) * 4;
								GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * block_size;

								if (array_size_ > 1)
								{
									if (0 == array_index)
									{
										glCompressedTexImage3D(target_type_, level, glinternalFormat,
											w, h, array_size_, 0, image_size * array_size_, nullptr);
									}

									if (!init_data.empty())
									{
										glCompressedTexSubImage3D(target_type_, level, 0, 0, array_index, w, h, 1,
											glformat, image_size, init_data[array_index * num_mip_maps_ + level].data);
									}
								}
								else
								{
									glCompressedTexImage2D(target_type_, level, glinternalFormat,
										w, h, 0, image_size, init_data.empty() ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
								}
							}
							else
							{
								if (array_size_ > 1)
								{
									if (0 == array_index)
									{
										glTexImage3D(target_type_, level, glinternalFormat, w, h, array_size_, 0,
											glformat, gltype, nullptr);
									}

									if (!init_data.empty())
									{
										glTexSubImage3D(target_type_, level, 0, 0, array_index, w, h, 1,
											glformat, gltype, init_data[array_index * num_mip_maps_ + level].data);
									}
								}
								else
								{
									glTexImage2D(target_type_, level, glinternalFormat, w, h, 0, glformat, gltype,
										init_data.empty() ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
								}
							}
						}
					}
				}
			}
		}
		else
		{
			glBindRenderbuffer(GL_RENDERBUFFER, texture_);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, sample_count_, glinternalFormat,
				width_, height_);
		}

		hw_res_ready_ = true;
	}

	void OGLTexture2D::UpdateSubresource2D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		re.BindTexture(0, target_type_, texture_);

		if (IsCompressedFormat(format_))
		{
			GLsizei const image_size = row_pitch * ((height + 3) / 4);

			if (array_size_ > 1)
			{
				glCompressedTexSubImage3D(target_type_, level, x_offset, y_offset, array_index,
					width, height, 1, gl_format, image_size, data);
			}
			else
			{
				glCompressedTexSubImage2D(target_type_, level, x_offset, y_offset,
					width, height, gl_format, image_size, data);
			}
		}
		else
		{
			glPixelStorei(GL_UNPACK_ROW_LENGTH, row_pitch / NumFormatBytes(format_));
			if (array_size_ > 1)
			{
				glTexSubImage3D(target_type_, level, x_offset, y_offset, array_index, width, height, 1,
					gl_format, gl_type, data);
			}
			else
			{
				glTexSubImage2D(target_type_, level, x_offset, y_offset, width, height,
					gl_format, gl_type, data);
			}
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}
	}
}
