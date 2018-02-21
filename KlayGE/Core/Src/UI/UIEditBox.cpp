#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/Font.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UniBuffer::UniBuffer(int nInitialSize)
	{
		analyse_required_ = true;

		buffer_.reserve(nInitialSize);
	}

	wchar_t& UniBuffer::operator[](size_t n)  // No param checking
	{
		// This version of operator[] is called only
		// if we are asking for write access, so
		// re-analysis is required.
		analyse_required_ = true;
		return buffer_[n];
	}

	void UniBuffer::Clear()
	{
		buffer_.clear();
		analyse_required_ = true;
	}

	// Inserts the char at specified index.
	void UniBuffer::InsertChar(size_t index, wchar_t wChar)
	{
		BOOST_ASSERT(index <= buffer_.size());

		buffer_.insert(index, 1, wChar);
		analyse_required_ = true;
	}

	// Removes the char at specified index.
	void UniBuffer::RemoveChar(size_t index)
	{
		BOOST_ASSERT(index <= buffer_.size());

		buffer_.erase(index, 1);
		analyse_required_ = true;
	}

	// Inserts the first nCount characters of the string pStr at specified index.
	void UniBuffer::InsertString(size_t index, std::wstring const & str)
	{
		BOOST_ASSERT(index <= buffer_.size());

		buffer_.insert(index, str);
		analyse_required_ = true;
	}

	void UniBuffer::SetText(std::wstring const & strText)
	{
		buffer_ = strText;
		analyse_required_ = true;
	}

	// Uniscribe -- Analyse() analyses the string in the buffer
	bool UniBuffer::Analyse()
	{
		if (font_)
		{
			char_width_.resize(buffer_.size() + 1);

			UISize size;

			for (size_t i = 0; i < char_width_.size(); ++ i)
			{
				std::wstring str = buffer_.substr(0, i);
				char_width_[i] = static_cast<uint32_t>(font_->CalcSize(str, font_size_).cx());
			}

			analyse_required_ = false;

			return true;
		}
		else
		{
			return false;
		}
	}

	int UniBuffer::CPtoX(int nCP, bool bTrail)
	{
		if (bTrail)
		{
			nCP = this->GetNextItemPos(nCP);
		}

		bool succeed = true;
		if (analyse_required_)
		{
			succeed = this->Analyse();
		}

		if (succeed)
		{
			return char_width_[nCP];
		}
		else
		{
			return 0;
		}
	}

	int UniBuffer::XtoCP(int nX, bool& bTrail)
	{
		int CP = 0;
		bTrail = false;  // Default

		bool succeed = true;
		if (analyse_required_)
		{
			succeed = this->Analyse();
		}

		if (succeed)
		{
			while ((CP < static_cast<int>(char_width_.size())) && (nX > char_width_[CP]))
			{
				++ CP;
			}
		}

		// If the coordinate falls outside the text region, we
		// can get character positions that don't exist.  We must
		// filter them here and convert them to those that do exist.
		if ((-1 == CP) && bTrail)
		{
			CP = 0;
			bTrail = false;
		}
		else
		{
			if ((CP >= static_cast<int>(buffer_.size())) && !bTrail)
			{
				CP = static_cast<int>(buffer_.size());
				bTrail = true;
			}
		}

		return CP;
	}

	int UniBuffer::GetPriorItemPos(int nCP) const
	{
		return std::max(nCP - 1, 0);
	}

	int UniBuffer::GetNextItemPos(int nCP) const
	{
		return std::min(nCP + 1, static_cast<int>(buffer_.size() - 1));
	}


	// Static member initialization
	bool UIEditBox::hide_caret_;   // If true, we don't render the caret.
	Timer UIEditBox::timer_;

	UIEditBox::UIEditBox(UIDialogPtr const & dialog)
					: UIControl(UIEditBox::Type, dialog),
						border_(5),	// Default border width
						spacing_(4),	// Default spacing
#ifdef KLAYGE_PLATFORM_WINDOWS
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
						blink_time_(GetCaretBlinkTime() * 0.001f),
#else
						blink_time_(1),
