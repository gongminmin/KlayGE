// UIStatic.cpp
// KlayGE 图形用户界面静态框类 实现文件
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
#include <KlayGE/Font.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	UIStatic::UIStatic(UIDialogPtr const & dialog)
					: UIStatic(UIStatic::Type, dialog)
	{
	}

	UIStatic::UIStatic(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog)
	{
		UIElement Element;

		{
			Element.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Top);
			Element.FontColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 200.0f / 255);

			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}
	}

	UIStatic::UIStatic(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, bool bIsDefault)
					: UIStatic(dialog)
	{
		this->SetText(strText);

		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetIsDefault(bIsDefault);
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
		this->GetDialog()->DrawString(text_, element, bounding_box_);
	}

	void UIStatic::SetText(std::wstring const & strText)
	{
		text_ = strText;
	}
}
