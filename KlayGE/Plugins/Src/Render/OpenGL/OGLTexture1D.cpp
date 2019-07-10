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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
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
	OGLTexture1D::OGLTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: OGLTexture(TT_1D, array_size, sample_count, sample_quality, access_hint)
	{
		format_ = format;

		if (0 == numMipMaps)
		{
			num_mip_maps_ = 1;
			uint32_t w = width;
			while (w != 1)
			{
				++ num_mip_maps_;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		width_ = width;

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

			GLsizei image_size;
			if (IsCompressedFormat(format_))
			{
				uint32_t const block_size = NumFormatBytes(format_) * 4;
				image_size = ((w + 3) / 4) * block_size;
			}
			else
			{
				uint32_t const texel_size = NumFormatBytes(format_);
				image_size = w * texel_size;
			}

			mipmap_start_offset_[level + 1] = mipmap_start_offset_[level] + image_size;
		}
	}

	uint32_t OGLTexture1D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	void OGLTexture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

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

	void OGLTexture1D::CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		BOOST_ASSERT(type_ == target.Type());
		
		if ((format_ == target.Format()) && !IsCompressedFormat(format_) && (glloader_GL_VERSION_4_3() || glloader_GL_ARB_copy_image())
			&& (src_width == dst_width) && (1 == sample_count_))
		{
			auto& ogl_target = checked_cast<OGLTexture&>(target);
			glCopyImageSubData(
				texture_, target_type_, src_level,
				src_x_offset, 0, src_array_index,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, 0, dst_array_index, src_width, 1, 1);
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
						glFramebufferTexture1D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target_type_, texture_, src_level);
					}
					else
					{
						glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0,
											GL_RENDERBUFFER, texture_);
					}
				}

				auto& ogl_target = checked_cast<OGLTexture&>(target);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_dst);
				if (array_size_ > 1)
				{
					glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ogl_target.GLTexture(), dst_level, dst_array_index);
				}
				else
				{
					glFramebufferTexture1D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target_type_, ogl_target.GLTexture(), dst_level);
				}

				glBlitFramebuffer(src_x_offset, 0, src_x_offset + src_width, 1,
								dst_x_offset, 0, dst_x_offset + dst_width, 1,
								GL_COLOR_BUFFER_BIT, (src_width == dst_width) ? GL_NEAREST : GL_LINEAR);

				re.BindFramebuffer(old_fbo, true);
			}
			else
			{
				if ((src_width == dst_width) && (format_ == target.Format()))
				{
					if (IsCompressedFormat(format_))
					{
						BOOST_ASSERT(0 == (src_x_offset & 0x3));
						BOOST_ASSERT(0 == (dst_x_offset & 0x3));
						BOOST_ASSERT(0 == (src_width & 0x3));
						BOOST_ASSERT(0 == (dst_width & 0x3));

						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, 0, this->Width(src_level));
						Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, 0, target.Width(dst_level));

						uint32_t const block_size = NumFormatBytes(format_) * 4;
						uint8_t const * s = mapper_src.Pointer<uint8_t>() + (src_x_offset / 4 * block_size);
						uint8_t* d = mapper_dst.Pointer<uint8_t>() + (dst_x_offset / 4 * block_size);
						std::memcpy(d, s, src_width / 4 * block_size);
					}
					else
					{
						size_t const format_size = NumFormatBytes(format_);

						Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_width);
						Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_width);
						uint8_t const * s = mapper_src.Pointer<uint8_t>();
						uint8_t* d = mapper_dst.Pointer<uint8_t>();

						std::memcpy(d, s, src_width * format_size);
					}
				}
				else
				{
					this->ResizeTexture1D(target, dst_array_index, dst_level, dst_x_offset, dst_width,
						src_array_index, src_level, src_x_offset, src_width, true);
				}
			}
		}
	}

	void OGLTexture1D::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t /*width*/, void*& data)
	{
		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);

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
			data = p + (x_offset / 4 * block_size);
		}
		else
		{
			data = p + x_offset * texel_size;
		}
	}

	void OGLTexture1D::Unmap1D(uint32_t array_index, uint32_t level)
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

				re.BindTexture(0, target_type_, texture_);

				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				GLvoid* offset = reinterpret_cast<GLvoid*>(
					static_cast<GLintptr>(array_index * mipmap_start_offset_.back() + mipmap_start_offset_[level]));
				if (IsCompressedFormat(format_))
				{
					uint32_t const block_size = NumFormatBytes(format_) * 4;
					GLsizei const image_size = ((w + 3) / 4) * block_size;

					if (array_size_ > 1)
					{
						glCompressedTexSubImage2D(target_type_, level, 0, array_index,
							w, 1, gl_format, image_size, offset);
					}
					else
					{
						glCompressedTexSubImage1D(target_type_, level, 0,
							w, gl_format, image_size, offset);
					}
				}
				else
				{
					if (array_size_ > 1)
					{
						glTexSubImage2D(target_type_, level, 0, array_index, w, 1,
							gl_format, gl_type, offset);
					}
					else
					{
						glTexSubImage1D(target_type_, level, 0, w, gl_format, gl_type, offset);
					}
				}
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid texture map access mode");
		}
	}

	void OGLTexture1D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		KFL_UNUSED(clear_value_hint);

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

				if (array_size_ > 1)
				{
					glTextureStorage2D(texture_, num_mip_maps_, glinternalFormat, w0, array_size_);
				}
				else
				{
					glTextureStorage1D(texture_, num_mip_maps_, glinternalFormat, w0);
				}

				if (!init_data.empty())
				{
					for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
					{
						for (uint32_t level = 0; level < num_mip_maps_; ++ level)
						{
							uint32_t const w = this->Width(level);
							GLvoid const * data = init_data[array_index * num_mip_maps_ + level].data;

							if (IsCompressedFormat(format_))
							{
								uint32_t const block_size = NumFormatBytes(format_) * 4;
								GLsizei const image_size = ((w + 3) / 4) * block_size;

								if (array_size_ > 1)
								{
									glCompressedTextureSubImage2D(texture_, level, 0, array_index,
										w, 1, glformat, image_size, data);
								}
								else
								{
									glCompressedTextureSubImage1D(texture_, level, 0,
										w, glformat, image_size, data);
								}
							}
							else
							{
								if (array_size_ > 1)
								{
									glTextureSubImage2D(texture_, level, 0, array_index, w, 1,
										glformat, gltype, data);
								}
								else
								{
									glTextureSubImage1D(texture_, level, 0, w, glformat, gltype, data);
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

					if (array_size_ > 1)
					{
						glTexStorage2D(target_type_, num_mip_maps_, glinternalFormat, w0, array_size_);
					}
					else
					{
						glTexStorage1D(target_type_, num_mip_maps_, glinternalFormat, w0);
					}

					if (!init_data.empty())
					{
						for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
						{
							for (uint32_t level = 0; level < num_mip_maps_; ++ level)
							{
								uint32_t const w = this->Width(level);
								GLvoid const * data = init_data[array_index * num_mip_maps_ + level].data;

								if (IsCompressedFormat(format_))
								{
									uint32_t const block_size = NumFormatBytes(format_) * 4;
									GLsizei const image_size = ((w + 3) / 4) * block_size;

									if (array_size_ > 1)
									{
										glCompressedTexSubImage2D(target_type_, level, 0, array_index,
											w, 1, glformat, image_size, data);
									}
									else
									{
										glCompressedTexSubImage1D(target_type_, level, 0,
											w, glformat, image_size, data);
									}
								}
								else
								{
									if (array_size_ > 1)
									{
										glTexSubImage2D(target_type_, level, 0, array_index, w, 1,
											glformat, gltype, data);
									}
									else
									{
										glTexSubImage1D(target_type_, level, 0, w, glformat, gltype, data);
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

							if (IsCompressedFormat(format_))
							{
								uint32_t const block_size = NumFormatBytes(format_) * 4;
								GLsizei const image_size = ((w + 3) / 4) * block_size;

								if (array_size_ > 1)
								{
									if (0 == array_index)
									{
										glCompressedTexImage2D(target_type_, level, glinternalFormat,
											w, array_size_, 0, image_size * array_size_, nullptr);
									}

									if (!init_data.empty())
									{
										glCompressedTexSubImage2D(target_type_, level, 0, array_index, w, 1,
											glformat, image_size, init_data[array_index * num_mip_maps_ + level].data);
									}
								}
								else
								{
									glCompressedTexImage1D(target_type_, level, glinternalFormat,
										w, 0, image_size,
										init_data.empty() ? nullptr : init_data[array_index * num_mip_maps_ + level].data);
								}
							}
							else
							{
								if (array_size_ > 1)
								{
									if (0 == array_index)
									{
										glTexImage2D(target_type_, level, glinternalFormat, w, array_size_, 0, glformat, gltype, nullptr);
									}

									if (!init_data.empty())
									{
										glTexSubImage2D(target_type_, level, 0, array_index, w, 1,
											glformat, gltype, init_data[array_index * num_mip_maps_ + level].data);
									}
								}
								else
								{
									glTexImage1D(target_type_, level, glinternalFormat, w, 0, glformat, gltype,
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
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, sample_count_, glinternalFormat, width_, 1);
		}

		hw_res_ready_ = true;
	}

	void OGLTexture1D::UpdateSubresource1D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t width,
		void const * data)
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
			uint32_t const block_size = NumFormatBytes(format_) * 4;
			GLsizei const image_size = ((width + 3) / 4) * block_size;

			if (array_size_ > 1)
			{
				glCompressedTexSubImage2D(target_type_, level, x_offset, array_index,
					width, 1, gl_format, image_size, data);
			}
			else
			{
				glCompressedTexSubImage1D(target_type_, level, x_offset,
					width, gl_format, image_size, data);
			}
		}
		else
		{
			if (array_size_ > 1)
			{
				glTexSubImage2D(target_type_, level, x_offset, array_index, width, 1,
					gl_format, gl_type, data);
			}
			else
			{
				glTexSubImage1D(target_type_, level, x_offset, width,
					gl_format, gl_type, data);
			}
		}
	}
}
