// UIProgressBar.cpp
// KlayGE ͼ���û������������ ʵ���ļ�
// Ver 3.12.0
// ��Ȩ����(C) ������, 2011
// Homepage: http://www.klayge.org
//
// 3.12.0
// ���ν��� (2011.2.5)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/Input.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIProgressBar::UIProgressBar(UIDialogPtr const & dialog)
					: UIControl(UIPolylineEditBox::Type, dialog),
						progress_(0)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UIProgressBar::UIProgressBar(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						progress_(0)
	{
		hotkey_ = 0;

		this->InitDefaultElements();
	}

	UIProgressBar::UIProgressBar(UIDialogPtr const & dialog, int ID, int progress, int4 const & coord_size, uint8_t hotkey, bool bIsDefault)
					: UIControl(UIProgressBar::Type, dialog),
						progress_(progress)
	{
		this->InitDefaultElements();

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UIProgressBar::InitDefaultElements()
	{
		UIElement Element;

		// Background
		{
			Element.TextureColor().States[UICS_Normal] = Color(1.0f, 1.0f, 1.0f, 1.0f);
			Element.TextureColor().SetState(UICS_Normal);
			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}

		// Bar
		{
			Element.TextureColor().States[UICS_Normal] = Color(0.2f, 0.4f, 0.6f, 1.0f);
			Element.TextureColor().SetState(UICS_Normal);
			elements_.push_back(MakeSharedPtr<UIElement>(Element));
		}
	}

	void UIProgressBar::SetValue(int value)
	{
		progress_ = value;
	}
	
	int UIProgressBar::GetValue() const
	{
		return progress_;
	}

	void UIProgressBar::Render()
	{
		if (visible_)
		{
			UIDialogPtr dlg = this->GetDialog();

			dlg->DrawRect(bounding_box_, 0, elements_[BACKGROUND_INDEX]->TextureColor().Current);

			IRect bar_bb = bounding_box_;
			bar_bb.right() = bounding_box_.left() + static_cast<int>((progress_ / 100.0f) * bounding_box_.Width() + 0.5f);
			dlg->DrawRect(bar_bb, 0, elements_[BAR_INDEX]->TextureColor().Current);
		}
	}
}
