// D3D9Typedefs.hpp
// KlayGE 一些d3d9相关的typedef 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9TYPEDEFS_HPP
#define _D3D9TYPEDEFS_HPP

#define NOMINMAX
#include <d3d9.h>

namespace KlayGE
{
	typedef boost::shared_ptr<IDirect3D9>				ID3D9Ptr;
	typedef boost::shared_ptr<IDirect3DDevice9>			ID3D9DevicePtr;
	typedef boost::shared_ptr<IDirect3DTexture9>		ID3D9TexturePtr;
	typedef boost::shared_ptr<IDirect3DVolumeTexture9>	ID3D9VolumeTexturePtr;
	typedef boost::shared_ptr<IDirect3DCubeTexture9>	ID3D9CubeTexturePtr;
	typedef boost::shared_ptr<IDirect3DBaseTexture9>	ID3D9BaseTexturePtr;
	typedef boost::shared_ptr<IDirect3DSurface9>		ID3D9SurfacePtr;
	typedef boost::shared_ptr<IDirect3DVolume9>			ID3D9VolumePtr;
	typedef boost::shared_ptr<IDirect3DSurface9>		ID3D9SurfacePtr;
}

#endif		// _D3D9TYPEDEFS_HPP
