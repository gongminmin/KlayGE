// D3D10FrameBuffer.cpp
// KlayGE D3D10帧缓存类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>

#include <KlayGE/D3D10/D3D10RenderEngine.hpp>
#include <KlayGE/D3D10/D3D10RenderView.hpp>
#include <KlayGE/D3D10/D3D10FrameBuffer.hpp>

namespace KlayGE
{
	D3D10FrameBuffer::D3D10FrameBuffer()
	{
		isDepthBuffered_ = false;

		left_ = 0;
		top_ = 0;

		viewport_.left	= left_;
		viewport_.top	= top_;

		d3d_viewport_.MinDepth = 0.0f;
		d3d_viewport_.MaxDepth = 1.0f;
		d3d_viewport_.TopLeftX = 0;
		d3d_viewport_.TopLeftY = 0;
	}

	D3D10FrameBuffer::~D3D10FrameBuffer()
	{
	}

	ID3D10RenderTargetViewPtr D3D10FrameBuffer::D3DRTView(uint32_t n) const
	{
		if (n < clr_views_.size())
		{
			if (clr_views_[n])
			{
				D3D10RenderTargetRenderView const & d3d_view(*checked_pointer_cast<D3D10RenderTargetRenderView>(clr_views_[n]));
				return d3d_view.D3DRenderTargetView();
			}
			else
			{
				return ID3D10RenderTargetViewPtr();
			}
		}
		else
		{
			return ID3D10RenderTargetViewPtr();
		}
	}

	ID3D10DepthStencilViewPtr D3D10FrameBuffer::D3DDSView() const
	{
		if (rs_view_)
		{
			D3D10DepthStencilRenderView const & d3d_view(*checked_pointer_cast<D3D10DepthStencilRenderView>(rs_view_));
			return d3d_view.D3DDepthStencilView();
		}
		else
		{
			return ID3D10DepthStencilViewPtr();
		}
	}

	std::wstring const & D3D10FrameBuffer::Description() const
	{
		static std::wstring const desc(L"Direct3D10 Framebuffer");
		return desc;
	}

	void D3D10FrameBuffer::OnBind()
	{
		D3D10RenderEngine const & re = *checked_cast<D3D10RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D10DevicePtr const & d3d_device = re.D3DDevice();

		std::vector<ID3D10RenderTargetView*> rt_view(clr_views_.size());
		for (uint32_t i = 0; i < clr_views_.size(); ++ i)
		{
			if (clr_views_[i])
			{
				rt_view[i] = this->D3DRTView(i).get();
			}
			else
			{
				rt_view[i] = NULL;
			}
		}

		d3d_device->OMSetRenderTargets(static_cast<UINT>(rt_view.size()), &rt_view[0], this->D3DDSView().get());

		d3d_viewport_.Width = viewport_.width;
		d3d_viewport_.Height = viewport_.height;
		d3d_device->RSSetViewports(1, &d3d_viewport_);
	}

	void D3D10FrameBuffer::OnUnbind()
	{
		D3D10RenderEngine const & re = *checked_cast<D3D10RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D10DevicePtr const & d3d_device = re.D3DDevice();

		d3d_device->OMSetRenderTargets(0, NULL, NULL);
	}

	void D3D10FrameBuffer::Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		D3D10RenderEngine const & re = *checked_cast<D3D10RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D10DevicePtr const & d3d_device = re.D3DDevice();

		if (flags & CBM_Color)
		{
			for (uint32_t i = 0; i < clr_views_.size(); ++ i)
			{
				ID3D10RenderTargetViewPtr rt_view = this->D3DRTView(i);
				if (rt_view)
				{
					d3d_device->ClearRenderTargetView(rt_view.get(), &clr.r());
				}
			}
		}
		if ((flags & CBM_Depth) && (flags & CBM_Stencil))
		{
			ID3D10DepthStencilViewPtr const & ds_view = this->D3DDSView();
			if (ds_view)
			{
				d3d_device->ClearDepthStencilView(ds_view.get(), D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, depth, static_cast<uint8_t>(stencil));
			}
		}
		else
		{
			if (flags & CBM_Depth)
			{
				ID3D10DepthStencilViewPtr const & ds_view = this->D3DDSView();
				if (ds_view)
				{
					d3d_device->ClearDepthStencilView(ds_view.get(), D3D10_CLEAR_DEPTH, depth, 0);
				}
			}

			if (flags & CBM_Stencil)
			{
				ID3D10DepthStencilViewPtr const & ds_view = this->D3DDSView();
				if (ds_view)
				{
					d3d_device->ClearDepthStencilView(ds_view.get(), D3D10_CLEAR_STENCIL, 1, static_cast<uint8_t>(stencil));
				}
			}
		}
	}
}
