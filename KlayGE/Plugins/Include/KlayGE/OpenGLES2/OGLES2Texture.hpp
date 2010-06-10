// OGLES2Texture.hpp
// KlayGE OpenGL ES 2纹理类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2TEXTURE_HPP
#define _OGLES2TEXTURE_HPP

#pragma once

#include <KlayGE/Texture.hpp>

#include <glloader/glloader.h>

namespace KlayGE
{
	class OGLES2Texture : public Texture
	{
	public:
		OGLES2Texture(TextureType type, uint32_t array_size, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
		virtual ~OGLES2Texture();

		std::wstring const & Name() const;

		virtual void CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset);
		virtual void CopyToTexture2D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);
		virtual void CopyToTexture3D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			uint32_t src_xOffset, uint32_t src_yOffset, uint32_t src_zOffset);
		virtual void CopyToTextureCube(Texture& target, CubeFaces face, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);

		void BuildMipSubLevels();

		void GLBindTexture();
		GLuint GLTexture() const
		{
			return texture_;
		}
		GLenum GLType() const
		{
			return target_type_;
		}

	private:
		virtual void Map1D(int level, TextureMapAccess tma,
			uint32_t width, uint32_t x_offset,
			void*& data);
		virtual void Map2D(int level, TextureMapAccess tma,
			uint32_t width, uint32_t height, uint32_t x_offset, uint32_t y_offset,
			void*& data, uint32_t& row_pitch);
		virtual void Map3D(int level, TextureMapAccess tma,
			uint32_t width, uint32_t height, uint32_t depth,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch);
		virtual void MapCube(CubeFaces face, int level, TextureMapAccess tma,
			uint32_t width, uint32_t height, uint32_t x_offset, uint32_t y_offset,
			void*& data, uint32_t& row_pitch);

		virtual void Unmap1D(int level);
		virtual void Unmap2D(int level);
		virtual void Unmap3D(int level);
		virtual void UnmapCube(CubeFaces face, int level);

	protected:
		ElementFormat SRGBToRGB(ElementFormat pf);

	protected:
		GLuint texture_;
		GLenum target_type_;
		TextureMapAccess last_tma_;
		std::vector<std::vector<uint8_t> > tex_data_;
	};

	typedef boost::shared_ptr<OGLES2Texture> OGLES2TexturePtr;


	class OGLES2Texture1D : public OGLES2Texture
	{
	public:
		OGLES2Texture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset);

	private:
		void Map1D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width, void*& data);
		void Unmap1D(int level);

	private:
		std::vector<uint32_t> widthes_;
	};

	class OGLES2Texture2D : public OGLES2Texture
	{
	public:
		OGLES2Texture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture2D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);
		void CopyToTextureCube(Texture& target, CubeFaces face, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);

	private:
		void Map2D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void Unmap2D(int level);

	private:
		std::vector<uint32_t> widthes_;
		std::vector<uint32_t> heights_;
	};

	class OGLES2Texture3D : public OGLES2Texture
	{
	public:
		OGLES2Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture3D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			uint32_t src_xOffset, uint32_t src_yOffset, uint32_t src_zOffset);

	private:
		void Map3D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch);
		void Unmap3D(int level);

	private:
		std::vector<uint32_t> widthes_;
		std::vector<uint32_t> heights_;
		std::vector<uint32_t> depthes_;
	};

	class OGLES2TextureCube : public OGLES2Texture
	{
	public:
		OGLES2TextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTextureCube(Texture& target, CubeFaces face, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);

	private:
		void MapCube(CubeFaces face, int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void UnmapCube(CubeFaces face, int level);

	private:
		std::vector<uint32_t> widthes_;
	};
}

#endif			// _OGLES2TEXTURE_HPP
