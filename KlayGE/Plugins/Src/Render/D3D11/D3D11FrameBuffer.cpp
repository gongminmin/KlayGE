// D3D11FrameBuffer.cpp
// KlayGE D3D11帧缓存类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
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

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>

namespace KlayGE
{
	D3D11FrameBuffer::D3D11FrameBuffer()
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

	D3D11FrameBuffer::~D3D11FrameBuffer()
	{
	}

	ID3D11RenderTargetViewPtr D3D11FrameBuffer::D3DRTView(uint32_t n) const
	{
		if (n < clr_views_.size())
		{
			if (clr_views_[n])
			{
				D3D11RenderTargetRenderView const & d3d_view(*checked_pointer_cast<D3D11RenderTargetRenderView>(clr_views_[n]));
				return d3d_view.D3DRenderTargetView();
			}
			else
			{
				return ID3D11RenderTargetViewPtr();
			}
		}
		else
		{
			return ID3D11RenderTargetViewPtr();
		}
	}

	ID3D11DepthStencilViewPtr D3D11FrameBuffer::D3DDSView() const
	{
		if (rs_view_)
		{
			D3D11DepthStencilRenderView const & d3d_view(*checked_pointer_cast<D3D11DepthStencilRenderView>(rs_view_));
			return d3d_view.D3DDepthStencilView();
		}
		else
		{
			return ID3D11DepthStencilViewPtr();
		}
	}

	std::wstring const & D3D11FrameBuffer::Description() const
	{
		static std::wstring const desc(L"Direct3D11 Framebuffer");
		return desc;
	}

	void D3D11FrameBuffer::OnBind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		std::vector<ID3D11RenderTargetView*> rt_view(clr_views_.size());
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

		for (uint32_t i = 0; i < rt_view.size(); ++ i)
		{
			if (rt_view[i] != NULL)
			{
				re.DetachTextureByRTV(rt_view[i]);
			}
		}

		d3d_imm_ctx->OMSetRenderTargets(static_cast<UINT>(rt_view.size()), &rt_view[0], this->D3DDSView().get());
		
		d3d_viewport_.Width = static_cast<float>(viewport_.width);
		d3d_viewport_.Height = static_cast<float>(viewport_.height);
		d3d_imm_ctx->RSSetViewports(1, &d3d_viewport_);
	}

	void D3D11FrameBuffer::OnUnbind()
	{
	}

	void D3D11FrameBuffer::Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		if (flags & CBM_Color)
		{
			for (uint32_t i = 0; i < clr_views_.size(); ++ i)
			{
				ID3D11RenderTargetViewPtr rt_view = this->D3DRTView(i);
				if (rt_view)
				{
					d3d_imm_ctx->ClearRenderTargetView(rt_view.get(), &clr.r());
				}
			}
		}
		if ((flags & CBM_Depth) && (flags & CBM_Stencil))
		{
			ID3D11DepthStencilViewPtr const & ds_view = this->D3DDSView();
			if (ds_view)
			{
				d3d_imm_ctx->ClearDepthStencilView(ds_view.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, static_cast<uint8_t>(stencil));
			}
		}
		else
		{
			if (flags & CBM_Depth)
			{
				ID3D11DepthStencilViewPtr const & ds_view = this->D3DDSView();
				if (ds_view)
				{
					d3d_imm_ctx->ClearDepthStencilView(ds_view.get(), D3D11_CLEAR_DEPTH, depth, 0);
				}
			}

			if (flags & CBM_Stencil)
			{
				ID3D11DepthStencilViewPtr const & ds_view = this->D3DDSView();
				if (ds_view)
				{
					d3d_imm_ctx->ClearDepthStencilView(ds_view.get(), D3D11_CLEAR_STENCIL, 1, static_cast<uint8_t>(stencil));
				}
			}
		}
	}
}
