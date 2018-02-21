#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "InputCaps.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	std::wstring key_name[] = 
	{
		L"",
		L"Escape",
		L"1",
		L"2",
		L"3",
		L"4",
		L"5",
		L"6",
		L"7",
		L"8",
		L"9",
		L"0",
		L"Minus",
		L"Equals",
		L"BackSpace",
		L"Tab",
		L"Q",
		L"W",
		L"E",
		L"R",
		L"T",
		L"Y",
		L"U",
		L"I",
		L"O",
		L"P",
		L"LeftBracket",
		L"RightBracket",
		L"Enter",
		L"LeftCtrl",
		L"A",
		L"S",
		L"D",
		L"F",
		L"G",
		L"H",
		L"J",
		L"K",
		L"L",
		L"Semicolon",
		L"Apostrophe",
		L"Grave",
		L"LeftShift",
		L"BackSlash",
		L"Z",
		L"X",
		L"C",
		L"V",
		L"B",
		L"N",
		L"M",
		L"Comma",
		L"Period",
		L"Slash",
		L"RightShift",
		L"NumPadStar",
		L"LeftAlt",
		L"Space",
		L"CapsLock",
		L"F1",
		L"F2",
		L"F3",
		L"F4",
		L"F5",
		L"F6",
		L"F7",
		L"F8",
		L"F9",
		L"F10",
		L"NumLock",
		L"ScrollLock",
		L"NumPad7",
		L"NumPad8",
		L"NumPad9",
		L"NumPadMinus",
		L"NumPad4",
		L"NumPad5",
		L"NumPad6",
		L"NumPadPlus",
		L"NumPad1",
		L"NumPad2",
		L"NumPad3",
		L"NumPad0",
		L"NumPadPeriod",
		L"OEM_102",
		L"F11",
		L"F12",
		L"F13",
		L"F14",
		L"F15",
		L"Kana",
		L"ABNT_C1",
		L"Convert",
		L"NoConvert",
		L"Yen",
		L"ABNT_C2",
		L"NumPadEquals",
		L"PrevTrack",
		L"AT",
		L"Colon",
		L"Underline",
		L"Kanji",
		L"Stop",
		L"AX",
		L"Unlabeled",
		L"NextTrack",
		L"NumPadEnter",
		L"RightCtrl",
		L"Mute",
		L"Calculator",
		L"PlayPause",
		L"MediaStop",
		L"VolumeDown",
		L"VolumeUp",
		L"WebHome",
		L"NumPadComma",
		L"NumPadSlash",
		L"SysRQ",
		L"RightAlt",
		L"Pause",
		L"Home",
		L"UpArrow",
		L"PageUp",
		L"LeftArrow",
		L"RightArrow",
		L"End",
		L"DownArrow",
		L"PageDown",
		L"Insert",
		L"Delete",
		L"LeftWin",
		L"RightWin",
		L"Apps",
		L"Power",
		L"Sleep",
		L"Wake",
		L"WebSearch",
		L"WebFavorites",
		L"WebRefresh",
		L"WebStop",
		L"WebForward",
		L"WebBack",
		L"MyComputer",
		L"Mail",
		L"MediaSelect"
	};

	std::wstring touch_name[] = 
	{
		L"None",
		L"Pan",
		L"Tap",
		L"Press",
		L"PressAndTap",
		L"Zoom",
		L"Rotate",
		L"Flick"
	};

	enum
	{
		KeyboardMsg,
		MouseMsg,
		JoystickMsg,
		TouchMsg,
		SensorMsg,

		Exit
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(KeyboardMsg, KS_AnyKey),

		InputActionDefine(MouseMsg, MS_X),
		InputActionDefine(MouseMsg, MS_Y),
		InputActionDefine(MouseMsg, MS_Z),
		InputActionDefine(MouseMsg, MS_AnyButton),

		InputActionDefine(JoystickMsg, JS_XPos),
		InputActionDefine(JoystickMsg, JS_YPos),
		InputActionDefine(JoystickMsg, JS_ZPos),
		InputActionDefine(JoystickMsg, JS_XRot),
		InputActionDefine(JoystickMsg, JS_YRot),
		InputActionDefine(JoystickMsg, JS_ZRot),
		InputActionDefine(JoystickMsg, JS_Slider0),
		InputActionDefine(JoystickMsg, JS_Slider1),
		InputActionDefine(JoystickMsg, JS_AnyButton),

		InputActionDefine(TouchMsg, TS_Pan),
		InputActionDefine(TouchMsg, TS_Tap),
		InputActionDefine(TouchMsg, TS_Press),
		InputActionDefine(TouchMsg, TS_PressAndTap),
		InputActionDefine(TouchMsg, TS_Zoom),
		InputActionDefine(TouchMsg, TS_Rotate),
		InputActionDefine(TouchMsg, TS_Flick),
		InputActionDefine(TouchMsg, TS_Wheel),
		InputActionDefine(TouchMsg, TS_AnyTouch),

		InputActionDefine(SensorMsg, SS_AnySensing),

		InputActionDefine(Exit, KS_Escape)
	};
}

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.location_sensor = true;
	Context::Instance().Config(cfg);

	InputCaps app;
	app.Create();
	app.Run();

	return 0;
}

