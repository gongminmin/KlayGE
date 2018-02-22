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
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
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
#include <KFL/XMLDom.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/TransientBuffer.hpp>
#include <KFL/Hash.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <cstring>
#include <fstream>
#include <mutex>

#include <KlayGE/UI.hpp>

namespace
{
	std::mutex singleton_mutex;

	bool BoolFromStr(std::string_view name)
	{
		if (("true" == name) || ("1" == name))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool ReadBool(KlayGE::XMLNodePtr& node, std::string const & name, bool default_val)
	{
		bool ret = default_val;

		KlayGE::XMLAttributePtr attr = node->Attrib(name);
		if (attr)
		{
			ret = BoolFromStr(attr->ValueString());
		}

		return ret;
	}

	enum
	{
		Key,
		Move,
		Wheel,
		Click,
		Touch
	};
}

namespace KlayGE
{
	std::unique_ptr<UIManager> UIManager::ui_mgr_instance_;


	class UIRectRenderable : public RenderableHelper
	{
	public:
		UIRectRenderable(TexturePtr const & texture, RenderEffectPtr const & effect)
			: RenderableHelper(L"UIRect"),
				texture_(texture)
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

			uint32_t const INDEX_PER_QUAD = restart_ ? 5 : 6;
			uint32_t const INIT_NUM_QUAD = 1024;
			tb_vb_ = MakeUniquePtr<TransientBuffer>(static_cast<uint32_t>(INIT_NUM_QUAD * 4 * sizeof(UIManager::VertexFormat)), TransientBuffer::BF_Vertex);
			tb_ib_ = MakeUniquePtr<TransientBuffer>(static_cast<uint32_t>(INIT_NUM_QUAD * INDEX_PER_QUAD * sizeof(uint16_t)), TransientBuffer::BF_Index);

			rl_->BindVertexStream(tb_vb_->GetBuffer(), { VertexElement(VEU_Position, 0, EF_BGR32F),
				VertexElement(VEU_Diffuse, 0, EF_ABGR32F), VertexElement(VEU_TextureCoord, 0, EF_GR32F) });
			rl_->BindIndexStream(tb_ib_->GetBuffer(), EF_R16UI);

			effect_ = effect;
			if (texture)
			{
				technique_ = effect->TechniqueByName("UITec");
			}
			else
			{
				technique_ = effect->TechniqueByName("UITecNoTex");
			}

			ui_tex_ep_ = effect->ParameterByName("ui_tex");
			half_width_height_ep_ = effect->ParameterByName("half_width_height");
			dpi_scale_ep_ = effect->ParameterByName("dpi_scale");
		}

		bool Empty() const
		{
			return tb_ib_sub_allocs_.empty();
		}

		void OnRenderBegin()
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			*ui_tex_ep_ = texture_;

			RenderEngine& re = rf.RenderEngineInstance();
			float const half_width = re.CurFrameBuffer()->Width() / 2.0f;
			float const half_height = re.CurFrameBuffer()->Height() / 2.0f;

			*half_width_height_ep_ = float2(half_width, half_height);
			*dpi_scale_ep_ = Context::Instance().AppInstance().MainWnd()->DPIScale();

			tb_vb_->EnsureDataReady();
			tb_ib_->EnsureDataReady();

			rl_->SetVertexStream(0, tb_vb_->GetBuffer());
			rl_->BindIndexStream(tb_ib_->GetBuffer(), EF_R16UI);
		}
		
		void OnRenderEnd()
		{
			tb_vb_->OnPresent();
			tb_ib_->OnPresent();

			tb_vb_sub_allocs_.clear();
			tb_ib_sub_allocs_.clear();
		}

		void Render()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			this->OnRenderBegin();

			BOOST_ASSERT(tb_vb_sub_allocs_.size() == tb_ib_sub_allocs_.size());

			for (size_t i = 0; i < tb_vb_sub_allocs_.size(); ++ i)
			{
				uint32_t vert_length = tb_vb_sub_allocs_[i].length_;
				uint32_t const ind_offset = tb_ib_sub_allocs_[i].offset_;
				uint32_t ind_length = tb_ib_sub_allocs_[i].length_;

				while ((i + 1 < tb_vb_sub_allocs_.size())
					&& (tb_vb_sub_allocs_[i].offset_ + tb_vb_sub_allocs_[i].length_ == tb_vb_sub_allocs_[i + 1].offset_)
					&& (tb_ib_sub_allocs_[i].offset_ + tb_ib_sub_allocs_[i].length_ == tb_ib_sub_allocs_[i + 1].offset_))
				{
					vert_length += tb_vb_sub_allocs_[i + 1].length_;
					ind_length += tb_ib_sub_allocs_[i + 1].length_;
					++ i;
				}

				rl_->NumVertices(vert_length / sizeof(UIManager::VertexFormat));
				rl_->StartIndexLocation(ind_offset / sizeof(uint16_t));
				rl_->NumIndices(ind_length / sizeof(uint16_t));

				re.Render(*this->GetRenderEffect(), *this->GetRenderTechnique(), *rl_);
			}

			for (size_t i = 0; i < tb_vb_sub_allocs_.size(); ++ i)
			{
				tb_vb_->Dealloc(tb_vb_sub_allocs_[i]);
				tb_ib_->Dealloc(tb_ib_sub_allocs_[i]);
			}

