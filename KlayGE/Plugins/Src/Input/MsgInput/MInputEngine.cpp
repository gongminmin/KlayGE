/**
 * @file MInputEngine.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <system_error>

#include <KlayGE/MsgInput/MInput.hpp>

namespace KlayGE
{
	MsgInputEngine::MsgInputEngine()
	{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		mod_hid_ = ::LoadLibraryEx(TEXT("hid.dll"), nullptr, 0);
		if (nullptr == mod_hid_)
		{
			::MessageBoxW(nullptr, L"Can't load hid.dll", L"Error", MB_OK);
		}

		if (mod_hid_ != nullptr)
		{
			DynamicHidP_GetCaps_ = reinterpret_cast<HidP_GetCapsFunc>(::GetProcAddress(mod_hid_, "HidP_GetCaps"));
			DynamicHidP_GetButtonCaps_ = reinterpret_cast<HidP_GetButtonCapsFunc>(::GetProcAddress(mod_hid_, "HidP_GetButtonCaps"));
			DynamicHidP_GetValueCaps_ = reinterpret_cast<HidP_GetValueCapsFunc>(::GetProcAddress(mod_hid_, "HidP_GetValueCaps"));
			DynamicHidP_GetUsages_ = reinterpret_cast<HidP_GetUsagesFunc>(::GetProcAddress(mod_hid_, "HidP_GetUsages"));
			DynamicHidP_GetUsageValue_ = reinterpret_cast<HidP_GetUsageValueFunc>(::GetProcAddress(mod_hid_, "HidP_GetUsageValue"));
		}
#endif
	}

	MsgInputEngine::~MsgInputEngine()
	{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		on_raw_input_.disconnect();
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_DARWIN)
		on_key_down_.disconnect();
		on_key_up_.disconnect();
#if defined KLAYGE_PLATFORM_ANDROID
		on_mouse_down_.disconnect();
		on_mouse_up_.disconnect();
		on_mouse_move_.disconnect();
		on_mouse_wheel_.disconnect();
		on_joystick_axis_.disconnect();
		on_joystick_buttons_.disconnect();
#endif
#endif
		on_pointer_down_.disconnect();
		on_pointer_up_.disconnect();
		on_pointer_update_.disconnect();
		on_pointer_wheel_.disconnect();
		devices_.clear();

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
#if defined KLAYGE_HAVE_LIBOVR
		OVR::System::Destroy();
#endif

		::FreeLibrary(mod_hid_);
#endif
	}

	void MsgInputEngine::DoSuspend()
	{
		// TODO
	}

	void MsgInputEngine::DoResume()
	{
		// TODO
	}

	std::wstring const & MsgInputEngine::Name() const
	{
		static std::wstring const name(L"Message-based Input Engine");
		return name;
	}

	void MsgInputEngine::EnumDevices()
	{
		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		HWND hwnd = main_wnd->HWnd();
			
		UINT devices = 0;
		if (::GetRawInputDeviceList(nullptr, &devices, sizeof(RAWINPUTDEVICELIST)) != 0)
		{
			TERRC(std::errc::function_not_supported);
		}

		std::vector<RAWINPUTDEVICELIST> raw_input_devices(devices);
		::GetRawInputDeviceList(&raw_input_devices[0], &devices, sizeof(raw_input_devices[0]));

		RAWINPUTDEVICE rid;
		rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid.hwndTarget = hwnd;
		std::vector<RAWINPUTDEVICE> rids;
		for (size_t i = 0; i < raw_input_devices.size(); ++ i)
		{
			InputDevicePtr device;
			switch (raw_input_devices[i].dwType)
			{
			case RIM_TYPEKEYBOARD:
				device = MakeSharedPtr<MsgInputKeyboard>();
				rid.usUsage = HID_USAGE_GENERIC_KEYBOARD;
				rid.dwFlags = 0;
				rids.push_back(rid);
				break;

			case RIM_TYPEMOUSE:
				device = MakeSharedPtr<MsgInputMouse>(hwnd, raw_input_devices[i].hDevice);
				rid.usUsage = HID_USAGE_GENERIC_MOUSE;
				rid.dwFlags = 0;
				rids.push_back(rid);
				break;

			case RIM_TYPEHID:
				{
					UINT size = 0;
					if (0 == ::GetRawInputDeviceInfo(raw_input_devices[i].hDevice, RIDI_DEVICEINFO, nullptr, &size))
					{
						std::vector<uint8_t> buf(size);
						::GetRawInputDeviceInfo(raw_input_devices[i].hDevice, RIDI_DEVICEINFO, &buf[0], &size);

						RID_DEVICE_INFO* info = reinterpret_cast<RID_DEVICE_INFO*>(&buf[0]);
						if ((HID_USAGE_PAGE_GENERIC == info->hid.usUsagePage)
							&& ((HID_USAGE_GENERIC_GAMEPAD == info->hid.usUsage)
								|| (HID_USAGE_GENERIC_JOYSTICK == info->hid.usUsage)))
						{
							device = MakeSharedPtr<MsgInputJoystick>(raw_input_devices[i].hDevice);
							rid.usUsage = HID_USAGE_GENERIC_GAMEPAD;
							rid.dwFlags = RIDEV_INPUTSINK;
							rids.push_back(rid);
							rid.usUsage = HID_USAGE_GENERIC_JOYSTICK;
							rid.dwFlags = RIDEV_INPUTSINK;
							rids.push_back(rid);
						}
					}
				}
				break;

			default:
				break;
			}
			
			if (device)
			{
				devices_.push_back(device);
			}
		}

		if (::RegisterRawInputDevices(&rids[0], static_cast<UINT>(rids.size()), sizeof(rids[0])))
		{
			on_raw_input_ = main_wnd->OnRawInput().connect(
				[this](Window const & wnd, HRAWINPUT ri)
				{
					this->OnRawInput(wnd, ri);
				});
		}
#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_DARWIN)
		on_key_down_ = main_wnd->OnKeyDown().connect(
			[this](Window const & wnd, uint32_t key)
			{
				KFL_UNUSED(wnd);
				this->OnKeyDown(key);
			});
		on_key_up_ = main_wnd->OnKeyUp().connect(
			[this](Window const & wnd, uint32_t key)
			{
				KFL_UNUSED(wnd);
				this->OnKeyUp(key);
			});
		devices_.push_back(MakeSharedPtr<MsgInputKeyboard>());
#if defined KLAYGE_PLATFORM_ANDROID
		on_mouse_down_ = main_wnd->OnMouseDown().connect(
			[this](Window const & wnd, int2 const & pt, uint32_t buttons)
			{
				KFL_UNUSED(wnd);
				this->OnMouseDown(pt, buttons);
			});
		on_mouse_up_ = main_wnd->OnMouseUp().connect(
			[this](Window const & wnd, int2 const & pt, uint32_t buttons)
			{
				KFL_UNUSED(wnd);
				this->OnMouseUp(pt, buttons);
			});
		on_mouse_move_ = main_wnd->OnMouseMove().connect(
			[this](Window const & wnd, int2 const & pt)
			{
				KFL_UNUSED(wnd);
				this->OnMouseMove(pt);
			});
		on_mouse_wheel_ = main_wnd->OnMouseWheel().connect(
			[this](Window const & wnd, int2 const & pt, int32_t wheel_delta)
			{
				KFL_UNUSED(wnd);
				this->OnMouseWheel(pt, wheel_delta);
			});
		devices_.push_back(MakeSharedPtr<MsgInputMouse>());

		on_joystick_axis_ = main_wnd->OnJoystickAxis().connect(
			[this](Window const & wnd, uint32_t axis, int32_t value)
			{
				KFL_UNUSED(wnd);
				this->OnJoystickAxis(axis, value);
			});
		on_joystick_buttons_ = main_wnd->OnJoystickButtons().connect(
			[this](Window const & wnd, uint32_t buttons)
			{
				KFL_UNUSED(wnd);
				this->OnJoystickButtons(buttons);
			});
		devices_.push_back(MakeSharedPtr<MsgInputJoystick>());
#endif
#elif defined KLAYGE_PLATFORM_LINUX
		// TODO
		KFL_UNUSED(main_wnd);
#endif

#if ((defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) && (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)) \
			|| defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_DARWIN)
		on_pointer_down_ = main_wnd->OnPointerDown().connect(
			[this](Window const & wnd, int2 const & pt, uint32_t id)
			{
				KFL_UNUSED(wnd);
				this->OnPointerDown(pt, id);
			});
		on_pointer_up_ = main_wnd->OnPointerUp().connect(
			[this](Window const & wnd, int2 const & pt, uint32_t id)
			{
				KFL_UNUSED(wnd);
				this->OnPointerUp(pt, id);
			});
		on_pointer_update_ = main_wnd->OnPointerUpdate().connect(
			[this](Window const & wnd, int2 const & pt, uint32_t id, bool down)
			{
				KFL_UNUSED(wnd);
				this->OnPointerUpdate(pt, id, down);
			});
		on_pointer_wheel_ = main_wnd->OnPointerWheel().connect(
			[this](Window const & wnd, int2 const & pt, uint32_t id, int32_t wheel_delta)
			{
				KFL_UNUSED(wnd);
				this->OnPointerWheel(pt, id, wheel_delta);
			});
		devices_.push_back(MakeSharedPtr<MsgInputTouch>());
#endif

#if (defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) && (defined KLAYGE_HAVE_LIBOVR)
		OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
		devices_.push_back(MakeSharedPtr<MsgInputOVR>());
#endif

#if defined(KLAYGE_PLATFORM_WINDOWS_DESKTOP) || defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID)
		devices_.push_back(MakeSharedPtr<MsgInputSensor>());
#endif
	}

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	void MsgInputEngine::OnRawInput(Window const & wnd, HRAWINPUT ri)
	{
		if (wnd.HWnd() == ::GetForegroundWindow())
		{
			UINT size = 0;
			if (0 == ::GetRawInputData(ri, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)))
			{
				std::vector<uint8_t> data(size);
				::GetRawInputData(ri, RID_INPUT, &data[0], &size, sizeof(RAWINPUTHEADER));

				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(&data[0]);

				for (auto const & device : devices_)
				{
					switch (raw->header.dwType)
					{
					case RIM_TYPEKEYBOARD:
						if (IDT_Keyboard == device->Type())
						{
							checked_pointer_cast<MsgInputKeyboard>(device)->OnRawInput(*raw);
						}
						break;

					case RIM_TYPEMOUSE:
						if (IDT_Mouse == device->Type())
						{
							checked_pointer_cast<MsgInputMouse>(device)->OnRawInput(*raw);
						}
						break;

					case RIM_TYPEHID:
						if (IDT_Joystick == device->Type())
						{
							checked_pointer_cast<MsgInputJoystick>(device)->OnRawInput(*raw);
						}
						break;

					default:
						break;
					}
				}
			}
		}
	}

#elif defined(KLAYGE_PLATFORM_WINDOWS_STORE) || defined(KLAYGE_PLATFORM_ANDROID) || defined(KLAYGE_PLATFORM_DARWIN)
	void MsgInputEngine::OnKeyDown(uint32_t key)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Keyboard == device->Type())
			{
				checked_pointer_cast<MsgInputKeyboard>(device)->OnKeyDown(key);
			}
		}
	}

	void MsgInputEngine::OnKeyUp(uint32_t key)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Keyboard == device->Type())
			{
				checked_pointer_cast<MsgInputKeyboard>(device)->OnKeyUp(key);
			}
		}
	}

#if defined KLAYGE_PLATFORM_ANDROID
	void MsgInputEngine::OnMouseDown(int2 const & pt, uint32_t buttons)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Mouse == device->Type())
			{
				checked_pointer_cast<MsgInputMouse>(device)->OnMouseDown(pt, buttons);
			}
		}
	}

	void MsgInputEngine::OnMouseUp(int2 const & pt, uint32_t buttons)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Mouse == device->Type())
			{
				checked_pointer_cast<MsgInputMouse>(device)->OnMouseUp(pt, buttons);
			}
		}
	}

	void MsgInputEngine::OnMouseMove(int2 const & pt)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Mouse == device->Type())
			{
				checked_pointer_cast<MsgInputMouse>(device)->OnMouseMove(pt);
			}
		}
	}

	void MsgInputEngine::OnMouseWheel(int2 const & pt, int32_t wheel_delta)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Mouse == device->Type())
			{
				checked_pointer_cast<MsgInputMouse>(device)->OnMouseWheel(pt, wheel_delta);
			}
		}
	}

	void MsgInputEngine::OnJoystickAxis(uint32_t axis, int32_t value)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Joystick == device->Type())
			{
				checked_pointer_cast<MsgInputJoystick>(device)->OnJoystickAxis(axis, value);
			}
		}
	}

	void MsgInputEngine::OnJoystickButtons(uint32_t buttons)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Joystick == device->Type())
			{
				checked_pointer_cast<MsgInputJoystick>(device)->OnJoystickButtons(buttons);
			}
		}
	}
#endif
#endif

	void MsgInputEngine::OnPointerDown(int2 const & pt, uint32_t id)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnPointerDown(pt, id);
			}
		}
	}

	void MsgInputEngine::OnPointerUp(int2 const & pt, uint32_t id)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnPointerUp(pt, id);
			}
		}
	}

	void MsgInputEngine::OnPointerUpdate(int2 const & pt, uint32_t id, bool down)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnPointerUpdate(pt, id, down);
			}
		}
	}

	void MsgInputEngine::OnPointerWheel(int2 const & pt, uint32_t id, int32_t wheel_delta)
	{
		for (auto const & device : devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnPointerWheel(pt, id, wheel_delta);
			}
		}
	}

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	NTSTATUS MsgInputEngine::HidP_GetCaps(PHIDP_PREPARSED_DATA PreparsedData, PHIDP_CAPS Capabilities) const
	{
		return DynamicHidP_GetCaps_(PreparsedData, Capabilities);
	}

	NTSTATUS MsgInputEngine::HidP_GetButtonCaps(HIDP_REPORT_TYPE ReportType, PHIDP_BUTTON_CAPS ButtonCaps,
		PUSHORT ButtonCapsLength, PHIDP_PREPARSED_DATA PreparsedData) const
	{
		return DynamicHidP_GetButtonCaps_(ReportType, ButtonCaps, ButtonCapsLength, PreparsedData);
	}

	NTSTATUS MsgInputEngine::HidP_GetValueCaps(HIDP_REPORT_TYPE ReportType, PHIDP_VALUE_CAPS ValueCaps,
		PUSHORT ValueCapsLength, PHIDP_PREPARSED_DATA PreparsedData) const
	{
		return DynamicHidP_GetValueCaps_(ReportType, ValueCaps, ValueCapsLength, PreparsedData);
	}

	NTSTATUS MsgInputEngine::HidP_GetUsages(HIDP_REPORT_TYPE ReportType, USAGE UsagePage,
		USHORT LinkCollection, PUSAGE UsageList, PULONG UsageLength, PHIDP_PREPARSED_DATA PreparsedData,
		PCHAR Report, ULONG ReportLength) const
	{
		return DynamicHidP_GetUsages_(ReportType, UsagePage, LinkCollection, UsageList, UsageLength, PreparsedData,
			Report, ReportLength);
	}

	NTSTATUS MsgInputEngine::HidP_GetUsageValue(HIDP_REPORT_TYPE ReportType, USAGE UsagePage,
		USHORT LinkCollection, USAGE Usage, PULONG UsageValue, PHIDP_PREPARSED_DATA PreparsedData,
		PCHAR Report, ULONG ReportLength) const
	{
		return DynamicHidP_GetUsageValue_(ReportType, UsagePage, LinkCollection, Usage, UsageValue, PreparsedData,
			Report, ReportLength);
	}
#endif
}
