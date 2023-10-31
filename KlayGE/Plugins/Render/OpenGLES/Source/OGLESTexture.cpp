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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>
#include <system_error>

#include <glloader/glloader.h>

#include "OGLESRenderEngine.hpp"
#include "OGLESUtil.hpp"
#include "OGLESTexture.hpp"

namespace KlayGE
{
	OGLESTexture::OGLESTexture(TextureType type, uint32_t array_size, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: Texture(type, sample_count, sample_quality, access_hint),
						hw_res_ready_(false)
	{
		array_size_ = array_size;

		switch (type_)
		{
		case TT_1D:
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
			KFL_UNREACHABLE("Invalid texture type");
		}

		glGenTextures(1, &texture_);
		glBindTexture(target_type_, texture_);
	}

	OGLESTexture::~OGLESTexture()
	{
		this->DeleteHWResource();

		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeleteTextures(1, &texture_);
		}
		else
		{
			glDeleteTextures(1, &texture_);
		}
	}

	std::wstring const & OGLESTexture::Name() const
	{
		static const std::wstring name(L"OpenGL Texture");
		return name;
	}

	uint32_t OGLESTexture::Width([[maybe_unused]] uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLESTexture::Height([[maybe_unused]] uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLESTexture::Depth([[maybe_unused]] uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	void OGLESTexture::CopyToSubTexture1D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_width,
		[[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] uint32_t src_level, [[maybe_unused]] uint32_t src_x_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] TextureFilter filter)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::CopyToSubTexture2D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_y_offset,
		[[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height, [[maybe_unused]] uint32_t src_array_index,
		[[maybe_unused]] uint32_t src_level, [[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] uint32_t src_height, [[maybe_unused]] TextureFilter filter)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::CopyToSubTexture3D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_y_offset,
		[[maybe_unused]] uint32_t dst_z_offset, [[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height,
		[[maybe_unused]] uint32_t dst_depth, [[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] uint32_t src_level,
		[[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset, [[maybe_unused]] uint32_t src_z_offset,
		[[maybe_unused]] uint32_t src_width, [[maybe_unused]] uint32_t src_height, [[maybe_unused]] uint32_t src_depth,
		[[maybe_unused]] TextureFilter filter)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::CopyToSubTextureCube([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
		[[maybe_unused]] CubeFaces dst_face, [[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset,
		[[maybe_unused]] uint32_t dst_y_offset, [[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height,
		[[maybe_unused]] uint32_t src_array_index, [[maybe_unused]] CubeFaces src_face, [[maybe_unused]] uint32_t src_level,
		[[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset, [[maybe_unused]] uint32_t src_width,
		[[maybe_unused]] uint32_t src_height, [[maybe_unused]] TextureFilter filter)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Map1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t width, [[maybe_unused]] void*& data)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Map2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t width,
		[[maybe_unused]] uint32_t height, [[maybe_unused]] void*& data, [[maybe_unused]] uint32_t& row_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Map3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level, [[maybe_unused]] TextureMapAccess tma,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t z_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth, [[maybe_unused]] void*& data,
		[[maybe_unused]] uint32_t& row_pitch, [[maybe_unused]] uint32_t& slice_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::MapCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] CubeFaces face, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] TextureMapAccess tma, [[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] void*& data,
		[[maybe_unused]] uint32_t& row_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Unmap1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Unmap2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Unmap3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::UnmapCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] CubeFaces face, [[maybe_unused]] uint32_t level)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	bool OGLESTexture::HwBuildMipSubLevels(TextureFilter filter)
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

		auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, target_type_, texture_);
		glGenerateMipmap(target_type_);
		return true;
	}

	void OGLESTexture::TexParameteri(GLenum pname, GLint param)
	{
		auto iter = tex_param_i_.find(pname);
		if ((iter == tex_param_i_.end()) || (iter->second != param))
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, target_type_, texture_);
			glTexParameteri(target_type_, pname, param);

			tex_param_i_[pname] = param;
		}
	}

	void OGLESTexture::TexParameterf(GLenum pname, GLfloat param)
	{
		auto iter = tex_param_f_.find(pname);
		if ((iter == tex_param_f_.end()) || (iter->second != param))
		{
			auto& re = checked_cast<OGLESRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, target_type_, texture_);
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

	void OGLESTexture::DeleteHWResource()
	{
		hw_res_ready_ = false;
	}

	bool OGLESTexture::HWResourceReady() const
	{
		return hw_res_ready_;
	}

	void OGLESTexture::UpdateSubresource1D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t width, [[maybe_unused]] void const* data)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::UpdateSubresource2D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t width,
		[[maybe_unused]] uint32_t height, [[maybe_unused]] void const* data, [[maybe_unused]] uint32_t row_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::UpdateSubresource3D([[maybe_unused]] uint32_t array_index, [[maybe_unused]] uint32_t level,
		[[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset, [[maybe_unused]] uint32_t z_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t depth,
		[[maybe_unused]] void const* data, [[maybe_unused]] uint32_t row_pitch, [[maybe_unused]] uint32_t slice_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::UpdateSubresourceCube([[maybe_unused]] uint32_t array_index, [[maybe_unused]] Texture::CubeFaces face,
		[[maybe_unused]] uint32_t level, [[maybe_unused]] uint32_t x_offset, [[maybe_unused]] uint32_t y_offset,
		[[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] void const* data,
		[[maybe_unused]] uint32_t row_pitch)
	{
		KFL_UNREACHABLE("Can't be called");
	}
}
