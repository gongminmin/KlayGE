// UIComboBox.cpp
// KlayGE 图形用户界面组合框类 实现文件
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
#include <KlayGE/Font.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIComboBox::UIComboBox(UIDialogPtr const & dialog)
						: UIComboBox(UIComboBox::Type, dialog)
	{
	}

	UIComboBox::UIComboBox(uint32_t type, UIDialogPtr const & dialog)
						: UIControl(type, dialog),
							selected_(-1), focused_(-1),
							drop_height_(100), scroll_bar_(dialog),
							sb_width_(16), opened_(false),
							pressed_(false)
	{
		UIElement Element;

		// Main
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ComboBox, 0));
			Element.SetFont(0);
			Element.TextureColor().States[UICS_Normal] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 150.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(230.0f / 255, 230.0f / 255, 230.0f / 255, 170.0f / 255);
			Element.TextureColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 70.0f / 255);
			Element.FontColor().States[UICS_MouseOver] = Color(0, 0, 0, 1);
			Element.FontColor().States[UICS_Pressed] = Color(0, 0, 0, 1);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Button
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ComboBox, 1));
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(150.0f / 255, 150.0f / 255, 150.0f / 255, 1);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 200.0f / 255);
			Element.TextureColor().States[UICS_Disabled] = Color(1, 1, 1, 70.0f / 255);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Dropdown
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ComboBox, 2));
			Element.SetFont(0, Color(0, 0, 0, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Selection
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ComboBox, 3));
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		this->GetDialog()->InitControl(scroll_bar_);
	}

	UIComboBox::UIComboBox(UIDialogPtr const & dialog, int ID, int4 const & coord_size, uint8_t hotkey, bool bIsDefault)
						: UIComboBox(dialog)
	{
		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	UIComboBox::~UIComboBox()
	{
		this->RemoveAllItems();
	}

	void UIComboBox::SetTextColor(Color const & color)
	{
		auto main_element = elements_[0].get();
		if (main_element)
		{
			main_element->FontColor().States[UICS_Normal] = color;
		}

		auto dropdown_element = elements_[2].get();
		if (dropdown_element)
		{
			dropdown_element->FontColor().States[UICS_Normal] = color;
		}
	}

	void UIComboBox::OnFocusOut()
	{
		UIControl::OnFocusOut();

		pressed_ = false;
		opened_ = false;
		this->UpdateRects();
	}

	void UIComboBox::UpdateRects()
	{
		UIControl::UpdateRects();

		show_rc_ = IRect(x_, y_, x_ + width_, y_ + height_);

		text_rc_ = button_rc_ = show_rc_;
		button_rc_.left() = button_rc_.right() - button_rc_.Height();
		text_rc_.right() = button_rc_.left();

		if (opened_)
		{
			dropdown_rc_ = text_rc_;
			dropdown_rc_ += int2(2, text_rc_.Height());
			dropdown_rc_.bottom() += drop_height_;

			dropdown_text_rc_ = dropdown_rc_;
			dropdown_text_rc_.left() += static_cast<int32_t>(0.1f * dropdown_rc_.Width());
			dropdown_text_rc_.right() -= static_cast<int32_t>(0.1f * dropdown_rc_.Width());
			dropdown_text_rc_.top() += static_cast<int32_t>(0.1f * dropdown_rc_.Height());
			dropdown_text_rc_.bottom() -= static_cast<int32_t>(0.1f * dropdown_rc_.Height());

			// Update the scrollbar's rects
			scroll_bar_.SetLocation(dropdown_rc_.right(), dropdown_rc_.top() + 2);
			scroll_bar_.SetSize(sb_width_, dropdown_rc_.Height() - 2);
			FontPtr const & font = UIManager::Instance().GetFont(elements_[2]->FontIndex());
			float font_size = UIManager::Instance().GetFontSize(elements_[2]->FontIndex());
			if (font && (font_size != 0))
			{
				scroll_bar_.SetPageSize(static_cast<size_t>(dropdown_text_rc_.Height() / font_size));

				// The selected item may have been scrolled off the page.
				// Ensure that it is in page again.
				scroll_bar_.ShowItem(selected_);
			}

			bounding_box_ = show_rc_ | button_rc_ | text_rc_ | dropdown_rc_ | dropdown_text_rc_ | scroll_bar_.BoundingBoxRect();
		}
		else
		{
			bounding_box_ = show_rc_ | button_rc_ | text_rc_;
		}
	}

	void UIComboBox::KeyDownHandler(UIDialog const & sender, uint32_t key)
	{
		scroll_bar_.KeyDownHandler(sender, key);

		switch (key & 0xFF)
		{
		case KS_Enter:
			if (opened_)
			{
				selected_ = focused_;
				this->OnSelectionChangedEvent()(*this);
				opened_ = false;

				this->UpdateRects();

				if (!this->GetDialog()->IsKeyboardInputEnabled())
				{
					this->GetDialog()->ClearFocus();
				}
			}
			break;

		case KS_F4:
			opened_ = !opened_;
			this->UpdateRects();

			if (!opened_)
			{
				this->OnSelectionChangedEvent()(*this);

				if (!this->GetDialog()->IsKeyboardInputEnabled())
				{
					this->GetDialog()->ClearFocus();
				}
			}
			break;

		case KS_LeftArrow:
		case KS_UpArrow:
			if (focused_ > 0)
			{
				-- focused_;
				selected_ = focused_;

				if (!opened_)
				{
					this->OnSelectionChangedEvent()(*this);
				}
			}
			break;

		case KS_RightArrow:
		case KS_DownArrow:
			if (focused_ + 1 < static_cast<int>(this->GetNumItems()))
			{
				++ focused_;
				selected_ = focused_;

				if (!opened_)
				{
					this->OnSelectionChangedEvent()(*this);
				}
			}
			break;
		}
	}

	void UIComboBox::KeyUpHandler(UIDialog const & sender, uint32_t key)
	{
		scroll_bar_.KeyUpHandler(sender, key);
	}

	void UIComboBox::MouseOverHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt)
	{
		scroll_bar_.MouseOverHandler(sender, buttons, pt);

		if (opened_ && dropdown_rc_.PtInRect(pt))
		{
			// Determine which item has been selected
			for (size_t i = 0; i < items_.size(); ++ i)
			{
				std::shared_ptr<UIComboBoxItem> const & pItem = items_[i];
				if (pItem->bVisible && pItem->rcActive.PtInRect(pt))
				{
					focused_ = static_cast<int>(i);
				}
			}
		}
	}

	void UIComboBox::MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt)
	{
		scroll_bar_.MouseDownHandler(sender, buttons, pt);

		if (buttons & MB_Left)
		{
			if (show_rc_.PtInRect(pt))
			{
				// Pressed while inside the control
				pressed_ = true;

				if (!has_focus_)
				{
					this->GetDialog()->RequestFocus(*this);
				}

				// Toggle dropdown
				if (has_focus_)
				{
					opened_ = !opened_;
					this->UpdateRects();

					if (!opened_)
					{
						if (!this->GetDialog()->IsKeyboardInputEnabled())
						{
							this->GetDialog()->ClearFocus();
						}
					}
				}

				return;
			}

			// Perhaps this click is within the dropdown
			if (opened_ && dropdown_rc_.PtInRect(pt))
			{
				// Determine which item has been selected
				for (size_t i = scroll_bar_.GetTrackPos(); i < items_.size(); ++ i)
				{
					std::shared_ptr<UIComboBoxItem> const & pItem = items_[i];
					if (pItem -> bVisible && pItem->rcActive.PtInRect(pt))
					{
						focused_ = static_cast<int>(i);
						break;
					}
				}

				return;
			}

			// Make sure the control is no longer in a pressed state
			pressed_ = false;

			if (!this->GetDialog()->IsKeyboardInputEnabled())
			{
				this->GetDialog()->ClearFocus();
			}
		}
	}

	void UIComboBox::MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt)
	{
		scroll_bar_.MouseUpHandler(sender, buttons, pt);

		if (buttons & MB_Left)
		{
			// Perhaps this click is within the dropdown
			if (opened_ && dropdown_rc_.PtInRect(pt))
			{
				selected_ = focused_;

				this->OnSelectionChangedEvent()(*this);
				opened_ = false;
				this->UpdateRects();

				pressed_ = false;
			}
			else
			{
				if (opened_ && !this->ContainsPoint(pt))
				{
					opened_ = false;
					this->UpdateRects();
				}
			}

			if (pressed_ && this->ContainsPoint(pt))
			{
				// Button click
				pressed_ = false;
			}
		}
	}

	void UIComboBox::MouseWheelHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt, int32_t z_delta)
	{
		scroll_bar_.MouseWheelHandler(sender, buttons, pt, z_delta);

		if (opened_)
		{
			int const WHEELSCROLLLINES = 3;
			scroll_bar_.Scroll(static_cast<int>(-z_delta / 120.0f * WHEELSCROLLLINES));
		}
		else
		{
			if (z_delta > 0)
			{
				if (focused_ > 0)
				{
					-- focused_;
					selected_ = focused_;

					if (!opened_)
					{
						this->OnSelectionChangedEvent()(*this);
					}
				}
			}
			else
			{
				if (focused_ + 1 < static_cast<int>(this->GetNumItems()))
				{
					++ focused_;
					selected_ = focused_;

					if (!opened_)
					{
						this->OnSelectionChangedEvent()(*this);
					}
				}
			}
		}
	}

	void UIComboBox::Render()
	{
		UI_Control_State iState = UICS_Normal;

		if (!opened_)
		{
			iState = UICS_Hidden;
		}

		auto& dropdown_element = *elements_[2];

		// If we have not initialized the scroll bar page size,
		// do that now.
		static bool bSBInit;
		if (!bSBInit)
		{
			// Update the page size of the scroll bar
			if (UIManager::Instance().GetFontSize(dropdown_element.FontIndex()) != 0)
			{
				scroll_bar_.SetPageSize(
					static_cast<size_t>(dropdown_text_rc_.Height() / UIManager::Instance().GetFontSize(dropdown_element.FontIndex())));
			}
			else
			{
				scroll_bar_.SetPageSize(dropdown_text_rc_.Height());
			}
			bSBInit = true;
		}

		// Scroll bar
		if (opened_)
		{
			scroll_bar_.Render();
		}

		// Blend current color
		dropdown_element.TextureColor().SetState(iState);
		dropdown_element.FontColor().SetState(iState);

		float depth_bias = 0.0f;
		if (opened_)
		{
			depth_bias = -0.1f;
		}

		this->GetDialog()->DrawSprite(dropdown_element, dropdown_rc_, depth_bias);

		// Selection outline
		auto& selection_element = *elements_[3];
		selection_element.TextureColor().Current = dropdown_element.TextureColor().Current;
		selection_element.FontColor().Current = selection_element.FontColor().States[UICS_Normal];

		FontPtr const & font = this->GetDialog()->GetFont(dropdown_element.FontIndex());
		uint32_t font_size = static_cast<uint32_t>(this->GetDialog()->GetFontSize(dropdown_element.FontIndex()) + 0.5f);
		if (font)
		{
			int curY = dropdown_text_rc_.top();
			int nRemainingHeight = dropdown_text_rc_.Height();

			for (size_t i = scroll_bar_.GetTrackPos(); i < items_.size(); ++ i)
			{
				std::shared_ptr<UIComboBoxItem> const & pItem = items_[i];

				// Make sure there's room left in the dropdown
				nRemainingHeight -= font_size;
				if (nRemainingHeight < 0)
				{
					pItem->bVisible = false;
					continue;
				}

				pItem->rcActive = IRect(dropdown_text_rc_.left(), curY, dropdown_text_rc_.right(), curY + font_size);
				curY += font_size;

				//debug
				//int blue = 50 * i;
				//m_pDialog->DrawRect(&pItem->rcActive, 0xFFFF0000 | blue);

				pItem->bVisible = true;

				if (opened_)
				{
					if (static_cast<int>(i) == focused_)
					{
						IRect rc(dropdown_rc_.left(), pItem->rcActive.top() - 2, dropdown_rc_.right(), pItem->rcActive.bottom() + 2);
						this->GetDialog()->DrawSprite(selection_element, rc, depth_bias);
						this->GetDialog()->DrawString(pItem->strText, selection_element, pItem->rcActive, false, depth_bias);
					}
					else
					{
						this->GetDialog()->DrawString(pItem->strText, dropdown_element, pItem->rcActive, false, depth_bias);
					}
				}
			}
		}

		iState = UICS_Normal;

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

		auto& button_element = *elements_[1];

		button_element.TextureColor().SetState(iState);

		IRect rcWindow = button_rc_;
		this->GetDialog()->DrawSprite(button_element, rcWindow, depth_bias);

		if (opened_)
		{
			iState = UICS_Pressed;
		}

		auto& text_element = *elements_[0];

		// Blend current color
		text_element.TextureColor().SetState(iState);
		text_element.FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(text_element, text_rc_, depth_bias);

		if ((selected_ >= 0) && (selected_ < static_cast<int>(items_.size())))
		{
			std::shared_ptr<UIComboBoxItem> const & pItem = items_[selected_];
			if (pItem)
			{
				this->GetDialog()->DrawString(pItem->strText, text_element, text_rc_, false, depth_bias);
			}
		}
	}

	int UIComboBox::AddItem(std::wstring const & strText)
	{
		BOOST_ASSERT(!strText.empty());

		// Create a new item and set the data
		std::shared_ptr<UIComboBoxItem> pItem = MakeSharedPtr<UIComboBoxItem>();
		pItem->strText = strText;
		pItem->rcActive = IRect(0, 0, 0, 0);
		pItem->bVisible = false;

		int ret = static_cast<int>(items_.size());

		items_.push_back(pItem);

		// Update the scroll bar with new range
		scroll_bar_.SetTrackRange(0, items_.size());

		// If this is the only item in the list, it's selected
		if (1 == this->GetNumItems())
		{
			selected_ = 0;
			focused_ = 0;
			this->OnSelectionChangedEvent()(*this);
		}

		return ret;
	}

	void UIComboBox::RemoveItem(uint32_t index)
	{
		items_.erase(items_.begin() + index);
		scroll_bar_.SetTrackRange(0, items_.size());
		if (selected_ >= static_cast<int>(items_.size()))
		{
			selected_ = static_cast<int>(items_.size() - 1);
		}
	}

	void UIComboBox::RemoveAllItems()
	{
		items_.clear();
		scroll_bar_.SetTrackRange(0, 1);
		focused_ = selected_ = -1;
	}

	bool UIComboBox::ContainsItem(std::wstring const & strText, uint32_t iStart) const
	{
		return (-1 != this->FindItem(strText, iStart));
	}

	int UIComboBox::FindItem(std::wstring const & strText, uint32_t iStart) const
	{
		if (strText.empty())
		{
			return -1;
		}

		for (size_t i = iStart; i < items_.size(); ++ i)
		{
			if (items_[i]->strText == strText)
			{
				return static_cast<int>(i);
			}
		}

		return -1;
	}

	std::shared_ptr<UIComboBoxItem> UIComboBox::GetSelectedItem() const
	{
		if (selected_ < 0)
		{
			return std::shared_ptr<UIComboBoxItem>();
		}

		return items_[selected_];
	}

	int UIComboBox::GetSelectedIndex() const
	{
		return selected_;
	}

	void UIComboBox::SetSelectedByIndex(uint32_t index)
	{
		BOOST_ASSERT(index < this->GetNumItems());

		focused_ = selected_ = index;
		this->OnSelectionChangedEvent()(*this);
	}

	void UIComboBox::SetSelectedByText(std::wstring const & strText)
	{
		BOOST_ASSERT(!strText.empty());

		int index = this->FindItem(strText);
		BOOST_ASSERT(index != -1);

		this->SetSelectedByIndex(index);
	}

	void UIComboBox::OnHotkey()
	{
		if (opened_)
		{
			return;
		}

		if (-1 == selected_)
		{
			return;
		}

		if (this->GetDialog()->IsKeyboardInputEnabled())
		{
			this->GetDialog()->RequestFocus(*this);
		}

		++ selected_;

		if (selected_ >= static_cast<int>(items_.size()))
		{
			selected_ = 0;
		}

		focused_ = selected_;
		this->OnSelectionChangedEvent()(*this);
	}
}
