// D3D11TextureCube.cpp
// KlayGE D3D11 Cube纹理类 实现文件
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
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>

namespace KlayGE
{
	D3D11TextureCube::D3D11TextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: D3D11Texture(TT_Cube, sample_count, sample_quality, access_hint)
	{
		if (0 == numMipMaps)
		{
			numMipMaps = 1;
			uint32_t w = size;
			while (w != 1)
			{
				++ numMipMaps;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}
		num_mip_maps_ = numMipMaps;

		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.DeviceFeatureLevel() <= D3D_FEATURE_LEVEL_9_3)
		{
			if (!re.DeviceCaps().full_npot_texture_support
				&& (num_mip_maps_ > 1) && ((size & (size - 1)) != 0))
			{
				// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
				num_mip_maps_ = 1;
			}

			if ((num_mip_maps_ > 1) && IsCompressedFormat(format))
			{
				// height or width is not a multiply of 4 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
				uint32_t clamped_num_mip_maps;
				for (clamped_num_mip_maps = 0; clamped_num_mip_maps < num_mip_maps_; ++ clamped_num_mip_maps)
				{
					uint32_t s = std::max<uint32_t>(1U, size >> clamped_num_mip_maps);
					if ((s & 0x3) != 0)
					{
						break;
					}
				}
				num_mip_maps_ = clamped_num_mip_maps;
			}
		}

		array_size_ = array_size;
		format_		= format;
		dxgi_fmt_ = D3D11Mapping::MappingFormat(format_);

		width_ = size;
	}

