// UIListBox.cpp
// KlayGE 图形用户界面列表框类 实现文件
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
#include <KlayGE/Context.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/Font.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIListBox::UIListBox(UIDialogPtr const & dialog)
						: UIListBox(UIListBox::Type, dialog)
	{
	}

	UIListBox::UIListBox(uint32_t type, UIDialogPtr const & dialog)
						: UIControl(type, dialog),
							scroll_bar_(dialog),
							sb_width_(16), border_(6), margin_(5), text_height_(0),
							style_(SINGLE_SELECTION),
							selected_(-1), sel_start_(0),
							drag_(false)
	{
		UIElement Element;

		// Main
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ListBox, 0));
			Element.SetFont(0, Color(0, 0, 0, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Selection
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ListBox, 1));
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		this->GetDialog()->InitControl(scroll_bar_);
	}

	UIListBox::UIListBox(UIDialogPtr const & dialog, int ID, int4 const & coord_size, STYLE dwStyle)
						: UIListBox(dialog)

	{
		style_ = dwStyle;

		// Set the ID and position
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
	}

	UIListBox::~UIListBox()
	{
		this->RemoveAllItems();
	}

	void UIListBox::UpdateRects()
	{
		UIControl::UpdateRects();

		selection_rc_ = IRect(x_, y_, x_ + width_, y_ + height_);
		selection_rc_.right() -= sb_width_;
		selection_rc_ += IRect(border_, border_, -border_, -border_);
		text_rc_ = selection_rc_;
		text_rc_ += IRect(margin_, 0, -margin_, 0);

		// Update the scrollbar's rects
		scroll_bar_.SetLocation(x_ + width_ - sb_width_, y_);
		scroll_bar_.SetSize(sb_width_, height_);
		FontPtr font = UIManager::Instance().GetFont(elements_[0]->FontIndex());
		float font_size = UIManager::Instance().GetFontSize(elements_[0]->FontIndex());
		if (font && (font_size != 0))
		{
			scroll_bar_.SetPageSize(static_cast<size_t>(text_rc_.Height() / font_size));

			// The selected item may have been scrolled off the page.
			// Ensure that it is in page again.
			scroll_bar_.ShowItem(selected_);
		}

		bounding_box_ = selection_rc_ | text_rc_ | scroll_bar_.BoundingBoxRect();
	}

	int UIListBox::AddItem(std::wstring const & strText)
	{
		std::shared_ptr<UIListBoxItem> pNewItem = MakeSharedPtr<UIListBoxItem>();
		pNewItem->strText = strText;
		pNewItem->rcActive = IRect(0, 0, 0, 0);
		pNewItem->bSelected = false;

		int ret = static_cast<int>(items_.size());

		items_.push_back(pNewItem);
		scroll_bar_.SetTrackRange(0, items_.size());

		return ret;
	}

	void UIListBox::InsertItem(int nIndex, std::wstring const & strText)
	{
		std::shared_ptr<UIListBoxItem> pNewItem = MakeSharedPtr<UIListBoxItem>();
		pNewItem->strText = strText;
		pNewItem->rcActive = IRect(0, 0, 0, 0);
		pNewItem->bSelected = false;

		items_.insert(items_.begin() + nIndex, pNewItem);
		scroll_bar_.SetTrackRange(0, items_.size());
	}

	void UIListBox::RemoveItem(int nIndex)
	{
		BOOST_ASSERT((nIndex >= 0) && (nIndex < static_cast<int>(items_.size())));

		items_.erase(items_.begin() + nIndex);
		scroll_bar_.SetTrackRange(0, items_.size());
		if (selected_ >= static_cast<int>(items_.size()))
		{
			selected_ = static_cast<int>(items_.size() - 1);
		}

		this->OnSelectionEvent()(*this);
	}

	void UIListBox::RemoveAllItems()
	{
		items_.clear();
		scroll_bar_.SetTrackRange(0, 1);
	}

	std::shared_ptr<UIListBoxItem> UIListBox::GetItem(int nIndex) const
	{
		BOOST_ASSERT((nIndex >= 0) && (nIndex < static_cast<int>(items_.size())));

		return items_[nIndex];
	}

	// For single-selection listbox, returns the index of the selected item.
	// For multi-selection, returns the first selected item after the nPreviousSelected position.
	// To search for the first selected item, the app passes -1 for nPreviousSelected.  For
	// subsequent searches, the app passes the returned index back to GetSelectedIndex as.
	// nPreviousSelected.
	// Returns -1 on error or if no item is selected.
	int UIListBox::GetSelectedIndex(int nPreviousSelected) const
	{
		if (nPreviousSelected < -1)
		{
			return -1;
		}

		if (MULTI_SELECTION == style_)
		{
			// Multiple selection enabled. Search for the next item with the selected flag.
			for (int i = nPreviousSelected + 1; i < static_cast<int>(items_.size()); ++ i)
			{
				if (items_[i]->bSelected)
				{
					return i;
				}
			}

			return -1;
		}
		else
		{
			// Single selection
			return selected_;
		}
	}

	void UIListBox::SelectItem(int nNewIndex)
	{
		// If no item exists, do nothing.
		if (items_.empty())
		{
			return;
		}

		int nOldSelected = selected_;

		// Adjust selected_
		selected_ = nNewIndex;

		// Perform capping
		if (selected_ < 0)
		{
			selected_ = 0;
		}
		if (selected_ >= static_cast<int>(items_.size()))
		{
			selected_ = static_cast<int>(items_.size() - 1);
		}

		if (nOldSelected != selected_)
		{
			if (MULTI_SELECTION == style_)
			{
				items_[selected_]->bSelected = true;
			}

			// Update selection start
			sel_start_ = selected_;

			// Adjust scroll bar
			scroll_bar_.ShowItem(selected_);
		}

		this->OnSelectionEvent()(*this);
	}

	void UIListBox::KeyDownHandler(UIDialog const & sender, uint32_t key)
	{
		scroll_bar_.KeyDownHandler(sender, key);

		switch (key & 0xFF)
		{
		case KS_UpArrow:
		case KS_DownArrow:
		case KS_PageDown:
		case KS_PageUp:
		case KS_Home:
		case KS_End:
			{
				// If no item exists, do nothing.
				if (items_.empty())
				{
					return;
				}

				int nOldSelected = selected_;

				// Adjust selected_
				switch (key & 0xFF)
				{
				case KS_UpArrow:
					-- selected_;
					break;

				case KS_DownArrow:
					++ selected_;
					break;

				case KS_PageDown:
					selected_ += static_cast<int>(scroll_bar_.GetPageSize() - 1);
					break;

				case KS_PageUp:
					selected_ -= static_cast<int>(scroll_bar_.GetPageSize() - 1);
					break;

				case KS_Home:
					selected_ = 0;
					break;

				case KS_End:
					selected_ = static_cast<int>(items_.size() - 1);
					break;
				}

				// Perform capping
				if (selected_ < 0)
				{
					selected_ = 0;
				}
				if (selected_ >= static_cast<int>(items_.size()))
				{
					selected_ = static_cast<int>(items_.size() - 1);
				}

				if (nOldSelected != selected_)
				{
					if (MULTI_SELECTION == style_)
					{
						// Multiple selection

						// Clear all selection
						for (size_t i = 0; i < items_.size(); ++ i)
						{
							items_[i]->bSelected = false;
						}

						if (key & MB_Shift)
						{
							// Select all items from sel_start_ to
							// selected_
							int nEnd = std::max(sel_start_, selected_);

							for (int n = std::min(sel_start_, selected_); n <= nEnd; ++ n)
							{
								items_[n]->bSelected = true;
							}
						}
						else
						{
							items_[selected_]->bSelected = true;

							// Update selection start
							sel_start_ = selected_;
						}
					}
					else
					{
						sel_start_ = selected_;
					}

					// Adjust scroll bar

					scroll_bar_.ShowItem(selected_);

					// Send notification

					this->OnSelectionEvent()(*this);
				}
			}
		}
	}

	void UIListBox::KeyUpHandler(UIDialog const & sender, uint32_t key)
	{
		scroll_bar_.KeyUpHandler(sender, key);
	}

	void UIListBox::MouseOverHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt)
	{
		// Let the scroll bar handle it first.
		scroll_bar_.MouseOverHandler(sender, buttons, pt);

		if (drag_)
		{
			// Compute the index of the item below cursor

			int nItem;
			if (text_height_ != 0)
			{
				nItem = static_cast<int>(scroll_bar_.GetTrackPos() + (pt.y() - text_rc_.top()) / text_height_);
			}
			else
			{
				nItem = -1;
			}

			// Only proceed if the cursor is on top of an item.

			if ((nItem >= static_cast<int>(scroll_bar_.GetTrackPos()))
				&& (nItem < static_cast<int>(items_.size()))
				&& (nItem < static_cast<int>(scroll_bar_.GetTrackPos() + scroll_bar_.GetPageSize())))
			{
				selected_ = nItem;
				this->OnSelectionEvent()(*this);
			}
			else
			{
				if (nItem < static_cast<int>(scroll_bar_.GetTrackPos()))
				{
					// User drags the mouse above window top
					scroll_bar_.Scroll(-1);
					selected_ = static_cast<int>(scroll_bar_.GetTrackPos());
					this->OnSelectionEvent()(*this);
				}
				else
				{
					if (nItem >= static_cast<int>(scroll_bar_.GetTrackPos() + scroll_bar_.GetPageSize()))
					{
						// User drags the mouse below window bottom
						scroll_bar_.Scroll(1);
						selected_ = static_cast<int>(std::min(items_.size(), scroll_bar_.GetTrackPos() + scroll_bar_.GetPageSize())) - 1;
						this->OnSelectionEvent()(*this);
					}
				}
			}
		}
	}

	void UIListBox::MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			if (!has_focus_)
			{
				this->GetDialog()->RequestFocus(*this);
			}
		}

		// Let the scroll bar handle it first.
		scroll_bar_.MouseDownHandler(sender, buttons, pt);

		if (buttons & MB_Left)
		{
			// Check for clicks in the text area
			if (!items_.empty() && selection_rc_.PtInRect(pt))
			{
				// Compute the index of the clicked item

				int nClicked;
				if (text_height_ != 0)
				{
					nClicked = static_cast<int>(scroll_bar_.GetTrackPos() + (pt.y() - text_rc_.top()) / text_height_);
				}
				else
				{
					nClicked = -1;
				}

				// Only proceed if the click falls on top of an item.

				if ((nClicked >= static_cast<int>(scroll_bar_.GetTrackPos()))
					&& (nClicked < static_cast<int>(items_.size()))
					&& (nClicked < static_cast<int>(scroll_bar_.GetTrackPos() + scroll_bar_.GetPageSize())))
				{
					drag_ = true;

					selected_ = nClicked;
					if (!(buttons & MB_Shift))
					{
						sel_start_ = selected_;
					}

					// If this is a multi-selection listbox, update per-item
					// selection data.

					if (MULTI_SELECTION == style_)
					{
						// Determine behavior based on the state of Shift and Ctrl

						std::shared_ptr<UIListBoxItem> const & pSelItem = items_[selected_];
						if (MB_Ctrl == (buttons & (MB_Shift | MB_Ctrl)))
						{
							// Control click. Reverse the selection of this item.

							pSelItem->bSelected = !pSelItem->bSelected;
						}
						else
						{
							if (MB_Shift == (buttons & (MB_Shift | MB_Ctrl)))
							{
								// Shift click. Set the selection for all items
								// from last selected item to the current item.
								// Clear everything else.

								int nBegin = std::min(sel_start_, selected_);
								int nEnd = std::max(sel_start_, selected_);

								for (int i = 0; i < nBegin; ++ i)
								{
									items_[i]->bSelected = false;
								}

								for (int i = nEnd + 1; i < static_cast<int>(items_.size()); ++ i)
								{
									items_[i]->bSelected = false;
								}

								for (int i = nBegin; i <= nEnd; ++ i)
								{
									items_[i]->bSelected = true;
								}
							}
							else
							{
								if ((MB_Shift | MB_Ctrl) == (buttons & (MB_Shift | MB_Ctrl)))
								{
									// Control-Shift-click.

									// The behavior is:
									//   Set all items from sel_start_ to selected_ to
									//     the same state as sel_start_, not including selected_.
									//   Set selected_ to selected.

									int nBegin = std::min(sel_start_, selected_);
									int nEnd = std::max(sel_start_, selected_);

									// The two ends do not need to be set here.

									bool bLastSelected = items_[sel_start_]->bSelected;
									for (int i = nBegin + 1; i < nEnd; ++ i)
									{
										items_[i]->bSelected = bLastSelected;
									}

									pSelItem->bSelected = true;

									// Restore selected_ to the previous value
									// This matches the Windows behavior

									selected_ = sel_start_;
								}
								else
								{
									// Simple click.  Clear all items and select the clicked
									// item.


									for (size_t i = 0; i < items_.size(); ++ i)
									{
										items_[i]->bSelected = false;
									}

									pSelItem->bSelected = true;
								}
							}
						}
					}  // End of multi-selection case

					this->OnSelectionEvent()(*this);
				}
			}
		}
	}

	void UIListBox::MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt)
	{
		// Let the scroll bar handle it first.
		scroll_bar_.MouseUpHandler(sender, buttons, pt);

		if (buttons & MB_Left)
		{
			drag_ = false;

			if (selected_ != -1)
			{
				// Set all items between sel_start_ and selected_ to
				// the same state as sel_start_
				int nEnd = std::max(sel_start_, selected_);

				for (int n = std::min(sel_start_, selected_) + 1; n < nEnd; ++ n)
				{
					items_[n]->bSelected = items_[sel_start_]->bSelected;
				}
				items_[selected_]->bSelected = items_[sel_start_]->bSelected;

				// If sel_start_ and selected_ are not the same,
				// the user has dragged the mouse to make a selection.
				// Notify the application of this.
				if (sel_start_ != selected_)
				{
					this->OnSelectionEvent()(*this);
				}

				this->OnSelectionEndEvent()(*this);
			}
		}
	}

	void UIListBox::MouseWheelHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt, int32_t z_delta)
	{
		// Let the scroll bar handle it first.
		scroll_bar_.MouseWheelHandler(sender, buttons, pt, z_delta);

		int const WHEELSCROLLLINES = 3;
		scroll_bar_.Scroll(static_cast<int>(-z_delta / 120.0f * WHEELSCROLLLINES));
	}

	void UIListBox::Render()
	{
		auto& main_element = *elements_[0];
		auto& selection_element = *elements_[1];
		if (this->GetEnabled())
		{
			main_element.TextureColor().SetState(UICS_Normal);
			main_element.FontColor().SetState(UICS_Normal);
			selection_element.TextureColor().SetState(UICS_Normal);
			selection_element.FontColor().SetState(UICS_Normal);
		}
		else
		{
			main_element.TextureColor().SetState(UICS_Disabled);
			main_element.FontColor().SetState(UICS_Disabled);
			selection_element.TextureColor().SetState(UICS_Disabled);
			selection_element.FontColor().SetState(UICS_Disabled);
		}

		this->GetDialog()->DrawSprite(main_element, IRect(x_, y_, x_ + width_, y_ + height_));

		// Render the text
		if (!items_.empty())
		{
			// Find out the height of a single line of text
			IRect rc = text_rc_;
			IRect rcSel = selection_rc_;
			rc.bottom() = static_cast<int32_t>(rc.top() + UIManager::Instance().GetFontSize(main_element.FontIndex()));

			// Update the line height formation
			text_height_ = rc.Height();

			static bool bSBInit;
			if (!bSBInit)
			{
				// Update the page size of the scroll bar
				if (text_height_)
				{
					scroll_bar_.SetPageSize(text_rc_.Height() / text_height_);
				}
				else
				{
					scroll_bar_.SetPageSize(text_rc_.Height());
				}
				bSBInit = true;
			}

			rc.right() = text_rc_.right();
			for (int i = static_cast<int>(scroll_bar_.GetTrackPos()); i < static_cast<int>(items_.size()); ++ i)
			{
				if (rc.bottom() > text_rc_.bottom())
				{
					break;
				}

				std::shared_ptr<UIListBoxItem> const & pItem = items_[i];

				// Determine if we need to render this item with the
				// selected element.
				bool bSelectedStyle = false;

				if (!(MULTI_SELECTION == style_) && (i == selected_))
				{
					bSelectedStyle = true;
				}
				else
				{
					if (MULTI_SELECTION == style_)
					{
						if (drag_
							&& (((i >= selected_) && (i < sel_start_))
								|| ((i <= selected_) && (i > sel_start_))))
						{
							bSelectedStyle = items_[sel_start_]->bSelected;
						}
						else
						{
							if (pItem->bSelected)
							{
								bSelectedStyle = true;
							}
						}
					}
				}

				if (bSelectedStyle)
				{
					rcSel.top() = rc.top();
					rcSel.bottom() = rc.bottom();
					this->GetDialog()->DrawSprite(selection_element, rcSel);
					this->GetDialog()->DrawString(pItem->strText, selection_element, rc);
				}
				else
				{
					this->GetDialog()->DrawString(pItem->strText, main_element, rc);
				}

				rc += int2(0, text_height_);
			}
		}

		// Render the scroll bar
		scroll_bar_.Render();
	}
}
