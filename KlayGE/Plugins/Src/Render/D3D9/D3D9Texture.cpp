// D3D9Font.cpp
// KlayGE D3D9Texture类 实现文件
// Ver 2.0.5
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/Memory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <d3dx9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <KlayGE/D3D9/D3D9Texture.hpp>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

namespace
{
	using namespace KlayGE;

	D3DFORMAT Convert(KlayGE::PixelFormat format)
	{
		switch (format)
		{
		case PF_L8:
			return D3DFMT_L8;

		case PF_A8:
			return D3DFMT_A8;

		case PF_A4L4:
			return D3DFMT_A4L4;

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
		}

		return D3DFMT_UNKNOWN;
	}

	KlayGE::PixelFormat Convert(D3DFORMAT format)
	{
		switch (format)
		{
		case D3DFMT_L8:
			return PF_L8;

		case D3DFMT_A8:
			return PF_A8;

		case D3DFMT_A4L4:
			return PF_A4L4;

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
		}

		return PF_Unknown;
	}

	void ColorMasks(KlayGE::PixelFormat format, U32& red, U32& green, U32& blue, U32& alpha,
		U8& redOffset, U8& greenOffset, U8& blueOffset, U8& alphaOffset)
	{
		switch (format)
		{
		case PF_L8:
			red			= 0x000000FF;
			green		= 0x000000FF;
			blue		= 0x000000FF;
			alpha		= 0x00000000;

			redOffset	= 0;
			greenOffset	= 0;
			blueOffset	= 0;
			alphaOffset	= 0;
			break;

		case PF_A8:
			red			= 0x00000000;
			green		= 0x00000000;
			blue		= 0x00000000;
			alpha		= 0x000000FF;

			redOffset	= 0xFF;
			greenOffset	= 0xFF;
			blueOffset	= 0xFF;
			alphaOffset	= 0;
			break;

		case PF_A4L4:
			red			= 0x0000000F;
			green		= 0x0000000F;
			blue		= 0x0000000F;
			alpha		= 0x000000F0;

			redOffset	= 0;
			greenOffset	= 0;
			blueOffset	= 0;
			alphaOffset	= 4;
			break;

		case PF_R5G6B5:
			red			= 0x0000F800;
			green		= 0x000007E0;
			blue		= 0x0000001F;
			alpha		= 0x00000000;

			redOffset	= 11;
			greenOffset	= 5;
			blueOffset	= 0;
			alphaOffset	= 0xFF;
			break;

		case PF_A4R4G4B4:
			red			= 0x00000F00;
			green		= 0x000000F0;
			blue		= 0x0000000F;
			alpha		= 0x0000F000;

			redOffset	= 8;
			greenOffset	= 4;
			blueOffset	= 0;
			alphaOffset	= 12;
			break;

		case PF_X8R8G8B8:
			red			= 0x00FF0000;
			green		= 0x0000FF00;
			blue		= 0x000000FF;
			alpha		= 0x00000000;

			redOffset	= 16;
			greenOffset	= 8;
			blueOffset	= 0;
			alphaOffset	= 0xFF;
			break;

		case PF_A8R8G8B8:
			red			= 0x00FF0000;
			green		= 0x0000FF00;
			blue		= 0x000000FF;
			alpha		= 0xFF000000;

			redOffset	= 16;
			greenOffset	= 8;
			blueOffset	= 0;
			alphaOffset	= 24;
			break;

		case PF_A2R10G10B10:
			red			= 0x3FF00000;
			green		= 0x000FFC00;
			blue		= 0x000003FF;
			alpha		= 0xC0000000;

			redOffset	= 20;
			greenOffset	= 10;
			blueOffset	= 0;
			alphaOffset	= 30;
			break;

		default:
			red			= 0x00000000;
			green		= 0x00000000;
			blue		= 0x00000000;
			alpha		= 0x00000000;
			
			redOffset	= 0xFF;
			greenOffset	= 0xFF;
			blueOffset	= 0xFF;
			alphaOffset	= 0xFF;
			break;
		}
	}

