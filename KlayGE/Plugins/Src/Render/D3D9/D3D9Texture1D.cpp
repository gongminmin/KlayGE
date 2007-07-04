// D3D9Texture1D.cpp
// KlayGE D3D9 1D纹理类 实现文件
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
	D3D9Texture1D::D3D9Texture1D(uint32_t width, uint16_t numMipMaps, ElementFormat format)
					: D3D9Texture(TT_1D),
						auto_gen_mipmaps_(false)
	{
		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3dDevice_ = renderEngine.D3DDevice();

		numMipMaps_ = numMipMaps;
		format_		= format;
		widths_.assign(1, width);

		bpp_ = NumFormatBits(format);

		d3dTexture1D_ = this->CreateTexture1D(0, D3DPOOL_MANAGED);

		this->QueryBaseTexture();
		this->UpdateParams();
	}

	uint32_t D3D9Texture1D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return widths_[level];
	}

	void D3D9Texture1D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D9Texture1D& other(*checked_cast<D3D9Texture1D*>(&target));

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
				
		ID3D9SurfacePtr src, dst;
		for (uint32_t level = 0; level < maxLevel; ++ level)
		{
			IDirect3DSurface9* temp;

			TIF(d3dTexture1D_->GetSurfaceLevel(level, &temp));
			src = MakeCOMPtr(temp);

			TIF(other.d3dTexture1D_->GetSurfaceLevel(level, &temp));
			dst = MakeCOMPtr(temp);

			if (FAILED(d3dDevice_->StretchRect(src.get(), NULL, dst.get(), NULL, D3DTEXF_LINEAR)))
			{
				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));
			}
		}

		if (this->NumMipMaps() != target.NumMipMaps())
		{
			target.BuildMipSubLevels();
		}
	}

	void D3D9Texture1D::CopyToTexture1D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D9Texture1D& other(*checked_cast<D3D9Texture1D*>(&target));

		DWORD filter = D3DX_FILTER_LINEAR;
		if (IsSRGB(format_))
		{
			filter |= D3DX_FILTER_SRGB_IN;
		}
		if (IsSRGB(target.Format()))
		{
			filter |= D3DX_FILTER_SRGB_OUT;
		}
				
		ID3D9SurfacePtr src, dst;
		{
			IDirect3DSurface9* temp;

			TIF(d3dTexture1D_->GetSurfaceLevel(level, &temp));
			src = MakeCOMPtr(temp);

			TIF(other.d3dTexture1D_->GetSurfaceLevel(level, &temp));
			dst = MakeCOMPtr(temp);

			RECT srcRc = { src_xOffset, 0, src_xOffset + src_width, 1 };
			RECT dstRc = { dst_xOffset, 0, dst_xOffset + dst_width, 1 };
			if (FAILED(d3dDevice_->StretchRect(src.get(), &srcRc, dst.get(), &dstRc, D3DTEXF_LINEAR)))
			{
				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, &dstRc, src.get(), NULL, &srcRc, filter, 0));
			}
		}
	}

	void D3D9Texture1D::CopyToMemory1D(int level, void* data)
	{
		BOOST_ASSERT(level < numMipMaps_);
		BOOST_ASSERT(data != NULL);

		ID3D9SurfacePtr surface;
		{
			IDirect3DSurface9* tmp_surface;
			TIF(d3dTexture1D_->GetSurfaceLevel(level, &tmp_surface));
			surface = MakeCOMPtr(tmp_surface);
		}
		if (TU_RenderTarget == usage_)
		{
			IDirect3DSurface9* tmp_surface;
			TIF(d3dDevice_->CreateOffscreenPlainSurface(this->Width(level), this->Height(level),
				D3D9Mapping::MappingFormat(format_), D3DPOOL_SYSTEMMEM, &tmp_surface, NULL));

			DWORD filter = D3DX_FILTER_NONE;
			if (IsSRGB(format_))
			{
				filter |= D3DX_FILTER_SRGB;
			}

			TIF(D3DXLoadSurfaceFromSurface(tmp_surface, NULL, NULL, surface.get(), NULL, NULL, filter, 0));

			surface = MakeCOMPtr(tmp_surface);
		}

		this->CopySurfaceToMemory(surface, data);
	}

	void D3D9Texture1D::CopyMemoryToTexture1D(int level, void const * data, ElementFormat pf,
		uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width, uint32_t src_xOffset)
	{
		IDirect3DSurface9* temp;
		TIF(d3dTexture1D_->GetSurfaceLevel(level, &temp));
		ID3D9SurfacePtr surface = MakeCOMPtr(temp);

		if (surface)
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

			RECT srcRc = { src_xOffset, 0, src_xOffset + src_width, 1 };
			RECT dstRc = { dst_xOffset, 0, dst_xOffset + dst_width, 1 };
			TIF(D3DXLoadSurfaceFromMemory(surface.get(), NULL, &dstRc, data, D3D9Mapping::MappingFormat(pf),
					src_width * NumFormatBytes(pf), NULL, &srcRc, filter, 0));
		}
	}

	void D3D9Texture1D::BuildMipSubLevels()
	{
		ID3D9BaseTexturePtr d3dBaseTexture;

		if (auto_gen_mipmaps_)
		{
			d3dTexture1D_->GenerateMipSubLevels();
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
				ID3D9TexturePtr d3dTexture1D = this->CreateTexture1D(D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);

				IDirect3DSurface9* temp;
				TIF(d3dTexture1D_->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr src = MakeCOMPtr(temp);

				TIF(d3dTexture1D->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr dst = MakeCOMPtr(temp);

				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));

				d3dTexture1D->GenerateMipSubLevels();
				d3dTexture1D_ = d3dTexture1D;

				auto_gen_mipmaps_ = true;
			}
			else
			{
				ID3D9TexturePtr d3dTexture1D = this->CreateTexture1D(0, D3DPOOL_SYSTEMMEM);

				IDirect3DBaseTexture9* base;
				d3dTexture1D->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&base));
				d3dBaseTexture = MakeCOMPtr(base);

				IDirect3DSurface9* temp;
				TIF(d3dTexture1D_->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr src = MakeCOMPtr(temp);

				TIF(d3dTexture1D->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr dst = MakeCOMPtr(temp);

				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));

				TIF(D3DXFilterTexture(d3dBaseTexture.get(), NULL, 0, filter));
				TIF(d3dDevice_->UpdateTexture(d3dBaseTexture.get(), d3dBaseTexture_.get()));
			}
		}
	}

	void D3D9Texture1D::DoOnLostDevice()
	{
		if (TU_RenderTarget == usage_)
		{
			d3dBaseTexture_.reset();
			d3dTexture1D_.reset();
		}
	}

	void D3D9Texture1D::DoOnResetDevice()
	{
		if (TU_RenderTarget == usage_)
		{
			d3dTexture1D_ = this->CreateTexture1D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
			this->QueryBaseTexture();
		}
	}

	ID3D9TexturePtr D3D9Texture1D::CreateTexture1D(uint32_t usage, D3DPOOL pool)
	{
		if (IsDepthFormat(format_))
		{
			usage |= D3DUSAGE_DEPTHSTENCIL;
		}

		IDirect3DTexture9* d3dTexture1D;
		TIF(d3dDevice_->CreateTexture(widths_[0], 1,
			numMipMaps_, usage, D3D9Mapping::MappingFormat(format_),
			pool, &d3dTexture1D, NULL));
		return MakeCOMPtr(d3dTexture1D);
	}

	void D3D9Texture1D::QueryBaseTexture()
	{
		IDirect3DBaseTexture9* d3dBaseTexture = NULL;
		d3dTexture1D_->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&d3dBaseTexture));
		d3dBaseTexture_ = MakeCOMPtr(d3dBaseTexture);
	}

	void D3D9Texture1D::UpdateParams()
	{
		D3DSURFACE_DESC desc;
		std::memset(&desc, 0, sizeof(desc));

		numMipMaps_ = static_cast<uint16_t>(d3dTexture1D_->GetLevelCount());
		BOOST_ASSERT(numMipMaps_ != 0);

		widths_.resize(numMipMaps_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			TIF(d3dTexture1D_->GetLevelDesc(level, &desc));

			widths_[level] = desc.Width;
		}

		bool srgb = IsSRGB(format_);
		format_ = D3D9Mapping::MappingFormat(desc.Format);
		if (srgb)
		{
			format_ = MakeSRGB(format_);
		}

		bpp_	= NumFormatBits(format_);
	}

	void D3D9Texture1D::Usage(TextureUsage usage)
	{
		if (usage != usage_)
		{
			ID3D9TexturePtr d3dTmpTexture1D;
			switch (usage)
			{
			case TU_Default:
				d3dTmpTexture1D = this->CreateTexture1D(0, D3DPOOL_MANAGED);
				break;
				
			case TU_RenderTarget:
				d3dTmpTexture1D = this->CreateTexture1D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
				break;
			}

			DWORD filter = D3DX_FILTER_NONE;
			if (IsSRGB(format_))
			{
				filter |= D3DX_FILTER_SRGB;
			}

			ID3D9SurfacePtr src_surf, dest_surf;
			for (uint32_t i = 0; i < d3dTexture1D_->GetLevelCount(); ++ i)
			{
				IDirect3DSurface9* pSrcSurf;
				d3dTexture1D_->GetSurfaceLevel(i, &pSrcSurf);
				src_surf = MakeCOMPtr(pSrcSurf);

				IDirect3DSurface9* pDestSurf;
				d3dTmpTexture1D->GetSurfaceLevel(i, &pDestSurf);
				dest_surf = MakeCOMPtr(pDestSurf);

				TIF(D3DXLoadSurfaceFromSurface(dest_surf.get(), NULL, NULL,
					src_surf.get(), NULL, NULL, filter, 0));
			}
			d3dTexture1D_ = d3dTmpTexture1D;

			this->QueryBaseTexture();
			this->UpdateParams();

			usage_ = usage;
		}
	}
}
