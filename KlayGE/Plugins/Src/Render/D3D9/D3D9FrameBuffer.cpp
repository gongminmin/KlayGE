// D3D9FrameBuffer.hpp
// KlayGE D3D9渲染纹理类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 改为FrameBuffer (2006.5.30)
//
// 3.0.0
// 在D3D9FrameBuffer中建立DepthStencil Buffer (2005.10.12)
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
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>

#define NOMINMAX
#include <d3d9.h>

#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>

namespace KlayGE
{
	D3D9FrameBuffer::D3D9FrameBuffer()
	{
		left_ = 0;
		top_ = 0;

		viewport_.left	= left_;
		viewport_.top	= top_;
	}

	void D3D9FrameBuffer::AttachTexture2D(uint32_t n, TexturePtr texture2D)
	{
		BOOST_ASSERT(Texture::TT_2D == texture2D->Type());

		if ((n < privateTexs_.size()) && gbuffers_[n])
		{
			this->Detach(n);
		}

		if (n != 0)
		{
			if (!privateTexs_.empty())
			{
				if (privateTexs_[0])
				{
					BOOST_ASSERT(privateTexs_[0]->Width(0) == texture2D->Width(0));
					BOOST_ASSERT(privateTexs_[0]->Height(0) == texture2D->Height(0));
					BOOST_ASSERT(privateTexs_[0]->Bpp() == texture2D->Bpp());
				}
			}
		}

		if (Texture::TU_RenderTarget != texture2D->Usage())
		{
			texture2D->Usage(Texture::TU_RenderTarget);
		}

		this->UpdateParams(n, texture2D);

		D3D9Texture2D const & tex(*checked_cast<D3D9Texture2D const *>(privateTexs_[n].get()));
		IDirect3DSurface9* surface;
		tex.D3DTexture2D()->GetSurfaceLevel(0, &surface);
		renderSurfaces_[n] = MakeCOMPtr(surface);

		active_ = true;
	}

