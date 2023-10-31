// OGLTexture.cpp
// KlayGE OpenGL纹理类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2003-2007
// Homepage: http://www.klayge.org
//
// 3.9.0
// 支持Texture Array (2009.8.5)
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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <glloader/glloader.h>

#include "OGLRenderEngine.hpp"
#include "OGLUtil.hpp"
#include "OGLTexture.hpp"

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
			if (array_size > 1)
			{
				target_type_ = GL_TEXTURE_CUBE_MAP_ARRAY;
			}
			else
			{
				target_type_ = GL_TEXTURE_CUBE_MAP;
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid texture type");
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
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeleteBuffers(1, &pbo_);
		}
		else
		{
			glDeleteBuffers(1, &pbo_);
		}

		if (sample_count_ <= 1)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteTextures(1, &texture_);
			}
			else
			{
				glDeleteTextures(1, &texture_);
			}
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

	uint32_t OGLTexture::Width([[maybe_unused]] uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLTexture::Height([[maybe_unused]] uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLTexture::Depth([[maybe_unused]] uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	void OGLTexture::CopyToSubTexture1D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_width,
		[[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] uint32_t src_level, [[maybe_unused]] uint32_t src_x_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] TextureFilter filter)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::CopyToSubTexture2D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_y_offset,
		[[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height, [[maybe_unused]] uint32_t src_array_index,
		[[maybe_unused]] uint32_t src_level, [[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] uint32_t src_height, [[maybe_unused]] TextureFilter filter)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::CopyToSubTexture3D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_y_offset,
		[[maybe_unused]] uint32_t dst_z_offset, [[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height,
		[[maybe_unused]] uint32_t dst_depth, [[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] uint32_t src_level,
		[[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset, [[maybe_unused]] uint32_t src_z_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] uint32_t src_height,
		[[maybe_unused]]uint32_t src_depth, [[maybe_unused]] TextureFilter filter)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::CopyToSubTextureCube([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] CubeFaces dst_face, [[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset,
		[[maybe_unused]] uint32_t dst_y_offset, [[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height,
		[[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] CubeFaces src_face, [[maybe_unused]] uint32_t src_level,
		[[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset, [[maybe_unused]] uint32_t src_width,
		[[maybe_unused]] uint32_t src_height, [[maybe_unused]] TextureFilter filter)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::Map1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t width, [[maybe_unused]] void*& data)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::Map2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t width,
		[[maybe_unused]] uint32_t height, [[maybe_unused]] void*& data, [[maybe_unused]] uint32_t& row_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::Map3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t z_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth, [[maybe_unused]] void*& data,
		[[maybe_unused]] uint32_t& row_pitch, [[maybe_unused]] uint32_t& slice_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::MapCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] CubeFaces face, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] TextureMapAccess tma, [[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] void*& data,
		[[maybe_unused]] uint32_t& row_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::Unmap1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::Unmap2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::Unmap3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::UnmapCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] CubeFaces face, [[maybe_unused]] uint32_t level)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	bool OGLTexture::HwBuildMipSubLevels(TextureFilter filter)
	{
		if (IsDepthFormat(format_) || (ChannelType<0>(format_) == ECT_UInt) || (ChannelType<0>(format_) == ECT_SInt))
		{
			if (filter != TextureFilter::Point)
			{
				return false;
			}
		}
		else
		{
			if (filter != TextureFilter::Linear)
			{
				return false;
			}
		}

		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glGenerateTextureMipmap(texture_);
		}
		else if (glloader_GL_EXT_direct_state_access())
		{
			glGenerateTextureMipmapEXT(texture_, target_type_);
		}
		else
		{
			auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, target_type_, texture_);
			glGenerateMipmap(target_type_);
		}
		return true;
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
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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
				auto& re = checked_cast<OGLRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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

	void OGLTexture::UpdateSubresource1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t width, [[maybe_unused]] void const* data)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::UpdateSubresource2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t width,
		[[maybe_unused]] uint32_t height, [[maybe_unused]] void const* data, [[maybe_unused]] uint32_t row_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::UpdateSubresource3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t z_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth,
		[[maybe_unused]] void const* data, [[maybe_unused]] uint32_t row_pitch, [[maybe_unused]] uint32_t slice_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTexture::UpdateSubresourceCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] Texture::CubeFaces face,
		[[maybe_unused]] uint32_t level, [[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] void const* data,
		[[maybe_unused]] uint32_t row_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}
}
