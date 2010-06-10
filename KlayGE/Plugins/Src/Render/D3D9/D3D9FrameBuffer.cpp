// D3D9FrameBuffer.hpp
// KlayGE D3D9渲染纹理类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://www.klayge.org
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
#include <KlayGE/Math.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <d3d9.h>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>

namespace KlayGE
{
	D3D9FrameBuffer::D3D9FrameBuffer()
	{
		isDepthBuffered_ = false;

		left_ = 0;
		top_ = 0;

		viewport_.left	= left_;
		viewport_.top	= top_;
	}

	D3D9FrameBuffer::~D3D9FrameBuffer()
	{
	}

	ID3D9SurfacePtr D3D9FrameBuffer::D3DRenderSurface(uint32_t n) const
	{
		if (n < clr_views_.size())
		{
			D3D9RenderView const & d3d_view(*checked_pointer_cast<D3D9RenderView>(clr_views_[n]));
			return d3d_view.D3DRenderSurface();
		}
		else
		{
			return ID3D9SurfacePtr();
		}
	}

	ID3D9SurfacePtr D3D9FrameBuffer::D3DRenderZBuffer() const
	{
		if (rs_view_)
		{
			D3D9RenderView const & d3d_view(*checked_pointer_cast<D3D9RenderView>(rs_view_));
			return d3d_view.D3DRenderSurface();
		}
		else
		{
			return ID3D9SurfacePtr();
		}
	}

	std::wstring const & D3D9FrameBuffer::Description() const
	{
		static std::wstring const desc(L"Direct3D9 Render To Texture");
		return desc;
	}

	void D3D9FrameBuffer::OnBind()
	{
		D3D9RenderEngine const & re = *checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D9DevicePtr const & d3dDevice = re.D3DDevice();

		bool srgb = false;
		for (uint32_t i = 0; i < clr_views_.size(); ++ i)
		{
			D3D9RenderView const & d3d_view(*checked_pointer_cast<D3D9RenderView>(clr_views_[i]));
			if (IsSRGB(d3d_view.Format()))
			{
				srgb = true;
				break;
			}
		}
		d3dDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgb);

		for (uint32_t i = 0; i < re.DeviceCaps().max_simultaneous_rts; ++ i)
		{
			TIF(d3dDevice->SetRenderTarget(i, this->D3DRenderSurface(i).get()));
		}
		TIF(d3dDevice->SetDepthStencilSurface(this->D3DRenderZBuffer().get()));
	}

	void D3D9FrameBuffer::Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		DWORD d3d_flags = 0;
		if (flags & CBM_Color)
		{
			d3d_flags |= D3DCLEAR_TARGET;
		}
		if (flags & CBM_Depth)
		{
			d3d_flags |= D3DCLEAR_ZBUFFER;
		}
		if (flags & CBM_Stencil)
		{
			d3d_flags |= D3DCLEAR_STENCIL;
		}

		RenderEngine const & re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		ID3D9DevicePtr const & d3d_device = checked_cast<D3D9RenderEngine const *>(&re)->D3DDevice();

		std::vector<IDirect3DSurface9*> old_rt(re.DeviceCaps().max_simultaneous_rts, NULL);
		if (flags & CBM_Color)
		{
			for (uint32_t i = 0; i < old_rt.size(); ++ i)
			{
				d3d_device->GetRenderTarget(i, &old_rt[i]);
			}
			for (uint32_t i = 0; i < re.DeviceCaps().max_simultaneous_rts; ++ i)
			{
				TIF(d3d_device->SetRenderTarget(i, this->D3DRenderSurface(i).get()));
			}
		}
		IDirect3DSurface9* old_ds = NULL;
		if ((flags & CBM_Depth) || (flags & CBM_Stencil))
		{
			d3d_device->GetDepthStencilSurface(&old_ds);
			if (old_ds != this->D3DRenderZBuffer().get())
			{
				TIF(d3d_device->SetDepthStencilSurface(this->D3DRenderZBuffer().get()));
			}
		}

		TIF(d3d_device->Clear(0, NULL, d3d_flags,
			D3DCOLOR_COLORVALUE(clr.r(), clr.g(), clr.b(), clr.a()), depth, stencil));

		if (flags & CBM_Color)
		{
			for (uint32_t i = 0; i < old_rt.size(); ++ i)
			{
				d3d_device->SetRenderTarget(i, old_rt[i]);
				if (old_rt[i] != NULL)
				{
					old_rt[i]->Release();
				}
			}
		}
		if ((flags & CBM_Depth) || (flags & CBM_Stencil))
		{
			if (old_ds != this->D3DRenderZBuffer().get())
			{
				TIF(d3d_device->SetDepthStencilSurface(old_ds));
			}
			if (old_ds != NULL)
			{
				old_ds->Release();
			}
		}
	}

	void D3D9FrameBuffer::DoOnLostDevice()
	{
	}

	void D3D9FrameBuffer::DoOnResetDevice()
	{
	}
}
