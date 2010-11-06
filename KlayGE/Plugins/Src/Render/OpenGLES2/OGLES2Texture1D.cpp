// OGLES2Texture1D.cpp
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
	OGLES2Texture1D::OGLES2Texture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
							uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: OGLES2Texture(TT_1D, array_size, sample_count, sample_quality, access_hint)
	{
		if (IsSRGB(format))
		{
			format = this->SRGBToRGB(format);
		}

		format_ = format;

		if (0 == numMipMaps)
		{
			uint32_t w = width;
			while (w > 1)
			{
				++ numMipMaps_;

				w /= 2;
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
		for (uint32_t level = 0; level < numMipMaps_; ++ level)
		{
			widthes_[level] = width;

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

				GLsizei const image_size = ((width + 3) / 4) * block_size;

				if (NULL == init_data)
				{
					tex_data_[level].resize(image_size, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
					memcpy(&tex_data_[level][0], init_data[level].data, image_size);
				}
				glCompressedTexImage2D(target_type_, level, glinternalFormat,
					width, 1, 0, image_size, &tex_data_[level][0]);
			}
			else
			{
				GLsizei const image_size = width * bpp_ / 8;

				if (NULL == init_data)
				{
					tex_data_[level].resize(image_size, 0);
				}
				else
				{
					tex_data_[level].resize(image_size);
					memcpy(&tex_data_[level][0], init_data[level].data, image_size);
				}
				glTexImage2D(target_type_, level, glinternalFormat, width, 1, 0, glformat, gltype, &tex_data_[level][0]);
			}

			width = std::max(static_cast<uint32_t>(1), width / 2);
		}
	}

	uint32_t OGLES2Texture1D::Width(int level) const
	{
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return widthes_[level];
	}

	uint32_t OGLES2Texture1D::Height(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return 1;
	}

	uint32_t OGLES2Texture1D::Depth(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return 1;
	}

	void OGLES2Texture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((format_ == target.Format()) && (widthes_[0] == target.Width(0)))
		{
			GLint gl_internalFormat;
			GLenum gl_format;
			GLenum gl_type;
			OGLES2Mapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

			OGLES2Texture1D& gles_target = *checked_cast<OGLES2Texture1D*>(&target);
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

					GLsizei const image_size = ((this->Width(level) + 3) / 4) * block_size;

					memcpy(&gles_target.tex_data_[level][0], &tex_data_[level][0], image_size);
					glCompressedTexSubImage2D(target_type_, level, 0, 0,
						this->Width(level), 1, gl_format, image_size, &tex_data_[level][0]);
				}
				else
				{
					GLsizei const image_size = target.Width(level) * bpp_ / 8;

					memcpy(&gles_target.tex_data_[level][0], &tex_data_[level][0], image_size);
					glTexSubImage2D(target_type_, level, 0, 0, this->Width(level), 1, gl_format, gl_type, &tex_data_[level][0]);
				}
			}
		}
		else
		{
			for (uint32_t level = 0; level < numMipMaps_; ++ level)
			{
				this->CopyToTexture1D(target, level, target.Width(level), 0, this->Width(level), 0);
			}
		}
	}

	void OGLES2Texture1D::CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset)
	{
		BOOST_ASSERT(type_ == target.Type());
		
		BOOST_ASSERT(format_ == target.Format());

		OGLES2Texture1D& other = static_cast<OGLES2Texture1D&>(target);

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		OGLES2Mapping::MappingFormat(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		OGLES2Mapping::MappingFormat(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		if (IsCompressedFormat(format_))
		{
			BOOST_ASSERT((0 == src_xOffset) && (0 == dst_xOffset));
			BOOST_ASSERT((src_width == dst_width));

			Texture::Mapper mapper_src(*this, 0, level, TMA_Read_Only, src_xOffset, src_width);
			Texture::Mapper mapper_dst(target, 0, level, TMA_Write_Only, dst_xOffset, dst_width);

			int block_size;
			if (EF_BC1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			GLsizei const image_size = ((dst_width + 3) / 4) * block_size;

			memcpy(mapper_dst.Pointer<uint8_t>(), mapper_src.Pointer<uint8_t>(), image_size);
		}
		else
		{
			size_t const src_format_size = NumFormatBytes(format_);
			size_t const dst_format_size = NumFormatBytes(target.Format());

			if (src_width != dst_width)
			{
				std::vector<uint8_t> data_in(src_width * src_format_size);
				std::vector<uint8_t> data_out(dst_width * dst_format_size);

				{
					Texture::Mapper mapper(*this, 0, level, TMA_Read_Only, src_xOffset, src_width);
					memcpy(&data_in[0], mapper.Pointer<uint8_t*>(), data_in.size() * sizeof(data_in[0]));
				}

				gluScaleImage(gl_format, src_width, 1, gl_type, &data_in[0],
					dst_width, 1, gl_target_type, &data_out[0]);

				{
					Texture::Mapper mapper(other, 0, level, TMA_Write_Only, dst_xOffset, dst_width);
					memcpy(mapper.Pointer<uint8_t*>(), &data_out[0], data_out.size() * sizeof(data_out[0]));
				}
			}
			else
			{
				Texture::Mapper mapper_src(*this, 0, level, TMA_Read_Only, src_xOffset, src_width);
				Texture::Mapper mapper_dst(target, 0, level, TMA_Write_Only, dst_xOffset, dst_width);
				uint8_t const * s = mapper_src.Pointer<uint8_t>();
				uint8_t* d = mapper_dst.Pointer<uint8_t>();
				
				memcpy(d, s, src_width * src_format_size);
			}
		}
	}

	void OGLES2Texture1D::Map1D(int level, TextureMapAccess tma, uint32_t x_offset, uint32_t /*width*/, void*& data)
	{
		last_tma_ = tma;

		uint32_t const size_fmt = NumFormatBytes(format_);
		data = &tex_data_[level][x_offset * size_fmt];
	}

	void OGLES2Texture1D::Unmap1D(int level)
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

					GLsizei const image_size = ((widthes_[level] + 3) / 4) * block_size;

					glCompressedTexSubImage2D(target_type_, level, 0, 0,
						widthes_[level], 1, gl_format, image_size, &tex_data_[level][0]);
				}
				else
				{
					glTexSubImage2D(target_type_, level, 0, 0, widthes_[level], 1,
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
