// D3D11Texture2D.cpp
// KlayGE D3D11 2D纹理类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>

namespace KlayGE
{
	D3D11Texture2D::D3D11Texture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
					: D3D11Texture(TT_2D, sample_count, sample_quality, access_hint)
	{
		if (0 == numMipMaps)
		{
			num_mip_maps_ = 1;
			uint32_t w = width;
			uint32_t h = height;
			while ((w != 1) || (h != 1))
			{
				++ num_mip_maps_;

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.DeviceFeatureLevel() <= D3D_FEATURE_LEVEL_9_3)
		{
			if ((num_mip_maps_ > 1) && (((width & (width - 1)) != 0) || ((height & (height - 1)) != 0)))
			{
				// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
				num_mip_maps_ = 1;
			}
		}

		array_size_ = array_size;
		format_		= format;
		widths_.assign(1, width);
		heights_.assign(1, height);

		desc_.Width = width;
		desc_.Height = height;
		desc_.MipLevels = num_mip_maps_;
		desc_.ArraySize = array_size_;
		switch (format_)
		{
		case EF_D16:
			desc_.Format = DXGI_FORMAT_R16_TYPELESS;
			break;

		case EF_D24S8:
			desc_.Format = DXGI_FORMAT_R24G8_TYPELESS;
			break;

		case EF_D32F:
			desc_.Format = DXGI_FORMAT_R32_TYPELESS;
			break;

		default:
			desc_.Format = D3D11Mapping::MappingFormat(format_);
			break;
		}
		desc_.SampleDesc.Count = sample_count;
		desc_.SampleDesc.Quality = sample_quality;

		this->GetD3DFlags(desc_.Usage, desc_.BindFlags, desc_.CPUAccessFlags, desc_.MiscFlags);

		std::vector<D3D11_SUBRESOURCE_DATA> subres_data(array_size_ * num_mip_maps_);
		if (init_data != NULL)
		{
			for (uint32_t j = 0; j < array_size_; ++ j)
			{
				for (uint32_t i = 0; i < num_mip_maps_; ++ i)
				{
					subres_data[j * num_mip_maps_ + i].pSysMem = init_data[j * num_mip_maps_ + i].data;
					subres_data[j * num_mip_maps_ + i].SysMemPitch = init_data[j * num_mip_maps_ + i].row_pitch;
					subres_data[j * num_mip_maps_ + i].SysMemSlicePitch = init_data[j * num_mip_maps_ + i].slice_pitch;
				}
			}
		}

		ID3D11Texture2D* d3d_tex;
		TIF(d3d_device_->CreateTexture2D(&desc_, (init_data != NULL) ? &subres_data[0] : NULL, &d3d_tex));
		d3dTexture2D_ = MakeCOMPtr(d3d_tex);

		this->UpdateParams();

		if ((access_hint & (EAH_GPU_Read | EAH_Generate_Mips)) && (num_mip_maps_ > 1))
		{
			this->RetriveD3DShaderResourceView(0, array_size_, 0, num_mip_maps_);
		}
	}

	uint32_t D3D11Texture2D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return widths_[level];
	}

