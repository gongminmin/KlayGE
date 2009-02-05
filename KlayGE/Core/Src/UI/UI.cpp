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
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
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

#include <KlayGE/Input.hpp>

#include <cstring>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/UI.hpp>

namespace
{
	using namespace KlayGE;

	std::string read_short_string(ResIdentifierPtr const & source)
	{
		uint8_t len;
		source->read(reinterpret_cast<char*>(&len), sizeof(len));
		std::string ret(len, '\0');
		source->read(&ret[0], len);

		return ret;
	}
}

namespace KlayGE
{
	UIManagerPtr UIManager::ui_mgr_instance_;


	class UIRectRenderable : public RenderableHelper
	{
	public:
		UIRectRenderable(TexturePtr texture, RenderEffectPtr effect)
			: RenderableHelper(L"UIRect")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
			rl_->BindVertexStream(vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F),
												vertex_element(VEU_Diffuse, 0, EF_ABGR32F),
												vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			ib_ = rf.MakeIndexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
			rl_->BindIndexStream(ib_, EF_R16);

			if (texture)
			{
				technique_ = effect->TechniqueByName("UITec");
				*(technique_->Effect().ParameterByName("ui_tex")) = texture;
			}
			else
			{
				technique_ = effect->TechniqueByName("UITecNoTex");
			}

			half_width_height_ep_ = technique_->Effect().ParameterByName("half_width_height");
			texel_to_pixel_offset_ep_ = technique_->Effect().ParameterByName("texel_to_pixel_offset");
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
			return vertices_;
		}

		std::vector<uint16_t>& Indices()
		{
			return indices_;
		}

		void OnRenderBegin()
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

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

			RenderEngine& re = rf.RenderEngineInstance();
			float const half_width = re.CurFrameBuffer()->Width() / 2.0f;
			float const half_height = re.CurFrameBuffer()->Height() / 2.0f;

			*half_width_height_ep_ = float2(half_width, half_height);

