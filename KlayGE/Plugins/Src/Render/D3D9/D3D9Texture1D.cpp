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

#pragma comment(lib, "d3d9.lib")
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx9d.lib")
#else
	#pragma comment(lib, "d3dx9.lib")
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

		bpp_ = ElementFormatBits(format);

		d3dTexture1D_ = this->CreateTexture1D(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);

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

		D3D9Texture1D& other(static_cast<D3D9Texture1D&>(target));

		BOOST_ASSERT(target.Type() == type_);

		uint32_t maxLevel = 1;
		if (this->NumMipMaps() == target.NumMipMaps())
		{
			maxLevel = this->NumMipMaps();
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
				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_LINEAR, 0));
			}
		}

		if (this->NumMipMaps() != target.NumMipMaps())
		{
			target.BuildMipSubLevels();
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

			TIF(D3DXLoadSurfaceFromSurface(tmp_surface, NULL, NULL, surface.get(), NULL, NULL, D3DX_FILTER_NONE, 0));

			surface = MakeCOMPtr(tmp_surface);
		}

		this->CopySurfaceToMemory(surface, data);
	}

	void D3D9Texture1D::CopyMemoryToTexture1D(int level, void* data, ElementFormat pf,
		uint32_t dst_width, uint32_t dst_xOffset, uint32_t src_width)
	{
		IDirect3DSurface9* temp;
		TIF(d3dTexture1D_->GetSurfaceLevel(level, &temp));
		ID3D9SurfacePtr surface = MakeCOMPtr(temp);

		if (surface)
		{
			RECT srcRc = { 0, 0, src_width, 1 };
			RECT dstRc = { dst_xOffset, 0, dst_xOffset + dst_width, 1 };
			TIF(D3DXLoadSurfaceFromMemory(surface.get(), NULL, &dstRc, data, D3D9Mapping::MappingFormat(pf),
					src_width * ElementFormatBits(pf) / 8, NULL, &srcRc, D3DX_DEFAULT, 0));
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
			if (TU_RenderTarget == usage_)
			{
				ID3D9TexturePtr d3dTexture1D = this->CreateTexture1D(D3DUSAGE_AUTOGENMIPMAP, D3DPOOL_DEFAULT);

				IDirect3DSurface9* temp;
				TIF(d3dTexture1D_->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr src = MakeCOMPtr(temp);

				TIF(d3dTexture1D->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr dst = MakeCOMPtr(temp);

				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));

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

				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));

				TIF(D3DXFilterTexture(d3dBaseTexture.get(), NULL, D3DX_DEFAULT, D3DX_DEFAULT));
				TIF(d3dDevice_->UpdateTexture(d3dBaseTexture.get(), d3dBaseTexture_.get()));
			}
		}
	}

	void D3D9Texture1D::DoOnLostDevice()
	{
		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		ID3D9TexturePtr tempTexture1D = this->CreateTexture1D(0, D3DPOOL_SYSTEMMEM);

		for (uint16_t i = 0; i < this->NumMipMaps(); ++ i)
		{
			IDirect3DSurface9* temp;
			TIF(d3dTexture1D_->GetSurfaceLevel(i, &temp));
			ID3D9SurfacePtr src = MakeCOMPtr(temp);

			TIF(tempTexture1D->GetSurfaceLevel(i, &temp));
			ID3D9SurfacePtr dst = MakeCOMPtr(temp);

			TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
		}
		tempTexture1D->AddDirtyRect(NULL);
		d3dTexture1D_ = tempTexture1D;

		this->QueryBaseTexture();
	}

	void D3D9Texture1D::DoOnResetDevice()
	{
		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		ID3D9TexturePtr tempTexture1D;
		if (TU_Default == usage_)
		{
			tempTexture1D = this->CreateTexture1D(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);
		}
		else
		{
			tempTexture1D = this->CreateTexture1D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
		}

		tempTexture1D->AddDirtyRect(NULL);

		d3dDevice_->UpdateTexture(d3dTexture1D_.get(), tempTexture1D.get());
		d3dTexture1D_ = tempTexture1D;

		this->QueryBaseTexture();
	}

	ID3D9TexturePtr D3D9Texture1D::CreateTexture1D(uint32_t usage, D3DPOOL pool)
	{
		IDirect3DTexture9* d3dTexture1D;
		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		TIF(D3DXCreateTexture(d3dDevice_.get(), widths_[0], 1,
			numMipMaps_, usage, D3D9Mapping::MappingFormat(format_),
			pool, &d3dTexture1D));
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

		format_ = D3D9Mapping::MappingFormat(desc.Format);
		bpp_	= ElementFormatBits(format_);
	}
}