	void D3D9FrameBuffer::AttachTextureCube(uint32_t n, TexturePtr textureCube, Texture::CubeFaces face)
	{
		BOOST_ASSERT(Texture::TT_Cube == textureCube->Type());

		if ((n < privateTexs_.size()) && gbuffers_[n])
		{
			this->Detach(n);
		}

		if (n != 0)
		{
			if (!privateTexs_.empty())
			{
				if (privateTexs_[0])
				{
					BOOST_ASSERT(privateTexs_[0]->Width(0) == textureCube->Width(0));
					BOOST_ASSERT(privateTexs_[0]->Height(0) == textureCube->Height(0));
					BOOST_ASSERT(privateTexs_[0]->Bpp() == textureCube->Bpp());
				}
			}
		}

		if (Texture::TU_RenderTarget != textureCube->Usage())
		{
			textureCube->Usage(Texture::TU_RenderTarget);
		}

		this->UpdateParams(n, textureCube);
		faces_[n] = face;

		D3D9TextureCube const & tex(*checked_cast<D3D9TextureCube const *>(privateTexs_[n].get()));
		IDirect3DSurface9* surface;
		tex.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face), 0, &surface);
		renderSurfaces_[n] = MakeCOMPtr(surface);

		active_ = true;
	}

	void D3D9FrameBuffer::AttachGraphicsBuffer(uint32_t n, GraphicsBufferPtr gb,
			uint32_t width, uint32_t height)
	{
		BOOST_ASSERT(gb->Usage() != BU_Static);

		if ((n < privateTexs_.size()) && privateTexs_[n])
		{
			this->Detach(n);
		}

		if (n != 0)
		{
			if (!privateTexs_.empty())
			{
				if (privateTexs_[0])
				{
					BOOST_ASSERT(privateTexs_[0]->Width(0) == width);
					BOOST_ASSERT(privateTexs_[0]->Height(0) == height);
					BOOST_ASSERT(privateTexs_[0]->Bpp() == 128);
				}
			}
		}

		this->UpdateParams(n, gb, width, height);

		renderSurfaces_[n] = this->CreateGBSurface(D3DPOOL_DEFAULT);

		active_ = true;
	}
	
	void D3D9FrameBuffer::Detach(uint32_t n)
	{
		BOOST_ASSERT(n < privateTexs_.size());

		privateTexs_[n].reset();

		if (gbuffers_[n])
		{
			this->CopyToGraphicsBuffer(n);

			gbuffers_[n].reset();
		}

		renderSurfaces_[n].reset();
	}

	void D3D9FrameBuffer::UpdateParams(uint32_t n, TexturePtr texture)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (n < re.DeviceCaps().max_simultaneous_rts)
		{
			if (privateTexs_.size() < n + 1)
			{
				privateTexs_.resize(n + 1);
				faces_.resize(n + 1);
				renderSurfaces_.resize(n + 1);

				gbuffers_.resize(n + 1);
			}
		}
		else
		{
			THR(E_FAIL);
		}

		privateTexs_[n] = texture;
		if ((privateTexs_[n]->Width(0) != width_) || (privateTexs_[n]->Height(0) != height_))
		{
			width_ = privateTexs_[n]->Width(0);
			height_ = privateTexs_[n]->Height(0);

			this->CreateDepthStencilBuffer();
		}

		viewport_.width		= width_;
		viewport_.height	= height_;

		colorDepth_ = privateTexs_[n]->Bpp();
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

	void D3D9FrameBuffer::UpdateParams(uint32_t n, GraphicsBufferPtr gb, uint32_t width, uint32_t height)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (n < re.DeviceCaps().max_simultaneous_rts)
		{
			if (privateTexs_.size() < n + 1)
			{
				privateTexs_.resize(n + 1);
				faces_.resize(n + 1);
				renderSurfaces_.resize(n + 1);

				gbuffers_.resize(n + 1);
			}
		}
		else
		{
			THR(E_FAIL);
		}

		gbuffers_[n] = gb;
		if (0 == n)
		{
			width_ = width;
			height_ = height;

			this->CreateDepthStencilBuffer();
		}

		viewport_.width		= width_;
		viewport_.height	= height_;

		colorDepth_ = 128;
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

	D3D9FrameBuffer::IDirect3DSurface9Ptr D3D9FrameBuffer::CreateGBSurface(D3DPOOL pool)
	{
		IDirect3DSurface9* surface = NULL;

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		boost::shared_ptr<IDirect3DDevice9> d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();
		if (D3DPOOL_SYSTEMMEM == pool)
		{
			TIF(d3d_device->CreateOffscreenPlainSurface(width_, height_,
				D3DFMT_A32B32G32R32F, pool, &surface, NULL));
		}
		else
		{
			TIF(d3d_device->CreateRenderTarget(width_, height_, D3DFMT_A32B32G32R32F,
				D3DMULTISAMPLE_NONE, 0, true, &surface, NULL));
		}

		return MakeCOMPtr(surface);
	}

	boost::shared_ptr<IDirect3DSurface9> D3D9FrameBuffer::D3DRenderSurface(uint32_t n) const
	{
		if (n < renderSurfaces_.size())
		{
			return renderSurfaces_[n];
		}
		else
		{
			return boost::shared_ptr<IDirect3DSurface9>();
		}
	}
	
	boost::shared_ptr<IDirect3DSurface9> D3D9FrameBuffer::D3DRenderZBuffer() const
	{
		return depthStencilSurface_;
	}

	void D3D9FrameBuffer::CreateDepthStencilBuffer()
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

	void D3D9FrameBuffer::CopyToGraphicsBuffer(uint32_t n)
	{
		D3DLOCKED_RECT locked_rect;
		TIF(renderSurfaces_[n]->LockRect(&locked_rect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		GraphicsBuffer::Mapper mapper(*gbuffers_[n], BA_Write_Only);

		const float* src = static_cast<float*>(locked_rect.pBits);
		float* dst = mapper.Pointer<float>();
		for (uint32_t i = 0; i < height_; ++ i)
		{
			std::copy(src, src + width_ * 4, dst);

			src += locked_rect.Pitch / sizeof(float);
			dst += width_ * 4;
		}

		renderSurfaces_[n]->UnlockRect();
	}

	void D3D9FrameBuffer::DoOnLostDevice()
	{
		for (size_t i = 0; i < renderSurfaces_.size(); ++ i)
		{
			renderSurfaces_[i].reset();
		}
		depthStencilSurface_.reset();

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		boost::shared_ptr<IDirect3DDevice9> d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();
		for (size_t i = 0; i < gbuffers_.size(); ++ i)
		{
			if (gbuffers_[i])
			{
				IDirect3DSurface9Ptr sys_mem_surf = this->CreateGBSurface(D3DPOOL_SYSTEMMEM);

				TIF(d3d_device->GetRenderTargetData(renderSurfaces_[i].get(), sys_mem_surf.get()));
				renderSurfaces_[i] = sys_mem_surf;
			}
		}
	}
	
	void D3D9FrameBuffer::DoOnResetDevice()
	{
		for (size_t i = 0; i < renderSurfaces_.size(); ++ i)
		{
			if (privateTexs_[i])
			{
				IDirect3DSurface9* surface = NULL;

				switch (privateTexs_[i]->Type())
				{
				case Texture::TT_2D:
					{
						D3D9Texture2D const & tex(*checked_cast<D3D9Texture2D const *>(privateTexs_[i].get()));
						tex.D3DTexture2D()->GetSurfaceLevel(0, &surface);
					}
					break;

				case Texture::TT_Cube:
					{
						D3D9TextureCube const & tex(*checked_cast<D3D9TextureCube const *>(privateTexs_[i].get()));
						tex.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(faces_[i]), 0, &surface);
					}
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}

				renderSurfaces_[i] = MakeCOMPtr(surface);
			}
		}

		this->CreateDepthStencilBuffer();

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		boost::shared_ptr<IDirect3DDevice9> d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();
		for (size_t i = 0; i < gbuffers_.size(); ++ i)
		{
			if (gbuffers_[i])
			{
				IDirect3DSurface9Ptr default_surf = this->CreateGBSurface(D3DPOOL_DEFAULT);

				TIF(D3DXLoadSurfaceFromSurface(renderSurfaces_[i].get(), NULL, NULL,
					default_surf.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
				renderSurfaces_[i] = default_surf;
			}
		}
	}
}
