// UI.cpp
// KlayGE 图形用户界面 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2007-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 直接从uiml文件读取ui布局 (2009.4.20)
//
// 3.6.0
// 初次建立 (2007.6.27)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/XMLDom.hpp>
#include <KlayGE/Font.hpp>

#include <KlayGE/Input.hpp>

#ifdef Bool
#undef Bool		// for boost::foreach
#endif

#include <cstring>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/UI.hpp>

namespace
{
	bool ReadBool(KlayGE::XMLNodePtr& node, std::string const & name, bool default_val)
	{
		bool ret = default_val;

		KlayGE::XMLAttributePtr attr = node->Attrib(name);
		if (attr)
		{
			std::string val_str = attr->ValueString();
			if (("true" == val_str) || ("1" == val_str))
			{
				ret = true;
			}
			else
			{
				BOOST_ASSERT(("false" == val_str) || ("0" == val_str));
				ret = false;
			}
		}

		return ret;
	}
}

namespace KlayGE
{
	UIManagerPtr UIManager::ui_mgr_instance_;


	class UIRectRenderable : public RenderableHelper
	{
	public:
		UIRectRenderable(TexturePtr const & texture, RenderEffectPtr const & effect)
			: RenderableHelper(L"UIRect"),
				dirty_(false), texture_(texture)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			restart_ = rf.RenderEngineInstance().DeviceCaps().primitive_restart_support;

			rl_ = rf.MakeRenderLayout();
			if (restart_)
			{
				rl_->TopologyType(RenderLayout::TT_TriangleStrip);
			}
			else
			{
				rl_->TopologyType(RenderLayout::TT_TriangleList);
			}

			vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
			rl_->BindVertexStream(vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F),
												vertex_element(VEU_Diffuse, 0, EF_ABGR32F),
												vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			ib_ = rf.MakeIndexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
			rl_->BindIndexStream(ib_, EF_R16UI);

			if (texture)
			{
				technique_ = effect->TechniqueByName("UITec");
			}
			else
			{
				technique_ = effect->TechniqueByName("UITecNoTex");
			}

			half_width_height_ep_ = technique_->Effect().ParameterByName("half_width_height");
		}

		void Clear()
		{
			vertices_.resize(0);
			indices_.resize(0);
		}

		bool Empty() const
		{
			return indices_.empty();
		}

		std::vector<UIManager::VertexFormat>& Vertices()
		{
			dirty_ = true;
			return vertices_;
		}

		std::vector<uint16_t>& Indices()
		{
			dirty_ = true;
			return indices_;
		}

		bool PriRestart() const
		{
			return restart_;
		}

		void UpdateBuffers()
		{
			if (dirty_)
			{
				if (!vertices_.empty() || !indices_.empty())
				{
					vb_->Resize(static_cast<uint32_t>(vertices_.size() * sizeof(vertices_[0])));
					{
						GraphicsBuffer::Mapper mapper(*vb_, BA_Write_Only);
						std::memcpy(mapper.Pointer<uint8_t>(), &vertices_[0], vb_->Size());
					}

					ib_->Resize(static_cast<uint32_t>(indices_.size() * sizeof(indices_[0])));
					{
						GraphicsBuffer::Mapper mapper(*ib_, BA_Write_Only);
						std::memcpy(mapper.Pointer<uint8_t>(), &indices_[0], ib_->Size());
					}
				}

				dirty_ = false;
			}
		}

		void OnRenderBegin()
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			*(technique_->Effect().ParameterByName("ui_tex")) = texture_;

			RenderEngine& re = rf.RenderEngineInstance();
			float const half_width = re.CurFrameBuffer()->Width() / 2.0f;
			float const half_height = re.CurFrameBuffer()->Height() / 2.0f;

