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
#include <d3d11.h>
#include <d3dx11.h>

#include <KlayGE/D3D11/D3D11Typedefs.hpp>
#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx11.lib")
#else
	#pragma comment(lib, "d3dx11.lib")
#endif
#endif

namespace KlayGE
{
	D3D11Texture2D::D3D11Texture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: D3D11Texture(TT_2D, sample_count, sample_quality, access_hint)
	{
		if (0 == numMipMaps)
		{
			numMipMaps_ = 1;
			uint32_t w = width;
			uint32_t h = height;
			while ((w != 1) || (h != 1))
			{
				++ numMipMaps_;

				w = std::max(static_cast<uint32_t>(1), w / 2);
				h = std::max(static_cast<uint32_t>(1), h / 2);
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.DeviceFeatureLevel() <= D3D_FEATURE_LEVEL_9_3)
		{
			if ((numMipMaps_ > 1) && (((width & (width - 1)) != 0) || ((height & (height - 1)) != 0)))
			{
				// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
				numMipMaps_ = 1;
			}
		}

		array_size_ = array_size;
		format_		= format;
		widthes_.assign(1, width);
		heights_.assign(1, height);

		bpp_ = NumFormatBits(format);

		desc_.Width = width;
		desc_.Height = height;
		desc_.MipLevels = numMipMaps_;
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

		std::vector<D3D11_SUBRESOURCE_DATA> subres_data(array_size_ * numMipMaps_);
		if (init_data != NULL)
		{
			for (uint32_t j = 0; j < array_size_; ++ j)
			{
				for (uint32_t i = 0; i < numMipMaps_; ++ i)
				{
					subres_data[j * numMipMaps_ + i].pSysMem = init_data[j * numMipMaps_ + i].data;
					subres_data[j * numMipMaps_ + i].SysMemPitch = init_data[j * numMipMaps_ + i].row_pitch;
					subres_data[j * numMipMaps_ + i].SysMemSlicePitch = init_data[j * numMipMaps_ + i].slice_pitch;
				}
			}
		}

		ID3D11Texture2D* d3d_tex;
		TIF(d3d_device_->CreateTexture2D(&desc_, (init_data != NULL) ? &subres_data[0] : NULL, &d3d_tex));
		d3dTexture2D_ = MakeCOMPtr(d3d_tex);

		if (access_hint_ & EAH_GPU_Read)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
			switch (format_)
			{
			case EF_D16:
				sr_desc.Format = DXGI_FORMAT_R16_UNORM;
				break;

			case EF_D24S8:
				sr_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				break;

			case EF_D32F:
				sr_desc.Format = DXGI_FORMAT_R32_FLOAT;
				break;

			default:
				sr_desc.Format = desc_.Format;
				break;
			}

			if (array_size_ > 1)
			{
				if (sample_count > 1)
				{
					sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
				}
				else
				{
					sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				}
				sr_desc.Texture2DArray.MostDetailedMip = 0;
				sr_desc.Texture2DArray.MipLevels = numMipMaps_;
				sr_desc.Texture2DArray.ArraySize = array_size_;
				sr_desc.Texture2DArray.FirstArraySlice = 0;
			}
			else
			{
				if (sample_count > 1)
				{
					sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				}
				sr_desc.Texture2D.MostDetailedMip = 0;
				sr_desc.Texture2D.MipLevels = numMipMaps_;
			}

			ID3D11ShaderResourceView* d3d_sr_view;
			d3d_device_->CreateShaderResourceView(d3dTexture2D_.get(), &sr_desc, &d3d_sr_view);
			d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
		}

		if (access_hint_ & EAH_GPU_Unordered)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
			uav_desc.Format = desc_.Format;
			if (array_size_ > 1)
			{
				uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
				uav_desc.Texture2DArray.ArraySize = array_size_;
				uav_desc.Texture2DArray.FirstArraySlice = 0;
				uav_desc.Texture2DArray.MipSlice = 0;
			}
			else
			{
				uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
				uav_desc.Texture2D.MipSlice = 0;
			}

			ID3D11UnorderedAccessView* d3d_ua_view;
			TIF(d3d_device_->CreateUnorderedAccessView(d3dTexture2D_.get(), &uav_desc, &d3d_ua_view));
			d3d_ua_view_ = MakeCOMPtr(d3d_ua_view);
		}

