// OGLES2Texture3D.cpp
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
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>
#include <GL/glu.h>

#include <KlayGE/OpenGLES2/OGLES2RenderEngine.hpp>
#include <KlayGE/OpenGLES2/OGLES2Mapping.hpp>
#include <KlayGE/OpenGLES2/OGLES2Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "libEGL.lib")
#pragma comment(lib, "libGLESv2.lib")
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	OGLES2Texture3D::OGLES2Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: OGLES2Texture(TT_3D, array_size, sample_count, sample_quality, access_hint)
	{
		if (!glloader_GLES_OES_texture_3D())
		{
			THR(boost::system::posix_error::not_supported);
		}

		if (IsSRGB(format))
		{
			format = this->SRGBToRGB(format);
		}

		format_ = format;

		if (0 == numMipMaps)
		{
			uint32_t w = width;
			uint32_t h = height;
			uint32_t d = depth;
			while ((w > 1) && (h > 1) && (d > 1))
			{
				++ numMipMaps_;

				w = std::max(static_cast<uint32_t>(1), w / 2);
				h = std::max(static_cast<uint32_t>(1), h / 2);
				d = std::max(static_cast<uint32_t>(1), d / 2);
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}
		array_size_ = 1;

		bpp_ = NumFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLES2Mapping::MappingFormat(glinternalFormat, glformat, gltype, format_);

		glGenTextures(1, &texture_);
		glBindTexture(target_type_, texture_);
		glTexParameteri(target_type_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target_type_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		tex_data_.resize(numMipMaps_);
		widthes_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		depthes_.resize(numMipMaps_);
		for (uint32_t level = 0; level < numMipMaps_; ++ level)
		{
			widthes_[level] = width;
			heights_[level] = height;
			depthes_[level] = depth;

			if (IsCompressedFormat(format_))
			{
				int block_size;
				if (EF_BC1 == format_)
				{
					block_size = 8;
				}
				else
				{
					block_size = 16;
				}

				GLsizei const image_size = ((width + 3) / 4) * ((height + 3) / 4) * depth * block_size;

				if (NULL == init_data)
				{
					tex_data_[level].resize(image_size, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
					memcpy(&tex_data_[level][0], init_data[level].data, image_size);
				}
				glCompressedTexImage3DOES(target_type_, level, glinternalFormat,
					width, height, depth, 0, image_size, &tex_data_[level][0]);
			}
			else
			{
				GLsizei const image_size = width * height * depth * bpp_ / 8;

				if (NULL == init_data)
				{
					tex_data_[level].resize(image_size, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
					memcpy(&tex_data_[level][0], init_data[level].data, image_size);
				}
				glTexImage3DOES(target_type_, level, glinternalFormat, width, height, depth, 0, glformat, gltype, &tex_data_[level][0]);
			}

			width = std::max(static_cast<uint32_t>(1), width / 2);
			height = std::max(static_cast<uint32_t>(1), height / 2);
			depth = std::max(static_cast<uint32_t>(1), depth / 2);
		}
	}

	uint32_t OGLES2Texture3D::Width(int level) const
	{
		return widthes_[level];
	}

	uint32_t OGLES2Texture3D::Height(int level) const
	{
		return heights_[level];
	}

	uint32_t OGLES2Texture3D::Depth(int level) const
	{
		return depthes_[level];
	}

	void OGLES2Texture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && (widthes_[0] == target.Width(0)) && (heights_[0] == target.Height(0)) && (depthes_[0] == target.Depth(0)))
		{
			GLint gl_internalFormat;
			GLenum gl_format;
			GLenum gl_type;
			OGLES2Mapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

			OGLES2Texture3D& gles_target = *checked_cast<OGLES2Texture3D*>(&target);
			for (uint32_t level = 0; level < numMipMaps_; ++ level)
			{
				glBindTexture(target_type_, gles_target.GLTexture());

				if (IsCompressedFormat(format_))
				{
					int block_size;
					if (EF_BC1 == format_)
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					GLsizei const image_size = ((this->Width(level) + 3) / 4) * ((this->Height(level) + 3) / 4) * this->Depth(level) * block_size;

					memcpy(&gles_target.tex_data_[level][0], &tex_data_[level][0], image_size);
					glCompressedTexSubImage3DOES(target_type_, level, 0, 0, 0,
						this->Width(level), this->Height(level), this->Depth(level), gl_format, image_size, &tex_data_[level][0]);
				}
				else
				{
					GLsizei const image_size = target.Width(level) * target.Height(level) * target.Depth(level) * bpp_ / 8;

					memcpy(&gles_target.tex_data_[level][0], &tex_data_[level][0], image_size);
					glTexSubImage3DOES(target_type_, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level),
							gl_format, gl_type, &tex_data_[level][0]);
				}
			}
		}
		else
		{
			for (uint32_t level = 0; level < numMipMaps_; ++ level)
			{
				this->CopyToTexture3D(target, level, target.Width(level), target.Height(level), target.Depth(level), 0, 0, 0,
					this->Width(level), this->Height(level), this->Depth(level), 0, 0, 0);
			}
		}
	}

	void OGLES2Texture3D::CopyToTexture3D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			uint32_t src_xOffset, uint32_t src_yOffset, uint32_t src_zOffset)
	{
		UNREF_PARAM(dst_depth);

		BOOST_ASSERT(type_ == target.Type());
		// fix me
		BOOST_ASSERT(src_depth == dst_depth);

		BOOST_ASSERT(format_ == target.Format());

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLES2Mapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		OGLES2Mapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		size_t const src_format_size = NumFormatBytes(format_);
		size_t const dst_format_size = NumFormatBytes(target.Format());

		std::vector<uint8_t> data_in(src_width * src_height * src_format_size);
		std::vector<uint8_t> data_out(dst_width * dst_height * dst_format_size);

		for (uint32_t z = 0; z < src_depth; ++ z)
		{
			{
				Texture::Mapper mapper(*this, level, TMA_Read_Only, src_xOffset, src_yOffset, src_zOffset + z,
					src_width, src_height, 1);
				uint8_t const * s = mapper.Pointer<uint8_t>();
				uint8_t* d = &data_in[0];
				for (uint32_t y = 0; y < src_height; ++ y)
				{
					memcpy(d, s, src_width * src_format_size);

					s += mapper.RowPitch();
					d += src_width * src_format_size;
				}
			}

			gluScaleImage(gl_format, src_width, src_height, gl_type, &data_in[0],
				dst_width, dst_height, gl_target_type, &data_out[0]);

			{
				Texture::Mapper mapper(target, level, TMA_Write_Only, dst_xOffset, dst_yOffset, dst_zOffset + z,
					dst_width, dst_height, 1);
				uint8_t const * s = &data_out[0];
				uint8_t* d = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < src_height; ++ y)
				{
					memcpy(d, s, dst_width * dst_format_size);

					s += src_width * src_format_size;
					d += mapper.RowPitch();
				}
			}
		}
	}

	void OGLES2Texture3D::Map3D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		last_tma_ = tma;

		uint32_t const size_fmt = NumFormatBytes(format_);
		data = &tex_data_[level][((z_offset * heights_[level] + y_offset) * widthes_[level] + x_offset) * size_fmt];
		row_pitch = widthes_[level] * size_fmt;
		slice_pitch = row_pitch * heights_[level];
	}

	void OGLES2Texture3D::Unmap3D(int level)
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
				OGLES2Mapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

				glBindTexture(target_type_, texture_);

				if (IsCompressedFormat(format_))
				{
					int block_size;
					if (EF_BC1 == format_)
					{
						block_size = 8;
					}
					else
					{
						block_size = 16;
					}

					GLsizei const image_size = ((widthes_[level] + 3) / 4) * ((heights_[level] + 3) / 4) * depthes_[level] * block_size;

					glCompressedTexSubImage3DOES(target_type_, level, 0, 0, 0,
							widthes_[level], heights_[level], depthes_[level], gl_format, image_size, &tex_data_[level][0]);
				}
				else
				{
					glTexSubImage3DOES(target_type_, level,
							0, 0, 0, widthes_[level], heights_[level], depthes_[level],
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
