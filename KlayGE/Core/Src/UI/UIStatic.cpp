// UIStatic.cpp
// KlayGE 图形用户界面静态框类 实现文件
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

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIStatic::UIStatic(UIDialogPtr dialog)
					: UIControl(UIStatic::Type, dialog)
	{
		this->InitDefaultElements();
	}

	UIStatic::UIStatic(uint32_t type, UIDialogPtr dialog)
					: UIControl(type, dialog)
	{
		this->InitDefaultElements();
	}

	UIStatic::UIStatic(UIDialogPtr dialog, int ID, std::wstring const & strText, int x, int y, int width, int height, bool bIsDefault)
					: UIControl(UIStatic::Type, dialog),
						text_(strText)
	{
		this->InitDefaultElements();

		// Set the ID and list index
		this->SetID(ID); 
		this->SetLocation(x, y);
		this->SetSize(width, height);
		this->SetIsDefault(bIsDefault);
	}

	void UIStatic::InitDefaultElements()
	{
		UIElement Element;

		{
			Element.SetFont(0);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}
	}

	void UIStatic::Render()
	{    
		if (!visible_)
		{
			return;
		}

		UI_Control_State iState = UICS_Normal;

		if (!enabled_)
		{
			iState = UICS_Disabled;
		}

		UIElement& element = *elements_[0];
		element.FontColor().SetState(iState);
		this->GetDialog()->DrawText(text_, element, bounding_box_);
	}

	void UIStatic::SetText(std::wstring const & strText)
	{
		text_ = strText;
	}
}
