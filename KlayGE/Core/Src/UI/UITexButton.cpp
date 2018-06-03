// UITexButton.cpp
// KlayGE 图形用户界面纹理按钮类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 初次建立 (2009.4.12)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UITexButton::UITexButton(UIDialogPtr const & dialog)
					: UITexButton(UITexButton::Type, dialog)
	{
	}

	UITexButton::UITexButton(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						pressed_(false)
	{
		hotkey_ = 0;

		UIElement Element;

		// Fill layer
		for (int i = 0; i < 9; ++ i)
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_TexButton, i), Color(1, 1, 1, 0));
			Element.TextureColor().States[UICS_MouseOver] = Color(1, 1, 1, 1);
			Element.TextureColor().States[UICS_Pressed] = Color(0, 0, 0, 60.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 30.0f / 255);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Button
		{
			Element.SetFont(0);
			Element.TextureColor().States[UICS_MouseOver] = Color(1, 1, 1, 1);
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 1);
			Element.TextureColor().States[UICS_Pressed] = Color(1, 1, 1, 1);
			Element.FontColor().States[UICS_MouseOver] = Color(0, 0, 0, 1);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}
	}

	UITexButton::UITexButton(UIDialogPtr const & dialog, int ID, TexturePtr const & tex, int4 const & coord_size, uint8_t hotkey, bool bIsDefault)
					: UITexButton(dialog)
	{
		tex_index_ = UIManager::Instance().AddTexture(tex);
		{
			auto& element = *elements_.back();
			if (tex)
			{
				element.SetTexture(static_cast<uint32_t>(tex_index_), IRect(0, 0, tex->Width(0), tex->Height(0)));
			}
			else
			{
				element.SetTexture(static_cast<uint32_t>(tex_index_), IRect(0, 0, 1, 1));
			}
		}

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UITexButton::KeyDownHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		if (KS_Space == (key & 0xFF))
		{
			pressed_ = true;
		}
	}

	void UITexButton::KeyUpHandler(UIDialog const & /*sender*/, uint32_t key)
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

	void UITexButton::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & /*pt*/)
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

	void UITexButton::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
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

	void UITexButton::Render()
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
			else
			{
				iState = UICS_Disabled;
			}
		}
		else
		{
			iState = UICS_Hidden;
		}

		//TODO: remove magic numbers

		IRect rcWindow = bounding_box_;

		for (int i = 0; i < 10; ++ i)
		{
			auto& element = *elements_[i];

			// Blend current color
			element.TextureColor().SetState(iState);
			element.FontColor().SetState(iState);
		}

		// Fill layer
		this->GetDialog()->DrawSprite(*elements_[0], IRect(bounding_box_.left(), bounding_box_.top(),
			bounding_box_.left() + 5, bounding_box_.top() + 5));
		this->GetDialog()->DrawSprite(*elements_[1], IRect(bounding_box_.left() + 5, bounding_box_.top(),
			bounding_box_.right() - 5, bounding_box_.top() + 5));
		this->GetDialog()->DrawSprite(*elements_[2], IRect(bounding_box_.right() - 5, bounding_box_.top(),
			bounding_box_.right(), bounding_box_.top() + 5));

		this->GetDialog()->DrawSprite(*elements_[3], IRect(bounding_box_.left(), bounding_box_.top() + 5,
			bounding_box_.left() + 5, bounding_box_.bottom() - 5));
		this->GetDialog()->DrawSprite(*elements_[4], IRect(bounding_box_.left() + 5, bounding_box_.top() + 5,
			bounding_box_.right() - 5, bounding_box_.bottom() - 5));
		this->GetDialog()->DrawSprite(*elements_[5], IRect(bounding_box_.right() - 5, bounding_box_.top() + 5,
			bounding_box_.right(), bounding_box_.bottom() - 5));

		this->GetDialog()->DrawSprite(*elements_[6], IRect(bounding_box_.left(), bounding_box_.bottom() - 5,
			bounding_box_.left() + 5, bounding_box_.bottom()));
		this->GetDialog()->DrawSprite(*elements_[7], IRect(bounding_box_.left() + 5, bounding_box_.bottom() - 5,
			bounding_box_.right() - 5, bounding_box_.bottom()));
		this->GetDialog()->DrawSprite(*elements_[8], IRect(bounding_box_.right() - 5, bounding_box_.bottom() - 5,
			bounding_box_.right(), bounding_box_.bottom()));

		// Main button
		this->GetDialog()->DrawSprite(*elements_[9], rcWindow);
	}

	TexturePtr const & UITexButton::GetTexture() const
	{
		return UIManager::Instance().GetTexture(tex_index_);
	}

	void UITexButton::SetTexture(TexturePtr const & tex)
	{
		tex_index_ = UIManager::Instance().AddTexture(tex);
		if (tex)
		{
			elements_[9]->SetTexture(static_cast<uint32_t>(tex_index_), IRect(0, 0, tex->Width(0), tex->Height(0)));
		}
		else
		{
			elements_[9]->SetTexture(static_cast<uint32_t>(tex_index_), IRect(0, 0, 1, 1));
		}
	}

	void UITexButton::OnHotkey()
	{
		if (this->GetDialog()->IsKeyboardInputEnabled())
		{
			this->GetDialog()->RequestFocus(*this);
		}

		this->OnClickedEvent()(*this);
	}
}
