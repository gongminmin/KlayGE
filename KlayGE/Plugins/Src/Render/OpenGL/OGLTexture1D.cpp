// OGLTexture1D.hpp
// KlayGE OpenGL 1D纹理类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Context.hpp>

#include <cstring>

#include <glloader/glloader.h>
#include <gl/glu.h>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

namespace KlayGE
{
	OGLTexture1D::OGLTexture1D(uint32_t width, uint16_t numMipMaps,
								PixelFormat format)
					: OGLTexture(TT_1D)
	{
		if (!glloader_GL_EXT_texture_sRGB())
		{
			format = this->SRGBToRGB(format);
		}

		format_		= format;

		if (0 == numMipMaps)
		{
			while (width > 1)
			{
				++ numMipMaps_;

				width /= 2;
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		bpp_ = PixelFormatBits(format_);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		this->Convert(glinternalFormat, glformat, gltype, format_);

		glGenTextures(1, &texture_);
		glBindTexture(GL_TEXTURE_1D, texture_);

		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glTexImage1D(GL_TEXTURE_1D, level, glinternalFormat,
				width, 0, glformat, gltype, NULL);

			width /= 2;
		}

		this->UpdateParams();
	}

	uint32_t OGLTexture1D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return static_cast<GLint>(widths_[level]);
	}

	void OGLTexture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		OGLTexture1D& other = static_cast<OGLTexture1D&>(target);

		GLint gl_internalFormat;
		GLenum gl_format;
		GLenum gl_type;
		this->Convert(gl_internalFormat, gl_format, gl_type, format_);

		GLint gl_target_internal_format;
		GLenum gl_target_format;
		GLenum gl_target_type;
		this->Convert(gl_target_internal_format, gl_target_format, gl_target_type, target.Format());

		std::vector<uint8_t> data_in;
		std::vector<uint8_t> data_out;
		for (int level = 0; level < numMipMaps_; ++ level)
		{
			data_in.resize(this->Width(level) * bpp_ / 8);
			data_out.resize(target.Width(level) * target.Bpp() / 8);

			this->CopyToMemory1D(level, &data_in[0]);

			gluScaleImage(gl_format, this->Width(level), 1, GL_UNSIGNED_BYTE, &data_in[0],
				other.Width(0), 1, gl_type, &data_out[0]);

			other.CopyMemoryToTexture1D(level, &data_out[0], format_,
				target.Width(level), 0, target.Width(level));
		}
	}

	void OGLTexture1D::CopyToMemory1D(int level, void* data)
	{
		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		this->Convert(glinternalFormat, glformat, gltype, format_);

		glBindTexture(GL_TEXTURE_1D, texture_);

		if (IsCompressedFormat(format_))
		{
			glGetCompressedTexImage(GL_TEXTURE_1D, level, data);
		}
		else
		{
			glGetTexImage(GL_TEXTURE_1D, level, glformat, gltype, data);
		}
	}

	void OGLTexture1D::CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
		uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width)
	{
		BOOST_ASSERT(src_width != 0);
		BOOST_ASSERT(dst_width != 0);

		GLint glinternalFormat;
		GLenum glformat;
		GLenum gltype;
		this->Convert(glinternalFormat, glformat, gltype, pf);

		glBindTexture(GL_TEXTURE_1D, texture_);

		std::vector<uint8_t> tmp;
		if (dst_width != src_width)
		{
			gluScaleImage(glformat, src_width, 1, gltype, data,
				dst_width, 1, gltype, &tmp[0]);
			data = &tmp[0];
		}

		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (PF_DXT1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			GLsizei const image_size = ((dst_width + 3) / 4) * block_size;

			glCompressedTexSubImage1D(GL_TEXTURE_1D, level, dst_xOffset,
				dst_width, glformat, image_size, data);
		}
		else
		{
			glTexSubImage1D(GL_TEXTURE_1D, level, dst_xOffset,
				dst_width, glformat, gltype, data);
		}
	}

	void OGLTexture1D::UpdateParams()
	{
		GLint w;

		widths_.resize(numMipMaps_);
		
		glBindTexture(GL_TEXTURE_1D, texture_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			glGetTexLevelParameteriv(GL_TEXTURE_1D, level, GL_TEXTURE_WIDTH, &w);
			widths_[level] = w;
		}
	}
}
