// UICheckBox.cpp
// KlayGE 图形用户界面复选框类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 初次建立 (2007.6.27)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/Font.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UICheckBox::UICheckBox(UIDialogPtr const & dialog)
					: UICheckBox(UICheckBox::Type, dialog)
	{
	}

	UICheckBox::UICheckBox(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						checked_(false), pressed_(false)
	{
		hotkey_ = 0;

		UIElement Element;

		// Box
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_CheckBox, 0));
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Middle);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 200.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(1, 1, 1, 1);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Check
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_CheckBox, 1));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}
	}

	UICheckBox::UICheckBox(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, bool bChecked, uint8_t hotkey, bool bIsDefault)
					: UICheckBox(dialog)
	{
		checked_ = bChecked;
		this->SetText(strText);

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UICheckBox::KeyDownHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			pressed_ = true;
		}
	}

	void UICheckBox::KeyUpHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			if (pressed_)
			{
				pressed_ = false;
				this->SetCheckedInternal(!checked_);
			}
		}
	}

	void UICheckBox::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & /*pt*/)
	{
		if (buttons & MB_Left)
		{
			pressed_ = true;

			if (!has_focus_)
			{
				this->GetDialog()->RequestFocus(*this);
			}
		}
	}

	void UICheckBox::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			pressed_ = false;

			if (this->ContainsPoint(pt))
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
		button_rc_ = text_rc_ = IRect(x_, y_, x_ + width_, y_ + height_);
		button_rc_.right() = button_rc_.left() + button_rc_.Height();
		text_rc_.left() += static_cast<int32_t>(1.25f * button_rc_.Width());

		bounding_box_ = button_rc_ | text_rc_;
	}

	void UICheckBox::Render()
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

		auto& box_element = *elements_[0];

		box_element.TextureColor().SetState(iState);
		box_element.FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(box_element, button_rc_);
		this->GetDialog()->DrawString(text_, box_element, text_rc_);

		if (!checked_)
		{
			iState = UICS_Hidden;
		}

		auto& check_element = *elements_[1];

		check_element.TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(check_element, button_rc_);
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
