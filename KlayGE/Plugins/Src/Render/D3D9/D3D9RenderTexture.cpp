#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderTexture.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#include <d3d9.h>

#include <KlayGE/D3D9/D3D9RenderTexture.hpp>

namespace KlayGE
{
	D3D9RenderTexture::D3D9RenderTexture(U32 width, U32 height)
	{
		left_ = 0;
		top_ = 0;
		privateTex_ = Engine::RenderFactoryInstance().MakeTexture(width, height, 0, PF_X8R8G8B8, Texture::TU_RenderTarget);
		width_ = privateTex_->Width();
		height_ = privateTex_->Height();

		viewport_.left		= left_;
		viewport_.top		= top_;
		viewport_.width		= width_;
		viewport_.height	= height_;
	}

	void D3D9RenderTexture::CustomAttribute(const String& name, void* pData)
	{
		if ("DDBACKBUFFER" == name)
		{
			D3D9TexturePtr tex(privateTex_);
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			tex->D3DTexture()->GetSurfaceLevel(0, &(*pSurf));
			(*pSurf)->Release();

			return;
		}

		if ("D3DZBUFFER" == name)
		{
			D3D9TexturePtr tex(privateTex_);
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = tex->DepthStencil().Get();

			return;
		}

		if ("DDFRONTBUFFER" == name)
		{
			D3D9TexturePtr tex(privateTex_);
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			tex->D3DTexture()->GetSurfaceLevel(0, &(*pSurf));
			(*pSurf)->Release();

			return;
		}

		if ("HWND" == name)
		{
			HWND* pHwnd = reinterpret_cast<HWND*>(pData);
			*pHwnd = NULL;

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
