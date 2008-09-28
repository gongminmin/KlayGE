// D3D10Texture.cpp
// KlayGE D3D10纹理类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include <cstring>
#include <boost/assert.hpp>

#include <d3d10.h>
#include <d3dx10.h>

#include <KlayGE/D3D10/D3D10Texture.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "d3d10.lib")
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "d3dx10d.lib")
#else
	#pragma comment(lib, "d3dx10.lib")
#endif
#endif

namespace KlayGE
{
	D3D10Texture::D3D10Texture(TextureType type, uint32_t access_hint)
					: Texture(type, access_hint)
	{
		if (access_hint & EAH_GPU_Write)
		{
			BOOST_ASSERT(!(access_hint & EAH_CPU_Read));
			BOOST_ASSERT(!(access_hint & EAH_CPU_Write));
		}
	}

	D3D10Texture::~D3D10Texture()
	{
	}

	std::wstring const & D3D10Texture::Name() const
	{
		static const std::wstring name(L"Direct3D10 Texture");
		return name;
	}

	void D3D10Texture::CopyToTexture1D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_xOffset*/, uint32_t /*src_width*/, uint32_t /*src_xOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::CopyToTexture2D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::CopyToTexture3D(Texture& /*target*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_depth*/,
			uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/, uint32_t /*dst_zOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_depth*/,
			uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/, uint32_t /*src_zOffset*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::CopyToTextureCube(Texture& /*target*/, CubeFaces /*face*/, int /*level*/,
			uint32_t /*dst_width*/, uint32_t /*dst_height*/, uint32_t /*dst_xOffset*/, uint32_t /*dst_yOffset*/,
			uint32_t /*src_width*/, uint32_t /*src_height*/, uint32_t /*src_xOffset*/, uint32_t /*src_yOffset*/)
	{
		BOOST_ASSERT(false);
	}

	uint32_t D3D10Texture::Width(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	uint32_t D3D10Texture::Height(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	uint32_t D3D10Texture::Depth(int /*level*/) const
	{
		BOOST_ASSERT(false);
		return 0;
	}

	void D3D10Texture::Map1D(int /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*width*/,
			void*& /*data*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::Map2D(int /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::Map3D(int /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*z_offset*/,
			uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
			void*& /*data*/, uint32_t& /*row_pitch*/, uint32_t& /*slice_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::MapCube(CubeFaces /*face*/, int /*level*/, TextureMapAccess /*tma*/,
			uint32_t /*x_offset*/, uint32_t /*y_offset*/, uint32_t /*width*/, uint32_t /*height*/,
			void*& /*data*/, uint32_t& /*row_pitch*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::Unmap1D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::Unmap2D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::Unmap3D(int /*level*/)
	{
		BOOST_ASSERT(false);
	}

	void D3D10Texture::UnmapCube(CubeFaces /*face*/, int /*level*/)
	{
		BOOST_ASSERT(false);
	}
}
