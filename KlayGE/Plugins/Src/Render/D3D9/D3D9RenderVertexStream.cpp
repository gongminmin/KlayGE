// D3D9RenderVertexStream.hpp
// KlayGE D3D9渲染到顶点流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 初次建立 (2005.7.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#define NOMINMAX
#include <d3d9.h>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>
#include <KlayGE/D3D9/D3D9RenderVertexStream.hpp>

namespace KlayGE
{
	D3D9RenderVertexStream::D3D9RenderVertexStream(uint32_t width, uint32_t height)
	{
		left_ = 0;
		top_ = 0;
		width_ = width;
		height_ = height;

		viewport_.left		= left_;
		viewport_.top		= top_;
		viewport_.width		= width_;
		viewport_.height	= height_;
	}

	void D3D9RenderVertexStream::Attach(GraphicsBufferPtr vs)
	{
		BOOST_ASSERT(vs->Usage() != BU_Static);

		RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		d3d_device_ = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

		vs_ = vs;

		isDepthBuffered_ = false;
		fmt_ = D3DFMT_A32B32G32R32F;
		colorDepth_ = 128;

		render_surf_ = this->CreateSurface(D3DPOOL_DEFAULT);
	}

	void D3D9RenderVertexStream::Detach()
	{
		this->CopyToVertexStream();

		vs_.reset();
		render_surf_.reset();
	}

	boost::shared_ptr<IDirect3DSurface9> D3D9RenderVertexStream::D3DRenderSurface() const
	{
		return render_surf_;
	}
	
	boost::shared_ptr<IDirect3DSurface9> D3D9RenderVertexStream::D3DRenderZBuffer() const
	{
		return boost::shared_ptr<IDirect3DSurface9>();
	}

	void D3D9RenderVertexStream::CustomAttribute(std::string const & name, void* pData)
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
			*pSurf = NULL;

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

	void D3D9RenderVertexStream::DoOnLostDevice()
	{
		if (vs_)
		{
			IDirect3DSurface9Ptr sys_mem_surf = this->CreateSurface(D3DPOOL_SYSTEMMEM);

			TIF(d3d_device_->GetRenderTargetData(render_surf_.get(), sys_mem_surf.get()));
			render_surf_ = sys_mem_surf;
		}
	}
	
	void D3D9RenderVertexStream::DoOnResetDevice()
	{
		if (vs_)
		{
			RenderEngine const & render_eng = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			d3d_device_ = checked_cast<D3D9RenderEngine const *>(&render_eng)->D3DDevice();

			IDirect3DSurface9Ptr default_surf = this->CreateSurface(D3DPOOL_DEFAULT);

			TIF(D3DXLoadSurfaceFromSurface(render_surf_.get(), NULL, NULL,
				default_surf.get(), NULL, NULL, D3DX_FILTER_NONE, 0));
			render_surf_ = default_surf;
		}
	}

	D3D9RenderVertexStream::IDirect3DSurface9Ptr D3D9RenderVertexStream::CreateSurface(D3DPOOL pool)
	{
		IDirect3DSurface9* surface = NULL;

		if (D3DPOOL_SYSTEMMEM == pool)
		{
			TIF(d3d_device_->CreateOffscreenPlainSurface(width_, height_, fmt_, pool, &surface, NULL));
		}
		else
		{
			TIF(d3d_device_->CreateRenderTarget(width_, height_, fmt_, D3DMULTISAMPLE_NONE, 0, true, &surface, NULL));
		}

		return MakeCOMPtr(surface);
	}

	void D3D9RenderVertexStream::CopyToVertexStream()
	{
		D3DLOCKED_RECT locked_rect;
		TIF(render_surf_->LockRect(&locked_rect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		GraphicsBuffer::Mapper mapper(*vs_, BA_Write_Only);

		const float* src = static_cast<float*>(locked_rect.pBits);
		float* dst = mapper.Pointer<float>();
		for (uint32_t i = 0; i < height_; ++ i)
		{
			std::copy(src, src + width_ * 4, dst);

			src += locked_rect.Pitch / sizeof(float);
			dst += width_ * 4;
		}

		render_surf_->UnlockRect();
	}
}
