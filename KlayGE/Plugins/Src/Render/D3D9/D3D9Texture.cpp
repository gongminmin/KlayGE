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
	D3D9Texture::D3D9Texture(uint32_t width, uint32_t height,
								uint16_t numMipMaps, PixelFormat format, TextureUsage usage)
	{
		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		numMipMaps_ = numMipMaps;
		format_		= format;
		width_		= width;
		height_		= height;

		bpp_ = PixelFormatBits(format);

		usage_ = usage;

		if (TU_Default == usage_)
		{
			IDirect3DTexture9* d3dTexture;
			// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), D3DUSAGE_DYNAMIC, ConvertFormat(format),
				D3DPOOL_DEFAULT, &d3dTexture));
			d3dTexture_ = MakeCOMPtr(d3dTexture);

			D3DSURFACE_DESC desc;
			// Check the actual dimensions vs requested
			TIF(d3dTexture_->GetLevelDesc(0, &desc));
			width_	= desc.Width;
			height_	= desc.Height;
			format_ = ConvertFormat(desc.Format);
			bpp_	= PixelFormatBits(format_);
			numMipMaps_ = static_cast<uint16_t>(d3dTexture_->GetLevelCount());
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

			IDirect3DTexture9* d3dTexture;
			// Now we create the texture.
			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), D3DUSAGE_RENDERTARGET,
				tempDesc.Format, D3DPOOL_DEFAULT, &d3dTexture));
			d3dTexture_ = MakeCOMPtr(d3dTexture);

			D3DSURFACE_DESC desc;
			// Check the actual dimensions vs requested
			TIF(d3dTexture_->GetLevelDesc(0, &desc));
			width_	= desc.Width;
			height_	= desc.Height;
			format_ = ConvertFormat(desc.Format);
			bpp_	= PixelFormatBits(format_);
			numMipMaps_ = static_cast<uint16_t>(d3dTexture_->GetLevelCount());

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

		RECT dstRC = { 0, 0, other.Width(), other.Height() };

		boost::shared_ptr<IDirect3DSurface9> src, dst;

		uint32_t maxLevel = 1;
		if (this->NumMipMaps() == target.NumMipMaps())
		{
			maxLevel = this->NumMipMaps();
		}

		for (uint32_t i = 0; i < maxLevel; ++ i)
		{
			IDirect3DSurface9* temp;
			TIF(this->D3DTexture()->GetSurfaceLevel(i, &temp));
			src = MakeCOMPtr(temp);

			TIF(other.D3DTexture()->GetSurfaceLevel(i, &temp));
			dst = MakeCOMPtr(temp);

			TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DTEXF_NONE, 0));
		}

		if (this->NumMipMaps() != target.NumMipMaps())
		{
			target.BuildMipSubLevels();
		}		
	}

	void D3D9Texture::CopyToMemory(int level, void* data)
	{
		assert(data != NULL);

		D3DLOCKED_RECT d3d_rc;
		d3dTexture_->LockRect(level, &d3d_rc, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);

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

		d3dTexture_->UnlockRect(0);
	}

	void D3D9Texture::CopyMemoryToTexture(int level, void* data, PixelFormat pf,
		uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset)
	{
		IDirect3DSurface9* temp;
		TIF(this->D3DTexture()->GetSurfaceLevel(level, &temp));
		boost::shared_ptr<IDirect3DSurface9> shadow = MakeCOMPtr(temp);

		RECT srcRc = { 0, 0, width, height };
		RECT dstRc = { xOffset, yOffset, xOffset + width, yOffset + height };
		TIF(D3DXLoadSurfaceFromMemory(shadow.get(), NULL, &dstRc, data, ConvertFormat(pf),
			width * PixelFormatBits(pf) / 8, NULL, &srcRc, D3DX_FILTER_NONE, 0));
		TIF(this->D3DTexture()->AddDirtyRect(&dstRc));
	}

	void D3D9Texture::BuildMipSubLevels()
	{
		boost::shared_ptr<IDirect3DTexture9> d3dTextureShadow;
		IDirect3DTexture9* d3dTexture;
		TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
			this->NumMipMaps(), 0, ConvertFormat(format_),
			D3DPOOL_SYSTEMMEM, &d3dTexture));
		d3dTextureShadow = MakeCOMPtr(d3dTexture);

		IDirect3DSurface9* temp;
		TIF(this->D3DTexture()->GetSurfaceLevel(0, &temp));
		boost::shared_ptr<IDirect3DSurface9> src = MakeCOMPtr(temp);

		TIF(d3dTextureShadow->GetSurfaceLevel(0, &temp));
		boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

		TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));

		TIF(D3DXFilterTexture(d3dTextureShadow.get(), NULL, D3DX_DEFAULT, D3DX_DEFAULT));
		TIF(d3dDevice_->UpdateTexture(d3dTextureShadow.get(), this->D3DTexture().get()));
	}

	void D3D9Texture::CustomAttribute(std::string const & name, void* pData)
	{
		// Valid attributes and their equivalent native functions:
		// D3DDEVICE            : D3DDeviceDriver
		// HWND					: WindowHandle

		if ("D3DDEVICE" == name)
		{
			IDirect3DDevice9** pDev = reinterpret_cast<IDirect3DDevice9**>(pData);
			*pDev = d3dDevice_.get();

			return;
		}

		if ("HWND" == name)
		{
			HWND *phWnd = reinterpret_cast<HWND*>(pData);
			*phWnd = NULL;

			return;
		}

		if ("IsTexture" == name)
		{
			bool* b = reinterpret_cast<bool*>(pData);
			*b = true;

			return;
		}
	}

	void D3D9Texture::DoOnLostDevice()
	{
		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		if (TU_Default == usage_)
		{
			IDirect3DTexture9* d3dTexture;
			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), 0, ConvertFormat(format_),
				D3DPOOL_SYSTEMMEM, &d3dTexture));
			boost::shared_ptr<IDirect3DTexture9> tempTexture = MakeCOMPtr(d3dTexture);

			for (uint16_t i = 0; i < this->NumMipMaps(); ++ i)
			{
				IDirect3DSurface9* temp;
				TIF(this->D3DTexture()->GetSurfaceLevel(i, &temp));
				boost::shared_ptr<IDirect3DSurface9> src = MakeCOMPtr(temp);

				TIF(tempTexture->GetSurfaceLevel(i, &temp));
				boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

				TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, src.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
			}
			tempTexture->AddDirtyRect(NULL);
			d3dTexture_ = tempTexture;
		}
		else
		{
			IDirect3DSurface9* tempSurf;
			D3DSURFACE_DESC tempDesc;

			d3dDevice_->GetRenderTarget(0, &tempSurf);
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			IDirect3DTexture9* d3dTexture;
			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), 0, tempDesc.Format, D3DPOOL_SYSTEMMEM, &d3dTexture));
			boost::shared_ptr<IDirect3DTexture9> tempTexture = MakeCOMPtr(d3dTexture);
			for (uint16_t i = 0; i < this->NumMipMaps(); ++ i)
			{
				IDirect3DSurface9* temp;
				TIF(this->D3DTexture()->GetSurfaceLevel(i, &temp));
				boost::shared_ptr<IDirect3DSurface9> src = MakeCOMPtr(temp);

				TIF(tempTexture->GetSurfaceLevel(i, &temp));
				boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

				TIF(d3dDevice_->GetRenderTargetData(src.get(), dst.get()));
			}
			tempTexture->AddDirtyRect(NULL);
			d3dTexture_ = tempTexture;

			d3dDevice_->GetDepthStencilSurface(&tempSurf);
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			TIF(d3dDevice_->CreateOffscreenPlainSurface(width_, height_, tempDesc.Format,
				D3DPOOL_SYSTEMMEM, &tempSurf, NULL));
			boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(tempSurf);
			TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, renderZBuffer_.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
			renderZBuffer_ = dst;
		}
	}

	void D3D9Texture::DoOnResetDevice()
	{
		d3dDevice_ = static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

		if (TU_Default == usage_)
		{
			IDirect3DTexture9* d3dTexture;
			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), D3DUSAGE_DYNAMIC, ConvertFormat(format_),
				D3DPOOL_DEFAULT, &d3dTexture));
			boost::shared_ptr<IDirect3DTexture9> tempTexture = MakeCOMPtr(d3dTexture);
			tempTexture->AddDirtyRect(NULL);

			d3dDevice_->UpdateTexture(d3dTexture_.get(), tempTexture.get());
			d3dTexture_ = tempTexture;
		}
		else
		{
			IDirect3DSurface9* tempSurf;
			D3DSURFACE_DESC tempDesc;

			d3dDevice_->GetRenderTarget(0, &tempSurf);
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			IDirect3DTexture9* d3dTexture;
			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), D3DUSAGE_RENDERTARGET,
				tempDesc.Format, D3DPOOL_DEFAULT, &d3dTexture));
			boost::shared_ptr<IDirect3DTexture9> tempTexture = MakeCOMPtr(d3dTexture);
			tempTexture->AddDirtyRect(NULL);

			d3dDevice_->UpdateTexture(d3dTexture_.get(), tempTexture.get());
			d3dTexture_ = tempTexture;

			d3dDevice_->GetDepthStencilSurface(&tempSurf);
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			TIF(d3dDevice_->CreateDepthStencilSurface(width_, height_, tempDesc.Format, tempDesc.MultiSampleType, 0, 
				FALSE, &tempSurf, NULL));
			boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(tempSurf);
			TIF(D3DXLoadSurfaceFromSurface(dst.get(), NULL, NULL, renderZBuffer_.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
			renderZBuffer_ = dst;
		}
	}
}
