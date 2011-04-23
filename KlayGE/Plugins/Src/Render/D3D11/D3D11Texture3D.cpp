// D3D11Texture3D.cpp
// KlayGE D3D11 3D纹理类 实现文件
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
#include <d3d11.h>
#include <d3dx11.h>

#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx11d.lib")
#else
	#pragma comment(lib, "d3dx11.lib")
#endif
#endif

namespace KlayGE
{
	D3D11Texture3D::D3D11Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: D3D11Texture(TT_3D, sample_count, sample_quality, access_hint)
	{
		BOOST_ASSERT(1 == array_size);

		if (0 == numMipMaps)
		{
			num_mip_maps_ = 1;
			uint32_t w = width;
			uint32_t h = height;
			uint32_t d = depth;
			while ((w != 1) || (h != 1) || (d != 1))
			{
				++ num_mip_maps_;

				w = std::max<uint32_t>(1U, w / 2);
				h = std::max<uint32_t>(1U, h / 2);
				d = std::max<uint32_t>(1U, d / 2);
			}
		}
		else
		{
			num_mip_maps_ = numMipMaps;
		}

		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.DeviceFeatureLevel() <= D3D_FEATURE_LEVEL_9_3)
		{
			if ((num_mip_maps_ > 1) && (((width & (width - 1)) != 0) || ((height & (height - 1)) != 0) || ((depth & (depth - 1)) != 0)))
			{
				// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
				num_mip_maps_ = 1;
			}
		}

		array_size_ = array_size;
		format_		= format;
		widthes_.assign(1, width);
		heights_.assign(1, height);
		depthes_.assign(1, depth);

		desc_.Width = width;
		desc_.Height = height;
		desc_.Depth = depth;
		desc_.MipLevels = num_mip_maps_;
		desc_.Format = D3D11Mapping::MappingFormat(format_);

		this->GetD3DFlags(desc_.Usage, desc_.BindFlags, desc_.CPUAccessFlags, desc_.MiscFlags);

		std::vector<D3D11_SUBRESOURCE_DATA> subres_data(num_mip_maps_);
		if (init_data != NULL)
		{
			for (uint32_t i = 0; i < num_mip_maps_; ++ i)
			{
				subres_data[i].pSysMem = init_data[i].data;
				subres_data[i].SysMemPitch = init_data[i].row_pitch;
				subres_data[i].SysMemSlicePitch = init_data[i].slice_pitch;
			}
		}

		ID3D11Texture3D* d3d_tex;
		TIF(d3d_device_->CreateTexture3D(&desc_, (init_data != NULL) ? &subres_data[0] : NULL, &d3d_tex));
		d3dTexture3D_ = MakeCOMPtr(d3d_tex);

		if (access_hint_ & EAH_GPU_Read)
		{
			ID3D11ShaderResourceView* d3d_sr_view;
			d3d_device_->CreateShaderResourceView(d3dTexture3D_.get(), NULL, &d3d_sr_view);
			d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
		}

		if (access_hint_ & EAH_GPU_Unordered)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
			uav_desc.Format = desc_.Format;
			uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
			uav_desc.Texture3D.MipSlice = 0;

