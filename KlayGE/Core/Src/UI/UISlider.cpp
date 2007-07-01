// UISlider.cpp
// KlayGE 图形用户界面滑动条类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.6.28)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Input.hpp>

#include <boost/bind.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UISlider::UISlider(UIDialogPtr dialog)
					: UIControl(UISlider::Type, dialog),
						min_(0), max_(100), value_(50),
						pressed_(false)
	{
	}

	UISlider::UISlider(uint32_t type, UIDialogPtr dialog)
					: UIControl(type, dialog),
						min_(0), max_(100), value_(50),
						pressed_(false)
	{
		this->InitDefaultElements();
	}

	UISlider::UISlider(UIDialogPtr dialog, int ID, int x, int y, int width, int height, int min, int max, int value, bool bIsDefault)
					: UIControl(UISlider::Type, dialog),
						min_(min), max_(max), value_(value),
						pressed_(false)
	{
		this->InitDefaultElements();

		// Set the ID and list index
		this->SetID(ID); 
		this->SetLocation(x, y);
		this->SetSize(width, height);
		this->SetIsDefault(bIsDefault);

		this->UpdateRects();
	}

	void UISlider::InitDefaultElements()
	{
		UIElement Element;
		
		// Track
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_Slider, 0));
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 200.0f / 255);
			Element.TextureColor().States[UICS_Disabled] = Color(1, 1, 1, 70.0f / 255);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		// Button
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_Slider, 1));

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		key_down_event_.connect(boost::bind(&UISlider::KeyDownHandler, this, _1, _2));

		mouse_over_event_.connect(boost::bind(&UISlider::MouseOverHandler, this, _1, _2));
		mouse_down_event_.connect(boost::bind(&UISlider::MouseDownHandler, this, _1, _2));
		mouse_up_event_.connect(boost::bind(&UISlider::MouseUpHandler, this, _1, _2));
		mouse_wheel_event_.connect(boost::bind(&UISlider::MouseWheelHandler, this, _1, _2));
	}

	void UISlider::KeyDownHandler(UIDialog const & /*sender*/, KeyEventArg const & arg)
	{
		switch (arg.key)
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

	void UISlider::MouseOverHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		if (pressed_)
		{
			this->SetValueInternal(ValueFromPos(x_ + arg.location.x() + drag_offset_));
		}
	}

	void UISlider::MouseDownHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		if (arg.buttons & MB_Left)
		{
			if (button_rc_.PtInRect(arg.location))
			{
				// Pressed while inside the control
				pressed_ = true;

				drag_x_ = arg.location.x();
				//m_nDragY = arg.location.y();
				drag_offset_ = button_x_ - drag_x_;

				//m_nDragValue = value_;

				if (!has_focus_)
				{
					this->GetDialog()->RequestFocus(*this);
				}

				return;
			}

			if (slider_rc_.PtInRect(arg.location))
			{
				drag_x_ = arg.location.x();
				drag_offset_ = 0;               
				pressed_ = true;

				if (!has_focus_)
				{
					this->GetDialog()->RequestFocus(*this);
				}

				if (arg.location.x() > button_x_ + x_)
				{
					this->SetValueInternal(value_ + 1);
					return;
				}

				if (arg.location.x() < button_x_ + x_)
				{
					this->SetValueInternal(value_ - 1);
					return;
				}
			}
		}
	}

	void UISlider::MouseUpHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		if (arg.buttons & MB_Left)
		{
			if (pressed_)
			{
				pressed_ = false;
				this->OnValueChangedEvent()(*this);
			}
		}
	}

	void UISlider::MouseWheelHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		this->SetValueInternal(value_ - arg.z_delta);
	}

	void UISlider::UpdateRects()
	{
		UIControl::UpdateRects();

		slider_rc_ = Rect_T<int32_t>(x_, y_, x_ + width_, y_ + height_);

		button_rc_ = slider_rc_;
		button_rc_.right() = button_rc_.left() + button_rc_.Height();
		button_rc_ += Vector_T<int32_t, 2>(-button_rc_.Width() / 2, 0);

		button_x_ = static_cast<int>((value_ - min_) * static_cast<float>(slider_rc_.Width()) / (max_ - min_));
		button_rc_ += Vector_T<int32_t, 2>(button_x_, 0);

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

		if (nValue == value_)
		{
			return;
		}

		value_ = nValue;
		this->UpdateRects();

		this->OnValueChangedEvent()(*this);
	}

	void UISlider::Render()
	{
		int nOffsetX = 0;
		int nOffsetY = 0;

		UI_Control_State iState = UICS_Normal;

		if (!visible_)
		{
			iState = UICS_Hidden;
		}
		else
		{
			if (!enabled_)
			{
				iState = UICS_Disabled;
			}
			else
			{
				if (pressed_)
				{
					iState = UICS_Pressed;

					nOffsetX = 1;
					nOffsetY = 2;
				}
				else
				{
					if (is_mouse_over_)
					{
						iState = UICS_MouseOver;

						nOffsetX = -1;
						nOffsetY = -2;
					}
					else
					{
						if (has_focus_)
						{
							iState = UICS_Focus;
						}
					}
				}
			}
		}

		UIElementPtr pElement = elements_[0];

		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, slider_rc_);

		pElement = elements_[1];

		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, button_rc_);
	}
}
