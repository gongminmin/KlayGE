// D3D9Texture2D.cpp
// KlayGE D3D9 2D纹理类 实现文件
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
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
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
	D3D9Texture2D::D3D9Texture2D(uint32_t width, uint32_t height,
								uint16_t numMipMaps, ElementFormat format, uint32_t access_hint, ElementInitData* init_data)
					: D3D9Texture(TT_2D, access_hint),
						auto_gen_mipmaps_(false)
	{
		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3dDevice_ = renderEngine.D3DDevice();

		numMipMaps_ = numMipMaps;
		format_		= format;
		widths_.assign(1, width);
		heights_.assign(1, height);

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
		d3dTexture2D_ = this->CreateTexture2D(usage, pool);

		this->QueryBaseTexture();
		this->UpdateParams();

		if (init_data != NULL)
		{
			if (access_hint & EAH_GPU_Write)
			{
				TexturePtr sys_mem = Context::Instance().RenderFactoryInstance().MakeTexture2D(widths_[0],
					heights_[0], numMipMaps_, format_, EAH_CPU_Write, init_data);
				sys_mem->CopyToTexture(*this);
			}
			else
			{
				for (int level = 0; level < numMipMaps_; ++ level)
				{
					Texture::Mapper mapper(*this, level, TMA_Write_Only, 0, 0, widths_[level], heights_[level]);

					if (IsCompressedFormat(format_))
					{
						int block_size;
						if (EF_BC1 == format)
						{
							block_size = 8;
						}
						else
						{
							block_size = 16;
						}

						memcpy(mapper.Pointer<uint8_t>(), &init_data[level].data[0],
							((widths_[level] + 3) / 4) * ((heights_[level] + 3) / 4) * block_size);
					}
					else
					{
						for (uint32_t h = 0; h < heights_[level]; ++ h)
						{
							memcpy(mapper.Pointer<uint8_t>() + mapper.RowPitch() * h, &init_data[level].data[init_data[level].row_pitch * h],
								std::min(mapper.RowPitch(), init_data[level].row_pitch));
						}
					}
				}
			}
		}
	}

	uint32_t D3D9Texture2D::Width(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return widths_[level];
	}

	uint32_t D3D9Texture2D::Height(int level) const
	{
		BOOST_ASSERT(level < numMipMaps_);

		return heights_[level];
	}

	void D3D9Texture2D::CopyToTexture(Texture& target)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D9Texture2D& other(*checked_cast<D3D9Texture2D*>(&target));

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

			TIF(d3dTexture2D_->GetSurfaceLevel(level, &temp));
			src = MakeCOMPtr(temp);

			TIF(other.d3dTexture2D_->GetSurfaceLevel(level, &temp));
			dst = MakeCOMPtr(temp);

			if ((this->AccessHint() & EAH_GPU_Write) && (target.AccessHint() & EAH_GPU_Write))
			{
				if (FAILED(d3dDevice_->StretchRect(src.get(), NULL, dst.get(), NULL, D3DTEXF_LINEAR)))
				{
					TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));
				}
			}
			else
			{
				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));
			}
		}

		if (this->NumMipMaps() != target.NumMipMaps())
		{
			target.BuildMipSubLevels();
		}
	}

	void D3D9Texture2D::CopyToTexture2D(Texture& target, int level,
			uint32_t dst_width, uint32_t dst_height, uint32_t dst_xOffset, uint32_t dst_yOffset,
			uint32_t src_width, uint32_t src_height, uint32_t src_xOffset, uint32_t src_yOffset)
	{
		BOOST_ASSERT(type_ == target.Type());

		D3D9Texture2D& other(*checked_cast<D3D9Texture2D*>(&target));

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

			TIF(d3dTexture2D_->GetSurfaceLevel(level, &temp));
			src = MakeCOMPtr(temp);

			TIF(other.d3dTexture2D_->GetSurfaceLevel(level, &temp));
			dst = MakeCOMPtr(temp);

			RECT srcRc = { src_xOffset, src_yOffset, src_xOffset + src_width, src_yOffset + src_height };
			RECT dstRc = { dst_xOffset, dst_yOffset, dst_xOffset + dst_width, dst_yOffset + dst_height };
			if ((this->AccessHint() & EAH_GPU_Write) && (target.AccessHint() & EAH_GPU_Write))
			{
				if (FAILED(d3dDevice_->StretchRect(src.get(), &srcRc, dst.get(), &dstRc, D3DTEXF_LINEAR)))
				{
					TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, &dstRc, src.get(), NULL, &srcRc, filter, 0));
				}
			}
			else
			{
				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, &dstRc, src.get(), NULL, &srcRc, filter, 0));
			}
		}
	}

	void D3D9Texture2D::Map2D(int level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch)
	{
		RECT rc = { x_offset, y_offset, x_offset + width, y_offset + height };
		D3DLOCKED_RECT locked_rc;
		d3dTexture2D_->LockRect(level, &locked_rc, &rc, TMA_Read_Only == tma ? D3DLOCK_READONLY : 0);
		data = locked_rc.pBits;
		row_pitch = locked_rc.Pitch;
	}

	void D3D9Texture2D::Unmap2D(int level)
	{
		d3dTexture2D_->UnlockRect(level);
	}

	void D3D9Texture2D::BuildMipSubLevels()
	{
		ID3D9BaseTexturePtr d3dBaseTexture;

		if (auto_gen_mipmaps_)
		{
			d3dTexture2D_->GenerateMipSubLevels();
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
				ID3D9TexturePtr d3dTexture2D = this->CreateTexture2D(D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);

				IDirect3DSurface9* temp;
				TIF(d3dTexture2D_->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr src = MakeCOMPtr(temp);

				TIF(d3dTexture2D->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr dst = MakeCOMPtr(temp);

				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));

				d3dTexture2D->GenerateMipSubLevels();
				d3dTexture2D_ = d3dTexture2D;

				auto_gen_mipmaps_ = true;
			}
			else
			{
				ID3D9TexturePtr d3dTexture2D = this->CreateTexture2D(0, D3DPOOL_SYSTEMMEM);

				IDirect3DBaseTexture9* base;
				d3dTexture2D->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&base));
				d3dBaseTexture = MakeCOMPtr(base);

				IDirect3DSurface9* temp;
				TIF(d3dTexture2D_->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr src = MakeCOMPtr(temp);

				TIF(d3dTexture2D->GetSurfaceLevel(0, &temp));
				ID3D9SurfacePtr dst = MakeCOMPtr(temp);

				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, filter, 0));

				TIF(D3DXFilterTexture(d3dBaseTexture.get(), NULL, 0, filter));
				TIF(d3dDevice_->UpdateTexture(d3dBaseTexture.get(), d3dBaseTexture_.get()));
			}
		}
	}

	void D3D9Texture2D::DoOnLostDevice()
	{
		if (this->AccessHint() & EAH_GPU_Write)
		{
			d3dBaseTexture_.reset();
			d3dTexture2D_.reset();
		}
	}

	void D3D9Texture2D::DoOnResetDevice()
	{
		if (this->AccessHint() & EAH_GPU_Write)
		{
			d3dTexture2D_ = this->CreateTexture2D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
			this->QueryBaseTexture();
		}
	}

	ID3D9TexturePtr D3D9Texture2D::CreateTexture2D(uint32_t usage, D3DPOOL pool)
	{
		if (IsDepthFormat(format_))
		{
			usage = D3DUSAGE_DEPTHSTENCIL;
			pool = D3DPOOL_DEFAULT;
		}

		IDirect3DTexture9* d3dTexture2D;
		TIF(d3dDevice_->CreateTexture(widths_[0], heights_[0],
			numMipMaps_, usage, D3D9Mapping::MappingFormat(format_),
			pool, &d3dTexture2D, NULL));
		return MakeCOMPtr(d3dTexture2D);
	}

	void D3D9Texture2D::QueryBaseTexture()
	{
		IDirect3DBaseTexture9* d3dBaseTexture = NULL;
		d3dTexture2D_->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&d3dBaseTexture));
		d3dBaseTexture_ = MakeCOMPtr(d3dBaseTexture);
	}

	void D3D9Texture2D::UpdateParams()
	{
		D3DSURFACE_DESC desc;
		std::memset(&desc, 0, sizeof(desc));

		numMipMaps_ = static_cast<uint16_t>(d3dTexture2D_->GetLevelCount());
		BOOST_ASSERT(numMipMaps_ != 0);

		widths_.resize(numMipMaps_);
		heights_.resize(numMipMaps_);
		for (uint16_t level = 0; level < numMipMaps_; ++ level)
		{
			TIF(d3dTexture2D_->GetLevelDesc(level, &desc));

			widths_[level] = desc.Width;
			heights_[level] = desc.Height;
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
