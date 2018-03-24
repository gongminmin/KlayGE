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
		if (ds_view_)
		{
			return checked_cast<D3D11DepthStencilRenderView*>(ds_view_.get())->D3DDepthStencilView();
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
		if (views_dirty_)
		{
			d3d_rt_src_.clear();
			d3d_rt_first_subres_.clear();
			d3d_rt_num_subres_.clear();
			d3d_rt_view_.resize(clr_views_.size());
			for (uint32_t i = 0; i < clr_views_.size(); ++ i)
			{
				if (clr_views_[i])
				{
					D3D11RenderTargetRenderView* p = checked_cast<D3D11RenderTargetRenderView*>(clr_views_[i].get());
					d3d_rt_src_.push_back(p->RTSrc());
					d3d_rt_first_subres_.push_back(p->RTFirstSubRes());
					d3d_rt_num_subres_.push_back(p->RTNumSubRes());
					d3d_rt_view_[i] = this->D3DRTView(i);
				}
				else
				{
					d3d_rt_view_[i] = nullptr;
				}
			}
			if (ds_view_)
			{
				D3D11DepthStencilRenderView* p = checked_cast<D3D11DepthStencilRenderView*>(ds_view_.get());
				d3d_rt_src_.push_back(p->RTSrc());
				d3d_rt_first_subres_.push_back(p->RTFirstSubRes());
				d3d_rt_num_subres_.push_back(p->RTNumSubRes());
				d3d_ds_view_ = this->D3DDSView();
			}
			else
			{
				d3d_ds_view_ = nullptr;
			}

			d3d_ua_view_.resize(ua_views_.size());
			d3d_ua_init_count_.resize(ua_views_.size());
			for (uint32_t i = 0; i < ua_views_.size(); ++ i)
			{
				if (ua_views_[i])
				{
					D3D11UnorderedAccessView* p = checked_cast<D3D11UnorderedAccessView*>(ua_views_[i].get());
					d3d_rt_src_.push_back(p->UASrc());
					d3d_rt_first_subres_.push_back(p->UAFirstSubRes());
					d3d_rt_num_subres_.push_back(p->UANumSubRes());
					d3d_ua_view_[i] = this->D3DUAView(i);
					d3d_ua_init_count_[i] = ua_views_[i]->InitCount();
				}
				else
				{
					d3d_ua_view_[i] = nullptr;
					d3d_ua_init_count_[i] = 0;
				}
			}

			d3d_viewport_.TopLeftX = static_cast<float>(viewport_->left);
			d3d_viewport_.TopLeftY = static_cast<float>(viewport_->top);
			d3d_viewport_.Width = static_cast<float>(viewport_->width);
			d3d_viewport_.Height = static_cast<float>(viewport_->height);

			views_dirty_ = false;
		}

		auto& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		for (size_t i = 0; i < d3d_rt_src_.size(); ++ i)
		{
			re.DetachSRV(d3d_rt_src_[i], d3d_rt_first_subres_[i], d3d_rt_num_subres_[i]);
		}

		if (ua_views_.empty())
		{
			re.OMSetRenderTargets(static_cast<UINT>(d3d_rt_view_.size()), &d3d_rt_view_[0], d3d_ds_view_);
		}
		else
		{
			ID3D11RenderTargetView** rts = d3d_rt_view_.empty() ? nullptr : &d3d_rt_view_[0];
			re.OMSetRenderTargetsAndUnorderedAccessViews(static_cast<UINT>(d3d_rt_view_.size()), rts, d3d_ds_view_,
				static_cast<UINT>(d3d_rt_view_.size()), static_cast<UINT>(d3d_ua_view_.size()), &d3d_ua_view_[0], &d3d_ua_init_count_[0]);
		}

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
			if (ds_view_)
			{
				ds_view_->ClearDepthStencil(depth, stencil);
			}
		}
		else
		{
			if (flags & CBM_Depth)
			{
				if (ds_view_)
				{
					ds_view_->ClearDepth(depth);
				}
			}

			if (flags & CBM_Stencil)
			{
				if (ds_view_)
				{
					ds_view_->ClearStencil(stencil);
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
			if (ds_view_)
			{
				ds_view_->Discard();
			}
		}
	}
}
