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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>
#include <system_error>

#include <glloader/glloader.h>

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>

namespace KlayGE
{
	OGLESTexture3D::OGLESTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: OGLESTexture(TT_3D, array_size, sample_count, sample_quality, access_hint)
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

		width_ = width;
		height_ = height;
		depth_ = depth;

		tex_data_.resize(num_mip_maps_);

		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(target_type_, GL_TEXTURE_MAX_LEVEL, num_mip_maps_ - 1);
	}

	uint32_t OGLESTexture3D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t OGLESTexture3D::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, height_ >> level);
	}

	uint32_t OGLESTexture3D::Depth(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, depth_ >> level);
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
		KFL_UNUSED(dst_depth);

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

	void OGLESTexture3D::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		last_tma_ = tma;

		uint32_t const texel_size = NumFormatBytes(format_);
		uint32_t const w = this->Width(level);
		uint32_t const h = this->Height(level);

		row_pitch = w * texel_size;
		slice_pitch = row_pitch * h;

		uint8_t* p = &tex_data_[level][0];		
		data = p + ((z_offset * h + y_offset) * w + x_offset) * texel_size;
	}

	void OGLESTexture3D::Unmap3D(uint32_t array_index, uint32_t level)
	{
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

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
				uint32_t const h = this->Height(level);
				uint32_t const d = this->Depth(level);

				auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindTexture(0, target_type_, texture_);

				if (IsCompressedFormat(format_))
				{
					uint32_t const block_size = NumFormatBytes(format_) * 4;
					GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * d * block_size;

					glCompressedTexSubImage3D(target_type_, level, 0, 0, 0,
						w, h, d, gl_format, image_size, &tex_data_[level][0]);
				}
				else
				{
					glTexSubImage3D(target_type_, level,
						0, 0, 0, w, h, d, gl_format, gl_type, &tex_data_[level][0]);
				}
			}
			break;

		default:KFL_UNREACHABLE("Invalid texture map access mode");
		}
	}

	void OGLESTexture3D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
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
			uint32_t const h0 = this->Height(0);
			uint32_t const d0 = this->Depth(0);

			glTexStorage3D(target_type_, num_mip_maps_, glinternalFormat, w0, h0, d0);

			for (uint32_t level = 0; level < num_mip_maps_; ++ level)
			{
				uint32_t const w = this->Width(level);
				uint32_t const h = this->Height(level);
				uint32_t const d = this->Depth(level);

				if (IsCompressedFormat(format_))
				{
					uint32_t const block_size = NumFormatBytes(format_) * 4;
					GLsizei const image_size = ((w + 3) / 4) * ((h + 3) / 4) * d * block_size;

					if (init_data.empty())
					{
						tex_data_[level].resize(image_size, 0);
					}
					else
					{
						GLvoid const * data = init_data[level].data;
						tex_data_[level].resize(image_size);
						std::memcpy(&tex_data_[level][0], data, image_size);

						glCompressedTexSubImage3D(target_type_, level, 0, 0, 0,
							w, h, d, glformat, image_size, data);
					}
				}
				else
				{
					GLsizei const image_size = w * h * d * texel_size;

					if (init_data.empty())
					{
						tex_data_[level].resize(image_size, 0);
					}
					else
					{
						GLvoid const * data = init_data[level].data;
						tex_data_[level].resize(image_size);
						std::memcpy(&tex_data_[level][0], init_data[level].data, image_size);

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

					void* ptr;
					if (init_data.empty())
					{
						tex_data_[level].resize(image_size, 0);
						ptr = nullptr;
					}
					else
					{
						tex_data_[level].resize(image_size);
						std::memcpy(&tex_data_[level][0], init_data[level].data, image_size);
						ptr = &tex_data_[level][0];
					}
					glCompressedTexImage3D(target_type_, level, glinternalFormat,
						w, h, d, 0, image_size, ptr);
				}
				else
				{
					GLsizei const image_size = w * h * d * texel_size;

					void* ptr;
					if (init_data.empty())
					{
						tex_data_[level].resize(image_size, 0);
						ptr = nullptr;
					}
					else
					{
						tex_data_[level].resize(image_size);
						std::memcpy(&tex_data_[level][0], init_data[level].data, image_size);
						ptr = &tex_data_[level][0];
					}
					glTexImage3D(target_type_, level, glinternalFormat, w, h, d, 0, glformat, gltype, ptr);
				}
			}
		}

		hw_res_ready_ = true;
	}

	void OGLESTexture3D::UpdateSubresource3D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void const * data, uint32_t row_pitch, uint32_t slice_pitch)
	{
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLESMapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

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