		this->UpdateParams();
	}

	uint32_t D3D11Texture2D::Width(int level) const
	{
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return widthes_[level];
	}

	uint32_t D3D11Texture2D::Height(int level) const
	{
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return heights_[level];
	}

	uint32_t D3D11Texture2D::Depth(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return 1;
	}

	void D3D11Texture2D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture2D& other(*checked_cast<D3D11Texture2D*>(&target));

		if ((this->Width(0) == target.Width(0)) && (this->Height(0) == target.Height(0)) && (this->Format() == target.Format())
			&& (this->NumMipMaps() == target.NumMipMaps()))
		{
			if ((this->SampleCount() > 1) && target.SampleCount() == 1)
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

			D3DX11LoadTextureFromTexture(d3d_imm_ctx_.get(), d3dTexture2D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D11Texture2D::CopyToTexture2D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture2D& other(*checked_cast<D3D11Texture2D*>(&target));

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			D3D11_BOX src_box;
			src_box.left = src_xOffset;
			src_box.top = src_yOffset;
			src_box.front = 0;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = src_yOffset + src_height;
			src_box.back = 1;

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DTexture().get(), D3D11CalcSubresource(level, 0, other.NumMipMaps()),
				dst_xOffset, dst_yOffset, 0, d3dTexture2D_.get(), D3D11CalcSubresource(level, 0, this->NumMipMaps()), &src_box);
		}
		else
		{
			D3D11_BOX src_box, dst_box;

			src_box.left = src_xOffset;
			src_box.top = src_yOffset;
			src_box.front = 0;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = src_yOffset + src_height;
			src_box.back = 1;

			dst_box.left = dst_xOffset;
			dst_box.top = dst_yOffset;
			dst_box.front = 0;
			dst_box.right = dst_xOffset + dst_width;
			dst_box.bottom = dst_yOffset + dst_height;
			dst_box.back = 1;

			D3DX11_TEXTURE_LOAD_INFO info;
			info.pSrcBox = &src_box;
			info.pDstBox = &dst_box;
			info.SrcFirstMip = D3D11CalcSubresource(level, 0, this->NumMipMaps());
			info.DstFirstMip = D3D11CalcSubresource(level, 0, other.NumMipMaps());
			info.NumMips = 1;
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

			D3DX11LoadTextureFromTexture(d3d_imm_ctx_.get(), d3dTexture2D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D11Texture2D::CopyToTextureCube(Texture& target, CubeFaces face, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset)
	{
		BOOST_ASSERT(TT_Cube == target.Type());

		D3D11TextureCube& other(*checked_cast<D3D11TextureCube*>(&target));

		if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
		{
			D3D11_BOX src_box;
			src_box.left = src_xOffset;
			src_box.top = src_yOffset;
			src_box.front = 0;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = src_yOffset + src_height;
			src_box.back = 1;

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DTexture().get(), D3D11CalcSubresource(level, face, other.NumMipMaps()),
				dst_xOffset, dst_yOffset, 0, d3dTexture2D_.get(), D3D11CalcSubresource(level, 0, this->NumMipMaps()), &src_box);
		}
		else
		{
			D3D11_BOX src_box, dst_box;

			src_box.left = src_xOffset;
			src_box.top = src_yOffset;
			src_box.front = 0;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = src_yOffset + src_height;
			src_box.back = 1;

			dst_box.left = dst_xOffset;
			dst_box.top = dst_yOffset;
			dst_box.front = 0;
			dst_box.right = dst_xOffset + dst_width;
			dst_box.bottom = dst_yOffset + dst_height;
			dst_box.back = 1;

			D3DX11_TEXTURE_LOAD_INFO info;
			info.pSrcBox = &src_box;
			info.pDstBox = &dst_box;
			info.SrcFirstMip = D3D11CalcSubresource(level, 0, this->NumMipMaps());
			info.DstFirstMip = D3D11CalcSubresource(level, face, other.NumMipMaps());
			info.NumMips = 1;
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

			D3DX11LoadTextureFromTexture(d3d_imm_ctx_.get(), d3dTexture2D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D11Texture2D::Map2D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t /*width*/, uint32_t /*height*/,
			void*& data, uint32_t& row_pitch)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		TIF(d3d_imm_ctx_->Map(d3dTexture2D_.get(), D3D11CalcSubresource(level, 0, numMipMaps_), D3D11Mapping::Mapping(tma, type_, access_hint_, numMipMaps_), 0, &mapped));
		uint8_t* p = static_cast<uint8_t*>(mapped.pData);
		data = p + (y_offset * mapped.RowPitch + x_offset) * NumFormatBytes(format_);
		row_pitch = mapped.RowPitch;
	}

	void D3D11Texture2D::Unmap2D(int level)
	{
		d3d_imm_ctx_->Unmap(d3dTexture2D_.get(), D3D11CalcSubresource(level, 0, numMipMaps_));
	}

	void D3D11Texture2D::BuildMipSubLevels()
	{
		if (d3d_sr_view_)
		{
			BOOST_ASSERT(access_hint_ & EAH_Generate_Mips);
			d3d_imm_ctx_->GenerateMips(d3d_sr_view_.get());
		}
		else
		{
			D3DX11FilterTexture(d3d_imm_ctx_.get(), d3dTexture2D_.get(), 0, D3DX11_FILTER_LINEAR);
		}
	}

	void D3D11Texture2D::UpdateParams()
	{
		d3dTexture2D_->GetDesc(&desc_);

		numMipMaps_ = desc_.MipLevels;
		array_size_ = desc_.ArraySize;
		BOOST_ASSERT(numMipMaps_ != 0);

		widthes_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		widthes_[0] = desc_.Width;
		heights_[0] = desc_.Height;
		for (uint32_t level = 1; level < numMipMaps_; ++ level)
		{
			widthes_[level] = widthes_[level - 1] / 2;
			heights_[level] = heights_[level - 1] / 2;
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
		bpp_	= NumFormatBits(format_);
	}
}
