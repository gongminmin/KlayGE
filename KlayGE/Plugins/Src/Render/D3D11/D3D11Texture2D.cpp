// D3D11Texture2D.cpp
// KlayGE D3D11 2D������ ʵ���ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/TexCompression.hpp>

#include <cstring>

#include <KlayGE/D3D11/D3D11Util.hpp>
#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>

namespace KlayGE
{
	D3D11Texture2D::D3D11Texture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: D3D11Texture(TT_2D, sample_count, sample_quality, access_hint)
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
		dxgi_fmt_ = D3D11Mapping::MappingFormat(format_);
		width_ = width;
		height_ = height;
	}

	D3D11Texture2D::D3D11Texture2D(ID3D11Texture2DPtr const & d3d_tex)
					: D3D11Texture(TT_2D, 1, 0, 0)
	{
		D3D11_TEXTURE2D_DESC desc;
		d3d_tex->GetDesc(&desc);

		num_mip_maps_ = desc.MipLevels;
		array_size_ = desc.ArraySize;
		format_ = D3D11Mapping::MappingFormat(desc.Format);
		sample_count_ = desc.SampleDesc.Count;
		sample_quality_ = desc.SampleDesc.Quality;
		width_ = desc.Width;
		height_ = desc.Height;

		access_hint_ = 0;
		switch (desc.Usage)
		{
		case D3D11_USAGE_DEFAULT:
			access_hint_ |= EAH_GPU_Read | EAH_GPU_Write;
			break;

		case D3D11_USAGE_IMMUTABLE:
			access_hint_ |= EAH_Immutable;
			break;

		case D3D11_USAGE_DYNAMIC:
			access_hint_ |= EAH_GPU_Read;
			if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE)
			{
				access_hint_ |= EAH_CPU_Write;
			}
			break;

		case D3D11_USAGE_STAGING:
			if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)
			{
				access_hint_ |= EAH_CPU_Read;
			}
			if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE)
			{
				access_hint_ |= EAH_CPU_Write;
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid usage");
		}
		if (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			access_hint_ |= EAH_GPU_Unordered;
		}
		if (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
		{
			access_hint_ |= EAH_Generate_Mips;
		}

		d3d_texture_ = d3d_tex;
	}

	uint32_t D3D11Texture2D::Width(uint32_t level) const noexcept
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t D3D11Texture2D::Height(uint32_t level) const noexcept
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, height_ >> level);
	}

	void D3D11Texture2D::CopyToTexture(Texture& target, TextureFilter filter)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((this->Width(0) == target.Width(0)) && (this->Height(0) == target.Height(0)) && (this->Format() == target.Format())
			&& (this->ArraySize() == target.ArraySize()) && (this->NumMipMaps() == target.NumMipMaps()))
		{
			auto& other = checked_cast<D3D11Texture2D&>(target);

			if ((this->SampleCount() > 1) && (1 == target.SampleCount()))
			{
				for (uint32_t l = 0; l < this->NumMipMaps(); ++ l)
				{
					d3d_imm_ctx_->ResolveSubresource(other.d3d_texture_.get(), D3D11CalcSubresource(l, 0, other.NumMipMaps()),
						d3d_texture_.get(), D3D11CalcSubresource(l, 0, this->NumMipMaps()), D3D11Mapping::MappingFormat(target.Format()));
				}
			}
			else
			{
				d3d_imm_ctx_->CopyResource(other.d3d_texture_.get(), d3d_texture_.get());
			}
		}
		else
		{
			uint32_t const array_size = std::min(this->ArraySize(), target.ArraySize());
			uint32_t const num_mips = std::min(this->NumMipMaps(), target.NumMipMaps());
			for (uint32_t index = 0; index < array_size; ++ index)
			{
				for (uint32_t level = 0; level < num_mips; ++ level)
				{
					this->ResizeTexture2D(target, index, level, 0, 0, target.Width(level), target.Height(level),
						index, level, 0, 0, this->Width(level), this->Height(level), filter);
				}
			}
		}
	}

	void D3D11Texture2D::CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
		uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset,
		uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter)
	{
		BOOST_ASSERT(type_ == target.Type());

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			auto& other = checked_cast<D3D11Texture&>(target);

			D3D11_BOX* src_box_ptr;
			D3D11_BOX src_box;
			if ((sample_count_ != 1) || IsDepthFormat(format_))
			{
				BOOST_ASSERT(other.SampleCount() == sample_count_);
				BOOST_ASSERT(dst_x_offset == 0);
				BOOST_ASSERT(dst_y_offset == 0);

				src_box_ptr = nullptr;
			}
			else
			{
				src_box.left = src_x_offset;
				src_box.top = src_y_offset;
				src_box.front = 0;
				src_box.right = src_x_offset + src_width;
				src_box.bottom = src_y_offset + src_height;
				src_box.back = 1;

				src_box_ptr = &src_box;
			}

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DResource(), D3D11CalcSubresource(dst_level, dst_array_index, target.NumMipMaps()),
				dst_x_offset, dst_y_offset, 0, this->D3DResource(), D3D11CalcSubresource(src_level, src_array_index, this->NumMipMaps()),
				src_box_ptr);
		}
		else
		{
			this->ResizeTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
		}
	}

	void D3D11Texture2D::CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level,
		uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, CubeFaces src_face,
		uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter)
	{
		KFL_UNUSED(src_face);
		BOOST_ASSERT(TT_Cube == target.Type());

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			auto& other = checked_cast<D3D11Texture&>(target);

			D3D11_BOX* src_box_ptr;
			D3D11_BOX src_box;
			if ((sample_count_ != 1) || IsDepthFormat(format_))
			{
				BOOST_ASSERT(other.SampleCount() == sample_count_);
				BOOST_ASSERT(dst_x_offset == 0);
				BOOST_ASSERT(dst_y_offset == 0);

				src_box_ptr = nullptr;
			}
			else
			{
				src_box.left = src_x_offset;
				src_box.top = src_y_offset;
				src_box.front = 0;
				src_box.right = src_x_offset + src_width;
				src_box.bottom = src_y_offset + src_height;
				src_box.back = 1;

				src_box_ptr = &src_box;
			}

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DResource(),
				D3D11CalcSubresource(dst_level, dst_array_index * 6 + dst_face - CF_Positive_X, target.NumMipMaps()),
				dst_x_offset, dst_y_offset, 0, this->D3DResource(),
				D3D11CalcSubresource(src_level, src_array_index, this->NumMipMaps()),
				src_box_ptr);
		}
		else
		{
			this->ResizeTexture2D(target, dst_array_index * 6 + dst_face - CF_Positive_X, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC D3D11Texture2D::FillSRVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
			uint32_t first_level, uint32_t num_levels) const
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
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
			desc.Format = D3D11Mapping::MappingFormat(pf);
			break;
		}

		if (array_size_ > 1)
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = first_array_index;
				desc.Texture2DMSArray.ArraySize = array_size;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = first_level;
				desc.Texture2DArray.MipLevels = num_levels;
				desc.Texture2DArray.FirstArraySlice = first_array_index;
				desc.Texture2DArray.ArraySize = array_size;
			}
		}
		else
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MostDetailedMip = first_level;
				desc.Texture2D.MipLevels = num_levels;
			}
		}

		return desc;
	}

	D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture2D::FillRTVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(pf);
		if (array_size_ > 1)
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = first_array_index;
				desc.Texture2DMSArray.ArraySize = array_size;
			}
			else
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = level;
				desc.Texture2DArray.FirstArraySlice = first_array_index;
				desc.Texture2DArray.ArraySize = array_size;
			}
		}
		else
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = level;
			}
		}

		return desc;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC D3D11Texture2D::FillDSVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(pf);
		desc.Flags = 0;
		if (array_size_ > 1)
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = first_array_index;
				desc.Texture2DMSArray.ArraySize = array_size;
			}
			else
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = level;
				desc.Texture2DArray.FirstArraySlice = first_array_index;
				desc.Texture2DArray.ArraySize = array_size;
			}
		}
		else
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = level;
			}
		}

		return desc;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC D3D11Texture2D::FillUAVDesc(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t level) const
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(pf);
		if (array_size_ > 1)
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.FirstArraySlice = first_array_index;
			desc.Texture2DArray.ArraySize = array_size;
		}
		else
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = level;
		}

		return desc;
	}

	void D3D11Texture2D::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t /*width*/, uint32_t /*height*/,
			void*& data, uint32_t& row_pitch)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		TIFHR(d3d_imm_ctx_->Map(d3d_texture_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_), D3D11Mapping::Mapping(tma, type_, access_hint_, num_mip_maps_), 0, &mapped));
		uint8_t* p = static_cast<uint8_t*>(mapped.pData);
		if (IsCompressedFormat(format_))
		{
			uint32_t const block_width = BlockWidth(format_);
			uint32_t const block_height = BlockHeight(format_);
			uint32_t const block_bytes = BlockBytes(format_);
			data = p + (y_offset / block_height) * mapped.RowPitch + (x_offset / block_width) * block_bytes;
		}
		else
		{
			data = p + y_offset * mapped.RowPitch + x_offset * NumFormatBytes(format_);
		}
		row_pitch = mapped.RowPitch;
	}

	void D3D11Texture2D::Unmap2D(uint32_t array_index, uint32_t level)
	{
		d3d_imm_ctx_->Unmap(d3d_texture_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_));
	}

	void D3D11Texture2D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
	{
		KFL_UNUSED(clear_value_hint);

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = width_;
		desc.Height = height_;
		desc.MipLevels = num_mip_maps_;
		desc.ArraySize = array_size_;
		switch (format_)
		{
		case EF_D16:
			desc.Format = DXGI_FORMAT_R16_TYPELESS;
			break;

		case EF_D24S8:
			desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			break;

		case EF_D32F:
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			break;

		default:
			desc.Format = dxgi_fmt_;
			break;
		}
		desc.SampleDesc.Count = sample_count_;
		desc.SampleDesc.Quality = sample_quality_;
		this->GetD3DFlags(desc.Usage, desc.BindFlags, desc.CPUAccessFlags, desc.MiscFlags);

		std::vector<D3D11_SUBRESOURCE_DATA> subres_data;
		if (!init_data.empty())
		{
			BOOST_ASSERT(init_data.size() == array_size_ * num_mip_maps_);
			subres_data.resize(init_data.size());
			for (size_t i = 0; i < init_data.size(); ++i)
			{
				subres_data[i].pSysMem = init_data[i].data;
				subres_data[i].SysMemPitch = init_data[i].row_pitch;
				subres_data[i].SysMemSlicePitch = init_data[i].slice_pitch;
			}
		}

		ID3D11Texture2DPtr d3d_tex;
		TIFHR(d3d_device_->CreateTexture2D(&desc, subres_data.data(), d3d_tex.put()));
		d3d_tex.as(d3d_texture_);
	}
}