			ID3D11UnorderedAccessView* d3d_ua_view;
			TIF(d3d_device_->CreateUnorderedAccessView(d3dTexture3D_.get(), &uav_desc, &d3d_ua_view));
			d3d_ua_view_ = MakeCOMPtr(d3d_ua_view);
		}

		this->UpdateParams();
	}

	uint32_t D3D11Texture3D::Width(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return widthes_[level];
	}

	uint32_t D3D11Texture3D::Height(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return heights_[level];
	}

	uint32_t D3D11Texture3D::Depth(uint32_t level) const
	{
		BOOST_ASSERT(level < num_mip_maps_);

		return depthes_[level];
	}

	void D3D11Texture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture3D& other(*checked_cast<D3D11Texture3D*>(&target));

		if ((this->Width(0) == target.Width(0)) && (this->Height(0) == target.Height(0))
			&& (this->Depth(0) == target.Depth(0)) && (this->Format() == target.Format())
			&& (this->NumMipMaps() == target.NumMipMaps()))
		{
			d3d_imm_ctx_->CopyResource(other.D3DTexture().get(), d3dTexture3D_.get());
		}
		else
		{
			D3DX11_TEXTURE_LOAD_INFO info;
			info.pSrcBox = NULL;
			info.pDstBox = NULL;
			info.SrcFirstMip = D3D11CalcSubresource(0, 0, this->NumMipMaps());
			info.DstFirstMip = D3D11CalcSubresource(0, 0, other.NumMipMaps());
			info.NumMips = std::min(this->NumMipMaps(), target.NumMipMaps());
			info.SrcFirstElement = 0;
			info.DstFirstElement = 0;
			info.NumElements = 0;
			info.Filter = D3DX11_FILTER_LINEAR;
			info.MipFilter = D3DX11_FILTER_LINEAR;
			if (IsSRGB(format_))
			{
				info.Filter |= D3DX11_FILTER_SRGB_IN;
				info.MipFilter |= D3DX11_FILTER_SRGB_IN;
			}
			if (IsSRGB(target.Format()))
			{
				info.Filter |= D3DX11_FILTER_SRGB_OUT;
				info.MipFilter |= D3DX11_FILTER_SRGB_OUT;
			}

			D3DX11LoadTextureFromTexture(d3d_imm_ctx_.get(), d3dTexture3D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D11Texture3D::CopyToSubTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		BOOST_ASSERT(type_ == target.Type());

		this->CopyToSubTexture(target,
			D3D11CalcSubresource(dst_level, dst_array_index, target.NumMipMaps()), dst_x_offset, dst_y_offset, dst_z_offset, dst_width, dst_height, dst_depth,
			D3D11CalcSubresource(src_level, src_array_index, this->NumMipMaps()), src_x_offset, src_y_offset, src_z_offset, src_width, src_height, src_depth);
	}

	ID3D11RenderTargetViewPtr const & D3D11Texture3D::RetriveD3DRenderTargetView(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);
		BOOST_ASSERT(0 == array_index);
		UNREF_PARAM(array_index);

		RTVDSVCreation rtv_creation;
		memset(&rtv_creation, 0, sizeof(rtv_creation));
		rtv_creation.array_index = array_index;
		rtv_creation.level = level;
		rtv_creation.for_3d_or_cube.for_3d.first_slice = first_slice;
		rtv_creation.for_3d_or_cube.for_3d.num_slices = num_slices;
		for (size_t i = 0; i < d3d_rt_views_.size(); ++ i)
		{
			if (0 == memcmp(&d3d_rt_views_[i].first, &rtv_creation, sizeof(rtv_creation)))
			{
				return d3d_rt_views_[i].second;
			}
		}

		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3D11Mapping::MappingFormat(this->Format());
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = level;
		desc.Texture3D.FirstWSlice = first_slice;
		desc.Texture3D.WSize = num_slices;

		ID3D11RenderTargetView* rt_view;
		TIF(d3d_device_->CreateRenderTargetView(this->D3DTexture().get(), &desc, &rt_view));
		d3d_rt_views_.push_back(std::make_pair(rtv_creation, MakeCOMPtr(rt_view)));
		return d3d_rt_views_.back().second;
	}

	ID3D11DepthStencilViewPtr const & D3D11Texture3D::RetriveD3DDepthStencilView(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level)
	{
		BOOST_ASSERT(this->AccessHint() & EAH_GPU_Write);

		RTVDSVCreation dsv_creation;
		memset(&dsv_creation, 0, sizeof(dsv_creation));
		dsv_creation.array_index = array_index;
		dsv_creation.level = level;
		for (size_t i = 0; i < d3d_ds_views_.size(); ++ i)
		{
			if (0 == memcmp(&d3d_ds_views_[i].first, &dsv_creation, sizeof(dsv_creation)))
			{
				return d3d_ds_views_[i].second;
			}
		}

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
		desc.Texture2DArray.ArraySize = num_slices;
		desc.Texture2DArray.FirstArraySlice = first_slice;

		ID3D11DepthStencilView* ds_view;
		TIF(d3d_device_->CreateDepthStencilView(this->D3DTexture().get(), &desc, &ds_view));
		d3d_ds_views_.push_back(std::make_pair(dsv_creation, MakeCOMPtr(ds_view)));
		return d3d_ds_views_.back().second;
	}

	void D3D11Texture3D::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		TIF(d3d_imm_ctx_->Map(d3dTexture3D_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_), D3D11Mapping::Mapping(tma, type_, access_hint_, num_mip_maps_), 0, &mapped));
		uint8_t* p = static_cast<uint8_t*>(mapped.pData);
		data = p + z_offset * mapped.DepthPitch + y_offset * mapped.RowPitch + x_offset * NumFormatBytes(format_);
		row_pitch = mapped.RowPitch;
		slice_pitch = mapped.DepthPitch;
	}

	void D3D11Texture3D::Unmap3D(uint32_t array_index, uint32_t level)
	{
		d3d_imm_ctx_->Unmap(d3dTexture3D_.get(), D3D11CalcSubresource(level, array_index, num_mip_maps_));
	}

	void D3D11Texture3D::BuildMipSubLevels()
	{
		if (d3d_sr_view_)
		{
			BOOST_ASSERT(access_hint_ & EAH_Generate_Mips);
			d3d_imm_ctx_->GenerateMips(d3d_sr_view_.get());
		}
		else
		{
			D3DX11FilterTexture(d3d_imm_ctx_.get(), d3dTexture3D_.get(), 0, D3DX11_FILTER_LINEAR);
		}
	}

	void D3D11Texture3D::UpdateParams()
	{
		d3dTexture3D_->GetDesc(&desc_);

		num_mip_maps_ = desc_.MipLevels;
		BOOST_ASSERT(num_mip_maps_ != 0);

		widthes_.resize(num_mip_maps_);
		heights_.resize(num_mip_maps_);
		depthes_.resize(num_mip_maps_);
		widthes_[0] = desc_.Width;
		heights_[0] = desc_.Height;
		depthes_[0] = desc_.Depth;
		for (uint32_t level = 1; level < num_mip_maps_; ++ level)
		{
			widthes_[level] = std::max<uint32_t>(1U, widthes_[level - 1] / 2);
			heights_[level] = std::max<uint32_t>(1U, heights_[level - 1] / 2);
			depthes_[level] = std::max<uint32_t>(1U, depthes_[level - 1] / 2);
		}

		format_ = D3D11Mapping::MappingFormat(desc_.Format);
	}
}
