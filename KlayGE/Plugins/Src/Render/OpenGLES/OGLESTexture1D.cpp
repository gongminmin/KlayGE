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
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
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
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
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
		array_size_ = 1;

		uint32_t texel_size = NumFormatBytes(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLESMapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		glGenTextures(1, &texture_);
		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target_type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);		
		if (glloader_GLES_VERSION_3_0())
		{
			glTexParameteri(target_type_, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);
		}
		else if (glloader_GLES_APPLE_texture_max_level())
		{
			glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL_APPLE, num_mip_maps_ - 1);
		}

		tex_data_.resize(num_mip_maps_);
		widths_.resize(num_mip_maps_);
		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			widths_[level] = width;

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

				GLsizei const image_size = ((width + 3) / 4) * block_size;

				if (nullptr == init_data)
				{
					tex_data_[level].resize(image_size, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
					std::memcpy(&tex_data_[level][0], init_data[level].data, image_size);
				}
				glCompressedTexImage2D(target_type_, level, glinternalFormat,
					width, 1, 0, image_size, &tex_data_[level][0]);
			}
			else
			{
				GLsizei const image_size = width * texel_size;

				if (nullptr == init_data)
				{
					tex_data_[level].resize(image_size, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
					std::memcpy(&tex_data_[level][0], init_data[level].data, image_size);
				}
				glTexImage2D(target_type_, level, glinternalFormat, width, 1, 0, glformat, gltype, &tex_data_[level][0]);
			}

			width = std::max(1U, width / 2);
		}
	}

	uint32_t OGLESTexture1D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return widths_[level];
	}

	void OGLESTexture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && (widths_[0] == target.Width(0)))
		{
			uint32_t texel_size = NumFormatBytes(format_);

			GLint gl_internalFormat;
			GLenum gl_format;
			GLenum gl_type;
			OGLESMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

			OGLESTexture1D& gles_target = *checked_cast<OGLESTexture1D*>(&target);
			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				glBindTexture(target_type_, gles_target.GLTexture());

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

					std::memcpy(&gles_target.tex_data_[level][0], &tex_data_[level][0], image_size);
					glCompressedTexSubImage2D(target_type_, level, 0, 0,
						this->Width(level), 1, gl_format, image_size, &tex_data_[level][0]);
				}
				else
				{
					GLsizei const image_size = target.Width(level) * texel_size;

					std::memcpy(&gles_target.tex_data_[level][0], &tex_data_[level][0], image_size);
					glTexSubImage2D(target_type_, level, 0, 0, this->Width(level), 1, gl_format, gl_type, &tex_data_[level][0]);
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

	void OGLESTexture1D::CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		BOOST_ASSERT(type_ == target.Type());
		
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

	void OGLESTexture1D::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t /*width*/, void*& data)
	{
		BOOST_ASSERT(0 == array_index);
		UNREF_PARAM(array_index);

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

		uint8_t* p = &tex_data_[level][0];
		if (IsCompressedFormat(format_))
		{
			data = p + (x_offset / 4 * block_size);
		}
		else
		{
			data = p + x_offset * texel_size;
		}
	}

	void OGLESTexture1D::Unmap1D(uint32_t array_index, uint32_t level)
	{
		BOOST_ASSERT(0 == array_index);
		UNREF_PARAM(array_index);

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

					GLsizei const image_size = ((widths_[level] + 3) / 4) * block_size;

					glCompressedTexSubImage2D(target_type_, level, 0, 0,
						widths_[level], 1, gl_format, image_size, &tex_data_[level][0]);
				}
				else
				{
					glTexSubImage2D(target_type_, level, 0, 0, widths_[level], 1,
							gl_format, gl_type, &tex_data_[level][0]);
				}
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}
}
