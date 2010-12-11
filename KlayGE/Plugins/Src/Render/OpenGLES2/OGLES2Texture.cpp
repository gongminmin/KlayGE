// OGLES2Texture.cpp
// KlayGE OpenGL ES 2 纹理类 实现文件
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
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>
#include <GL/glu.h>

#include <KlayGE/OpenGLES2/OGLES2Mapping.hpp>
#include <KlayGE/OpenGLES2/OGLES2Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "libEGL.lib")
#pragma comment(lib, "libGLESv2.lib")
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	OGLES2Texture::OGLES2Texture(TextureType type, uint32_t array_size, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: Texture(type, sample_count, sample_quality, access_hint)
	{
		if (array_size > 1)
		{
			THR(boost::system::posix_error::not_supported);
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
				THR(boost::system::posix_error::not_supported);
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

	OGLES2Texture::~OGLES2Texture()
	{
		glDeleteTextures(1, &texture_);
	}

	std::wstring const & OGLES2Texture::Name() const
	{
		static const std::wstring name(L"OpenGL Texture");
		return name;
	}

	void OGLES2Texture::CopyToTexture1D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_xOffset*/, uint32_t /*src_width*/, uint32_t /*src_xOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::CopyToTexture2D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::CopyToTexture3D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_zOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/,
			uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/, uint32_t /*src_zOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::CopyToTextureCube(Texture& /*target*/, CubeFaces /*face*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::CopyToTextureArray(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_array_index*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/, uint32_t /*src_array_index*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::Map1D(int /*array_index*/, int /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*width*/, void*& /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::Map2D(int /*array_index*/, int /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/,
		uint32_t /*width*/, uint32_t /*height*/,
		void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::Map3D(int /*array_index*/, int /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
		uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
		void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::MapCube(int /*array_index*/, CubeFaces /*face*/, int /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
		void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::Unmap1D(int /*array_index*/, int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::Unmap2D(int /*array_index*/, int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::Unmap3D(int /*array_index*/, int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::UnmapCube(int /*array_index*/, CubeFaces /*face*/, int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLES2Texture::BuildMipSubLevels()
	{
		this->GLBindTexture();
		glGenerateMipmap(target_type_);
	}

	void OGLES2Texture::GLBindTexture()
	{
		glBindTexture(target_type_, texture_);
	}

	ElementFormat OGLES2Texture::SRGBToRGB(ElementFormat pf)
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
