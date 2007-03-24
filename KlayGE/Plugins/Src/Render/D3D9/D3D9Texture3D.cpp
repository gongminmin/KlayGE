// D3D9Texture3D.cpp
// KlayGE D3D9 3D纹理类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.4.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <d3dx9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "d3d9.lib")
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx9d.lib")
#else
	#pragma comment(lib, "d3dx9.lib")
#endif
#endif

namespace KlayGE
{
	D3D9Texture3D::D3D9Texture3D(uint32_t width, uint32_t height, uint32_t depth,
								uint16_t numMipMaps, ElementFormat format)
					: D3D9Texture(TT_3D),
						auto_gen_mipmaps_(false)
	{
		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3dDevice_ = renderEngine.D3DDevice();

		numMipMaps_ = numMipMaps;
		format_		= format;
		widths_.assign(1, width);
		heights_.assign(1, height);
		depths_.assign(1, depth);

		bpp_ = NumFormatBits(format);

		d3dTexture3D_ = this->CreateTexture3D(0, D3DPOOL_MANAGED);

		this->QueryBaseTexture();
		this->UpdateParams();
	}

	uint32_t D3D9Texture3D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return widths_[level];
	}

	uint32_t D3D9Texture3D::Height(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return heights_[level];
	}

	uint32_t D3D9Texture3D::Depth(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return depths_[level];
	}

	void D3D9Texture3D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D9Texture3D& other(*checked_cast<D3D9Texture3D*>(&target));

		BOOST_ASSERT(target.Depth(0) == depths_[0]);
		BOOST_ASSERT(target.Type() == type_);

		uint32_t maxLevel = 1;
		if (this->NumMipMaps() == target.NumMipMaps())
		{
			maxLevel = this->NumMipMaps();
		}

		DWORD filter = D3DX_FILTER_LINEAR;
		if (IsSRGB(format_))
		{
			filter |= D3DX_FILTER_SRGB_IN;
		}
		if (IsSRGB(target.Format()))
		{
			filter |= D3DX_FILTER_SRGB_OUT;
		}

		ID3D9VolumePtr src, dst;
		for (uint32_t level = 0; level < maxLevel; ++ level)
		{
			IDirect3DVolume9* temp;

			TIF(d3dTexture3D_->GetVolumeLevel(level, &temp));
			src = MakeCOMPtr(temp);

			TIF(other.d3dTexture3D_->GetVolumeLevel(level, &temp));
			dst = MakeCOMPtr(temp);

			TIF(D3DXLoadVolumeFromVolume(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));
		}

		if (this->NumMipMaps() != target.NumMipMaps())
		{
			target.BuildMipSubLevels();
		}		
	}

	void D3D9Texture3D::CopyToMemory3D(int level, void* data)
	{
		BOOST_ASSERT(level < numMipMaps_);
		BOOST_ASSERT(data != NULL);
		BOOST_ASSERT(TT_3D == type_);

		ID3D9VolumePtr volume;
		{
			IDirect3DVolume9* tmp_vol;
			TIF(d3dTexture3D_->GetVolumeLevel(level, &tmp_vol));
			volume = MakeCOMPtr(tmp_vol);
		}

		this->CopyVolumeToMemory(volume, data);
	}

	void D3D9Texture3D::CopyMemoryToTexture3D(int level, void const * data, ElementFormat pf,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t dst_xOffset, uint32_t dst_yOffset, uint32_t dst_zOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth)
	{
		BOOST_ASSERT(TT_3D == type_);

		IDirect3DVolume9* temp;
		TIF(d3dTexture3D_->GetVolumeLevel(level, &temp));
		ID3D9VolumePtr volume = MakeCOMPtr(temp);

		if (volume)
		{
			DWORD filter = D3DX_FILTER_LINEAR;
			if (IsSRGB(pf))
			{
				filter |= D3DX_FILTER_SRGB_IN;
			}
			if (IsSRGB(format_))
			{
				filter |= D3DX_FILTER_SRGB_OUT;
			}

			uint32_t const srcRowPitch = src_width * NumFormatBytes(pf);
			uint32_t const srcSlicePitch = srcRowPitch * src_height;

			D3DBOX srcBox = { 0, 0, src_width, src_height, 0, src_depth };
			D3DBOX dstBox = { dst_xOffset, dst_yOffset, dst_xOffset + dst_width, dst_yOffset + dst_height,
				dst_zOffset, dst_zOffset + dst_depth };
			TIF(D3DXLoadVolumeFromMemory(volume.get(), NULL, &dstBox, data, D3D9Mapping::MappingFormat(pf),
					srcRowPitch, srcSlicePitch, NULL, &srcBox, filter, 0));
		}
	}

	void D3D9Texture3D::BuildMipSubLevels()
	{
		ID3D9BaseTexturePtr d3dBaseTexture;

		if (auto_gen_mipmaps_)
		{
			d3dTexture3D_->GenerateMipSubLevels();
		}
		else
		{
			DWORD filter = D3DX_FILTER_NONE;
			if (IsSRGB(format_))
			{
				filter |= D3DX_FILTER_SRGB;
			}

			if (TU_RenderTarget == usage_)
			{
				ID3D9VolumeTexturePtr d3dTexture3D = this->CreateTexture3D(D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);

				IDirect3DVolume9* temp;
				TIF(d3dTexture3D_->GetVolumeLevel(0, &temp));
				ID3D9VolumePtr src = MakeCOMPtr(temp);

				TIF(d3dTexture3D->GetVolumeLevel(0, &temp));
				ID3D9VolumePtr dst = MakeCOMPtr(temp);

				TIF(D3DXLoadVolumeFromVolume(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));

				d3dTexture3D->GenerateMipSubLevels();
				d3dTexture3D_ = d3dTexture3D;

				auto_gen_mipmaps_ = true;
			}
			else
			{
				ID3D9VolumeTexturePtr d3dTexture3D = this->CreateTexture3D(0, D3DPOOL_SYSTEMMEM);

				IDirect3DBaseTexture9* base;
				d3dTexture3D->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&base));
				d3dBaseTexture = MakeCOMPtr(base);

				IDirect3DVolume9* temp;
				TIF(d3dTexture3D_->GetVolumeLevel(0, &temp));
				ID3D9VolumePtr src = MakeCOMPtr(temp);

				TIF(d3dTexture3D->GetVolumeLevel(0, &temp));
				ID3D9VolumePtr dst = MakeCOMPtr(temp);

				TIF(D3DXLoadVolumeFromVolume(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));

				TIF(D3DXFilterTexture(d3dBaseTexture.get(), NULL, 0, filter));
				TIF(d3dDevice_->UpdateTexture(d3dBaseTexture.get(), d3dBaseTexture_.get()));
			}
		}
	}

	void D3D9Texture3D::DoOnLostDevice()
	{
		if (TU_RenderTarget == usage_)
		{
			d3dBaseTexture_.reset();
			d3dTexture3D_.reset();
		}
	}

	void D3D9Texture3D::DoOnResetDevice()
	{
		if (TU_RenderTarget == usage_)
		{
			d3dTexture3D_ = this->CreateTexture3D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
			this->QueryBaseTexture();
		}
	}

	ID3D9VolumeTexturePtr D3D9Texture3D::CreateTexture3D(uint32_t usage, D3DPOOL pool)
	{
		if (IsDepthFormat(format_))
		{
			usage |= D3DUSAGE_DEPTHSTENCIL;
		}

		IDirect3DVolumeTexture9* d3dTexture3D;
		TIF(d3dDevice_->CreateVolumeTexture(widths_[0], heights_[0], depths_[0],
			numMipMaps_, usage, D3D9Mapping::MappingFormat(format_),
			pool, &d3dTexture3D, NULL));
		return MakeCOMPtr(d3dTexture3D);
	}

	void D3D9Texture3D::QueryBaseTexture()
	{
		IDirect3DBaseTexture9* d3dBaseTexture = NULL;
		d3dTexture3D_->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&d3dBaseTexture));
		d3dBaseTexture_ = MakeCOMPtr(d3dBaseTexture);
	}

	void D3D9Texture3D::UpdateParams()
	{
		D3DVOLUME_DESC desc;
		std::memset(&desc, 0, sizeof(desc));

		numMipMaps_ = static_cast<uint16_t>(d3dTexture3D_->GetLevelCount());
		BOOST_ASSERT(numMipMaps_ != 0);

		widths_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		depths_.resize(numMipMaps_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			TIF(d3dTexture3D_->GetLevelDesc(level, &desc));

			widths_[level] = desc.Width;
			heights_[level] = desc.Height;
			depths_[level] = desc.Depth;
		}

		bool srgb = IsSRGB(format_);
		format_ = D3D9Mapping::MappingFormat(desc.Format);
		if (srgb)
		{
			format_ = MakeSRGB(format_);
		}

		bpp_	= NumFormatBits(format_);
	}

	void D3D9Texture3D::Usage(TextureUsage usage)
	{
		if (usage != usage_)
		{
			ID3D9VolumeTexturePtr d3dTmpTexture3D;
			switch (usage)
			{
			case TU_Default:
				d3dTmpTexture3D = this->CreateTexture3D(0, D3DPOOL_MANAGED);
				break;
				
			case TU_RenderTarget:
				d3dTmpTexture3D = this->CreateTexture3D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
				break;
			}

			DWORD filter = D3DX_FILTER_NONE;
			if (IsSRGB(format_))
			{
				filter |= D3DX_FILTER_SRGB;
			}

			ID3D9VolumePtr src_vol, dest_vol;
			for (uint32_t i = 0; i < d3dTexture3D_->GetLevelCount(); ++ i)
			{
				IDirect3DVolume9* pSrcVol;
				d3dTexture3D_->GetVolumeLevel(i, &pSrcVol);
				src_vol = MakeCOMPtr(pSrcVol);

				IDirect3DVolume9* pDestVol;
				d3dTmpTexture3D->GetVolumeLevel(i, &pDestVol);
				dest_vol = MakeCOMPtr(pDestVol);

				TIF(D3DXLoadVolumeFromVolume(dest_vol.get(), NULL, NULL,
					src_vol.get(), NULL, NULL, filter, 0));
			}
			d3dTexture3D_ = d3dTmpTexture3D;

			this->QueryBaseTexture();
			this->UpdateParams();

			usage_ = usage;
		}
	}
}
