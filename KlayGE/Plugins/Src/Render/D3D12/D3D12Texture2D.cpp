/**
 * @file D3D12Texture2D.cpp
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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <cstring>
#include <iterator>

#include <KlayGE/D3D12/D3D12Typedefs.hpp>
#include <KlayGE/D3D12/D3D12RenderEngine.hpp>
#include <KlayGE/D3D12/D3D12Mapping.hpp>
#include <KlayGE/D3D12/D3D12Texture.hpp>
#include <KlayGE/D3D12/D3D12ShaderObject.hpp>
#include <KlayGE/D3D12/D3D12RenderStateObject.hpp>
#include <KlayGE/D3D12/D3D12FrameBuffer.hpp>
#include <KlayGE/D3D12/D3D12RenderLayout.hpp>

namespace KlayGE
{
	D3D12Texture2D::D3D12Texture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: D3D12Texture(TT_2D, sample_count, sample_quality, access_hint)
	{
		if (0 == numMipMaps)
		{
			numMipMaps = 1;
			uint32_t w = width;
			uint32_t h = height;
			while ((w != 1) || (h != 1))
			{
				++ numMipMaps;

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
			}
		}
		num_mip_maps_ = numMipMaps;

		array_size_ = array_size;
		format_		= format;
		width_ = width;
		height_ = height;

		switch (format_)
		{
		case EF_D16:
			dxgi_fmt_ = DXGI_FORMAT_R16_TYPELESS;
			break;

		case EF_D24S8:
			dxgi_fmt_ = DXGI_FORMAT_R24G8_TYPELESS;
			break;

		case EF_D32F:
			dxgi_fmt_ = DXGI_FORMAT_R32_TYPELESS;
			break;

		default:
			dxgi_fmt_ = D3D12Mapping::MappingFormat(format_);
			break;
		}

		curr_states_.assign(array_size_ * num_mip_maps_, D3D12_RESOURCE_STATE_COMMON);
	}

	D3D12Texture2D::D3D12Texture2D(ID3D12ResourcePtr const & d3d_tex)
					: D3D12Texture(TT_2D, 1, 0, 0)
	{
		D3D12_HEAP_PROPERTIES heap_prop;
		d3d_tex->GetHeapProperties(&heap_prop, nullptr);
		D3D12_RESOURCE_DESC const desc = d3d_tex->GetDesc();

		num_mip_maps_ = desc.MipLevels;
		array_size_ = desc.DepthOrArraySize;
		format_ = D3D12Mapping::MappingFormat(desc.Format);
		sample_count_ = desc.SampleDesc.Count;
		sample_quality_ = desc.SampleDesc.Quality;
		width_ = static_cast<uint32_t>(desc.Width);
		height_ = static_cast<uint32_t>(desc.Height);

		access_hint_ = 0;
		switch (heap_prop.Type)
		{
		case D3D12_HEAP_TYPE_DEFAULT:
			access_hint_ |= EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered;
			break;

		case D3D12_HEAP_TYPE_UPLOAD:
			access_hint_ |= EAH_CPU_Read | EAH_CPU_Write | EAH_GPU_Read;
			break;

		case D3D12_HEAP_TYPE_READBACK:
			access_hint_ |= EAH_CPU_Read;
			break;

		case D3D12_HEAP_TYPE_CUSTOM:
			access_hint_ |= EAH_CPU_Read | EAH_CPU_Write;
			break;

		default:
			KFL_UNREACHABLE("Invalid heap type");
		}

		d3d_resource_ = d3d_tex;

		curr_states_.assign(array_size_ * num_mip_maps_, D3D12_RESOURCE_STATE_COMMON);
	}

	uint32_t D3D12Texture2D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t D3D12Texture2D::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, height_ >> level);
	}

	void D3D12Texture2D::CopyToTexture(Texture& target, TextureFilter filter)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((this->Width(0) == target.Width(0)) && (this->Height(0) == target.Height(0)) && (this->Format() == target.Format())
			&& (this->ArraySize() == target.ArraySize()) && (this->NumMipMaps() == target.NumMipMaps()))
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
					this->ResizeTexture2D(target, index, level, 0, 0, target.Width(level), target.Height(level), index, level, 0, 0,
						this->Width(level), this->Height(level), filter);
				}
			}
		}
	}

	void D3D12Texture2D::CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
		uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset,
		uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			uint32_t const src_subres = CalcSubresource(src_level, src_array_index, 0,
				this->NumMipMaps(), this->ArraySize());
			uint32_t const dst_subres = CalcSubresource(dst_level, dst_array_index, 0,
				target.NumMipMaps(), target.ArraySize());

			this->DoHWCopyToSubTexture(target, dst_subres, dst_x_offset, dst_y_offset, 0,
				src_subres, src_x_offset, src_y_offset, 0,
				src_width, src_height, 1);
		}
		else
		{
			this->ResizeTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
		}
	}

	void D3D12Texture2D::CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level,
		uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, CubeFaces src_face,
		uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter)
	{
		KFL_UNUSED(src_face);
		BOOST_ASSERT(TT_Cube == target.Type());

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			uint32_t const src_subres = CalcSubresource(src_level, src_array_index, 0,
				this->NumMipMaps(), this->ArraySize());
			uint32_t const dst_subres = CalcSubresource(dst_level, dst_array_index * 6 + dst_face - CF_Positive_X, 0,
				target.NumMipMaps(), target.ArraySize() * 6);

			this->DoHWCopyToSubTexture(target, dst_subres, dst_x_offset, dst_y_offset, 0,
				src_subres, src_x_offset, src_y_offset, 0,
				src_width, src_height, 1);
		}
		else
		{
			this->ResizeTexture2D(target, dst_array_index * 6 + dst_face - CF_Positive_X, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
		}
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC D3D12Texture2D::FillSRVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level,
		uint32_t num_levels) const
	{
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

		if (array_size_ > 1)
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.ArraySize = array_size;
				desc.Texture2DMSArray.FirstArraySlice = first_array_index;
			}
			else
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = first_level;
				desc.Texture2DArray.MipLevels = num_levels;
				desc.Texture2DArray.ArraySize = array_size;
				desc.Texture2DArray.FirstArraySlice = first_array_index;
				desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
				desc.Texture2DArray.ResourceMinLODClamp = 0;
			}
		}
		else
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MostDetailedMip = first_level;
				desc.Texture2D.MipLevels = num_levels;
				desc.Texture2D.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
				desc.Texture2D.ResourceMinLODClamp = 0;
			}
		}

		return desc;
	}

	D3D12_RENDER_TARGET_VIEW_DESC D3D12Texture2D::FillRTVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		if (sample_count_ > 1)
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
			desc.Texture2DMSArray.FirstArraySlice = first_array_index;
			desc.Texture2DMSArray.ArraySize = array_size;
		}
		else
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.FirstArraySlice = first_array_index;
			desc.Texture2DArray.ArraySize = array_size;
			desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
		}

		return desc;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12Texture2D::FillDSVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		desc.Flags = D3D12_DSV_FLAG_NONE;
		if (sample_count_ > 1)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
			desc.Texture2DMSArray.FirstArraySlice = first_array_index;
			desc.Texture2DMSArray.ArraySize = array_size;
		}
		else
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.FirstArraySlice = first_array_index;
			desc.Texture2DArray.ArraySize = array_size;
		}

		return desc;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12Texture2D::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = D3D12Mapping::MappingFormat(pf);
		if (array_size_ > 1)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.ArraySize = array_size;
			desc.Texture2DArray.FirstArraySlice = first_array_index;
			desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
		}
		else
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = level;
			desc.Texture2D.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
		}

		return desc;
	}

	void D3D12Texture2D::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch)
	{
		uint32_t const subres = CalcSubresource(level, array_index, 0, num_mip_maps_, array_size_);

		uint32_t slice_pitch;
		this->DoMap(subres, tma, x_offset, y_offset, 0, width, height, 1, data, row_pitch, slice_pitch);
	}

	void D3D12Texture2D::Unmap2D(uint32_t array_index, uint32_t level)
	{
		uint32_t const subres = CalcSubresource(level, array_index, 0, num_mip_maps_, array_size_);
		this->DoUnmap(subres);
	}

	void D3D12Texture2D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		this->DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			width_, height_, 1, array_size_, init_data, clear_value_hint);
	}
}
