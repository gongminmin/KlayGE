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
#include <KFL/Util.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>

namespace KlayGE
{
	D3D11FrameBuffer::D3D11FrameBuffer()
	{
		left_ = 0;
		top_ = 0;

		viewport_->left	= left_;
		viewport_->top	= top_;

		d3d_viewport_.MinDepth = 0.0f;
		d3d_viewport_.MaxDepth = 1.0f;
	}

	D3D11FrameBuffer::~D3D11FrameBuffer()
	{
	}

	ID3D11RenderTargetView* D3D11FrameBuffer::D3DRTView(uint32_t n) const
	{
		if (n < clr_views_.size())
		{
			if (clr_views_[n])
			{
				return checked_cast<D3D11RenderTargetRenderView*>(clr_views_[n].get())->D3DRenderTargetView();
			}
		}

		return nullptr;
	}

	ID3D11DepthStencilView* D3D11FrameBuffer::D3DDSView() const
	{
		if (rs_view_)
		{
			return checked_cast<D3D11DepthStencilRenderView*>(rs_view_.get())->D3DDepthStencilView();
		}

		return nullptr;
	}

	ID3D11UnorderedAccessView* D3D11FrameBuffer::D3DUAView(uint32_t n) const
	{
		if (n < ua_views_.size())
		{
			if (ua_views_[n])
			{
				return checked_cast<D3D11UnorderedAccessView*>(ua_views_[n].get())->D3DUnorderedAccessView();
			}
		}

		return nullptr;
	}

	std::wstring const & D3D11FrameBuffer::Description() const
	{
		static std::wstring const desc(L"Direct3D11 Framebuffer");
		return desc;
	}

	void D3D11FrameBuffer::OnBind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext* d3d_imm_ctx = re.D3DDeviceImmContext();

		std::vector<void*> rt_src;
		std::vector<uint32_t> rt_first_subres;
		std::vector<uint32_t> rt_num_subres;
		std::vector<ID3D11RenderTargetView*> rt_view(clr_views_.size());
		for (uint32_t i = 0; i < clr_views_.size(); ++ i)
		{
			if (clr_views_[i])
			{
				D3D11RenderTargetRenderView* p = checked_cast<D3D11RenderTargetRenderView*>(clr_views_[i].get());
				rt_src.push_back(p->RTSrc());
				rt_first_subres.push_back(p->RTFirstSubRes());
				rt_num_subres.push_back(p->RTNumSubRes());
				rt_view[i] = this->D3DRTView(i);
			}
			else
			{
				rt_view[i] = nullptr;
			}
		}
		if (rs_view_)
		{
			D3D11DepthStencilRenderView* p = checked_cast<D3D11DepthStencilRenderView*>(rs_view_.get());
			rt_src.push_back(p->RTSrc());
			rt_first_subres.push_back(p->RTFirstSubRes());
			rt_num_subres.push_back(p->RTNumSubRes());
		}
		std::vector<ID3D11UnorderedAccessView*> ua_view(ua_views_.size());
		std::vector<UINT> ua_init_count(ua_views_.size());
		for (uint32_t i = 0; i < ua_views_.size(); ++ i)
		{
			if (ua_views_[i])
			{
				D3D11UnorderedAccessView* p = checked_cast<D3D11UnorderedAccessView*>(ua_views_[i].get());
				rt_src.push_back(p->UASrc());
				rt_first_subres.push_back(p->UAFirstSubRes());
				rt_num_subres.push_back(p->UANumSubRes());
				ua_view[i] = this->D3DUAView(i);
				ua_init_count[i] = ua_views_[i]->InitCount();
			}
			else
			{
				ua_view[i] = nullptr;
				ua_init_count[i] = 0;
			}
		}

		for (size_t i = 0; i < rt_src.size(); ++ i)
		{
			re.DetachSRV(rt_src[i], rt_first_subres[i], rt_num_subres[i]);
		}

		if (ua_views_.empty())
		{
			d3d_imm_ctx->OMSetRenderTargets(static_cast<UINT>(rt_view.size()), &rt_view[0], this->D3DDSView());
		}
		else
		{
			ID3D11RenderTargetView** rts = rt_view.empty() ? nullptr : &rt_view[0];
			d3d_imm_ctx->OMSetRenderTargetsAndUnorderedAccessViews(static_cast<UINT>(rt_view.size()), rts, this->D3DDSView(),
				0, static_cast<UINT>(ua_view.size()), &ua_view[0], &ua_init_count[0]);
		}
	
		d3d_viewport_.TopLeftX = static_cast<float>(viewport_->left);
		d3d_viewport_.TopLeftY = static_cast<float>(viewport_->top);
		d3d_viewport_.Width = static_cast<float>(viewport_->width);
		d3d_viewport_.Height = static_cast<float>(viewport_->height);
		re.RSSetViewports(1, &d3d_viewport_);
	}

	void D3D11FrameBuffer::OnUnbind()
	{
	}

	void D3D11FrameBuffer::Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		if (flags & CBM_Color)
		{
			for (uint32_t i = 0; i < clr_views_.size(); ++ i)
			{
				if (clr_views_[i])
				{
					clr_views_[i]->ClearColor(clr);
				}
			}
		}
		if ((flags & CBM_Depth) && (flags & CBM_Stencil))
		{
			if (rs_view_)
			{
				rs_view_->ClearDepthStencil(depth, stencil);
			}
		}
		else
		{
			if (flags & CBM_Depth)
			{
				if (rs_view_)
				{
					rs_view_->ClearDepth(depth);
				}
			}

			if (flags & CBM_Stencil)
			{
				if (rs_view_)
				{
					rs_view_->ClearStencil(stencil);
				}
			}
		}
	}

	void D3D11FrameBuffer::Discard(uint32_t flags)
	{
		if (flags & CBM_Color)
		{
			for (uint32_t i = 0; i < clr_views_.size(); ++ i)
			{
				if (clr_views_[i])
				{
					clr_views_[i]->Discard();
				}
			}
		}
		if ((flags & CBM_Depth) || (flags & CBM_Stencil))
		{
			if (rs_view_)
			{
				rs_view_->Discard();
			}
		}
	}
}
