// UISlider.cpp
// KlayGE 图形用户界面滑动条类 实现文件
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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Input.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UISlider::UISlider(UIDialogPtr const & dialog)
					: UISlider(UISlider::Type, dialog)
	{
	}

	UISlider::UISlider(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						value_(50), min_(0), max_(100),
						pressed_(false)
	{
		UIElement Element;

		// Track
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_Slider, 0));
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 200.0f / 255);
			Element.TextureColor().States[UICS_Disabled] = Color(1, 1, 1, 70.0f / 255);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Button
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_Slider, 1));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}
	}

	UISlider::UISlider(UIDialogPtr const & dialog, int ID, int4 const & coord_size, int min, int max, int value, bool bIsDefault)
					: UISlider(dialog)
	{
		value_ = value;
		min_ = min;
		max_ = max;

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetIsDefault(bIsDefault);

		this->UpdateRects();
	}

	void UISlider::KeyDownHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		switch (key & 0xFF)
		{
		case KS_Home:
			this->SetValueInternal(min_);
			break;

		case KS_End:
			this->SetValueInternal(max_);
			break;

		case KS_LeftArrow:
		case KS_DownArrow:
			this->SetValueInternal(value_ - 1);
			break;

		case KS_RightArrow:
		case KS_UpArrow:
			this->SetValueInternal(value_ + 1);
			break;

		case KS_PageDown:
			this->SetValueInternal(value_ - (10 > (max_ - min_) / 10 ? 10 : (max_ - min_) / 10));
			break;

		case KS_PageUp:
			this->SetValueInternal(value_ + (10 > (max_ - min_) / 10 ? 10 : (max_ - min_) / 10));
			break;
		}
	}

	void UISlider::MouseOverHandler(UIDialog const & /*sender*/, uint32_t /*buttons*/, int2 const & pt)
	{
		if (pressed_)
		{
			this->SetValueInternal(ValueFromPos(x_ + pt.x() + drag_offset_));
		}
	}

	void UISlider::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			if (button_rc_.PtInRect(pt))
			{
				// Pressed while inside the control
				pressed_ = true;

				drag_x_ = pt.x();
				//m_nDragY = arg.location.y();
				drag_offset_ = button_x_ - drag_x_;

				//m_nDragValue = value_;

				if (!has_focus_)
				{
					this->GetDialog()->RequestFocus(*this);
				}

				return;
			}

			if (slider_rc_.PtInRect(pt))
			{
				drag_x_ = pt.x();
				drag_offset_ = 0;
				pressed_ = true;

				if (!has_focus_)
				{
					this->GetDialog()->RequestFocus(*this);
				}

				if (pt.x() > button_x_ + x_)
				{
					this->SetValueInternal(value_ + 1);
					return;
				}

				if (pt.x() < button_x_ + x_)
				{
					this->SetValueInternal(value_ - 1);
					return;
				}
			}
		}
	}

	void UISlider::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & /*pt*/)
	{
		if (buttons & MB_Left)
		{
			if (pressed_)
			{
				pressed_ = false;
				this->OnValueChangedEvent()(*this);
			}
		}
	}

	void UISlider::MouseWheelHandler(UIDialog const & /*sender*/, uint32_t /*buttons*/, int2 const & /*pt*/, int32_t z_delta)
	{
		this->SetValueInternal(value_ - z_delta);
	}

	void UISlider::UpdateRects()
	{
		UIControl::UpdateRects();

		slider_rc_ = IRect(x_, y_, x_ + width_, y_ + height_);

		button_rc_ = slider_rc_;
		button_rc_.right() = button_rc_.left() + button_rc_.Height();
		button_rc_ += int2(-button_rc_.Width() / 2, 0);

		button_x_ = static_cast<int>((value_ - min_) * static_cast<float>(slider_rc_.Width()) / (max_ - min_));
		button_rc_ += int2(button_x_, 0);

		bounding_box_ = button_rc_ | slider_rc_;
	}

	int UISlider::ValueFromPos(int x)
	{
		float fValuePerPixel = static_cast<float>(max_ - min_) / slider_rc_.Width();
		return static_cast<int>(0.5f + min_ + fValuePerPixel * (x - slider_rc_.left())) ;
	}

	void UISlider::SetRange(int nMin, int nMax)
	{
		min_ = nMin;
		max_ = nMax;

		this->SetValueInternal(value_);
	}

	void UISlider::SetValueInternal(int nValue)
	{
		// Clamp to range
		nValue = std::max(min_, nValue);
		nValue = std::min(max_, nValue);

		value_ = nValue;
		this->UpdateRects();

		this->OnValueChangedEvent()(*this);
	}

	void UISlider::Render()
	{
		UI_Control_State iState = UICS_Normal;

		if (visible_)
		{
			if (enabled_)
			{
				if (pressed_)
				{
					iState = UICS_Pressed;
				}
				else if (is_mouse_over_)
				{
					iState = UICS_MouseOver;
				}
				else if (has_focus_)
				{
					iState = UICS_Focus;
				}
			}
			else
			{
				iState = UICS_Disabled;
			}
		}
		else
		{
			iState = UICS_Hidden;
		}

		auto& track_element = *elements_[0];

		track_element.TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(track_element, slider_rc_);

		auto& button_element = *elements_[1];

		button_element.TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(button_element, button_rc_);
	}
}
