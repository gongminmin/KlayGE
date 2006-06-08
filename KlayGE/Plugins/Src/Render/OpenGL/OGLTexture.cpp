// OGLTexture.hpp
// KlayGE OpenGL纹理类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

	void OGLTexture::CopyToMemory1D(int /*level*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToMemory2D(int /*level*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToMemory3D(int /*level*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToMemoryCube(CubeFaces /*face*/, int /*level*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyMemoryToTexture1D(int /*level*/, void* /*data*/, ElementFormat /*pf*/,
		uint32_t /*dst_width*/, uint32_t /*dst_xOffset*/, uint32_t /*src_width*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyMemoryToTexture2D(int /*level*/, void* /*data*/, ElementFormat /*pf*/,
		uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
		uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyMemoryToTexture3D(int /*level*/, void* /*data*/, ElementFormat /*pf*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_zOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyMemoryToTextureCube(CubeFaces /*face*/, int /*level*/, void* /*data*/, ElementFormat /*pf*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/)
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

	void OGLTexture::CustomAttribute(std::string const & /*name*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
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

		case EF_DXT1_SRGB:
			return EF_DXT1;

		case EF_DXT3_SRGB:
			return EF_DXT3;

		case EF_DXT5_SRGB:
			return EF_DXT5;

		default:
			return pf;
		}
	}
}
