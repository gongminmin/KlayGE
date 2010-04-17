// D3D9Texture.hpp
// KlayGE D3D9纹理类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 去掉了ZBuffer (2005.10.12)
//
// 2.7.0
// 增加了AddressingMode, Filtering和Anisotropy (2005.6.27)
// 增加了MaxMipLevel和MipMapLodBias (2005.6.28)
//
// 2.4.0
// 改为派生自D3D9Resource (2005.3.3)
// 增加了D3DBaseTexture (2005.3.7)
// 增加了1D/2D/3D/cube的支持 (2005.3.8)
//
// 2.3.0
// 增加了CopyToMemory (2005.2.6)
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9TEXTURE_HPP
#define _D3D9TEXTURE_HPP

#pragma once

#include <boost/smart_ptr.hpp>

#include <KlayGE/D3D9/D3D9MinGWDefs.hpp>

#include <d3d9.h>
#include <d3dx9.h>

#include <KlayGE/Texture.hpp>
#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>

namespace KlayGE
{
	class D3D9Texture : public Texture, public D3D9Resource
	{
	public:
		explicit D3D9Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
		virtual ~D3D9Texture();

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

		ID3D9BaseTexturePtr D3DBaseTexture() const
			{ return d3dBaseTexture_; }

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
		void CopySurfaceToMemory(ID3D9SurfacePtr const & surface, void* data);
		void CopyVolumeToMemory(ID3D9VolumePtr const & volume, void* data);
		void CopySurfaceToSurface(ID3D9SurfacePtr const & dst, ID3D9SurfacePtr const & src);

	protected:
		ID3D9DevicePtr			d3dDevice_;
		ID3D9BaseTexturePtr	d3dBaseTexture_;
	};

	typedef boost::shared_ptr<D3D9Texture> D3D9TexturePtr;


	class D3D9Texture1D : public D3D9Texture
	{
	public:
		D3D9Texture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset);

		void BuildMipSubLevels();

		ID3D9TexturePtr D3DTexture1D() const
			{ return d3dTexture1D_; }

	private:
		void Map1D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width, void*& data);
		void Unmap1D(int level);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void QueryBaseTexture();
		void UpdateParams();

		ID3D9TexturePtr CreateTexture1D(uint32_t usage, D3DPOOL pool);

	private:
		ID3D9TexturePtr			d3dTexture1D_;

		std::vector<uint32_t>	widths_;

		bool auto_gen_mipmaps_;
	};

	class D3D9Texture2D : public D3D9Texture
	{
	public:
		D3D9Texture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

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

		void BuildMipSubLevels();

		ID3D9TexturePtr D3DTexture2D() const
			{ return d3dTexture2D_; }

	private:
		void Map2D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void Unmap2D(int level);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void QueryBaseTexture();
		void UpdateParams();

		ID3D9TexturePtr CreateTexture2D(uint32_t usage, D3DPOOL pool);

	private:
		ID3D9TexturePtr			d3dTexture2D_;

		std::vector<uint32_t>	widths_;
		std::vector<uint32_t>	heights_;

		bool auto_gen_mipmaps_;
	};

	class D3D9Texture3D : public D3D9Texture
	{
	public:
		D3D9Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture3D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			uint32_t src_xOffset, uint32_t src_yOffset, uint32_t src_zOffset);

		void BuildMipSubLevels();

		ID3D9VolumeTexturePtr D3DTexture3D() const
			{ return d3dTexture3D_; }

	private:
		void Map3D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch);
		void Unmap3D(int level);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void QueryBaseTexture();
		void UpdateParams();

		ID3D9VolumeTexturePtr CreateTexture3D(uint32_t usage, D3DPOOL pool);

	private:
		ID3D9VolumeTexturePtr	d3dTexture3D_;

		std::vector<uint32_t>	widths_;
		std::vector<uint32_t>	heights_;
		std::vector<uint32_t>	depths_;

		bool auto_gen_mipmaps_;
	};

	class D3D9TextureCube : public D3D9Texture
	{
	public:
		D3D9TextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTextureCube(Texture& target, CubeFaces face, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);

		void BuildMipSubLevels();

		ID3D9CubeTexturePtr D3DTextureCube() const
			{ return d3dTextureCube_; }

	private:
		void MapCube(CubeFaces face, int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void UnmapCube(CubeFaces face, int level);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		void QueryBaseTexture();
		void UpdateParams();

		ID3D9CubeTexturePtr CreateTextureCube(uint32_t usage, D3DPOOL pool);

	private:
		ID3D9CubeTexturePtr		d3dTextureCube_;

		std::vector<uint32_t>	widths_;

		bool auto_gen_mipmaps_;
	};
}

#endif			// _D3D9TEXTURE_HPP
