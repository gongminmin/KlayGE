// D3D9Texture.cpp
// KlayGE D3D9纹理类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 去掉了ZBuffer (2005.10.12)
//
// 2.7.0
// 可以读取RenderTarget (2005.6.18)
// 加速了拷贝到RenderTarget (2005.6.22)
// 增加了AddressingMode, Filtering和Anisotropy (2005.6.27)
// 增加了MaxMipLevel和MipMapLodBias (2005.6.28)
//
// 2.6.0
// 增加了对surface的检查 (2005.5.15)
//
// 2.4.0
// 增加了1D/2D/3D/cube的支持 (2005.3.8)
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

#include <KlayGE/D3D9/D3D9Texture.hpp>

#pragma comment(lib, "d3d9.lib")
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx9d.lib")
#else
	#pragma comment(lib, "d3dx9.lib")
#endif

namespace KlayGE
{
	D3D9Texture::D3D9Texture(TextureType type)
					: Texture(type)
	{
	}

	D3D9Texture::~D3D9Texture()
	{
	}

	std::wstring const & D3D9Texture::Name() const
	{
		static const std::wstring name(L"Direct3D9 Texture");
		return name;
	}

	uint32_t D3D9Texture::Width(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	uint32_t D3D9Texture::Height(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	uint32_t D3D9Texture::Depth(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	void D3D9Texture::CopySurfaceToMemory(boost::shared_ptr<IDirect3DSurface9> const & surface, void* data)
	{
		D3DLOCKED_RECT d3d_rc;
		TIF(surface->LockRect(&d3d_rc, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		uint8_t* dst = static_cast<uint8_t*>(data);
		uint8_t* src = static_cast<uint8_t*>(d3d_rc.pBits);

		D3DSURFACE_DESC desc;
		surface->GetDesc(&desc);

		uint32_t const srcPitch = d3d_rc.Pitch;
		uint32_t const dstPitch = desc.Width * bpp_ / 8;

		for (uint32_t i = 0; i < desc.Height; ++ i)
		{
			std::copy(src, src + dstPitch, dst);

			src += srcPitch;
			dst += dstPitch;
		}

		surface->UnlockRect();
	}

	void D3D9Texture::CopyToMemory1D(int /*level*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyToMemory2D(int /*level*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyToMemory3D(int /*level*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyToMemoryCube(CubeFaces /*face*/, int /*level*/, void* /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyMemoryToTexture1D(int /*level*/, void* /*data*/, PixelFormat /*pf*/,
		uint32_t /*dst_width*/, uint32_t /*dst_xOffset*/, uint32_t /*src_width*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyMemoryToTexture2D(int /*level*/, void* /*data*/, PixelFormat /*pf*/,
		uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
		uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyMemoryToTexture3D(int /*level*/, void* /*data*/, PixelFormat /*pf*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_zOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyMemoryToTextureCube(CubeFaces /*face*/, int /*level*/, void* /*data*/, PixelFormat /*pf*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::Usage(Texture::TextureUsage usage)
	{
		if (usage != usage_)
		{
			this->OnLostDevice();
			usage_ = usage;
			this->OnResetDevice();
		}
	}

	D3DFORMAT D3D9Texture::ConvertFormat(PixelFormat format)
	{
		switch (format)
		{
		case PF_L8:
			return D3DFMT_L8;

		case PF_A8:
			return D3DFMT_A8;

		case PF_AL4:
			return D3DFMT_A4L4;

		case PF_L16:
			return D3DFMT_L16;

		case PF_AL8:
			return D3DFMT_A8L8;

		case PF_R5G6B5:
			return D3DFMT_R5G6B5;

		case PF_ARGB4:
			return D3DFMT_A4R4G4B4;

		case PF_XRGB8:
			return D3DFMT_X8R8G8B8;

		case PF_ARGB8:
		case PF_ARGB8_SRGB:
			return D3DFMT_A8R8G8B8;

		case PF_A2RGB10:
			return D3DFMT_A2B10G10R10;

		case PF_R16F:
			return D3DFMT_R16F;

		case PF_GR16F:
			return D3DFMT_G16R16F;
		case PF_ABGR16F:
			return D3DFMT_A16B16G16R16F;

		case PF_R32F:
			return D3DFMT_R32F;

		case PF_GR32F:
			return D3DFMT_G32R32F;

		case PF_ABGR32F:
			return D3DFMT_A32B32G32R32F;

		case PF_DXT1:
		case PF_DXT1_SRGB:
			return D3DFMT_DXT1;

		case PF_DXT3:
		case PF_DXT3_SRGB:
			return D3DFMT_DXT3;

		case PF_DXT5:
		case PF_DXT5_SRGB:
			return D3DFMT_DXT5;

		case PF_D16:
			return D3DFMT_D16;

		case PF_D24X8:
			return D3DFMT_D24X8;

		case PF_D24S8:
			return D3DFMT_D24S8;

		default:
			BOOST_ASSERT(false);
			return D3DFMT_UNKNOWN;
		}
	}

	PixelFormat D3D9Texture::ConvertFormat(D3DFORMAT format)
	{
		switch (format)
		{
		case D3DFMT_L8:
			return PF_L8;

		case D3DFMT_A8:
			return PF_A8;

		case D3DFMT_A4L4:
			return PF_AL4;

		case D3DFMT_L16:
			return PF_L16;

		case D3DFMT_A8L8:
			return PF_AL8;

		case D3DFMT_R5G6B5:
			return PF_R5G6B5;

		case D3DFMT_A4R4G4B4:
			return PF_ARGB4;

		case D3DFMT_X8R8G8B8:
			return PF_XRGB8;

		case D3DFMT_A8R8G8B8:
			return PF_ARGB8;

		case D3DFMT_A2B10G10R10:
			return PF_A2RGB10;

		case D3DFMT_R16F:
			return PF_R16F;

		case D3DFMT_G16R16F:
			return PF_GR16F;

		case D3DFMT_A16B16G16R16F:
			return PF_ABGR16F;

		case D3DFMT_R32F:
			return PF_R32F;

		case D3DFMT_G32R32F:
			return PF_GR32F;

		case D3DFMT_A32B32G32R32F:
			return PF_ABGR32F;

		case D3DFMT_DXT1:
			return PF_DXT1;

		case D3DFMT_DXT3:
			return PF_DXT3;

		case D3DFMT_DXT5:
			return PF_DXT5;

		case D3DFMT_D16:
			return PF_D16;

		case D3DFMT_D24X8:
			return PF_D24X8;

		case D3DFMT_D24S8:
			return PF_D24S8;

		default:
			BOOST_ASSERT(false);
			return PF_Unknown;
		}
	}
}
