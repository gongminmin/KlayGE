// UIButton.cpp
// KlayGE 图形用户界面按钮类 实现文件
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

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIButton::UIButton(UIDialogPtr const & dialog)
					: UIButton(UIButton::Type, dialog)
	{
	}

	UIButton::UIButton(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						pressed_(false)
	{
		hotkey_ = 0;

		UIElement Element;

		// Button
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_Button, 0));
			Element.SetFont(0);
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(1, 1, 1, 200.0f / 255);
			Element.FontColor().States[UICS_MouseOver] = Color(0, 0, 0, 1.0f);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Fill layer
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_Button, 1), Color(1, 1, 1, 0));
			Element.TextureColor().States[UICS_MouseOver] = Color(1, 1, 1, 160.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(0, 0, 0, 60.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 30.0f / 255);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}
	}

	UIButton::UIButton(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, uint8_t hotkey, bool bIsDefault)
					: UIButton(dialog)
	{
		this->SetText(strText);

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UIButton::KeyDownHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			pressed_ = true;
		}
	}

	void UIButton::KeyUpHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			if (pressed_)
			{
				pressed_ = false;
				this->OnClickedEvent()(*this);
			}
		}
	}

	void UIButton::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & /*pt*/)
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

	void UIButton::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			pressed_ = false;

			if (!this->GetDialog()->IsKeyboardInputEnabled())
			{
				this->GetDialog()->ClearFocus();
			}

			if (this->ContainsPoint(pt))
			{
				this->OnClickedEvent()(*this);
			}
		}
	}

	void UIButton::Render()
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

		// TODO: remove magic numbers

		// Background fill layer
		auto& bg_element = *elements_[0];

		// Blend current color
		bg_element.TextureColor().SetState(iState);
		bg_element.FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(bg_element, bounding_box_);
		this->GetDialog()->DrawString(text_, bg_element, bounding_box_);

		// Main button
		auto& main_button_element = *elements_[1];

		// Blend current color
		main_button_element.TextureColor().SetState(iState);
		main_button_element.FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(main_button_element, bounding_box_);
		this->GetDialog()->DrawString(text_, main_button_element, bounding_box_);
	}

	std::wstring const & UIButton::GetText() const
	{
		return text_;
	}

	void UIButton::SetText(std::wstring const & strText)
	{
		text_ = strText;
	}

	void UIButton::OnHotkey()
	{
		if (this->GetDialog()->IsKeyboardInputEnabled())
		{
			this->GetDialog()->RequestFocus(*this);
		}

		this->OnClickedEvent()(*this);
	}
}
