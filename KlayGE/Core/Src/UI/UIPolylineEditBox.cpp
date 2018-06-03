// UIPolylineEditBox.cpp
// KlayGE 图形用户界面折线编辑框类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 初次建立 (2009.4.18)
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
	UIPolylineEditBox::UIPolylineEditBox(UIDialogPtr const & dialog)
					: UIPolylineEditBox(UIPolylineEditBox::Type, dialog)
	{
	}

	UIPolylineEditBox::UIPolylineEditBox(uint32_t type, UIDialogPtr const & dialog)
					: UIControl(type, dialog),
						active_pt_(-1),
						move_point_(false)
	{
		hotkey_ = 0;

		UIElement Element;

		// Background
		{
			Element.TextureColor().States[UICS_Normal] = Color(0.7f, 0.7f, 0.7f, 1.0f);
			Element.TextureColor().SetState(UICS_Normal);
			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Coord line
		{
			Element.TextureColor().States[UICS_Normal] = Color(0.6f, 0.6f, 0.6f, 1.0f);
			Element.TextureColor().SetState(UICS_Normal);
			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Polyline
		{
			Element.TextureColor().States[UICS_Normal] = Color(0, 1, 0, 1);
			Element.TextureColor().States[UICS_MouseOver] = Color(1, 0, 0, 1);
			Element.TextureColor().SetState(UICS_Normal);
			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}

		// Control points
		{
			Element.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 1);
			Element.TextureColor().States[UICS_MouseOver] = Color(1, 0, 0, 1);
			Element.TextureColor().SetState(UICS_Normal);
			elements_.push_back(MakeUniquePtr<UIElement>(Element));
		}
	}

	UIPolylineEditBox::UIPolylineEditBox(UIDialogPtr const & dialog, int ID, int4 const & coord_size, uint8_t hotkey, bool bIsDefault)
					: UIPolylineEditBox(dialog)
	{
		// Set the ID and list index
		this->SetID(ID);
		this->SetLocation(coord_size.x(), coord_size.y());
		this->SetSize(coord_size.z(), coord_size.w());
		this->SetHotkey(hotkey);
		this->SetIsDefault(bIsDefault);
	}

	void UIPolylineEditBox::ActivePoint(int index)
	{
		BOOST_ASSERT(index < static_cast<int>(ctrl_points_.size()));
		active_pt_ = index;
	}
	
	int UIPolylineEditBox::ActivePoint() const
	{
		return active_pt_;
	}

	void UIPolylineEditBox::ClearCtrlPoints()
	{
		active_pt_ = -1;
		ctrl_points_.clear();
		move_point_ = false;
	}

	int UIPolylineEditBox::AddCtrlPoint(float pos, float value)
	{
		pos = MathLib::clamp(pos, 0.0f, 1.0f);
		value = MathLib::clamp(pos, 0.0f, 1.0f);

		int index;
		if (ctrl_points_.size() >= 1)
		{
			auto iter = ctrl_points_.begin();
			while ((iter != ctrl_points_.end() - 1) && ((iter + 1)->x() < pos))
			{
				++ iter;
			}
			index = static_cast<int>(iter + 1 - ctrl_points_.begin());
			if ((iter + 1 == ctrl_points_.end()) || (MathLib::abs((iter + 1)->x() - pos) > 0.05f))
			{
				ctrl_points_.insert(iter + 1, float2(pos, value));
			}
		}
		else
		{
			index = 0;
			ctrl_points_.push_back(float2(pos, value));
		}
		this->ActivePoint(index);

		return index;
	}

	int UIPolylineEditBox::AddCtrlPoint(float pos)
	{
		pos = MathLib::clamp(pos, 0.0f, 1.0f);
		float value = this->GetValue(pos);

		return this->AddCtrlPoint(pos, value);
	}

	void UIPolylineEditBox::DelCtrlPoint(int index)
	{
		if (active_pt_ == index)
		{
			active_pt_ = -1;
		}

		ctrl_points_.erase(ctrl_points_.begin() + index);
	}

	void UIPolylineEditBox::SetCtrlPoint(int index, float pos, float value)
	{
		ctrl_points_[index] = float2(pos, value);
	}

	void UIPolylineEditBox::SetCtrlPoints(std::vector<float2> const & ctrl_points)
	{
		ctrl_points_ = ctrl_points;
	}

	void UIPolylineEditBox::SetColor(Color const & clr)
	{
		elements_[POLYLINE_INDEX]->TextureColor().States[UICS_Normal] = clr;
	}

	size_t UIPolylineEditBox::NumCtrlPoints() const
	{
		return ctrl_points_.size();
	}

	float2 const & UIPolylineEditBox::GetCtrlPoint(size_t i) const
	{
		return *(ctrl_points_.begin() + i);
	}

	std::vector<float2> const & UIPolylineEditBox::GetCtrlPoints() const
	{
		return ctrl_points_;
	}

	float2 UIPolylineEditBox::PtFromCoord(int x, int y) const
	{
		return float2(static_cast<float>(x - bounding_box_.left()) / bounding_box_.Width(),
			static_cast<float>(bounding_box_.bottom() - 1 - y) / bounding_box_.Height());
	}

	float UIPolylineEditBox::GetValue(float pos) const
	{
		for (auto iter = ctrl_points_.begin(); iter != ctrl_points_.end() - 1; ++ iter)
		{
			if ((iter + 1)->x() >= pos)
			{
				float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
				return iter->y() + ((iter + 1)->y() - iter->y()) * s;
			}
		}
		return -1;
	}

	void UIPolylineEditBox::KeyDownHandler(UIDialog const & /*sender*/, uint32_t /*key*/)
	{
	}

	void UIPolylineEditBox::KeyUpHandler(UIDialog const & /*sender*/, uint32_t /*key*/)
	{
	}

	void UIPolylineEditBox::MouseDownHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			if (!has_focus_)
			{
				this->GetDialog()->RequestFocus(*this);
			}

			if (this->ContainsPoint(pt))
			{
				float2 p = this->PtFromCoord(pt.x(), pt.y());
				if (MathLib::abs(this->GetValue(p.x()) - p.y()) < 0.1f)
				{
					bool found = false;
					for (int i = 0; i < static_cast<int>(this->NumCtrlPoints()); ++ i)
					{
						float2 cp = this->GetCtrlPoint(i);
						cp = p - cp;
						if (MathLib::dot(cp, cp) < 0.01f)
						{
							this->ActivePoint(i);
							move_point_ = true;
							found = true;
							break;
						}
					}

					if (!found)
					{
						this->AddCtrlPoint(p.x());
						move_point_ = true;
					}
				}
			}
		}
	}

	void UIPolylineEditBox::MouseUpHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (buttons & MB_Left)
		{
			if (!this->GetDialog()->IsKeyboardInputEnabled())
			{
				this->GetDialog()->ClearFocus();
			}

			if (this->ContainsPoint(pt))
			{
				if (move_point_)
				{
					if ((this->ActivePoint() != 0) && (this->ActivePoint() != static_cast<int>(this->NumCtrlPoints() - 1)))
					{
						float2 p = this->PtFromCoord(pt.x(), pt.y());
						if ((p.x() < 0) || (p.x() > 1) || (p.y() < 0) || (p.y() > 1))
						{
							this->DelCtrlPoint(this->ActivePoint());
						}
					}

					move_point_ = false;
				}
			}
		}
	}

	void UIPolylineEditBox::MouseOverHandler(UIDialog const & /*sender*/, uint32_t buttons, int2 const & pt)
	{
		if (move_point_ || this->ContainsPoint(pt))
		{
			if (0 == buttons)
			{
				float2 p = this->PtFromCoord(pt.x(), pt.y());
				if (MathLib::abs(this->GetValue(p.x()) - p.y()) < 0.1f)
				{
					elements_[POLYLINE_INDEX]->TextureColor().SetState(UICS_MouseOver);
				}
				else
				{
					elements_[POLYLINE_INDEX]->TextureColor().SetState(UICS_Normal);
				}

				int move_over_pt = -1;
				for (int i = 0; i < static_cast<int>(this->NumCtrlPoints()); ++ i)
				{
					float2 cp = this->GetCtrlPoint(i);
					cp = p - cp;
					if (MathLib::dot(cp, cp) < 0.01f)
					{
						move_over_pt = i;
						break;
					}
				}
				if (move_over_pt != -1)
				{
					active_pt_ = move_over_pt;
				}
			}
			else
			{
				if (buttons & MB_Left)
				{
					if (move_point_)
					{
						float2 p = this->PtFromCoord(pt.x(), pt.y());
						if (0 == this->ActivePoint())
						{
							p.x() = 0;
							p.y() = MathLib::clamp(p.y(), 0.0f, 1.0f);
						}
						else
						{
							if (static_cast<int>(this->NumCtrlPoints() - 1) == this->ActivePoint())
							{
								p.x() = 1;
								p.y() = MathLib::clamp(p.y(), 0.0f, 1.0f);
							}
						}
						if (this->ActivePoint() < static_cast<int>(this->NumCtrlPoints() - 1))
						{
							float2 next_p = this->GetCtrlPoint(this->ActivePoint() + 1);
							if (next_p.x() <= p.x())
							{
								p.x() = next_p.x();
							}
						}
						this->SetCtrlPoint(this->ActivePoint(), p.x(), p.y());
					}
				}
			}
		}
	}

	void UIPolylineEditBox::Render()
	{
		if (visible_)
		{
			UIDialogPtr dlg = this->GetDialog();

			dlg->DrawRect(bounding_box_, 0, elements_[BACKGROUND_INDEX]->TextureColor().Current);

			for (size_t i = 0; i < 21; ++ i)
			{
				int32_t x = static_cast<int32_t>(bounding_box_.left() + bounding_box_.Width() * i / 20.0f);
				int32_t offset = 0;
				if (10 == i)
				{
					offset = -1;
				}
				dlg->DrawRect(IRect(x + offset, bounding_box_.top(), x + 1, bounding_box_.bottom()),
					0, elements_[COORDLINE_INDEX]->TextureColor().Current);
			}
			for (size_t i = 0; i < 11; ++ i)
			{
				int32_t y = static_cast<int32_t>(bounding_box_.top() + bounding_box_.Height() * i / 10.0f);
				int32_t offset = 0;
				if (5 == i)
				{
					offset = -1;
				}
				dlg->DrawRect(IRect(bounding_box_.left(), y + offset, bounding_box_.right(), y + 1),
					0, elements_[COORDLINE_INDEX]->TextureColor().Current);
			}

			{
				Color const & clr = elements_[POLYLINE_INDEX]->TextureColor().Current;
				for (size_t i = 0; i < ctrl_points_.size() - 1; ++ i)
				{
					float2 dir = ctrl_points_[i + 1] - ctrl_points_[i + 0];
					dir = MathLib::normalize(float2(dir.y(), dir.x())) / 2.0f;
					float x0 = bounding_box_.left() + ctrl_points_[i + 1].x() * bounding_box_.Width() + dir.x();
					float x1 = bounding_box_.left() + ctrl_points_[i + 0].x() * bounding_box_.Width() + dir.x();
					float x2 = bounding_box_.left() + ctrl_points_[i + 0].x() * bounding_box_.Width() - dir.x();
					float x3 = bounding_box_.left() + ctrl_points_[i + 1].x() * bounding_box_.Width() - dir.x();
					float y0 = bounding_box_.bottom() - 1 - ctrl_points_[i + 1].y() * bounding_box_.Height() + dir.y();
					float y1 = bounding_box_.bottom() - 1 - ctrl_points_[i + 0].y() * bounding_box_.Height() + dir.y();
					float y2 = bounding_box_.bottom() - 1 - ctrl_points_[i + 0].y() * bounding_box_.Height() - dir.y();
					float y3 = bounding_box_.bottom() - 1 - ctrl_points_[i + 1].y() * bounding_box_.Height() - dir.y();
					UIManager::VertexFormat vd[] = 
					{
						UIManager::VertexFormat(float3(x0, y0, 0), clr, float2(0, 0)),
						UIManager::VertexFormat(float3(x1, y1, 0), clr, float2(0, 0)),
						UIManager::VertexFormat(float3(x2, y2, 0), clr, float2(0, 0)),
						UIManager::VertexFormat(float3(x3, y3, 0), clr, float2(0, 0))
					};
					dlg->DrawQuad(vd, 0, TexturePtr());
				}
			}

			{
				Color const & clr_normal = elements_[CTRLPOINTS_INDEX]->TextureColor().Current;
				Color const & clr_active = elements_[CTRLPOINTS_INDEX]->TextureColor().States[UICS_MouseOver];
				for (int i = 0; i < static_cast<int>(ctrl_points_.size()); ++ i)
				{
					int32_t offset = (i == this->ActivePoint()) ? 3 : 2;

					int32_t x0 = bounding_box_.left() + static_cast<int32_t>(ctrl_points_[i].x() * bounding_box_.Width()) - offset;
					int32_t x1 = bounding_box_.left() + static_cast<int32_t>(ctrl_points_[i].x() * bounding_box_.Width()) + offset;
					int32_t y0 = bounding_box_.bottom() - 1 - static_cast<int32_t>(ctrl_points_[i].y() * bounding_box_.Height()) - offset;
					int32_t y1 = bounding_box_.bottom() - 1 - static_cast<int32_t>(ctrl_points_[i].y() * bounding_box_.Height()) + offset;
					dlg->DrawRect(IRect(x0, y0, x1, y1), 0, (i == this->ActivePoint()) ? clr_active : clr_normal);
				}
			}
		}
	}
}
