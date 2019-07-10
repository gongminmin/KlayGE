// OGLESTexture1D.cpp
// KlayGE OpenGL ES 2 1D纹理类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
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

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>

namespace KlayGE
{
	OGLESTexture1D::OGLESTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: OGLESTexture(TT_1D, array_size, sample_count, sample_quality, access_hint)
	{
		if (IsSRGB(format))
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

				w = std::max<uint32_t>(1U, w / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		width_ = width;

		tex_data_.resize(array_size_ * num_mip_maps_);

		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);
	}

	uint32_t OGLESTexture1D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	void OGLESTexture1D::CopyToTexture(Texture& target)
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

	void OGLESTexture1D::CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((sample_count_ > 1) && !IsCompressedFormat(format_) && (glloader_GLES_EXT_texture_rg() || (4 == NumComponents(format_))))
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		
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

			auto& ogl_target = checked_cast<OGLESTexture&>(target);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_dst);
			if (array_size_ > 1)
			{
				glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ogl_target.GLTexture(), dst_level, dst_array_index);
			}
			else
			{
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ogl_target.GLType(), ogl_target.GLTexture(), dst_level);
			}

			glBlitFramebuffer(src_x_offset, 0, src_x_offset + src_width, 1,
							dst_x_offset, 0, dst_x_offset + dst_width, 1,
							GL_COLOR_BUFFER_BIT, (src_width == dst_width) ? GL_NEAREST : GL_LINEAR);

			re.BindFramebuffer(old_fbo, true);
		}
		else
		{
			BOOST_ASSERT(format_ == target.Format());

			GLint gl_internalFormat;
			GLenum gl_format;
			GLenum gl_type;
			OGLESMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

			GLint gl_target_internal_format;
			GLenum gl_target_format;
			GLenum gl_target_type;
			OGLESMapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

			if (IsCompressedFormat(format_))
			{
				BOOST_ASSERT(src_width == dst_width);
				BOOST_ASSERT(0 == (src_x_offset & 0x3));
				BOOST_ASSERT(0 == (dst_x_offset & 0x3));
				BOOST_ASSERT(0 == (src_width & 0x3));
				BOOST_ASSERT(0 == (dst_width & 0x3));

				Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_width);
				Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_width);

				uint32_t const block_size = NumFormatBytes(format_) * 4;
				uint8_t const * s = mapper_src.Pointer<uint8_t>();
				uint8_t* d = mapper_dst.Pointer<uint8_t>();
				std::memcpy(d, s, src_width / 4 * block_size);
			}
			else
			{
				if (src_width == dst_width)
				{
					size_t const format_size = NumFormatBytes(format_);

					Texture::Mapper mapper_src(*this, src_array_index, src_level, TMA_Read_Only, src_x_offset, src_width);
					Texture::Mapper mapper_dst(target, dst_array_index, dst_level, TMA_Write_Only, dst_x_offset, dst_width);
					uint8_t const * s = mapper_src.Pointer<uint8_t>();
					uint8_t* d = mapper_dst.Pointer<uint8_t>();

					std::memcpy(d, s, src_width * format_size);
				}
				else
				{
					this->ResizeTexture1D(target, dst_array_index, dst_level, dst_x_offset, dst_width,
							src_array_index, src_level, src_x_offset, src_width, true);
				}
			}
		}
	}

	void OGLESTexture1D::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t /*width*/, void*& data)
	{
		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);

		uint8_t* p = &tex_data_[array_index * num_mip_maps_ + level][0];
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

	void OGLESTexture1D::Unmap1D(uint32_t array_index, uint32_t level)
	{
		switch (last_tma_)
		{
		case TMA_Read_Only:
			break;

		case TMA_Write_Only:
		case TMA_Read_Write:
			{
				GLint gl_internalFormat;
				GLenum gl_format;
				GLenum gl_type;
				OGLESMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				uint32_t const w = this->Width(level);

				auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindTexture(0, target_type_, texture_);

				if (IsCompressedFormat(format_))
				{
					uint32_t const block_size = NumFormatBytes(format_) * 4; 
					GLsizei const image_size = ((w + 3) / 4) * block_size;

					if (array_size_ > 1)
					{
						glCompressedTexSubImage3D(target_type_, level, 0, 0, array_index,
							w, 1, 1, gl_format, image_size, &tex_data_[array_index * num_mip_maps_ + level][0]);
					}
					else
					{
						glCompressedTexSubImage2D(target_type_, level, 0, 0,
							w, 1, gl_format, image_size, &tex_data_[array_index * num_mip_maps_ + level][0]);
					}
				}
				else
				{
					if (array_size_ > 1)
					{
						glTexSubImage3D(target_type_, level, 0, 0, array_index, w, 1, 1,
							gl_format, gl_type, &tex_data_[array_index * num_mip_maps_ + level][0]);
					}
					else
					{
						glTexSubImage2D(target_type_, level, 0, 0, w, 1,
							gl_format, gl_type, &tex_data_[array_index * num_mip_maps_ + level][0]);
					}
				}
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid texture map access mode");
		}
	}

	void OGLESTexture1D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		KFL_UNUSED(clear_value_hint);

		uint32_t texel_size = NumFormatBytes(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLESMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, target_type_, texture_);

		if (!re.HackForAdreno())
		{
			uint32_t const w0 = this->Width(0);

			if (array_size_ > 1)
			{
				glTexStorage3D(target_type_, num_mip_maps_, glinternalFormat, w0, 1, array_size_);
			}
			else
			{
				glTexStorage2D(target_type_, num_mip_maps_, glinternalFormat, w0, 1);
			}

			for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
			{
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					uint32_t const w = this->Width(level);

					if (IsCompressedFormat(format_))
					{
						uint32_t const block_size = NumFormatBytes(format_) * 4;
						GLsizei const image_size = ((w + 3) / 4) * block_size;

						if (init_data.empty())
						{
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size, 0);
						}
						else
						{
							GLvoid const * data = init_data[array_index * num_mip_maps_ + level].data;
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size);
							std::memcpy(&tex_data_[array_index * num_mip_maps_ + level][0],
								data, image_size);

							if (array_size_ > 1)
							{
								glCompressedTexSubImage3D(target_type_, level, 0, 0, array_index,
									w, 1, 1, glformat, image_size, data);
							}
							else
							{
								glCompressedTexSubImage2D(target_type_, level, 0, 0,
									w, 1, glformat, image_size, data);
							}
						}
					}
					else
					{
						GLsizei const image_size = w * texel_size;

						if (init_data.empty())
						{
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size, 0);
						}
						else
						{
							GLvoid const * data = init_data[array_index * num_mip_maps_ + level].data;
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size);
							std::memcpy(&tex_data_[array_index * num_mip_maps_ + level][0],
								data, image_size);

							if (array_size_ > 1)
							{
								glTexSubImage3D(target_type_, level, 0, 0, array_index, w, 1, 1,
									glformat, gltype, data);
							}
							else
							{
								glTexSubImage2D(target_type_, level, 0, 0, w, 1, glformat, gltype, data);
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

						void* ptr;
						if (init_data.empty())
						{
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size, 0);
							ptr = nullptr;
						}
						else
						{
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size);
							std::memcpy(&tex_data_[array_index * num_mip_maps_ + level][0],
								init_data[array_index * num_mip_maps_ + level].data, image_size);
							ptr = &tex_data_[array_index * num_mip_maps_ + level][0];
						}

						if (array_size_ > 1)
						{
							if (0 == array_index)
							{
								glCompressedTexImage3D(target_type_, level, glinternalFormat,
									w, 1, array_size_, 0, image_size * array_size_, nullptr);
							}

							if (!init_data.empty())
							{
								glCompressedTexSubImage3D(target_type_, level, 0, 0, array_index, w, 1, 1,
									glformat, image_size, init_data[array_index * num_mip_maps_ + level].data);
							}
						}
						else
						{
							glCompressedTexImage2D(target_type_, level, glinternalFormat,
								w, 1, 0, image_size, ptr);
						}
					}
					else
					{
						GLsizei const image_size = w * texel_size;

						void* ptr;
						if (init_data.empty())
						{
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size, 0);
							ptr = nullptr;
						}
						else
						{
							tex_data_[array_index * num_mip_maps_ + level].resize(image_size);
							std::memcpy(&tex_data_[array_index * num_mip_maps_ + level][0],
								init_data[array_index * num_mip_maps_ + level].data, image_size);
							ptr = &tex_data_[array_index * num_mip_maps_ + level][0];
						}

						if (array_size_ > 1)
						{
							if (0 == array_index)
							{
								glTexImage3D(target_type_, level, glinternalFormat, w, 1, array_size_, 0, glformat, gltype, nullptr);
							}

							if (!init_data.empty())
							{
								glTexSubImage3D(target_type_, level, 0, 0, array_index, w, 1, 1,
									glformat, gltype, init_data[array_index * num_mip_maps_ + level].data);
							}
						}
						else
						{
							glTexImage2D(target_type_, level, glinternalFormat, w, 1, 0, glformat, gltype, ptr);
						}
					}
				}
			}
		}

		hw_res_ready_ = true;
	}

	void OGLESTexture1D::UpdateSubresource1D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t width,
		void const * data)
	{
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLESMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		re.BindTexture(0, target_type_, texture_);

		if (IsCompressedFormat(format_))
		{
			uint32_t const block_size = NumFormatBytes(format_) * 4;
			GLsizei const image_size = ((width + 3) / 4) * block_size;

			if (array_size_ > 1)
			{
				glCompressedTexSubImage3D(target_type_, level, x_offset, 0, array_index,
					width, 1, 1, gl_format, image_size, data);
			}
			else
			{
				glCompressedTexSubImage2D(target_type_, level, x_offset, 0,
					width, 1, gl_format, image_size, data);
			}
		}
		else
		{
			if (array_size_ > 1)
			{
				glTexSubImage3D(target_type_, level, x_offset, 0, array_index, width, 1, 1,
					gl_format, gl_type, data);
			}
			else
			{
				glTexSubImage2D(target_type_, level, x_offset, 0, width, 1,
					gl_format, gl_type, data);
			}
		}
	}
}
