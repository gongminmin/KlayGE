// D3D11Texture1D.cpp
// KlayGE D3D11 1D纹理类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
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
	D3D11Texture1D::D3D11Texture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: D3D11Texture(TT_1D, sample_count, sample_quality, access_hint)
	{
		if (0 == numMipMaps)
		{
			numMipMaps_ = 1;
			uint32_t w = width;
			while (w != 1)
			{
				++ numMipMaps_;

				w = std::max(static_cast<uint32_t>(1), w / 2);
			}
		}
		else
		{
			numMipMaps_ = numMipMaps;
		}

		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.DeviceFeatureLevel() <= D3D_FEATURE_LEVEL_9_3)
		{
			if ((numMipMaps_ > 1) && ((width & (width - 1)) != 0))
			{
				// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
				numMipMaps_ = 1;
			}
		}

		array_size_ = array_size;
		format_		= format;
		widthes_.assign(1, width);

		bpp_ = NumFormatBits(format);

		desc_.Width = width;
		desc_.MipLevels = numMipMaps_;
		desc_.ArraySize = array_size_;
		desc_.Format = D3D11Mapping::MappingFormat(format_);

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

		ID3D11Texture1D* d3d_tex;
		TIF(d3d_device_->CreateTexture1D(&desc_, (init_data != NULL) ? &subres_data[0] : NULL, &d3d_tex));
		d3dTexture1D_ = MakeCOMPtr(d3d_tex);

		if (access_hint_ & EAH_GPU_Read)
		{
			ID3D11ShaderResourceView* d3d_sr_view;
			d3d_device_->CreateShaderResourceView(d3dTexture1D_.get(), NULL, &d3d_sr_view);
			d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
		}

		if (access_hint_ & EAH_GPU_Unordered)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
			uav_desc.Format = desc_.Format;
			if (array_size_ > 1)
			{
				uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
				uav_desc.Texture1DArray.ArraySize = array_size_;
				uav_desc.Texture1DArray.FirstArraySlice = 0;
				uav_desc.Texture1DArray.MipSlice = 0;
			}
			else
			{
				uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
				uav_desc.Texture1D.MipSlice = 0;
			}

			ID3D11UnorderedAccessView* d3d_ua_view;
			TIF(d3d_device_->CreateUnorderedAccessView(d3dTexture1D_.get(), &uav_desc, &d3d_ua_view));
			d3d_ua_view_ = MakeCOMPtr(d3d_ua_view);
		}

		this->UpdateParams();
	}

	uint32_t D3D11Texture1D::Width(int level) const
	{
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return widthes_[level];
	}

	uint32_t D3D11Texture1D::Height(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return 1;
	}

	uint32_t D3D11Texture1D::Depth(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return 1;
	}

	void D3D11Texture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture1D& other(*checked_cast<D3D11Texture1D*>(&target));

		if ((this->Width(0) == target.Width(0)) && (this->Format() == target.Format()) && (this->NumMipMaps() == target.NumMipMaps()))
		{
			d3d_imm_ctx_->CopyResource(other.D3DTexture().get(), d3dTexture1D_.get());
		}
		else
		{
			D3DX11_TEXTURE_LOAD_INFO info;
			info.pSrcBox = NULL;
			info.pDstBox = NULL;
			info.SrcFirstMip = D3D11CalcSubresource(0, 0, this->NumMipMaps());
			info.DstFirstMip = D3D11CalcSubresource(0, 0, target.NumMipMaps());
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

			D3DX11LoadTextureFromTexture(d3d_imm_ctx_.get(), d3dTexture1D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D11Texture1D::CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D11Texture1D& other(*checked_cast<D3D11Texture1D*>(&target));

		if ((src_width == dst_width) && (this->Format() == target.Format()))
		{
			D3D11_BOX src_box;
			src_box.left = src_xOffset;
			src_box.top = 0;
			src_box.front = 0;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = 1;
			src_box.back = 1;

			d3d_imm_ctx_->CopySubresourceRegion(other.D3DTexture().get(), D3D11CalcSubresource(level, 0, other.NumMipMaps()),
				dst_xOffset, 0, 0, d3dTexture1D_.get(), D3D11CalcSubresource(level, 0, this->NumMipMaps()), &src_box);
		}
		else
		{
			D3D11_BOX src_box, dst_box;

			src_box.left = src_xOffset;
			src_box.top = 0;
			src_box.front = 0;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = 1;
			src_box.back = 1;

			dst_box.left = dst_xOffset;
			dst_box.top = 0;
			dst_box.front = 0;
			dst_box.right = dst_xOffset + dst_width;
			dst_box.bottom = 1;
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

			D3DX11LoadTextureFromTexture(d3d_imm_ctx_.get(), d3dTexture1D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D11Texture1D::Map1D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t /*width*/,
			void*& data)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		TIF(d3d_imm_ctx_->Map(d3dTexture1D_.get(), D3D11CalcSubresource(level, 0, numMipMaps_), D3D11Mapping::Mapping(tma, type_, access_hint_, numMipMaps_), 0, &mapped));
		data = static_cast<uint8_t*>(mapped.pData) + x_offset * NumFormatBytes(format_);
	}

	void D3D11Texture1D::Unmap1D(int level)
	{
		d3d_imm_ctx_->Unmap(d3dTexture1D_.get(), D3D11CalcSubresource(level, 0, numMipMaps_));
	}

	void D3D11Texture1D::BuildMipSubLevels()
	{
		if (d3d_sr_view_)
		{
			if (!(desc_.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS))
			{
				desc_.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

				ID3D11Texture1D* d3d_tex;
				TIF(d3d_device_->CreateTexture1D(&desc_, NULL, &d3d_tex));

				d3d_imm_ctx_->CopyResource(d3d_tex, d3dTexture1D_.get());

				d3dTexture1D_ = MakeCOMPtr(d3d_tex);

				ID3D11ShaderResourceView* d3d_sr_view;
				d3d_device_->CreateShaderResourceView(d3dTexture1D_.get(), NULL, &d3d_sr_view);
				d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
			}

			d3d_imm_ctx_->GenerateMips(d3d_sr_view_.get());
		}
		else
		{
			D3DX11FilterTexture(d3d_imm_ctx_.get(), d3dTexture1D_.get(), 0, D3DX11_FILTER_LINEAR);
		}
	}

	void D3D11Texture1D::UpdateParams()
	{
		d3dTexture1D_->GetDesc(&desc_);

		numMipMaps_ = desc_.MipLevels;
		array_size_ = desc_.ArraySize;
		BOOST_ASSERT(numMipMaps_ != 0);

		widthes_.resize(numMipMaps_);
		widthes_[0] = desc_.Width;
		for (uint32_t level = 1; level < numMipMaps_; ++ level)
		{
			widthes_[level] = widthes_[level - 1] / 2;
		}

		format_ = D3D11Mapping::MappingFormat(desc_.Format);
		bpp_	= NumFormatBits(format_);
	}
}
