// D3D10Texture3D.cpp
// KlayGE D3D10 3D纹理类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
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

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>
#include <d3dx10.h>

#include <KlayGE/D3D10/D3D10Typedefs.hpp>
#include <KlayGE/D3D10/D3D10RenderEngine.hpp>
#include <KlayGE/D3D10/D3D10Mapping.hpp>
#include <KlayGE/D3D10/D3D10Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx10d.lib")
#else
	#pragma comment(lib, "d3dx10.lib")
#endif
#endif

namespace KlayGE
{
	D3D10Texture3D::D3D10Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
						uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: D3D10Texture(TT_3D, sample_count, sample_quality, access_hint)
	{
		BOOST_ASSERT(1 == array_size);

		numMipMaps_ = numMipMaps;
		array_size_ = array_size;
		format_		= format;
		widthes_.assign(1, width);
		heights_.assign(1, height);
		depthes_.assign(1, depth);

		bpp_ = NumFormatBits(format);

		desc_.Width = width;
		desc_.Height = height;
		desc_.Depth = depth;
		desc_.MipLevels = numMipMaps_;
		desc_.Format = D3D10Mapping::MappingFormat(format_);

		this->GetD3DFlags(desc_.Usage, desc_.BindFlags, desc_.CPUAccessFlags, desc_.MiscFlags);

		std::vector<D3D10_SUBRESOURCE_DATA> subres_data(numMipMaps_);
		if (init_data != NULL)
		{
			for (uint32_t i = 0; i < numMipMaps_; ++ i)
			{
				subres_data[i].pSysMem = init_data[i].data;
				subres_data[i].SysMemPitch = init_data[i].row_pitch;
				subres_data[i].SysMemSlicePitch = init_data[i].slice_pitch;
			}
		}

		ID3D10Texture3D* d3d_tex;
		TIF(d3d_device_->CreateTexture3D(&desc_, (init_data != NULL) ? &subres_data[0] : NULL, &d3d_tex));
		d3dTexture3D_ = MakeCOMPtr(d3d_tex);

		if (access_hint_ & EAH_GPU_Read)
		{
			ID3D10ShaderResourceView* d3d_sr_view;
			d3d_device_->CreateShaderResourceView(d3dTexture3D_.get(), NULL, &d3d_sr_view);
			d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
		}

		this->UpdateParams();
	}

	uint32_t D3D10Texture3D::Width(int level) const
	{
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return widthes_[level];
	}

	uint32_t D3D10Texture3D::Height(int level) const
	{
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return heights_[level];
	}

	uint32_t D3D10Texture3D::Depth(int level) const
	{
		BOOST_ASSERT(static_cast<uint32_t>(level) < numMipMaps_);

		return depthes_[level];
	}

	void D3D10Texture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D10Texture3D& other(*checked_cast<D3D10Texture3D*>(&target));

