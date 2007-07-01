// UI.cpp
// KlayGE 图形用户界面 实现文件
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
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/Input.hpp>

#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	class UIRectRenderable : public RenderableHelper
	{
	public:
		UIRectRenderable(std::vector<UIManager::VertexFormat> const & vertices, std::vector<uint16_t> const & indices, TexturePtr texture, RenderEffectPtr effect)
			: RenderableHelper(L"UIRect")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static);
			vb->Resize(static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])));
			{
				GraphicsBuffer::Mapper mapper(*vb, BA_Write_Only);
				std::copy(vertices.begin(), vertices.end(), mapper.Pointer<UIManager::VertexFormat>());
			}
			rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F),
												vertex_element(VEU_Diffuse, 0, EF_ABGR32F),
												vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
			ib->Resize(static_cast<uint32_t>(indices.size() * sizeof(indices[0])));
			{
				GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
				std::copy(indices.begin(), indices.end(), mapper.Pointer<uint16_t>());
			}
			rl_->BindIndexStream(ib, EF_R16);

			if (texture)
			{
				technique_ = effect->TechniqueByName("UITec");
				*(technique_->Effect().ParameterByName("texUISampler")) = texture;
			}
			else
			{
				technique_ = effect->TechniqueByName("UITecNoTex");
			}
		}

		void OnRenderBegin()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			*(technique_->Effect().ParameterByName("halfWidth")) = static_cast<int>(re.CurFrameBuffer()->Width() / 2);
			*(technique_->Effect().ParameterByName("halfHeight")) = static_cast<int>(re.CurFrameBuffer()->Height() / 2);
		}

		void Render()
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			this->OnRenderBegin();
			renderEngine.Render(*this->GetRenderTechnique(), *rl_);
			this->OnRenderEnd();
		}
	};

	class UIRectObject : public SceneObjectHelper
	{
	public:
		explicit UIRectObject(RenderablePtr renderable)
			: SceneObjectHelper(renderable, SceneObject::SOA_ShortAge)
		{
		}
	};


	void UIStatesColor::Init(Color const & default_color,
			Color const & disabled_color,
			Color const & hidden_color)
	{
		for (int i = 0; i < UICS_Num_Control_States; ++ i)
		{
			States[i] = default_color;
		}

		States[UICS_Disabled] = disabled_color;
		States[UICS_Hidden] = hidden_color;
		Current = hidden_color;
	}

	void UIStatesColor::SetState(UI_Control_State state)
	{
		Current = States[state];
	}


	void UIElement::SetTexture(uint32_t tex_index, Rect_T<int32_t> const & tex_rect, Color const & default_texture_color)
	{
		tex_index_ = tex_index;
		tex_rect_ = tex_rect;
		texture_color_.Init(default_texture_color);
	}

	void UIElement::SetFont(uint32_t font_index, Color const & default_font_color,
		uint32_t text_align)
	{
		font_index_ = font_index;
		text_align_ = text_align;
		font_color_.Init(default_font_color);
	}
	
	void UIElement::Refresh()
	{
		texture_color_.SetState(UICS_Hidden);
		font_color_.SetState(UICS_Hidden);
	}


	UIManager& UIManager::Instance()
	{
		static UIManager ret;
		return ret;
	}

	UIManager::UIManager()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		effect_ = rf.LoadEffect("UI.kfx");

		elem_texture_rcs_[UICT_Button].push_back(Rect_T<int32_t>(0, 0, 136, 54));
		elem_texture_rcs_[UICT_Button].push_back(Rect_T<int32_t>(136, 0, 252, 54));

		elem_texture_rcs_[UICT_CheckBox].push_back(Rect_T<int32_t>(0, 54, 27, 81));
		elem_texture_rcs_[UICT_CheckBox].push_back(Rect_T<int32_t>(27, 54, 54, 81));

		elem_texture_rcs_[UICT_RadioButton].push_back(Rect_T<int32_t>(54, 54, 81, 81));
		elem_texture_rcs_[UICT_RadioButton].push_back(Rect_T<int32_t>(81, 54, 108, 81));

		elem_texture_rcs_[UICT_Slider].push_back(Rect_T<int32_t>(1, 187, 93, 228));
		elem_texture_rcs_[UICT_Slider].push_back(Rect_T<int32_t>(151, 193, 192, 234));

		elem_texture_rcs_[UICT_ScrollBar].push_back(Rect_T<int32_t>(196, 212, 218, 223));
		elem_texture_rcs_[UICT_ScrollBar].push_back(Rect_T<int32_t>(196, 192, 218, 212));
		elem_texture_rcs_[UICT_ScrollBar].push_back(Rect_T<int32_t>(196, 223, 218, 244));
		elem_texture_rcs_[UICT_ScrollBar].push_back(Rect_T<int32_t>(220, 192, 238, 234));

		elem_texture_rcs_[UICT_ListBox].push_back(Rect_T<int32_t>(13, 123, 241, 160));
		elem_texture_rcs_[UICT_ListBox].push_back(Rect_T<int32_t>(16, 166, 240, 183));

		elem_texture_rcs_[UICT_ComboBox].push_back(Rect_T<int32_t>(7, 81, 247, 123));
		elem_texture_rcs_[UICT_ComboBox].push_back(Rect_T<int32_t>(98, 189, 151, 238));
		elem_texture_rcs_[UICT_ComboBox].push_back(Rect_T<int32_t>(13, 123, 241, 160));
		elem_texture_rcs_[UICT_ComboBox].push_back(Rect_T<int32_t>(12, 163, 239, 183));

		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(14, 90, 241, 113));
		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(8, 82, 14, 90));
		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(14, 82, 241, 90));
		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(241, 82, 246, 90));
		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(8, 90, 14, 113));
		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(241, 90, 246, 113));
		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(8, 113, 14, 121));
		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(14, 113, 241, 121));
		elem_texture_rcs_[UICT_EditBox].push_back(Rect_T<int32_t>(241, 113, 246, 121));
	}

	UIDialogPtr UIManager::MakeDialog(TexturePtr control_tex)
	{
		UIDialogPtr ret(new UIDialog(control_tex));
		this->RegisterDialog(ret);
		return ret;
	}

	bool UIManager::RegisterDialog(UIDialogPtr dialog)
	{
		// Check that the dialog isn't already registered.
		for (size_t i = 0; i < dialogs_.size(); ++ i)
		{
			if (dialogs_[i] == dialog)
			{
				return true;
			}
		}

		// Add to the list.
		dialogs_.push_back(dialog);
		return true;
	}

	void UIManager::UnregisterDialog(UIDialogPtr dialog)
	{
		// Search for the dialog in the list.
		for (size_t i = 0; i < dialogs_.size(); ++ i)
		{
			if (dialogs_[i] == dialog)
			{
				dialogs_.erase(dialogs_.begin() + i);
				return;
			}
		}
	}

	void UIManager::EnableKeyboardInputForAllDialogs()
	{
		// Enable keyboard input for all registered dialogs
		for (size_t i = 0; i < dialogs_.size(); ++ i)
		{
			dialogs_[i]->EnableKeyboardInput(true);
		}
	}

	UIDialogPtr UIManager::GetNextDialog(UIDialogPtr dialog) const
	{
		if (dialog == dialogs_.back())
		{
			return UIDialogPtr();
		}
		else
		{
			for (size_t i = 0; i < dialogs_.size(); ++ i)
			{
				if (dialogs_[i] == dialog)
				{
					return dialogs_[i + 1];
				}
			}

			return UIDialogPtr();
		}
	}

	UIDialogPtr UIManager::GetPrevDialog(UIDialogPtr dialog) const
	{
		if (dialog == dialogs_.front())
		{
			return UIDialogPtr();
		}
		else
		{
			for (size_t i = 0; i < dialogs_.size(); ++ i)
			{
				if (dialogs_[i] == dialog)
				{
					return dialogs_[i - 1];
				}
			}

			return UIDialogPtr();
		}
	}

	void UIManager::Render()
	{
		BOOST_FOREACH(BOOST_TYPEOF(rects_)::reference rect, rects_)
		{
			rect.second.first.clear();
			rect.second.second.clear();
		}
		BOOST_FOREACH(BOOST_TYPEOF(strings_)::reference string, strings_)
		{
			string.second.clear();
		}

		for (size_t i = 0; i < dialogs_.size(); ++ i)
		{
			dialogs_[i]->Render();
		}

		BOOST_FOREACH(BOOST_TYPEOF(rects_)::reference rect, rects_)
		{
			if (!rect.second.first.empty() && !rect.second.second.empty())
			{
				boost::shared_ptr<UIRectObject> rect_obj(new UIRectObject(
					boost::shared_ptr<UIRectRenderable>(new UIRectRenderable(rect.second.first, rect.second.second, rect.first, effect_))));
				rect_obj->AddToSceneManager();
			}
		}
		BOOST_FOREACH(BOOST_TYPEOF(strings_)::reference string, strings_)
		{
			BOOST_FOREACH(BOOST_TYPEOF(string.second)::reference s, string.second)
			{
				string.first->RenderText(s.rc, s.depth, 1, 1, s.clr, s.text, s.align);
			}
		}
	}

	void UIManager::DrawRect(float3 const & pos, float width, float height, Color const * clrs,
				Rect_T<int32_t> const & rcTexture, TexturePtr texture)
	{
		Rect texcoord;
		if (texture)
		{
			texcoord = Rect((rcTexture.left() + 0.5f) / texture->Width(0),
				(rcTexture.top() + 0.5f) / texture->Height(0),
				(rcTexture.right() + 0.5f) / texture->Width(0),
				(rcTexture.bottom() + 0.5f) / texture->Height(0));
		}
		else
		{
			texcoord = Rect(0, 0, 0, 0);
		}

		std::pair<std::vector<UIManager::VertexFormat>, std::vector<uint16_t> >& rect = rects_[texture];
		uint16_t const last_index = static_cast<uint16_t>(rect.first.size());
		rect.second.push_back(last_index + 0);
		rect.second.push_back(last_index + 1);
		rect.second.push_back(last_index + 2);
		rect.second.push_back(last_index + 2);
		rect.second.push_back(last_index + 3);
		rect.second.push_back(last_index + 0);

		rect.first.push_back(UIManager::VertexFormat(pos + float3(0, 0, 0),
			float4(&clrs[0].r()),
			float2(texcoord.left(), texcoord.top())));
		rect.first.push_back(UIManager::VertexFormat(pos + float3(width, 0, 0),
			float4(&clrs[1].r()),
			float2(texcoord.right(), texcoord.top())));
		rect.first.push_back(UIManager::VertexFormat(pos + float3(width, height, 0),
			float4(&clrs[2].r()),
			float2(texcoord.right(), texcoord.bottom())));
		rect.first.push_back(UIManager::VertexFormat(pos + float3(0, height, 0),
			float4(&clrs[3].r()),
			float2(texcoord.left(), texcoord.bottom())));
	}

	void UIManager::DrawText(std::wstring const & strText, uint32_t font_index,
		Rect_T<int32_t> const & rc, float depth, Color const & clr, uint32_t align)
	{
		strings_[font_cache_[font_index]].push_back(string_cache());
		string_cache& sc = strings_[font_cache_[font_index]].back();
		sc.rc = Rect(static_cast<float>(rc.left()),
			static_cast<float>(rc.top()), static_cast<float>(rc.right()), static_cast<float>(rc.bottom()));
		sc.depth = depth;
		sc.clr = clr;
		sc.text = strText;
		sc.align = align;
	}

	void UIManager::HandleInput()
	{
		for (size_t i = 0; i < dialogs_.size(); ++ i)
		{
			if (dialogs_[i]->GetVisible())
			{
				dialogs_[i]->HandleInput();
			}
		}

		InputEngine& ie = Context::Instance().InputFactoryInstance().InputEngineInstance();
		for (uint32_t i = 0; i < ie.NumDevices(); ++ i)
		{
			InputKeyboardPtr key_board = boost::dynamic_pointer_cast<InputKeyboard>(ie.Device(i));
			InputMousePtr mouse = boost::dynamic_pointer_cast<InputMouse>(ie.Device(i));
			if (key_board)
			{
				for (size_t j = 0; j < last_key_states_.size(); ++ j)
				{
					last_key_states_[j] = key_board->Key(j);
				}
			}
			if (mouse)
			{
				for (size_t j = 0; j < last_mouse_states_.size(); ++ j)
				{
					last_mouse_states_[j] = mouse->Button(j);
				}
			}
		}
	}

	Rect_T<int32_t> const & UIManager::ElementTextureRect(uint32_t ctrl, uint32_t elem_index)
	{
		BOOST_ASSERT(ctrl < elem_texture_rcs_.size());
		BOOST_ASSERT(elem_index < elem_texture_rcs_[ctrl].size());

		return elem_texture_rcs_[ctrl][elem_index];
	}

	
	UIDialog::UIDialog(TexturePtr control_tex)
			: x_(0), y_(0), width_(0), height_(0),
					visible_(true), show_caption_(false),
					minimized_(false), drag_(false),
					caption_height_(18),
					top_left_clr_(0, 0, 0, 0), top_right_clr_(0, 0, 0, 0),
					bottom_left_clr_(0, 0, 0, 0), bottom_right_clr_(0, 0, 0, 0),
					default_control_id_(0xFFFF),
					keyboard_input_(false), mouse_input_(true)
	{
		if (!control_tex)
		{
			control_tex = LoadTexture("ui.dds");
		}

		this->SetTexture(0, control_tex);
		this->InitDefaultElements();
	}
	
	UIDialog::~UIDialog()
	{
		this->RemoveAllControls();
	}

	void UIDialog::AddControl(UIControlPtr control)
	{
		this->InitControl(*control);

		// Add to the list
		controls_.push_back(control);
	}
	
	void UIDialog::InitControl(UIControl& control)
	{
		control.SetIndex(static_cast<uint32_t>(controls_.size()));
	}

	UIControlPtr UIDialog::GetControl(int ID) const
	{
		// Try to find the control with the given ID
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr control = controls_[i];
			if (control->GetID() == ID)
			{
				return control;
			}
		}

		// Not found
		return UIControlPtr();
	}
	
	UIControlPtr UIDialog::GetControl(int ID, uint32_t type) const
	{
		// Try to find the control with the given ID
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr control = controls_[i];
			if ((control->GetID() == ID) && (control->GetType() == type))
			{
				return control;
			}
		}

		// Not found
		return UIControlPtr();
	}

	UIControlPtr UIDialog::GetControlAtPoint(Vector_T<int32_t, 2> const & pt) const
	{
		// Search through all child controls for the first one which
		// contains the mouse point
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr control = controls_[i];

			if (!control)
			{
				continue;
			}

			// We only return the current control if it is visible
			// and enabled.  Because GetControlAtPoint() is used to do mouse
			// hittest, it makes sense to perform this filtering.
			if (control->ContainsPoint(pt) && control->GetEnabled() && control->GetVisible())
			{
				return control;
			}
		}

		return UIControlPtr();
	}

	bool UIDialog::GetControlEnabled(int ID) const
	{
		UIControlPtr control = this->GetControl(ID);
		if (!control)
		{
			return false;
		}

		return control->GetEnabled();
	}

	void UIDialog::SetControlEnabled(int ID, bool enabled)
	{
		UIControlPtr control = this->GetControl(ID);
		if (!control)
		{
			return;
		}

		control->SetEnabled(enabled);
	}

	void UIDialog::Render()
	{
		// For invisible dialog, out now.
		if (!visible_ || (minimized_ && !show_caption_))
		{
			return;
		}

		bool bBackgroundIsVisible = (top_left_clr_.a() != 0) || (top_right_clr_.a() != 0)
			|| (bottom_right_clr_.a() != 0) || (bottom_left_clr_.a() != 0);
		if (!minimized_ && bBackgroundIsVisible)
		{
			boost::array<Color, 4> clrs;
			clrs[0] = top_left_clr_;
			clrs[1] = top_right_clr_;
			clrs[2] = bottom_right_clr_;
			clrs[3] = bottom_left_clr_;

			float3 pos(static_cast<float>(x_), static_cast<float>(y_), 0.5f);
			Rect_T<int32_t> rc(0, 0, width_, height_);
			UIManager::Instance().DrawRect(pos, static_cast<float>(width_),
				static_cast<float>(height_), &clrs[0], Rect_T<int32_t>(0, 0, 0, 0), TexturePtr());
		}

		// Render the caption if it's enabled.
		if (show_caption_)
		{
			// DrawSprite will offset the rect down by
			// caption_height_, so adjust the rect higher
			// here to negate the effect.
			Rect_T<int32_t> rc(0, -caption_height_, width_, 0);
			this->DrawSprite(cap_element_, rc);
			rc.left() += 5; // Make a left margin
			std::wstring wstrOutput = caption_;
			if (minimized_)
			{
				wstrOutput += L" (Minimized)";
			}
			this->DrawText(wstrOutput, cap_element_, rc, true);
		}

		// If the dialog is minimized, skip rendering
		// its controls.
		if (!minimized_)
		{
			std::vector<std::vector<size_t> > intersected_groups;
			for (size_t i = 0; i < controls_.size(); ++ i)
			{
				for (size_t j = 0; j < i; ++ j)
				{
					Rect_T<int32_t> rc = controls_[i]->BoundingBoxRect() & controls_[j]->BoundingBoxRect();
					if ((rc.Width() > 0) && (rc.Height() > 0))
					{
						size_t k = 0;
						while (k < intersected_groups.size())
						{
							if (std::find(intersected_groups[k].begin(), intersected_groups[k].end(),
								i) != intersected_groups[k].end())
							{
								intersected_groups[k].push_back(i);
								break;
							}

							++ k;
						}

						if (k == intersected_groups.size())
						{
							intersected_groups.push_back(std::vector<size_t>());
							intersected_groups.back().push_back(j);
							intersected_groups.back().push_back(i);
						}

						break;
					}
				}
			}

			std::vector<size_t> intersected_controls;
			for (size_t i = 0; i < intersected_groups.size(); ++ i)
			{
				intersected_controls.insert(intersected_controls.end(), intersected_groups[i].begin(), intersected_groups[i].end());
			}
			std::sort(intersected_controls.begin(), intersected_controls.end());

			depth_base_ = 0.5f;
			for (size_t i = 0; i < controls_.size(); ++ i)
			{
				BOOST_TYPEOF(intersected_controls)::iterator iter
					= std::lower_bound(intersected_controls.begin(), intersected_controls.end(), i);
				if ((iter == intersected_controls.end()) || (*iter != i))
				{
					controls_[i]->Render();
				}
			}

			for (size_t i = 0; i < intersected_groups.size(); ++ i)
			{
				depth_base_ = 0.5f;
				for (size_t j = 0; j < intersected_groups[i].size(); ++ j)
				{
					controls_[intersected_groups[i][j]]->Render();
					depth_base_ -= 0.05f;
				}
			}
		}
	}

	void UIDialog::RequestFocus(UIControl& control)
	{
		if ((control_focus_.lock().get() != &control) && control.CanHaveFocus())
		{
			if (control_focus_.lock())
			{
				control_focus_.lock()->OnFocusOut();
			}

			control.OnFocusIn();
			control_focus_ = control.shared_from_this();
		}
	}

	void UIDialog::SetBackgroundColors(Color const & colorAllCorners)
	{
		this->SetBackgroundColors(colorAllCorners, colorAllCorners, colorAllCorners, colorAllCorners);
	}
	
	void UIDialog::SetBackgroundColors(Color const & colorTopLeft, Color const & colorTopRight,
		Color const & colorBottomLeft, Color const & colorBottomRight)
	{
		top_left_clr_ = colorTopLeft;
		top_right_clr_ = colorTopRight;
		bottom_left_clr_ = colorBottomLeft;
		bottom_right_clr_ = colorBottomRight;
	}

	void UIDialog::ClearFocus()
	{
		if (control_focus_.lock())
		{
			control_focus_.lock()->OnFocusOut();
			control_focus_.reset();
		}
	}

	UIControlPtr UIDialog::GetNextControl(UIControlPtr control) const
	{
		int index = control->GetIndex() + 1;

		UIDialogPtr dialog = control->GetDialog();

		// Cycle through dialogs in the loop to find the next control. Note
		// that if only one control exists in all looped dialogs it will
		// be the returned 'next' control.
		while (index >= static_cast<int>(dialog->controls_.size()))
		{
			dialog = UIManager::Instance().GetNextDialog(dialog);
			index = 0;
		}

		return dialog->controls_[index];
	}

	UIControlPtr UIDialog::GetPrevControl(UIControlPtr control) const
	{
		int index = control->GetIndex() - 1;

		UIDialogPtr dialog = control->GetDialog();

		// Cycle through dialogs in the loop to find the next control. Note
		// that if only one control exists in all looped dialogs it will
		// be the returned 'previous' control.
		while (index < 0)
		{
			dialog = UIManager::Instance().GetPrevDialog(dialog);
			if (!dialog)
			{
				dialog = control->GetDialog();
			}

			index = static_cast<int>(dialog->controls_.size() - 1);
		}

		return dialog->controls_[index];    
	}

	void UIDialog::ClearRadioButtonGroup(uint32_t nButtonGroup)
	{
		// Find all radio buttons with the given group number
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr pControl = controls_[i];

			if (UICT_RadioButton == pControl->GetType())
			{
				UIRadioButton* pRadioButton = dynamic_cast<UIRadioButton*>(pControl.get());

				if (pRadioButton->GetButtonGroup() == nButtonGroup)
				{
					pRadioButton->SetChecked(false, false);
				}
			}
		}
	}

	void UIDialog::RemoveControl(int ID)
	{
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr control = controls_[i];
			if (control->GetID() == ID)
			{
				this->ClearFocus();

				if (control_focus_.lock() == control)
				{
					control_focus_.reset();
				}
				if (control_mouse_over_.lock() == control)
				{
					control_mouse_over_.reset();
				}

				controls_.erase(controls_.begin() + i);

				return;
			}
		}
	}
	
	void UIDialog::RemoveAllControls()
	{
		if (control_focus_.lock() && (control_focus_.lock()->GetDialog().get() == this))
		{
			control_focus_.reset();
		}
		control_mouse_over_.reset();

		controls_.clear();
	}

	// Device state notification
	void UIDialog::Refresh()
	{
		if (control_focus_.lock())
		{
			control_focus_.lock()->OnFocusOut();
		}
		if (control_mouse_over_.lock())
		{
			control_mouse_over_.lock()->OnMouseLeave();
		}
		control_mouse_over_.reset();

		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			controls_[i]->Refresh();
		}

		if (keyboard_input_)
		{
			this->FocusDefaultControl();
		}
	}

	// Shared resource access. Indexed fonts and textures are shared among
	// all the controls.
	void UIDialog::SetFont(size_t index, FontPtr font)
	{
		// Make sure the list is at least as large as the index being set
		if (index + 1 > fonts_.size())
		{
			fonts_.resize(index + 1, -1);
		}
		fonts_[index] = static_cast<int>(UIManager::Instance().AddFont(font));
	}

	FontPtr UIDialog::GetFont(size_t index) const
	{
		return UIManager::Instance().GetFont(fonts_[index]);
	}

	void UIDialog::SetTexture(size_t index, TexturePtr texture)
	{
		// Make sure the list is at least as large as the index being set
		if (index + 1 > textures_.size())
		{
			textures_.resize(index + 1, -1);
		}
		textures_[index] = static_cast<int>(UIManager::Instance().AddTexture(texture));
	}

	TexturePtr UIDialog::GetTexture(size_t index) const
	{
		return UIManager::Instance().GetTexture(textures_[index]);
	}

	void UIDialog::FocusDefaultControl()
	{
		// Check for default control in this dialog
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr control = controls_[i];
			if (control->GetIsDefault())
			{
				// Remove focus from the current control
				this->ClearFocus();

				// Give focus to the default control
				control_focus_ = control;
				control->OnFocusIn();

				break;
			}
		}
	}

	void UIDialog::DrawRect(Rect_T<int32_t> const & rc, float depth, Color const & clr)
	{
		Rect_T<int32_t> rcScreen = rc + this->GetLocation();
		if (this->IsCaptionEnabled())
		{
			rcScreen += Vector_T<int32_t, 2>(0, this->GetCaptionHeight());
		}

		float3 pos(static_cast<float>(rcScreen.left()), static_cast<float>(rcScreen.top()), depth_base_ + depth);
		std::vector<Color> clrs(4, clr);
		UIManager::Instance().DrawRect(pos, static_cast<float>(rcScreen.Width()), static_cast<float>(rc.Height()),
			&clrs[0], Rect_T<int32_t>(0, 0, 0, 0), TexturePtr());
	}

	void UIDialog::DrawSprite(UIElement const & element, Rect_T<int32_t> const & rcDest)
	{
		// No need to draw fully transparent layers
		if (0 == element.TextureColor().Current.a())
		{
			return;
		}

		Rect_T<int32_t> rcTexture = element.TexRect();
		Rect_T<int32_t> rcScreen = rcDest + this->GetLocation();
		TexturePtr tex = this->GetTexture(element.TextureIndex());

		// If caption is enabled, offset the Y position by its height.
		if (this->IsCaptionEnabled())
		{
			rcScreen += Vector_T<int32_t, 2>(0, this->GetCaptionHeight());
		}

		float3 pos(static_cast<float>(rcScreen.left()), static_cast<float>(rcScreen.top()), depth_base_);
		std::vector<Color> clrs(4, element.TextureColor().Current);
		UIManager::Instance().DrawRect(pos, static_cast<float>(rcScreen.Width()),
			static_cast<float>(rcScreen.Height()), &clrs[0], rcTexture, tex);
	}

	void UIDialog::DrawText(std::wstring const & strText, UIElement const & uie, Rect_T<int32_t> const & rc, bool bShadow)
	{
		if (bShadow)
		{
			Rect_T<int32_t> rcShadow = rc;
			rcShadow += Vector_T<int32_t, 2>(1, 1);

			Rect_T<int32_t> r = rcShadow;
			r += this->GetLocation();
			if (this->IsCaptionEnabled())
			{
				r += Vector_T<int32_t, 2>(0, this->GetCaptionHeight());
			}

			UIManager::Instance().DrawText(strText, uie.FontIndex(), r, depth_base_ - 0.01f,
				Color(0, 0, 0, uie.FontColor().Current.a()), uie.TextAlign());
		}

		Rect_T<int32_t> r = rc;
		r += this->GetLocation();
		if (this->IsCaptionEnabled())
		{
			r += Vector_T<int32_t, 2>(0, this->GetCaptionHeight());
		}

		UIManager::Instance().DrawText(strText, uie.FontIndex(), r, depth_base_ - 0.01f,
			uie.FontColor().Current, uie.TextAlign());
	}

	// Initialize default Elements
	void UIDialog::InitDefaultElements()
	{
		this->SetFont(0, Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 12));

		UIElement Element;

		// Element for the caption
		cap_element_.SetFont(0);
		cap_element_.SetTexture(0, Rect_T<int32_t>(17, 269, 241, 287));
		cap_element_.TextureColor().States[UICS_Normal] = Color(1, 1, 1, 1);
		cap_element_.FontColor().States[UICS_Normal] = Color(1, 1, 1, 1);
		cap_element_.SetFont(0, Color(1, 1, 1, 1), Font::FA_Hor_Left | Font::FA_Ver_Middle);
		// Pre-blend as we don't need to transition the state
		cap_element_.TextureColor().SetState(UICS_Normal);
		cap_element_.FontColor().SetState(UICS_Normal);
	}

	bool UIDialog::OnCycleFocus(bool bForward)
	{
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			if (controls_[i] == control_focus_.lock())
			{
				if (bForward)
				{
					for (size_t j = 1; j < controls_.size(); ++ j)
					{
						UIControlPtr try_control = controls_[(i + j) % controls_.size()];
						if (try_control->CanHaveFocus())
						{
							this->RequestFocus(*try_control);
							return true;
						}
					}
				}
				else
				{
					for (size_t j = 1; j < controls_.size(); ++ j)
					{
						UIControlPtr try_control = controls_[(i - j) % controls_.size()];
						if (try_control->CanHaveFocus())
						{
							this->RequestFocus(*try_control);
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	void UIDialog::HandleInput()
	{
		InputEngine& ie = Context::Instance().InputFactoryInstance().InputEngineInstance();
		for (uint32_t i = 0; i < ie.NumDevices(); ++ i)
		{
			InputKeyboardPtr key_board = boost::dynamic_pointer_cast<InputKeyboard>(ie.Device(i));
			InputMousePtr mouse = boost::dynamic_pointer_cast<InputMouse>(ie.Device(i));
			if (key_board)
			{
				if (control_focus_.lock() && control_focus_.lock()->GetEnabled())
				{
					UIControl::KeyEventArg arg;
					arg.all_keys = key_board->Keys();
					for (size_t j = 0; j < key_board->NumKeys(); ++ j)
					{
						arg.key = static_cast<uint8_t>(j);
						if (!UIManager::Instance().GetLastKey(j) && key_board->Key(j))
						{
							control_focus_.lock()->GetKeyDownEvent()(*this, arg);
						}
						if (UIManager::Instance().GetLastKey(j) && !key_board->Key(j))
						{
							control_focus_.lock()->GetKeyUpEvent()(*this, arg);
						}
					}
				}
				else
				{
					bool handled = false;
					if (!control_focus_.lock() || (control_focus_.lock()->GetType() != UICT_EditBox))
					{
						for (size_t j = 0; j < key_board->NumKeys(); ++ j)
						{
							if (!UIManager::Instance().GetLastKey(j) && key_board->Key(j))
							{
								// See if this matches a control's hotkey
								// Activate the hotkey if the focus doesn't belong to an
								// edit box.
								BOOST_FOREACH(BOOST_TYPEOF(controls_)::reference control, controls_)
								{
									if (control->GetHotkey() == static_cast<uint8_t>(j))
									{
										control->OnHotkey();
										handled = true;
										break;
									}
								}
							}
						}
					}

					// Not yet handled, check for focus messages
					if (!handled)
					{
						// If keyboard input is not enabled, this message should be ignored
						if (keyboard_input_)
						{
							if (key_board->Key(KS_RightArrow) || key_board->Key(KS_DownArrow))
							{
								if (control_focus_.lock())
								{
									this->OnCycleFocus(true);
								}
							}

							if (key_board->Key(KS_LeftArrow) || key_board->Key(KS_UpArrow))
							{
								if (control_focus_.lock())
								{
									this->OnCycleFocus(false);
								}
							}

							if (key_board->Key(KS_Tab)) 
							{
								bool shift_down = key_board->Key(KS_LeftShift) | key_board->Key(KS_RightShift);
								this->OnCycleFocus(!shift_down);
							}
						}
					}
				}
			}
			if (mouse)
			{
				UIControl::MouseEventArg arg;
				arg.buttons = UIControl::MB_None;
				arg.flags = UIControl::MF_None;
				if (mouse->LeftButton())
				{
					arg.buttons |= UIControl::MB_Left;
				}
				if (mouse->RightButton())
				{
					arg.buttons |= UIControl::MB_Right;
				}
				if (mouse->MiddleButton())
				{
					arg.buttons |= UIControl::MB_Middle;
				}
				arg.location = Vector_T<int32_t, 2>(mouse->AbsX(), mouse->AbsY()) - this->GetLocation();
				if (show_caption_)
				{
					arg.location.y() -= caption_height_;
				}
				arg.z_delta = mouse->Z();

				for (uint32_t i = 0; i < ie.NumDevices(); ++ i)
				{
					InputKeyboardPtr key_board = boost::dynamic_pointer_cast<InputKeyboard>(ie.Device(i));
					if (key_board)
					{
						if (key_board->Key(KS_LeftShift) || key_board->Key(KS_RightShift))
						{
							arg.flags |= UIControl::MF_Shift;
						}
						if (key_board->Key(KS_LeftCtrl) || key_board->Key(KS_RightCtrl))
						{
							arg.flags |= UIControl::MF_Ctrl;
						}
						if (key_board->Key(KS_LeftAlt) || key_board->Key(KS_RightAlt))
						{
							arg.flags |= UIControl::MF_Alt;
						}
					}
				}

				UIControlPtr control;
				if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
					&& control_focus_.lock()->ContainsPoint(arg.location))
				{
					control = control_focus_.lock();
				}
				else
				{
					// Figure out which control the mouse is over now
					control = this->GetControlAtPoint(arg.location);

					if (control_mouse_over_.lock() != control)
					{
						// Handle mouse leaving the old control
						if (control_mouse_over_.lock())
						{
							control_mouse_over_.lock()->OnMouseLeave();
						}

						// Handle mouse entering the new control
						control_mouse_over_ = control;
						if (control)
						{
							control->OnMouseEnter();
						}
					}
				}

				if (control)
				{
					control->GetMouseOverEvent()(*this, arg);

					if ((!UIManager::Instance().GetLastMouseButton(0) && mouse->LeftButton())
						|| (!UIManager::Instance().GetLastMouseButton(1) && mouse->RightButton())
						|| (!UIManager::Instance().GetLastMouseButton(2) && mouse->MiddleButton()))
					{
						control->GetMouseDownEvent()(*this, arg);
					}
					arg.buttons = UIControl::MB_None;
					if (UIManager::Instance().GetLastMouseButton(0) && !mouse->LeftButton())
					{
						arg.buttons |= UIControl::MB_Left;
					}
					if (UIManager::Instance().GetLastMouseButton(1) && !mouse->RightButton())
					{
						arg.buttons |= UIControl::MB_Right;
					}
					if (UIManager::Instance().GetLastMouseButton(2) && !mouse->MiddleButton())
					{
						arg.buttons |= UIControl::MB_Middle;
					}
					if (arg.buttons != UIControl::MB_None)
					{
						control->GetMouseUpEvent()(*this, arg);
					}

					if (mouse->Z() != 0)
					{
						control->GetMouseWheelEvent()(*this, arg);
					}
				}
				else
				{
					if (!UIManager::Instance().GetLastMouseButton(0) && mouse->LeftButton()
						&& control_focus_.lock())
					{
						control_focus_.lock()->OnFocusOut();
						control_focus_.reset();
					}
				}
			}
		}
	}
}