InputCaps::InputCaps()
			: App3DFramework("InputCaps")
{
	ResLoader::Instance().AddPath("../../Tutorials/media/InputCaps");
}

void InputCaps::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	this->LookAt(float3(2, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 100);

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

	UIManager::Instance().Load(ResLoader::Instance().Open("InputCaps.uiml"));
}

void InputCaps::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void InputCaps::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case KeyboardMsg:
		{
			key_str_.clear();
			InputKeyboardActionParamPtr param = checked_pointer_cast<InputKeyboardActionParam>(action.second);
			for (uint32_t i = 0; i < 0xEF; ++ i)
			{
				if (param->buttons_state[i])
				{
					key_str_ += key_name[i] + L' ';
				}
			}
		}
		break;

	case MouseMsg:
		{
			InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
			std::wostringstream stream;
			stream << param->abs_coord.x() << ' ' << param->abs_coord.y() << ' ';
			stream << param->move_vec.x() << ' ' << param->move_vec.y() << ' ' << param->wheel_delta << ' ';
			for (uint32_t i = 0; i < 8; ++ i)
			{
				if (param->buttons_state & (1UL << i))
				{
					stream << "button" << i << L' ';
				}
			}
			mouse_str_ = stream.str();
		}
		break;

	case JoystickMsg:
		{
			InputJoystickActionParamPtr param = checked_pointer_cast<InputJoystickActionParam>(action.second);
			std::wostringstream stream;
			stream << param->pos.x() << ' ' << param->pos.y() << ' ' << param->pos.z() << ' ';
			stream << param->rot.x() << ' ' << param->rot.y() << ' ' << param->rot.z() << ' ';
			stream << param->slider.x() << ' ' << param->slider.y() << ' ';
			for (uint32_t i = 0; i < 16; ++ i)
			{
				if (param->buttons_state & (1UL << i))
				{
					stream << "button" << i << L' ';
				}
			}
			joystick_str_ = stream.str();
		}
		break;

	case TouchMsg:
		{
			InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);
			std::wostringstream stream;
			stream << touch_name[param->gesture - 0x300] << ' ';
			if (param->gesture != TS_None)
			{
				stream << "center " << param->center.x() << ' ' << param->center.y() << ' ';
				switch (param->gesture)
				{
				case TS_Pan:
				case TS_Tap:
				case TS_Flick:
					stream << "move " << param->move_vec.x() << ' ' << param->move_vec.y() << ' ';
					break;

				case TS_Zoom:
					stream << "factor " << param->zoom << ' ';
					break;
				
				case TS_Rotate:
					stream << "angle " << param->rotate_angle << ' ';
					break;

				default:
					break;
				}
			}
			if (param->wheel_delta != 0)
			{
				stream << "Wheel " << param->wheel_delta << ' ';
			}			
			for (uint32_t i = 0; i < 16; ++ i)
			{
				if (param->touches_down & (1UL << i))
				{
					stream << "Touch" << i << L" Down ";
				}
				if (param->touches_up & (1UL << i))
				{
					stream << "Touch" << i << L" Up ";
				}
			}
			touch_str_ = stream.str();
		}
		break;

	case SensorMsg:
		{
			InputSensorActionParamPtr param = checked_pointer_cast<InputSensorActionParam>(action.second);
			std::wostringstream stream;
			stream << "Lat: " << param->latitude << "  Lng: " << param->longitude;
			stream << " Orientation: " << param->orientation_quat.x() << ' ' << param->orientation_quat.y()
				<< ' ' << param->orientation_quat.z() << ' ' << param->orientation_quat.w();
			sensor_str_ = stream.str();
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void InputCaps::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Input Caps", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), renderEngine.Name(), 16);

	font_->RenderText(0, 72, Color(1, 1, 1, 1), L"Keyboard: ", 24);
	font_->RenderText(128, 72, Color(1, 1, 1, 1), key_str_, 24);

	font_->RenderText(0, 99, Color(1, 1, 1, 1), L"Mouse: ", 24);
	font_->RenderText(128, 99, Color(1, 1, 1, 1), mouse_str_, 24);

	font_->RenderText(0, 126, Color(1, 1, 1, 1), L"Joystick: ", 24);
	font_->RenderText(128, 126, Color(1, 1, 1, 1), joystick_str_, 24);

	font_->RenderText(0, 153, Color(1, 1, 1, 1), L"Touch: ", 24);
	font_->RenderText(128, 153, Color(1, 1, 1, 1), touch_str_, 24);

	font_->RenderText(0, 180, Color(1, 1, 1, 1), L"Sensor: ", 24);
	font_->RenderText(128, 180, Color(1, 1, 1, 1), sensor_str_, 24);
}

uint32_t InputCaps::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