	void ColorMasks(D3DFORMAT format, U32& red, U32& green, U32& blue, U32& alpha,
		U8& redOffset, U8& greenOffset, U8& blueOffset, U8& alphaOffset)
	{
		switch (format)
		{
		case D3DFMT_L8:
			red			= 0x000000FF;
			green		= 0x000000FF;
			blue		= 0x000000FF;
			alpha		= 0x00000000;

			redOffset	= 0;
			greenOffset	= 0;
			blueOffset	= 0;
			alphaOffset	= 0;
			break;

		case D3DFMT_A8:
			red			= 0x00000000;
			green		= 0x00000000;
			blue		= 0x00000000;
			alpha		= 0x000000FF;

			redOffset	= 0xFF;
			greenOffset	= 0xFF;
			blueOffset	= 0xFF;
			alphaOffset	= 0;
			break;

		case D3DFMT_A4L4:
			red			= 0x0000000F;
			green		= 0x0000000F;
			blue		= 0x0000000F;
			alpha		= 0x000000F0;

			redOffset	= 0;
			greenOffset	= 0;
			blueOffset	= 0;
			alphaOffset	= 4;
			break;

		case D3DFMT_R5G6B5:
			red			= 0x0000F800;
			green		= 0x000007E0;
			blue		= 0x0000001F;
			alpha		= 0x00000000;

			redOffset	= 11;
			greenOffset	= 5;
			blueOffset	= 0;
			alphaOffset	= 0xFF;
			break;

		case D3DFMT_A4R4G4B4:
			red			= 0x00000F00;
			green		= 0x000000F0;
			blue		= 0x0000000F;
			alpha		= 0x0000F000;

			redOffset	= 8;
			greenOffset	= 4;
			blueOffset	= 0;
			alphaOffset	= 12;
			break;

		case D3DFMT_R8G8B8:
		case D3DFMT_X8R8G8B8:
			red			= 0x00FF0000;
			green		= 0x0000FF00;
			blue		= 0x000000FF;
			alpha		= 0x00000000;

			redOffset	= 16;
			greenOffset	= 8;
			blueOffset	= 0;
			alphaOffset	= 0xFF;
			break;

		case D3DFMT_A8R8G8B8:
			red			= 0x00FF0000;
			green		= 0x0000FF00;
			blue		= 0x000000FF;
			alpha		= 0xFF000000;

			redOffset	= 16;
			greenOffset	= 8;
			blueOffset	= 0;
			alphaOffset	= 24;
			break;

		case D3DFMT_A2B10G10R10:
			red			= 0x3FF00000;
			green		= 0x000FFC00;
			blue		= 0x000003FF;
			alpha		= 0xC0000000;

			redOffset	= 20;
			greenOffset	= 10;
			blueOffset	= 0;
			alphaOffset	= 30;
			break;

		default:
			red			= 0x00000000;
			green		= 0x00000000;
			blue		= 0x00000000;
			alpha		= 0x00000000;

			redOffset	= 0xFF;
			greenOffset	= 0xFF;
			blueOffset	= 0xFF;
			alphaOffset	= 0xFF;
			break;
		}
	}

	U8 NumberOfBits(U32 mask)
	{
		U8 bits(0);
		while (mask)
		{
			mask = mask & (mask - 1);
			++ bits;
		}

		return bits;
	}
}

