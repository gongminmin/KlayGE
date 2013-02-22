// Input.hpp
// KlayGE 输入引擎类 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// 增加了Action map id (2005.4.3)
//
// 2.0.4
// 改进了InputActionsType
//
// 2.0.0
// 初次建立 (2003.8.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _INPUT_HPP
#define _INPUT_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

#include <KFL/Vector.hpp>
#include <KFL/Timer.hpp>

#include <boost/container/flat_map.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4100 4512 4913 6011)
#endif
#include <boost/signals2.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <vector>
#include <string>

namespace KlayGE
{
	// 键盘动作
	enum KeyboardSemantic
	{
		KS_Escape			= 0x01,
		KS_1				= 0x02,
		KS_2				= 0x03,
		KS_3				= 0x04,
		KS_4				= 0x05,
		KS_5				= 0x06,
		KS_6				= 0x07,
		KS_7				= 0x08,
		KS_8				= 0x09,
		KS_9				= 0x0A,
		KS_0				= 0x0B,
		KS_Minus			= 0x0C,		// - on main keyboard
		KS_Equals			= 0x0D,
		KS_BackSpace		= 0x0E,		// backspace
		KS_Tab				= 0x0F,
		KS_Q				= 0x10,
		KS_W				= 0x11,
		KS_E				= 0x12,
		KS_R				= 0x13,
		KS_T				= 0x14,
		KS_Y				= 0x15,
		KS_U				= 0x16,
		KS_I				= 0x17,
		KS_O				= 0x18,
		KS_P				= 0x19,
		KS_LeftBracket		= 0x1A,
		KS_ReftBracket		= 0x1B,
		KS_Enter			= 0x1C,		// Enter on main keyboard
		KS_LeftCtrl			= 0x1D,
		KS_A				= 0x1E,
		KS_S				= 0x1F,
		KS_D				= 0x20,
		KS_F				= 0x21,
		KS_G				= 0x22,
		KS_H				= 0x23,
		KS_J				= 0x24,
		KS_K				= 0x25,
		KS_L				= 0x26,
		KS_Semicolon		= 0x27,
		KS_Apostrophe		= 0x28,
		KS_Grave			= 0x29,		// accent grave
		KS_LeftShift		= 0x2A,
		KS_BackSlash		= 0x2B,
		KS_Z				= 0x2C,
		KS_X				= 0x2D,
		KS_C				= 0x2E,
		KS_V				= 0x2F,
		KS_B				= 0x30,
		KS_N				= 0x31,
		KS_M				= 0x32,
		KS_Comma			= 0x33,
		KS_Period			= 0x34,		// . on main keyboard
		KS_Slash			= 0x35,		// / on main keyboard
		KS_RightShift		= 0x36,
		KS_NumPadStar		= 0x37,		// * on numeric keypad
		KS_LeftAlt			= 0x38,
		KS_Space			= 0x39,
		KS_CapsLock			= 0x3A,
		KS_F1				= 0x3B,
		KS_F2				= 0x3C,
		KS_F3				= 0x3D,
		KS_F4				= 0x3E,
		KS_F5				= 0x3F,
		KS_F6				= 0x40,
		KS_F7				= 0x41,
		KS_F8				= 0x42,
		KS_F9				= 0x43,
		KS_F10				= 0x44,
		KS_NumLock			= 0x45,
		KS_ScrollLock		= 0x46,
		KS_NumPad7			= 0x47,
		KS_NumPad8			= 0x48,
		KS_NumPad9			= 0x49,
		KS_NumPadMinus		= 0x4A,		// - on numeric keypad
		KS_NumPad4			= 0x4B,
		KS_NumPad5			= 0x4C,
		KS_NumPadD6			= 0x4D,
		KS_NumPadPlus		= 0x4E,		// + on numeric keypad
		KS_NumPad1			= 0x4F,
		KS_NumPad2			= 0x50,
		KS_NumPad3			= 0x51,
		KS_NumPad0			= 0x52,
		KS_NumPadPeriod		= 0x53,		// . on numeric keypad
		KS_OEM_102			= 0x56,		// <> or \| on RT 102-key keyboard (Non-U.S.)
		KS_F11				= 0x57,
		KS_F12				= 0x58,
		KS_F13				= 0x64,		//                     (NEC PC98)
		KS_F14				= 0x65,		//                     (NEC PC98)
		KS_F15				= 0x66,		//                     (NEC PC98)
		KS_Kana				= 0x70,		// (Japanese keyboard)
		KS_ABNT_C1			= 0x73,		// /? on Brazilian keyboard
		KS_Convert			= 0x79,		// (Japanese keyboard)
		KS_NoConvert		= 0x7B,		// (Japanese keyboard)
		KS_Yen				= 0x7D,		// (Japanese keyboard)
		KS_ABNT_C2			= 0x7E,		// Numpad . on Brazilian keyboard
		KS_NumPadEquals		= 0x8D,		// = on numeric keypad (NEC PC98)
		KS_PrevTrack		= 0x90,		// Previous Track (DKS_CIRCUMFLEX on Japanese keyboard)
		KS_AT				= 0x91,		//                     (NEC PC98)
		KS_Colon			= 0x92,		//                     (NEC PC98)
		KS_Underline		= 0x93,		//                     (NEC PC98)
		KS_Kanji			= 0x94,		// (Japanese keyboard)
		KS_Stop				= 0x95,		//                     (NEC PC98)
		KS_AX				= 0x96,		//                     (Japan AX)
		KS_Unlabeled		= 0x97,		//                        (J3100)
		KS_NextTrack		= 0x99,		// Next Track
		KS_NumPadEnter		= 0x9C,		// Enter on numeric keypad
		KS_RightCtrl		= 0x9D,
		KS_Mute				= 0xA0,		// Mute
		KS_Calculator		= 0xA1,		// Calculator
		KS_PlayPause		= 0xA2,		// Play / Pause
		KS_MediaStop		= 0xA4,		// Media Stop
		KS_VolumeDown		= 0xAE,		// Volume -
		KS_VolumeUp			= 0xB0,		// Volume +
		KS_WebHome			= 0xB2,		// Web home
		KS_NumPadComma		= 0xB3,		// , on numeric keypad (NEC PC98)
		KS_NumPadSlash		= 0xB5,		// / on numeric keypad
		KS_SysRQ			= 0xB7,
		KS_RightAlt			= 0xB8,		// right Alt
		KS_Pause			= 0xC5,		// Pause
		KS_Home				= 0xC7,		// Home on arrow keypad
		KS_UpArrow			= 0xC8,		// UpArrow on arrow keypad
		KS_PageUp			= 0xC9,		// PgUp on arrow keypad
		KS_LeftArrow		= 0xCB,		// LeftArrow on arrow keypad
		KS_RightArrow		= 0xCD,		// RightArrow on arrow keypad
		KS_End				= 0xCF,		// End on arrow keypad
		KS_DownArrow		= 0xD0,		// DownArrow on arrow keypad
		KS_PageDown			= 0xD1,		// PgDn on arrow keypad
		KS_Insert			= 0xD2,		// Insert on arrow keypad
		KS_Delete			= 0xD3,		// Delete on arrow keypad
		KS_LeftWin			= 0xDB,		// Left Windows key
		KS_RightWin			= 0xDC,		// Right Windows key
		KS_Apps				= 0xDD,		// AppMenu key
		KS_Power			= 0xDE,		// System Power
		KS_Sleep			= 0xDF,		// System Sleep
		KS_Wake				= 0xE3,		// System Wake
		KS_WebSearch		= 0xE5,		// Web Search
		KS_WebFavorites		= 0xE6,		// Web Favorites
		KS_WebRefresh		= 0xE7,		// Web Refresh
		KS_WebStop			= 0xE8,		// Web Stop
		KS_WebForward		= 0xE9,		// Web Forward
		KS_WebBack			= 0xEA,		// Web Back
		KS_MyComputer		= 0xEB,		// My Computer
		KS_Mail				= 0xEC,		// Mail
		KS_MediaSelect		= 0xED,		// Media Select
	};

