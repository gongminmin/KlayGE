/**
 * @file D3D12Texture.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _D3D12TEXTURE_HPP
#define _D3D12TEXTURE_HPP

#pragma once

#include <KlayGE/Texture.hpp>
#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12RenderView.hpp>

namespace KlayGE
{
	class D3D12Texture : public Texture
	{
	public:
		explicit D3D12Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
		virtual ~D3D12Texture();

		std::wstring const & Name() const;

		uint32_t Width(uint32_t level) const;
		uint32_t Height(uint32_t level) const;
		uint32_t Depth(uint32_t level) const;

		virtual void CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width);
		virtual void CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height);
		virtual void CopyToSubTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth);
		virtual void CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height);

		ID3D12ResourcePtr const & D3DResource() const
		{
			return d3d_texture_;
		}

		virtual D3D12ShaderResourceViewSimulationPtr const & RetriveD3DShaderResourceView(uint32_t first_array_index, uint32_t num_items, uint32_t first_level, uint32_t num_levels);

		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items, uint32_t level);
		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level);
		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items, CubeFaces first_face, uint32_t num_faces, uint32_t level);

		virtual D3D12RenderTargetViewSimulationPtr const & RetriveD3DRenderTargetView(uint32_t first_array_index, uint32_t array_size, uint32_t level);
		virtual D3D12RenderTargetViewSimulationPtr const & RetriveD3DRenderTargetView(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level);
		virtual D3D12RenderTargetViewSimulationPtr const & RetriveD3DRenderTargetView(uint32_t array_index, CubeFaces face, uint32_t level);
		virtual D3D12DepthStencilViewSimulationPtr const & RetriveD3DDepthStencilView(uint32_t first_array_index, uint32_t array_size, uint32_t level);
		virtual D3D12DepthStencilViewSimulationPtr const & RetriveD3DDepthStencilView(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level);
		virtual D3D12DepthStencilViewSimulationPtr const & RetriveD3DDepthStencilView(uint32_t array_index, CubeFaces face, uint32_t level);

		virtual void DeleteHWResource() override;
		virtual bool HWResourceReady() const override;

	protected:
		void DoHWCopyToTexture(Texture& target);
		void DoHWCopyToSubTexture(Texture& target,
			uint32_t dst_subres, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset,
			uint32_t src_subres, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset,
			uint32_t width, uint32_t height, uint32_t depth);

		void DoCreateHWResource(D3D12_RESOURCE_DIMENSION dim,
			uint32_t width, uint32_t height, uint32_t depth, uint32_t array_size,
			ElementInitData const * init_data);

		void DoMap(uint32_t subres, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch);
		void DoUnmap(uint32_t subres);

		D3D12ShaderResourceViewSimulationPtr const & RetriveD3DSRV(D3D12_SHADER_RESOURCE_VIEW_DESC const & desc);
		D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC const & desc);
		D3D12RenderTargetViewSimulationPtr const & RetriveD3DRTV(D3D12_RENDER_TARGET_VIEW_DESC const & desc);
		D3D12DepthStencilViewSimulationPtr const & RetriveD3DDSV(D3D12_DEPTH_STENCIL_VIEW_DESC const & desc);

	private:
		virtual void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width,
			void*& data);
		virtual void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		virtual void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch);
		virtual void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);

		virtual void Unmap1D(uint32_t array_index, uint32_t level);
		virtual void Unmap2D(uint32_t array_index, uint32_t level);
		virtual void Unmap3D(uint32_t array_index, uint32_t level);
		virtual void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level);

	protected:
		DXGI_FORMAT dxgi_fmt_;

		ID3D12ResourcePtr d3d_texture_;
		ID3D12ResourcePtr d3d_texture_upload_heaps_;
		ID3D12ResourcePtr d3d_texture_readback_heaps_;

		TextureMapAccess last_tma_;

		std::unordered_map<size_t, D3D12ShaderResourceViewSimulationPtr> d3d_sr_views_;
		std::unordered_map<size_t, D3D12UnorderedAccessViewSimulationPtr> d3d_ua_views_;
		std::unordered_map<size_t, D3D12RenderTargetViewSimulationPtr> d3d_rt_views_;
		std::unordered_map<size_t, D3D12DepthStencilViewSimulationPtr> d3d_ds_views_;
	};

	typedef std::shared_ptr<D3D12Texture> D3D12TexturePtr;


	class D3D12Texture1D : public D3D12Texture
	{
	public:
		D3D12Texture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const;

		void CopyToTexture(Texture& target);
		void CopyToSubTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width);

		virtual D3D12ShaderResourceViewSimulationPtr const & RetriveD3DShaderResourceView(uint32_t first_array_index, uint32_t num_items, uint32_t first_level, uint32_t num_levels) override;

		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items, uint32_t level) override;

		virtual D3D12RenderTargetViewSimulationPtr const & RetriveD3DRenderTargetView(uint32_t first_array_index, uint32_t array_size, uint32_t level) override;
		virtual D3D12DepthStencilViewSimulationPtr const & RetriveD3DDepthStencilView(uint32_t first_array_index, uint32_t array_size, uint32_t level) override;

		void BuildMipSubLevels();

		virtual void CreateHWResource(ElementInitData const * init_data) override;

	private:
		void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width,
			void*& data);
		void Unmap1D(uint32_t array_index, uint32_t level);

	private:
		uint32_t width_;
	};

	class D3D12Texture2D : public D3D12Texture
	{
	public:
		D3D12Texture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
		explicit D3D12Texture2D(ID3D12ResourcePtr const & d3d_tex);

		uint32_t Width(uint32_t level) const;
		uint32_t Height(uint32_t level) const;

		void CopyToTexture(Texture& target);
		void CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height);
		void CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height);

		virtual D3D12ShaderResourceViewSimulationPtr const & RetriveD3DShaderResourceView(uint32_t first_array_index, uint32_t num_items, uint32_t first_level, uint32_t num_levels) override;

		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items, uint32_t level) override;

		virtual D3D12RenderTargetViewSimulationPtr const & RetriveD3DRenderTargetView(uint32_t first_array_index, uint32_t array_size, uint32_t level) override;
		virtual D3D12DepthStencilViewSimulationPtr const & RetriveD3DDepthStencilView(uint32_t first_array_index, uint32_t array_size, uint32_t level) override;

		void BuildMipSubLevels();

		virtual void CreateHWResource(ElementInitData const * init_data) override;

	private:
		void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void Unmap2D(uint32_t array_index, uint32_t level);

	private:
		uint32_t width_;
		uint32_t height_;
	};

	class D3D12Texture3D : public D3D12Texture
	{
	public:
		D3D12Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const;
		uint32_t Height(uint32_t level) const;
		uint32_t Depth(uint32_t level) const;

		void CopyToTexture(Texture& target);
		void CopyToSubTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth);

		virtual D3D12ShaderResourceViewSimulationPtr const & RetriveD3DShaderResourceView(uint32_t first_array_index, uint32_t num_items, uint32_t first_level, uint32_t num_levels) override;

		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items, uint32_t level) override;
		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) override;

		virtual D3D12RenderTargetViewSimulationPtr const & RetriveD3DRenderTargetView(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) override;
		virtual D3D12DepthStencilViewSimulationPtr const & RetriveD3DDepthStencilView(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) override;

		void BuildMipSubLevels();

		virtual void CreateHWResource(ElementInitData const * init_data) override;

	private:
		void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch);
		void Unmap3D(uint32_t array_index, uint32_t level);

	private:
		uint32_t width_;
		uint32_t height_;
		uint32_t depth_;
	};

	class D3D12TextureCube : public D3D12Texture
	{
	public:
		D3D12TextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const;
		uint32_t Height(uint32_t level) const;

		void CopyToTexture(Texture& target);
		void CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height);
		void CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height);

		virtual D3D12ShaderResourceViewSimulationPtr const & RetriveD3DShaderResourceView(uint32_t first_array_index, uint32_t num_items, uint32_t first_level, uint32_t num_levels) override;

		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items, uint32_t level) override;
		virtual D3D12UnorderedAccessViewSimulationPtr const & RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items, CubeFaces first_face, uint32_t num_faces, uint32_t level) override;

		virtual D3D12RenderTargetViewSimulationPtr const & RetriveD3DRenderTargetView(uint32_t first_array_index, uint32_t array_size, uint32_t level) override;
		virtual D3D12RenderTargetViewSimulationPtr const & RetriveD3DRenderTargetView(uint32_t array_index, CubeFaces face, uint32_t level) override;
		virtual D3D12DepthStencilViewSimulationPtr const & RetriveD3DDepthStencilView(uint32_t first_array_index, uint32_t array_size, uint32_t level) override;
		virtual D3D12DepthStencilViewSimulationPtr const & RetriveD3DDepthStencilView(uint32_t array_index, CubeFaces face, uint32_t level) override;

		void BuildMipSubLevels();

		virtual void CreateHWResource(ElementInitData const * init_data) override;

	private:
		void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch);
		void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level);

	private:
		uint32_t width_;
	};
}

#endif			// _D3D12TEXTURE_HPP