namespace KlayGE
{
	D3D9Texture::D3D9Texture(U32 width, U32 height,
								U16 numMipMaps, PixelFormat format, TextureUsage usage)
	{
		d3dDevice_ = static_cast<const D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice();

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
				this->NumMipMaps(), 0, Convert(format),
				D3DPOOL_SYSTEMMEM, &d3dTexture));
			d3dTempTexture_ = COMPtr<IDirect3DTexture9>(d3dTexture);

			TIF(D3DXCreateTexture(d3dDevice_.get(), this->Width(), this->Height(),
				this->NumMipMaps(), 0, Convert(format),
				D3DPOOL_DEFAULT, &d3dTexture));
			d3dTexture_ = COMPtr<IDirect3DTexture9>(d3dTexture);

			D3DSURFACE_DESC desc;
			// Check the actual dimensions vs requested
			TIF(d3dTexture_->GetLevelDesc(0, &desc));
			width_	= desc.Width;
			height_	= desc.Height;
			format_ = Convert(desc.Format);
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
			d3dTexture_ = COMPtr<IDirect3DTexture9>(d3dTexture);

			D3DSURFACE_DESC desc;
			// Check the actual dimensions vs requested
			TIF(d3dTexture_->GetLevelDesc(0, &desc));
			width_	= desc.Width;
			height_	= desc.Height;
			format_ = Convert(desc.Format);
			bpp_	= PixelFormatBits(format_);

			// Now get the format of the depth stencil surface.
			d3dDevice_->GetDepthStencilSurface(&tempSurf);
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			// We also create a depth buffer for our render target.
			TIF(d3dDevice_->CreateDepthStencilSurface(width_, height_, tempDesc.Format, tempDesc.MultiSampleType, NULL, 
				FALSE, &tempSurf, NULL));
			renderZBuffer_ = COMPtr<IDirect3DSurface9>(tempSurf);
		}
	}

	D3D9Texture::~D3D9Texture()
	{
	}

	const std::wstring& D3D9Texture::Name() const
	{
		static std::wstring name(L"Direct3D9 Texture");
		return name;
	}

	void D3D9Texture::CopyToTexture(Texture& target)
	{
		D3D9Texture& other(static_cast<D3D9Texture&>(target));

		if (this->Usage() == other.Usage())
		{
			RECT dstRC = { 0, 0, other.Width(), other.Height() };

			COMPtr<IDirect3DSurface9> src, dst;

			IDirect3DSurface9* temp;
			TIF(this->D3DTexture()->GetSurfaceLevel(0, &temp));
			src = COMPtr<IDirect3DSurface9>(temp);

			TIF(other.D3DTexture()->GetSurfaceLevel(0, &temp));
			dst = COMPtr<IDirect3DSurface9>(temp);

			TIF(d3dDevice_->StretchRect(src.get(), NULL, dst.get(), &dstRC, D3DTEXF_NONE));
		}
	}

	void D3D9Texture::CopyMemoryToTexture(void* pData, PixelFormat pf,
		U32 width, U32 height, U32 xOffset, U32 yOffset)
	{
		U16 bpp(PixelFormatBits(pf));

		if (0 == width)
		{
			width = width_;
		}
		if (0 == height)
		{
			height = height_;
		}

		U8* pBuffer(static_cast<U8*>(pData));

		U32 srcRed, srcGreen, srcBlue, srcAlpha;
		U8 srcRedOffset, srcGreenOffset, srcBlueOffset, srcAlphaOffset;
		ColorMasks(pf, srcRed, srcGreen, srcBlue, srcAlpha,
			srcRedOffset, srcGreenOffset, srcBlueOffset, srcAlphaOffset);

		U32 destRed, destGreen, destBlue, destAlpha;
		U8 destRedOffset, destGreenOffset, destBlueOffset, destAlphaOffset;
		ColorMasks(this->Format(), destRed, destGreen, destBlue, destAlpha,
			destRedOffset, destGreenOffset, destBlueOffset, destAlphaOffset);

		RECT rc = { xOffset, yOffset, xOffset + width, yOffset + height };
		D3DLOCKED_RECT d3dlr;
		TIF(d3dTempTexture_->LockRect(0, &d3dlr, &rc, D3DLOCK_NOSYSLOCK));

		const U32 srcPitch(width * bpp / 8);
		const U16 destPitch(d3dlr.Pitch);
		U8* pBits(static_cast<U8*>(d3dlr.pBits));

		if ((srcRed == destRed) && (srcGreen == destGreen)
			&& (srcBlue == destBlue) && (srcAlpha == destAlpha))
		{
			for (U32 y = 0; y < height; ++ y)
			{
				void* dst(pBits + y * destPitch);
				pBuffer = static_cast<U8*>(pData) + y * srcPitch;

				MemoryLib::Copy(dst, pBuffer, srcPitch);
			}
		}
		else
		{
			const U8 srcRedBitCount(NumberOfBits(srcRed));
			const U8 srcGreenBitCount(NumberOfBits(srcGreen));
			const U8 srcBlueBitCount(NumberOfBits(srcBlue));
			const U8 srcAlphaBitCount(NumberOfBits(srcAlpha));

			const U8 destRedBitCount(NumberOfBits(destRed));
			const U8 destGreenBitCount(NumberOfBits(destGreen));
			const U8 destBlueBitCount(NumberOfBits(destBlue));
			const U8 destAlphaBitCount(NumberOfBits(destAlpha));

			for (U32 y = 0; y < height; ++ y)
			{
				U8* pDest(pBits + y * destPitch);
				U8* pSrc(pBuffer + y * width);

				for (U32 x = 0; x < width; ++ x)
				{
					U32 srcPixel(0);
					MemoryLib::Copy(&srcPixel, pSrc, bpp / 8);
					pSrc += bpp / 8;

					// 转化成R8G8B8A8
					U32 red(static_cast<U8>((srcPixel & srcRed) >> srcRedOffset << (8 - srcRedBitCount)));
					U32 green(static_cast<U8>((srcPixel & srcGreen) >> srcGreenOffset<< (8 - srcGreenBitCount)));
					U32 blue(static_cast<U8>((srcPixel & srcBlue) >> srcBlueOffset<< (8 - srcBlueBitCount)));
					U32 alpha(static_cast<U8>((srcPixel & srcAlpha) >> srcAlphaOffset<< (8 - srcAlphaBitCount)));

					red		= red >> (8 - destRedBitCount) << destRedOffset;
					green	= green >> (8 - destGreenBitCount) << destGreenOffset;
					blue	= blue >> (8 - destBlueBitCount) << destBlueOffset;
					alpha	= alpha >> (8 - destAlphaBitCount) << destAlphaOffset;

					const U32 destPixel(red | green | blue | alpha);
					MemoryLib::Copy(pDest, &destPixel, bpp_ / 8);
					pDest += bpp_ / 8;
				}
			}
		}

		d3dTempTexture_->UnlockRect(0);

		TIF(D3DXFilterTexture(d3dTempTexture_.get(), NULL, D3DX_DEFAULT, D3DX_DEFAULT));
		TIF(d3dDevice_->UpdateTexture(d3dTempTexture_.get(), d3dTexture_.get()));
	}

	void D3D9Texture::CustomAttribute(const std::string& name, void* pData)
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
