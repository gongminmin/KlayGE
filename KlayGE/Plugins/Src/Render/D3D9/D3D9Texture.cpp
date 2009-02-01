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
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>
#include <boost/assert.hpp>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

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
	D3D9Texture::D3D9Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
					: Texture(type, sample_count, sample_quality, access_hint)
	{
		if (access_hint & EAH_GPU_Write)
		{
			BOOST_ASSERT(!(access_hint & EAH_CPU_Read));
			BOOST_ASSERT(!(access_hint & EAH_CPU_Write));
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

	void D3D9Texture::CopyToTexture1D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_xOffset*/, uint32_t /*src_width*/, uint32_t /*src_xOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyToTexture2D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyToTexture3D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_zOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/,
			uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/, uint32_t /*src_zOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopyToTextureCube(Texture& /*target*/, CubeFaces /*face*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::CopySurfaceToMemory(ID3D9SurfacePtr const & surface, void* data)
	{
		D3DLOCKED_RECT d3d_rc;
		TIF(surface->LockRect(&d3d_rc, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		uint8_t* dst = static_cast<uint8_t*>(data);
		uint8_t* src = static_cast<uint8_t*>(d3d_rc.pBits);

		D3DSURFACE_DESC desc;
		surface->GetDesc(&desc);

		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (EF_BC1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			uint32_t const image_size = ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * block_size;
			std::copy(src, src + image_size, dst);
		}
		else
		{
			uint32_t const srcPitch = d3d_rc.Pitch;
			uint32_t const dstPitch = desc.Width * bpp_ / 8;

			for (uint32_t i = 0; i < desc.Height; ++ i)
			{
				std::copy(src, src + dstPitch, dst);

				src += srcPitch;
				dst += dstPitch;
			}
		}

		surface->UnlockRect();
	}

	void D3D9Texture::CopyVolumeToMemory(ID3D9VolumePtr const & volume, void* data)
	{
		D3DLOCKED_BOX d3d_box;
		volume->LockBox(&d3d_box, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);

		uint8_t* dst = static_cast<uint8_t*>(data);
		uint8_t* src = static_cast<uint8_t*>(d3d_box.pBits);

		D3DVOLUME_DESC desc;
		volume->GetDesc(&desc);

		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (EF_BC1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			uint32_t const image_size = ((desc.Width + 3) / 4) * ((desc.Height + 3) / 4) * desc.Depth * block_size;
			std::copy(src, src + image_size, dst);
		}
		else
		{
			uint32_t const srcPitch = d3d_box.RowPitch;
			uint32_t const dstPitch = desc.Width * bpp_ / 8;

			for (uint32_t j = 0; j < desc.Depth; ++ j)
			{
				src = static_cast<uint8_t*>(d3d_box.pBits) + j * d3d_box.SlicePitch;

				for (uint32_t i = 0; i < desc.Height; ++ i)
				{
					std::copy(src, src + dstPitch, dst);

					src += srcPitch;
					dst += dstPitch;
				}
			}
		}

		volume->UnlockBox();
	}

	void D3D9Texture::CopySurfaceToSurface(ID3D9SurfacePtr const & dst, ID3D9SurfacePtr const & src)
	{
		D3DSURFACE_DESC src_desc;
		src->GetDesc(&src_desc);
		D3DSURFACE_DESC dst_desc;
		dst->GetDesc(&dst_desc);
		BOOST_ASSERT(src_desc.Width == dst_desc.Width);
		BOOST_ASSERT(src_desc.Height == dst_desc.Height);
		BOOST_ASSERT(src_desc.Format == dst_desc.Format);

		D3DLOCKED_RECT src_locked_rect;
		src->LockRect(&src_locked_rect, NULL, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
		D3DLOCKED_RECT dst_locked_rect;
		dst->LockRect(&dst_locked_rect, NULL, D3DLOCK_NOSYSLOCK);

		uint8_t* src_ptr = static_cast<uint8_t*>(src_locked_rect.pBits);
		uint8_t* dst_ptr = static_cast<uint8_t*>(dst_locked_rect.pBits);
		uint32_t line_size;
		if (IsCompressedFormat(format_))
		{
			int block_size;
			if (EF_BC1 == format_)
			{
				block_size = 8;
			}
			else
			{
				block_size = 16;
			}

			line_size = ((src_desc.Width + 3) / 4) * block_size;

			for (uint32_t y = 0; y < (src_desc.Height + 3) / 4; ++ y)
			{
				memcpy(dst_ptr, src_ptr, line_size);
				dst_ptr += dst_locked_rect.Pitch;
				src_ptr += src_locked_rect.Pitch;
			}
		}
		else
		{
			line_size = src_desc.Width * NumFormatBytes(format_);

			for (uint32_t y = 0; y < src_desc.Height; ++ y)
			{
				memcpy(dst_ptr, src_ptr, line_size);
				dst_ptr += dst_locked_rect.Pitch;
				src_ptr += src_locked_rect.Pitch;
			}
		}

		src->UnlockRect();
		dst->UnlockRect();
	}

	void D3D9Texture::Map1D(int /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*width*/,
			void*& /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::Map2D(int /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::Map3D(int /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::MapCube(CubeFaces /*face*/, int /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::Unmap1D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::Unmap2D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::Unmap3D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D9Texture::UnmapCube(CubeFaces /*face*/, int /*level*/)
	{
		BOOST_ASSERT(false);
	}
}
