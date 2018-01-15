// UI.hpp
// KlayGE 图形用户界面 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2007-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 增加了TexButton (2009.4.12)
// 增加了PolylineEditBox (2009.4.18)
//
// 3.6.0
// 初次建立 (2007.6.27)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _UI_HPP
#define _UI_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX17/string_view.hpp>
#include <KFL/Timer.hpp>
#include <KlayGE/Input.hpp>

#include <array>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter 'sp'
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/signals2.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#include <KFL/CXX17/any.hpp>

namespace KlayGE
{
	enum UI_Control_State
	{
		UICS_Normal = 0,
		UICS_Disabled,
		UICS_Hidden,
		UICS_Focus,
		UICS_MouseOver,
		UICS_Pressed,

		UICS_Num_Control_States
	};

	enum UI_Control_Type
	{
		UICT_Static,
		UICT_Button,
		UICT_CheckBox,
		UICT_RadioButton,
		UICT_Slider,
		UICT_ScrollBar,
		UICT_ListBox,
		UICT_ComboBox,
		UICT_EditBox,
		UICT_TexButton,
		UICT_PolylineEditBox,
		UICT_ProgressBar,

		UICT_Num_Control_Types
	};

	struct KLAYGE_CORE_API UIStatesColor
	{
		void Init(Color const & default_color,
			Color const & disabled_color = Color(0.5f, 0.5f, 0.5f, 0.78f),
			Color const & hidden_color = Color(0, 0, 0, 0));

		void SetState(UI_Control_State state);

		Color States[UICS_Num_Control_States]; // Modulate colors for all possible control states
		Color Current;
	};

	class KLAYGE_CORE_API UIElement
	{
	public:
		void SetTexture(uint32_t tex_index, IRect const & tex_rect, Color const & default_texture_color = Color(1, 1, 1, 1));

		void SetFont(uint32_t font_index);
		void SetFont(uint32_t font_index, Color const & default_font_color);
		void SetFont(uint32_t font_index, Color const & default_font_color, uint32_t text_align);

		void Refresh();

		uint32_t TextureIndex() const
		{
			return tex_index_;
		}

		uint32_t FontIndex() const
		{
			return font_index_;
		}

		uint32_t TextAlign() const
		{
			return text_align_;
		}

		IRect const & TexRect() const
		{
			return tex_rect_;
		}

		UIStatesColor const & TextureColor() const
		{
			return texture_color_;
		}
		UIStatesColor& TextureColor()
		{
			return texture_color_;
		}

		UIStatesColor const & FontColor() const
		{
			return font_color_;
		}
		UIStatesColor& FontColor()
		{
			return font_color_;
		}

	private:
		uint32_t tex_index_;
		uint32_t font_index_;
		uint32_t text_align_;

		IRect tex_rect_;

		UIStatesColor texture_color_;
		UIStatesColor font_color_;
	};

	class KLAYGE_CORE_API UIControl : public std::enable_shared_from_this<UIControl>, boost::noncopyable
	{
	public:
		UIControl(uint32_t type, UIDialogPtr const & dialog)
			: visible_(true),
					is_mouse_over_(false), has_focus_(false), is_default_(false),
					x_(0), y_(0), width_(0), height_(0),
					dialog_(dialog), index_(0),
					id_(0), type_(type), enabled_(true),
					bounding_box_(0, 0, 0, 0)
		{
			BOOST_ASSERT(dialog);
		}
		virtual ~UIControl()
		{
		}

		virtual void Refresh()
		{
			is_mouse_over_ = false;
			has_focus_ = false;

			for (size_t i = 0; i < elements_.size(); ++ i)
			{
				elements_[i]->Refresh();
			}
		}


		virtual void Render() = 0;

		virtual bool CanHaveFocus() const
		{
			return false;
		}
		virtual bool HasFocus() const
		{
			return has_focus_;
		}
		virtual void OnFocusIn()
		{
			has_focus_ = true;
		}
		virtual void OnFocusOut()
		{
			has_focus_ = false;
		}
		virtual void OnMouseEnter()
		{
			is_mouse_over_ = true;
		}
		virtual void OnMouseLeave()
		{
			is_mouse_over_ = false;
		}
		virtual void OnHotkey()
		{
		}

		IRect const & BoundingBoxRect() const
		{
			return bounding_box_;
		}
		virtual bool ContainsPoint(int2 const & pt) const
		{
			return bounding_box_.PtInRect(pt);
		}

		virtual void SetEnabled(bool bEnabled)
		{
			enabled_ = bEnabled;
		}
		virtual bool GetEnabled() const
		{
			return enabled_;
		}
		virtual void SetVisible(bool bVisible)
		{
			visible_ = bVisible;
		}
		virtual bool GetVisible() const
		{
			return visible_;
		}

		uint32_t GetType() const
		{
			return type_;
		}

		int GetID() const
		{
			return id_;
		}
		void SetID(int ID)
		{
			id_ = ID;
		}

		void SetLocation(int x, int y)
		{
			x_ = x;
			y_ = y;
			this->UpdateRects();
		}
		void SetSize(int width, int height)
		{
			width_ = width;
			height_ = height;
			this->UpdateRects();
		}

		void SetHotkey(uint8_t hotkey)
		{
			hotkey_ = hotkey;
		}
		uint8_t GetHotkey() const
		{
			return hotkey_;
		}

