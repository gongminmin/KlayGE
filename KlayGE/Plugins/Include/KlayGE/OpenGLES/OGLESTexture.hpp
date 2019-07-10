// OGLESTexture.hpp
// KlayGE OpenGL ES纹理类 头文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESTEXTURE_HPP
#define _OGLESTEXTURE_HPP

#pragma once

#include <KlayGE/Texture.hpp>

#include <map>

#include <glloader/glloader.h>

namespace KlayGE
{
	class OGLESTexture : public Texture
	{
	public:
		OGLESTexture(TextureType type, uint32_t array_size, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
		~OGLESTexture() override;

		std::wstring const & Name() const override;

		uint32_t Width(uint32_t level) const override;
		uint32_t Height(uint32_t level) const override;
		uint32_t Depth(uint32_t level) const override;

		void CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width) override;
		void CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
			uint32_t src_width, uint32_t src_height) override;
		void CopyToSubTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth) override;
		void CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
			uint32_t src_width, uint32_t src_height) override;

		void BuildMipSubLevels() override;

		GLuint GLTexture() const
		{
			return texture_;
		}
		GLenum GLType() const
		{
			return target_type_;
		}

		void TexParameteri(GLenum pname, GLint param);
		void TexParameterf(GLenum pname, GLfloat param);

		void DeleteHWResource() override;
		bool HWResourceReady() const override;

		void UpdateSubresource1D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t width,
			void const * data) override;
		void UpdateSubresource2D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void const * data, uint32_t row_pitch) override;
		void UpdateSubresource3D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void const * data, uint32_t row_pitch, uint32_t slice_pitch) override;
		void UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void const * data, uint32_t row_pitch) override;

	private:
		void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width,
			void*& data) override;
		void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch) override;
		void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch) override;
		void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch) override;

		void Unmap1D(uint32_t array_index, uint32_t level) override;
		void Unmap2D(uint32_t array_index, uint32_t level) override;
		void Unmap3D(uint32_t array_index, uint32_t level) override;
		void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level) override;

	protected:
		ElementFormat SRGBToRGB(ElementFormat pf);

	protected:
		GLuint texture_;
		GLenum target_type_;
		TextureMapAccess last_tma_;
		std::vector<std::vector<uint8_t>> tex_data_;

		std::map<GLenum, GLint> tex_param_i_;
		std::map<GLenum, GLfloat> tex_param_f_;

		bool hw_res_ready_;
	};

	typedef std::shared_ptr<OGLESTexture> OGLES2TexturePtr;


	class OGLESTexture1D : public OGLESTexture
	{
	public:
		OGLESTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const override;

		void CopyToTexture(Texture& target) override;
		void CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

		void UpdateSubresource1D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t width,
			void const * data) override;

	private:
		void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width, void*& data) override;
		void Unmap1D(uint32_t array_index, uint32_t level) override;

	private:
		uint32_t width_;
	};

	class OGLESTexture2D : public OGLESTexture
	{
	public:
		OGLESTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const override;
		uint32_t Height(uint32_t level) const override;

		void CopyToTexture(Texture& target) override;
		void CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
			uint32_t src_width, uint32_t src_height) override;
		void CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
			uint32_t src_width, uint32_t src_height) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

		void UpdateSubresource2D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void const * data, uint32_t row_pitch) override;

	private:
		void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch) override;
		void Unmap2D(uint32_t array_index, uint32_t level) override;

	private:
		uint32_t width_;
		uint32_t height_;
	};

	class OGLESTexture3D : public OGLESTexture
	{
	public:
		OGLESTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const override;
		uint32_t Height(uint32_t level) const override;
		uint32_t Depth(uint32_t level) const override;

		void CopyToTexture(Texture& target) override;
		void CopyToSubTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

		void UpdateSubresource3D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void const * data, uint32_t row_pitch, uint32_t slice_pitch) override;

	private:
		void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch) override;
		void Unmap3D(uint32_t array_index, uint32_t level) override;

	private:
		uint32_t width_;
		uint32_t height_;
		uint32_t depth_;
	};

	class OGLESTextureCube : public OGLESTexture
	{
	public:
		OGLESTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const override;
		uint32_t Height(uint32_t level) const override;

		void CopyToTexture(Texture& target) override;
		void CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level,
			uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level,
			uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height) override;
		void CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
			uint32_t src_width, uint32_t src_height) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

		void UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void const * data, uint32_t row_pitch) override;

	private:
		void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch) override;
		void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level) override;

	private:
		uint32_t width_;
	};
}

#endif			// _OGLESTEXTURE_HPP
