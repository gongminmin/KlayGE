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
	OGLTexture3D::OGLTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: OGLTexture(TT_3D, array_size, sample_count, sample_quality, access_hint)
	{
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

		width_ = width;
		height_ = height;
		depth_ = depth;

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
			uint32_t const d = this->Depth(level);

			GLsizei image_size;
			if (IsCompressedFormat(format_))
			{
				uint32_t const block_size = NumFormatBytes(format_) * 4;
				image_size = ((w + 3) / 4) * ((h + 3) / 4) * d * block_size;
			}
			else
			{
				uint32_t const texel_size = NumFormatBytes(format_);
				image_size = w * h * d * texel_size;
			}

			mipmap_start_offset_[level + 1] = mipmap_start_offset_[level] + image_size;
		}
	}

	uint32_t OGLTexture3D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t OGLTexture3D::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, height_ >> level);
	}

	uint32_t OGLTexture3D::Depth(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, depth_ >> level);
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
		KFL_UNUSED(dst_depth);

		BOOST_ASSERT(type_ == target.Type());
		BOOST_ASSERT(0 == src_array_index);
		BOOST_ASSERT(0 == dst_array_index);

		if ((format_ == target.Format()) && !IsCompressedFormat(format_) && (glloader_GL_VERSION_4_3() || glloader_GL_ARB_copy_image())
			&& (src_width == dst_width) && (src_height == dst_height) && (src_depth == dst_depth) && (1 == sample_count_))
		{
			OGLTexture& ogl_target = checked_cast<OGLTexture&>(target);
			glCopyImageSubData(
				texture_, target_type_, src_level,
				src_x_offset, src_y_offset, src_z_offset,
				ogl_target.GLTexture(), ogl_target.GLType(), dst_level,
				dst_x_offset, dst_y_offset, dst_z_offset, src_width, src_height, src_depth);
		}
		else
		{
			if (!IsCompressedFormat(format_) && (glloader_GL_ARB_texture_rg() || (4 == NumComponents(format_))))
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				GLuint fbo_src, fbo_dst;
				re.GetFBOForBlit(fbo_src, fbo_dst);

				GLuint old_fbo = re.BindFramebuffer();

				for (uint32_t depth = 0; depth < src_depth; ++ depth)
				{
					glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_src);
					glFramebufferTexture3D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target_type_, texture_, src_level, src_z_offset + depth);

					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_dst);
					glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target_type_,
						checked_cast<OGLTexture&>(target).GLTexture(), dst_level, dst_z_offset + depth);

					glBlitFramebuffer(src_x_offset, src_y_offset, src_x_offset + src_width, src_y_offset + src_height,
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
								std::memcpy(d, s, src_width * format_size);

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
		KFL_UNUSED(array_index);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);

		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);
		uint32_t const w = this->Width(level);
		uint32_t const h = this->Height(level);
		uint32_t const d = this->Depth(level);

		row_pitch = w * texel_size;
		slice_pitch = row_pitch * h;

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
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, pbo_);

				re.BindTexture(0, target_type_, texture_);
				glGetTexImage(target_type_, level, gl_format, gl_type, reinterpret_cast<GLvoid*>(subres_offset));

				p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
			}
			break;

		case TMA_Write_Only:
			{
				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
				p = static_cast<uint8_t*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid texture map access mode");
		}

		p += subres_offset;
		data = p + ((z_offset * d + y_offset) * w + x_offset) * texel_size;
	}

	void OGLTexture3D::Unmap3D(uint32_t array_index, uint32_t level)
	{
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		uint32_t const w = this->Width(level);
		uint32_t const h = this->Height(level);
		uint32_t const d = this->Depth(level);

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

				re.BindTexture(0, target_type_, texture_);

				re.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

				GLvoid* offset = reinterpret_cast<GLvoid*>(
					static_cast<GLintptr>(array_index * mipmap_start_offset_.back() + mipmap_start_offset_[level]));
				if (IsCompressedFormat(format_))
				{
					uint32_t const block_size = NumFormatBytes(format_) * 4;
					GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * d * block_size;

					glCompressedTexSubImage3D(target_type_, level, 0, 0, 0,
							w, h, d, gl_format, image_size,
							offset);
				}
				else
				{
					glTexSubImage3D(target_type_, level, 0, 0, 0, w, h, d, gl_format, gl_type, offset);
				}
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid texture map access mode");
		}
	}

	void OGLTexture3D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		KFL_UNUSED(clear_value_hint);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		uint32_t const pbo_size = mipmap_start_offset_.back() * array_size_;
		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glTextureParameteri(texture_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);

			glNamedBufferStorage(pbo_, pbo_size, nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

			uint32_t const w0 = this->Width(0);
			uint32_t const h0 = this->Height(0);
			uint32_t const d0 = this->Depth(0);

			glTextureStorage3D(texture_, num_mip_maps_, glinternalFormat, w0, h0, d0);

			if (!init_data.empty())
			{
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					uint32_t const w = this->Width(level);
					uint32_t const h = this->Height(level);
					uint32_t const d = this->Depth(level);
					GLvoid const * data = init_data[level].data;

					if (IsCompressedFormat(format_))
					{
						uint32_t const block_size = NumFormatBytes(format_) * 4;
						GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * d * block_size;

						glCompressedTextureSubImage3D(texture_, level, 0, 0, 0,
								w, h, d, glformat, image_size, data);
					}
					else
					{
						glTextureSubImage3D(texture_, level, 0, 0, 0, w, h, d, glformat, gltype, data);
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
				uint32_t const d0 = this->Depth(0);

				glTexStorage3D(target_type_, num_mip_maps_, glinternalFormat, w0, h0, d0);

				if (!init_data.empty())
				{
					for (uint32_t level = 0; level < num_mip_maps_; ++ level)
					{
						uint32_t const w = this->Width(level);
						uint32_t const h = this->Height(level);
						uint32_t const d = this->Depth(level);
						GLvoid const * data = init_data[level].data;

						if (IsCompressedFormat(format_))
						{
							uint32_t const block_size = NumFormatBytes(format_) * 4;
							GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * d * block_size;

							glCompressedTexSubImage3D(target_type_, level, 0, 0, 0,
									w, h, d, glformat, image_size, data);
						}
						else
						{
							glTexSubImage3D(target_type_, level, 0, 0, 0, w, h, d, glformat, gltype, data);
						}
					}
				}
			}
			else
			{
				for (uint32_t level = 0; level < num_mip_maps_; ++ level)
				{
					uint32_t const w = this->Width(level);
					uint32_t const h = this->Height(level);
					uint32_t const d = this->Depth(level);

					if (IsCompressedFormat(format_))
					{
						uint32_t const block_size = NumFormatBytes(format_) * 4;
						GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * d * block_size;

						glCompressedTexImage3D(target_type_, level, glinternalFormat,
							w, h, d, 0, image_size, init_data.empty() ? nullptr : init_data[level].data);
					}
					else
					{
						glTexImage3D(target_type_, level, glinternalFormat, w, h, d, 0, glformat, gltype,
							init_data.empty() ? nullptr : init_data[level].data);
					}
				}
			}
		}

		hw_res_ready_ = true;
	}

	void OGLTexture3D::UpdateSubresource3D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void const * data, uint32_t row_pitch, uint32_t slice_pitch)
	{
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		re.BindTexture(0, target_type_, texture_);

		if (IsCompressedFormat(format_))
		{
			GLsizei const image_size = slice_pitch * depth;

			glCompressedTexSubImage3D(target_type_, level, x_offset, y_offset, z_offset,
				width, height, depth, gl_format, image_size,
				data);
		}
		else
		{
			glPixelStorei(GL_UNPACK_ROW_LENGTH, row_pitch / NumFormatBytes(format_));
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, slice_pitch / row_pitch);
			glTexSubImage3D(target_type_, level, x_offset, y_offset, z_offset, width, height, depth,
				gl_format, gl_type, data);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
		}
	}
}