			this->OnRenderEnd();
		}

		void AddQuad(std::vector<UIManager::VertexFormat> const & vertices)
		{
			tb_vb_sub_allocs_.push_back(tb_vb_->Alloc(static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])), &vertices[0]));
			
			uint16_t const last_index = static_cast<uint16_t>(tb_vb_sub_allocs_.back().offset_ / sizeof(UIManager::VertexFormat));
			std::vector<uint16_t> indices;
			indices.resize(restart_ ? 5 : 6);
			indices[0] = last_index + 0;
			indices[1] = last_index + 1;
			if (restart_)
			{
				indices[2] = last_index + 3;
				indices[3] = last_index + 2;
				indices[4] = 0xFFFF;
			}
			else
			{
				indices[2] = last_index + 2;
				indices[3] = last_index + 2;
				indices[4] = last_index + 3;
				indices[5] = last_index + 0;
			}
			BOOST_ASSERT(last_index + 3 <= 0xFFFF);

			tb_ib_sub_allocs_.push_back(tb_ib_->Alloc(static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]));
		}

	private:
		bool restart_;

		RenderEffectParameter* dpi_scale_ep_;
		RenderEffectParameter* ui_tex_ep_;
		RenderEffectParameter* half_width_height_ep_;

		TexturePtr texture_;

		std::unique_ptr<TransientBuffer> tb_vb_;
		std::unique_ptr<TransientBuffer> tb_ib_;
		std::vector<SubAlloc> tb_vb_sub_allocs_;
		std::vector<SubAlloc> tb_ib_sub_allocs_;
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


	void UIElement::SetTexture(uint32_t tex_index, IRect const & tex_rect, Color const & default_texture_color)
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


	UIManager::UIManager()
		: mouse_on_ui_(false),
			inited_(false)
	{
	}

	UIManager::~UIManager()
	{
	}
	
	UIManager& UIManager::Instance()
	{
		if (!ui_mgr_instance_)
		{
			std::lock_guard<std::mutex> lock(singleton_mutex);
			if (!ui_mgr_instance_)
			{
				ui_mgr_instance_ = MakeUniquePtr<UIManager>();
			}
		}

		return *ui_mgr_instance_;
	}

	void UIManager::Destroy()
	{
		ui_mgr_instance_.reset();
	}

	void UIManager::Suspend()
	{
		// TODO
	}

	void UIManager::Resume()
	{
		// TODO
	}

	void UIManager::Init()
	{
		effect_ = SyncLoadRenderEffect("UI.fxml");

		elem_texture_rcs_[UICT_Button].push_back(IRect(0, 0, 136, 54));
		elem_texture_rcs_[UICT_Button].push_back(IRect(136, 0, 252, 54));

		elem_texture_rcs_[UICT_CheckBox].push_back(IRect(0, 54, 27, 81));
		elem_texture_rcs_[UICT_CheckBox].push_back(IRect(27, 54, 54, 81));

		elem_texture_rcs_[UICT_RadioButton].push_back(IRect(54, 54, 81, 81));
		elem_texture_rcs_[UICT_RadioButton].push_back(IRect(81, 54, 108, 81));

		elem_texture_rcs_[UICT_Slider].push_back(IRect(1, 187, 93, 228));
		elem_texture_rcs_[UICT_Slider].push_back(IRect(151, 193, 192, 234));

		elem_texture_rcs_[UICT_ScrollBar].push_back(IRect(196, 212, 218, 223));
		elem_texture_rcs_[UICT_ScrollBar].push_back(IRect(196, 192, 218, 212));
		elem_texture_rcs_[UICT_ScrollBar].push_back(IRect(196, 223, 218, 244));
		elem_texture_rcs_[UICT_ScrollBar].push_back(IRect(220, 192, 238, 234));

		elem_texture_rcs_[UICT_ListBox].push_back(IRect(13, 123, 241, 160));
		elem_texture_rcs_[UICT_ListBox].push_back(IRect(16, 166, 240, 183));

		elem_texture_rcs_[UICT_ComboBox].push_back(IRect(7, 81, 247, 123));
		elem_texture_rcs_[UICT_ComboBox].push_back(IRect(98, 189, 151, 238));
		elem_texture_rcs_[UICT_ComboBox].push_back(IRect(13, 123, 241, 160));
		elem_texture_rcs_[UICT_ComboBox].push_back(IRect(12, 163, 239, 183));

		elem_texture_rcs_[UICT_EditBox].push_back(IRect(14, 90, 241, 113));
		elem_texture_rcs_[UICT_EditBox].push_back(IRect(8, 82, 14, 90));
		elem_texture_rcs_[UICT_EditBox].push_back(IRect(14, 82, 241, 90));
		elem_texture_rcs_[UICT_EditBox].push_back(IRect(241, 82, 246, 90));
		elem_texture_rcs_[UICT_EditBox].push_back(IRect(8, 90, 14, 113));
		elem_texture_rcs_[UICT_EditBox].push_back(IRect(241, 90, 246, 113));
		elem_texture_rcs_[UICT_EditBox].push_back(IRect(8, 113, 14, 121));
		elem_texture_rcs_[UICT_EditBox].push_back(IRect(14, 113, 241, 121));
		elem_texture_rcs_[UICT_EditBox].push_back(IRect(241, 113, 246, 121));

		elem_texture_rcs_[UICT_TexButton].push_back(IRect(136, 0, 141, 5));
		elem_texture_rcs_[UICT_TexButton].push_back(IRect(141, 0, 247, 5));
		elem_texture_rcs_[UICT_TexButton].push_back(IRect(247, 0, 252, 5));
		elem_texture_rcs_[UICT_TexButton].push_back(IRect(136, 5, 141, 49));
		elem_texture_rcs_[UICT_TexButton].push_back(IRect(141, 5, 247, 49));
		elem_texture_rcs_[UICT_TexButton].push_back(IRect(247, 5, 252, 49));
		elem_texture_rcs_[UICT_TexButton].push_back(IRect(136, 49, 141, 54));
		elem_texture_rcs_[UICT_TexButton].push_back(IRect(141, 49, 247, 54));
		elem_texture_rcs_[UICT_TexButton].push_back(IRect(247, 49, 252, 54));

		InputActionDefine actions[] =
		{
			InputActionDefine(Key, KS_AnyKey),
			InputActionDefine(Move, MS_X),
			InputActionDefine(Move, MS_Y),
			InputActionDefine(Wheel, MS_Z),
			InputActionDefine(Click, MS_Button0),
			InputActionDefine(Touch, TS_AnyTouch)
		};

		InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
		InputActionMap actionMap;
		actionMap.AddActions(actions, actions + std::size(actions));

		action_handler_t input_handler = MakeSharedPtr<input_signal>();
		input_handler->connect(
			[this](InputEngine const & sender, InputAction const & action)
			{
				this->InputHandler(sender, action);
			});
		inputEngine.ActionMap(actionMap, input_handler);
	}

	void UIManager::Load(ResIdentifierPtr const & source)
	{
		if (source)
		{
			if (!inited_)
			{
				this->Init();
				inited_ = true;
			}

			XMLDocument doc;
			XMLNodePtr root = doc.Parse(source);

			XMLAttributePtr attr;

			std::vector<std::unique_ptr<XMLDocument>> include_docs;
			for (XMLNodePtr node = root->FirstNode("include"); node;)
			{
				attr = node->Attrib("name");
				include_docs.push_back(MakeUniquePtr<XMLDocument>());
				XMLNodePtr include_root = include_docs.back()->Parse(ResLoader::Instance().Open(std::string(attr->ValueString())));

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
					std::string_view const id = node->AttribString("id", "");
					std::string_view const caption = node->AttribString("caption", "");
					std::string_view const skin = node->AttribString("skin", "");
					x = node->Attrib("x")->ValueInt();
					y = node->Attrib("y")->ValueInt();
					width = node->Attrib("width")->ValueInt();
					height = node->Attrib("height")->ValueInt();
					attr = node->Attrib("align_x");
					if (attr)
					{
						std::string_view const align_x_str = attr->ValueString();
						if ("left" == align_x_str)
						{
							align_x = UIDialog::CA_Left;
						}
						else if ("right" == align_x_str)
						{
							align_x = UIDialog::CA_Right;
						}
						else
						{
							BOOST_ASSERT("center" == align_x_str);
							align_x = UIDialog::CA_Center;
						}
					}
					attr = node->Attrib("align_y");
					if (attr)
					{
						std::string_view const align_y_str = attr->ValueString();
						if ("top" == align_y_str)
						{
							align_y = UIDialog::CA_Top;
						}
						else if ("bottom" == align_y_str)
						{
							align_y = UIDialog::CA_Bottom;
						}
						else
						{
							BOOST_ASSERT("middle" == align_y_str);
							align_y = UIDialog::CA_Middle;
						}
					}

					TexturePtr tex;
					if (!skin.empty())
					{
						tex = SyncLoadTexture(std::string(skin), EAH_GPU_Read | EAH_Immutable);
					}
					dlg = this->MakeDialog(tex);
					dlg->SetID(std::string(id));
					std::wstring wcaption;
					Convert(wcaption, caption);
					dlg->SetCaptionText(wcaption);

					dlg->EnableCaption(ReadBool(node, "show_caption", true));
					dlg->AlwaysInOpacity(ReadBool(node, "opacity", false));

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

				std::vector<std::string_view> ctrl_ids;
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
						std::string_view id_str = ctrl_node->Attrib("id")->ValueString();
						id = static_cast<uint32_t>(std::find(ctrl_ids.begin(), ctrl_ids.end(), id_str) - ctrl_ids.begin());
						dlg->AddIDName(std::string(id_str), id);
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
						std::string_view const align_x_str = attr->ValueString();
						if ("left" == align_x_str)
						{
							align_x = UIDialog::CA_Left;
						}
						else if ("right" == align_x_str)
						{
							align_x = UIDialog::CA_Right;
						}
						else
						{
							BOOST_ASSERT("center" == align_x_str);
							align_x = UIDialog::CA_Center;
						}
					}
					attr = ctrl_node->Attrib("align_y");
					if (attr)
					{
						std::string_view const align_y_str = attr->ValueString();
						if ("top" == align_y_str)
						{
							align_y = UIDialog::CA_Top;
						}
						else if ("bottom" == align_y_str)
						{
							align_y = UIDialog::CA_Bottom;
						}
						else
						{
							BOOST_ASSERT("middle" == align_y_str);
							align_y = UIDialog::CA_Middle;
						}
					}

					{
						UIDialog::ControlLocation loc = { x, y, align_x, align_y };
						dlg->CtrlLocation(id, loc);
					}

					std::string_view const type_str = ctrl_node->Attrib("type")->ValueString();
					size_t const type_str_hash = HashRange(type_str.begin(), type_str.end());
					if (CT_HASH("static") == type_str_hash)
					{
						std::string_view const caption = ctrl_node->Attrib("caption")->ValueString();
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UIStatic>(dlg, id, wcaption,
							int4(x, y, width, height), is_default));
					}
					else if (CT_HASH("button") == type_str_hash)
					{
						std::string_view const caption = ctrl_node->Attrib("caption")->ValueString();
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UIButton>(dlg, id, wcaption,
							int4(x, y, width, height), hotkey, is_default));
					}
					else if (CT_HASH("tex_button") == type_str_hash)
					{
						TexturePtr tex;
						attr = ctrl_node->Attrib("texture");
						if (attr)
						{
							std::string_view const tex_name = attr->ValueString();
							tex = SyncLoadTexture(std::string(tex_name), EAH_GPU_Read | EAH_Immutable);
						}
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						dlg->AddControl(MakeSharedPtr<UITexButton>(dlg, id, tex,
							int4(x, y, width, height), hotkey, is_default));
					}
					else if (CT_HASH("check_box") == type_str_hash)
					{
						std::string_view const caption = ctrl_node->Attrib("caption")->ValueString();
						bool checked = ReadBool(ctrl_node, "checked", false);
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UICheckBox>(dlg, id, wcaption,
							int4(x, y, width, height), checked, hotkey, is_default));
					}
					else if (CT_HASH("radio_button") == type_str_hash)
					{
						std::string_view const caption = ctrl_node->Attrib("caption")->ValueString();
						int32_t button_group = ctrl_node->Attrib("button_group")->ValueInt();
						bool checked = ReadBool(ctrl_node, "checked", false);
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UIRadioButton>(dlg, id, button_group, wcaption,
							int4(x, y, width, height), checked, hotkey, is_default));
					}
					else if (CT_HASH("slider") == type_str_hash)
					{
						int32_t min_v = ctrl_node->AttribInt("min", 0);
						int32_t max_v = ctrl_node->AttribInt("max", 100);
						int32_t value = ctrl_node->AttribInt("value", 50);
						dlg->AddControl(MakeSharedPtr<UISlider>(dlg, id,
							int4(x, y, width, height), min_v, max_v, value, is_default));
					}
					else if (CT_HASH("scroll_bar") == type_str_hash)
					{
						int32_t track_start = ctrl_node->AttribInt("track_start", 0);
						int32_t track_end = ctrl_node->AttribInt("track_end", 1);
						int32_t track_pos = ctrl_node->AttribInt("track_pos", 1);
						int32_t page_size = ctrl_node->AttribInt("page_size", 1);
						dlg->AddControl(MakeSharedPtr<UIScrollBar>(dlg, id,
							int4(x, y, width, height), track_start, track_end, track_pos, page_size));
					}
					else if (CT_HASH("list_box") == type_str_hash)
					{
						UIListBox::STYLE style = UIListBox::SINGLE_SELECTION;
						attr = ctrl_node->Attrib("style");
						if (attr)
						{
							std::string_view const style_str = attr->ValueString();
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
							int4(x, y, width, height), style ? UIListBox::SINGLE_SELECTION : UIListBox::MULTI_SELECTION));

						for (XMLNodePtr item_node = ctrl_node->FirstNode("item"); item_node; item_node = item_node->NextSibling("item"))
						{
							std::string_view const caption = item_node->Attrib("name")->ValueString();
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
					else if (CT_HASH("combo_box") == type_str_hash)
					{
						uint8_t hotkey = static_cast<uint8_t>(ctrl_node->AttribInt("hotkey", 0));
						dlg->AddControl(MakeSharedPtr<UIComboBox>(dlg, id,
							int4(x, y, width, height), hotkey, is_default));

						for (XMLNodePtr item_node = ctrl_node->FirstNode("item"); item_node; item_node = item_node->NextSibling("item"))
						{
							std::string_view const caption = item_node->Attrib("name")->ValueString();
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
					else if (CT_HASH("edit_box") == type_str_hash)
					{
						std::string_view const caption = ctrl_node->Attrib("caption")->ValueString();
						std::wstring wcaption;
						Convert(wcaption, caption);
						dlg->AddControl(MakeSharedPtr<UIEditBox>(dlg, id, wcaption,
							int4(x, y, width, height), is_default));
					}
					else if (CT_HASH("polyline_edit_box") == type_str_hash)
					{
						Color line_clr(0, 1, 0, 1);
						line_clr.r() = ctrl_node->AttribFloat("line_r", 0);
						line_clr.g() = ctrl_node->AttribFloat("line_g", 1);
						line_clr.b() = ctrl_node->AttribFloat("line_b", 0);
						line_clr.a() = ctrl_node->AttribFloat("line_a", 1);
						dlg->AddControl(MakeSharedPtr<UIPolylineEditBox>(dlg, id,
							int4(x, y, width, height), is_default));
						dlg->Control<UIPolylineEditBox>(id)->SetColor(line_clr);
					}
					else if (CT_HASH("progress_bar") == type_str_hash)
					{
						int32_t progress = ctrl_node->AttribInt("value", 0);
						dlg->AddControl(MakeSharedPtr<UIProgressBar>(dlg, id, progress,
							int4(x, y, width, height), is_default));
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

	size_t UIManager::AddFont(FontPtr const & font, float font_size)
	{
		font_cache_.emplace_back(font, font_size);
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

	float UIManager::GetFontSize(size_t index) const
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

	UIDialogPtr const & UIManager::GetDialog(std::string_view id) const
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
		for (auto& str : strings_)
		{
			str.second.clear();
		}

		for (auto const & dialog : dialogs_)
		{
			dialog->Render();
		}

		for (auto const & rect : rects_)
		{
			if (!checked_pointer_cast<UIRectRenderable>(rect.second)->Empty())
			{
				SceneObjectHelperPtr ui_rect_obj
					= MakeSharedPtr<SceneObjectHelper>(rect.second, SceneObject::SOA_Overlay);
				ui_rect_obj->AddToSceneManager();
			}
		}
		for (auto const & str : strings_)
		{
			auto const & font = font_cache_[str.first];
			for (auto const & s : str.second)
			{
				font.first->RenderText(s.rc, s.depth, 1, 1, s.clr, s.text, font.second, s.align);
			}
		}
	}

	void UIManager::DrawRect(float3 const & pos, float width, float height, Color const * clrs,
				IRect const & rcTexture, TexturePtr const & texture)
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

		std::shared_ptr<UIRectRenderable> renderable;
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

		std::vector<VertexFormat> vertices(4);
		vertices[0] = VertexFormat(pos + float3(0, 0, 0),
			clrs[0], float2(texcoord.left(), texcoord.top()));
		vertices[1] = VertexFormat(pos + float3(width, 0, 0),
			clrs[1], float2(texcoord.right(), texcoord.top()));
		vertices[2] = VertexFormat(pos + float3(width, height, 0),
			clrs[2], float2(texcoord.right(), texcoord.bottom()));
		vertices[3] = VertexFormat(pos + float3(0, height, 0),
			clrs[3], float2(texcoord.left(), texcoord.bottom()));

		renderable->AddQuad(vertices);
	}

	void UIManager::DrawQuad(float3 const & offset, VertexFormat const * vertices, TexturePtr const & texture)
	{
		std::shared_ptr<UIRectRenderable> renderable;
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

		std::vector<VertexFormat> verts(4);
		verts[0] = VertexFormat(offset + vertices[0].pos,
			vertices[0].clr, vertices[0].tex);
		verts[1] = VertexFormat(offset + vertices[1].pos,
			vertices[1].clr, vertices[1].tex);
		verts[2] = VertexFormat(offset + vertices[2].pos,
			vertices[2].clr, vertices[2].tex);
		verts[3] = VertexFormat(offset + vertices[3].pos,
			vertices[3].clr, vertices[3].tex);

		renderable->AddQuad(verts);
	}

	void UIManager::DrawString(std::wstring const & strText, uint32_t font_index,
		IRect const & rc, float depth, Color const & clr, uint32_t align)
	{
		strings_[font_index].push_back(string_cache());
		string_cache& sc = strings_[font_index].back();
		sc.rc = rc;
		sc.depth = depth;
		sc.clr = clr;
		sc.text = strText;
		sc.align = align;
	}

	Size_T<float> UIManager::CalcSize(std::wstring const & strText, uint32_t font_index,
		IRect const & /*rc*/, uint32_t /*align*/)
	{
		auto const & font = font_cache_[font_index];
		return font.first->CalcSize(strText, font.second);
	}

	void UIManager::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
	{
		switch (action.first)
		{
		case Key:
			switch (action.second->type)
			{
			case InputEngine::IDT_Keyboard:
				{
					InputKeyboardActionParamPtr param = checked_pointer_cast<InputKeyboardActionParam>(action.second);
					uint16_t shift_ctrl_alt = ((param->buttons_down[KS_LeftShift] || param->buttons_down[KS_RightShift]) ? MB_Shift : 0)
						| ((param->buttons_down[KS_LeftCtrl] || param->buttons_down[KS_RightCtrl]) ? MB_Ctrl : 0)
						| ((param->buttons_down[KS_LeftAlt] || param->buttons_down[KS_RightAlt]) ? MB_Alt : 0);
					for (auto const & dialog : dialogs_)
					{
						if (dialog->GetVisible())
						{
							for (uint32_t i = 0; i < 256; ++ i)
							{
								if (param->buttons_down[i])
								{
									dialog->KeyDownHandler(i | shift_ctrl_alt);
								}
								if (param->buttons_up[i])
								{
									dialog->KeyUpHandler(i | shift_ctrl_alt);
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
			break;

		case Move:
			switch (action.second->type)
			{
			case InputEngine::IDT_Mouse:
				{
					InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
					int2 abs_coord = param->abs_coord;
					float const dpi_scale = Context::Instance().AppInstance().MainWnd()->DPIScale();
					abs_coord.x() = static_cast<int32_t>(abs_coord.x() / dpi_scale);
					abs_coord.y() = static_cast<int32_t>(abs_coord.y() / dpi_scale);
					mouse_on_ui_ = false;
					for (auto const & dialog : dialogs_)
					{
						if (dialog->GetVisible() && dialog->ContainsPoint(abs_coord))
						{
							mouse_on_ui_ = true;
						}
						dialog->MouseOverHandler(param->buttons_state, abs_coord);
					}
				}
				break;

			default:
				break;
			}
			break;

		case Wheel:
			switch (action.second->type)
			{
			case InputEngine::IDT_Mouse:
				{
					InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
					int2 abs_coord = param->abs_coord;
					float const dpi_scale = Context::Instance().AppInstance().MainWnd()->DPIScale();
					abs_coord.x() = static_cast<int32_t>(abs_coord.x() / dpi_scale);
					abs_coord.y() = static_cast<int32_t>(abs_coord.y() / dpi_scale);
					mouse_on_ui_ = false;
					for (auto const & dialog : dialogs_)
					{
						if (dialog->GetVisible() && dialog->ContainsPoint(abs_coord))
						{
							mouse_on_ui_ = true;
						}
						dialog->MouseWheelHandler(param->buttons_state, abs_coord, param->wheel_delta);
					}
				}
				break;

			default:
				break;
			}
			break;

		case Click:
			switch (action.second->type)
			{
			case InputEngine::IDT_Mouse:
				{
					InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
					int2 abs_coord = param->abs_coord;
					float const dpi_scale = Context::Instance().AppInstance().MainWnd()->DPIScale();
					abs_coord.x() = static_cast<int32_t>(abs_coord.x() / dpi_scale);
					abs_coord.y() = static_cast<int32_t>(abs_coord.y() / dpi_scale);
					mouse_on_ui_ = false;
					for (auto const & dialog : dialogs_)
					{
						if (dialog->GetVisible() && dialog->ContainsPoint(abs_coord))
						{
							mouse_on_ui_ = true;
						}
						if (param->buttons_down & MB_Left)
						{
							dialog->MouseDownHandler(param->buttons_down, abs_coord);
						}
						else if (param->buttons_up & MB_Left)
						{
							dialog->MouseUpHandler(param->buttons_up, abs_coord);
						}
					}
				}
				break;

			default:
				break;
			}
			break;
		
		case Touch:
			switch (action.second->type)
			{
			case InputEngine::IDT_Touch:
				{
					InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);
					int2 abs_coord = param->touches_coord[0];
					float const dpi_scale = Context::Instance().AppInstance().MainWnd()->DPIScale();
					abs_coord.x() = static_cast<int32_t>(abs_coord.x() / dpi_scale);
					abs_coord.y() = static_cast<int32_t>(abs_coord.y() / dpi_scale);
					mouse_on_ui_ = false;
					for (auto const & dialog : dialogs_)
					{
						if (dialog->GetVisible() && dialog->ContainsPoint(abs_coord))
						{
							mouse_on_ui_ = true;
						}
						if (param->touches_down & 1UL)
						{
							dialog->MouseDownHandler(MB_Left, abs_coord);
						}
						else if (param->touches_up & 1UL)
						{
							dialog->MouseUpHandler(MB_Left, abs_coord);
						}
						else if (param->touches_state & 1UL)
						{
							dialog->MouseOverHandler(MB_Left, abs_coord);
						}
					}
				}
				break;

			default:
				break;
			}
			break;
		}
	}

	IRect const & UIManager::ElementTextureRect(uint32_t ctrl, uint32_t elem_index)
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

	void UIManager::SettleCtrls()
	{
		for (auto const & dialog : dialogs_)
		{
			dialog->SettleCtrls();
		}
	}


	UIDialog::UIDialog(TexturePtr const & control_tex)
			: keyboard_input_(false), mouse_input_(true),
					visible_(true), show_caption_(true),
					minimized_(false),
					bounding_box_(0, 0, 0, 0),
					caption_height_(18),
					top_left_clr_(0, 0, 0, 0), top_right_clr_(0, 0, 0, 0),
					bottom_left_clr_(0, 0, 0, 0), bottom_right_clr_(0, 0, 0, 0),
					opacity_(0.5f)
	{
		TexturePtr ct;
		if (control_tex)
		{
			ct = control_tex;
		}
		else
		{
			ct = SyncLoadTexture("ui.dds", EAH_GPU_Read | EAH_Immutable);
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
		for (auto const & control : controls_)
		{
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
		for (auto const & control : controls_)
		{
			if ((control->GetID() == ID) && (control->GetType() == type))
			{
				return control;
			}
		}

		// Not found
		static UIControlPtr ret;
		return ret;
	}

	UIControlPtr const & UIDialog::GetControlAtPoint(int2 const & pt) const
	{
		// Search through all child controls for the first one which
		// contains the mouse point
		for (auto const & control : controls_)
		{
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

		bool bBackgroundIsVisible = (top_left_clr_.a() != 0) || (top_right_clr_.a() != 0)
			|| (bottom_right_clr_.a() != 0) || (bottom_left_clr_.a() != 0);
		if (!minimized_ && bBackgroundIsVisible)
		{
			std::array<Color, 4> clrs;
			clrs[0] = top_left_clr_;
			clrs[1] = top_right_clr_;
			clrs[2] = bottom_right_clr_;
			clrs[3] = bottom_left_clr_;
			if (!always_in_opacity_)
			{
				clrs[0].a() *= opacity_;
				clrs[1].a() *= opacity_;
				clrs[2].a() *= opacity_;
				clrs[3].a() *= opacity_;
			}

			IRect rc(0, 0, this->GetWidth(), this->GetHeight());
			IRect rcScreen = rc + this->GetLocation();
			if (this->IsCaptionEnabled())
			{
				rcScreen += int2(0, this->GetCaptionHeight());
			}

			float3 pos(static_cast<float>(rcScreen.left()), static_cast<float>(rcScreen.top()), depth_base_);
			UIManager::Instance().DrawRect(pos, static_cast<float>(this->GetWidth()),
				static_cast<float>(this->GetHeight()), &clrs[0], IRect(0, 0, 0, 0), TexturePtr());
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
			IRect rc(0, -caption_height_, w, 0);

			UISize size = this->CalcSize(wstrOutput, cap_element_, rc, true);

			w = std::min(w, static_cast<int32_t>(size.cx() * 1.2f));
			rc.right() = w;

			Color clr = cap_element_.TextureColor().Current;
			if (!always_in_opacity_)
			{
				clr.a() *= opacity_;
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

			this->DrawString(wstrOutput, cap_element_, rc, true);
		}

		// If the dialog is minimized, skip rendering
		// its controls.
		if (!minimized_)
		{
			std::vector<std::vector<size_t>> intersected_groups;
			for (size_t i = 0; i < controls_.size(); ++ i)
			{
				for (size_t j = 0; j < i; ++ j)
				{
					IRect rc = controls_[i]->BoundingBoxRect() & controls_[j]->BoundingBoxRect();
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
				auto iter = std::lower_bound(intersected_controls.begin(), intersected_controls.end(), i);
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

	bool UIDialog::ContainsPoint(int2 const & pt) const
	{
		bool inside = bounding_box_.PtInRect(pt);
		if (!inside)
		{
			int2 const local_pt = this->ToLocal(pt);

			for (auto const & control : controls_)
			{
				if (control->ContainsPoint(local_pt))
				{
					inside = true;
					break;
				}
			}
		}
		return inside;
	}

	int2 UIDialog::ToLocal(int2 const & pt) const
	{
		int2 ret = pt - this->GetLocation();
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
		for (auto const & control : controls_)
		{
			if (UICT_RadioButton == control->GetType())
			{
				UIRadioButton* pRadioButton = checked_cast<UIRadioButton*>(control.get());
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
			UIControlPtr const & control = controls_[i];
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

		for (auto const & control : controls_)
		{
			control->Refresh();
		}

		if (keyboard_input_)
		{
			this->FocusDefaultControl();
		}
	}

	// Shared resource access. Indexed fonts and textures are shared among
	// all the controls.
	void UIDialog::SetFont(size_t index, FontPtr const & font, float font_size)
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

	float UIDialog::GetFontSize(size_t index) const
	{
		return UIManager::Instance().GetFontSize(fonts_[index]);
	}

	void UIDialog::FocusDefaultControl()
	{
		// Check for default control in this dialog
		for (auto const & control : controls_)
		{
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

	void UIDialog::DrawRect(IRect const & rc, float depth, Color const & clr)
	{
		IRect rcScreen = rc + this->GetLocation();
		if (this->IsCaptionEnabled())
		{
			rcScreen += int2(0, this->GetCaptionHeight());
		}

		float3 pos(static_cast<float>(rcScreen.left()), static_cast<float>(rcScreen.top()), depth_base_ + depth);
		std::array<Color, 4> clrs;
		clrs.fill(clr);
		if (!always_in_opacity_)
		{
			for (size_t i = 0; i < clrs.size(); ++ i)
			{
				clrs[i].a() *= opacity_;
			}
		}
		UIManager::Instance().DrawRect(pos, static_cast<float>(rcScreen.Width()), static_cast<float>(rc.Height()),
			&clrs[0], IRect(0, 0, 0, 0), TexturePtr());
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

	void UIDialog::DrawSprite(UIElement const & element, IRect const & rcDest, float depth_bias)
	{
		// No need to draw fully transparent layers
		if (0 == element.TextureColor().Current.a())
		{
			return;
		}

		IRect rcTexture = element.TexRect();
		IRect rcScreen = rcDest + this->GetLocation();
		TexturePtr const & tex = UIManager::Instance().GetTexture(element.TextureIndex());

		// If caption is enabled, offset the Y position by its height.
		if (this->IsCaptionEnabled())
		{
			rcScreen += int2(0, this->GetCaptionHeight());
		}

		float3 pos(static_cast<float>(rcScreen.left()), static_cast<float>(rcScreen.top()), depth_base_ + depth_bias);
		std::array<Color, 4> clrs;
		clrs.fill(element.TextureColor().Current);
		if (!always_in_opacity_)
		{
			for (size_t i = 0; i < clrs.size(); ++ i)
			{
				clrs[i].a() *= opacity_;
			}
		}
		UIManager::Instance().DrawRect(pos, static_cast<float>(rcScreen.Width()),
			static_cast<float>(rcScreen.Height()), &clrs[0], rcTexture, tex);
	}

	void UIDialog::DrawString(std::wstring const & strText, UIElement const & uie, IRect const & rc, bool bShadow, float depth_bias)
	{
		if (bShadow)
		{
			IRect rcShadow = rc;
			rcShadow += int2(1, 1);

			IRect r = rcShadow;
			r += this->GetLocation();
			if (this->IsCaptionEnabled())
			{
				r += int2(0, this->GetCaptionHeight());
			}

			float alpha = uie.FontColor().Current.a();
			if (!always_in_opacity_)
			{
				alpha *= opacity_;
			}
			UIManager::Instance().DrawString(strText, uie.FontIndex(), r, depth_base_ + depth_bias - 0.01f,
				Color(0, 0, 0, alpha), uie.TextAlign());
		}

		IRect r = rc;
		r += this->GetLocation();
		if (this->IsCaptionEnabled())
		{
			r += int2(0, this->GetCaptionHeight());
		}

		Color clr = uie.FontColor().Current;
		if (!always_in_opacity_)
		{
			clr.a() *= opacity_;
		}
		UIManager::Instance().DrawString(strText, uie.FontIndex(), r, depth_base_ + depth_bias - 0.01f,
			clr, uie.TextAlign());
	}

	UISize UIDialog::CalcSize(std::wstring const & strText, UIElement const & uie, IRect const & rc, bool bShadow)
	{
		IRect r = rc;
		r += this->GetLocation();
		if (this->IsCaptionEnabled())
		{
			r += int2(0, this->GetCaptionHeight());
		}

		UISize size = UIManager::Instance().CalcSize(strText, uie.FontIndex(), r, uie.TextAlign());
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
		this->SetFont(0, SyncLoadFont("gkai00mp.kfont"), 12);

		// Element for the caption
		cap_element_.SetFont(0);
		cap_element_.SetTexture(static_cast<uint32_t>(tex_index_), IRect(17, 269, 241, 287));
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
		id_name_.emplace(name, id);
	}

	int UIDialog::IDFromName(std::string const & name)
	{
		return id_name_[name];
	}

	void UIDialog::CtrlLocation(int id, UIDialog::ControlLocation const & loc)
	{
		id_location_.emplace(id, loc);
	}

	UIDialog::ControlLocation const & UIDialog::CtrlLocation(int id)
	{
		return id_location_[id];
	}

	void UIDialog::SettleCtrls()
	{
		float const dpi_scale = Context::Instance().AppInstance().MainWnd()->DPIScale();
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		uint32_t width = static_cast<uint32_t>(re.ScreenFrameBuffer()->Width() / dpi_scale);
		uint32_t height = static_cast<uint32_t>(re.ScreenFrameBuffer()->Height() / dpi_scale);

		for (auto const & id_loc : id_location_)
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
				KFL_UNREACHABLE("Invalid alignment mode");
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
				KFL_UNREACHABLE("Invalid alignment mode");
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

	void UIDialog::KeyDownHandler(uint32_t key)
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
				for (auto const & control : controls_)
				{
					if (control->GetHotkey() == static_cast<uint8_t>(key & 0xFF))
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
				if ((KS_RightArrow == (key & 0xFF)) || (KS_DownArrow == (key & 0xFF)))
				{
					if (control_focus_.lock())
					{
						this->OnCycleFocus(true);
					}
				}

				if ((KS_LeftArrow == (key & 0xFF)) || (KS_UpArrow == (key & 0xFF)))
				{
					if (control_focus_.lock())
					{
						this->OnCycleFocus(false);
					}
				}

				if (KS_Tab == (key & 0xFF))
				{
					this->OnCycleFocus(!(key & MB_Shift));
				}
			}
		}

		if (control_focus_.lock() || control_mouse_over_.lock())
		{
			opacity_ = 1.0f;
		}
		else
		{
			opacity_ = 0.5f;
		}
	}

	void UIDialog::KeyUpHandler(uint32_t key)
	{
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled())
		{
			control_focus_.lock()->KeyUpHandler(*this, key);
		}

		if (control_focus_.lock() || control_mouse_over_.lock())
		{
			opacity_ = 1.0f;
		}
		else
		{
			opacity_ = 0.5f;
		}
	}

	void UIDialog::MouseDownHandler(uint32_t buttons, int2 const & pt)
	{
		int2 const local_pt = this->ToLocal(pt);

		UIControlPtr control;
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
			&& control_focus_.lock()->ContainsPoint(local_pt))
		{
			control = control_focus_.lock();
		}
		else
		{
			// Figure out which control the mouse is over now
			control = this->GetControlAtPoint(local_pt);
		}

		if (control)
		{
			control->MouseDownHandler(*this, buttons, local_pt);
		}
		else
		{
			if ((buttons & MB_Left) && control_focus_.lock())
			{
				control_focus_.lock()->OnFocusOut();
				control_focus_.reset();
			}
		}

		if (this->ContainsPoint(pt) || control_focus_.lock() || control_mouse_over_.lock())
		{
			opacity_ = 1.0f;
		}
		else
		{
			opacity_ = 0.5f;
		}
	}

	void UIDialog::MouseUpHandler(uint32_t buttons, int2 const & pt)
	{
		int2 const local_pt = this->ToLocal(pt);

		UIControlPtr control;
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
			&& control_focus_.lock()->ContainsPoint(local_pt))
		{
			control = control_focus_.lock();
		}
		else
		{
			// Figure out which control the mouse is over now
			control = this->GetControlAtPoint(local_pt);
		}

		if (control)
		{
			control->MouseUpHandler(*this, buttons, local_pt);
		}
		else
		{
			if ((buttons & MB_Left) && control_focus_.lock())
			{
				control_focus_.lock()->OnFocusOut();
				control_focus_.reset();
			}
		}

		if (this->ContainsPoint(pt) || control_focus_.lock() || control_mouse_over_.lock())
		{
			opacity_ = 1.0f;
		}
		else
		{
			opacity_ = 0.5f;
		}
	}

	void UIDialog::MouseWheelHandler(uint32_t buttons, int2 const & pt, int32_t z_delta)
	{
		int2 const local_pt = this->ToLocal(pt);

		UIControlPtr control;
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
			&& control_focus_.lock()->ContainsPoint(local_pt))
		{
			control = control_focus_.lock();
		}
		else
		{
			// Figure out which control the mouse is over now
			control = this->GetControlAtPoint(local_pt);
		}

		if (control)
		{
			control->MouseWheelHandler(*this, buttons, local_pt, z_delta);
		}
		else
		{
			if ((buttons & MB_Left) && control_focus_.lock())
			{
				control_focus_.lock()->OnFocusOut();
				control_focus_.reset();
			}
		}

		if (this->ContainsPoint(pt) || control_focus_.lock() || control_mouse_over_.lock())
		{
			opacity_ = 1.0f;
		}
		else
		{
			opacity_ = 0.5f;
		}
	}

	void UIDialog::MouseOverHandler(uint32_t buttons, int2 const & pt)
	{
		int2 const local_pt = this->ToLocal(pt);

		UIControlPtr control;
		if (control_focus_.lock() && control_focus_.lock()->GetEnabled()
			&& ((buttons & MB_Left) || control_focus_.lock()->ContainsPoint(local_pt)))
		{
			control = control_focus_.lock();
		}
		else
		{
			// Figure out which control the mouse is over now
			control = this->GetControlAtPoint(local_pt);

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
			control->MouseOverHandler(*this, buttons, local_pt);
		}

		if (this->ContainsPoint(pt) || control_focus_.lock() || control_mouse_over_.lock())
		{
			opacity_ = 1.0f;
		}
		else
		{
			opacity_ = 0.5f;
		}
	}
}
