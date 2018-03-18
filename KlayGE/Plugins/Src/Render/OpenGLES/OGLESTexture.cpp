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

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>

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
			auto& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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

	uint32_t OGLESTexture::Width(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLESTexture::Height(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	uint32_t OGLESTexture::Depth(uint32_t level) const
	{
		KFL_UNUSED(level);
		BOOST_ASSERT(level < num_mip_maps_);

		return 1;
	}

	void OGLESTexture::CopyToSubTexture1D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width)
	{
		KFL_UNUSED(target);
		KFL_UNUSED(dst_array_index);
		KFL_UNUSED(dst_level);
		KFL_UNUSED(dst_x_offset);
		KFL_UNUSED(dst_width);
		KFL_UNUSED(src_array_index);
		KFL_UNUSED(src_level);
		KFL_UNUSED(src_x_offset);
		KFL_UNUSED(src_width);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::CopyToSubTexture2D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_width, uint32_t src_height)
	{
		KFL_UNUSED(target);
		KFL_UNUSED(dst_array_index);
		KFL_UNUSED(dst_level);
		KFL_UNUSED(dst_x_offset);
		KFL_UNUSED(dst_y_offset);
		KFL_UNUSED(dst_width);
		KFL_UNUSED(dst_height);
		KFL_UNUSED(src_array_index);
		KFL_UNUSED(src_level);
		KFL_UNUSED(src_x_offset);
		KFL_UNUSED(src_y_offset);
		KFL_UNUSED(src_width);
		KFL_UNUSED(src_height);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::CopyToSubTexture3D(Texture& target,
		uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		KFL_UNUSED(target);
		KFL_UNUSED(dst_array_index);
		KFL_UNUSED(dst_level);
		KFL_UNUSED(dst_x_offset);
		KFL_UNUSED(dst_y_offset);
		KFL_UNUSED(dst_z_offset);
		KFL_UNUSED(dst_width);
		KFL_UNUSED(dst_height);
		KFL_UNUSED(dst_depth);
		KFL_UNUSED(src_array_index);
		KFL_UNUSED(src_level);
		KFL_UNUSED(src_x_offset);
		KFL_UNUSED(src_y_offset);
		KFL_UNUSED(src_z_offset);
		KFL_UNUSED(src_width);
		KFL_UNUSED(src_height);
		KFL_UNUSED(src_depth);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::CopyToSubTextureCube(Texture& target,
		uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_width, uint32_t src_height)
	{
		KFL_UNUSED(target);
		KFL_UNUSED(dst_array_index);
		KFL_UNUSED(dst_face);
		KFL_UNUSED(dst_level);
		KFL_UNUSED(dst_x_offset);
		KFL_UNUSED(dst_y_offset);
		KFL_UNUSED(dst_width);
		KFL_UNUSED(dst_height);
		KFL_UNUSED(src_array_index);
		KFL_UNUSED(src_face);
		KFL_UNUSED(src_level);
		KFL_UNUSED(src_x_offset);
		KFL_UNUSED(src_y_offset);
		KFL_UNUSED(src_width);
		KFL_UNUSED(src_height);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t width, void*& data)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(data);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset,
		uint32_t width, uint32_t height,
		void*& data, uint32_t& row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(z_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(depth);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);
		KFL_UNUSED(slice_pitch);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void*& data, uint32_t& row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		KFL_UNUSED(tma);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Unmap1D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Unmap2D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::Unmap3D(uint32_t array_index, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::BuildMipSubLevels()
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, target_type_, texture_);
		glGenerateMipmap(target_type_);
	}

	void OGLESTexture::TexParameteri(GLenum pname, GLint param)
	{
		auto iter = tex_param_i_.find(pname);
		if ((iter == tex_param_i_.end()) || (iter->second != param))
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
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

	void OGLESTexture::UpdateSubresource1D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t width,
		void const * data)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(data);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::UpdateSubresource2D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::UpdateSubresource3D(uint32_t array_index, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
		uint32_t width, uint32_t height, uint32_t depth,
		void const * data, uint32_t row_pitch, uint32_t slice_pitch)
	{
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

		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTexture::UpdateSubresourceCube(uint32_t array_index, Texture::CubeFaces face, uint32_t level,
		uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
		void const * data, uint32_t row_pitch)
	{
		KFL_UNUSED(array_index);
		KFL_UNUSED(face);
		KFL_UNUSED(level);
		KFL_UNUSED(x_offset);
		KFL_UNUSED(y_offset);
		KFL_UNUSED(width);
		KFL_UNUSED(height);
		KFL_UNUSED(data);
		KFL_UNUSED(row_pitch);

		KFL_UNREACHABLE("Can't be called");
	}
}
