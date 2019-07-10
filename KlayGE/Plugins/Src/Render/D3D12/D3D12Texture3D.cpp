/**
 * @file D3D12Texture3D.cpp
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

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>

namespace KlayGE
{
	D3D12Texture3D::D3D12Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: D3D12Texture(TT_3D, sample_count, sample_quality, access_hint)
	{
		BOOST_ASSERT(1 == array_size);

		if (0 == numMipMaps)
		{
			numMipMaps = 1;
			uint32_t w = width;
			uint32_t h = height;
			uint32_t d = depth;
			while ((w != 1) || (h != 1) || (d != 1))
			{
				++ numMipMaps;

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
				d = std::max<uint32_t>(1U, d / 2);
			}
		}
		num_mip_maps_ = numMipMaps;

		array_size_ = array_size;
		format_		= format;

		width_ = width;
		height_ = height;
		depth_ = depth;

		dxgi_fmt_ = D3D12Mapping::MappingFormat(format_);

		curr_states_.assign(array_size_ * num_mip_maps_, D3D12_RESOURCE_STATE_COMMON);
	}

	uint32_t D3D12Texture3D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t D3D12Texture3D::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, height_ >> level);
	}

	uint32_t D3D12Texture3D::Depth(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, depth_ >> level);
	}

	void D3D12Texture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((this->Width(0) == target.Width(0)) && (this->Height(0) == target.Height(0))
			&& (this->Depth(0) == target.Depth(0)) && (this->Format() == target.Format())
			&& (this->NumMipMaps() == target.NumMipMaps()))
		{
			this->DoHWCopyToTexture(target);
		}
		else
		{
			uint32_t const array_size = std::min(this->ArraySize(), target.ArraySize());
			uint32_t const num_mips = std::min(this->NumMipMaps(), target.NumMipMaps());
			for (uint32_t index = 0; index < array_size; ++ index)
			{
				for (uint32_t level = 0; level < num_mips; ++ level)
				{
					this->ResizeTexture3D(target, index, level, 0, 0, 0, target.Width(level), target.Height(level), target.Depth(level),
						index, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level), true);
				}
			}
		}
	}

	void D3D12Texture3D::CopyToSubTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			uint32_t const src_subres = CalcSubresource(src_level, src_array_index, 0,
				this->NumMipMaps(), this->ArraySize());
			uint32_t const dst_subres = CalcSubresource(dst_level, dst_array_index, 0,
				target.NumMipMaps(), target.ArraySize());

			this->DoHWCopyToSubTexture(target, dst_subres, dst_x_offset, dst_y_offset, dst_z_offset,
				src_subres, src_x_offset, src_y_offset, src_z_offset,
				src_width, src_height, src_depth);
		}
		else
		{
			this->ResizeTexture3D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_z_offset, dst_width, dst_height, dst_depth,
				src_array_index, src_level, src_x_offset, src_y_offset, src_z_offset, src_width, src_height, src_depth, true);
		}
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC D3D12Texture3D::FillSRVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t first_level, uint32_t num_levels) const
	{
		BOOST_ASSERT(0 == first_array_index);
		BOOST_ASSERT(1 == array_size);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(array_size);

		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		switch (pf)
		{
		case EF_D16:
			desc.Format = DXGI_FORMAT_R16_UNORM;
			break;

		case EF_D24S8:
			desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;

		case EF_D32F:
			desc.Format = DXGI_FORMAT_R32_FLOAT;
			break;

		default:
			desc.Format = D3D12Mapping::MappingFormat(pf);
			break;
		}
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MostDetailedMip = first_level;
		desc.Texture3D.MipLevels = num_levels;
		desc.Texture3D.ResourceMinLODClamp = 0;

		return desc;
	}

	D3D12_RENDER_TARGET_VIEW_DESC D3D12Texture3D::FillRTVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = level;
		desc.Texture3D.FirstWSlice = first_slice;
		desc.Texture3D.WSize = num_slices;

		return desc;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12Texture3D::FillDSVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		desc.Flags = D3D12_DSV_FLAG_NONE;
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		}
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.ArraySize = num_slices;
		desc.Texture2DArray.FirstArraySlice = first_slice;

		return desc;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12Texture3D::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		BOOST_ASSERT(0 == first_array_index);
		BOOST_ASSERT(1 == array_size);
		KFL_UNUSED(array_size);

		return this->FillUAVDesc(pf, first_array_index, 0, depth_, level);
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12Texture3D::FillUAVDesc(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
		uint32_t num_slices, uint32_t level) const
	{
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = level;
		desc.Texture3D.FirstWSlice = first_slice;
		desc.Texture3D.WSize = num_slices;

		return desc;
	}

	void D3D12Texture3D::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		uint32_t const subres = CalcSubresource(level, array_index, 0, num_mip_maps_, array_size_);
		this->DoMap(subres, tma, x_offset, y_offset, z_offset, width, height, depth, data, row_pitch, slice_pitch);
	}

	void D3D12Texture3D::Unmap3D(uint32_t array_index, uint32_t level)
	{
		uint32_t const subres = CalcSubresource(level, array_index, 0, num_mip_maps_, array_size_);
		this->DoUnmap(subres);
	}

	void D3D12Texture3D::BuildMipSubLevels()
	{		
		// TODO
		// GPU mipmapper
		for (uint32_t index = 0; index < this->ArraySize(); ++ index)
		{
			for (uint32_t level = 1; level < this->NumMipMaps(); ++ level)
			{
				this->ResizeTexture3D(*this, index, level, 0, 0, 0, this->Width(level), this->Height(level), this->Depth(level),
					index, level - 1, 0, 0, 0, this->Width(level - 1), this->Height(level - 1), this->Depth(level - 1), true);
			}
		}
	}

	void D3D12Texture3D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		this->DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE3D,
			width_, height_, depth_, array_size_, init_data, clear_value_hint);
	}
}