		if ((this->Width(0) == target.Width(0)) && (this->Height(0) == target.Height(0))
			&& (this->Depth(0) == target.Depth(0)) && (this->Format() == target.Format())
			&& (this->NumMipMaps() == target.NumMipMaps()))
		{
			d3d_device_->CopyResource(other.D3DTexture().get(), d3dTexture3D_.get());
		}
		else
		{
			D3DX10_TEXTURE_LOAD_INFO info;
			info.pSrcBox = NULL;
			info.pDstBox = NULL;
			info.SrcFirstMip = D3D10CalcSubresource(0, 0, this->NumMipMaps());
			info.DstFirstMip = D3D10CalcSubresource(0, 0, target.NumMipMaps());
			info.NumMips = std::min(this->NumMipMaps(), target.NumMipMaps());
			info.SrcFirstElement = 0;
			info.DstFirstElement = 0;
			info.NumElements = 0;
			info.Filter = D3DX10_FILTER_LINEAR;
			info.MipFilter = D3DX10_FILTER_LINEAR;
			if (IsSRGB(format_))
			{
				info.Filter |= D3DX10_FILTER_SRGB_IN;
				info.MipFilter |= D3DX10_FILTER_SRGB_IN;
			}
			if (IsSRGB(target.Format()))
			{
				info.Filter |= D3DX10_FILTER_SRGB_OUT;
				info.MipFilter |= D3DX10_FILTER_SRGB_OUT;
			}

			D3DX10LoadTextureFromTexture(d3dTexture3D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D10Texture3D::CopyToTexture3D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			uint32_t src_xOffset, uint32_t src_yOffset, uint32_t src_zOffset)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D10Texture3D& other(*checked_cast<D3D10Texture3D*>(&target));

		if ((src_width == dst_width) && (src_height == dst_height) && (src_depth == dst_depth) && (this->Format() == target.Format()))
		{
			D3D10_BOX src_box;
			src_box.left = src_xOffset;
			src_box.top = src_yOffset;
			src_box.front = src_zOffset;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = src_yOffset + src_height;
			src_box.back = src_zOffset = src_depth;

			d3d_device_->CopySubresourceRegion(other.D3DTexture().get(), D3D10CalcSubresource(level, 0, other.NumMipMaps()),
				dst_xOffset, dst_yOffset, 0, d3dTexture3D_.get(), D3D10CalcSubresource(level, 0, this->NumMipMaps()), &src_box);
		}
		else
		{
			D3D10_BOX src_box, dst_box;

			src_box.left = src_xOffset;
			src_box.top = src_yOffset;
			src_box.front = src_zOffset;
			src_box.right = src_xOffset + src_width;
			src_box.bottom = src_yOffset + src_height;
			src_box.back = src_zOffset + src_depth;

			dst_box.left = dst_xOffset;
			dst_box.top = dst_yOffset;
			dst_box.front = dst_zOffset;
			dst_box.right = dst_xOffset + dst_width;
			dst_box.bottom = dst_yOffset + dst_height;
			dst_box.back = dst_zOffset + dst_depth;

			D3DX10_TEXTURE_LOAD_INFO info;
			info.pSrcBox = &src_box;
			info.pDstBox = &dst_box;
			info.SrcFirstMip = D3D10CalcSubresource(level, 0, this->NumMipMaps());
			info.DstFirstMip = D3D10CalcSubresource(level, 0, target.NumMipMaps());
			info.NumMips = 1;
			info.SrcFirstElement = 0;
			info.DstFirstElement = 0;
			info.NumElements = 0;
			info.Filter = D3DX10_FILTER_LINEAR;
			info.MipFilter = D3DX10_FILTER_LINEAR;
			if (IsSRGB(format_))
			{
				info.Filter |= D3DX10_FILTER_SRGB_IN;
				info.MipFilter |= D3DX10_FILTER_SRGB_IN;
			}
			if (IsSRGB(target.Format()))
			{
				info.Filter |= D3DX10_FILTER_SRGB_OUT;
				info.MipFilter |= D3DX10_FILTER_SRGB_OUT;
			}

			D3DX10LoadTextureFromTexture(d3dTexture3D_.get(), &info, other.D3DTexture().get());
		}
	}

	void D3D10Texture3D::Map3D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
	{
		D3D10_MAPPED_TEXTURE3D mapped;
		TIF(d3dTexture3D_->Map(D3D10CalcSubresource(level, 0, numMipMaps_), D3D10Mapping::Mapping(tma, type_, access_hint_, numMipMaps_), 0, &mapped));
		uint8_t* p = static_cast<uint8_t*>(mapped.pData);
		data = p + (z_offset * mapped.DepthPitch + y_offset * mapped.RowPitch + x_offset) * NumFormatBytes(format_);
		row_pitch = mapped.RowPitch;
		slice_pitch = mapped.DepthPitch;
	}

	void D3D10Texture3D::Unmap3D(int level)
	{
		d3dTexture3D_->Unmap(D3D10CalcSubresource(level, 0, numMipMaps_));
	}

	void D3D10Texture3D::BuildMipSubLevels()
	{
		if (d3d_sr_view_)
		{
			if (!(desc_.MiscFlags & D3D10_RESOURCE_MISC_GENERATE_MIPS))
			{
				desc_.MiscFlags |= D3D10_RESOURCE_MISC_GENERATE_MIPS;

				ID3D10Texture3D* d3d_tex;
				TIF(d3d_device_->CreateTexture3D(&desc_, NULL, &d3d_tex));

				d3d_device_->CopyResource(d3d_tex, d3dTexture3D_.get());

				d3dTexture3D_ = MakeCOMPtr(d3d_tex);

				ID3D10ShaderResourceView* d3d_sr_view;
				d3d_device_->CreateShaderResourceView(d3dTexture3D_.get(), NULL, &d3d_sr_view);
				d3d_sr_view_ = MakeCOMPtr(d3d_sr_view);
			}

			d3d_device_->GenerateMips(d3d_sr_view_.get());
		}
		else
		{
			D3DX10FilterTexture(d3dTexture3D_.get(), 0, D3DX10_FILTER_LINEAR);
		}
	}

	void D3D10Texture3D::UpdateParams()
	{
		d3dTexture3D_->GetDesc(&desc_);

		numMipMaps_ = desc_.MipLevels;
		BOOST_ASSERT(numMipMaps_ != 0);

		widthes_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		depthes_.resize(numMipMaps_);
		widthes_[0] = desc_.Width;
		heights_[0] = desc_.Height;
		depthes_[0] = desc_.Depth;
		for (uint16_t level = 1; level < numMipMaps_; ++ level)
		{
			widthes_[level] = widthes_[level - 1] / 2;
			heights_[level] = heights_[level - 1] / 2;
			depthes_[level] = depthes_[level - 1] / 2;
		}

		format_ = D3D10Mapping::MappingFormat(desc_.Format);
		bpp_	= NumFormatBits(format_);
	}
}
