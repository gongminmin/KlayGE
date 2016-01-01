// FrameBuffer.cpp
// KlayGE 渲染到纹理类 实现文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// 初次建立 (2005.10.29)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>

namespace KlayGE
{
	FrameBuffer::FrameBuffer()
					: left_(0), top_(0), width_(0), height_(0),
						viewport_(MakeSharedPtr<Viewport>())
	{
	}

	FrameBuffer::~FrameBuffer()
	{
	}

	// 渲染目标的左坐标
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t FrameBuffer::Left() const
	{
		return left_;
	}

	// 渲染目标的顶坐标
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t FrameBuffer::Top() const
	{
		return top_;
	}

	// 渲染目标的宽度
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t FrameBuffer::Width() const
	{
		return width_;
	}

	// 渲染目标的高度
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t FrameBuffer::Height() const
	{
		return height_;
	}

	// 获取视口
	/////////////////////////////////////////////////////////////////////////////////
	ViewportPtr const & FrameBuffer::GetViewport() const
	{
		return viewport_;
	}

	ViewportPtr& FrameBuffer::GetViewport()
	{
		return viewport_;
	}

	// 设置视口
	/////////////////////////////////////////////////////////////////////////////////
	void FrameBuffer::SetViewport(ViewportPtr const & viewport)
	{
		viewport_ = viewport;
	}

	void FrameBuffer::Attach(uint32_t att, RenderViewPtr const & view)
	{
		switch (att)
		{
		case ATT_DepthStencil:
			{
				if (rs_view_)
				{
					this->Detach(att);
				}

				rs_view_ = view;
			}
			break;

		default:
			{
				BOOST_ASSERT(att >= ATT_Color0);

				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				if (att >= static_cast<uint32_t>(ATT_Color0 + re.DeviceCaps().max_simultaneous_rts))
				{
					THR(errc::function_not_supported);
				}

				uint32_t clr_id = att - ATT_Color0;
				if ((clr_id < clr_views_.size()) && clr_views_[clr_id])
				{
					this->Detach(att);
				}

				if (clr_views_.size() < clr_id + 1)
				{
					clr_views_.resize(clr_id + 1);
				}

				clr_views_[clr_id] = view;
				size_t min_clr_index = clr_id;
				for (size_t i = 0; i < clr_id; ++ i)
				{
					if (clr_views_[i])
					{
						min_clr_index = i;
					}
				}
				if (min_clr_index == clr_id)
				{
					width_ = view->Width();
					height_ = view->Height();

					viewport_->left		= 0;
					viewport_->top		= 0;
					viewport_->width	= width_;
					viewport_->height	= height_;
				}
			}
			break;
		}

		if (view)
		{
			view->OnAttached(*this, att);
		}

		views_dirty_ = true;
	}

	void FrameBuffer::Detach(uint32_t att)
	{
		switch (att)
		{
		case ATT_DepthStencil:
			{
				rs_view_.reset();
			}
			break;

		default:
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				if (att >= static_cast<uint32_t>(ATT_Color0 + re.DeviceCaps().max_simultaneous_rts))
				{
					THR(errc::function_not_supported);
				}

				uint32_t clr_id = att - ATT_Color0;
				if ((clr_views_.size() < clr_id + 1) && clr_views_[clr_id])
				{
					clr_views_[clr_id]->OnDetached(*this, att);
					clr_views_[clr_id].reset();
				}
			}
			break;
		}

		views_dirty_ = true;
	}

	RenderViewPtr FrameBuffer::Attached(uint32_t att) const
	{
		switch (att)
		{
		case ATT_DepthStencil:
			return rs_view_;

		default:
			{
				uint32_t clr_id = att - ATT_Color0;
				if (clr_views_.size() < clr_id + 1)
				{
					return RenderViewPtr();
				}
				else
				{
					return clr_views_[clr_id];
				}
			}
		}
	}

	void FrameBuffer::AttachUAV(uint32_t att, UnorderedAccessViewPtr const & view)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (att >= static_cast<uint32_t>(re.DeviceCaps().max_simultaneous_uavs))
		{
			THR(errc::function_not_supported);
		}

		if ((att < ua_views_.size()) && ua_views_[att])
		{
			this->DetachUAV(att);
		}

		if (ua_views_.size() < att + 1)
		{
			ua_views_.resize(att + 1);
		}

		ua_views_[att] = view;
		if (view)
		{
			view->OnAttached(*this, att);
		}

		views_dirty_ = true;
	}

	void FrameBuffer::DetachUAV(uint32_t att)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (att >= static_cast<uint32_t>(ATT_Color0 + re.DeviceCaps().max_simultaneous_rts))
		{
			THR(errc::function_not_supported);
		}

		if ((ua_views_.size() < att + 1) && ua_views_[att])
		{
			ua_views_[att]->OnDetached(*this, att);
			ua_views_[att].reset();
		}

		views_dirty_ = true;
	}

	UnorderedAccessViewPtr FrameBuffer::AttachedUAV(uint32_t att) const
	{
		if (ua_views_.size() < att + 1)
		{
			return UnorderedAccessViewPtr();
		}
		else
		{
			return ua_views_[att];
		}
	}

	void FrameBuffer::OnBind()
	{
		for (uint32_t i = 0; i < clr_views_.size(); ++ i)
		{
			if (clr_views_[i])
			{
				clr_views_[i]->OnBind(*this, ATT_Color0 + i);
			}
		}
		if (rs_view_)
		{
			rs_view_->OnBind(*this, ATT_DepthStencil);
		}
		for (uint32_t i = 0; i < ua_views_.size(); ++ i)
		{
			if (ua_views_[i])
			{
				ua_views_[i]->OnBind(*this, i);
			}
		}
		views_dirty_ = false;
	}

	void FrameBuffer::OnUnbind()
	{
		for (uint32_t i = 0; i < clr_views_.size(); ++ i)
		{
			if (clr_views_[i])
			{
				clr_views_[i]->OnUnbind(*this, ATT_Color0 + i);
			}
		}
		if (rs_view_)
		{
			rs_view_->OnUnbind(*this, ATT_DepthStencil);
		}
		for (uint32_t i = 0; i < ua_views_.size(); ++ i)
		{
			if (ua_views_[i])
			{
				ua_views_[i]->OnUnbind(*this, i);
			}
		}
	}
}
