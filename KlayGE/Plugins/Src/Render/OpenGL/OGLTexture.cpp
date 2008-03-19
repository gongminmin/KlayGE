// OGLTexture.hpp
// KlayGE OpenGL纹理类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2003-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 用pbo加速 (2007.3.13)
//
// 2.7.0
// 增加了AddressingMode, Filtering和Anisotropy (2005.6.27)
// 增加了MaxMipLevel和MipMapLodBias (2005.6.28)
//
// 2.3.0
// 增加了CopyToMemory (2005.2.6)
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
#include <gl/glu.h>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#endif

namespace KlayGE
{
	OGLTexture::OGLTexture(TextureType type)
					: Texture(type)
	{
		switch (type_)
		{
		case TT_1D:
			target_type_ = GL_TEXTURE_1D;
			break;

		case TT_2D:
			target_type_ = GL_TEXTURE_2D;
			break;

		case TT_3D:
			target_type_ = GL_TEXTURE_3D;
			break;

		case TT_Cube:
			target_type_ = GL_TEXTURE_CUBE_MAP;
			break;

		default:
			BOOST_ASSERT(false);
			target_type_ = GL_TEXTURE_1D;
			break;
		}
	}

	OGLTexture::~OGLTexture()
	{
		glDeleteBuffers(static_cast<GLsizei>(pbos_.size()), &pbos_[0]);
		glDeleteTextures(1, &texture_);
	}

	std::wstring const & OGLTexture::Name() const
	{
		static const std::wstring name(L"OpenGL Texture");
		return name;
	}

	uint32_t OGLTexture::Width(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	uint32_t OGLTexture::Height(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	uint32_t OGLTexture::Depth(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	void OGLTexture::CopyToTexture1D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_xOffset*/, uint32_t /*src_width*/, uint32_t /*src_xOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToTexture2D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToTexture3D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_zOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/,
			uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/, uint32_t /*src_zOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToTextureCube(Texture& /*target*/, CubeFaces /*face*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Map1D(int /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*width*/, void*& /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Map2D(int /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/,
		uint32_t /*width*/, uint32_t /*height*/,
		void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Map3D(int /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
		uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
		void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::MapCube(CubeFaces /*face*/, int /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
		void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Unmap1D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Unmap2D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Unmap3D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::UnmapCube(CubeFaces /*face*/, int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::BuildMipSubLevels()
	{
		if (glloader_GL_EXT_framebuffer_object())
		{
			this->GLBindTexture();
			glGenerateMipmapEXT(target_type_);
		}
		else
		{
			THR(E_FAIL);
		}
	}

	void OGLTexture::GLBindTexture()
	{
		glBindTexture(target_type_, texture_);
	}

	void OGLTexture::Usage(Texture::TextureUsage usage)
	{
		usage_ = usage;
	}

	ElementFormat OGLTexture::SRGBToRGB(ElementFormat pf)
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