		virtual void SetTextColor(Color const & color)
		{
			UIElement* element = elements_[0].get();
			if (element)
			{
				element->FontColor().States[UICS_Normal] = color;
			}
		}
		UIElement* GetElement(uint32_t iElement) const
		{
			return elements_[iElement].get();
		}
		void SetElement(uint32_t iElement, UIElement const & element)
		{
			// Make certain the array is this large
			for (uint32_t i = static_cast<uint32_t>(elements_.size()); i <= iElement; ++ i)
			{
				elements_.push_back(MakeUniquePtr<UIElement>());
			}

			// Update the data
			*elements_[iElement] = element;
		}

		bool GetIsDefault() const
		{
			return is_default_;
		}
		void SetIsDefault(bool bIsDefault)
		{
			is_default_ = bIsDefault;
		}
		uint32_t GetIndex() const
		{
			return index_;
		}
		void SetIndex(uint32_t Index)
		{
			index_ = Index;
		}
		UIDialogPtr GetDialog() const
		{
			return dialog_.lock();
		}

		virtual void KeyDownHandler(UIDialog const & /*sender*/, uint32_t /*key*/)
		{
		}
		virtual void KeyUpHandler(UIDialog const & /*sender*/, uint32_t /*key*/)
		{
		}
		virtual void MouseDownHandler(UIDialog const & /*sender*/, uint32_t /*buttons*/, int2 const & /*pt*/)
		{
		}
		virtual void MouseUpHandler(UIDialog const & /*sender*/, uint32_t /*buttons*/, int2 const & /*pt*/)
		{
		}
		virtual void MouseWheelHandler(UIDialog const & /*sender*/, uint32_t /*buttons*/, int2 const & /*pt*/, int32_t /*z_delta*/)
		{
		}
		virtual void MouseOverHandler(UIDialog const & /*sender*/, uint32_t /*buttons*/, int2 const & /*pt*/)
		{
		}

	protected:
		bool visible_;                // Shown/hidden flag
		bool is_mouse_over_;              // Mouse pointer is above control
		bool has_focus_;               // Control has input focus
		bool is_default_;              // Is the default control

		// Size, scale, and positioning members
		int x_, y_;
		int width_, height_;

		// These members are set by the container
		std::weak_ptr<UIDialog> dialog_;    // Parent container
		uint32_t index_;              // Index within the control list

		std::vector<std::unique_ptr<UIElement>> elements_;  // All display elements

	protected:
		virtual void UpdateRects()
		{
			bounding_box_ = IRect(x_, y_, x_ + width_, y_ + height_);
		}

		int  id_;				// ID number
		uint32_t type_;			// Control type, set once in constructor
		uint8_t hotkey_;		// Virtual key code for this control's hotkey

		bool enabled_;			// Enabled/disabled flag

		IRect bounding_box_;		// Rectangle defining the active region of the control
	};

	class KLAYGE_CORE_API UIManager : boost::noncopyable, public std::enable_shared_from_this<UIManager>
	{
	public:
		struct VertexFormat
		{
			float3 pos;
			Color clr;
			float2 tex;

			VertexFormat()
			{
			}
			VertexFormat(float3 const & p, Color const & c, float2 const & t)
				: pos(p), clr(c), tex(t)
			{
			}
		};

		UIManager();
		~UIManager();

		static UIManager& Instance();
		static void Destroy();

		void Suspend();
		void Resume();

		void Load(ResIdentifierPtr const & source);

		UIDialogPtr MakeDialog(TexturePtr const & control_tex = TexturePtr());

		size_t AddTexture(TexturePtr const & texture);
		size_t AddFont(FontPtr const & font, float font_size);

		TexturePtr const & GetTexture(size_t index) const;
		FontPtr const & GetFont(size_t index) const;
		float GetFontSize(size_t index) const;

		bool RegisterDialog(UIDialogPtr const & dialog);
		void UnregisterDialog(UIDialogPtr const & dialog);
		void EnableKeyboardInputForAllDialogs();

		RenderEffectPtr const & GetEffect() const
		{
			return effect_;
		}

		std::vector<UIDialogPtr> const & GetDialogs() const
		{
			return dialogs_;
		}
		UIDialogPtr const & GetDialog(std::string_view id) const;

		UIDialogPtr const & GetNextDialog(UIDialogPtr const & dialog) const;
		UIDialogPtr const & GetPrevDialog(UIDialogPtr const & dialog) const;

		void Render();

		void DrawRect(float3 const & pos, float width, float height, Color const * clrs,
			IRect const & rcTexture, TexturePtr const & texture);
		void DrawQuad(float3 const & offset, VertexFormat const * vertices, TexturePtr const & texture);
		void DrawString(std::wstring const & strText, uint32_t font_index,
			IRect const & rc, float depth, Color const & clr, uint32_t align);
		Size_T<float> CalcSize(std::wstring const & strText, uint32_t font_index,
			IRect const & rc, uint32_t align);

		IRect const & ElementTextureRect(uint32_t ctrl, uint32_t elem_index);
		size_t NumElementTextureRect(uint32_t ctrl) const;

		void SettleCtrls();

		bool MouseOnUI() const
		{
			return mouse_on_ui_;
		}

	private:
		void Init();
		void InputHandler(InputEngine const & sender, InputAction const & action);

	private:
		static std::unique_ptr<UIManager> ui_mgr_instance_;

		// Shared between all dialogs
		RenderEffectPtr effect_;

		std::vector<UIDialogPtr> dialogs_;            // Dialogs registered

