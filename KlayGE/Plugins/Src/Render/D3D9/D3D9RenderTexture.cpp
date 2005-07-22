// D3D9RenderTexture.hpp
// KlayGE D3D9渲染纹理类 实现文件
// Ver 2.3.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderTexture.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#define NOMINMAX
#include <d3d9.h>

#include <KlayGE/D3D9/D3D9RenderTexture.hpp>

namespace KlayGE
{
	D3D9RenderTexture::D3D9RenderTexture()
	{
		left_ = 0;
		top_ = 0;

		viewport_.left		= left_;
		viewport_.top		= top_;
	}

	void D3D9RenderTexture::AttachTexture2D(TexturePtr texture2D)
	{
		assert(Texture::TU_RenderTarget == texture2D->Usage());
		assert(Texture::TT_2D == texture2D->Type());
		assert(dynamic_cast<D3D9Texture*>(texture2D.get()) != NULL);

		privateTex_ = texture2D;
		width_ = privateTex_->Width(0);
		height_ = privateTex_->Height(0);

		viewport_.width		= width_;
		viewport_.height	= height_;

		D3D9Texture const & tex(static_cast<D3D9Texture&>(*privateTex_));
		IDirect3DSurface9* surface;
		tex.D3DTexture2D()->GetSurfaceLevel(0, &surface);
		renderSurface_ = MakeCOMPtr(surface);
		depthStencilSurface_ = tex.DepthStencil();

		colorDepth_ = privateTex_->Bpp();
		isDepthBuffered_ = depthStencilSurface_;
	}

	void D3D9RenderTexture::AttachTextureCube(TexturePtr textureCube, Texture::CubeFaces face)
	{
		assert(Texture::TU_RenderTarget == textureCube->Usage());
		assert(Texture::TT_Cube == textureCube->Type());
		assert(dynamic_cast<D3D9Texture*>(textureCube.get()) != NULL);

		face_ = face;
		privateTex_ = textureCube;
		width_ = privateTex_->Width(0);
		height_ = privateTex_->Height(0);

		viewport_.width		= width_;
		viewport_.height	= height_;

		D3D9Texture const & tex(static_cast<D3D9Texture&>(*privateTex_));
		IDirect3DSurface9* surface;
		tex.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), 0, &surface);
		renderSurface_ = MakeCOMPtr(surface);
		depthStencilSurface_ = tex.DepthStencil();
	}

	void D3D9RenderTexture::DetachTexture()
	{
		privateTex_.reset();
	}

	boost::shared_ptr<IDirect3DSurface9> D3D9RenderTexture::D3DRenderSurface() const
	{
		return renderSurface_;
	}
	
	boost::shared_ptr<IDirect3DSurface9> D3D9RenderTexture::D3DRenderZBuffer() const
	{
		return depthStencilSurface_;
	}

	void D3D9RenderTexture::CustomAttribute(std::string const & name, void* pData)
	{
		if (("DDBACKBUFFER" == name) || ("DDFRONTBUFFER" == name))
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = this->D3DRenderSurface().get();

			return;
		}

		if ("D3DZBUFFER" == name)
		{
			IDirect3DSurface9** pSurf = reinterpret_cast<IDirect3DSurface9**>(pData);
			*pSurf = this->D3DRenderZBuffer().get();

			return;
		}

		if ("HWND" == name)
		{
			HWND* pHwnd = reinterpret_cast<HWND*>(pData);
			*pHwnd = NULL;

			return;
		}

		assert(false);
	}

	void D3D9RenderTexture::DoOnLostDevice()
	{
		renderSurface_.reset();
		depthStencilSurface_.reset();
	}
	
	void D3D9RenderTexture::DoOnResetDevice()
	{
		if (privateTex_)
		{
			D3D9Texture const & tex(static_cast<D3D9Texture&>(*privateTex_));
			IDirect3DSurface9* surface = NULL;

			switch (privateTex_->Type())
			{
			case Texture::TT_2D:
				tex.D3DTexture2D()->GetSurfaceLevel(0, &surface);
				break;

			case Texture::TT_Cube:
				tex.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face_), 0, &surface);
				break;

			default:
				assert(false);
				break;
			}

			renderSurface_ = MakeCOMPtr(surface);
			depthStencilSurface_ = tex.DepthStencil();
		}
	}
}
