// D3D9RenderView.cpp
// KlayGE D3D9渲染视图类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>

#include <boost/assert.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>

namespace KlayGE
{
	D3D9RenderView::~D3D9RenderView()
	{
	}


	D3D9Texture1DRenderView::D3D9Texture1DRenderView(Texture& texture_1d, int level)
		: texture_1d_(static_cast<D3D9Texture1D&>(texture_1d)),
			level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d.Type());
		BOOST_ASSERT(dynamic_cast<D3D9Texture1D*>(&texture_1d) != NULL);

		IDirect3DSurface9* surface;
		texture_1d_.D3DTexture1D()->GetSurfaceLevel(level_, &surface);
		surface_ = MakeCOMPtr(surface);

		width_ = texture_1d_.Width(level);
		height_ = 1;
		bpp_ = texture_1d_.Bpp();
	}

	void D3D9Texture1DRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
		if (Texture::TU_RenderTarget != texture_1d_.Usage())
		{
			texture_1d_.Usage(Texture::TU_RenderTarget);
		}

		IDirect3DSurface9* surface;
		texture_1d_.D3DTexture1D()->GetSurfaceLevel(level_, &surface);
		surface_ = MakeCOMPtr(surface);
	}

	void D3D9Texture1DRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D9Texture1DRenderView::DoOnLostDevice()
	{
		surface_.reset();
	}

	void D3D9Texture1DRenderView::DoOnResetDevice()
	{
		IDirect3DSurface9* surface;
		texture_1d_.D3DTexture1D()->GetSurfaceLevel(level_, &surface);
		surface_ = MakeCOMPtr(surface);
	}


	D3D9Texture2DRenderView::D3D9Texture2DRenderView(Texture& texture_2d, int level)
		: texture_2d_(static_cast<D3D9Texture2D&>(texture_2d)),
			level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d.Type());
		BOOST_ASSERT(dynamic_cast<D3D9Texture2D*>(&texture_2d) != NULL);

		IDirect3DSurface9* surface;
		texture_2d_.D3DTexture2D()->GetSurfaceLevel(level_, &surface);
		surface_ = MakeCOMPtr(surface);

		width_ = texture_2d_.Width(level);
		height_ = texture_2d_.Height(level);
		bpp_ = texture_2d_.Bpp();
	}

	void D3D9Texture2DRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*n*/)
	{
		if (Texture::TU_RenderTarget != texture_2d_.Usage())
		{
			texture_2d_.Usage(Texture::TU_RenderTarget);
		}

		IDirect3DSurface9* surface;
		texture_2d_.D3DTexture2D()->GetSurfaceLevel(level_, &surface);
		surface_ = MakeCOMPtr(surface);
	}

	void D3D9Texture2DRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*n*/)
	{
	}

	void D3D9Texture2DRenderView::DoOnLostDevice()
	{
		surface_.reset();
	}

	void D3D9Texture2DRenderView::DoOnResetDevice()
	{
		IDirect3DSurface9* surface;
		texture_2d_.D3DTexture2D()->GetSurfaceLevel(level_, &surface);
		surface_ = MakeCOMPtr(surface);
	}


	D3D9TextureCubeRenderView::D3D9TextureCubeRenderView(Texture& texture_cube, Texture::CubeFaces face, int level)
		: texture_cube_(static_cast<D3D9TextureCube&>(texture_cube)),
			face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());
		BOOST_ASSERT(dynamic_cast<D3D9TextureCube*>(&texture_cube) != NULL);

		IDirect3DSurface9* surface;
		texture_cube_.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face_), level_, &surface);
		surface_ = MakeCOMPtr(surface);

		width_ = texture_cube_.Width(level);
		height_ = texture_cube_.Height(level);
		bpp_ = texture_cube_.Bpp();
	}

	void D3D9TextureCubeRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
		if (Texture::TU_RenderTarget != texture_cube_.Usage())
		{
			texture_cube_.Usage(Texture::TU_RenderTarget);
		}

		IDirect3DSurface9* surface;
		texture_cube_.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face_), level_, &surface);
		surface_ = MakeCOMPtr(surface);
	}

	void D3D9TextureCubeRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D9TextureCubeRenderView::DoOnLostDevice()
	{
		surface_.reset();
	}

	void D3D9TextureCubeRenderView::DoOnResetDevice()
	{
		IDirect3DSurface9* surface;
		texture_cube_.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face_), level_, &surface);
		surface_ = MakeCOMPtr(surface);
	}


	D3D9GraphicsBufferRenderView::D3D9GraphicsBufferRenderView(GraphicsBuffer& gb,
									uint32_t width, uint32_t height, PixelFormat pf)
		: gbuffer_(gb), pf_(pf)
	{
		BOOST_ASSERT(PF_ABGR32F == pf_);

		width_ = width;
		height_ = height;
		bpp_ = PixelFormatBits(pf_);
	}

	void D3D9GraphicsBufferRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
		surface_ = this->CreateGBSurface(D3DPOOL_DEFAULT);
	}

	void D3D9GraphicsBufferRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
		gbuffer_.Resize(width_ * height_ * sizeof(float) * 4);

		D3DLOCKED_RECT locked_rect;
		TIF(surface_->LockRect(&locked_rect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		GraphicsBuffer::Mapper mapper(gbuffer_, BA_Write_Only);

		const float* src = static_cast<float*>(locked_rect.pBits);
		float* dst = mapper.Pointer<float>();
		for (uint32_t i = 0; i < height_; ++ i)
		{
			std::copy(src, src + width_ * 4, dst);

			src += locked_rect.Pitch / sizeof(float);
			dst += width_ * 4;
		}

		surface_->UnlockRect();
	}

	ID3D9SurfacePtr D3D9GraphicsBufferRenderView::CreateGBSurface(D3DPOOL pool)
	{
		IDirect3DSurface9* surface = NULL;

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();
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

	void D3D9GraphicsBufferRenderView::DoOnLostDevice()
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		boost::shared_ptr<IDirect3DDevice9> d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		ID3D9SurfacePtr sys_mem_surf = this->CreateGBSurface(D3DPOOL_SYSTEMMEM);
		TIF(d3d_device->GetRenderTargetData(surface_.get(), sys_mem_surf.get()));
		surface_ = sys_mem_surf;
	}

	void D3D9GraphicsBufferRenderView::DoOnResetDevice()
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		boost::shared_ptr<IDirect3DDevice9> d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		ID3D9SurfacePtr default_surf = this->CreateGBSurface(D3DPOOL_DEFAULT);
		TIF(D3DXLoadSurfaceFromSurface(surface_.get(), NULL, NULL,
			default_surf.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
		surface_ = default_surf;
	}
}
