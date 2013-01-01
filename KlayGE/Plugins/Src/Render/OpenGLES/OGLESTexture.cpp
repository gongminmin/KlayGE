// OGLESTexture.cpp
// KlayGE OpenGL ES 2 纹理类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>

#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>

namespace KlayGE
{
	OGLESTexture::OGLESTexture(TextureType type, uint32_t array_size, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: Texture(type, sample_count, sample_quality, access_hint)
	{
		if (array_size > 1)
		{
			THR(errc::not_supported);
		}

		switch (type_)
		{
		case TT_1D:
		case TT_2D:
			target_type_ = GL_TEXTURE_2D;
			break;

		case TT_3D:
			if (!glloader_GLES_OES_texture_3D())
			{
				THR(errc::not_supported);
			}
			target_type_ = GL_TEXTURE_3D_OES;
			break;

		case TT_Cube:
			target_type_ = GL_TEXTURE_CUBE_MAP;
			break;

		default:
			BOOST_ASSERT(false);
			target_type_ = GL_TEXTURE_2D;
			break;
		}
	}

	OGLESTexture::~OGLESTexture()
	{
		glDeleteTextures(1, &texture_);
	}

	std::wstring const & OGLESTexture::Name() const
	{
		static const std::wstring name(L"OpenGL Texture");
		return name;
	}

	uint32_t OGLESTexture::Width(uint32_t level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLESTexture::Height(uint32_t level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLESTexture::Depth(uint32_t level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	void OGLESTexture::CopyToSubTexture1D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_width*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_width*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::CopyToSubTexture2D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::CopyToSubTexture3D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_z_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_z_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::CopyToSubTextureCube(Texture& /*target*/,
			uint32_t /*dst_array_index*/, CubeFaces /*dst_face*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
			uint32_t /*src_array_index*/, CubeFaces /*src_face*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::Map1D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*width*/, void*& /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::Map2D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/,
		uint32_t /*width*/, uint32_t /*height*/,
		void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::Map3D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
		uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
		void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::MapCube(uint32_t /*array_index*/, CubeFaces /*face*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
		void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::Unmap1D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::Unmap2D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::Unmap3D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::UnmapCube(uint32_t /*array_index*/, CubeFaces /*face*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLESTexture::BuildMipSubLevels()
	{
		this->GLBindTexture();
		glGenerateMipmap(target_type_);
	}

	void OGLESTexture::GLBindTexture()
	{
		glBindTexture(target_type_, texture_);
	}

	void OGLESTexture::TexParameteri(GLenum pname, GLint param)
	{
		KLAYGE_AUTO(iter, tex_param_i_.find(pname));
		if ((iter == tex_param_i_.end()) || (iter->second != param))
		{
			glBindTexture(target_type_, texture_);
			glTexParameteri(target_type_, pname, param);

			tex_param_i_[pname] = param;
		}
	}

	void OGLESTexture::TexParameterf(GLenum pname, GLfloat param)
	{
		KLAYGE_AUTO(iter, tex_param_f_.find(pname));
		if ((iter == tex_param_f_.end()) || (iter->second != param))
		{
			glBindTexture(target_type_, texture_);
			glTexParameterf(target_type_, pname, param);

			tex_param_f_[pname] = param;
		}
	}

	ElementFormat OGLESTexture::SRGBToRGB(ElementFormat pf)
	{
		switch (pf)
		{
		case EF_ARGB8_SRGB:
			return EF_ARGB8;

		case EF_BC1_SRGB:
			return EF_BC1;

		case EF_BC2_SRGB:
			return EF_BC2;

		case EF_BC3_SRGB:
			return EF_BC3;

		default:
			return pf;
		}
	}
}