			*half_width_height_ep_ = float2(half_width, half_height);
		}

		void Render()
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			this->OnRenderBegin();
			renderEngine.Render(*this->GetRenderTechnique(), *rl_);
			this->OnRenderEnd();
		}

	private:
		bool restart_;
		bool dirty_;

		RenderEffectParameterPtr half_width_height_ep_;

		TexturePtr texture_;

		std::vector<UIManager::VertexFormat> vertices_;
		std::vector<uint16_t> indices_;

		GraphicsBufferPtr vb_;
		GraphicsBufferPtr ib_;
	};

	class UIRectObject : public SceneObjectHelper
	{
	public:
		UIRectObject(RenderablePtr const & renderable, uint32_t attrib)
			: SceneObjectHelper(renderable, attrib)
		{
		}

		void Update()
		{
			checked_pointer_cast<UIRectRenderable>(renderable_)->UpdateBuffers();
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

	void UIElement::SetFont(uint32_t font_index)
	{
		this->SetFont(font_index, Color(1, 1, 1, 1));
	}

	void UIElement::SetFont(uint32_t font_index, Color const & default_font_color)
	{
		this->SetFont(font_index, default_font_color, Font::FA_Hor_Center | Font::FA_Ver_Middle);
	}

	void UIElement::SetFont(uint32_t font_index, Color const & default_font_color, uint32_t text_align)
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
		if (!ui_mgr_instance_)
		{
			ui_mgr_instance_ = MakeSharedPtr<UIManager>();
		}

		return *ui_mgr_instance_;
	}

	void UIManager::ForceDestroy()
	{
		if (ui_mgr_instance_)
		{
			ui_mgr_instance_.reset();
		}
	}

	UIManager::UIManager()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		effect_ = rf.LoadEffect("UI.fxml");

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

		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(136, 0, 141, 5));
		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(141, 0, 247, 5));
		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(247, 0, 252, 5));
		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(136, 5, 141, 49));
		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(141, 5, 247, 49));
		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(247, 5, 252, 49));
		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(136, 49, 141, 54));
		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(141, 49, 247, 54));
		elem_texture_rcs_[UICT_TexButton].push_back(Rect_T<int32_t>(247, 49, 252, 54));

		WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
		main_wnd->OnKeyDown().connect(boost::bind(&UIManager::KeyDownHandler, this, _2));
		main_wnd->OnKeyUp().connect(boost::bind(&UIManager::KeyUpHandler, this, _2));
		main_wnd->OnMouseDown().connect(boost::bind(&UIManager::MouseDownHandler, this, _2, _3));
		main_wnd->OnMouseUp().connect(boost::bind(&UIManager::MouseUpHandler, this, _2, _3));
		main_wnd->OnMouseWheel().connect(boost::bind(&UIManager::MouseWheelHandler, this, _2, _3, _4));
		main_wnd->OnMouseOver().connect(boost::bind(&UIManager::MouseOverHandler, this, _2, _3));
	}

	void UIManager::Load(ResIdentifierPtr const & source)
	{
		if (source)
		{
			XMLDocument doc;
			XMLNodePtr root = doc.Parse(source);

			XMLAttributePtr attr;

			std::vector<XMLDocumentPtr> include_docs;
			for (XMLNodePtr node = root->FirstNode("include"); node;)
			{
				attr = node->Attrib("name");
				include_docs.push_back(MakeSharedPtr<XMLDocument>());
				XMLNodePtr include_root = include_docs.back()->Parse(ResLoader::Instance().Open(attr->ValueString()));

				for (XMLNodePtr child_node = include_root->FirstNode(); child_node; child_node = child_node->NextSibling())
				{
					if (XNT_Element == child_node->Type())
					{
						root->InsertNode(node, doc.CloneNode(child_node));
					}
				}

				XMLNodePtr node_next = node->NextSibling("include");
				root->RemoveNode(node);
				node = node_next;
			}

			for (XMLNodePtr node = root->FirstNode("dialog"); node; node = node->NextSibling("dialog"))
			{
				UIDialogPtr dlg;
				{
					int32_t x, y;
					uint32_t width, height;
					UIDialog::ControlAlignment align_x = UIDialog::CA_Left, align_y = UIDialog::CA_Top;
					std::string id = node->AttribString("id", "");
					std::string caption = node->AttribString("caption", "");
					std::string skin = node->AttribString("skin", "");
					x = node->Attrib("x")->ValueInt();
					y = node->Attrib("y")->ValueInt();
					width = node->Attrib("width")->ValueInt();
					height = node->Attrib("height")->ValueInt();
					attr = node->Attrib("align_x");
					if (attr)
					{
						std::string align_x_str = attr->ValueString();
						if ("left" == align_x_str)
						{
							align_x = UIDialog::CA_Left;
						}
						else
						{
							if ("right" == align_x_str)
							{
								align_x = UIDialog::CA_Right;
							}
							else
							{
								BOOST_ASSERT("center" == align_x_str);
								align_x = UIDialog::CA_Center;
							}
						}
					}
					attr = node->Attrib("align_y");
					if (attr)
					{
						std::string align_y_str = attr->ValueString();
						if ("top" == align_y_str)
						{
							align_y = UIDialog::CA_Top;
						}
						else
						{
							if ("bottom" == align_y_str)
							{
								align_y = UIDialog::CA_Bottom;
							}
							else
							{
								BOOST_ASSERT("middle" == align_y_str);
								align_y = UIDialog::CA_Middle;
							}
						}
					}

					TexturePtr tex;
					if (!skin.empty())
					{
						tex = SyncLoadTexture(skin, EAH_GPU_Read | EAH_Immutable);
					}
					dlg = this->MakeDialog(tex);
					dlg->SetID(id);
					std::wstring wcaption;
					Convert(wcaption, caption);
					dlg->SetCaptionText(wcaption);

					dlg->EnableCaption(ReadBool(node, "show_caption", true));

					Color bg_clr(0.4f, 0.6f, 0.8f, 1);
					attr = node->Attrib("bg_color_r");
					if (attr)
					{
						bg_clr.r() = attr->ValueFloat();
					}
					attr = node->Attrib("bg_color_g");
					if (attr)
					{
						bg_clr.g() = attr->ValueFloat();
					}
					attr = node->Attrib("bg_color_b");
					if (attr)
					{
						bg_clr.b() = attr->ValueFloat();
					}
					attr = node->Attrib("bg_color_a");
					if (attr)
					{
						bg_clr.a() = attr->ValueFloat();
					}
					dlg->SetBackgroundColors(bg_clr);

					UIDialog::ControlLocation loc = { x, y, align_x, align_y };
					dlg->CtrlLocation(-1, loc);
					dlg->SetSize(width, height);
				}

				std::vector<std::string> ctrl_ids;
				for (XMLNodePtr ctrl_node = node->FirstNode("control"); ctrl_node; ctrl_node = ctrl_node->NextSibling("control"))
				{
					ctrl_ids.push_back(ctrl_node->Attrib("id")->ValueString());
				}
				std::sort(ctrl_ids.begin(), ctrl_ids.end());
				ctrl_ids.erase(std::unique(ctrl_ids.begin(), ctrl_ids.end()), ctrl_ids.end());

				for (XMLNodePtr ctrl_node = node->FirstNode("control"); ctrl_node; ctrl_node = ctrl_node->NextSibling("control"))
				{
					int32_t x, y;
					uint32_t width, height;
					bool is_default = false;
					bool visible = true;
					UIDialog::ControlAlignment align_x = UIDialog::CA_Left, align_y = UIDialog::CA_Top;

					uint32_t id;
					{
						std::string id_str = ctrl_node->Attrib("id")->ValueString();
						id = static_cast<uint32_t>(std::find(ctrl_ids.begin(), ctrl_ids.end(), id_str) - ctrl_ids.begin());
						dlg->AddIDName(id_str, id);
					}

					x = ctrl_node->Attrib("x")->ValueInt();
					y = ctrl_node->Attrib("y")->ValueInt();
					width = ctrl_node->Attrib("width")->ValueInt();
					height = ctrl_node->Attrib("height")->ValueInt();
					is_default = ReadBool(ctrl_node, "is_default", false);
					visible = ReadBool(ctrl_node, "visible", true);
					attr = ctrl_node->Attrib("align_x");
					if (attr)
					{
						std::string align_x_str = attr->ValueString();
						if ("left" == align_x_str)
						{
							align_x = UIDialog::CA_Left;
						}
						else
						{
							if ("right" == align_x_str)
							{
								align_x = UIDialog::CA_Right;
							}
							else
							{
								BOOST_ASSERT("center" == align_x_str);
								align_x = UIDialog::CA_Center;
							}
						}
					}
					attr = ctrl_node->Attrib("align_y");
					if (attr)
					{
						std::string align_y_str = attr->ValueString();
						if ("top" == align_y_str)
						{
							align_y = UIDialog::CA_Top;
						}
						else
						{
							if ("bottom" == align_y_str)
							{
								align_y = UIDialog::CA_Bottom;
							}
							else
							{
								BOOST_ASSERT("middle" == align_y_str);
								align_y = UIDialog::CA_Middle;
							}
						}
					}

					{
						UIDialog::ControlLocation loc = { x, y, align_x, align_y };
						dlg->CtrlLocation(id, loc);
					}

					std::string type_str = ctrl_node->Attrib("type")->ValueString();
					if ("static" == type_str)
					{
						std::string caption = ctrl_node->Attrib("caption")->ValueString();
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UIStatic>(dlg, id, wcaption,
							x, y, width, height, is_default));
					}
					else if ("button" == type_str)
					{
						std::string caption = ctrl_node->Attrib("caption")->ValueString();
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UIButton>(dlg, id, wcaption,
							x, y, width, height, hotkey, is_default));
					}
					else if ("tex_button" == type_str)
					{
						TexturePtr tex;
						attr = ctrl_node->Attrib("texture");
						if (attr)
						{
							std::string tex_name = attr->ValueString();
							tex = SyncLoadTexture(tex_name, EAH_GPU_Read | EAH_Immutable);
						}
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						dlg->AddControl(MakeSharedPtr<UITexButton>(dlg, id, tex,
							x, y, width, height, hotkey, is_default));
					}
					else if ("check_box" == type_str)
					{
						std::string caption = ctrl_node->Attrib("caption")->ValueString();
						bool checked = ReadBool(ctrl_node, "checked", false);
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UICheckBox>(dlg, id, wcaption,
							x, y, width, height, checked, hotkey, is_default));
					}
					else if ("radio_button" == type_str)
					{
						std::string caption = ctrl_node->Attrib("caption")->ValueString();
						int32_t button_group = ctrl_node->Attrib("button_group")->ValueInt();
						bool checked = ReadBool(ctrl_node, "checked", false);
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UIRadioButton>(dlg, id, button_group, wcaption,
							x, y, width, height, checked, hotkey, is_default));
					}
					else if ("slider" == type_str)
					{
						int32_t min_v = ctrl_node->AttribInt("min", 0);
						int32_t max_v = ctrl_node->AttribInt("max", 100);
						int32_t value = ctrl_node->AttribInt("value", 50);
						dlg->AddControl(MakeSharedPtr<UISlider>(dlg, id,
							x, y, width, height, min_v, max_v, value, is_default));
					}
					else if ("scroll_bar" == type_str)
					{
						int32_t track_start = ctrl_node->AttribInt("track_start", 0);
						int32_t track_end = ctrl_node->AttribInt("track_end", 1);
						int32_t track_pos = ctrl_node->AttribInt("track_pos", 1);
						int32_t page_size = ctrl_node->AttribInt("page_size", 1);
						dlg->AddControl(MakeSharedPtr<UIScrollBar>(dlg, id,
							x, y, width, height, track_start, track_end, track_pos, page_size));
					}
					else if ("list_box" == type_str)
					{
						UIListBox::STYLE style = UIListBox::SINGLE_SELECTION;
						attr = ctrl_node->Attrib("style");
						if (attr)
						{
							std::string style_str = attr->ValueString();
							if ("single" == style_str)
							{
								style = UIListBox::SINGLE_SELECTION;
							}
							else
							{
								BOOST_ASSERT("multi" == style_str);
								style = UIListBox::MULTI_SELECTION;
							}
						}
						dlg->AddControl(MakeSharedPtr<UIListBox>(dlg, id,
							x, y, width, height, style ? UIListBox::SINGLE_SELECTION : UIListBox::MULTI_SELECTION));

						for (XMLNodePtr item_node = ctrl_node->FirstNode("item"); item_node; item_node = item_node->NextSibling("item"))
						{
							std::string caption = item_node->Attrib("name")->ValueString();
							std::wstring wcaption;
							Convert(wcaption, caption);
							dlg->Control<UIListBox>(id)->AddItem(wcaption);
						}

						attr = ctrl_node->Attrib("selected");
						if (attr)
						{
							dlg->Control<UIListBox>(id)->SelectItem(attr->ValueInt());
						}
					}
					else if ("combo_box" == type_str)
					{
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						dlg->AddControl(MakeSharedPtr<UIComboBox>(dlg, id,
							x, y, width, height, hotkey, is_default));

						for (XMLNodePtr item_node = ctrl_node->FirstNode("item"); item_node; item_node = item_node->NextSibling("item"))
						{
							std::string caption = item_node->Attrib("name")->ValueString();
							std::wstring wcaption;
							Convert(wcaption, caption);
							dlg->Control<UIComboBox>(id)->AddItem(wcaption);
						}

						attr = ctrl_node->Attrib("selected");
						if (attr)
						{
							dlg->Control<UIComboBox>(id)->SetSelectedByIndex(attr->ValueInt());
						}
					}
					else if ("edit_box" == type_str)
					{
						std::string caption = ctrl_node->Attrib("caption")->ValueString();
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UIEditBox>(dlg, id, wcaption,
							x, y, width, height, is_default));
					}
					else if ("polyline_edit_box" == type_str)
					{
						Color line_clr(0, 1, 0, 1);
						line_clr.r() = ctrl_node->AttribFloat("line_r", 0);
						line_clr.g() = ctrl_node->AttribFloat("line_g", 1);
						line_clr.b() = ctrl_node->AttribFloat("line_b", 0);
						line_clr.a() = ctrl_node->AttribFloat("line_a", 1);
						dlg->AddControl(MakeSharedPtr<UIPolylineEditBox>(dlg, id,
							x, y, width, height, is_default));
						dlg->Control<UIPolylineEditBox>(id)->SetColor(line_clr);
					}
					else if ("progress_bar" == type_str)
					{
						int32_t progress = ctrl_node->AttribInt("value", 0);
						dlg->AddControl(MakeSharedPtr<UIProgressBar>(dlg, id, progress,
							x, y, width, height, is_default));
					}

					dlg->GetControl(id)->SetVisible(visible);
				}
			}
		}
	}

	UIDialogPtr UIManager::MakeDialog(TexturePtr const & control_tex)
	{
		UIDialogPtr ret = MakeSharedPtr<UIDialog>(control_tex);
		this->RegisterDialog(ret);
		return ret;
	}

	size_t UIManager::AddTexture(TexturePtr const & texture)
	{
		texture_cache_.push_back(texture);
		return texture_cache_.size() - 1;
	}

	size_t UIManager::AddFont(FontPtr const & font, uint32_t font_size)
	{
		font_cache_.push_back(std::make_pair(font, font_size));
		return font_cache_.size() - 1;
	}

	TexturePtr const & UIManager::GetTexture(size_t index) const
	{
		if (index < texture_cache_.size())
		{
			return texture_cache_[index];
		}
		else
		{
			static TexturePtr empty = TexturePtr();
			return empty;
		}
	}

	FontPtr const & UIManager::GetFont(size_t index) const
	{
		if (index < font_cache_.size())
		{
			return font_cache_[index].first;
		}
		else
		{
			static FontPtr empty = FontPtr();
			return empty;
		}
	}

	uint32_t UIManager::GetFontSize(size_t index) const
	{
		if (index < font_cache_.size())
		{
			return font_cache_[index].second;
		}
		else
		{
			return 0;
		}
	}

	bool UIManager::RegisterDialog(UIDialogPtr const & dialog)
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

	void UIManager::UnregisterDialog(UIDialogPtr const & dialog)
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

	UIDialogPtr const & UIManager::GetDialog(std::string const & id) const
	{
		for (size_t i = 0; i < dialogs_.size(); ++ i)
		{
			if (dialogs_[i]->GetID() == id)
			{
				return dialogs_[i];
			}
		}

		static UIDialogPtr empty;
		return empty;
	}

	UIDialogPtr const & UIManager::GetNextDialog(UIDialogPtr const & dialog) const
	{
		static UIDialogPtr empty;

		if (dialog == dialogs_.back())
		{
			return empty;
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

			return empty;
		}
	}

	UIDialogPtr const & UIManager::GetPrevDialog(UIDialogPtr const & dialog) const
	{
		static UIDialogPtr empty;

		if (dialog == dialogs_.front())
		{
			return empty;
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

			return empty;
		}
	}

	void UIManager::Render()
	{
		BOOST_FOREACH(BOOST_TYPEOF(rects_)::reference rect, rects_)
		{
			checked_pointer_cast<UIRectRenderable>(rect.second)->Clear();
		}
		BOOST_FOREACH(BOOST_TYPEOF(strings_)::reference string, strings_)
		{
			string.second.clear();
		}

		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			dialog->Render();
		}

		BOOST_FOREACH(BOOST_TYPEOF(rects_)::reference rect, rects_)
		{
			if (!checked_pointer_cast<UIRectRenderable>(rect.second)->Empty())
			{
				boost::shared_ptr<UIRectObject> ui_rect_obj = MakeSharedPtr<UIRectObject>(rect.second, SceneObject::SOA_Overlay);
				ui_rect_obj->AddToSceneManager();
			}
		}
		BOOST_FOREACH(BOOST_TYPEOF(strings_)::reference string, strings_)
		{
			BOOST_TYPEOF(font_cache_)::reference font = font_cache_[string.first];
			BOOST_FOREACH(BOOST_TYPEOF(string.second)::reference s, string.second)
			{
				font.first->RenderText(s.rc, s.depth, 1, 1, s.clr, s.text, font.second, s.align);
			}
		}
	}

	void UIManager::DrawRect(float3 const & pos, float width, float height, Color const * clrs,
				Rect_T<int32_t> const & rcTexture, TexturePtr const & texture)
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

		boost::shared_ptr<UIRectRenderable> renderable;
		if (rects_.find(texture) == rects_.end())
		{
			renderable = MakeSharedPtr<UIRectRenderable>(texture, effect_);
			rects_[texture] = renderable;
		}
		else
		{
			renderable = checked_pointer_cast<UIRectRenderable>(rects_[texture]);
		}
		BOOST_ASSERT(renderable);

		std::vector<VertexFormat>& vertices = renderable->Vertices();
		std::vector<uint16_t>& indices = renderable->Indices();
		vertices.reserve(vertices.size() + 4);

		uint16_t const last_index = static_cast<uint16_t>(vertices.size());
		if (renderable->PriRestart())
		{
			indices.reserve(indices.size() + 5);
			indices.push_back(last_index + 0);
			indices.push_back(last_index + 1);
			indices.push_back(last_index + 3);
			indices.push_back(last_index + 2);
			indices.push_back(0xFFFF);
		}
		else
		{
			indices.reserve(indices.size() + 6);
			indices.push_back(last_index + 0);
			indices.push_back(last_index + 1);
			indices.push_back(last_index + 2);
			indices.push_back(last_index + 2);
			indices.push_back(last_index + 3);
			indices.push_back(last_index + 0);
		}

		vertices.push_back(VertexFormat(pos + float3(0, 0, 0),
			clrs[0], float2(texcoord.left(), texcoord.top())));
		vertices.push_back(VertexFormat(pos + float3(width, 0, 0),
			clrs[1], float2(texcoord.right(), texcoord.top())));
		vertices.push_back(VertexFormat(pos + float3(width, height, 0),
			clrs[2], float2(texcoord.right(), texcoord.bottom())));
		vertices.push_back(VertexFormat(pos + float3(0, height, 0),
			clrs[3], float2(texcoord.left(), texcoord.bottom())));
	}

	void UIManager::DrawQuad(float3 const & offset, VertexFormat const * vertices, TexturePtr const & texture)
	{
		boost::shared_ptr<UIRectRenderable> renderable;
		if (rects_.find(texture) == rects_.end())
		{
			renderable = MakeSharedPtr<UIRectRenderable>(texture, effect_);
			rects_[texture] = renderable;
		}
		else
		{
			renderable = checked_pointer_cast<UIRectRenderable>(rects_[texture]);
		}
		BOOST_ASSERT(renderable);

		std::vector<VertexFormat>& verts = renderable->Vertices();
		std::vector<uint16_t>& indices = renderable->Indices();
		verts.reserve(verts.size() + 4);

		uint16_t const last_index = static_cast<uint16_t>(verts.size());
		if (renderable->PriRestart())
		{
			indices.reserve(indices.size() + 5);
			indices.push_back(last_index + 0);
			indices.push_back(last_index + 1);
			indices.push_back(last_index + 3);
			indices.push_back(last_index + 2);
			indices.push_back(0xFFFF);
		}
		else
		{
			indices.reserve(indices.size() + 6);
			indices.push_back(last_index + 0);
			indices.push_back(last_index + 1);
			indices.push_back(last_index + 2);
			indices.push_back(last_index + 2);
			indices.push_back(last_index + 3);
			indices.push_back(last_index + 0);
		}

		verts.push_back(VertexFormat(offset + vertices[0].pos,
			vertices[0].clr, vertices[0].tex));
		verts.push_back(VertexFormat(offset + vertices[1].pos,
			vertices[1].clr, vertices[1].tex));
		verts.push_back(VertexFormat(offset + vertices[2].pos,
			vertices[2].clr, vertices[2].tex));
		verts.push_back(VertexFormat(offset + vertices[3].pos,
			vertices[3].clr, vertices[3].tex));
	}

	void UIManager::DrawText(std::wstring const & strText, uint32_t font_index,
		Rect_T<int32_t> const & rc, float depth, Color const & clr, uint32_t align)
	{
		strings_[font_index].push_back(string_cache());
		string_cache& sc = strings_[font_index].back();
		sc.rc = rc;
		sc.depth = depth;
		sc.clr = clr;
		sc.text = strText;
		sc.align = align;
	}

	Size_T<uint32_t> UIManager::CalcSize(std::wstring const & strText, uint32_t font_index,
		Rect_T<int32_t> const & /*rc*/, uint32_t /*align*/)
	{
		BOOST_TYPEOF(font_cache_)::reference font = font_cache_[font_index];
		return font.first->CalcSize(strText, font.second);
	}

	void UIManager::KeyDownHandler(wchar_t key)
	{
		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			if (dialog->GetVisible())
			{
				dialog->KeyDownHandler(key);
			}
		}
	}

	void UIManager::KeyUpHandler(wchar_t key)
	{
		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			if (dialog->GetVisible())
			{
				dialog->KeyUpHandler(key);
			}
		}
	}

	void UIManager::MouseDownHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
	{
		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			if (dialog->GetVisible())
			{
				dialog->MouseDownHandler(buttons, dialog->ToLocal(pt));
			}
		}
	}

	void UIManager::MouseUpHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
	{
		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			if (dialog->GetVisible())
			{
				dialog->MouseUpHandler(buttons, dialog->ToLocal(pt));
			}
		}
	}

	void UIManager::MouseWheelHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt, int32_t z_delta)
	{
		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			if (dialog->GetVisible())
			{
				dialog->MouseWheelHandler(buttons, dialog->ToLocal(pt), z_delta);
			}
		}
	}

	void UIManager::MouseOverHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
	{
		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			if (dialog->GetVisible())
			{
				dialog->MouseOverHandler(buttons, dialog->ToLocal(pt));
			}
		}
	}

	Rect_T<int32_t> const & UIManager::ElementTextureRect(uint32_t ctrl, uint32_t elem_index)
	{
		BOOST_ASSERT(ctrl < elem_texture_rcs_.size());
		BOOST_ASSERT(elem_index < elem_texture_rcs_[ctrl].size());

		return elem_texture_rcs_[ctrl][elem_index];
	}

	size_t UIManager::NumElementTextureRect(uint32_t ctrl) const
	{
		BOOST_ASSERT(ctrl < elem_texture_rcs_.size());
		return elem_texture_rcs_[ctrl].size();
	}

	void UIManager::SettleCtrls(uint32_t width, uint32_t height)
	{
		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			dialog->SettleCtrls(width, height);
		}
	}


	UIDialog::UIDialog(TexturePtr const & control_tex)
			: keyboard_input_(false), mouse_input_(true), default_control_id_(0xFFFF),
					visible_(true), show_caption_(true),
					minimized_(false), drag_(false),
					bounding_box_(0, 0, 0, 0),
					caption_height_(18),
					top_left_clr_(0, 0, 0, 0), top_right_clr_(0, 0, 0, 0),
					bottom_left_clr_(0, 0, 0, 0), bottom_right_clr_(0, 0, 0, 0)
	{
		TexturePtr ct;
		if (!control_tex)
		{
			ct = SyncLoadTexture("ui.dds", EAH_GPU_Read | EAH_Immutable);
		}
		else
		{
			ct = control_tex;
		}

		tex_index_ = UIManager::Instance().AddTexture(ct);
		this->InitDefaultElements();
	}

	UIDialog::~UIDialog()
	{
		this->RemoveAllControls();
	}

	void UIDialog::AddControl(UIControlPtr const & control)
	{
		this->InitControl(*control);

		// Add to the list
		controls_.push_back(control);
	}

	void UIDialog::InitControl(UIControl& control)
	{
		control.SetIndex(static_cast<uint32_t>(controls_.size()));
	}

	UIControlPtr const & UIDialog::GetControl(int ID) const
	{
		// Try to find the control with the given ID
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr const & control = controls_[i];
			if (control->GetID() == ID)
			{
				return control;
			}
		}

		// Not found
		static UIControlPtr ret;
		return ret;
	}

	UIControlPtr const & UIDialog::GetControl(int ID, uint32_t type) const
	{
		// Try to find the control with the given ID
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr const & control = controls_[i];
			if ((control->GetID() == ID) && (control->GetType() == type))
			{
				return control;
			}
		}

		// Not found
		static UIControlPtr ret;
		return ret;
	}

	UIControlPtr const & UIDialog::GetControlAtPoint(Vector_T<int32_t, 2> const & pt) const
	{
		// Search through all child controls for the first one which
		// contains the mouse point
		for (size_t i = 0; i < controls_.size(); ++ i)
		{
			UIControlPtr const & control = controls_[i];

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

		static UIControlPtr ret;
		return ret;
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

		depth_base_ = 0.5f;

		bool gamma = Context::Instance().Config().graphics_cfg.gamma;

		bool bBackgroundIsVisible = (top_left_clr_.a() != 0) || (top_right_clr_.a() != 0)
			|| (bottom_right_clr_.a() != 0) || (bottom_left_clr_.a() != 0);
		if (!minimized_ && bBackgroundIsVisible)
		{
			boost::array<Color, 4> clrs;
			if (gamma)
			{
				clrs[0] = Color(MathLib::srgb_to_linear(top_left_clr_.r()), MathLib::srgb_to_linear(top_left_clr_.g()), MathLib::srgb_to_linear(top_left_clr_.b()), top_left_clr_.a());
				clrs[1] = Color(MathLib::srgb_to_linear(top_right_clr_.r()), MathLib::srgb_to_linear(top_right_clr_.g()), MathLib::srgb_to_linear(top_right_clr_.b()), top_right_clr_.a());
				clrs[2] = Color(MathLib::srgb_to_linear(bottom_right_clr_.r()), MathLib::srgb_to_linear(bottom_right_clr_.g()), MathLib::srgb_to_linear(bottom_right_clr_.b()), bottom_right_clr_.a());
				clrs[3] = Color(MathLib::srgb_to_linear(bottom_left_clr_.r()), MathLib::srgb_to_linear(bottom_left_clr_.g()), MathLib::srgb_to_linear(bottom_left_clr_.b()), bottom_left_clr_.a());
			}
			else
			{
				clrs[0] = top_left_clr_;
				clrs[1] = top_right_clr_;
				clrs[2] = bottom_right_clr_;
				clrs[3] = bottom_left_clr_;
			}

			Rect_T<int32_t> rc(0, 0, this->GetWidth(), this->GetHeight());
			Rect_T<int32_t> rcScreen = rc + this->GetLocation();
			if (this->IsCaptionEnabled())
			{
				rcScreen += Vector_T<int32_t, 2>(0, this->GetCaptionHeight());
			}

			float3 pos(static_cast<float>(rcScreen.left()), static_cast<float>(rcScreen.top()), depth_base_);
			UIManager::Instance().DrawRect(pos, static_cast<float>(this->GetWidth()),
				static_cast<float>(this->GetHeight()), &clrs[0], Rect_T<int32_t>(0, 0, 0, 0), TexturePtr());
		}

		// Render the caption if it's enabled.
		if (this->IsCaptionEnabled())
		{
			std::wstring wstrOutput = caption_;
			if (minimized_)
			{
				wstrOutput += L" (Minimized)";
			}

			// DrawSprite will offset the rect down by
			// caption_height_, so adjust the rect higher
			// here to negate the effect.
			int32_t w = this->GetWidth();
			Rect_T<int32_t> rc(0, -caption_height_, w, 0);

			Size_T<uint32_t> size = this->CalcSize(wstrOutput, cap_element_, rc, true);

			w = std::min(w, static_cast<int32_t>(size.cx() * 1.2f));
			rc.right() = w;

			Color clr = cap_element_.TextureColor().Current;
			if (gamma)
			{
				clr = Color(MathLib::srgb_to_linear(clr.r()), MathLib::srgb_to_linear(clr.g()), MathLib::srgb_to_linear(clr.b()), clr.a());
			}
			UIManager::VertexFormat vertices[] =
			{
				UIManager::VertexFormat(float3(0, static_cast<float>(-caption_height_), 0), clr, float2(0, 0)),
				UIManager::VertexFormat(float3(static_cast<float>(w), static_cast<float>(-caption_height_), 0), clr, float2(0, 0)),
				UIManager::VertexFormat(float3(static_cast<float>(w + caption_height_), 0, 0), clr, float2(0, 0)),
				UIManager::VertexFormat(float3(0, 0, 0), clr, float2(0, 0))
			};
			this->DrawQuad(&vertices[0], 0, TexturePtr());
			rc.left() += 5; // Make a left margin

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

	Vector_T<int32_t, 2> UIDialog::ToLocal(Vector_T<int32_t, 2> const & pt) const
	{
		Vector_T<int32_t, 2> ret = pt - this->GetLocation();
		if (this->IsCaptionEnabled())
		{
			ret.y() -= caption_height_;
		}
		return ret;
	}

	void UIDialog::ClearFocus()
	{
		if (control_focus_.lock())
		{
			control_focus_.lock()->OnFocusOut();
			control_focus_.reset();
		}
	}

	UIControlPtr const & UIDialog::GetNextControl(UIControlPtr const & control) const
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

	UIControlPtr const & UIDialog::GetPrevControl(UIControlPtr const & control) const
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
	void UIDialog::SetFont(size_t index, FontPtr const & font, uint32_t font_size)
	{
		// Make sure the list is at least as large as the index being set
		if (index + 1 > fonts_.size())
		{
			fonts_.resize(index + 1, -1);
		}
		fonts_[index] = static_cast<int>(UIManager::Instance().AddFont(font, font_size));
	}

	FontPtr const & UIDialog::GetFont(size_t index) const
	{
		return UIManager::Instance().GetFont(fonts_[index]);
	}

	uint32_t UIDialog::GetFontSize(size_t index) const
	{
		return UIManager::Instance().GetFontSize(fonts_[index]);
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

	void UIDialog::DrawQuad(UIManager::VertexFormat const * vertices, float depth, TexturePtr const & texture)
	{
		float3 offset = float3(static_cast<float>(bounding_box_.left()), static_cast<float>(bounding_box_.top()), depth_base_ + depth);
		if (this->IsCaptionEnabled())
		{
			offset.y() += this->GetCaptionHeight();
		}

		UIManager::Instance().DrawQuad(offset, vertices, texture);
	}

	void UIDialog::DrawSprite(UIElement const & element, Rect_T<int32_t> const & rcDest, float depth_bias)
	{
		// No need to draw fully transparent layers
		if (0 == element.TextureColor().Current.a())
		{
			return;
		}

		Rect_T<int32_t> rcTexture = element.TexRect();
		Rect_T<int32_t> rcScreen = rcDest + this->GetLocation();
		TexturePtr const & tex = UIManager::Instance().GetTexture(element.TextureIndex());

		// If caption is enabled, offset the Y position by its height.
		if (this->IsCaptionEnabled())
		{
			rcScreen += Vector_T<int32_t, 2>(0, this->GetCaptionHeight());
		}

		float3 pos(static_cast<float>(rcScreen.left()), static_cast<float>(rcScreen.top()), depth_base_ + depth_bias);
		std::vector<Color> clrs(4, element.TextureColor().Current);
		UIManager::Instance().DrawRect(pos, static_cast<float>(rcScreen.Width()),
			static_cast<float>(rcScreen.Height()), &clrs[0], rcTexture, tex);
	}

	void UIDialog::DrawText(std::wstring const & strText, UIElement const & uie, Rect_T<int32_t> const & rc, bool bShadow, float depth_bias)
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

			UIManager::Instance().DrawText(strText, uie.FontIndex(), r, depth_base_ + depth_bias - 0.01f,
				Color(0, 0, 0, uie.FontColor().Current.a()), uie.TextAlign());
		}

		Rect_T<int32_t> r = rc;
		r += this->GetLocation();
		if (this->IsCaptionEnabled())
		{
			r += Vector_T<int32_t, 2>(0, this->GetCaptionHeight());
		}

		UIManager::Instance().DrawText(strText, uie.FontIndex(), r, depth_base_ + depth_bias - 0.01f,
			uie.FontColor().Current, uie.TextAlign());
	}

	Size_T<uint32_t> UIDialog::CalcSize(std::wstring const & strText, UIElement const & uie, Rect_T<int32_t> const & rc, bool bShadow)
	{
		Rect_T<int32_t> r = rc;
		r += this->GetLocation();
		if (this->IsCaptionEnabled())
		{
			r += Vector_T<int32_t, 2>(0, this->GetCaptionHeight());
		}

		Size_T<uint32_t> size = UIManager::Instance().CalcSize(strText, uie.FontIndex(), r, uie.TextAlign());
		if (bShadow)
		{
			size.cx() += 1;
			size.cy() += 1;
		}
		return size;
	}

	// Initialize default Elements
	void UIDialog::InitDefaultElements()
	{
		this->SetFont(0, Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont"), 12);

		// Element for the caption
		cap_element_.SetFont(0);
		cap_element_.SetTexture(static_cast<uint32_t>(tex_index_), Rect_T<int32_t>(17, 269, 241, 287));
		cap_element_.TextureColor().States[UICS_Normal] = Color(0.4f, 0.6f, 0.4f, 1);
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

	void UIDialog::AddIDName(std::string const & name, int id)
	{
		id_name_.insert(std::make_pair(name, id));
	}

	int UIDialog::IDFromName(std::string const & name)
	{
		return id_name_[name];
	}

	void UIDialog::CtrlLocation(int id, UIDialog::ControlLocation const & loc)
	{
		id_location_.insert(std::make_pair(id, loc));
	}

	UIDialog::ControlLocation const & UIDialog::CtrlLocation(int id)
	{
		return id_location_[id];
	}

	void UIDialog::SettleCtrls(uint32_t width, uint32_t height)
	{
		BOOST_FOREACH(BOOST_TYPEOF(id_location_)::reference id_loc, id_location_)
		{
			int x = id_loc.second.x;
			int y = id_loc.second.y;

			uint32_t w = id_loc.first < 0 ? width : this->GetWidth();
			uint32_t h = id_loc.first < 0 ? height : this->GetHeight();

			switch (id_loc.second.align_x)
			{
			case CA_Left:
				break;

			case CA_Right:
				x = w + x;
				break;

			case CA_Center:
				x = w / 2 + x;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			switch (id_loc.second.align_y)
			{
			case CA_Top:
				break;

			case CA_Bottom:
				y = h + y;
				break;

			case CA_Middle:
				y = h / 2 + y;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			if (id_loc.first < 0)
			{
				this->SetLocation(x, y);
			}
			else
			{
				this->GetControl(id_loc.first)->SetLocation(x, y);
			}
		}
	}

	void UIDialog::KeyDownHandler(wchar_t key)
	{
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled())
		{
			control_focus_.lock()->KeyDownHandler(*this, key);
		}
		else
		{
			bool handled = false;
			if (!control_focus_.lock() || (control_focus_.lock()->GetType() != UICT_EditBox))
			{
				// See if this matches a control's hotkey
				// Activate the hotkey if the focus doesn't belong to an
				// edit box.
				BOOST_FOREACH(BOOST_TYPEOF(controls_)::reference control, controls_)
				{
					if (control->GetHotkey() == static_cast<uint8_t>(key))
					{
						control->OnHotkey();
						handled = true;
						break;
					}
				}
			}

			// Not yet handled, check for focus messages
			if (!handled)
			{
				// If keyboard input is not enabled, this message should be ignored
				if ((KS_RightArrow == key) || (KS_DownArrow == key))
				{
					if (control_focus_.lock())
					{
						this->OnCycleFocus(true);
					}
				}

				if ((KS_LeftArrow == key) || (KS_UpArrow == key))
				{
					if (control_focus_.lock())
					{
						this->OnCycleFocus(false);
					}
				}

				if (KS_Tab == key)
				{
					bool shift_down = (KS_LeftShift == key) | (KS_RightShift == key);
					this->OnCycleFocus(!shift_down);
				}
			}
		}
	}

	void UIDialog::KeyUpHandler(wchar_t key)
	{
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled())
		{
			control_focus_.lock()->KeyUpHandler(*this, key);
		}
	}

	void UIDialog::MouseDownHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
	{
		Vector_T<int32_t, 2> pt_local = pt;
		pt_local -= this->GetLocation();

		UIControlPtr control;
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
			&& control_focus_.lock()->ContainsPoint(pt))
		{
			control = control_focus_.lock();
		}
		else
		{
			// Figure out which control the mouse is over now
			control = this->GetControlAtPoint(pt);
		}

		if (control)
		{
			control->MouseDownHandler(*this, buttons, pt);
		}
		else
		{
			if ((buttons & MB_Left) && control_focus_.lock())
			{
				control_focus_.lock()->OnFocusOut();
				control_focus_.reset();
			}
		}
	}

	void UIDialog::MouseUpHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
	{
		UIControlPtr control;
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
			&& control_focus_.lock()->ContainsPoint(pt))
		{
			control = control_focus_.lock();
		}
		else
		{
			// Figure out which control the mouse is over now
			control = this->GetControlAtPoint(pt);
		}

		if (control)
		{
			control->MouseUpHandler(*this, buttons, pt);
		}
		else
		{
			if ((buttons & MB_Left) && control_focus_.lock())
			{
				control_focus_.lock()->OnFocusOut();
				control_focus_.reset();
			}
		}
	}

	void UIDialog::MouseWheelHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt, int32_t z_delta)
	{
		UIControlPtr control;
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
			&& control_focus_.lock()->ContainsPoint(pt))
		{
			control = control_focus_.lock();
		}
		else
		{
			// Figure out which control the mouse is over now
			control = this->GetControlAtPoint(pt);
		}

		if (control)
		{
			control->MouseWheelHandler(*this, buttons, pt, z_delta);
		}
		else
		{
			if ((buttons & MB_Left) && control_focus_.lock())
			{
				control_focus_.lock()->OnFocusOut();
				control_focus_.reset();
			}
		}
	}

	void UIDialog::MouseOverHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
	{
		UIControlPtr control;
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
			&& ((buttons & MB_Left) || control_focus_.lock()->ContainsPoint(pt)))
		{
			control = control_focus_.lock();
		}
		else
		{
			// Figure out which control the mouse is over now
			control = this->GetControlAtPoint(pt);

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
			control->MouseOverHandler(*this, buttons, pt);
		}
	}
}