	uint32_t D3D11Texture2D::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return heights_[level];
	}

	void D3D11Texture2D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture2D& other(*checked_cast<D3D11Texture2D*>(&target));

		if ((this->Width(0) == target.Width(0)) && (this->Height(0) == target.Height(0)) && (this->Format() == target.Format())
			&& (this->NumMipMaps() == target.NumMipMaps()))
		{
			if ((this->SampleCount() > 1) && (1 == target.SampleCount()))
			{
				for (uint32_t l = 0; l < this->NumMipMaps(); ++ l)
				{
					d3d_imm_ctx_->ResolveSubresource(other.D3DTexture().get(), D3D11CalcSubresource(l, 0, other.NumMipMaps()),
						d3dTexture2D_.get(), D3D11CalcSubresource(l, 0, this->NumMipMaps()), D3D11Mapping::MappingFormat(target.Format()));
				}
			}
			else
			{
				d3d_imm_ctx_->CopyResource(other.D3DTexture().get(), d3dTexture2D_.get());
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
						index, level, 0, 0, this->Width(level), this->Height(level), true);
				}
			}
		}
	}

	void D3D11Texture2D::CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture& other(*checked_cast<D3D11Texture*>(&target));

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			D3D11_BOX src_box;
			src_box.left = src_x_offset;
			src_box.top = src_y_offset;
			src_box.front = 0;
			src_box.right = src_x_offset + src_width;
			src_box.bottom = src_y_offset + src_height;
			src_box.back = 1;

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DResource().get(), D3D11CalcSubresource(dst_level, dst_array_index, target.NumMipMaps()),
				dst_x_offset, dst_y_offset, 0, this->D3DResource().get(), D3D11CalcSubresource(src_level, src_array_index, this->NumMipMaps()), &src_box);
		}
		else
		{
			this->ResizeTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
		}
	}

	void D3D11Texture2D::CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		UNREF_PARAM(src_face);
		BOOST_ASSERT(TT_Cube == target.Type());

		D3D11Texture& other(*checked_cast<D3D11Texture*>(&target));

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			D3D11_BOX src_box;
			src_box.left = src_x_offset;
			src_box.top = src_y_offset;
			src_box.front = 0;
			src_box.right = src_x_offset + src_width;
			src_box.bottom = src_y_offset + src_height;
			src_box.back = 1;

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DResource().get(), D3D11CalcSubresource(dst_level, dst_array_index * 6 + dst_face - CF_Positive_X, target.NumMipMaps()),
				dst_x_offset, dst_y_offset, 0, this->D3DResource().get(), D3D11CalcSubresource(src_level, src_array_index, this->NumMipMaps()), &src_box);
		}
		else
		{
			this->ResizeTexture2D(target, dst_array_index * 6 + dst_face - CF_Positive_X, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
		}
	}

	ID3D11ShaderResourceViewPtr const & D3D11Texture2D::RetriveD3DShaderResourceView(uint32_t first_array_index, uint32_t num_items, uint32_t first_level, uint32_t num_levels)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Read);

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		switch (format_)
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
			desc.Format = desc_.Format;
			break;
		}

		if (array_size_ > 1)
		{
			if (sample_count_ > 1)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			}
			desc.Texture2DArray.MostDetailedMip = first_level;
			desc.Texture2DArray.MipLevels = num_levels;
			desc.Texture2DArray.ArraySize = num_items;
			desc.Texture2DArray.FirstArraySlice = first_array_index;
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
			}
			desc.Texture2D.MostDetailedMip = first_level;
			desc.Texture2D.MipLevels = num_levels;
		}

		return this->RetriveD3DSRV(desc);
	}

	ID3D11UnorderedAccessViewPtr const & D3D11Texture2D::RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Unordered);

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.Format = desc_.Format;
		if (array_size_ > 1)
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.ArraySize = num_items;
			desc.Texture2DArray.FirstArraySlice = first_array_index;
		}
		else
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = level;
		}

		return this->RetriveD3DUAV(desc);
	}

	ID3D11RenderTargetViewPtr const & D3D11Texture2D::RetriveD3DRenderTargetView(uint32_t first_array_index, uint32_t array_size, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(first_array_index < this->ArraySize());
		BOOST_ASSERT(first_array_index + array_size <= this->ArraySize());

		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.Format = D3D11Mapping::MappingFormat(this->Format());
		if (this->SampleCount() > 1)
		{
			if (this->ArraySize() > 1)
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
			}
			else
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			}
		}
		else
		{
			if (this->ArraySize() > 1)
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			}
		}
		if (this->ArraySize() > 1)
		{
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.ArraySize = array_size;
			desc.Texture2DArray.FirstArraySlice = first_array_index;
		}
		else
		{
			desc.Texture2D.MipSlice = level;
		}

		return this->RetriveD3DRTV(desc);
	}

	ID3D11DepthStencilViewPtr const & D3D11Texture2D::RetriveD3DDepthStencilView(uint32_t first_array_index, uint32_t array_size, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(first_array_index < this->ArraySize());
		BOOST_ASSERT(first_array_index + array_size <= this->ArraySize());

		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.Format = D3D11Mapping::MappingFormat(this->Format());
		desc.Flags = 0;
		if (this->SampleCount() > 1)
		{
			if (this->ArraySize() > 1)
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
			}
			else
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			}
		}
		else
		{
			if (this->ArraySize() > 1)
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			}
		}
		if (this->ArraySize() > 1)
		{
			desc.Texture2DArray.MipSlice = level;
			desc.Texture2DArray.ArraySize = array_size;
			desc.Texture2DArray.FirstArraySlice = first_array_index;
		}
		else
		{
			desc.Texture2D.MipSlice = level;
		}

		return this->RetriveD3DDSV(desc);
	}

	void D3D11Texture2D::Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t /*width*/, uint32_t /*height*/,
			void*& data, uint32_t& row_pitch)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		TIF(d3d_imm_ctx_->Map(d3dTexture2D_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_), D3D11Mapping::Mapping(tma, type_, access_hint_, num_mip_maps_), 0, &mapped));
		uint8_t* p = static_cast<uint8_t*>(mapped.pData);
		data = p + y_offset * mapped.RowPitch + x_offset * NumFormatBytes(format_);
		row_pitch = mapped.RowPitch;
	}

	void D3D11Texture2D::Unmap2D(uint32_t array_index, uint32_t level)
	{
		d3d_imm_ctx_->Unmap(d3dTexture2D_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_));
	}

	void D3D11Texture2D::BuildMipSubLevels()
	{
		if (!d3d_sr_views_.empty())
		{
			BOOST_ASSERT(access_hint_ & EAH_Generate_Mips);
			d3d_imm_ctx_->GenerateMips(d3d_sr_views_.begin()->second.get());
		}
		else
		{
			for (uint32_t index = 0; index < this->ArraySize(); ++ index)
			{
				for (uint32_t level = 1; level < this->NumMipMaps(); ++ level)
				{
					this->ResizeTexture2D(*this, index, level, 0, 0, this->Width(level), this->Height(level),
						index, level - 1, 0, 0, this->Width(level - 1), this->Height(level - 1), true);
				}
			}
		}
	}

	void D3D11Texture2D::UpdateParams()
	{
		d3dTexture2D_->GetDesc(&desc_);

		num_mip_maps_ = desc_.MipLevels;
		array_size_ = desc_.ArraySize;
		BOOST_ASSERT(num_mip_maps_ != 0);

		widths_.resize(num_mip_maps_);
		heights_.resize(num_mip_maps_);
		widths_[0] = desc_.Width;
		heights_[0] = desc_.Height;
		for (uint32_t level = 1; level < num_mip_maps_; ++ level)
		{
			widths_[level] = std::max<uint32_t>(1U, widths_[level - 1] / 2);
			heights_[level] = std::max<uint32_t>(1U, heights_[level - 1] / 2);
		}

		switch (desc_.Format)
		{
		case DXGI_FORMAT_R16_TYPELESS:
			format_ = EF_D16;
			break;

		case DXGI_FORMAT_R24G8_TYPELESS:
			format_ = EF_D24S8;
			break;

		case DXGI_FORMAT_R32_TYPELESS:
			format_ = EF_D32F;
			break;

		default:
			format_ = D3D11Mapping::MappingFormat(desc_.Format);
			break;
		}
	}
}
