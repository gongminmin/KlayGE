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
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGe/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>

namespace KlayGE
{
	D3D9RenderView::~D3D9RenderView()
	{
	}

	void D3D9RenderView::Clear(Color const & clr)
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		std::vector<IDirect3DSurface9*> old_rt(render_eng.DeviceCaps().max_simultaneous_rts, NULL);
		for (uint32_t i = 0; i < old_rt.size(); ++ i)
		{
			d3d_device->GetRenderTarget(i, &old_rt[i]);
		}
		IDirect3DSurface9* old_ds;
		d3d_device->GetDepthStencilSurface(&old_ds);

		for (uint32_t i = 1; i < old_rt.size(); ++ i)
		{
			d3d_device->SetRenderTarget(i, NULL);
		}
		d3d_device->SetDepthStencilSurface(NULL);
		d3d_device->SetRenderTarget(0, surface_.get());

		TIF(d3d_device->Clear(0, NULL, D3DCLEAR_TARGET,
			D3DCOLOR_COLORVALUE(clr.r(), clr.g(), clr.b(), clr.a()), 0, 0));

		for (uint32_t i = 0; i < old_rt.size(); ++ i)
		{
			d3d_device->SetRenderTarget(i, old_rt[i]);
			if (old_rt[i] != NULL)
			{
				old_rt[i]->Release();
			}
		}
		d3d_device->SetDepthStencilSurface(old_ds);
		if (old_ds != NULL)
		{
			old_ds->Release();
		}
	}

	void D3D9RenderView::Clear(float depth)
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		IDirect3DSurface9* old_ds;
		d3d_device->GetDepthStencilSurface(&old_ds);
		if (old_ds != surface_.get())
		{
			d3d_device->SetDepthStencilSurface(surface_.get());
		}

		TIF(d3d_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, depth, 0));

		if (old_ds != surface_.get())
		{
			d3d_device->SetDepthStencilSurface(old_ds);
		}
		old_ds->Release();
	}

	void D3D9RenderView::Clear(int32_t stencil)
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		IDirect3DSurface9* old_ds;
		d3d_device->GetDepthStencilSurface(&old_ds);
		if (old_ds != surface_.get())
		{
			d3d_device->SetDepthStencilSurface(surface_.get());
		}

		TIF(d3d_device->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0, stencil));

		if (old_ds != surface_.get())
		{
			d3d_device->SetDepthStencilSurface(old_ds);
		}
		old_ds->Release();
	}

	void D3D9RenderView::Clear(float depth, int32_t stencil)
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		IDirect3DSurface9* old_ds;
		d3d_device->GetDepthStencilSurface(&old_ds);
		if (old_ds != surface_.get())
		{
			d3d_device->SetDepthStencilSurface(surface_.get());
		}

		uint32_t flags = 0;
		if (IsDepthFormat(pf_))
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		if (IsStencilFormat(pf_))
		{
			flags |= D3DCLEAR_STENCIL;
		}

		TIF(d3d_device->Clear(0, NULL, flags, 0, depth, stencil));

		if (old_ds != surface_.get())
		{
			d3d_device->SetDepthStencilSurface(old_ds);
		}
		old_ds->Release();
	}


	D3D9SurfaceRenderView::D3D9SurfaceRenderView(ID3D9SurfacePtr surf)
	{
		surface_ = surf;

		D3DSURFACE_DESC desc;
		surface_->GetDesc(&desc);

		width_ = desc.Width;
		height_ = desc.Height;
		pf_ = D3D9Mapping::MappingFormat(desc.Format);
	}

	void D3D9SurfaceRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D9SurfaceRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D9SurfaceRenderView::DoOnLostDevice()
	{
	}

	void D3D9SurfaceRenderView::DoOnResetDevice()
	{
	}


	D3D9Texture1DRenderView::D3D9Texture1DRenderView(Texture& texture_1d, int level)
		: texture_1d_(*checked_cast<D3D9Texture1D*>(&texture_1d)),
			level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d.Type());
		BOOST_ASSERT(texture_1d_.AccessHint() & EAH_GPU_Write);

		IDirect3DSurface9* surface;
		texture_1d_.D3DTexture1D()->GetSurfaceLevel(level_, &surface);
		surface_ = MakeCOMPtr(surface);

		width_ = texture_1d_.Width(level);
		height_ = 1;
		pf_ = texture_1d_.Format();
	}

	void D3D9Texture1DRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
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
		: texture_2d_(*checked_cast<D3D9Texture2D*>(&texture_2d)),
			level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d.Type());
		BOOST_ASSERT(texture_2d_.AccessHint() & EAH_GPU_Write);

		IDirect3DSurface9* surface;
		texture_2d_.D3DTexture2D()->GetSurfaceLevel(level_, &surface);
		surface_ = MakeCOMPtr(surface);

		width_ = texture_2d_.Width(level);
		height_ = texture_2d_.Height(level);
		pf_ = texture_2d_.Format();
	}

	void D3D9Texture2DRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*n*/)
	{
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


	D3D9Texture3DRenderView::D3D9Texture3DRenderView(Texture& texture_3d, uint32_t slice, int level)
		: texture_3d_(*checked_cast<D3D9Texture3D*>(&texture_3d)),
			slice_(slice), level_(level)
	{
		BOOST_ASSERT(Texture::TT_3D == texture_3d.Type());
		BOOST_ASSERT(texture_3d_.Depth(level) > slice);

		width_ = texture_3d_.Width(level);
		height_ = texture_3d_.Height(level);
		pf_ = texture_3d_.Format();

		surface_ = this->CreateSurface(D3DPOOL_DEFAULT);
	}

	void D3D9Texture3DRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D9Texture3DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		this->OnUnbind(fb, att);
	}

	void D3D9Texture3DRenderView::OnUnbind(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
		IDirect3DVolume9* pTmp;
		texture_3d_.D3DTexture3D()->GetVolumeLevel(level_, &pTmp);
		ID3D9VolumePtr pVol = MakeCOMPtr(pTmp);

		D3DLOCKED_RECT locked_rect;
		TIF(surface_->LockRect(&locked_rect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		D3DBOX src_box = { 0, 0, width_, height_, 0, 1 };
		D3DBOX dst_box = { 0, 0, width_, height_, level_, level_ + 1 };
		D3DXLoadVolumeFromMemory(pVol.get(), NULL, &dst_box, locked_rect.pBits,
			D3D9Mapping::MappingFormat(pf_), locked_rect.Pitch, locked_rect.Pitch * height_,
			NULL, &src_box, D3DX_FILTER_NONE, 0);

		surface_->UnlockRect();
	}

	ID3D9SurfacePtr D3D9Texture3DRenderView::CreateSurface(D3DPOOL pool)
	{
		IDirect3DSurface9* surface = NULL;

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();
		if (D3DPOOL_SYSTEMMEM == pool)
		{
			TIF(d3d_device->CreateOffscreenPlainSurface(width_, height_,
				D3D9Mapping::MappingFormat(pf_), pool, &surface, NULL));
		}
		else
		{
			TIF(d3d_device->CreateRenderTarget(width_, height_, D3D9Mapping::MappingFormat(pf_),
				D3DMULTISAMPLE_NONE, 0, true, &surface, NULL));
		}

		return MakeCOMPtr(surface);
	}

	void D3D9Texture3DRenderView::DoOnLostDevice()
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		ID3D9SurfacePtr sys_mem_surf = this->CreateSurface(D3DPOOL_SYSTEMMEM);
		TIF(d3d_device->GetRenderTargetData(surface_.get(), sys_mem_surf.get()));
		surface_ = sys_mem_surf;
	}

	void D3D9Texture3DRenderView::DoOnResetDevice()
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		ID3D9SurfacePtr default_surf = this->CreateSurface(D3DPOOL_DEFAULT);
		TIF(D3DXLoadSurfaceFromSurface(surface_.get(), NULL, NULL,
			default_surf.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
		surface_ = default_surf;
	}


	D3D9TextureCubeRenderView::D3D9TextureCubeRenderView(Texture& texture_cube, Texture::CubeFaces face, int level)
		: texture_cube_(*checked_cast<D3D9TextureCube*>(&texture_cube)),
			face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube.Type());
		BOOST_ASSERT(texture_cube_.AccessHint() & EAH_GPU_Write);

		IDirect3DSurface9* surface;
		texture_cube_.D3DTextureCube()->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(face_), level_, &surface);
		surface_ = MakeCOMPtr(surface);

		width_ = texture_cube_.Width(level);
		height_ = texture_cube_.Height(level);
		pf_ = texture_cube_.Format();
	}

	void D3D9TextureCubeRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
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
									uint32_t width, uint32_t height, ElementFormat pf)
		: gbuffer_(gb)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;

		surface_ = this->CreateGBSurface(D3DPOOL_DEFAULT);
	}

	void D3D9GraphicsBufferRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}

	void D3D9GraphicsBufferRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
		this->CopyToGB();
	}

	void D3D9GraphicsBufferRenderView::OnUnbind(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
		this->CopyToGB();
	}

	ID3D9SurfacePtr D3D9GraphicsBufferRenderView::CreateGBSurface(D3DPOOL pool)
	{
		IDirect3DSurface9* surface = NULL;

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();
		if (D3DPOOL_SYSTEMMEM == pool)
		{
			TIF(d3d_device->CreateOffscreenPlainSurface(width_, height_,
				D3D9Mapping::MappingFormat(pf_), pool, &surface, NULL));
		}
		else
		{
			TIF(d3d_device->CreateRenderTarget(width_, height_, D3D9Mapping::MappingFormat(pf_),
				D3DMULTISAMPLE_NONE, 0, true, &surface, NULL));
		}

		return MakeCOMPtr(surface);
	}

	void D3D9GraphicsBufferRenderView::CopyToGB()
	{
		size_t const format_size = NumFormatBytes(pf_);

		gbuffer_.Resize(static_cast<uint32_t>(width_ * height_ * format_size));

		D3DLOCKED_RECT locked_rect;
		TIF(surface_->LockRect(&locked_rect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		GraphicsBuffer::Mapper mapper(gbuffer_, BA_Write_Only);

		const uint8_t* src = static_cast<uint8_t*>(locked_rect.pBits);
		uint8_t* dst = mapper.Pointer<uint8_t>();
		for (uint32_t i = 0; i < height_; ++ i)
		{
			std::copy(src, src + width_ * format_size, dst);

			src += locked_rect.Pitch;
			dst += width_ * format_size;
		}

		surface_->UnlockRect();
	}

	void D3D9GraphicsBufferRenderView::DoOnLostDevice()
	{
		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		ID3D9SurfacePtr sys_mem_surf = this->CreateGBSurface(D3DPOOL_SYSTEMMEM);
		TIF(d3d_device->GetRenderTargetData(surface_.get(), sys_mem_surf.get()));
		surface_ = sys_mem_surf;
	}

	void D3D9GraphicsBufferRenderView::DoOnResetDevice()
	{
		ID3D9SurfacePtr default_surf = this->CreateGBSurface(D3DPOOL_DEFAULT);
		TIF(D3DXLoadSurfaceFromSurface(surface_.get(), NULL, NULL,
			default_surf.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
		surface_ = default_surf;
	}


	D3D9DepthStencilRenderView::D3D9DepthStencilRenderView(uint32_t width, uint32_t height,
											ElementFormat pf, uint32_t multi_sample)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;

		if (multi_sample > 16)
		{
			multi_sample_ = D3DMULTISAMPLE_16_SAMPLES;
		}
		else
		{
			multi_sample_ = D3DMULTISAMPLE_TYPE(multi_sample);
		}

		surface_ = this->CreateSurface();
	}

	D3D9DepthStencilRenderView::D3D9DepthStencilRenderView(Texture& texture_1d_2d, int level, uint32_t multi_sample)
	{
		BOOST_ASSERT(IsDepthFormat(texture_1d_2d.Format()));

		BOOST_ASSERT((Texture::TT_2D == texture_1d_2d.Type()) || (Texture::TT_1D == texture_1d_2d.Type()));
		BOOST_ASSERT(texture_1d_2d.AccessHint() & EAH_GPU_Write);

		IDirect3DSurface9* surface;
		checked_cast<D3D9Texture2D*>(&texture_1d_2d)->D3DTexture2D()->GetSurfaceLevel(level, &surface);
		surface_ = MakeCOMPtr(surface);

		width_ = texture_1d_2d.Width(level);
		height_ = texture_1d_2d.Height(level);
		pf_ = texture_1d_2d.Format();

		if (multi_sample > 16)
		{
			multi_sample_ = D3DMULTISAMPLE_16_SAMPLES;
		}
		else
		{
			multi_sample_ = D3DMULTISAMPLE_TYPE(multi_sample);
		}
	}

	void D3D9DepthStencilRenderView::OnAttached(FrameBuffer& /*fb*/, uint32_t att)
	{
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);
	}

	void D3D9DepthStencilRenderView::OnDetached(FrameBuffer& /*fb*/, uint32_t att)
	{
		UNREF_PARAM(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);
	}

	ID3D9SurfacePtr D3D9DepthStencilRenderView::CreateSurface()
	{
		IDirect3DSurface9* surface = NULL;

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();
		TIF(d3d_device->CreateDepthStencilSurface(width_, height_, D3D9Mapping::MappingFormat(pf_),
			multi_sample_, 0, false, &surface, NULL));

		return MakeCOMPtr(surface);
	}

	void D3D9DepthStencilRenderView::DoOnLostDevice()
	{
		surface_.reset();
	}

	void D3D9DepthStencilRenderView::DoOnResetDevice()
	{
		surface_ = this->CreateSurface();
	}
}
