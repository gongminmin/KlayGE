// UICheckBox.cpp
// KlayGE 图形用户界面复选框类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.6.27)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Input.hpp>

#include <boost/bind.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UICheckBox::UICheckBox(UIDialogPtr dialog)
					: UIControl(UICheckBox::Type, dialog),
						checked_(false), pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UICheckBox::UICheckBox(uint32_t type, UIDialogPtr dialog)
					: UIControl(type, dialog),
						checked_(false), pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UICheckBox::UICheckBox(UIDialogPtr dialog, int ID, std::wstring const & strText, int x, int y, int width, int height, bool bChecked, uint8_t hotkey, bool bIsDefault)
					: UIControl(UICheckBox::Type, dialog),
						checked_(bChecked), pressed_(false),
						text_(strText)
	{
		this->InitDefaultElements();

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(x, y);
		this->SetSize(width, height);
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UICheckBox::InitDefaultElements()
	{
		UIElement Element;

		// Box
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_CheckBox, 0));
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Middle);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 200.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(1, 1, 1, 1);

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}

		// Check
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_CheckBox, 1));

			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}

		key_down_event_.connect(boost::bind(&UICheckBox::KeyDownHandler, this, _1, _2));
		key_up_event_.connect(boost::bind(&UICheckBox::KeyUpHandler, this, _1, _2));

		mouse_down_event_.connect(boost::bind(&UICheckBox::MouseDownHandler, this, _1, _2));
		mouse_up_event_.connect(boost::bind(&UICheckBox::MouseUpHandler, this, _1, _2));
	}

	void UICheckBox::KeyDownHandler(UIDialog const & /*sender*/, KeyEventArg const & arg)
	{
		if (KS_Space == arg.key)
		{
			pressed_ = true;
		}
	}

	void UICheckBox::KeyUpHandler(UIDialog const & /*sender*/, KeyEventArg const & arg)
	{
		if (KS_Space == arg.key)
		{
			if (pressed_)
			{
				pressed_ = false;
				this->SetCheckedInternal(!checked_);
			}
		}
	}

	void UICheckBox::MouseDownHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		if (arg.buttons & MB_Left)
		{
			pressed_ = true;

			if (!has_focus_)
			{
				this->GetDialog()->RequestFocus(*this);
			}
		}
	}

	void UICheckBox::MouseUpHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		if (arg.buttons & MB_Left)
		{
			pressed_ = false;

			if (this->ContainsPoint(arg.location))
			{
				this->SetCheckedInternal(!checked_);
			}
		}
	}

	void UICheckBox::SetCheckedInternal(bool bChecked)
	{
		checked_ = bChecked;

		this->OnChangedEvent()(*this);
	}

	void UICheckBox::UpdateRects()
	{
		button_rc_ = text_rc_ = Rect_T<int32_t>(x_, y_, x_ + width_, y_ + height_);
		button_rc_.right() = button_rc_.left() + button_rc_.Height();
		text_rc_.left() += static_cast<int32_t>(1.25f * button_rc_.Width());

		bounding_box_ = button_rc_ | text_rc_;
	}

	void UICheckBox::Render()
	{
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
				}
				else
				{
					if (is_mouse_over_)
					{
						iState = UICS_MouseOver;
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
		pElement->FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(*pElement, button_rc_);
		this->GetDialog()->DrawText(text_, *pElement, text_rc_);

		if (!checked_)
		{
			iState = UICS_Hidden;
		}

		pElement = elements_[1];

		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, button_rc_);
	}

	std::wstring const & UICheckBox::GetText() const
	{
		return text_;
	}

	void UICheckBox::SetText(std::wstring const & strText)
	{
		text_ = strText;
	}

	void UICheckBox::OnHotkey()
	{
		if (this->GetDialog()->IsKeyboardInputEnabled())
		{
			this->GetDialog()->RequestFocus(*this);
		}

		this->SetCheckedInternal(!checked_);
	}
}
