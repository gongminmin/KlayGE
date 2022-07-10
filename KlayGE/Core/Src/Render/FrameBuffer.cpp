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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderView.hpp>

#include <system_error>

#include <KlayGE/FrameBuffer.hpp>

namespace KlayGE
{
	FrameBuffer::FrameBuffer()
					: left_(0), top_(0), width_(0), height_(0),
						viewport_(MakeSharedPtr<KlayGE::Viewport>())
	{
		viewport_->Left(left_);
		viewport_->Top(top_);
		viewport_->Width(width_);
		viewport_->Height(height_);
	}

	FrameBuffer::~FrameBuffer() noexcept = default;

	FrameBuffer::Attachment FrameBuffer::CalcAttachment(uint32_t index)
	{
		return static_cast<Attachment>(static_cast<uint32_t>(Attachment::Color0) + index);
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
	ViewportPtr const & FrameBuffer::Viewport() const
	{
		return viewport_;
	}

	ViewportPtr& FrameBuffer::Viewport()
	{
		return viewport_;
	}

	// 设置视口
	/////////////////////////////////////////////////////////////////////////////////
	void FrameBuffer::Viewport(ViewportPtr const & viewport)
	{
		viewport_ = viewport;
	}

	void FrameBuffer::Attach(Attachment att, RenderTargetViewPtr const & view)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (static_cast<uint32_t>(att) >= re.DeviceCaps().max_simultaneous_rts)
		{
			TERRC(std::errc::function_not_supported);
		}

		uint32_t const rt_index = static_cast<uint32_t>(att);
		if ((rt_index < rt_views_.size()) && rt_views_[rt_index])
		{
			this->Detach(att);
		}

		if (rt_views_.size() < rt_index + 1)
		{
			rt_views_.resize(rt_index + 1);
		}

		rt_views_[rt_index] = view;
		size_t min_rt_index = rt_index;
		for (size_t i = 0; i < rt_index; ++ i)
		{
			if (rt_views_[i])
			{
				min_rt_index = i;
			}
		}
		if (min_rt_index == rt_index)
		{
			width_ = view->Width();
			height_ = view->Height();

			viewport_->Left(0);
			viewport_->Top(0);
			viewport_->Width(width_);
			viewport_->Height(height_);
		}

		if (view)
		{
			view->OnAttached(*this, att);
		}

		views_dirty_ = true;
	}

	void FrameBuffer::Detach(Attachment att)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (static_cast<uint32_t>(att) >= re.DeviceCaps().max_simultaneous_rts)
		{
			TERRC(std::errc::function_not_supported);
		}

		uint32_t const rt_index = static_cast<uint32_t>(att);
		if ((rt_index < rt_views_.size()) && rt_views_[rt_index])
		{
			rt_views_[rt_index]->OnDetached(*this, att);
			rt_views_[rt_index].reset();
		}

		views_dirty_ = true;
	}

	RenderTargetViewPtr const & FrameBuffer::AttachedRtv(Attachment att) const
	{
		uint32_t const rt_index = static_cast<uint32_t>(att);
		if (rt_index < rt_views_.size())
		{
			return rt_views_[rt_index];
		}
		else
		{
			static RenderTargetViewPtr null_view;
			return null_view;
		}
	}
	
	void FrameBuffer::Attach(DepthStencilViewPtr const & view)
	{
		if (ds_view_)
		{
			this->Detach();
		}

		ds_view_ = view;

		if (view)
		{
			view->OnAttached(*this);
		}

		views_dirty_ = true;
	}
	
	void FrameBuffer::Detach()
	{
		ds_view_.reset();

		views_dirty_ = true;
	}
	
	DepthStencilViewPtr const & FrameBuffer::AttachedDsv() const
	{
		return ds_view_;
	}

	void FrameBuffer::Attach(uint32_t index, UnorderedAccessViewPtr const & view)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (index >= re.DeviceCaps().max_simultaneous_uavs)
		{
			TERRC(std::errc::function_not_supported);
		}

		if ((index < ua_views_.size()) && ua_views_[index])
		{
			this->Detach(index);
		}

		if (ua_views_.size() < index + 1)
		{
			ua_views_.resize(index + 1);
		}

		ua_views_[index] = view;
		if (view)
		{
			view->OnAttached(*this, index);
		}

		views_dirty_ = true;
	}

	void FrameBuffer::Detach(uint32_t index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		if (index >= re.DeviceCaps().max_simultaneous_rts)
		{
			TERRC(std::errc::function_not_supported);
		}

		if ((index < ua_views_.size()) && ua_views_[index])
		{
			ua_views_[index]->OnDetached(*this, index);
			ua_views_[index].reset();
		}

		views_dirty_ = true;
	}

	UnorderedAccessViewPtr const & FrameBuffer::AttachedUav(uint32_t index) const
	{
		if (index < ua_views_.size())
		{
			return ua_views_[index];
		}
		else
		{
			static UnorderedAccessViewPtr null_view;
			return null_view;
		}
	}
}
