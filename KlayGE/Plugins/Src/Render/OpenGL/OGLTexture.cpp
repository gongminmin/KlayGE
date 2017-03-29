// OGLTexture.cpp
// KlayGE OpenGL������ ʵ���ļ�
// Ver 3.12.0
// ��Ȩ����(C) ������, 2003-2007
// Homepage: http://www.klayge.org
//
// 3.9.0
// ֧��Texture Array (2009.8.5)
//
// 3.6.0
// ��pbo���� (2007.3.13)
//
// 2.7.0
// ������AddressingMode, Filtering��Anisotropy (2005.6.27)
// ������MaxMipLevel��MipMapLodBias (2005.6.28)
//
// 2.3.0
// ������CopyToMemory (2005.2.6)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>

namespace KlayGE
{
	OGLTexture::OGLTexture(TextureType type, uint32_t array_size, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: Texture(type, sample_count, sample_quality, access_hint),
						hw_res_ready_(false)
	{
		array_size_ = array_size;

		switch (type_)
		{
		case TT_1D:
			if (array_size > 1)
			{
				target_type_ = GL_TEXTURE_1D_ARRAY;
			}
			else
			{
				target_type_ = GL_TEXTURE_1D;
			}
			break;

		case TT_2D:
			if (array_size > 1)
			{
				target_type_ = GL_TEXTURE_2D_ARRAY;
			}
			else
			{
				target_type_ = GL_TEXTURE_2D;
			}
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

		if (sample_count_ <= 1)
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glCreateTextures(target_type_, 1, &texture_);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glGenTextures(1, &texture_);
			}
			else
			{
				glGenTextures(1, &texture_);
				glBindTexture(target_type_, texture_);
			}
		}
		else
		{
			glGenRenderbuffers(1, &texture_);
		}
	}

	OGLTexture::~OGLTexture()
	{
		this->DeleteHWResource();

		if (Context::Instance().RenderFactoryValid())
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeleteBuffers(1, &pbo_);
		}
		else
		{
			glDeleteBuffers(1, &pbo_);
		}

		if (sample_count_ <= 1)
		{
			glDeleteTextures(1, &texture_);
		}
		else
		{
			glDeleteRenderbuffers(1, &texture_);
		}
	}

	std::wstring const & OGLTexture::Name() const
	{
		static const std::wstring name(L"OpenGL Texture");
		return name;
	}

	uint32_t OGLTexture::Width(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLTexture::Height(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLTexture::Depth(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	void OGLTexture::CopyToSubTexture1D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_width*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_width*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToSubTexture2D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToSubTexture3D(Texture& /*target*/,
			uint32_t /*dst_array_index*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_z_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*src_array_index*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_z_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::CopyToSubTextureCube(Texture& /*target*/,
			uint32_t /*dst_array_index*/, CubeFaces /*dst_face*/, uint32_t /*dst_level*/, uint32_t /*dst_x_offset*/, uint32_t /*dst_y_offset*/, uint32_t /*dst_width*/, uint32_t /*dst_height*/,
			uint32_t /*src_array_index*/, CubeFaces /*src_face*/, uint32_t /*src_level*/, uint32_t /*src_x_offset*/, uint32_t /*src_y_offset*/, uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Map1D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*width*/, void*& /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Map2D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/,
		uint32_t /*width*/, uint32_t /*height*/,
		void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Map3D(uint32_t /*array_index*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
		uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
		void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::MapCube(uint32_t /*array_index*/, CubeFaces /*face*/, uint32_t /*level*/, TextureMapAccess /*tma*/,
		uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
		void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Unmap1D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Unmap2D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::Unmap3D(uint32_t /*array_index*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::UnmapCube(uint32_t /*array_index*/, CubeFaces /*face*/, uint32_t /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void OGLTexture::BuildMipSubLevels()
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, target_type_, texture_);
		glGenerateMipmap(target_type_);
	}

	void OGLTexture::TexParameteri(GLenum pname, GLint param)
	{
		auto iter = tex_param_i_.find(pname);
		if ((iter == tex_param_i_.end()) || (iter->second != param))
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glTextureParameteri(texture_, pname, param);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glTextureParameteriEXT(texture_, target_type_, pname, param);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindTexture(0, target_type_, texture_);
				glTexParameteri(target_type_, pname, param);
			}

			tex_param_i_[pname] = param;
		}
	}

	void OGLTexture::TexParameterf(GLenum pname, GLfloat param)
	{
		auto iter = tex_param_f_.find(pname);
		if ((iter == tex_param_f_.end()) || (iter->second != param))
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glTextureParameterf(texture_, pname, param);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glTextureParameterfEXT(texture_, target_type_, pname, param);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindTexture(0, target_type_, texture_);
				glTexParameterf(target_type_, pname, param);
			}

			tex_param_f_[pname] = param;
		}
	}

	void OGLTexture::TexParameterfv(GLenum pname, GLfloat const * param)
	{
		float4 const f4_param(param);
		auto iter = tex_param_fv_.find(pname);
		if ((iter == tex_param_fv_.end()) || (iter->second != f4_param))
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glTextureParameterfv(texture_, pname, param);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glTextureParameterfvEXT(texture_, target_type_, pname, param);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindTexture(0, target_type_, texture_);
				glTexParameterfv(target_type_, pname, param);
			}

			tex_param_fv_[pname] = f4_param;
		}
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

	void OGLTexture::DeleteHWResource()
	{
		hw_res_ready_ = false;
	}

	bool OGLTexture::HWResourceReady() const
	{
		return hw_res_ready_;
	}

	void OGLTexture::UpdateSubresource1D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t width,
		void const * data)
	{
		BOOST_ASSERT(false);

		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(data);
	}

	void OGLTexture::UpdateSubresource2D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		BOOST_ASSERT(false);

		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);
	}

	void OGLTexture::UpdateSubresource3D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void const * data, uint32_t row_pitch, uint32_t slice_pitch)
	{
		BOOST_ASSERT(false);

		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(z_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);
		KFL_UNUSED(slice_pitch);
	}

	void OGLTexture::UpdateSubresourceCube(uint32_t array_index, Texture::CubeFaces face, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		BOOST_ASSERT(false);

		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);
	}
}