		std::vector<TexturePtr> texture_cache_;   // Shared textures
		std::vector<std::pair<FontPtr, float>> font_cache_;         // Shared fonts

		std::array<std::vector<IRect >, UICT_Num_Control_Types> elem_texture_rcs_;

		std::map<TexturePtr, RenderablePtr> rects_;

		struct string_cache
		{
			Rect rc;
			float depth;
			Color clr;
			std::wstring text;
			uint32_t align;
		};
		std::map<size_t, std::vector<string_cache>> strings_;

		bool mouse_on_ui_;
		bool inited_;
	};

	class KLAYGE_CORE_API UIDialog : boost::noncopyable
	{
		friend class UIManager;

	public:
		enum ControlAlignment
		{
			CA_Left,
			CA_Right,
			CA_Center,
			CA_Top,
			CA_Bottom,
			CA_Middle
		};

		struct ControlLocation
		{
			int x, y;
			ControlAlignment align_x, align_y;
		};

	public:
		explicit UIDialog(TexturePtr const & control_tex);
		~UIDialog();

		void AddIDName(std::string const & name, int id);
		int IDFromName(std::string const & name);

		void CtrlLocation(int id, ControlLocation const & loc);
		ControlLocation const & CtrlLocation(int id);

		void SettleCtrls();

		void AddControl(UIControlPtr const & control);
		void InitControl(UIControl& control);

		// Control retrieval
		template <typename T>
		std::shared_ptr<T> Control(int ID) const
		{
			return checked_pointer_cast<T>(this->GetControl(ID, T::Type));
		}

		UIControlPtr const & GetControl(int ID) const;
		UIControlPtr const & GetControl(int ID, uint32_t type) const;
		UIControlPtr const & GetControlAtPoint(int2 const & pt) const;

		bool GetControlEnabled(int ID) const;
		void SetControlEnabled(int ID, bool enabled);

		void ClearRadioButtonGroup(uint32_t nGroup);

		void Render();

		void RequestFocus(UIControl& control);
		void ClearFocus();

		// Attributes
		bool GetVisible() const
		{
			return visible_;
		}
		void SetVisible(bool bVisible)
		{
			visible_ = bVisible;
		}
		bool GetMinimized() const
		{
			return minimized_;
		}
		void SetMinimized(bool bMinimized)
		{
			minimized_ = bMinimized;
		}
		void SetBackgroundColors(Color const & colorAllCorners);
		void SetBackgroundColors(Color const & colorTopLeft, Color const & colorTopRight,
			Color const & colorBottomLeft, Color const & colorBottomRight);
		void EnableCaption(bool bEnable)
		{
			show_caption_ = bEnable;
		}
		bool IsCaptionEnabled() const
		{
			return show_caption_;
		}
		int GetCaptionHeight() const
		{
			return caption_height_;
		}
		void SetCaptionHeight(int nHeight)
		{
			caption_height_ = nHeight;
		}
		void SetID(std::string const & id)
		{
			id_ = id;
		}
		std::string const & GetID() const
		{
			return id_;
		}
		void SetCaptionText(std::wstring const & strText)
		{
			caption_ = strText;
		}
		int2 GetLocation() const
		{
			return int2(bounding_box_.left(), bounding_box_.top());
		}
		void SetLocation(int x, int y)
		{
			int const w = this->GetWidth();
			int const h = this->GetHeight();
			bounding_box_.left() = x;
			bounding_box_.top() = y;
			bounding_box_.right() = x + w;
			bounding_box_.bottom() = y + h;
		}
		void SetSize(int width, int height)
		{
			bounding_box_.right() = bounding_box_.left() + width;
			bounding_box_.bottom() = bounding_box_.top() + height;
		}
		int GetWidth() const
		{
			return bounding_box_.Width();
		}
		int GetHeight() const
		{
			return bounding_box_.Height();
		}
		void AlwaysInOpacity(bool opacity)
		{
			always_in_opacity_ = opacity;
		}
		bool AlwaysInOpacity() const
		{
			return always_in_opacity_;
		}

		UIControlPtr const & GetNextControl(UIControlPtr const & control) const;
		UIControlPtr const & GetPrevControl(UIControlPtr const & control) const;

		void RemoveControl(int ID);
		void RemoveAllControls();

		void EnableKeyboardInput(bool bEnable)
		{
			keyboard_input_ = bEnable;
		}
		void EnableMouseInput(bool bEnable)
		{
			mouse_input_ = bEnable;
		}
		bool IsKeyboardInputEnabled() const
		{
			return keyboard_input_;
		}

		bool ContainsPoint(int2 const & pt) const;
		int2 ToLocal(int2 const & pt) const;

		// Device state notification
		void Refresh();

		// Shared resource access. Indexed fonts and textures are shared among
		// all the controls.
		void SetFont(size_t index, FontPtr const & font, float font_size);
		FontPtr const & GetFont(size_t index) const;
		float GetFontSize(size_t index) const;

		void FocusDefaultControl();

		void DrawRect(IRect const & rc, float depth, Color const & clr);
		void DrawQuad(UIManager::VertexFormat const * vertices, float depth, TexturePtr const & texture);
		void DrawSprite(UIElement const & element, IRect const & rcDest, float depth_bias = 0.0f);
		void DrawString(std::wstring const & strText, UIElement const & uie, IRect const & rc, bool bShadow = false, float depth_bias = 0.0f);
		UISize CalcSize(std::wstring const & strText, UIElement const & uie, IRect const & rc, bool bShadow = false);

