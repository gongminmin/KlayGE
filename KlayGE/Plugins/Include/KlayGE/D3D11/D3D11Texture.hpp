// D3D11Texture.hpp
// KlayGE D3D11������ ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11TEXTURE_HPP
#define _D3D11TEXTURE_HPP

#pragma once

#include <unordered_map>

#include <KlayGE/Texture.hpp>
#include <KlayGE/D3D11/D3D11Util.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>

namespace KlayGE
{
	class D3D11Texture : public Texture
	{
	public:
		D3D11Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		std::wstring const & Name() const override;
#ifndef KLAYGE_SHIP
		void DebugName(std::wstring_view name) override;
#endif

		uint32_t Width(uint32_t level) const noexcept override;
		uint32_t Height(uint32_t level) const noexcept override;
		uint32_t Depth(uint32_t level) const noexcept override;

		void CopyToSubTexture1D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width, TextureFilter filter) override;
		void CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset,
			uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter) override;
		void CopyToSubTexture3D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth, uint32_t src_array_index,
			uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width,
			uint32_t src_height, uint32_t src_depth, TextureFilter filter) override;
		void CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset,
			uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, CubeFaces src_face,
			uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			TextureFilter filter) override;

		ID3D11Resource* D3DResource() const noexcept
		{
			return d3d_texture_.get();
		}

		ID3D11ShaderResourceViewPtr const & RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
			uint32_t first_level, uint32_t num_levels);
		ID3D11ShaderResourceViewPtr const& RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t array_index, CubeFaces face,
			uint32_t first_level, uint32_t num_levels);

		ID3D11RenderTargetViewPtr const & RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
			uint32_t level);
		ID3D11RenderTargetViewPtr const & RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
			uint32_t num_slices, uint32_t level);
		ID3D11RenderTargetViewPtr const & RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t array_index, CubeFaces face,
			uint32_t level);

		ID3D11DepthStencilViewPtr const & RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
			uint32_t level);
		ID3D11DepthStencilViewPtr const & RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
			uint32_t num_slices, uint32_t level);
		ID3D11DepthStencilViewPtr const & RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t array_index, CubeFaces face,
			uint32_t level);

		ID3D11UnorderedAccessViewPtr const & RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_array_index,
			uint32_t array_size, uint32_t level);
		ID3D11UnorderedAccessViewPtr const & RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
			uint32_t num_slices, uint32_t level);
		ID3D11UnorderedAccessViewPtr const & RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_array_index,
			uint32_t array_size, CubeFaces first_face, uint32_t num_faces, uint32_t level);

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

	protected:
		bool HwBuildMipSubLevels(TextureFilter filter) override;
		void GetD3DFlags(D3D11_USAGE& usage, UINT& bind_flags, UINT& cpu_access_flags, UINT& misc_flags);

		void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t width, void*& data) override;
		void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t y_offset, uint32_t width,
			uint32_t height, void*& data, uint32_t& row_pitch) override;
		void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth, void*& data, uint32_t& row_pitch, uint32_t& slice_pitch) override;
		void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t y_offset,
			uint32_t width, uint32_t height, void*& data, uint32_t& row_pitch) override;

		void Unmap1D(uint32_t array_index, uint32_t level) override;
		void Unmap2D(uint32_t array_index, uint32_t level) override;
		void Unmap3D(uint32_t array_index, uint32_t level) override;
		void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level) override;

		virtual D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const;
		virtual D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
			ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t first_level, uint32_t num_levels) const;

		virtual D3D11_RENDER_TARGET_VIEW_DESC FillRTVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const;
		virtual D3D11_RENDER_TARGET_VIEW_DESC FillRTVDesc(
			ElementFormat pf, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const;
		virtual D3D11_RENDER_TARGET_VIEW_DESC FillRTVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t level) const;

		virtual D3D11_DEPTH_STENCIL_VIEW_DESC FillDSVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const;
		virtual D3D11_DEPTH_STENCIL_VIEW_DESC FillDSVDesc(
			ElementFormat pf, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const;
		virtual D3D11_DEPTH_STENCIL_VIEW_DESC FillDSVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t level) const;

		virtual D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const;
		virtual D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(
			ElementFormat pf, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const;
		virtual D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
			CubeFaces first_face, uint32_t num_faces, uint32_t level) const;

	protected:
		ID3D11Device1*				d3d_device_;
		ID3D11DeviceContext1*		d3d_imm_ctx_;

		DXGI_FORMAT dxgi_fmt_;

		ID3D11ResourcePtr d3d_texture_;

		// TODO: Not caching those views
		std::unordered_map<size_t, ID3D11ShaderResourceViewPtr> d3d_sr_views_;
		std::unordered_map<size_t, ID3D11RenderTargetViewPtr> d3d_rt_views_;
		std::unordered_map<size_t, ID3D11DepthStencilViewPtr> d3d_ds_views_;
		std::unordered_map<size_t, ID3D11UnorderedAccessViewPtr> d3d_ua_views_;
	};


	class D3D11Texture1D final : public D3D11Texture
	{
	public:
		D3D11Texture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const noexcept override;

		void CopyToTexture(Texture& target, TextureFilter filter) override;
		void CopyToSubTexture1D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width, TextureFilter filter) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

	protected:
		void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t width, void*& data) override;
		void Unmap1D(uint32_t array_index, uint32_t level) override;

		D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const override;
		D3D11_RENDER_TARGET_VIEW_DESC FillRTVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;
		D3D11_DEPTH_STENCIL_VIEW_DESC FillDSVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;
		D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;

		using D3D11Texture::FillSRVDesc;
		using D3D11Texture::FillRTVDesc;
		using D3D11Texture::FillDSVDesc;
		using D3D11Texture::FillUAVDesc;

	private:
		uint32_t width_;
	};

	class D3D11Texture2D final : public D3D11Texture
	{
	public:
		D3D11Texture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
		explicit D3D11Texture2D(ID3D11Texture2DPtr const & d3d_tex);

		uint32_t Width(uint32_t level) const noexcept override;
		uint32_t Height(uint32_t level) const noexcept override;

		void CopyToTexture(Texture& target, TextureFilter filter) override;
		void CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset,
			uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter) override;
		void CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset,
			uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, CubeFaces src_face,
			uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			TextureFilter filter) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

	protected:
		void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t y_offset, uint32_t width,
			uint32_t height, void*& data, uint32_t& row_pitch) override;
		void Unmap2D(uint32_t array_index, uint32_t level) override;

		D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const override;
		D3D11_RENDER_TARGET_VIEW_DESC FillRTVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;
		D3D11_DEPTH_STENCIL_VIEW_DESC FillDSVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;
		D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;

		using D3D11Texture::FillSRVDesc;
		using D3D11Texture::FillRTVDesc;
		using D3D11Texture::FillDSVDesc;
		using D3D11Texture::FillUAVDesc;

	private:
		uint32_t width_;
		uint32_t height_;
	};

	class D3D11Texture3D final : public D3D11Texture
	{
	public:
		D3D11Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const noexcept override;
		uint32_t Height(uint32_t level) const noexcept override;
		uint32_t Depth(uint32_t level) const noexcept override;

		void CopyToTexture(Texture& target, TextureFilter filter) override;
		void CopyToSubTexture3D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth, uint32_t src_array_index,
			uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width,
			uint32_t src_height, uint32_t src_depth, TextureFilter filter) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

	protected:
		void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth, void*& data, uint32_t& row_pitch, uint32_t& slice_pitch) override;
		void Unmap3D(uint32_t array_index, uint32_t level) override;

		D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const override;
		D3D11_RENDER_TARGET_VIEW_DESC FillRTVDesc(
			ElementFormat pf, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const override;
		D3D11_DEPTH_STENCIL_VIEW_DESC FillDSVDesc(
			ElementFormat pf, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const override;
		D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;
		D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(
			ElementFormat pf, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const override;

		using D3D11Texture::FillSRVDesc;
		using D3D11Texture::FillRTVDesc;
		using D3D11Texture::FillDSVDesc;
		using D3D11Texture::FillUAVDesc;

	private:
		uint32_t width_;
		uint32_t height_;
		uint32_t depth_;
	};

	class D3D11TextureCube final : public D3D11Texture
	{
	public:
		D3D11TextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
			uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

		uint32_t Width(uint32_t level) const noexcept override;
		uint32_t Height(uint32_t level) const noexcept override;

		void CopyToTexture(Texture& target, TextureFilter filter) override;
		void CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset,
			uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter) override;
		void CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset,
			uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, CubeFaces src_face,
			uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			TextureFilter filter) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

	protected:
		void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t y_offset,
			uint32_t width, uint32_t height, void*& data, uint32_t& row_pitch) override;
		void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level) override;

		D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const override;
		D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
			ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t first_level, uint32_t num_levels) const override;
		D3D11_RENDER_TARGET_VIEW_DESC FillRTVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;
		D3D11_RENDER_TARGET_VIEW_DESC FillRTVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t level) const override;
		D3D11_DEPTH_STENCIL_VIEW_DESC FillDSVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;
		D3D11_DEPTH_STENCIL_VIEW_DESC FillDSVDesc(ElementFormat pf, uint32_t array_index, CubeFaces face, uint32_t level) const override;
		D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(
			ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t level) const override;
		D3D11_UNORDERED_ACCESS_VIEW_DESC FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
			CubeFaces first_face, uint32_t num_faces, uint32_t level) const override;

		using D3D11Texture::FillSRVDesc;
		using D3D11Texture::FillRTVDesc;
		using D3D11Texture::FillDSVDesc;
		using D3D11Texture::FillUAVDesc;

	private:
		uint32_t width_;
	};
}

#endif			// _D3D11TEXTURE_HPP