	uint32_t D3D11TextureCube::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return std::max<uint32_t>(1U, width_ >> level);
	}

	uint32_t D3D11TextureCube::Height(uint32_t level) const
	{
		return this->Width(level);
	}

	void D3D11TextureCube::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11TextureCube& other(*checked_cast<D3D11TextureCube*>(&target));

		if ((this->Width(0) == target.Width(0)) && (this->Format() == target.Format())
			&& (this->ArraySize() == target.ArraySize()) && (this->NumMipMaps() == target.NumMipMaps()))
		{
			d3d_imm_ctx_->CopyResource(other.d3d_texture_.get(), d3d_texture_.get());
		}
		else
		{
			uint32_t const array_size = std::min(this->ArraySize(), target.ArraySize());
			uint32_t const num_mips = std::min(this->NumMipMaps(), target.NumMipMaps());
			for (uint32_t index = 0; index < array_size; ++ index)
			{
				for (int f = 0; f < 6; ++ f)
				{
					CubeFaces const face = static_cast<CubeFaces>(f);
					for (uint32_t level = 0; level < num_mips; ++ level)
					{
						this->ResizeTextureCube(target, index, face, level, 0, 0, target.Width(level), target.Height(level),
							index, face, level, 0, 0, this->Width(level), this->Height(level), true);
					}
				}
			}
		}
	}

	void D3D11TextureCube::CopyToSubTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		BOOST_ASSERT((TT_2D == target.Type()) || (TT_Cube == target.Type()));

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

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DResource(), D3D11CalcSubresource(dst_level, dst_array_index, target.NumMipMaps()),
				dst_x_offset, dst_y_offset, 0, this->D3DResource(), D3D11CalcSubresource(src_level, src_array_index, this->NumMipMaps()), &src_box);
		}
		else
		{
			this->ResizeTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
		}
	}

	void D3D11TextureCube::CopyToSubTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height)
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

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DResource(), D3D11CalcSubresource(dst_level, dst_array_index * 6 + dst_face - CF_Positive_X, target.NumMipMaps()),
				dst_x_offset, dst_y_offset, 0, this->D3DResource(), D3D11CalcSubresource(src_level, src_array_index * 6 + src_face - CF_Positive_X, this->NumMipMaps()), &src_box);
		}
		else
		{
			this->ResizeTextureCube(target, dst_array_index, dst_face, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
				src_array_index, src_face, src_level, src_x_offset, src_y_offset, src_width, src_height, true);
		}
	}

	ID3D11ShaderResourceViewPtr const & D3D11TextureCube::RetriveD3DShaderResourceView(uint32_t first_array_index, uint32_t num_items,
			uint32_t first_level, uint32_t num_levels)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Read);
		BOOST_ASSERT(0 == first_array_index);
		BOOST_ASSERT(1 == num_items);
		KFL_UNUSED(first_array_index);
		KFL_UNUSED(num_items);

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
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
			desc.Format = dxgi_fmt_;
			break;
		}

		desc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MostDetailedMip = first_level;
		desc.TextureCube.MipLevels = num_levels;

		return this->RetriveD3DSRV(desc);
	}

	ID3D11UnorderedAccessViewPtr const & D3D11TextureCube::RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items,
			uint32_t level)
	{
		return this->RetriveD3DUnorderedAccessView(first_array_index, num_items, CF_Positive_X, 6, level);
	}

	ID3D11UnorderedAccessViewPtr const & D3D11TextureCube::RetriveD3DUnorderedAccessView(uint32_t first_array_index, uint32_t num_items,
			CubeFaces first_face, uint32_t num_faces, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Read);

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = dxgi_fmt_;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.FirstArraySlice = first_array_index * 6 + first_face;
		desc.Texture2DArray.ArraySize = num_items * 6 + num_faces;

		return this->RetriveD3DUAV(desc);
	}

	ID3D11RenderTargetViewPtr const & D3D11TextureCube::RetriveD3DRenderTargetView(uint32_t first_array_index, uint32_t array_size, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(first_array_index < this->ArraySize());
		BOOST_ASSERT(first_array_index + array_size <= this->ArraySize());

		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(this->Format());
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		}
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
		desc.Texture2DArray.ArraySize = array_size * 6;

		return this->RetriveD3DRTV(desc);
	}

	ID3D11RenderTargetViewPtr const & D3D11TextureCube::RetriveD3DRenderTargetView(uint32_t array_index, Texture::CubeFaces face, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);

		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(this->Format());
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		}
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.FirstArraySlice = array_index * 6 + face - CF_Positive_X;
		desc.Texture2DArray.ArraySize = 1;

		return this->RetriveD3DRTV(desc);
	}

	ID3D11DepthStencilViewPtr const & D3D11TextureCube::RetriveD3DDepthStencilView(uint32_t first_array_index, uint32_t array_size, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(first_array_index < this->ArraySize());
		BOOST_ASSERT(first_array_index + array_size <= this->ArraySize());

		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(this->Format());
		desc.Flags = 0;
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		}
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
		desc.Texture2DArray.ArraySize = array_size * 6;

		return this->RetriveD3DDSV(desc);
	}

	ID3D11DepthStencilViewPtr const & D3D11TextureCube::RetriveD3DDepthStencilView(uint32_t array_index, Texture::CubeFaces face, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);

		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(this->Format());
		desc.Flags = 0;
		if (this->SampleCount() > 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		}
		desc.Texture2DArray.MipSlice = level;
		desc.Texture2DArray.FirstArraySlice = array_index * 6 + face - CF_Positive_X;
		desc.Texture2DArray.ArraySize = 1;

		return this->RetriveD3DDSV(desc);
	}

	void D3D11TextureCube::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t /*width*/, uint32_t /*height*/,
			void*& data, uint32_t& row_pitch)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		TIF(d3d_imm_ctx_->Map(d3d_texture_.get(), D3D11CalcSubresource(level, array_index * 6 + face - CF_Positive_X, num_mip_maps_),
			D3D11Mapping::Mapping(tma, type_, access_hint_, num_mip_maps_), 0, &mapped));
		uint8_t* p = static_cast<uint8_t*>(mapped.pData);
		data = p + y_offset * mapped.RowPitch + x_offset * NumFormatBytes(format_);
		row_pitch = mapped.RowPitch;
	}

	void D3D11TextureCube::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
	{
		d3d_imm_ctx_->Unmap(d3d_texture_.get(), D3D11CalcSubresource(level, array_index * 6 + face - CF_Positive_X, num_mip_maps_));
	}

	void D3D11TextureCube::BuildMipSubLevels()
	{
		if (d3d_sr_views_.empty())
		{
			for (uint32_t index = 0; index < this->ArraySize(); ++ index)
			{
				for (int f = 0; f < 6; ++ f)
				{
					CubeFaces const face = static_cast<CubeFaces>(f);
					for (uint32_t level = 1; level < this->NumMipMaps(); ++ level)
					{
						this->ResizeTextureCube(*this, index, face, level, 0, 0, this->Width(level), this->Height(level),
							index, face, level - 1, 0, 0, this->Width(level - 1), this->Height(level - 1), true);
					}
				}
			}
		}
		else
		{
			BOOST_ASSERT(access_hint_ & EAH_Generate_Mips);
			d3d_imm_ctx_->GenerateMips(d3d_sr_views_.begin()->second.get());
		}
	}

	void D3D11TextureCube::CreateHWResource(ElementInitData const * init_data)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = width_;
		desc.Height = width_;
		desc.MipLevels = num_mip_maps_;
		desc.ArraySize = 6 * array_size_;
		desc.Format = D3D11Mapping::MappingFormat(format_);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		this->GetD3DFlags(desc.Usage, desc.BindFlags, desc.CPUAccessFlags, desc.MiscFlags);
		desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

		std::vector<D3D11_SUBRESOURCE_DATA> subres_data(6 * num_mip_maps_);
		if (init_data != nullptr)
		{
			for (int face = 0; face < 6; ++ face)
			{
				for (uint32_t i = 0; i < num_mip_maps_; ++ i)
				{
					subres_data[face * num_mip_maps_ + i].pSysMem = init_data[face * num_mip_maps_ + i].data;
					subres_data[face * num_mip_maps_ + i].SysMemPitch = init_data[face * num_mip_maps_ + i].row_pitch;
					subres_data[face * num_mip_maps_ + i].SysMemSlicePitch = init_data[face * num_mip_maps_ + i].slice_pitch;
				}
			}
		}

		ID3D11Texture2D* d3d_tex;
		TIF(d3d_device_->CreateTexture2D(&desc, (init_data != nullptr) ? &subres_data[0] : nullptr, &d3d_tex));
		d3d_texture_ = MakeCOMPtr(d3d_tex);

		if ((access_hint_ & (EAH_GPU_Read | EAH_Generate_Mips)) && (num_mip_maps_ > 1))
		{
			this->RetriveD3DShaderResourceView(0, array_size_, 0, num_mip_maps_);
		}
	}
}
