// D3D9Texture.hpp
// KlayGE D3D9纹理类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 改为派生自D3D9Resource (2005.3.3)
// 增加了D3DBaseTexture (2005.3.7)
// 增加了1D/2D/3D/cube的支持 (2005.3.8)
//
// 2.3.0
// 增加了CopyToMemory (2005.2.6)
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9TEXTURE_HPP
#define _D3D9TEXTURE_HPP

#include <boost/smart_ptr.hpp>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <KlayGE/D3D9/D3D9Resource.hpp>
#include <KlayGE/D3D9/D3D9RenderEngine.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9Texture : public Texture, public D3D9Resource
	{
		typedef boost::shared_ptr<IDirect3DDevice9>			IDirect3DDevice9Ptr;
		typedef boost::shared_ptr<IDirect3DTexture9>		IDirect3DTexture9Ptr;
		typedef boost::shared_ptr<IDirect3DVolumeTexture9>	IDirect3DVolumeTexture9Ptr;
		typedef boost::shared_ptr<IDirect3DCubeTexture9>	IDirect3DCubeTexture9Ptr;
		typedef boost::shared_ptr<IDirect3DBaseTexture9>	IDirect3DBaseTexture9Ptr;
		typedef boost::shared_ptr<IDirect3DSurface9>		IDirect3DSurface9Ptr;
		typedef boost::shared_ptr<IDirect3DVolume9>			IDirect3DVolume9Ptr;

	public:
		D3D9Texture(uint32_t width, uint16_t numMipMaps, PixelFormat format, TextureUsage usage);
		D3D9Texture(uint32_t width, uint32_t height, uint16_t numMipMaps, PixelFormat format, TextureUsage usage);
		D3D9Texture(uint32_t width, uint32_t height, uint32_t depth, uint16_t numMipMaps, PixelFormat format, TextureUsage usage);
		D3D9Texture(uint32_t size, bool cube, uint16_t numMipMaps, PixelFormat format, TextureUsage usage);
		~D3D9Texture();

		std::wstring const & Name() const;

		void CustomAttribute(std::string const & name, void* pData);

		void CopyToTexture(Texture& target);
		void CopyToMemory(int level, void* data);
		void CopyMemoryToTexture1D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t xOffset);
		void CopyMemoryToTexture2D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset);
		void CopyMemoryToTexture3D(int level, void* data, PixelFormat pf,
			uint32_t width, uint32_t height, uint32_t depth,
			uint32_t xOffset, uint32_t yOffset, uint32_t zOffset);
		void CopyMemoryToTextureCube(CubeFaces face, int level, void* data, PixelFormat pf,
			uint32_t size, uint32_t xOffset);

		void BuildMipSubLevels();

		boost::shared_ptr<IDirect3DTexture9> D3DTexture2D() const
			{ return d3dTexture2D_; }
		boost::shared_ptr<IDirect3DVolumeTexture9> D3DTexture3D() const
			{ return d3dTexture3D_; }
		boost::shared_ptr<IDirect3DCubeTexture9> D3DTextureCube() const
			{ return d3dTextureCube_; }
		boost::shared_ptr<IDirect3DBaseTexture9> D3DBaseTexture() const
			{ return d3dBaseTexture_; }
		boost::shared_ptr<IDirect3DSurface9> DepthStencil() const
			{ return renderZBuffer_; }

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		IDirect3DTexture9Ptr CreateTexture2D(uint32_t usage, D3DPOOL pool);
		IDirect3DVolumeTexture9Ptr CreateTexture3D(uint32_t usage, D3DPOOL pool);
		IDirect3DCubeTexture9Ptr CreateTextureCube(uint32_t usage, D3DPOOL pool);

		void CreateDepthStencilBuffer(D3DPOOL pool);

		void QueryBaseTexture();
		void UpdateParams();

	private:
		IDirect3DDevice9Ptr			d3dDevice_;

		IDirect3DTexture9Ptr		d3dTexture2D_;
		IDirect3DVolumeTexture9Ptr	d3dTexture3D_;
		IDirect3DCubeTexture9Ptr	d3dTextureCube_;

		IDirect3DBaseTexture9Ptr	d3dBaseTexture_;

		IDirect3DSurface9Ptr		renderZBuffer_;		// The z-buffer for the render surface.
	};

	typedef boost::shared_ptr<D3D9Texture> D3D9TexturePtr;
}

#endif			// _D3D9TEXTURE_HPP