	// 鼠标动作
	enum MouseSemantic
	{
		MS_X				= 0x100,
		MS_Y				= 0x101,
		MS_Z				= 0x102,
		MS_Button0			= 0x103,
		MS_Button1			= 0x104,
		MS_Button2			= 0x105,
		MS_Button3			= 0x106,
		MS_Button4			= 0x107,
		MS_Button5			= 0x108,
		MS_Button6			= 0x109,
		MS_Button7			= 0x10A,
	};

	// 游戏杆动作
	enum JoystickSemantic
	{
		JS_XPos				= 0x200,
		JS_YPos				= 0x201,
		JS_ZPos				= 0x202,
		JS_XRot				= 0x203,
		JS_YRot				= 0x204,
		JS_ZRot				= 0x205,
		JS_Slider0			= 0x206,
		JS_Slider1			= 0x207,
		JS_Button0			= 0x208,
		JS_Button1			= 0x209,
		JS_Button2			= 0x20A,
		JS_Button3			= 0x20B,
		JS_Button4			= 0x20C,
		JS_Button5			= 0x20D,
		JS_Button6			= 0x20E,
		JS_Button7			= 0x20F,
		JS_Button8			= 0x210,
		JS_Button9			= 0x211,
		JS_Button10			= 0x212,
		JS_Button11			= 0x213,
		JS_Button12			= 0x214,
		JS_Button13			= 0x215,
		JS_Button14			= 0x216,
		JS_Button15			= 0x217,
		JS_Button16			= 0x218,
		JS_Button17			= 0x219,
		JS_Button18			= 0x21A,
		JS_Button19			= 0x21B,
		JS_Button20			= 0x21C,
		JS_Button21			= 0x21D,
		JS_Button22			= 0x21E,
		JS_Button23			= 0x21F,
		JS_Button24			= 0x220,
		JS_Button25			= 0x221,
		JS_Button26			= 0x222,
		JS_Button27			= 0x223,
		JS_Button28			= 0x224,
		JS_Button29			= 0x225,
		JS_Button30			= 0x226,
		JS_Button31			= 0x227,
	};

