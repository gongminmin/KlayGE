// D3D9Texture.cpp
// KlayGE D3D9纹理类 实现文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.3.0
// 增加了对浮点纹理格式的支持 (2005.1.25)
// 改进了CopyMemoryToTexture (2005.2.1)
// 增加了CopyToMemory (2005.2.6)
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 2.0.5
// 改用GenerateMipSubLevels来生成mipmap (2004.4.8)
//
// 2.0.4
// 修正了当源和目标格式不同时CopyMemoryToTexture出错的Bug (2004.3.19)
//
// 2.0.0
// 初次建立 (2003.8.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Util.hpp>

#include <cstring>

#include <d3dx9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <KlayGE/D3D9/D3D9Texture.hpp>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

namespace
{
	using namespace KlayGE;

	D3DFORMAT ConvertFormat(KlayGE::PixelFormat format)
	{
		switch (format)
		{
		case PF_L8:
			return D3DFMT_L8;

		case PF_A8:
			return D3DFMT_A8;

		case PF_A4L4:
			return D3DFMT_A4L4;

		case PF_A8L8:
			return D3DFMT_A8L8;

		case PF_R5G6B5:
			return D3DFMT_R5G6B5;

		case PF_A4R4G4B4:
			return D3DFMT_A4R4G4B4;

		case PF_X8R8G8B8:
			return D3DFMT_X8R8G8B8;

		case PF_A8R8G8B8:
			return D3DFMT_A8R8G8B8;

		case PF_A2R10G10B10:
			return D3DFMT_A2B10G10R10;

		case PF_R16F:
			return D3DFMT_R16F;

		case PF_G16R16F:
			return D3DFMT_G16R16F;
		case PF_A16B16G16R16F:
			return D3DFMT_A16B16G16R16F;

		case PF_R32F:
			return D3DFMT_R32F;

		case PF_G32R32F:
			return D3DFMT_G32R32F;

		case PF_A32B32G32R32F:
			return D3DFMT_A32B32G32R32F;

		case PF_DXT1:
			return D3DFMT_DXT1;

		case PF_DXT3:
			return D3DFMT_DXT3;

		case PF_DXT5:
			return D3DFMT_DXT5;
		}

		assert(false);
		return D3DFMT_UNKNOWN;
	}

	KlayGE::PixelFormat ConvertFormat(D3DFORMAT format)
	{
		switch (format)
		{
		case D3DFMT_L8:
			return PF_L8;

		case D3DFMT_A8:
			return PF_A8;

		case D3DFMT_A4L4:
			return PF_A4L4;

		case D3DFMT_A8L8:
			return PF_A8L8;

		case D3DFMT_R5G6B5:
			return PF_R5G6B5;

		case D3DFMT_A4R4G4B4:
			return PF_A4R4G4B4;

		case D3DFMT_X8R8G8B8:
			return PF_X8R8G8B8;

		case D3DFMT_A8R8G8B8:
			return PF_A8R8G8B8;

		case D3DFMT_A2B10G10R10:
			return PF_A2R10G10B10;

		case D3DFMT_R16F:
			return PF_R16F;

		case D3DFMT_G16R16F:
			return PF_G16R16F;

		case D3DFMT_A16B16G16R16F:
			return PF_A16B16G16R16F;

		case D3DFMT_R32F:
			return PF_R32F;

		case D3DFMT_G32R32F:
			return PF_G32R32F;

		case D3DFMT_A32B32G32R32F:
			return PF_A32B32G32R32F;

		case D3DFMT_DXT1:
			return PF_DXT1;

		case D3DFMT_DXT3:
			return PF_DXT3;

		case D3DFMT_DXT5:
			return PF_DXT5;
		}

		assert(false);
		return PF_Unknown;
	}
}