			float4 texel_to_pixel = re.TexelToPixelOffset();
			texel_to_pixel.x() /= half_width;
			texel_to_pixel.y() /= half_height;
			*texel_to_pixel_offset_ep_ = texel_to_pixel;
		}

		void Render()
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			this->OnRenderBegin();
			renderEngine.Render(*this->GetRenderTechnique(), *rl_);
			this->OnRenderEnd();
		}

	private:
		RenderEffectParameterPtr half_width_height_ep_;
		RenderEffectParameterPtr texel_to_pixel_offset_ep_;

		std::vector<UIManager::VertexFormat> vertices_;
		std::vector<uint16_t> indices_;

		GraphicsBufferPtr vb_;
		GraphicsBufferPtr ib_;
	};

	class UIRectObject : public SceneObjectHelper
	{
	public:
		UIRectObject(RenderablePtr renderable, uint32_t attrib)
			: SceneObjectHelper(renderable, attrib)
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

	void UIManager::Load(ResIdentifierPtr const & source)
	{
	#pragma pack(push, 1)
		struct uiml_header
		{
			uint32_t fourcc;
			uint32_t ver;
			uint32_t num_dlgs;
		};
	#pragma pack(pop)

		if (source)
		{
			uiml_header header;
			source->read(reinterpret_cast<char*>(&header), sizeof(header));
			BOOST_ASSERT((MakeFourCC<'U', 'I', 'M', 'L'>::value == header.fourcc));
			BOOST_ASSERT(1 == header.ver);

			for (uint32_t i = 0; i < header.num_dlgs; ++ i)
			{
				UIDialogPtr dlg;
				{
					std::string caption = read_short_string(source);
					std::string skin = read_short_string(source);
					TexturePtr tex;
					if (!skin.empty())
					{
						tex = LoadTexture(skin, EAH_GPU_Read)();
					}
					dlg = this->MakeDialog(tex);
					std::wstring wcaption;
					Convert(wcaption, caption);
					dlg->SetCaptionText(wcaption);
				}
				
				uint32_t num_ids;
				source->read(reinterpret_cast<char*>(&num_ids), sizeof(num_ids));
				for (uint32_t j = 0; j < num_ids; ++ j)
				{
					dlg->AddIdName(read_short_string(source), j);
				}

				uint32_t num_ctrls;
				source->read(reinterpret_cast<char*>(&num_ctrls), sizeof(num_ctrls));
				for (uint32_t j = 0; j < num_ctrls; ++ j)
				{
					uint32_t type, id;
					int32_t x, y;
					uint32_t width, height;
					bool is_default, align_x, align_y;
					source->read(reinterpret_cast<char*>(&type), sizeof(type));
					source->read(reinterpret_cast<char*>(&id), sizeof(id));
					source->read(reinterpret_cast<char*>(&x), sizeof(x));
					source->read(reinterpret_cast<char*>(&y), sizeof(y));
					source->read(reinterpret_cast<char*>(&width), sizeof(width));
					source->read(reinterpret_cast<char*>(&height), sizeof(height));
					source->read(reinterpret_cast<char*>(&is_default), sizeof(is_default));
					source->read(reinterpret_cast<char*>(&align_x), sizeof(align_x));
					source->read(reinterpret_cast<char*>(&align_y), sizeof(align_y));

					UIDialog::ControlLocation loc = { x, y,
						static_cast<UIDialog::ControlAlignment>(align_x), static_cast<UIDialog::ControlAlignment>(align_y) };
					dlg->CtrlLocation(id, loc);

					switch (type)
					{
					case UICT_Static:
						{
							std::string caption = read_short_string(source);
							std::wstring wcaption;
							Convert(wcaption, caption);
							dlg->AddControl(UIControlPtr(new UIStatic(dlg, id, wcaption,
								x, y, width, height, is_default)));
						}
						break;

					case UICT_Button:
						{
							std::string caption = read_short_string(source);
							uint32_t hotkey;
							source->read(reinterpret_cast<char*>(&hotkey), sizeof(hotkey));
							std::wstring wcaption;
							Convert(wcaption, caption);
							dlg->AddControl(UIControlPtr(new UIButton(dlg, id, wcaption,
								x, y, width, height, hotkey, is_default)));
						}
						break;

					case UICT_CheckBox:
						{
							std::string caption = read_short_string(source);
							bool checked;
							uint32_t hotkey;
							source->read(reinterpret_cast<char*>(&checked), sizeof(checked));
							source->read(reinterpret_cast<char*>(&hotkey), sizeof(hotkey));
							std::wstring wcaption;
							Convert(wcaption, caption);
							dlg->AddControl(UIControlPtr(new UICheckBox(dlg, id, wcaption,
								x, y, width, height, checked, hotkey, is_default)));
						}
						break;

					case UICT_RadioButton:
						{
							std::string caption = read_short_string(source);
							int32_t button_group;
							bool checked;
							uint32_t hotkey;
							source->read(reinterpret_cast<char*>(&button_group), sizeof(button_group));
							source->read(reinterpret_cast<char*>(&checked), sizeof(checked));
							source->read(reinterpret_cast<char*>(&hotkey), sizeof(hotkey));
							std::wstring wcaption;
							Convert(wcaption, caption);
							dlg->AddControl(UIControlPtr(new UIRadioButton(dlg, id, button_group, wcaption,
								x, y, width, height, checked, hotkey, is_default)));
						}
						break;

					case UICT_Slider:
						{
							int32_t min_v, max_v, value;
							source->read(reinterpret_cast<char*>(&min_v), sizeof(min_v));
							source->read(reinterpret_cast<char*>(&max_v), sizeof(max_v));
							source->read(reinterpret_cast<char*>(&value), sizeof(value));
							dlg->AddControl(UIControlPtr(new UISlider(dlg, id,
								x, y, width, height, min_v, max_v, value, is_default)));
						}
						break;

					case UICT_ScrollBar:
						{
							int32_t track_start, track_end, track_pos, page_size;
							source->read(reinterpret_cast<char*>(&track_start), sizeof(track_start));
							source->read(reinterpret_cast<char*>(&track_end), sizeof(track_end));
							source->read(reinterpret_cast<char*>(&track_pos), sizeof(track_pos));
							source->read(reinterpret_cast<char*>(&page_size), sizeof(page_size));
							dlg->AddControl(UIControlPtr(new UIScrollBar(dlg, id,
								x, y, width, height, track_start, track_end, track_pos, page_size)));
						}
						break;

					case UICT_ListBox:
						{
							bool style;
							source->read(reinterpret_cast<char*>(&style), sizeof(style));
							dlg->AddControl(UIControlPtr(new UIListBox(dlg, id,
								x, y, width, height, style ? UIListBox::SINGLE_SELECTION : UIListBox::MULTI_SELECTION)));

							uint32_t num_items;
							source->read(reinterpret_cast<char*>(&num_items), sizeof(num_items));
							for (uint32_t j = 0; j < num_items; ++ j)
							{
								std::string caption = read_short_string(source);
								std::wstring wcaption;
								Convert(wcaption, caption);
								dlg->Control<UIListBox>(id)->AddItem(wcaption);
							}
						}
						break;

					case UICT_ComboBox:
						{
							uint32_t hotkey;
							source->read(reinterpret_cast<char*>(&hotkey), sizeof(hotkey));
							dlg->AddControl(UIControlPtr(new UIComboBox(dlg, id,
								x, y, width, height, hotkey, is_default)));

							uint32_t num_items;
							source->read(reinterpret_cast<char*>(&num_items), sizeof(num_items));
							for (uint32_t j = 0; j < num_items; ++ j)
							{
								std::string caption = read_short_string(source);
								std::wstring wcaption;
								Convert(wcaption, caption);
								dlg->Control<UIComboBox>(id)->AddItem(wcaption);
							}
						}
						break;

					case UICT_EditBox:
						{
							std::string caption = read_short_string(source);
							std::wstring wcaption;
							Convert(wcaption, caption);
							dlg->AddControl(UIControlPtr(new UIEditBox(dlg, id, wcaption,
								x, y, width, height, is_default)));
						}
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
				}
			}
		}
	}

	UIDialogPtr UIManager::MakeDialog(TexturePtr control_tex)
	{
		UIDialogPtr ret = MakeSharedPtr<UIDialog>(control_tex);
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
				boost::shared_ptr<UIRectObject> ui_rect_obj = MakeSharedPtr<UIRectObject>(rect.second, SceneObject::SOA_ShortAge);
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
		indices.reserve(indices.size() + 6);

		uint16_t const last_index = static_cast<uint16_t>(vertices.size());
		indices.push_back(last_index + 0);
		indices.push_back(last_index + 1);
		indices.push_back(last_index + 2);
		indices.push_back(last_index + 2);
		indices.push_back(last_index + 3);
		indices.push_back(last_index + 0);

		vertices.push_back(VertexFormat(pos + float3(0, 0, 0),
			clrs[0], float2(texcoord.left(), texcoord.top())));
		vertices.push_back(VertexFormat(pos + float3(width, 0, 0),
			clrs[1], float2(texcoord.right(), texcoord.top())));
		vertices.push_back(VertexFormat(pos + float3(width, height, 0),
			clrs[2], float2(texcoord.right(), texcoord.bottom())));
		vertices.push_back(VertexFormat(pos + float3(0, height, 0),
			clrs[3], float2(texcoord.left(), texcoord.bottom())));
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

	void UIManager::HandleInput()
	{
		BOOST_FOREACH(BOOST_TYPEOF(dialogs_)::reference dialog, dialogs_)
		{
			if (dialog->GetVisible())
			{
				dialog->HandleInput();
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
			if (dialog->GetVisible())
			{
				dialog->SettleCtrls(width, height);
			}
		}
	}


	UIDialog::UIDialog(TexturePtr control_tex)
			: keyboard_input_(false), mouse_input_(true), default_control_id_(0xFFFF),
					visible_(true), show_caption_(false),
					minimized_(false), drag_(false),
					x_(0), y_(0), width_(0), height_(0),
					caption_height_(18),
					top_left_clr_(0, 0, 0, 0), top_right_clr_(0, 0, 0, 0),
					bottom_left_clr_(0, 0, 0, 0), bottom_right_clr_(0, 0, 0, 0)
	{
		if (!control_tex)
		{
			control_tex = LoadTexture("ui.dds", EAH_GPU_Read)();
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
	void UIDialog::SetFont(size_t index, FontPtr font, uint32_t font_size)
	{
		// Make sure the list is at least as large as the index being set
		if (index + 1 > fonts_.size())
		{
			fonts_.resize(index + 1, -1);
		}
		fonts_[index] = static_cast<int>(UIManager::Instance().AddFont(font, font_size));
	}

	FontPtr UIDialog::GetFont(size_t index) const
	{
		return UIManager::Instance().GetFont(fonts_[index]);
	}

	uint32_t UIDialog::GetFontSize(size_t index) const
	{
		return UIManager::Instance().GetFontSize(fonts_[index]);
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
		this->SetFont(0, Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont"), 12);

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

	void UIDialog::AddIdName(std::string const & name, int id)
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

			switch (id_loc.second.align_x)
			{
			case CA_Left:
				break;

			case CA_Right:
				x = width + x;
				break;

			case CA_Center:
				x = width / 2 + x;
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
				y = height + y;
				break;

			case CA_Middle:
				y = height / 2 + y;
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			this->GetControl(id_loc.first)->SetLocation(x, y);
		}
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
						if (key_board->KeyDown(j))
						{
							control_focus_.lock()->GetKeyDownEvent()(*this, arg);
						}
						if (key_board->KeyUp(j))
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
							if (key_board->KeyDown(j))
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

					if (mouse->ButtonDown(0) || mouse->ButtonDown(1) || mouse->ButtonDown(2))
					{
						control->GetMouseDownEvent()(*this, arg);
					}
					arg.buttons = UIControl::MB_None;
					if (mouse->ButtonUp(0))
					{
						arg.buttons |= UIControl::MB_Left;
					}
					if (mouse->ButtonUp(1))
					{
						arg.buttons |= UIControl::MB_Right;
					}
					if (mouse->ButtonUp(2))
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
					if ((mouse->ButtonDown(0) || mouse->ButtonUp(0)) && control_focus_.lock())
					{
						control_focus_.lock()->OnFocusOut();
						control_focus_.reset();
					}
				}
			}
		}
	}
}
