#ifndef _D3D9TEXTURE_HPP
#define _D3D9TEXTURE_HPP

#include <boost/smart_ptr.hpp>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>

#ifdef _DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9Texture : public Texture
	{
	public:
		D3D9Texture(uint32_t width, uint32_t height, uint16_t numMipMaps, PixelFormat format, TextureUsage usage = TU_Default);
		~D3D9Texture();

		std::wstring const & Name() const;

		void CustomAttribute(std::string const & name, void* pData);

		void CopyToTexture(Texture& target);
		void CopyMemoryToTexture(void* pData, PixelFormat pf,
			uint32_t width = 0, uint32_t height = 0, uint32_t xOffset = 0, uint32_t yOffset = 0);

		boost::shared_ptr<IDirect3DTexture9> const & D3DTexture() const
			{ return this->d3dTexture_; }
		boost::shared_ptr<IDirect3DSurface9> const & DepthStencil() const
			{ return this->renderZBuffer_; }

	private:
		boost::shared_ptr<IDirect3DDevice9>		d3dDevice_;
		boost::shared_ptr<IDirect3DTexture9>	d3dTexture_;		// The actual texture surface
		boost::shared_ptr<IDirect3DTexture9>	d3dTempTexture_;

		boost::shared_ptr<IDirect3DSurface9>	renderZBuffer_;		// The z-buffer for the render surface.
	};

	typedef boost::shared_ptr<D3D9Texture> D3D9TexturePtr;
}

#endif			// _D3D9TEXTURE_HPP
