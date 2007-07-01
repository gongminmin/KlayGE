// UIButton.cpp
// KlayGE 图形用户界面按钮类 实现文件
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
#include <KlayGE/Math.hpp>
#include <KlayGE/Input.hpp>

#include <boost/bind.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIButton::UIButton(UIDialogPtr dialog)
					: UIControl(UIButton::Type, dialog),
						pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UIButton::UIButton(uint32_t type, UIDialogPtr dialog)
					: UIControl(type, dialog),
						pressed_(false)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UIButton::UIButton(UIDialogPtr dialog, int ID, std::wstring const & strText, int x, int y, int width, int height, uint8_t hotkey, bool bIsDefault)
					: UIControl(UIButton::Type, dialog),
						pressed_(false),
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

	void UIButton::InitDefaultElements()
	{
		UIElement Element;
		
		// Button
		{
			Element.SetTexture(0, Rect_T<int32_t>(0, 0, 136, 54));
			Element.SetFont(0);
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(1, 1, 1, 200.0f / 255);
			Element.FontColor().States[UICS_MouseOver] = Color(0, 0, 0, 1.0f);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		// Fill layer
		{
			Element.SetTexture(0, Rect_T<int32_t>(136, 0, 252, 54), Color(1, 1, 1, 0));
			Element.TextureColor().States[UICS_MouseOver] = Color(1, 1, 1, 160.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(0, 0, 0, 60.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 30.0f / 255);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		key_down_event_.connect(boost::bind(&UIButton::KeyDownHandler, this, _1, _2));
		key_up_event_.connect(boost::bind(&UIButton::KeyUpHandler, this, _1, _2));

		mouse_down_event_.connect(boost::bind(&UIButton::MouseDownHandler, this, _1, _2));
		mouse_up_event_.connect(boost::bind(&UIButton::MouseUpHandler, this, _1, _2));
	}

	void UIButton::KeyDownHandler(UIDialog const & /*sender*/, KeyEventArg const & arg)
	{
		if (KS_Space == arg.key)
		{
			pressed_ = true;
		}
	}

	void UIButton::KeyUpHandler(UIDialog const & /*sender*/, KeyEventArg const & arg)
	{
		if (KS_Space == arg.key)
		{
			if (pressed_)
			{
				pressed_ = false;
				this->OnClickedEvent()(*this);
			}
		}
	}

	void UIButton::MouseDownHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
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

	void UIButton::MouseUpHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		if (arg.buttons & MB_Left)
		{
			pressed_ = false;

			if (!this->GetDialog()->IsKeyboardInputEnabled())
			{
				this->GetDialog()->ClearFocus();
			}

			if (this->ContainsPoint(arg.location))
			{
				this->OnClickedEvent()(*this);
			}
		}
	}

	void UIButton::Render()
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

		// Background fill layer
		//TODO: remove magic numbers
		UIElementPtr pElement = elements_[0];

		Rect_T<int32_t> rcWindow = bounding_box_ + Vector_T<int32_t, 2>(nOffsetX, nOffsetY);

		// Blend current color
		pElement->TextureColor().SetState(iState);
		pElement->FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(*pElement, rcWindow);
		this->GetDialog()->DrawText(text_, *pElement, rcWindow);

		// Main button
		pElement = elements_[1];


		// Blend current color
		pElement->TextureColor().SetState(iState);
		pElement->FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(*pElement, rcWindow);
		this->GetDialog()->DrawText(text_, *pElement, rcWindow);
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