	typedef std::pair<uint16_t, uint16_t> InputActionDefine;
	typedef std::pair<uint16_t, long> InputAction;
	typedef std::vector<InputAction> InputActionsType;


	// 输入动作格式
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API InputActionMap
	{
	public:
		void AddAction(InputActionDefine const & action_define);

		template <typename ForwardIterator>
		void AddActions(ForwardIterator first, ForwardIterator last)
		{
			for (ForwardIterator iter = first; iter != last; ++ iter)
			{
				this->AddAction(*iter);
			}
		}

		void UpdateInputActions(InputActionsType& actions, uint16_t key, long value = 0);

		bool HasAction(uint16_t key) const;
		uint16_t Action(uint16_t key) const;

	private:
		boost::container::flat_map<uint16_t, uint16_t> actionMap_;
	};

	typedef boost::signals2::signal<void(InputEngine const &, InputAction const &)> input_signal;
	typedef boost::shared_ptr<input_signal> action_handler_t;
	typedef boost::container::flat_map<uint32_t, InputActionMap> action_maps_t;


	// 输入引擎
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API InputEngine
	{
		typedef std::vector<std::pair<InputActionMap, action_handler_t> > action_handlers_t;

	public:
		virtual ~InputEngine();

		static InputEnginePtr NullObject();

		virtual std::wstring const & Name() const = 0;

		virtual void EnumDevices() = 0;

		void Update();
		float ElapsedTime() const;

		void ActionMap(InputActionMap const & actionMap,
			action_handler_t handler, bool reenumerate = false);

		size_t NumDevices() const;
		InputDevicePtr Device(size_t index) const;

	protected:
		typedef std::vector<InputDevicePtr>	InputDevicesType;
		InputDevicesType	devices_;

		action_handlers_t action_handlers_;

		Timer timer_;
		float elapsed_time_;
	};

	class KLAYGE_CORE_API InputDevice
	{
	public:
		virtual ~InputDevice();

		virtual std::wstring const & Name() const = 0;

		virtual void UpdateInputs() = 0;
		virtual InputActionsType UpdateActionMap(uint32_t id) = 0;

		virtual void ActionMap(uint32_t id, InputActionMap const & actionMap) = 0;

	protected:
		action_maps_t actionMaps_;
	};

	class KLAYGE_CORE_API InputKeyboard : public InputDevice
	{
	public:
		InputKeyboard();
		virtual ~InputKeyboard();

		size_t NumKeys() const;
		bool Key(size_t n) const;
		bool const * Keys() const;

		bool KeyDown(size_t n) const;
		bool KeyUp(size_t n) const;

		InputActionsType UpdateActionMap(uint32_t id);
		void ActionMap(uint32_t id, InputActionMap const & actionMap);

	protected:
		array<array<bool, 256>, 2> keys_;
		bool index_;
	};

	class KLAYGE_CORE_API InputMouse : public InputDevice
	{
	public:
		InputMouse();
		virtual ~InputMouse();

		long AbsX() const;
		long AbsY() const;

		long X() const;
		long Y() const;
		long Z() const;
		bool LeftButton() const;
		bool RightButton() const;
		bool MiddleButton() const;

		size_t NumButtons() const;
		bool Button(size_t n) const;

		bool ButtonDown(size_t n) const;
		bool ButtonUp(size_t n) const;

		InputActionsType UpdateActionMap(uint32_t id);
		void ActionMap(uint32_t id, InputActionMap const & actionMap);

	protected:
		Vector_T<long, 2> abs_pos_;
		Vector_T<long, 3> offset_;

		array<array<bool, 8>, 2> buttons_;
		bool index_;
	};

	class KLAYGE_CORE_API InputJoystick : public InputDevice
	{
	public:
		InputJoystick();
		virtual ~InputJoystick();

		long XPos() const;
		long YPos() const;
		long ZPos() const;
		long XRot() const;
		long YRot() const;
		long ZRot() const;

		size_t NumSliders() const;
		long Slider(size_t index) const;

		size_t NumButtons() const;
		bool Button(size_t n) const;

		bool ButtonDown(size_t n) const;
		bool ButtonUp(size_t n) const;

		InputActionsType UpdateActionMap(uint32_t id);
		void ActionMap(uint32_t id, InputActionMap const & actionMap);

	protected:
		Vector_T<long, 3> pos_;		// x, y, z axis position
		Vector_T<long, 3> rot_;		// x, y, z axis rotation

		Vector_T<long, 2> slider_;		// extra axes positions

		array<array<bool, 32>, 2> buttons_;	// 32 buttons
		bool index_;
	};
}

#endif		// _INPUT_HPP
