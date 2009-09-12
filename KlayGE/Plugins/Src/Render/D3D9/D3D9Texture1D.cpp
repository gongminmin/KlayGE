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
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>

#include <KlayGE/D3D9/D3D9MinGWDefs.hpp>

#include <d3d9.h>
#include <d3dx9.h>

#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx9d.lib")
#else
	#pragma comment(lib, "d3dx9.lib")
#endif
#endif

namespace KlayGE
{
	D3D9Texture1D::D3D9Texture1D(uint32_t width, uint16_t numMipMaps, uint16_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData* init_data)
					: D3D9Texture(TT_1D, sample_count, sample_quality, access_hint),
						auto_gen_mipmaps_(false)
	{
		if (array_size > 1)
		{
			THR(boost::system::posix_error::not_supported);
		}

		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3dDevice_ = renderEngine.D3DDevice();

		numMipMaps_ = numMipMaps;
		array_size_ = 1;
		format_		= format;
		widths_.assign(1, width);

		bpp_ = NumFormatBits(format);

		uint32_t usage;
		D3DPOOL pool;
		if (access_hint & EAH_GPU_Write)
		{
			usage = D3DUSAGE_RENDERTARGET;
			pool = D3DPOOL_DEFAULT;
		}
		else
		{
			usage = 0;
			pool = D3DPOOL_MANAGED;
		}
		d3dTexture1D_ = this->CreateTexture1D(usage, pool);

		this->QueryBaseTexture();
		this->UpdateParams();

		if (init_data != NULL)
		{
			if (access_hint & EAH_GPU_Write)
			{
				TexturePtr sys_mem = Context::Instance().RenderFactoryInstance().MakeTexture1D(widths_[0], numMipMaps_, array_size_,
					format_, sample_count, sample_quality, EAH_CPU_Write, init_data);
				sys_mem->CopyToTexture(*this);
			}
			else
			{
				for (int level = 0; level < numMipMaps_; ++ level)
				{
					Texture::Mapper mapper(*this, level, TMA_Write_Only, 0, widths_[level]);
					memcpy(mapper.Pointer<uint8_t>(), init_data[level].data,
						std::min(mapper.RowPitch(), init_data[level].row_pitch));
				}
			}
		}
	}

	uint32_t D3D9Texture1D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return widths_[level];
	}

	uint32_t D3D9Texture1D::Height(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(level < numMipMaps_);

		return 1;
	}

	uint32_t D3D9Texture1D::Depth(int level) const
	{
		UNREF_PARAM(level);
		BOOST_ASSERT(level < numMipMaps_);

		return 1;
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

		for (uint32_t level = 0; level < maxLevel; ++ level)
		{
			IDirect3DSurface9* src;
			TIF(d3dTexture1D_->GetSurfaceLevel(level, &src));

			IDirect3DSurface9* dst;
			TIF(other.d3dTexture1D_->GetSurfaceLevel(level, &dst));

			if ((this->AccessHint() & EAH_GPU_Write) && (target.AccessHint() & EAH_GPU_Write))
			{
				if (FAILED(d3dDevice_->StretchRect(src, NULL, dst, NULL, D3DTEXF_LINEAR)))
				{
					TIF(D3DXLoadSurfaceFromSurface(dst, NULL, NULL, src, NULL, NULL, filter, 0));
				}
			}
			else
			{
				TIF(D3DXLoadSurfaceFromSurface(dst, NULL, NULL, src, NULL, NULL, filter, 0));
			}

			src->Release();
			dst->Release();
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
			IDirect3DSurface9* src;
			TIF(d3dTexture1D_->GetSurfaceLevel(level, &src));

			IDirect3DSurface9* dst;
			TIF(other.d3dTexture1D_->GetSurfaceLevel(level, &dst));

			RECT srcRc = { src_xOffset, 0, src_xOffset + src_width, 1 };
			RECT dstRc = { dst_xOffset, 0, dst_xOffset + dst_width, 1 };
			if ((this->AccessHint() & EAH_GPU_Write) && (target.AccessHint() & EAH_GPU_Write))
			{
				if (FAILED(d3dDevice_->StretchRect(src, &srcRc, dst, &dstRc, D3DTEXF_LINEAR)))
				{
					TIF(D3DXLoadSurfaceFromSurface(dst, NULL, &dstRc, src, NULL, &srcRc, filter, 0));
				}
			}
			else
			{
				TIF(D3DXLoadSurfaceFromSurface(dst, NULL, &dstRc, src, NULL, &srcRc, filter, 0));
			}

			src->Release();
			dst->Release();
		}
	}

	void D3D9Texture1D::Map1D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width,
			void*& data)
	{
		RECT rc = { x_offset, 0, x_offset + width, 1 };
		D3DLOCKED_RECT locked_rc;
		d3dTexture1D_->LockRect(level, &locked_rc, &rc, TMA_Read_Only == tma ? D3DLOCK_READONLY : 0);
		data = locked_rc.pBits;
	}

	void D3D9Texture1D::Unmap1D(int level)
	{
		d3dTexture1D_->UnlockRect(level);
	}

	void D3D9Texture1D::BuildMipSubLevels()
	{
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

			if (this->AccessHint() & EAH_GPU_Write)
			{
				ID3D9TexturePtr d3dTexture1D = this->CreateTexture1D(D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);

				IDirect3DSurface9* src;
				TIF(d3dTexture1D_->GetSurfaceLevel(0, &src));

				IDirect3DSurface9* dst;
				TIF(d3dTexture1D->GetSurfaceLevel(0, &dst));

				TIF(D3DXLoadSurfaceFromSurface(dst, NULL, NULL, src, NULL, NULL, filter, 0));

				d3dTexture1D->GenerateMipSubLevels();
				d3dTexture1D_ = d3dTexture1D;

				auto_gen_mipmaps_ = true;

				src->Release();
				dst->Release();
			}
			else
			{
				ID3D9TexturePtr d3dTexture1D = this->CreateTexture1D(0, D3DPOOL_SYSTEMMEM);

				IDirect3DBaseTexture9* base;
				d3dTexture1D->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&base));

				IDirect3DSurface9* src;
				TIF(d3dTexture1D_->GetSurfaceLevel(0, &src));

				IDirect3DSurface9* dst;
				TIF(d3dTexture1D->GetSurfaceLevel(0, &dst));

				TIF(D3DXLoadSurfaceFromSurface(dst, NULL, NULL, src, NULL, NULL, filter, 0));

				TIF(D3DXFilterTexture(base, NULL, 0, filter));
				TIF(d3dDevice_->UpdateTexture(base, d3dBaseTexture_.get()));

				base->Release();
				src->Release();
				dst->Release();
			}
		}
	}

	void D3D9Texture1D::DoOnLostDevice()
	{
		if (this->AccessHint() & EAH_GPU_Write)
		{
			d3dBaseTexture_.reset();
			d3dTexture1D_.reset();
		}
	}

	void D3D9Texture1D::DoOnResetDevice()
	{
		if (this->AccessHint() & EAH_GPU_Write)
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
}