	private:
		void KeyDownHandler(uint32_t key);
		void KeyUpHandler(uint32_t key);
		void MouseDownHandler(uint32_t buttons, int2 const & pt);
		void MouseUpHandler(uint32_t buttons, int2 const & pt);
		void MouseWheelHandler(uint32_t buttons, int2 const & pt, int32_t z_delta);
		void MouseOverHandler(uint32_t buttons, int2 const & pt);

	private:
		bool keyboard_input_;
		bool mouse_input_;

		// Initialize default Elements
		void InitDefaultElements();

		// Control events
		bool OnCycleFocus(bool bForward);

		std::weak_ptr<UIControl> control_focus_;				// The control which has focus
		std::weak_ptr<UIControl> control_mouse_over_;			// The control which is hovered over

		bool visible_;
		bool show_caption_;
		bool always_in_opacity_;
		bool minimized_;
		std::string id_;
		std::wstring caption_;

		IRect bounding_box_;
		int caption_height_;

		Color top_left_clr_;
		Color top_right_clr_;
		Color bottom_left_clr_;
		Color bottom_right_clr_;

		std::vector<int> fonts_;		// Index into font_cache_;
		size_t tex_index_;				// Index into texture_cache_;

		std::vector<UIControlPtr> controls_;

		UIElement cap_element_;  // Element for the caption

		float depth_base_;
		float opacity_;

		std::map<std::string, int> id_name_;
		std::map<int, ControlLocation> id_location_;
	};

	class KLAYGE_CORE_API UIStatic : public UIControl
	{
	public:
		enum
		{
			Type = UICT_Static
		};

	public:
		explicit UIStatic(UIDialogPtr const & dialog);
		UIStatic(uint32_t type, UIDialogPtr const & dialog);
		UIStatic(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, bool bIsDefault = false);
		virtual ~UIStatic()
		{
		}

		virtual void Render();
		virtual bool ContainsPoint(int2 const & /*pt*/) const
		{
			return false;
		}

		std::wstring const & GetText() const
		{
			return text_;
		}
		void SetText(std::wstring const & strText);

	protected:
		virtual void InitDefaultElements();

		std::wstring text_;			// Window text
	};

	class KLAYGE_CORE_API UIButton : public UIControl
	{
	public:
		enum
		{
			Type = UICT_Button
		};

	public:
		explicit UIButton(UIDialogPtr const & dialog);
		UIButton(uint32_t type, UIDialogPtr const & dialog);
		UIButton(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, uint8_t hotkey = 0, bool bIsDefault = false);
		virtual ~UIButton()
		{
		}

		virtual bool CanHaveFocus() const
		{
			return visible_ && enabled_;
		}
		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			pressed_ = false;
		}
		virtual void OnHotkey();

		virtual void Render();

		std::wstring const & GetText() const;
		void SetText(std::wstring const & strText);

