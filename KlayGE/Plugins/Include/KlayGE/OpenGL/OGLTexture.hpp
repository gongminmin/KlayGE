// OGLTexture.hpp
// KlayGE OpenGL纹理类 头文件
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

#ifndef _OGLTEXTURE_HPP
#define _OGLTEXTURE_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_OpenGL
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/Texture.hpp>

#include <windows.h>
#include <glloader/glloader.h>

namespace KlayGE
{
	class OGLTexture : public Texture
	{
	public:
		OGLTexture(TextureType type);
		virtual ~OGLTexture();

		std::wstring const & Name() const;

		virtual uint32_t Width(int level) const;
		virtual uint32_t Height(int level) const;
		virtual uint32_t Depth(int level) const;

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

		using Texture::Usage;
		void Usage(TextureUsage usage);

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
		std::vector<GLuint> pbos_;
		TextureMapAccess last_tma_;
	};

	typedef boost::shared_ptr<OGLTexture> OGLTexturePtr;


	class OGLTexture1D : public OGLTexture
	{
	public:
		OGLTexture1D(uint32_t width, uint16_t numMipMaps, ElementFormat format);

		uint32_t Width(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset);

	private:
		void Map1D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width, void*& data);
		void Unmap1D(int level);

	private:
		void UpdateParams();

	private:
		std::vector<uint32_t> widths_;

		uint32_t last_width_;
		uint32_t last_x_offset_;
	};

	class OGLTexture2D : public OGLTexture
	{
	public:
		OGLTexture2D(uint32_t width, uint32_t height, uint16_t numMipMaps, ElementFormat format);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture2D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);

	private:
		void Map2D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void Unmap2D(int level);

	private:
		void UpdateParams();

	private:
		std::vector<uint32_t> widths_;
		std::vector<uint32_t> heights_;

		uint32_t last_width_, last_height_;
		uint32_t last_x_offset_, last_y_offset_;
	};

	class OGLTexture3D : public OGLTexture
	{
	public:
		OGLTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps, ElementFormat format);

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
		void UpdateParams();

	private:
		std::vector<uint32_t> widths_;
		std::vector<uint32_t> heights_;
		std::vector<uint32_t> depths_;

		uint32_t last_width_, last_height_, last_depth_;
		uint32_t last_x_offset_, last_y_offset_, last_z_offset_;
	};

	class OGLTextureCube : public OGLTexture
	{
	public:
		OGLTextureCube(uint32_t size, uint16_t numMipMaps, ElementFormat format);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;

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
		void UpdateParams();

	private:
		std::vector<uint32_t> widths_;

		uint32_t last_width_, last_height_;
		uint32_t last_x_offset_, last_y_offset_;
	};
}

#endif			// _OGLTEXTURE_HPP
