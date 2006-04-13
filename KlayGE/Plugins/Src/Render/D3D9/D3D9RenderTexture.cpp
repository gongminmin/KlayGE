// D3D9RenderTexture.hpp
// KlayGE D3D9渲染纹理类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 在D3D9RenderTexture中建立DepthStencil Buffer (2005.10.12)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderTexture.hpp>
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

		viewport_.left	= left_;
		viewport_.top	= top_;
	}

	void D3D9RenderTexture::AttachTexture2D(TexturePtr texture2D)
	{
		BOOST_ASSERT(Texture::TT_2D == texture2D->Type());

		if (Texture::TU_RenderTarget != texture2D->Usage())
		{
			texture2D->Usage(Texture::TU_RenderTarget);
		}

		this->UpdateParams(texture2D);

		D3D9Texture const & tex(*checked_cast<D3D9Texture const *>(privateTex_.get()));
		IDirect3DSurface9* surface;
		tex.D3DTexture2D()->GetSurfaceLevel(0, &surface);
		renderSurface_ = MakeCOMPtr(surface);

		active_ = true;
	}

	void D3D9RenderTexture::AttachTextureCube(TexturePtr textureCube, Texture::CubeFaces face)
	{
		BOOST_ASSERT(Texture::TT_Cube == textureCube->Type());

		if (Texture::TU_RenderTarget != textureCube->Usage())
		{
			textureCube->Usage(Texture::TU_RenderTarget);
		}

		face_ = face;
		this->UpdateParams(textureCube);

		D3D9Texture const & tex(*checked_cast<D3D9Texture const *>(privateTex_.get()));
		IDirect3DSurface9* surface;
		tex.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), 0, &surface);
		renderSurface_ = MakeCOMPtr(surface);

		active_ = true;
	}

	void D3D9RenderTexture::UpdateParams(TexturePtr texture)
	{
		privateTex_ = texture;
		if ((privateTex_->Width(0) != width_) || (privateTex_->Height(0) != height_))
		{
			width_ = privateTex_->Width(0);
			height_ = privateTex_->Height(0);

			this->CreateDepthStencilBuffer();
		}

		viewport_.width		= width_;
		viewport_.height	= height_;

		colorDepth_ = privateTex_->Bpp();
		isDepthBuffered_ = depthStencilSurface_;

		if (isDepthBuffered_)
		{
			D3DSURFACE_DESC desc;
			depthStencilSurface_->GetDesc(&desc);
			switch (desc.Format)
			{
			case D3DFMT_D15S1:
				depthBits_ = 15;
				stencilBits_ = 1;
				break;

			case D3DFMT_D16:
				depthBits_ = 16;
				stencilBits_ = 0;
				break;

			case D3DFMT_D24X8:
				depthBits_ = 24;
				stencilBits_ = 0;
				break;

			case D3DFMT_D24S8:
				depthBits_ = 24;
				stencilBits_ = 8;
				break;

			case D3DFMT_D32:
				depthBits_ = 32;
				stencilBits_ = 0;
				break;

			default:
				depthBits_ = 0;
				stencilBits_ = 0;
				break;
			}
		}
		else
		{
			depthBits_ = 0;
			stencilBits_ = 0;
		}
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

	void D3D9RenderTexture::CreateDepthStencilBuffer()
	{
		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		boost::shared_ptr<IDirect3DDevice9> d3dDevice = renderEngine.D3DDevice();

		IDirect3DSurface9* tempSurf = NULL;
		D3DSURFACE_DESC tempDesc;

		// Get the format of the depth stencil surface.
		d3dDevice->GetDepthStencilSurface(&tempSurf);
		if (tempSurf)
		{
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();

			TIF(d3dDevice->CreateDepthStencilSurface(width_, height_, tempDesc.Format,
				tempDesc.MultiSampleType, 0, FALSE, &tempSurf, NULL));
			depthStencilSurface_ = MakeCOMPtr(tempSurf);
		}
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

		BOOST_ASSERT(false);
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
				BOOST_ASSERT(false);
				break;
			}

			renderSurface_ = MakeCOMPtr(surface);
			this->CreateDepthStencilBuffer();
		}
	}
}