	public:
		typedef boost::signals2::signal<void(UIButton const & sender)> ClickedEvent;
		ClickedEvent& OnClickedEvent()
		{
			return clicked_event_;
		}

		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void KeyUpHandler(UIDialog const & sender, uint32_t key);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);

	protected:
		ClickedEvent clicked_event_;

	protected:
		virtual void InitDefaultElements();

		bool pressed_;

		std::wstring text_;			// Window text
	};

	class KLAYGE_CORE_API UITexButton : public UIControl
	{
	public:
		enum
		{
			Type = UICT_TexButton
		};

	public:
		explicit UITexButton(UIDialogPtr const & dialog);
		UITexButton(uint32_t type, UIDialogPtr const & dialog);
		UITexButton(UIDialogPtr const & dialog, int ID, TexturePtr const & tex, int4 const & coord_size, uint8_t hotkey = 0, bool bIsDefault = false);
		virtual ~UITexButton()
		{
		}

		virtual bool CanHaveFocus() const
		{
			return visible_ && enabled_;
		}
		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			pressed_ = false;
		}
		virtual void OnHotkey();

		virtual void Render();

		TexturePtr const & GetTexture() const;
		void SetTexture(TexturePtr const & tex);

	public:
		typedef boost::signals2::signal<void(UITexButton const & sender)> ClickedEvent;
		ClickedEvent& OnClickedEvent()
		{
			return clicked_event_;
		}

		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void KeyUpHandler(UIDialog const & sender, uint32_t key);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);

	protected:
		ClickedEvent clicked_event_;

	protected:
		virtual void InitDefaultElements();

		bool pressed_;

		size_t tex_index_;
	};

	class KLAYGE_CORE_API UICheckBox : public UIControl
	{
	public:
		enum
		{
			Type = UICT_CheckBox
		};

	public:
		explicit UICheckBox(UIDialogPtr const & dialog);
		UICheckBox(uint32_t type, UIDialogPtr const & dialog);
		UICheckBox(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, bool bChecked = false, uint8_t hotkey = 0, bool bIsDefault = false);
		virtual ~UICheckBox()
		{
		}

		virtual bool CanHaveFocus() const
		{
			return (visible_ && enabled_);
		}
		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			pressed_ = false;
		}
		virtual void OnHotkey();
		virtual void UpdateRects();

		virtual void Render();

		bool GetChecked() const
		{
			return checked_;
		}
		void SetChecked(bool bChecked)
		{
			this->SetCheckedInternal(bChecked);
		}

		std::wstring const & GetText() const;
		void SetText(std::wstring const & strText);

	public:
		typedef boost::signals2::signal<void(UICheckBox const & sender)> ChangedEvent;
		ChangedEvent& OnChangedEvent()
		{
			return changed_event_;
		}

		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void KeyUpHandler(UIDialog const & sender, uint32_t key);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);

	protected:
		ChangedEvent changed_event_;

	protected:
		virtual void SetCheckedInternal(bool bChecked);
		virtual void InitDefaultElements();

		bool checked_;
		IRect button_rc_;
		IRect text_rc_;

		bool pressed_;

		std::wstring text_;      // Window text
	};

	class KLAYGE_CORE_API UIRadioButton : public UIControl
	{
	public:
		enum
		{
			Type = UICT_RadioButton
		};

	public:
		explicit UIRadioButton(UIDialogPtr const & dialog);
		UIRadioButton(uint32_t type, UIDialogPtr const & dialog);
		UIRadioButton(UIDialogPtr const & dialog, int ID, uint32_t nButtonGroup, std::wstring const & strText, int4 const & coord_size, bool bChecked = false, uint8_t hotkey = 0, bool bIsDefault = false);
		virtual ~UIRadioButton()
		{
		}

		void SetChecked(bool bChecked, bool bClearGroup = true)
		{
			this->SetCheckedInternal(bChecked, bClearGroup);
		}
		void SetButtonGroup(uint32_t nButtonGroup)
		{
			button_group_ = nButtonGroup;
		}
		uint32_t GetButtonGroup() const
		{
			return button_group_;
		}

		virtual bool CanHaveFocus() const
		{
			return visible_ && enabled_;
		}
		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			pressed_ = false;
		}
		virtual void OnHotkey();
		virtual void UpdateRects();

		virtual void Render();

		bool GetChecked() const
		{
			return checked_;
		}

		std::wstring const & GetText() const;
		void SetText(std::wstring const & strText);

	public:
		typedef boost::signals2::signal<void(UIRadioButton const & sender)> ChangedEvent;
		ChangedEvent& OnChangedEvent()
		{
			return changed_event_;
		}

		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void KeyUpHandler(UIDialog const & sender, uint32_t key);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);

	protected:
		ChangedEvent changed_event_;

	protected:
		virtual void SetCheckedInternal(bool bChecked, bool bClearGroup);
		virtual void InitDefaultElements();

		uint32_t button_group_;

		bool checked_;
		IRect button_rc_;
		IRect text_rc_;

		bool pressed_;

		std::wstring text_;      // Window text
	};

	class KLAYGE_CORE_API UISlider : public UIControl
	{
	public:
		enum
		{
			Type = UICT_Slider
		};

	public:
		explicit UISlider(UIDialogPtr const & dialog);
		UISlider(uint32_t type, UIDialogPtr const & dialog);
		UISlider(UIDialogPtr const & dialog, int ID, int4 const & coord_size, int min = 0, int max = 100, int value = 50, bool bIsDefault = false);
		virtual ~UISlider()
		{
		}

		virtual bool CanHaveFocus() const
		{
			return visible_ && enabled_;
		}
		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			pressed_ = false;
		}

		virtual void UpdateRects();

		virtual void Render();

		void SetValue(int nValue)
		{
			this->SetValueInternal(nValue);
		}
		int GetValue() const
		{
			return value_;
		};

		void GetRange(int &nMin, int &nMax) const
		{
			nMin = min_;
			nMax = max_;
		}
		void SetRange(int nMin, int nMax);

	public:
		typedef boost::signals2::signal<void(UISlider const & sender)> ValueChangedEvent;
		ValueChangedEvent& OnValueChangedEvent()
		{
			return value_changed_event_;
		}

		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void MouseOverHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseWheelHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt, int32_t z_delta);

	protected:
		ValueChangedEvent value_changed_event_;

	protected:
		virtual void InitDefaultElements();

		void SetValueInternal(int nValue);
		int ValueFromPos(int x);

		int value_;

		int min_;
		int max_;

		int drag_x_;      // Mouse position at start of drag
		int drag_offset_; // Drag offset from the center of the button
		int button_x_;

		bool pressed_;

		IRect button_rc_;
		IRect slider_rc_;
	};

	class KLAYGE_CORE_API UIScrollBar : public UIControl
	{
	public:
		enum
		{
			Type = UICT_ScrollBar
		};

	public:
		explicit UIScrollBar(UIDialogPtr const & dialog);
		UIScrollBar(uint32_t type, UIDialogPtr const & dialog);
		UIScrollBar(UIDialogPtr const & dialog, int ID, int4 const & coord_size, int nTrackStart = 0, int nTrackEnd = 1, int nTrackPos = 0, int nPageSize = 1);
		virtual ~UIScrollBar();

		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			drag_ = false;
		}

		virtual void Render();
		virtual void UpdateRects();

		void SetTrackRange(size_t nStart, size_t nEnd);
		size_t GetTrackPos() const
		{
			return position_;
		}
		void SetTrackPos(size_t nPosition)
		{
			position_ = nPosition;
			this->Cap();
			this->UpdateThumbRect();
		}
		size_t GetPageSize() const
		{
			return page_size_;
		}
		void SetPageSize(size_t nPageSize)
		{
			page_size_ = nPageSize;
			this->Cap();
			this->UpdateThumbRect();
		}

		void Scroll(int nDelta);    // Scroll by nDelta items (plus or minus)
		void ShowItem(size_t nIndex);  // Ensure that item nIndex is displayed, scroll if necessary

		void MouseOverHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);

	protected:
		virtual void InitDefaultElements();

		// ARROWSTATE indicates the state of the arrow buttons.
		enum ARROWSTATE
		{
			CLEAR,			// No arrow is down.
			CLICKED_UP,		// Up arrow is clicked.
			CLICKED_DOWN,	// Down arrow is clicked.
			HELD_UP,		// Up arrow is held down for sustained period.
			HELD_DOWN		// Down arrow is held down for sustained period.
		};

		void UpdateThumbRect();
		void Cap();  // Clips position at boundaries. Ensures it stays within legal range.

		bool show_thumb_;
		bool drag_;
		IRect up_button_rc_;
		IRect down_button_rc_;
		IRect track_rc_;
		IRect thumb_rc_;
		size_t position_;  // Position of the first displayed item
		size_t page_size_;  // How many items are displayable in one page
		size_t start_;     // First item
		size_t end_;       // The index after the last item
		int2 last_mouse_;// Last mouse position
		ARROWSTATE arrow_; // State of the arrows
		double arrow_ts_;  // Timestamp of last arrow event.
		int thumb_offset_y_;

		static Timer timer_;
	};

	struct KLAYGE_CORE_API UIListBoxItem
	{
		std::wstring strText;
		std::any data;

		IRect  rcActive;
		bool  bSelected;
	};

	class KLAYGE_CORE_API UIListBox : public UIControl
	{
	public:
		enum
		{
			Type = UICT_ListBox
		};

	public:
		enum STYLE
		{
			SINGLE_SELECTION = 0,
			MULTI_SELECTION = 1
		};

		explicit UIListBox(UIDialogPtr const & dialog);
		UIListBox(uint32_t type, UIDialogPtr const & dialog);
		UIListBox(UIDialogPtr const & dialog, int ID, int4 const & coord_size, STYLE dwStyle = SINGLE_SELECTION);
		virtual ~UIListBox();

		virtual bool CanHaveFocus() const
		{
			return visible_ && enabled_;
		}
		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			drag_ = false;
		}

		virtual void    Render();
		virtual void    UpdateRects();

		STYLE GetStyle() const
		{
			return style_;
		}
		uint32_t GetSize() const
		{
			return static_cast<uint32_t>(items_.size());
		}
		void SetStyle(STYLE style)
		{
			style_ = style;
		}
		int  GetScrollBarWidth() const
		{
			return sb_width_;
		}
		void SetScrollBarWidth(int width)
		{
			sb_width_ = width;
			this->UpdateRects();
		}
		void SetBorder(int border, int margin)
		{
			border_ = border;
			margin_ = margin;
		}
		int AddItem(std::wstring const & strText);
		void SetItemData(int nIndex, std::any const & data);
		int AddItem(std::wstring const & strText, std::any const & data);
		void InsertItem(int nIndex, std::wstring const & strText, std::any const & data);
		void RemoveItem(int nIndex);
		void RemoveAllItems();

		std::shared_ptr<UIListBoxItem> GetItem(int nIndex) const;
		int GetSelectedIndex(int nPreviousSelected = -1) const;
		std::shared_ptr<UIListBoxItem> GetSelectedItem(int nPreviousSelected = -1) const
		{
			return this->GetItem(this->GetSelectedIndex(nPreviousSelected));
		}
		void SelectItem(int nNewIndex);

	public:
		typedef boost::signals2::signal<void(UIListBox const & sender)> SelectionEvent;
		SelectionEvent& OnSelectionEvent()
		{
			return selection_event_;
		}
		SelectionEvent& OnSelectionEndEvent()
		{
			return selection_end_event_;
		}

		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void KeyUpHandler(UIDialog const & sender, uint32_t key);
		void MouseOverHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseWheelHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt, int32_t z_delta);

	protected:
		SelectionEvent selection_event_;
		SelectionEvent selection_end_event_;

	protected:
		virtual void InitDefaultElements();

		IRect text_rc_;      // Text rendering bound
		IRect selection_rc_; // Selection box bound
		UIScrollBar scroll_bar_;
		int sb_width_;
		int border_;
		int margin_;
		int text_height_;  // Height of a single line of text
		STYLE style_;    // List box style
		int selected_;    // Index of the selected item for single selection list box
		int sel_start_;    // Index of the item where selection starts (for handling multi-selection)
		bool drag_;       // Whether the user is dragging the mouse to select

		std::vector<std::shared_ptr<UIListBoxItem>> items_;
	};

	struct UIComboBoxItem
	{
		std::wstring strText;
		std::any data;

		IRect  rcActive;
		bool  bVisible;
	};

	class KLAYGE_CORE_API UIComboBox : public UIControl
	{
	public:
		enum
		{
			Type = UICT_ComboBox
		};

	public:
		explicit UIComboBox(UIDialogPtr const & dialog);
		UIComboBox(uint32_t type, UIDialogPtr const & dialog);
		UIComboBox(UIDialogPtr const & dialog, int ID, int4 const & coord_size, uint8_t hotkey = 0, bool bIsDefault = false);
		virtual ~UIComboBox();

		virtual void SetTextColor(Color const & color);

		virtual bool CanHaveFocus() const
		{
			return visible_ && enabled_;
		}
		virtual void OnHotkey();
		virtual void OnFocusOut();
		virtual void Render();

		virtual void UpdateRects();

		int AddItem(std::wstring const & strText);
		void SetItemData(int nIndex, std::any const & data);
		int AddItem(std::wstring const & strText, std::any const & data);
		void RemoveAllItems();
		void RemoveItem(uint32_t index);
		bool ContainsItem(std::wstring const & strText, uint32_t iStart = 0) const;
		int FindItem(std::wstring const & strText, uint32_t iStart = 0) const;
		std::any const GetItemData(std::wstring const & strText) const;
		std::any const GetItemData(int nIndex) const;
		void SetDropHeight(uint32_t nHeight)
		{
			drop_height_ = nHeight;
			this->UpdateRects();
		}
		int GetScrollBarWidth() const
		{
			return sb_width_;
		}
		void SetScrollBarWidth(int nWidth)
		{
			sb_width_ = nWidth;
			this->UpdateRects();
		}

		std::any const GetSelectedData() const;
		std::shared_ptr<UIComboBoxItem> GetSelectedItem() const;
		int GetSelectedIndex() const;

		uint32_t GetNumItems() const
		{
			return static_cast<uint32_t>(items_.size());
		}
		std::shared_ptr<UIComboBoxItem> GetItem(uint32_t index) const
		{
			return items_[index];
		}

		void SetSelectedByIndex(uint32_t index);
		void SetSelectedByText(std::wstring const & strText);

		template <typename T>
		void SetSelectedByData(T const & data)
		{
			for (uint32_t i = 0; i < items_.size(); ++ i)
			{
				std::shared_ptr<UIComboBoxItem> pItem = items_[i];

				if (std::any_cast<T>(pItem->data) == data)
				{
					this->SetSelectedByIndex(static_cast<uint32_t>(i));
				}
			}
		}

	public:
		typedef boost::signals2::signal<void(UIComboBox const & sender)> SelectionChangedEvent;
		SelectionChangedEvent& OnSelectionChangedEvent()
		{
			return selection_changed_event_;
		}

		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void KeyUpHandler(UIDialog const & sender, uint32_t key);
		void MouseOverHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseWheelHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt, int32_t z_delta);

	protected:
		SelectionChangedEvent selection_changed_event_;

	protected:
		virtual void InitDefaultElements();

		int     selected_;
		int     focused_;
		int     drop_height_;
		UIScrollBar scroll_bar_;
		int     sb_width_;

		bool    opened_;

		IRect show_rc_;
		IRect text_rc_;
		IRect button_rc_;
		IRect dropdown_rc_;
		IRect dropdown_text_rc_;

		std::vector<std::shared_ptr<UIComboBoxItem>> items_;

		bool pressed_;
	};

	// UniBuffer class for the edit control
	class KLAYGE_CORE_API UniBuffer : boost::noncopyable
	{
	public:
		explicit UniBuffer(int nInitialSize = 1);

		uint32_t GetTextSize() const
		{
			return static_cast<uint32_t>(buffer_.size());
		}
		std::wstring& GetBuffer()
		{
			return buffer_;
		}
		std::wstring const & GetBuffer() const
		{
			return buffer_;
		}
		wchar_t const & operator[](size_t n) const
		{
			return buffer_[n];
		}
		wchar_t& operator[](size_t n);
		FontPtr const & GetFont() const
		{
			return font_;
		}
		void SetFont(FontPtr const & font, float font_size)
		{
			font_ = font;
			font_size_ = font_size;
		}
		void Clear();

		void InsertChar(size_t index, wchar_t wChar); // Inserts the char at specified index
		void RemoveChar(size_t index);  // Removes the char at specified index
		void InsertString(size_t index, std::wstring const & str);  // Inserts the first nCount characters of the string pStr at specified index.
		void SetText(std::wstring const & strText);

		// Uniscribe
		int CPtoX(int nCP, bool bTrail);
		int XtoCP(int nX, bool& bTrail);
		int GetPriorItemPos(int nCP) const;
		int GetNextItemPos(int nCP) const;

	private:
		bool Analyse();      // Uniscribe -- Analyse() analyses the string in the buffer

		std::wstring buffer_;	// Buffer to hold text
		std::vector<int> char_width_;

		// Uniscribe-specific
		FontPtr font_;				// Font node for the font that this buffer uses
		float font_size_;
		bool analyse_required_;			// True if the string has changed since last analysis.
	};

	// EditBox control
	class KLAYGE_CORE_API UIEditBox : public UIControl
	{
	public:
		enum
		{
			Type = UICT_EditBox
		};

	public:
		explicit UIEditBox(UIDialogPtr const & dialog);
		UIEditBox(uint32_t type, UIDialogPtr const & dialog);
		UIEditBox(UIDialogPtr const & dialog, int ID, std::wstring const & strText, int4 const & coord_size, bool bIsDefault = false);
		virtual ~UIEditBox();

		virtual void UpdateRects();
		virtual bool CanHaveFocus() const
		{
			return visible_ && enabled_;
		}
		virtual void OnFocusIn();
		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			mouse_drag_ = false;
		}
		virtual void Render();

		void SetText(std::wstring const & wszText, bool bSelected = false);
		std::wstring const & GetText() const
		{
			return buffer_.GetBuffer();
		}
		int GetTextLength() const
		{
			return buffer_.GetTextSize();	// Returns text length in chars excluding nullptr.
		}
		void ClearText();
		virtual void SetTextColor(Color const & Color)
		{
			text_color_ = Color;	// Text color
		}
		void SetSelectedTextColor(Color const & Color)
		{
			sel_text_color_ = Color;	// Selected text color
		}
		void SetSelectedBackColor(Color const & Color)
		{
			sel_bk_color_ = Color;	// Selected background color
		}
		void SetCaretColor(Color const & Color)
		{
			caret_color_ = Color;	// Caret color
		}
		void SetBorderWidth(int nBorder)
		{
			// Border of the window
			border_ = nBorder;
			this->UpdateRects();
		}
		void SetSpacing(int nSpacing)
		{
			spacing_ = nSpacing;
			this->UpdateRects();
		}

	public:
		typedef boost::signals2::signal<void(UIEditBox const & sender)> EditBoxEvent;
		EditBoxEvent& OnChangedEvent()
		{
			return changed_event_;
		}
		EditBoxEvent& OnStringEvent()
		{
			return string_event_;
		}

		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void MouseOverHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);

		void CharHandler(Window const & win, wchar_t ch);

	protected:
		EditBoxEvent changed_event_;
		EditBoxEvent string_event_;

	protected:
		virtual void InitDefaultElements();

		void PlaceCaret(int nCP);
		void DeleteSelectionText();
		void ResetCaretBlink();
		void CopyToClipboard();
		void PasteFromClipboard();

		UniBuffer buffer_;     // Buffer to hold text
		int      border_;      // Border of the window
		int      spacing_;     // Spacing between the text and the edge of border
		IRect     text_rc_;       // Bounding rectangle for the text
		IRect     render_rc_[9];  // Convenient rectangles for rendering elements
		double   blink_time_;      // Caret blink time in milliseconds
		double   last_blink_time_;  // Last timestamp of caret blink
		bool     caret_on_;     // Flag to indicate whether caret is currently visible
		int      caret_pos_;       // Caret position, in characters
		bool     insert_mode_;  // If true, control is in insert mode. Else, overwrite mode.
		int      sel_start_;    // Starting position of the selection. The caret marks the end.
		int      first_visible_;// First visible character in the edit control
		Color	text_color_;    // Text color
		Color	sel_text_color_; // Selected text color
		Color	sel_bk_color_;   // Selected background color
		Color	caret_color_;   // Caret color

		// Mouse-specific
		bool mouse_drag_;       // True to indicate drag in progress

		boost::signals2::connection on_char_connect_;

		// Static
		static bool hide_caret_;   // If true, we don't render the caret.
		static Timer timer_;
	};

	class KLAYGE_CORE_API UIPolylineEditBox : public UIControl
	{
	public:
		enum
		{
			Type = UICT_PolylineEditBox
		};

	public:
		explicit UIPolylineEditBox(UIDialogPtr const & dialog);
		UIPolylineEditBox(uint32_t type, UIDialogPtr const & dialog);
		UIPolylineEditBox(UIDialogPtr const & dialog, int ID, int4 const & coord_size, uint8_t hotkey = 0, bool bIsDefault = false);
		virtual ~UIPolylineEditBox()
		{
		}

		virtual bool CanHaveFocus() const
		{
			return visible_ && enabled_;
		}
		virtual void OnFocusOut()
		{
			UIControl::OnFocusOut();
			move_point_ = false;
		}

		virtual void Render();

		void ActivePoint(int index);
		int ActivePoint() const;
		void ClearCtrlPoints();
		int AddCtrlPoint(float pos, float value);
		int AddCtrlPoint(float pos);
		void DelCtrlPoint(int index);
		void SetCtrlPoint(int index, float pos, float value);
		void SetCtrlPoints(std::vector<float2> const & ctrl_points);
		void SetColor(Color const & clr);
		size_t NumCtrlPoints() const;
		float2 const & GetCtrlPoint(size_t i) const;
		std::vector<float2> const & GetCtrlPoints() const;
		float2 PtFromCoord(int x, int y) const;
		float GetValue(float pos) const;

	public:
		void KeyDownHandler(UIDialog const & sender, uint32_t key);
		void KeyUpHandler(UIDialog const & sender, uint32_t key);
		void MouseDownHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseUpHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);
		void MouseOverHandler(UIDialog const & sender, uint32_t buttons, int2 const & pt);

	protected:
		virtual void InitDefaultElements();
		static const int BACKGROUND_INDEX = 0;
		static const int COORDLINE_INDEX = 1;
		static const int POLYLINE_INDEX = 2;
		static const int CTRLPOINTS_INDEX = 3;

	protected:
		std::vector<float2> ctrl_points_;
		int active_pt_;
		int move_over_pt_;

		bool move_point_;
	};

	class KLAYGE_CORE_API UIProgressBar : public UIControl
	{
	public:
		enum
		{
			Type = UICT_ProgressBar
		};

	public:
		explicit UIProgressBar(UIDialogPtr const & dialog);
		UIProgressBar(uint32_t type, UIDialogPtr const & dialog);
		UIProgressBar(UIDialogPtr const & dialog, int ID, int progress, int4 const & coord_size, uint8_t hotkey = 0, bool bIsDefault = false);
		virtual ~UIProgressBar()
		{
		}

		virtual bool CanHaveFocus() const
		{
			return false;
		}

		virtual void Render();

		void SetValue(int value);
		int GetValue() const;

	protected:
		virtual void InitDefaultElements();
		static const int BACKGROUND_INDEX = 0;
		static const int BAR_INDEX = 1;

	protected:
		int progress_;
	};
}

#endif		// _UI_HPP
