// D3D9Font.cpp
// KlayGE D3D9纹理类 实现文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.3.0
// 增加了对浮点纹理格式的支持 (2005.1.25)
// 改进了CopyMemoryToTexture (2005.2.1)
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
				this->NumMipMaps(), 0, ConvertFormat(format),
				D3DPOOL_SYSTEMMEM, &d3dTexture));
			d3dTempTexture_ = MakeCOMPtr(d3dTexture);

			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), 0, ConvertFormat(format),
				D3DPOOL_DEFAULT, &d3dTexture));
			d3dTexture_ = MakeCOMPtr(d3dTexture);

			D3DSURFACE_DESC desc;
			// Check the actual dimensions vs requested
			TIF(d3dTexture_->GetLevelDesc(0, &desc));
			width_	= desc.Width;
			height_	= desc.Height;
			format_ = ConvertFormat(desc.Format);
			bpp_	= PixelFormatBits(format_);
		}
		else
		{
			// We need to set the pixel format of the render target texture to be the same as the
			// one of the front buffer.
			IDirect3DSurface9* tempSurf;
			D3DSURFACE_DESC tempDesc;

			d3dDevice_->GetRenderTarget(0, &tempSurf);
			tempSurf->GetDesc(&tempDesc);
			D3DFORMAT d3dFmt = tempDesc.Format;
			tempSurf->Release();

			IDirect3DTexture9* d3dTexture;
			// Now we create the texture.
			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), D3DUSAGE_RENDERTARGET,
				d3dFmt, D3DPOOL_DEFAULT, &d3dTexture));
			d3dTexture_ = MakeCOMPtr(d3dTexture);

			D3DSURFACE_DESC desc;
			// Check the actual dimensions vs requested
			TIF(d3dTexture_->GetLevelDesc(0, &desc));
			width_	= desc.Width;
			height_	= desc.Height;
			format_ = ConvertFormat(desc.Format);
			bpp_	= PixelFormatBits(format_);

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

		if (this->Usage() == other.Usage())
		{
			RECT dstRC = { 0, 0, other.Width(), other.Height() };

			boost::shared_ptr<IDirect3DSurface9> src, dst;

			IDirect3DSurface9* temp;
			TIF(this->D3DTexture()->GetSurfaceLevel(0, &temp));
			src = MakeCOMPtr(temp);

			TIF(other.D3DTexture()->GetSurfaceLevel(0, &temp));
			dst = MakeCOMPtr(temp);

			TIF(d3dDevice_->StretchRect(src.get(), NULL, dst.get(), &dstRC, D3DTEXF_NONE));
		}
	}

	void D3D9Texture::CopyMemoryToTexture(void* data, PixelFormat pf,
		uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset)
	{
		IDirect3DSurface9* temp;
		TIF(d3dTempTexture_->GetSurfaceLevel(0, &temp));
		boost::shared_ptr<IDirect3DSurface9> dst = MakeCOMPtr(temp);

		RECT srcRc = { 0, 0, width, height };
		RECT dstRc = { xOffset, yOffset, xOffset + width, yOffset + height };
		TIF(D3DXLoadSurfaceFromMemory(dst.get(), NULL, &dstRc, data, ConvertFormat(pf),
			width * PixelFormatBits(pf) / 8, NULL, &srcRc, D3DX_DEFAULT, 0));

		TIF(D3DXFilterTexture(d3dTempTexture_.get(), NULL, D3DX_DEFAULT, D3DX_DEFAULT));
		TIF(d3dDevice_->UpdateTexture(d3dTempTexture_.get(), d3dTexture_.get()));
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
}
