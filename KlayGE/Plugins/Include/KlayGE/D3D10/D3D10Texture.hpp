// D3D10Texture.hpp
// KlayGE D3D10纹理类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10TEXTURE_HPP
#define _D3D10TEXTURE_HPP

#pragma once

#include <boost/smart_ptr.hpp>

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>
#include <d3dx10.h>

#include <KlayGE/Texture.hpp>
#include <KlayGE/D3D10/D3D10Typedefs.hpp>
#include <KlayGE/D3D10/D3D10RenderView.hpp>

namespace KlayGE
{
	class D3D10Texture : public Texture
	{
	public:
		explicit D3D10Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
		virtual ~D3D10Texture();

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

		ID3D10ShaderResourceViewPtr const & D3DShaderResourceView() const
		{
			return d3d_sr_view_;
		}

	protected:
		void GetD3DFlags(D3D10_USAGE& usage, UINT& bind_flags, UINT& cpu_access_flags, UINT& misc_flags);

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
		ID3D10DevicePtr			d3d_device_;
		ID3D10ShaderResourceViewPtr d3d_sr_view_;
	};

	typedef boost::shared_ptr<D3D10Texture> D3D10TexturePtr;


	class D3D10Texture1D : public D3D10Texture
	{
	public:
		D3D10Texture1D(uint32_t width, uint16_t numMipMaps, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset);

		void BuildMipSubLevels();

		ID3D10Texture1DPtr const & D3DTexture() const
		{
			return d3dTexture1D_;
		}

	private:
		void Map1D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width, void*& data);
		void Unmap1D(int level);

		void UpdateParams();

	private:
		D3D10_TEXTURE1D_DESC desc_;
		ID3D10Texture1DPtr d3dTexture1D_;

		std::vector<uint32_t> widthes_;
	};

	class D3D10Texture2D : public D3D10Texture
	{
	public:
		D3D10Texture2D(uint32_t width, uint32_t height, uint16_t numMipMaps, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTexture2D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);

		void BuildMipSubLevels();

		ID3D10Texture2DPtr const & D3DTexture() const
		{
			return d3dTexture2D_;
		}

	private:
		void Map2D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void Unmap2D(int level);

		void UpdateParams();

	private:
		D3D10_TEXTURE2D_DESC desc_;
		ID3D10Texture2DPtr d3dTexture2D_;

		std::vector<uint32_t>	widthes_;
		std::vector<uint32_t>	heights_;
	};

	class D3D10Texture3D : public D3D10Texture
	{
	public:
		D3D10Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps, ElementFormat format,
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

		void BuildMipSubLevels();

		ID3D10Texture3DPtr const & D3DTexture() const
		{
			return d3dTexture3D_;
		}

	private:
		void Map3D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch);
		void Unmap3D(int level);

		void UpdateParams();

	private:
		D3D10_TEXTURE3D_DESC desc_;
		ID3D10Texture3DPtr d3dTexture3D_;

		std::vector<uint32_t>	widthes_;
		std::vector<uint32_t>	heights_;
		std::vector<uint32_t>	depthes_;
	};

	class D3D10TextureCube : public D3D10Texture
	{
	public:
		D3D10TextureCube(uint32_t size, uint16_t numMipMaps, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data);

		uint32_t Width(int level) const;
		uint32_t Height(int level) const;
		uint32_t Depth(int level) const;

		void CopyToTexture(Texture& target);
		void CopyToTextureCube(Texture& target, CubeFaces face, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset);

		void BuildMipSubLevels();

		ID3D10TextureCubePtr const & D3DTexture() const
		{
			return d3dTextureCube_;
		}

	private:
		void MapCube(CubeFaces face, int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void UnmapCube(CubeFaces face, int level);

		void UpdateParams();

	private:
		D3D10_TEXTURE2D_DESC desc_;
		ID3D10TextureCubePtr d3dTextureCube_;

		std::vector<uint32_t>	widthes_;
	};
}

#endif			// _D3D10TEXTURE_HPP