#endif
#else
						blink_time_(1),
#endif
						last_blink_time_(timer_.current_time()),
						caret_on_(true),
						caret_pos_(0),
						insert_mode_(true),
						sel_start_(0),
						first_visible_(0),
						text_color_(16.0f / 255, 16.0f / 255, 16.0f / 255, 1),
						sel_text_color_(1, 1, 1, 1),
						sel_bk_color_(40.0f / 255, 50.0f / 255, 92.0f / 255, 1),
						caret_color_(0, 0, 0, 1),
						mouse_drag_(false)
	{
		hide_caret_ = false;

		this->InitDefaultElements();
	}

	UIEditBox::UIEditBox(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						border_(5),	// Default border width
						spacing_(4),	// Default spacing
#ifdef KLAYGE_PLATFORM_WINDOWS
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
						blink_time_(GetCaretBlinkTime() * 0.001f),
#else
						blink_time_(1),
#endif
#else
						blink_time_(1),
#endif
						last_blink_time_(timer_.current_time()),
						caret_on_(true),
						caret_pos_(0),
						insert_mode_(true),
						sel_start_(0),
						first_visible_(0),
						text_color_(16.0f / 255, 16.0f / 255, 16.0f / 255, 1),
						sel_text_color_(1, 1, 1, 1),
						sel_bk_color_(40.0f / 255, 50.0f / 255, 92.0f / 255, 1),
						caret_color_(0, 0, 0, 1),
						mouse_drag_(false)
	{
		hide_caret_ = false;

		this->InitDefaultElements();
	}

	UIEditBox::UIEditBox(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, bool bIsDefault)
					: UIControl(UIEditBox::Type, dialog),
						border_(5),	// Default border width
						spacing_(4),	// Default spacing
#ifdef KLAYGE_PLATFORM_WINDOWS
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
						blink_time_(GetCaretBlinkTime() * 0.001f),
#else
						blink_time_(1),
#endif
#else
						blink_time_(1),
#endif
						last_blink_time_(timer_.current_time()),
						caret_on_(true),
						caret_pos_(0),
						insert_mode_(true),
						sel_start_(0),
						first_visible_(0),
						text_color_(16.0f / 255, 16.0f / 255, 16.0f / 255, 1),
						sel_text_color_(1, 1, 1, 1),
						sel_bk_color_(40.0f / 255, 50.0f / 255, 92.0f / 255, 1),
						caret_color_(0, 0, 0, 1),
						mouse_drag_(false)
	{
		hide_caret_ = false;

		this->InitDefaultElements();

		// Set the ID and position
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetIsDefault(bIsDefault);

		this->SetText(strText);
	}

	void UIEditBox::InitDefaultElements()
	{
		UIElement Element;

		// Element assignment:
		//   0 - text area
		//   1 - top left border
		//   2 - top border
		//   3 - top right border
		//   4 - left border
		//   5 - right border
		//   6 - lower left border
		//   7 - lower border
		//   8 - lower right border

		Element.SetFont(0, Color(0, 0, 0, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);

		// Assign the style
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 0));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 1));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 2));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 3));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 4));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 5));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 6));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 7));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_EditBox, 8));

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
		on_char_connect_ = main_wnd->OnChar().connect(
			[this](Window const & win, wchar_t ch)
			{
				this->CharHandler(win, ch);
			});
	}

	UIEditBox::~UIEditBox()
	{
		on_char_connect_.disconnect();
	}

	// PlaceCaret: Set the caret to a character position, and adjust the scrolling if
	//             necessary.
	void UIEditBox::PlaceCaret(int nCP)
	{
		BOOST_ASSERT((nCP >= 0) && (nCP <= static_cast<int>(buffer_.GetTextSize())));
		caret_pos_ = nCP;

		// Obtain the X offset of the character.
		int nX2;
		int nX1st = buffer_.CPtoX(first_visible_, false);  // 1st visible char
		int nX = buffer_.CPtoX(nCP, false);  // LEAD
		// If nCP is the nullptr terminator, get the leading edge instead of trailing.
		if (nCP == static_cast<int>(buffer_.GetTextSize()))
		{
			nX2 = nX;
		}
		else
		{
			nX2 = buffer_.CPtoX(nCP, true);  // TRAIL
		}

		// If the left edge of the char is smaller than the left edge of the 1st visible char,
		// we need to scroll left until this char is visible.
		if (nX < nX1st)
		{
			// Simply make the first visible character the char at the new caret position.
			first_visible_ = nCP;
		}
		else
		{
			// If the right of the character is bigger than the offset of the control's
			// right edge, we need to scroll right to this character.
			if (nX2 > nX1st + text_rc_.Width())
			{
				// Compute the X of the new left-most pixel
				int nXNewLeft = nX2 - text_rc_.Width();

				// Compute the char position of this character
				bool bNewTrail;
				int nCPNew1st = buffer_.XtoCP(nXNewLeft, bNewTrail);

				// If this coordinate is not on a character border,
				// start from the next character so that the caret
				// position does not fall outside the text rectangle.
				int nXNew1st = buffer_.CPtoX(nCPNew1st, false);
				if (nXNew1st < nXNewLeft)
				{
					++ nCPNew1st;
				}

				first_visible_ = nCPNew1st;
			}
		}
	}

	void UIEditBox::ClearText()
	{
		buffer_.Clear();
		first_visible_ = 0;
		this->PlaceCaret(0);
		sel_start_ = 0;
	}

	void UIEditBox::SetText(std::wstring const & wszText, bool bSelected)
	{
		buffer_.SetText(wszText);
		first_visible_ = 0;
		// Move the caret to the end of the text
		this->PlaceCaret(buffer_.GetTextSize());
		sel_start_ = bSelected ? 0 : caret_pos_;
	}

	void UIEditBox::DeleteSelectionText()
	{
		int nFirst = std::min(caret_pos_, sel_start_);
		int nLast = std::max(caret_pos_, sel_start_);
		// Update caret and selection
		this->PlaceCaret(nFirst);
		sel_start_ = caret_pos_;
		// Remove the characters
		for (int i = nFirst; i < nLast; ++ i)
		{
			buffer_.RemoveChar(nFirst);
		}
	}

	void UIEditBox::UpdateRects()
	{
		UIControl::UpdateRects();

		// Update the text rectangle
		text_rc_ = bounding_box_;
		// First inflate by border_ to compute render rects
		text_rc_ += IRect(border_, border_, -border_, -border_);

		// Update the render rectangles
		render_rc_[0] = text_rc_;
		render_rc_[1] = IRect(bounding_box_.left(), bounding_box_.top(), text_rc_.left(), text_rc_.top());
		render_rc_[2] = IRect(text_rc_.left(), bounding_box_.top(), text_rc_.right(), text_rc_.top());
		render_rc_[3] = IRect(text_rc_.right(), bounding_box_.top(), bounding_box_.right(), text_rc_.top());
		render_rc_[4] = IRect(bounding_box_.left(), text_rc_.top(), text_rc_.left(), text_rc_.bottom());
		render_rc_[5] = IRect(text_rc_.right(), text_rc_.top(), bounding_box_.right(), text_rc_.bottom());
		render_rc_[6] = IRect(bounding_box_.left(), text_rc_.bottom(), text_rc_.left(), bounding_box_.bottom());
		render_rc_[7] = IRect(text_rc_.left(), text_rc_.bottom(), text_rc_.right(), bounding_box_.bottom());
		render_rc_[8] = IRect(text_rc_.right(), text_rc_.bottom(), bounding_box_.right(), bounding_box_.bottom());

		// Inflate further by spacing_
		text_rc_ += IRect(spacing_, spacing_, -spacing_, -spacing_);

		bounding_box_ = text_rc_;
		for (int i = 0; i < 9; ++ i)
		{
			bounding_box_ |= render_rc_[i];
		}
	}

	void UIEditBox::OnFocusIn()
	{
		UIControl::OnFocusIn();

		this->ResetCaretBlink();
	}

	void UIEditBox::CopyToClipboard()
	{
	}

	void UIEditBox::PasteFromClipboard()
	{
	}

	void UIEditBox::KeyDownHandler(UIDialog const & /*sender*/, uint32_t key)
	{
		switch (key & 0xFF)
		{
		case KS_Tab:
			// We don't process Tab in case keyboard input is enabled and the user
			// wishes to Tab to other controls.
			break;

		case KS_Home:
			this->PlaceCaret(0);
			if (!(key & MB_Shift))
			{
				// Shift is not down. Update selection
				// start along with the caret.
				sel_start_ = caret_pos_;
			}
			this->ResetCaretBlink();
			break;

		case KS_End:
			this->PlaceCaret(buffer_.GetTextSize());
			if (!(key & MB_Shift))
			{
				// Shift is not down. Update selection
				// start along with the caret.
				sel_start_ = caret_pos_;
			}
			this->ResetCaretBlink();
			break;

		case KS_Insert:
			if (key & MB_Ctrl)
			{
				// Control Insert. Copy to clipboard
				this->CopyToClipboard();
			}
			else
			{
				if (key & MB_Shift)
				{
					// Shift Insert. Paste from clipboard
					this->PasteFromClipboard();
				}
				else
				{
					// Toggle caret insert mode
					insert_mode_ = !insert_mode_;
				}
			}
			break;

		case KS_Delete:
			// Check if there is a text selection.
			if (caret_pos_ != sel_start_)
			{
				this->DeleteSelectionText();
				this->OnChangedEvent()(*this);
			}
			else
			{
				// Deleting one character
				buffer_.RemoveChar(caret_pos_);
				this->OnChangedEvent()(*this);
			}
			this->ResetCaretBlink();
			break;

		case KS_LeftArrow:
			if (key & MB_Ctrl)
			{
				// Control is down. Move the caret to a new item
				// instead of a character.
				caret_pos_ = buffer_.GetPriorItemPos(caret_pos_);
				this->PlaceCaret(caret_pos_);
			}
			else
			{
				if (caret_pos_ > 0)
				{
					this->PlaceCaret(caret_pos_ - 1);
				}
			}
			if (!(key & MB_Shift))
			{
				// Shift is not down. Update selection
				// start along with the caret.
				sel_start_ = caret_pos_;
			}
			this->ResetCaretBlink();
			break;

		case KS_RightArrow:
			if (key & MB_Ctrl)
			{
				// Control is down. Move the caret to a new item
				// instead of a character.
				caret_pos_ = buffer_.GetNextItemPos(caret_pos_);
				this->PlaceCaret(caret_pos_);
			}
			else
			{
				if (caret_pos_ < static_cast<int>(buffer_.GetTextSize()))
				{
					this->PlaceCaret(caret_pos_ + 1);
				}
			}
			if (!(key & MB_Shift))
			{
				// Shift is not down. Update selection
				// start along with the caret.
				sel_start_ = caret_pos_;
			}
			this->ResetCaretBlink();
			break;

		case KS_UpArrow:
		case KS_DownArrow:
			// Trap up and down arrows so that the dialog
			// does not switch focus to another control.
			break;
		}
	}

	void UIEditBox::MouseOverHandler(UIDialog const & /*sender*/, uint32_t /*buttons*/, int2 const & pt)
	{
		if (mouse_drag_)
		{
			// Determine the character corresponding to the coordinates.
			int nX1st = buffer_.CPtoX(first_visible_, false);  // X offset of the 1st visible char
			bool bTrail;
			int nCP = buffer_.XtoCP(pt.x() - text_rc_.left() + nX1st, bTrail);
			// Cap at the nullptr character.
			if (bTrail && (nCP < static_cast<int>(buffer_.GetTextSize())))
			{
				this->PlaceCaret(nCP + 1);
			}
			else
			{
				this->PlaceCaret(nCP);
			}
		}
	}

	void UIEditBox::CharHandler(Window const & /*win*/, wchar_t ch)
	{
#ifdef KLAYGE_PLATFORM_WINDOWS
		if (has_focus_)
		{
			switch (ch)
			{
				// Backspace
			case VK_BACK:
				// If there's a selection, treat this
				// like a delete key.
				if (caret_pos_ != sel_start_)
				{
					this->DeleteSelectionText();
					this->OnChangedEvent()(*this);
				}
				else
				{
					if (caret_pos_ > 0)
					{
						// Move the caret, then delete the char.
						this->PlaceCaret(caret_pos_ - 1);
						sel_start_ = caret_pos_;
						buffer_.RemoveChar(caret_pos_);
						this->OnChangedEvent()(*this);
					}
					this->ResetCaretBlink();
				}
				break;

			case 24:        // Ctrl-X Cut
			case VK_CANCEL: // Ctrl-C Copy
				this->CopyToClipboard();

				// If the key is Ctrl-X, delete the selection too.
				if (24 == ch)
				{
					this->DeleteSelectionText();
					this->OnChangedEvent()(*this);
				}

				break;

				// Ctrl-V Paste
			case 22:
				this->PasteFromClipboard();
				this->OnChangedEvent()(*this);
				break;

				// Ctrl-A Select All
			case 1:
				if (sel_start_ == caret_pos_)
				{
					sel_start_ = 0;
					this->PlaceCaret(buffer_.GetTextSize());
				}
				break;

			case VK_RETURN:
				// Invoke the callback when the user presses Enter.
				this->OnStringEvent()(*this);
				break;

				// Junk characters we don't want in the string
			case 26:  // Ctrl 'Z'
			case 2:   // Ctrl 'B'
			case 14:  // Ctrl 'N'
			case 19:  // Ctrl 'S'
			case 4:   // Ctrl 'D'
			case 6:   // Ctrl 'F'
			case 7:   // Ctrl 'G'
			case 10:  // Ctrl 'J'
			case 11:  // Ctrl 'K'
			case 12:  // Ctrl 'L'
			case 17:  // Ctrl 'Q'
			case 23:  // Ctrl 'W'
			case 5:   // Ctrl 'E'
			case 18:  // Ctrl 'R'
			case 20:  // Ctrl 'T'
			case 25:  // Ctrl 'Y'
			case 21:  // Ctrl 'U'
			case 9:   // Ctrl 'I'
			case 15:  // Ctrl 'O'
			case 16:  // Ctrl 'P'
			case 27:  // Ctrl '['
			case 29:  // Ctrl ']'
			case 28:  // Ctrl '\'
				break;

			default:
				// If there's a selection and the user
				// starts to type, the selection should
				// be deleted.
				if (caret_pos_ != sel_start_)
				{
					this->DeleteSelectionText();
				}

				// If we are in overwrite mode and there is already
				// a char at the caret's position, simply replace it.
				// Otherwise, we insert the char as normal.
				if (!insert_mode_ && (caret_pos_ < static_cast<int>(buffer_.GetTextSize())))
				{
					buffer_[caret_pos_] = ch;
					this->PlaceCaret(caret_pos_ + 1);
					sel_start_ = caret_pos_;
				}
				else
				{
					// Insert the char
					buffer_.InsertChar(caret_pos_, ch);
					this->PlaceCaret(caret_pos_ + 1);
					sel_start_ = caret_pos_;
				}
				this->ResetCaretBlink();
				this->OnChangedEvent()(*this);
				break;
			}
		}
#else
		KFL_UNUSED(ch);
#endif
	}

	void UIEditBox::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			if (!has_focus_)
			{
				this->GetDialog()->RequestFocus(*this);
			}

			if (this->ContainsPoint(pt))
			{
				mouse_drag_ = true;

				// Determine the character corresponding to the coordinates.
				int nX1st = buffer_.CPtoX(first_visible_, false);  // X offset of the 1st visible char
				bool bTrail;
				int nCP = buffer_.XtoCP(pt.x() - text_rc_.left() + nX1st, bTrail);

				// Cap at the nullptr character.
				if (bTrail && (nCP < static_cast<int>(buffer_.GetTextSize())))
				{
					this->PlaceCaret(nCP + 1);
				}
				else
				{
					this->PlaceCaret(nCP);
				}
				sel_start_ = caret_pos_;
				this->ResetCaretBlink();
			}
		}
	}

	void UIEditBox::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & /*pt*/)
	{
		if (buttons & MB_Left)
		{
			mouse_drag_ = false;
		}
	}

	void UIEditBox::ResetCaretBlink()
	{
		caret_on_ = true;
		last_blink_time_ = timer_.current_time();
	}

	void UIEditBox::Render()
	{
		auto text_element = elements_[0].get();
		if (text_element)
		{
			buffer_.SetFont(this->GetDialog()->GetFont(text_element->FontIndex()),
				this->GetDialog()->GetFontSize(text_element->FontIndex()));
			this->PlaceCaret(caret_pos_);  // Call PlaceCaret now that we have the font info (node),
			// so that scrolling can be handled.
		}

		// Render the control graphics
		for (int i = 0; i < 9; ++ i)
		{
			auto& element = *elements_[i];
			if (this->GetEnabled())
			{
				element.TextureColor().SetState(UICS_Normal);
			}
			else
			{
				element.TextureColor().SetState(UICS_Disabled);
			}

			this->GetDialog()->DrawSprite(element, render_rc_[i]);
		}

		//
		// Compute the X coordinates of the first visible character.
		//
		int nXFirst = buffer_.CPtoX(first_visible_, false);

		//
		// Compute the X coordinates of the selection rectangle
		//
		int nSelStartX = 0;  // Left and right X cordinates of the selection region
		int nCaretX = buffer_.CPtoX(caret_pos_, false);
		if (caret_pos_ != sel_start_)
		{
			nSelStartX = buffer_.CPtoX(sel_start_, false);
		}
		else
		{
			nSelStartX = nCaretX;
		}

		//
		// Render the selection rectangle
		//
		IRect rcSelection;  // Make this available for rendering selected text
		if (caret_pos_ != sel_start_)
		{
			int nSelLeftX = nCaretX, nSelRightX = nSelStartX;
			// Swap if left is bigger than right
			if (nSelLeftX > nSelRightX)
			{
				std::swap(nSelLeftX, nSelRightX);
			}

			rcSelection = IRect(nSelLeftX, text_rc_.top(), nSelRightX, text_rc_.bottom());
			rcSelection += int2(text_rc_.left() - nXFirst, 0);
			rcSelection &= text_rc_;

			this->GetDialog()->DrawRect(rcSelection, -0.005f, sel_bk_color_);
		}

		//
		// Render the text
		//
		// Element 0 for text
		elements_[0]->FontColor().Current = text_color_;
		this->GetDialog()->DrawString(buffer_.GetBuffer().substr(first_visible_), *elements_[0], text_rc_);

		// Render the selected text
		if (caret_pos_ != sel_start_)
		{
			int nFirstToRender = std::max(first_visible_, std::min(sel_start_, caret_pos_));
			int nNumChatToRender = std::max(sel_start_, caret_pos_) - nFirstToRender;
			elements_[0]->FontColor().Current = sel_text_color_;
			this->GetDialog()->DrawString(buffer_.GetBuffer().substr(nFirstToRender, nNumChatToRender),
				*elements_[0], rcSelection);
		}

		//
		// Blink the caret
		//
		if (timer_.current_time() - last_blink_time_ >= blink_time_)
		{
			caret_on_ = !caret_on_;
			last_blink_time_ = timer_.current_time();
		}

		//
		// Render the caret if this control has the focus
		//
		if (has_focus_ && caret_on_ && !hide_caret_)
		{
			// Start the rectangle with insert mode caret
			IRect rcCaret(text_rc_.left() - nXFirst + nCaretX - 1, text_rc_.top(),
				text_rc_.left() - nXFirst + nCaretX + 1, text_rc_.bottom());

			// If we are in overwrite mode, adjust the caret rectangle
			// to fill the entire character.
			if (!insert_mode_)
			{
				// Obtain the right edge X coord of the current character
				int nRightEdgeX = buffer_.CPtoX(caret_pos_, true);
				rcCaret.right() = text_rc_.left() - nXFirst + nRightEdgeX;
			}

			this->GetDialog()->DrawRect(rcCaret, -0.1f, caret_color_);
		}
	}
}
