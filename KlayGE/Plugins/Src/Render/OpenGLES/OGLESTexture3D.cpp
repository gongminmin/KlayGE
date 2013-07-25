// OGLESTexture3D.cpp
// KlayGE OpenGL ES 2 3D纹理类 实现文件
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
	OGLESTexture3D::OGLESTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
					: OGLESTexture(TT_3D, array_size, sample_count, sample_quality, access_hint)
	{
		if (!glloader_GLES_VERSION_3_0() && !glloader_GLES_OES_texture_3D())
		{
			THR(errc::function_not_supported);
		}

		if (IsSRGB(format))
		{
			format = this->SRGBToRGB(format);
		}

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
		heights_.resize(num_mip_maps_);
		depthes_.resize(num_mip_maps_);
		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			widths_[level] = width;
			heights_[level] = height;
			depthes_[level] = depth;

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

				if (nullptr == init_data)
				{
					tex_data_[level].resize(image_size, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
					std::memcpy(&tex_data_[level][0], init_data[level].data, image_size);
				}
				if (glloader_GLES_VERSION_3_0())
				{
					glCompressedTexImage3D(target_type_, level, glinternalFormat,
						width, height, depth, 0, image_size, &tex_data_[level][0]);
				}
				else
				{
					glCompressedTexImage3DOES(target_type_, level, glinternalFormat,
						width, height, depth, 0, image_size, &tex_data_[level][0]);
				}
			}
			else
			{
				GLsizei const image_size = width * height * depth * texel_size;

				if (nullptr == init_data)
				{
					tex_data_[level].resize(image_size, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
					std::memcpy(&tex_data_[level][0], init_data[level].data, image_size);
				}
				if (glloader_GLES_VERSION_3_0())
				{
					glTexImage3D(target_type_, level, glinternalFormat, width, height, depth, 0, glformat, gltype, &tex_data_[level][0]);
				}
				else
				{
					glTexImage3DOES(target_type_, level, glinternalFormat, width, height, depth, 0, glformat, gltype, &tex_data_[level][0]);
				}
			}

			width = std::max(1U, width / 2);
			height = std::max(1U, height / 2);
			depth = std::max(1U, depth / 2);
		}
	}

	uint32_t OGLESTexture3D::Width(uint32_t level) const
	{
		return widths_[level];
	}

	uint32_t OGLESTexture3D::Height(uint32_t level) const
	{
		return heights_[level];
	}

	uint32_t OGLESTexture3D::Depth(uint32_t level) const
	{
		return depthes_[level];
	}

	void OGLESTexture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		for (uint32_t level = 0; level < num_mip_maps_; ++ level)
		{
			this->CopyToSubTexture3D(target,
				0, level, 0, 0, 0, target.Width(level), target.Height(level), target.Depth(level),
				0, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level));
		}
	}

	void OGLESTexture3D::CopyToSubTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		UNREF_PARAM(dst_depth);

		BOOST_ASSERT(type_ == target.Type());
		BOOST_ASSERT(0 == src_array_index);
		BOOST_ASSERT(0 == dst_array_index);

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

	void OGLESTexture3D::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		BOOST_ASSERT(0 == array_index);
		UNREF_PARAM(array_index);

		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);

		row_pitch = widths_[level] * texel_size;
		slice_pitch = row_pitch * heights_[level];

		uint8_t* p = &tex_data_[level][0];		
		data = p + ((z_offset * heights_[level] + y_offset) * widths_[level] + x_offset) * texel_size;
	}

	void OGLESTexture3D::Unmap3D(uint32_t array_index, uint32_t level)
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

					GLsizei const image_size = ((widths_[level] + 3) / 4) * ((heights_[level] + 3) / 4) * depthes_[level] * block_size;

					if (glloader_GLES_VERSION_3_0())
					{
						glCompressedTexSubImage3D(target_type_, level, 0, 0, 0,
							widths_[level], heights_[level], depthes_[level], gl_format, image_size, &tex_data_[level][0]);
					}
					else
					{
						glCompressedTexSubImage3DOES(target_type_, level, 0, 0, 0,
							widths_[level], heights_[level], depthes_[level], gl_format, image_size, &tex_data_[level][0]);
					}
				}
				else
				{
					if (glloader_GLES_VERSION_3_0())
					{
						glTexSubImage3D(target_type_, level,
							0, 0, 0, widths_[level], heights_[level], depthes_[level],
							gl_format, gl_type, &tex_data_[level][0]);
					}
					else
					{
						glTexSubImage3DOES(target_type_, level,
							0, 0, 0, widths_[level], heights_[level], depthes_[level],
							gl_format, gl_type, &tex_data_[level][0]);
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
