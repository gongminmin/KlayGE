// UIScrollBar.cpp
// KlayGE 图形用户界面滚动条类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 初次建立 (2007.6.28)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	// Minimum scroll bar thumb size
	int constexpr SCROLLBAR_MINTHUMBSIZE = 8;

	// Delay and repeat period when clicking on the scroll bar arrows
	float constexpr SCROLLBAR_ARROWCLICK_DELAY = 0.33f;
	float constexpr SCROLLBAR_ARROWCLICK_REPEAT = 0.05f;

	Timer UIScrollBar::timer_;


	UIScrollBar::UIScrollBar(UIDialogPtr const & dialog)
					: UIScrollBar(UIScrollBar::Type, dialog)
	{
	}

	UIScrollBar::UIScrollBar(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						show_thumb_(true), drag_(false),
						position_(0), page_size_(1),
						start_(0), end_(1),
						arrow_(CLEAR), arrow_ts_(0)
	{
		up_button_rc_ = IRect(0, 0, 0, 0);
		down_button_rc_ = IRect(0, 0, 0, 0);
		track_rc_ = IRect(0, 0, 0, 0);
		thumb_rc_ = IRect(0, 0, 0, 0);

		UIElement Element;

		// Track
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ScrollBar, 0));
			Element.TextureColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 1);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Up Arrow
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ScrollBar, 1));
			Element.TextureColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 1);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Down Arrow
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ScrollBar, 2));
			Element.TextureColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 1);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Button
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ScrollBar, 3));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}
	}

	UIScrollBar::UIScrollBar(UIDialogPtr const & dialog, int ID, int4 const & coord_size, int nTrackStart, int nTrackEnd, int nTrackPos, int nPageSize)
					: UIScrollBar(UIScrollBar::Type, dialog)
	{
		position_ = nTrackPos;
		page_size_ = nPageSize;
		start_ = nTrackStart;
		end_ = nTrackEnd;

		// Set the ID and position
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
	}

	UIScrollBar::~UIScrollBar()
	{
	}

	void UIScrollBar::UpdateRects()
	{
		UIControl::UpdateRects();

		// Make the buttons square

		up_button_rc_ = IRect(bounding_box_.left(), bounding_box_.top(),
			bounding_box_.right(), bounding_box_.top() + bounding_box_.Width());
		down_button_rc_ = IRect(bounding_box_.left(), bounding_box_.bottom() - bounding_box_.Width(),
			bounding_box_.right(), bounding_box_.bottom());
		track_rc_ = IRect(up_button_rc_.left(), up_button_rc_.bottom(),
			down_button_rc_.right(), down_button_rc_.top());
		thumb_rc_.left() = up_button_rc_.left();
		thumb_rc_.right() = up_button_rc_.right();

		this->UpdateThumbRect();

		bounding_box_ = up_button_rc_ | down_button_rc_ | track_rc_;
		if (end_ - start_ > page_size_)
		{
			bounding_box_ |= thumb_rc_;
		}
	}

	// Compute the dimension of the scroll thumb
	void UIScrollBar::UpdateThumbRect()
	{
		if (end_ - start_ > page_size_)
		{
			int nThumbHeight = std::max(static_cast<int>(track_rc_.Height() * page_size_ / (end_ - start_)), SCROLLBAR_MINTHUMBSIZE);
			size_t nMaxPosition = end_ - start_ - page_size_;
			thumb_rc_.top() = track_rc_.top() + static_cast<int>((position_ - start_) * (track_rc_.Height() - nThumbHeight) / nMaxPosition);
			thumb_rc_.bottom() = thumb_rc_.top() + nThumbHeight;
			show_thumb_ = true;
		}
		else
		{
			// No content to scroll
			thumb_rc_.bottom() = thumb_rc_.top();
			show_thumb_ = false;
		}
	}

	// Scroll() scrolls by nDelta items.  A positive value scrolls down, while a negative
	// value scrolls up.
	void UIScrollBar::Scroll(int nDelta)
	{
		// Perform scroll
		int new_pos = static_cast<int>(position_) + nDelta;
		position_ = MathLib::clamp(new_pos, 0, static_cast<int>(end_) - 1);

		// Cap position
		this->Cap();

		// Update thumb position
		this->UpdateThumbRect();
	}

	void UIScrollBar::ShowItem(size_t nIndex)
	{
		// Cap the index

		if (nIndex >= end_)
		{
			nIndex = end_ - 1;
		}

		// Adjust position

		if (position_ > nIndex)
		{
			position_ = nIndex;
		}
		else
		{
			if (position_ + page_size_ <= nIndex)
			{
				position_ = nIndex - page_size_ + 1;
			}
		}

		this->UpdateThumbRect();
	}

	void UIScrollBar::MouseOverHandler(UIDialog const & /*sender*/, uint32_t /*buttons*/, int2 const & pt)
	{
		last_mouse_ = pt;
		is_mouse_over_ = true;

		if (drag_)
		{
			thumb_rc_.bottom() += pt.y() - thumb_offset_y_ - thumb_rc_.top();
			thumb_rc_.top() = pt.y() - thumb_offset_y_;
			if (thumb_rc_.top() < track_rc_.top())
			{
				thumb_rc_ += int2(0, track_rc_.top() - thumb_rc_.top());
			}
			else
			{
				if (thumb_rc_.bottom() > track_rc_.bottom())
				{
					thumb_rc_ += int2(0, track_rc_.bottom() - thumb_rc_.bottom());
				}
			}

			// Compute first item index based on thumb position

			size_t nMaxFirstItem = end_ - start_ - page_size_;  // Largest possible index for first item
			size_t nMaxThumb = track_rc_.Height() - thumb_rc_.Height();  // Largest possible thumb position from the top

			position_ = start_ +
				(thumb_rc_.top() - track_rc_.top() +
				nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
				nMaxFirstItem  / nMaxThumb;
		}
	}

	void UIScrollBar::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		last_mouse_ = pt;
		if (buttons & MB_Left)
		{
			if (up_button_rc_.PtInRect(pt))
			{
				if (position_ > start_)
				{
					-- position_;
				}
				this->UpdateThumbRect();
				arrow_ = CLICKED_UP;
				arrow_ts_ = timer_.current_time();
				return;
			}

			// Check for click on down button

			if (down_button_rc_.PtInRect(pt))
			{
				if (position_ + page_size_ < end_)
				{
					++ position_;
				}
				this->UpdateThumbRect();
				arrow_ = CLICKED_DOWN;
				arrow_ts_ = timer_.current_time();
				return;
			}

			// Check for click on thumb

			if (thumb_rc_.PtInRect(pt))
			{
				drag_ = true;
				thumb_offset_y_ = pt.y() - thumb_rc_.top();
				return;
			}

			// Check for click on track

			if ((thumb_rc_.left() <= pt.x()) && (thumb_rc_.right() > pt.x()))
			{
				if ((thumb_rc_.top() > pt.y()) && (track_rc_.top() <= pt.y()))
				{
					this->Scroll(-static_cast<int>(page_size_ - 1));
				}
				else
				{
					if ((thumb_rc_.bottom() <= pt.y()) && (track_rc_.bottom() > pt.y()))
					{
						this->Scroll(static_cast<int>(page_size_ - 1));
					}
				}
			}
		}
	}

	void UIScrollBar::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		last_mouse_ = pt;
		if (buttons & MB_Left)
		{
			drag_ = false;
			this->UpdateThumbRect();
			arrow_ = CLEAR;
		}
	}

	void UIScrollBar::Render()
	{
		// Check if the arrow button has been held for a while.
		// If so, update the thumb position to simulate repeated
		// scroll.
		if (arrow_ != CLEAR)
		{
			double dCurrTime = timer_.current_time();
			if (up_button_rc_.PtInRect(last_mouse_))
			{
				switch (arrow_)
				{
				case CLICKED_UP:
					if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - arrow_ts_)
					{
						this->Scroll(-1);
						arrow_ = HELD_UP;
						arrow_ts_ = dCurrTime;
					}
					break;

				case HELD_UP:
					if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - arrow_ts_)
					{
						this->Scroll(-1);
						arrow_ts_ = dCurrTime;
					}
					break;

				default:
					KFL_UNREACHABLE("Invalid arrow state");
				}
			}
			else
			{
				if (down_button_rc_.PtInRect(last_mouse_))
				{
					switch (arrow_)
					{
					case CLICKED_DOWN:
						if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - arrow_ts_)
						{
							this->Scroll(1);
							arrow_ = HELD_DOWN;
							arrow_ts_ = dCurrTime;
						}
						break;

					case HELD_DOWN:
						if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - arrow_ts_)
						{
							this->Scroll(1);
							arrow_ts_ = dCurrTime;
						}
						break;

					default:
						KFL_UNREACHABLE("Invalid arrow state");
					}
				}
			}
		}

		UI_Control_State iState = UICS_Normal;

		if (visible_)
		{
			if (!enabled_ || !show_thumb_)
			{
				iState = UICS_Disabled;
			}
			else
			{
				if (is_mouse_over_)
				{
					iState = UICS_MouseOver;
				}
				else if (has_focus_)
				{
					iState = UICS_Focus;
				}
			}
		}
		else
		{
			iState = UICS_Hidden;
		}


		auto& track_element = *elements_[0];

		track_element.TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(track_element, track_rc_);

		auto& up_arrow_element = *elements_[1];

		up_arrow_element.TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(up_arrow_element, up_button_rc_);

		auto& down_arrow_element = *elements_[2];

		down_arrow_element.TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(down_arrow_element, down_button_rc_);

		auto& thumb_element = *elements_[3];

		thumb_element.TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(thumb_element, thumb_rc_);
	}

	void UIScrollBar::SetTrackRange(size_t nStart, size_t nEnd)
	{
		start_ = nStart;
		end_ = nEnd;
		this->Cap();
		this->UpdateThumbRect();
	}

	void UIScrollBar::Cap()  // Clips position at boundaries. Ensures it stays within legal range.
	{
		if ((position_ < start_) || (end_ - start_ <= page_size_))
		{
			position_ = start_;
		}
		else
		{
			if (position_ + page_size_ > end_)
			{
				position_ = end_ - page_size_;
			}
		}
	}
}
