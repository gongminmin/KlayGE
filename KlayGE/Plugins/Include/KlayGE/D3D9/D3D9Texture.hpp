#ifndef _D3D9TEXTURE_HPP
#define _D3D9TEXTURE_HPP

#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/COMPtr.hpp>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	class D3D9Texture : public Texture
	{
	public:
		D3D9Texture(U32 width, U32 height, U16 numMipMaps, PixelFormat format, TextureUsage usage = TU_Default);
		~D3D9Texture();

		const WString& Name() const;

		void CustomAttribute(const String& name, void* pData);

		void CopyToTexture(Texture& target);
		void CopyMemoryToTexture(void* pData, PixelFormat pf,
			U32 width = 0, U32 height = 0, U32 xOffset = 0, U32 yOffset = 0);
		void CopyToMemory(void* data);

		const COMPtr<IDirect3DTexture9>& D3DTexture() const
			{ return this->d3dTexture_; }
		const COMPtr<IDirect3DSurface9>& DepthStencil() const
			{ return this->renderZBuffer_; }

	private:
		COMPtr<IDirect3DDevice9>	d3dDevice_;
		COMPtr<IDirect3DTexture9>	d3dTexture_;		// The actual texture surface
		COMPtr<IDirect3DTexture9>	d3dTempTexture_;

		COMPtr<IDirect3DSurface9>	renderZBuffer_;		// The z-buffer for the render surface.
	};

	typedef SharedPtr<D3D9Texture>		D3D9TexturePtr;
}

#endif			// _D3D9TEXTURE_HPP
