// OGLESTextureCube.cpp
// KlayGE OpenGL ES 2 Cube纹理类 实现文件
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
	OGLESTextureCube::OGLESTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
								uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: OGLESTexture(TT_Cube, array_size, sample_count, sample_quality, access_hint)
	{
		if (IsSRGB(format))
		{
			format = this->SRGBToRGB(format);
		}

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
		
		tex_data_.resize(array_size_ * num_mip_maps_ * 6);

		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);
	}

	uint32_t OGLESTextureCube::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t OGLESTextureCube::Height(uint32_t level) const
	{
		return this->Width(level);
	}

	void OGLESTextureCube::CopyToTexture(Texture& target)
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

	void OGLESTextureCube::CopyToSubTexture2D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level,
		uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, uint32_t src_level,
		uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(TT_2D == target.Type());

		if (!IsCompressedFormat(format_) && (glloader_GLES_EXT_texture_rg() || (4 == NumComponents(format_))))
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
				glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + src_array_index, texture_, src_level);
			}

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_dst);
			if (target.ArraySize() > 1)
			{
				glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, dst_level, dst_array_index);
			}
			else
			{
				glFramebufferTexture2D(
					GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, checked_cast<OGLESTexture&>(target).GLTexture(), dst_level);
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
					for (uint32_t y = 0; y < src_height; ++y)
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
					src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
			}
		}
	}

	void OGLESTextureCube::CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(type_ == target.Type());

		if (!IsCompressedFormat(format_) && (glloader_GLES_EXT_texture_rg() || (4 == NumComponents(format_))))
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

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
				glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, checked_cast<OGLESTexture&>(target).GLTexture(),
					dst_level, dst_array_index * 6 + dst_face - CF_Positive_X);
			}
			else
			{
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + dst_face - CF_Positive_X,
					checked_cast<OGLESTexture&>(target).GLTexture(), dst_level);
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

	void OGLESTextureCube::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
					uint32_t x_offset, uint32_t y_offset, uint32_t /*width*/, uint32_t /*height*/,
					void*& data, uint32_t& row_pitch)
	{
		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);
		uint32_t const w = this->Width(level);

		row_pitch = w * texel_size;

		uint8_t* p = &tex_data_[(array_index * 6 + face) * num_mip_maps_ + level][0];
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

	void OGLESTextureCube::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
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
					GLsizei const image_size = ((this->Width(level) + 3) / 4) * ((this->Height(level) + 3) / 4) * block_size;

					glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
						0, 0, w, w, gl_format, image_size, &tex_data_[(array_index * 6 + face) * num_mip_maps_ + level][0]);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level,
						0, 0, w, w, gl_format, gl_type, &tex_data_[(array_index * 6 + face) * num_mip_maps_ + level][0]);
				}
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid texture map access mode");
		}
	}

	void OGLESTextureCube::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
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
			uint32_t const s0 = this->Width(0);

			if (array_size_ > 1)
			{
				glTexStorage3D(target_type_, num_mip_maps_, glinternalFormat, s0, s0, array_size_ * 6);
			}
			else
			{
				glTexStorage2D(target_type_, num_mip_maps_, glinternalFormat, s0, s0);
			}

			for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
			{
				for (int face = 0; face < 6; ++ face)
				{
					for (uint32_t level = 0; level < num_mip_maps_; ++ level)
					{
						uint32_t const s = this->Width(level);

						if (IsCompressedFormat(format_))
						{
							uint32_t const block_size = NumFormatBytes(format_) * 4;
							GLsizei const image_size = ((s + 3) / 4) * ((s + 3) / 4) * block_size;

							if (init_data.empty())
							{
								tex_data_[(array_index * 6 + face) * num_mip_maps_ + level].resize(image_size, 0);
							}
							else
							{
								GLvoid const * data = init_data[(array_index * 6 + face) * num_mip_maps_ + level].data;
								tex_data_[(array_index * 6 + face) * num_mip_maps_ + level].resize(image_size);
								std::memcpy(&tex_data_[(array_index * 6 + face) * num_mip_maps_ + level][0],
									data, image_size);

								if (array_size_ > 1)
								{
									glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, array_index,
										s, s, 1, glformat, image_size, data);
								}
								else
								{
									glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0,
										s, s, glformat, image_size, data);
								}
							}
						}
						else
						{
							GLsizei const image_size = s * s * texel_size;

							if (init_data.empty())
							{
								tex_data_[array_index * num_mip_maps_ + level].resize(image_size, 0);
							}
							else
							{
								GLvoid const * data = init_data[(array_index * 6 + face) * num_mip_maps_ + level].data;
								tex_data_[array_index * num_mip_maps_ + level].resize(image_size);
								std::memcpy(&tex_data_[array_index * num_mip_maps_ + level][0],
									data, image_size);

								if (array_size_ > 1)
								{
									glTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, array_index, s, s, 1,
										glformat, gltype, data);
								}
								else
								{
									glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, s, s, glformat, gltype, data);
								}
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
				for (int face = 0; face < 6; ++ face)
				{
					for (uint32_t level = 0; level < num_mip_maps_; ++ level)
					{
						uint32_t const s = this->Width(level);

						if (IsCompressedFormat(format_))
						{
							uint32_t const block_size = NumFormatBytes(format_) * 4;
							GLsizei const image_size = ((s + 3) / 4) * ((s + 3) / 4) * block_size;

							void* ptr;
							if (init_data.empty())
							{
								tex_data_[(array_index * 6 + face) * num_mip_maps_ + level].resize(image_size, 0);
								ptr = nullptr;
							}
							else
							{
								tex_data_[(array_index * 6 + face) * num_mip_maps_ + level].resize(image_size);
								std::memcpy(&tex_data_[(array_index * 6 + face) * num_mip_maps_ + level][0],
									init_data[(array_index * 6 + face) * num_mip_maps_ + level].data, image_size);
								ptr = &tex_data_[(array_index * 6 + face) * num_mip_maps_ + level][0];
							}

							if (array_size_ > 1)
							{
								if (0 == array_index)
								{
									glCompressedTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
										s, s, array_size_, 0, image_size * array_size_, nullptr);
								}

								if (!init_data.empty())
								{
									glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, array_index, s, s, 1,
										glformat, image_size, init_data[(array_index * 6 + face) * num_mip_maps_ + level].data);
								}
							}
							else
							{
								glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
									s, s, 0, image_size, ptr);
							}
						}
						else
						{
							GLsizei const image_size = s * s * texel_size;

							void* ptr;
							if (init_data.empty())
							{
								tex_data_[(array_index * 6 + face) * num_mip_maps_ + level].resize(image_size, 0);
								ptr = nullptr;
							}
							else
							{
								tex_data_[(array_index * 6 + face) * num_mip_maps_ + level].resize(image_size);
								std::memcpy(&tex_data_[(array_index * 6 + face) * num_mip_maps_ + level][0],
									init_data[face * num_mip_maps_ + level].data, image_size);
								ptr = &tex_data_[(array_index * 6 + face) * num_mip_maps_ + level][0];
							}

							if (array_size_ > 1)
							{
								if (0 == array_index)
								{
									glTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat, s, s, array_size_, 0, glformat, gltype, nullptr);
								}

								if (!init_data.empty())
								{
									glTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, array_index, s, s, 1,
										glformat, gltype, init_data[(array_index * 6 + face) * num_mip_maps_ + level].data);
								}
							}
							else
							{
								glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, glinternalFormat,
									s, s, 0, glformat, gltype, ptr);
							}
						}
					}
				}
			}
		}

		hw_res_ready_ = true;
	}

	void OGLESTextureCube::UpdateSubresourceCube(uint32_t array_index, Texture::CubeFaces face, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLESMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		re.BindTexture(0, target_type_, texture_);

		if (IsCompressedFormat(format_))
		{
			GLsizei const image_size = row_pitch * ((height + 3) / 4);

			if (array_size_ > 1)
			{
				glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, x_offset, y_offset, array_index,
					width, height, 1, gl_format, image_size, data);
			}
			else
			{
				glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, x_offset, y_offset,
					width, height, gl_format, image_size, data);
			}
		}
		else
		{
			glPixelStorei(GL_UNPACK_ROW_LENGTH, row_pitch / NumFormatBytes(format_));
			if (array_size_ > 1)
			{
				glTexSubImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, x_offset, y_offset, array_index, width, height, 1,
					gl_format, gl_type, data);
			}
			else
			{
				glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, x_offset, y_offset, width, height,
					gl_format, gl_type, data);
			}
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}
	}
}
