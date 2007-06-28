// UIComboBox.cpp
// KlayGE 图形用户界面组合框类 实现文件
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
	UIComboBox::UIComboBox(UIDialogPtr dialog)
						: UIControl(UIComboBox::Type, dialog),
							scroll_bar_(dialog),
							pressed_(false),
							drop_height_(100),
							sb_width_(16), opened_(false), selected_(-1), focused_(-1)
	{
		this->InitDefaultElements();

		this->GetDialog()->InitControl(scroll_bar_);
	}

	UIComboBox::UIComboBox(uint32_t type, UIDialogPtr dialog)
						: UIControl(type, dialog),
							scroll_bar_(dialog),
							pressed_(false),
							drop_height_(100),
							sb_width_(16), opened_(false), selected_(-1), focused_(-1)
	{
		this->InitDefaultElements();

		this->GetDialog()->InitControl(scroll_bar_);
	}

	UIComboBox::UIComboBox(UIDialogPtr dialog, int ID, int x, int y, int width, int height, uint32_t nHotkey, bool bIsDefault)
						: UIControl(UIComboBox::Type, dialog),
							scroll_bar_(dialog),
							pressed_(false),
							drop_height_(100),
							sb_width_(16), opened_(false), selected_(-1), focused_(-1)
	{
		this->InitDefaultElements();

		this->GetDialog()->InitControl(scroll_bar_);

		// Set the ID and list index
		this->SetID(ID); 
		this->SetLocation(x, y);
		this->SetSize(width, height);
		this->SetHotkey(nHotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UIComboBox::InitDefaultElements()
	{
		UIElement Element;

		// Main
		{
			Element.SetTexture(0, Rect_T<int32_t>(7, 81, 247, 123));
			Element.SetFont(0);
			Element.TextureColor().States[UICS_Normal] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 150.0f / 255);
			Element.TextureColor().States[UICS_Focus] = Color(230.0f / 255, 230.0f / 255, 230.0f / 255, 170.0f / 255);
			Element.TextureColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 70.0f / 255);
			Element.FontColor().States[UICS_MouseOver] = Color(0, 0, 0, 1);
			Element.FontColor().States[UICS_Pressed] = Color(0, 0, 0, 1);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		// Button
		{
			Element.SetTexture(0, Rect_T<int32_t>(98, 189, 151, 238));
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 150.0f / 255);
			Element.TextureColor().States[UICS_Pressed] = Color(150.0f / 255, 150.0f / 255, 150.0f / 255, 1);
			Element.TextureColor().States[UICS_Focus] = Color(1, 1, 1, 200.0f / 255);
			Element.TextureColor().States[UICS_Disabled] = Color(1, 1, 1, 70.0f / 255);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		// Dropdown
		{
			Element.SetTexture(0, Rect_T<int32_t>(13, 123, 241, 160));
			Element.SetFont(0, Color(0, 0, 0, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		// Selection
		{
			Element.SetTexture(0, Rect_T<int32_t>(12, 163, 239, 183));
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		key_down_event_.connect(boost::bind(&UIComboBox::KeyDownHandler, this, _1, _2));
		key_up_event_.connect(boost::bind(&UIComboBox::KeyUpHandler, this, _1, _2));

		mouse_over_event_.connect(boost::bind(&UIComboBox::MouseOverHandler, this, _1, _2));
		mouse_down_event_.connect(boost::bind(&UIComboBox::MouseDownHandler, this, _1, _2));
		mouse_up_event_.connect(boost::bind(&UIComboBox::MouseUpHandler, this, _1, _2));
		mouse_wheel_event_.connect(boost::bind(&UIComboBox::MouseWheelHandler, this, _1, _2));
	}

	UIComboBox::~UIComboBox()
	{
		this->RemoveAllItems();
	}

	void UIComboBox::SetTextColor(Color const & color)
	{
		UIElementPtr pElement = elements_[0];

		if (pElement)
		{
			pElement->FontColor().States[UICS_Normal] = color;
		}

		pElement = elements_[2];

		if (pElement)
		{
			pElement->FontColor().States[UICS_Normal] = color;
		}
	}

	void UIComboBox::OnFocusOut()
	{
		UIControl::OnFocusOut();

		opened_ = false;
		this->UpdateRects();
	}

	void UIComboBox::UpdateRects()
	{
		UIControl::UpdateRects();

		text_rc_ = button_rc_ = Rect_T<int32_t>(x_, y_, x_ + width_, y_ + height_);
		button_rc_.left() = button_rc_.right() - button_rc_.Height();
		text_rc_.right() = button_rc_.left();

		if (opened_)
		{
			dropdown_rc_ = text_rc_;
			dropdown_rc_ += Vector_T<int32_t, 2>(0, static_cast<int32_t>(0.90f * text_rc_.Height()));
			dropdown_rc_.bottom() += drop_height_;
			dropdown_rc_.right() -= sb_width_;

			dropdown_text_rc_ = dropdown_rc_;
			dropdown_text_rc_.left() += static_cast<int32_t>(0.1f * dropdown_rc_.Width());
			dropdown_text_rc_.right() -= static_cast<int32_t>(0.1f * dropdown_rc_.Width());
			dropdown_text_rc_.top() += static_cast<int32_t>(0.1f * dropdown_rc_.Height());
			dropdown_text_rc_.bottom() -= static_cast<int32_t>(0.1f * dropdown_rc_.Height());

			// Update the scrollbar's rects
			scroll_bar_.SetLocation(dropdown_rc_.right(), dropdown_rc_.top() + 2);
			scroll_bar_.SetSize(sb_width_, dropdown_rc_.Height() - 2);
			FontPtr font = UIManager::Instance().GetFont(elements_[2]->FontIndex());
			if (font && font->FontHeight())
			{
				scroll_bar_.SetPageSize(dropdown_text_rc_.Height() / font->FontHeight());

				// The selected item may have been scrolled off the page.
				// Ensure that it is in page again.
				scroll_bar_.ShowItem(selected_);
			}

			bounding_box_ = button_rc_ | text_rc_ | dropdown_rc_ | dropdown_text_rc_ | scroll_bar_.BoundingBoxRect();
		}
		else
		{
			bounding_box_ = button_rc_ | text_rc_;
		}		
	}

	void UIComboBox::KeyDownHandler(UIDialog const & sender, KeyEventArg const & arg)
	{
		scroll_bar_.GetKeyDownEvent()(sender, arg);

		switch (arg.key)
		{
		case KS_Enter:
			if (opened_)
			{
				if (selected_ != focused_)
				{
					selected_ = focused_;
					this->OnSelectionChangedEvent()(*this);
				}
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

	void UIComboBox::KeyUpHandler(UIDialog const & sender, KeyEventArg const & arg)
	{
		scroll_bar_.GetKeyUpEvent()(sender, arg);
	}

	void UIComboBox::MouseOverHandler(UIDialog const & sender, MouseEventArg const & arg)
	{
		scroll_bar_.GetMouseOverEvent()(sender, arg);

		if (opened_ && dropdown_rc_.PtInRect(arg.location))
		{
			// Determine which item has been selected
			for (size_t i = 0; i < items_.size(); ++ i)
			{
				boost::shared_ptr<UIComboBoxItem> pItem = items_[i];
				if (pItem->bVisible && pItem->rcActive.PtInRect(arg.location))
				{
					focused_ = static_cast<int>(i);
				}
			}
		}
	}

	void UIComboBox::MouseDownHandler(UIDialog const & sender, MouseEventArg const & arg)
	{
		scroll_bar_.GetMouseDownEvent()(sender, arg);

		if (arg.buttons & UIControl::MB_Left)
		{
			if (this->ContainsPoint(arg.location))
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
			if (opened_ && dropdown_rc_.PtInRect(arg.location))
			{
				// Determine which item has been selected
				for (size_t i = scroll_bar_.GetTrackPos(); i < items_.size(); ++ i)
				{
					boost::shared_ptr<UIComboBoxItem> pItem = items_[i];
					if (pItem -> bVisible && pItem->rcActive.PtInRect(arg.location))
					{
						focused_ = selected_ = static_cast<int>(i);
						this->OnSelectionChangedEvent()(*this);
						opened_ = false;
						this->UpdateRects();

						if (!this->GetDialog()->IsKeyboardInputEnabled())
						{
							this->GetDialog()->ClearFocus();
						}

						break;
					}
				}

				return;
			}

			// Mouse click not on main control or in dropdown, fire an event if needed
			if (opened_)
			{
				focused_ = selected_;

				this->OnSelectionChangedEvent()(*this);
				opened_ = false;
				this->UpdateRects();
			}

			// Make sure the control is no longer in a pressed state
			pressed_ = false;

			if (!this->GetDialog()->IsKeyboardInputEnabled())
			{
				this->GetDialog()->ClearFocus();
			}
		}
	}

	void UIComboBox::MouseUpHandler(UIDialog const & sender, MouseEventArg const & arg)
	{
		scroll_bar_.GetMouseUpEvent()(sender, arg);

		if (arg.buttons & UIControl::MB_Left)
		{
			if (pressed_ && this->ContainsPoint(arg.location))
			{
				// Button click
				pressed_ = false;
			}
		}
	}

	void UIComboBox::MouseWheelHandler(UIDialog const & sender, MouseEventArg const & arg)
	{
		scroll_bar_.GetMouseWheelEvent()(sender, arg);

		if (opened_)
		{
			int const WHEELSCROLLLINES = 3;
			scroll_bar_.Scroll(-arg.z_delta * WHEELSCROLLLINES);
		}
		else
		{
			if (arg.z_delta > 0)
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
				if (focused_ + 1 < (int)GetNumItems())
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

		// Dropdown box
		UIElementPtr pElement = elements_[2];

		// If we have not initialized the scroll bar page size,
		// do that now.
		static bool bSBInit;
		if (!bSBInit)
		{
			// Update the page size of the scroll bar
			if (UIManager::Instance().GetFont(pElement->FontIndex())->FontHeight())
			{
				scroll_bar_.SetPageSize(dropdown_text_rc_.Height() / UIManager::Instance().GetFont(pElement->FontIndex())->FontHeight());
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
		pElement->TextureColor().SetState(iState);
		pElement->FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(*pElement, dropdown_rc_);

		// Selection outline
		UIElementPtr pSelectionElement = elements_[3];
		pSelectionElement->TextureColor().Current = pElement->TextureColor().Current;
		pSelectionElement->FontColor().Current = pSelectionElement->FontColor().States[UICS_Normal];

		FontPtr font = this->GetDialog()->GetFont(pElement->FontIndex());
		if (font)
		{
			int curY = dropdown_text_rc_.top();
			int nRemainingHeight = dropdown_text_rc_.Height();

			for (size_t i = scroll_bar_.GetTrackPos(); i < items_.size(); ++ i)
			{
				boost::shared_ptr<UIComboBoxItem> pItem = items_[i];

				// Make sure there's room left in the dropdown
				nRemainingHeight -= font->FontHeight();
				if (nRemainingHeight < 0)
				{
					pItem->bVisible = false;
					continue;
				}

				pItem->rcActive = Rect_T<int32_t>(dropdown_text_rc_.left(), curY, dropdown_text_rc_.right(), curY + font->FontHeight());
				curY += font->FontHeight();

				//debug
				//int blue = 50 * i;
				//m_pDialog->DrawRect(&pItem->rcActive, 0xFFFF0000 | blue);

				pItem->bVisible = true;

				if (opened_)
				{
					if (static_cast<int>(i) == focused_)
					{
						Rect_T<int32_t> rc(dropdown_rc_.left(), pItem->rcActive.top() - 2, dropdown_rc_.right(), pItem->rcActive.bottom() + 2);
						this->GetDialog()->DrawSprite(*pSelectionElement, rc);
						this->GetDialog()->DrawText(pItem->strText, *pSelectionElement, pItem->rcActive);
					}
					else
					{
						this->GetDialog()->DrawText(pItem->strText, *pElement, pItem->rcActive);
					}
				}
			}
		}

		int nOffsetX = 0;
		int nOffsetY = 0;

		iState = UICS_Normal;

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

		// Button
		pElement = elements_[1];

		// Blend current color
		pElement->TextureColor().SetState(iState);

		Rect_T<int32_t> rcWindow = button_rc_;
		rcWindow += Vector_T<int32_t, 2>(nOffsetX, nOffsetY);
		this->GetDialog()->DrawSprite(*pElement, rcWindow);

		if (opened_)
		{
			iState = UICS_Pressed;
		}

		// Main text box
		pElement = elements_[0];

		// Blend current color
		pElement->TextureColor().SetState(iState);
		pElement->FontColor().SetState(iState);

		this->GetDialog()->DrawSprite(*pElement, text_rc_);

		if ((selected_ >= 0) && (selected_ < static_cast<int>(items_.size())))
		{
			boost::shared_ptr<UIComboBoxItem> pItem = items_[selected_];
			if (pItem != NULL)
			{
				this->GetDialog()->DrawText(pItem->strText, *pElement, text_rc_);
			}
		}
	}

	void UIComboBox::AddItem(std::wstring const & strText, void* pData)
	{
		assert(!strText.empty());

		// Create a new item and set the data
		boost::shared_ptr<UIComboBoxItem> pItem(new UIComboBoxItem);
		pItem->strText = strText;
		pItem->pData = pData;
		pItem->rcActive = Rect_T<int32_t>(0, 0, 0, 0);
		pItem->bVisible = false;

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

	void* UIComboBox::GetSelectedData() const
	{
		if (selected_ < 0)
		{
			return NULL;
		}

		return items_[selected_]->pData;
	}

	boost::shared_ptr<UIComboBoxItem> UIComboBox::GetSelectedItem() const
	{
		if (selected_ < 0)
		{
			return boost::shared_ptr<UIComboBoxItem>();
		}

		return items_[selected_];
	}

	void* UIComboBox::GetItemData(std::wstring const & strText) const
	{
		int index = this->FindItem(strText);
		if (index == -1)
		{
			return NULL;
		}

		boost::shared_ptr<UIComboBoxItem> pItem = items_[index];
		if (!pItem)
		{
			return NULL;
		}

		return pItem->pData;
	}

	void* UIComboBox::GetItemData(int nIndex) const
	{
		if ((nIndex < 0) || (nIndex >= static_cast<int>(items_.size())))
		{
			return NULL;
		}

		return items_[nIndex]->pData;
	}

	void UIComboBox::SetSelectedByIndex(uint32_t index)
	{
		assert(index < GetNumItems());

		focused_ = selected_ = index;
		this->OnSelectionChangedEvent()(*this);
	}

	void UIComboBox::SetSelectedByText(std::wstring const & strText)
	{
		assert(!strText.empty());

		int index = this->FindItem(strText);
		assert(index != -1);

		this->SetSelectedByIndex(index);
	}

	void UIComboBox::SetSelectedByData(void* pData)
	{
		for (size_t i = 0; i < items_.size(); ++ i)
		{
			boost::shared_ptr<UIComboBoxItem> pItem = items_[i];

			if (pItem->pData == pData)
			{
				this->SetSelectedByIndex(static_cast<uint32_t>(i));
			}
		}
	}
}