namespace KlayGE
{
	D3D9Texture::D3D9Texture(uint32_t width, uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
	{
		type_	= TT_1D;

		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		numMipMaps_ = numMipMaps;
		format_		= format;
		width_		= width;
		height_		= 1;
		depth_		= 1;

		bpp_ = PixelFormatBits(format);

		usage_ = usage;

		assert(TU_Default == usage_);

		d3dTexture2D_ = this->CreateTexture2D(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);

		this->QueryBaseTexture();

		D3DSURFACE_DESC desc;
		// Check the actual dimensions vs requested
		TIF(d3dTexture2D_->GetLevelDesc(0, &desc));
		width_	= desc.Width;
		height_	= desc.Height;
		format_ = ConvertFormat(desc.Format);
		bpp_	= PixelFormatBits(format_);
		numMipMaps_ = static_cast<uint16_t>(d3dTexture2D_->GetLevelCount());
	}

	D3D9Texture::D3D9Texture(uint32_t width, uint32_t height,
								uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
	{
		type_	= TT_2D;

		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		numMipMaps_ = numMipMaps;
		format_		= format;
		width_		= width;
		height_		= height;
		depth_		= 1;

		bpp_ = PixelFormatBits(format);

		usage_ = usage;

		if (TU_Default == usage_)
		{
			d3dTexture2D_ = this->CreateTexture2D(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);

			this->QueryBaseTexture();

			D3DSURFACE_DESC desc;
			// Check the actual dimensions vs requested
			TIF(d3dTexture2D_->GetLevelDesc(0, &desc));
			width_	= desc.Width;
			height_	= desc.Height;
			format_ = ConvertFormat(desc.Format);
			bpp_	= PixelFormatBits(format_);
			numMipMaps_ = static_cast<uint16_t>(d3dTexture2D_->GetLevelCount());
		}
		else
		{
			// We need to set the pixel format of the render target texture to be the same as the
			// one of the front buffer.
			IDirect3DSurface9* tempSurf;
			D3DSURFACE_DESC tempDesc;

			d3dDevice_->GetRenderTarget(0, &tempSurf);
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			d3dTexture2D_ = this->CreateTexture2D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);

			this->QueryBaseTexture();

			D3DSURFACE_DESC desc;
			// Check the actual dimensions vs requested
			TIF(d3dTexture2D_->GetLevelDesc(0, &desc));
			width_	= desc.Width;
			height_	= desc.Height;
			format_ = ConvertFormat(desc.Format);
			bpp_	= PixelFormatBits(format_);
			numMipMaps_ = static_cast<uint16_t>(d3dTexture2D_->GetLevelCount());

			// Now get the format of the depth stencil surface.
			d3dDevice_->GetDepthStencilSurface(&tempSurf);
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			// We also create a depth buffer for our render target.
			TIF(d3dDevice_->CreateDepthStencilSurface(width_, height_, tempDesc.Format, tempDesc.MultiSampleType, 0, 
				FALSE, &tempSurf, NULL));
			renderZBuffer_ = MakeCOMPtr(tempSurf);
		}
	}

	D3D9Texture::D3D9Texture(uint32_t width, uint32_t height, uint32_t depth,
								uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
	{
		type_	= TT_3D;

		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		numMipMaps_ = numMipMaps;
		format_		= format;
		width_		= width;
		height_		= height;
		depth_		= depth;

		bpp_ = PixelFormatBits(format);

		usage_ = usage;

		assert(TU_Default == usage_);

		d3dTexture3D_ = this->CreateTexture3D(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);

		this->QueryBaseTexture();

		D3DVOLUME_DESC desc;
		// Check the actual dimensions vs requested
		TIF(d3dTexture3D_->GetLevelDesc(0, &desc));
		width_	= desc.Width;
		height_	= desc.Height;
		depth_	= desc.Depth;
		format_ = ConvertFormat(desc.Format);
		bpp_	= PixelFormatBits(format_);
		numMipMaps_ = static_cast<uint16_t>(d3dTexture3D_->GetLevelCount());
	}

	D3D9Texture::D3D9Texture(uint32_t size, bool /*cube*/, uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
	{
		type_	= TT_Cube;

		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		numMipMaps_ = numMipMaps;
		format_		= format;
		width_		= size;
		height_		= size;
		depth_		= 1;

		bpp_ = PixelFormatBits(format);

		usage_ = usage;

		assert(TU_Default == usage_);

		d3dTextureCube_ = this->CreateTextureCube(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);

		this->QueryBaseTexture();

		D3DSURFACE_DESC desc;
		// Check the actual dimensions vs requested
		TIF(d3dTextureCube_->GetLevelDesc(0, &desc));
		width_	= desc.Width;
		height_	= desc.Height;
		format_ = ConvertFormat(desc.Format);
		bpp_	= PixelFormatBits(format_);
		numMipMaps_ = static_cast<uint16_t>(d3dTexture2D_->GetLevelCount());
	}

	D3D9Texture::~D3D9Texture()
	{
	}

	std::wstring const & D3D9Texture::Name() const
	{
		static const std::wstring name(L"Direct3D9 Texture");
		return name;
	}

	void D3D9Texture::CopyToTexture(Texture& target)
	{
		D3D9Texture& other(static_cast<D3D9Texture&>(target));

		assert(target.Width() == width_);
		assert(target.Height() == height_);
		assert(target.Depth() == depth_);
		assert(target.Type() == type_);

		uint32_t maxLevel = 1;
		if (this->NumMipMaps() == target.NumMipMaps())
		{
			maxLevel = this->NumMipMaps();
		}

		switch (type_)
		{
		case TT_1D:
		case TT_2D:
			{
				boost::shared_ptr<IDirect3DSurface9> src, dst;

				for (uint32_t i = 0; i < maxLevel; ++ i)
				{
					IDirect3DSurface9* temp;
					TIF(d3dTexture2D_->GetSurfaceLevel(i, &temp));
					src = MakeCOMPtr(temp);

					TIF(other.d3dTexture2D_->GetSurfaceLevel(i, &temp));
					dst = MakeCOMPtr(temp);

					TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DTEXF_NONE, 0));
				}
			}
			break;

		case TT_3D:
			{
				boost::shared_ptr<IDirect3DVolume9> src, dst;

				for (uint32_t i = 0; i < maxLevel; ++ i)
				{
					IDirect3DVolume9* temp;
					TIF(d3dTexture3D_->GetVolumeLevel(i, &temp));
					src = MakeCOMPtr(temp);

					TIF(other.d3dTexture3D_->GetVolumeLevel(i, &temp));
					dst = MakeCOMPtr(temp);

					TIF(D3DXLoadVolumeFromVolume(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DTEXF_NONE, 0));
				}
			}
			break;

		case TT_Cube:
			{
				boost::shared_ptr<IDirect3DSurface9> src, dst;

				for (uint32_t face = D3DCUBEMAP_FACE_POSITIVE_X; face < D3DCUBEMAP_FACE_NEGATIVE_Z; ++ face)
				{
					for (uint32_t level = 0; level < maxLevel; ++ level)
					{
						IDirect3DSurface9* temp;
						TIF(d3dTextureCube_->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), level, &temp));
						src = MakeCOMPtr(temp);

						TIF(other.d3dTextureCube_->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), level, &temp));
						dst = MakeCOMPtr(temp);

						TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DTEXF_NONE, 0));
					}
				}
			}
			break;

		default:
			assert(false);
			break;
		}

		if (this->NumMipMaps() != target.NumMipMaps())
		{
			target.BuildMipSubLevels();
		}		
	}

	void D3D9Texture::CopyToMemory(int level, void* data)
	{
		assert(data != NULL);

		switch (type_)
		{
		case TT_1D:
		case TT_2D:
			{
				D3DLOCKED_RECT d3d_rc;
				d3dTexture2D_->LockRect(level, &d3d_rc, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);

				uint8_t* dst = static_cast<uint8_t*>(data);
				uint8_t* src = static_cast<uint8_t*>(d3d_rc.pBits);

				uint32_t const srcPitch = d3d_rc.Pitch;
				uint32_t const dstPitch = this->Width() * PixelFormatBits(format_) / 8;

				for (uint32_t i = 0; i < this->Height(); ++ i)
				{
					std::copy(src, src + dstPitch, dst);

					src += srcPitch;
					dst += dstPitch;
				}

				d3dTexture2D_->UnlockRect(0);
			}
			break;

		case TT_3D:
			{
				D3DLOCKED_BOX d3d_box;
				d3dTexture3D_->LockBox(level, &d3d_box, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);

				uint8_t* dst = static_cast<uint8_t*>(data);
				uint8_t* src = static_cast<uint8_t*>(d3d_box.pBits);

				uint32_t const srcPitch = d3d_box.RowPitch;
				uint32_t const dstPitch = this->Width() * PixelFormatBits(format_) / 8;

				for (uint32_t j = 0; j < this->Depth(); ++ j)
				{
					src = static_cast<uint8_t*>(d3d_box.pBits) + j * d3d_box.SlicePitch;

					for (uint32_t i = 0; i < this->Height(); ++ i)
					{
						std::copy(src, src + dstPitch, dst);

						src += srcPitch;
						dst += dstPitch;
					}
				}

				d3dTexture3D_->UnlockBox(0);
			}
			break;

		case TT_Cube:
			{
				for (uint32_t face = D3DCUBEMAP_FACE_POSITIVE_X; face < D3DCUBEMAP_FACE_NEGATIVE_Z; ++ face)
				{
					D3DLOCKED_RECT d3d_rc;
					d3dTextureCube_->LockRect(static_cast<D3DCUBEMAP_FACES>(face),
						level, &d3d_rc, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);

					uint8_t* dst = static_cast<uint8_t*>(data);
					uint8_t* src = static_cast<uint8_t*>(d3d_rc.pBits);

					uint32_t const srcPitch = d3d_rc.Pitch;
					uint32_t const dstPitch = this->Width() * PixelFormatBits(format_) / 8;

					for (uint32_t i = 0; i < this->Height(); ++ i)
					{
						std::copy(src, src + dstPitch, dst);

						src += srcPitch;
						dst += dstPitch;
					}

					d3dTextureCube_->UnlockRect(static_cast<D3DCUBEMAP_FACES>(face), 0);
				}
			}
			break;

		default:
			assert(false);
			break;
		}
	}

	void D3D9Texture::CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
		uint32_t width, uint32_t xOffset)
	{
		this->CopyMemoryToTexture2D(level, data, pf, width, 1, xOffset, 0);
	}

	void D3D9Texture::CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
		uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset)
	{
		assert((TT_1D == type_) || (TT_2D == type_));

		IDirect3DSurface9* temp;
		TIF(d3dTexture2D_->GetSurfaceLevel(level, &temp));
		boost::shared_ptr<IDirect3DSurface9> shadow = MakeCOMPtr(temp);

		RECT srcRc = { 0, 0, width, height };
		RECT dstRc = { xOffset, yOffset, xOffset + srcRc.right, yOffset + srcRc.bottom };
		TIF(D3DXLoadSurfaceFromMemory(shadow.get(), NULL, &dstRc, data, ConvertFormat(pf),
				width * PixelFormatBits(pf) / 8, NULL, &srcRc, D3DX_FILTER_NONE, 0));
	}

	void D3D9Texture::CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t depth,
			uint32_t xOffset, uint32_t yOffset, uint32_t zOffset)
	{
		assert(TT_3D == type_);

		IDirect3DVolume9* temp;
		TIF(d3dTexture3D_->GetVolumeLevel(level, &temp));
		boost::shared_ptr<IDirect3DVolume9> shadow = MakeCOMPtr(temp);

		uint32_t const srcRowPitch = width * PixelFormatBits(pf) / 8;
		uint32_t const srcSlicePitch = srcRowPitch * height;

		D3DBOX srcBox = { 0, 0, width, height, 0, depth };
		D3DBOX dstBox = { xOffset, yOffset, xOffset + srcBox.Right, yOffset + srcBox.Bottom,
			zOffset, zOffset + srcBox.Back };
		TIF(D3DXLoadVolumeFromMemory(shadow.get(), NULL, &dstBox, data, ConvertFormat(pf),
				srcRowPitch, srcSlicePitch, NULL, &srcBox, D3DX_FILTER_NONE, 0));
	}

	void D3D9Texture::CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t size, uint32_t xOffset)
	{
		assert(TT_Cube == type_);

		IDirect3DSurface9* temp;
		TIF(d3dTextureCube_->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), level, &temp));
		boost::shared_ptr<IDirect3DSurface9> shadow = MakeCOMPtr(temp);

		RECT srcRc = { 0, 0, size, size };
		RECT dstRc = { xOffset, xOffset, xOffset + srcRc.right, xOffset + srcRc.bottom };
		TIF(D3DXLoadSurfaceFromMemory(shadow.get(), NULL, &dstRc, data, ConvertFormat(pf),
				size * PixelFormatBits(pf) / 8, NULL, &srcRc, D3DX_FILTER_NONE, 0));
	}

	void D3D9Texture::BuildMipSubLevels()
	{
		boost::shared_ptr<IDirect3DBaseTexture9> d3dBaseTexture;

		switch (type_)
		{
		case TT_1D:
		case TT_2D:
			{
				boost::shared_ptr<IDirect3DTexture9> d3dTexture2D = this->CreateTexture2D(0, D3DPOOL_SYSTEMMEM);

				IDirect3DBaseTexture9* base;
				d3dTexture2D->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&base));
				d3dBaseTexture = MakeCOMPtr(base);

				IDirect3DSurface9* temp;
				TIF(d3dTexture2D_->GetSurfaceLevel(0, &temp));
				boost::shared_ptr<IDirect3DSurface9> src = MakeCOMPtr(temp);

				TIF(d3dTexture2D->GetSurfaceLevel(0, &temp));
				boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
			}
			break;

		case TT_3D:
			{
				boost::shared_ptr<IDirect3DVolumeTexture9> d3dTexture3D = this->CreateTexture3D(0, D3DPOOL_SYSTEMMEM);

				IDirect3DBaseTexture9* base;
				d3dTexture3D->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&base));
				d3dBaseTexture = MakeCOMPtr(base);

				IDirect3DVolume9* temp;
				TIF(d3dTexture3D_->GetVolumeLevel(0, &temp));
				boost::shared_ptr<IDirect3DVolume9> src = MakeCOMPtr(temp);

				TIF(d3dTexture3D->GetVolumeLevel(0, &temp));
				boost::shared_ptr<IDirect3DVolume9> dst = MakeCOMPtr(temp);

				TIF(D3DXLoadVolumeFromVolume(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
			}
			break;

		case TT_Cube:
			{
				boost::shared_ptr<IDirect3DCubeTexture9> d3dTextureCube = this->CreateTextureCube(0, D3DPOOL_SYSTEMMEM);

				IDirect3DBaseTexture9* base;
				d3dTextureCube->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&base));
				d3dBaseTexture = MakeCOMPtr(base);

				for (uint32_t face = D3DCUBEMAP_FACE_POSITIVE_X; face < D3DCUBEMAP_FACE_NEGATIVE_Z; ++ face)
				{
					IDirect3DSurface9* temp;
					TIF(d3dTextureCube_->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), 0, &temp));
					boost::shared_ptr<IDirect3DSurface9> src = MakeCOMPtr(temp);

					TIF(d3dTextureCube->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), 0, &temp));
					boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

					TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
				}
			}
			break;

		default:
			assert(false);
			break;
		}

		TIF(D3DXFilterTexture(d3dBaseTexture.get(), NULL, D3DX_DEFAULT, D3DX_DEFAULT));
		TIF(d3dDevice_->UpdateTexture(d3dBaseTexture.get(), d3dBaseTexture_.get()));
	}

	void D3D9Texture::CustomAttribute(std::string const & /*name*/, void* /*data*/)
	{
		assert(false);
	}

	void D3D9Texture::DoOnLostDevice()
	{
		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		switch (type_)
		{
		case TT_1D:
		case TT_2D:
			if (TU_Default == usage_)
			{
				boost::shared_ptr<IDirect3DTexture9> tempTexture2D = this->CreateTexture2D(0, D3DPOOL_SYSTEMMEM);

				for (uint16_t i = 0; i < this->NumMipMaps(); ++ i)
				{
					IDirect3DSurface9* temp;
					TIF(d3dTexture2D_->GetSurfaceLevel(i, &temp));
					boost::shared_ptr<IDirect3DSurface9> src = MakeCOMPtr(temp);

					TIF(tempTexture2D->GetSurfaceLevel(i, &temp));
					boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

					TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
				}
				tempTexture2D->AddDirtyRect(NULL);
				d3dTexture2D_ = tempTexture2D;

				this->QueryBaseTexture();
			}
			else
			{
				IDirect3DSurface9* tempSurf;
				D3DSURFACE_DESC tempDesc;

				d3dDevice_->GetRenderTarget(0, &tempSurf);
				tempSurf->GetDesc(&tempDesc);
				tempSurf->Release();

				boost::shared_ptr<IDirect3DTexture9> tempTexture2D = this->CreateTexture2D(0, D3DPOOL_SYSTEMMEM);

				for (uint16_t i = 0; i < this->NumMipMaps(); ++ i)
				{
					IDirect3DSurface9* temp;
					TIF(d3dTexture2D_->GetSurfaceLevel(i, &temp));
					boost::shared_ptr<IDirect3DSurface9> src = MakeCOMPtr(temp);

					TIF(tempTexture2D->GetSurfaceLevel(i, &temp));
					boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

					TIF(d3dDevice_->GetRenderTargetData(src.get(), dst.get()));
				}
				tempTexture2D->AddDirtyRect(NULL);
				d3dTexture2D_ = tempTexture2D;

				this->QueryBaseTexture();

				d3dDevice_->GetDepthStencilSurface(&tempSurf);
				tempSurf->GetDesc(&tempDesc);
				tempSurf->Release();

				TIF(d3dDevice_->CreateOffscreenPlainSurface(width_, height_, tempDesc.Format,
					D3DPOOL_SYSTEMMEM, &tempSurf, NULL));
				boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(tempSurf);
				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, renderZBuffer_.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
				renderZBuffer_ = dst;
			}
			break;

		case TT_3D:
			{
				boost::shared_ptr<IDirect3DVolumeTexture9> tempTexture3D = this->CreateTexture3D(0, D3DPOOL_SYSTEMMEM);

				for (uint16_t i = 0; i < this->NumMipMaps(); ++ i)
				{
					IDirect3DVolume9* temp;
					TIF(d3dTexture3D_->GetVolumeLevel(i, &temp));
					boost::shared_ptr<IDirect3DVolume9> src = MakeCOMPtr(temp);

					TIF(tempTexture3D->GetVolumeLevel(i, &temp));
					boost::shared_ptr<IDirect3DVolume9> dst = MakeCOMPtr(temp);

					TIF(D3DXLoadVolumeFromVolume(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
				}
				tempTexture3D->AddDirtyBox(NULL);
				d3dTexture3D_ = tempTexture3D;

				this->QueryBaseTexture();
			}
			break;

		case TT_Cube:
			{
				boost::shared_ptr<IDirect3DCubeTexture9> tempTextureCube = this->CreateTextureCube(0, D3DPOOL_SYSTEMMEM);

				for (uint32_t face = D3DCUBEMAP_FACE_POSITIVE_X; face < D3DCUBEMAP_FACE_NEGATIVE_Z; ++ face)
				{
					for (uint16_t level = 0; level < this->NumMipMaps(); ++ level)
					{
					
						IDirect3DSurface9* temp;
						TIF(d3dTextureCube_->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), level, &temp));
						boost::shared_ptr<IDirect3DSurface9> src = MakeCOMPtr(temp);

						TIF(tempTextureCube->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), level, &temp));
						boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

						TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));

						tempTextureCube->AddDirtyRect(static_cast<D3DCUBEMAP_FACES>(face), NULL);
					}
				}
				d3dTextureCube_ = tempTextureCube;

				this->QueryBaseTexture();
			}
			break;

		default:
			assert(false);
			break;
		}
	}

	void D3D9Texture::DoOnResetDevice()
	{
		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		switch (type_)
		{
		case TT_1D:
		case TT_2D:
			if (TU_Default == usage_)
			{
				boost::shared_ptr<IDirect3DTexture9> tempTexture2D = this->CreateTexture2D(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);
				tempTexture2D->AddDirtyRect(NULL);

				d3dDevice_->UpdateTexture(d3dTexture2D_.get(), tempTexture2D.get());
				d3dTexture2D_ = tempTexture2D;

				this->QueryBaseTexture();
			}
			else
			{
				IDirect3DSurface9* tempSurf;
				D3DSURFACE_DESC tempDesc;

				d3dDevice_->GetRenderTarget(0, &tempSurf);
				tempSurf->GetDesc(&tempDesc);
				tempSurf->Release();

				boost::shared_ptr<IDirect3DTexture9> tempTexture2D = this->CreateTexture2D(D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
				tempTexture2D->AddDirtyRect(NULL);

				d3dDevice_->UpdateTexture(d3dTexture2D_.get(), tempTexture2D.get());
				d3dTexture2D_ = tempTexture2D;

				this->QueryBaseTexture();

				d3dDevice_->GetDepthStencilSurface(&tempSurf);
				tempSurf->GetDesc(&tempDesc);
				tempSurf->Release();

				TIF(d3dDevice_->CreateDepthStencilSurface(width_, height_, tempDesc.Format, tempDesc.MultiSampleType, 0, 
					FALSE, &tempSurf, NULL));
				boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(tempSurf);
				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, renderZBuffer_.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
				renderZBuffer_ = dst;
			}
			break;

		case TT_3D:
			{
				boost::shared_ptr<IDirect3DVolumeTexture9> tempTexture3D = this->CreateTexture3D(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);
				tempTexture3D->AddDirtyBox(NULL);

				d3dDevice_->UpdateTexture(d3dTexture3D_.get(), tempTexture3D.get());
				d3dTexture3D_ = tempTexture3D;

				this->QueryBaseTexture();
			}
			break;

		case TT_Cube:
			{
				boost::shared_ptr<IDirect3DCubeTexture9> tempTextureCube = this->CreateTextureCube(D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);
				for (uint32_t face = D3DCUBEMAP_FACE_POSITIVE_X; face < D3DCUBEMAP_FACE_NEGATIVE_Z; ++ face)
				{
					tempTextureCube->AddDirtyRect(static_cast<D3DCUBEMAP_FACES>(face), NULL);
				}

				d3dDevice_->UpdateTexture(d3dTextureCube_.get(), tempTextureCube.get());
				d3dTextureCube_ = tempTextureCube;

				this->QueryBaseTexture();
			}
			break;

		default:
			assert(false);
			break;
		}
	}

	boost::shared_ptr<IDirect3DTexture9> D3D9Texture::CreateTexture2D(uint32_t usage, D3DPOOL pool)
	{
		IDirect3DTexture9* d3dTexture2D;
		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
			this->NumMipMaps(), usage, ConvertFormat(format_),
			pool, &d3dTexture2D));
		return MakeCOMPtr(d3dTexture2D);
	}

	boost::shared_ptr<IDirect3DVolumeTexture9> D3D9Texture::CreateTexture3D(uint32_t usage, D3DPOOL pool)
	{
		IDirect3DVolumeTexture9* d3dTexture3D;
		TIF(D3DXCreateVolumeTexture(d3dDevice_.get(), this->Width(), this->Height(), this->Depth(),
			this->NumMipMaps(), usage, ConvertFormat(format_),
			pool, &d3dTexture3D));
		return MakeCOMPtr(d3dTexture3D);
	}

	boost::shared_ptr<IDirect3DCubeTexture9> D3D9Texture::CreateTextureCube(uint32_t usage, D3DPOOL pool)
	{
		IDirect3DCubeTexture9* d3dTextureCube;
		TIF(D3DXCreateCubeTexture(d3dDevice_.get(), this->Width(), 
			this->NumMipMaps(), usage, ConvertFormat(format_),
			pool, &d3dTextureCube));
		return MakeCOMPtr(d3dTextureCube);
	}

	void D3D9Texture::QueryBaseTexture()
	{
		IDirect3DBaseTexture9* d3dBaseTexture;
		switch (type_)
		{
		case TT_1D:
		case TT_2D:
            d3dTexture2D_->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&d3dBaseTexture));
			break;

		case TT_3D:
			d3dTexture3D_->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&d3dBaseTexture));
			break;

		case TT_Cube:
			d3dTextureCube_->QueryInterface(IID_IDirect3DBaseTexture9, reinterpret_cast<void**>(&d3dBaseTexture));
			break;
		}

		d3dBaseTexture_ = MakeCOMPtr(d3dBaseTexture);
	}
}
