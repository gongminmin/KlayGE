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
#include <KFL/ThrowErr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

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

#if (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)
		HMODULE mod_user = ::GetModuleHandle(TEXT("user32"));
		BOOST_ASSERT(mod_user != nullptr);

		DynamicRegisterTouchWindow_ = reinterpret_cast<RegisterTouchWindowFunc>(::GetProcAddress(mod_user, "RegisterTouchWindow"));
		DynamicGetTouchInputInfo_ = reinterpret_cast<GetTouchInputInfoFunc>(::GetProcAddress(mod_user, "GetTouchInputInfo"));
		DynamicCloseTouchInputHandle_ = reinterpret_cast<CloseTouchInputHandleFunc>(::GetProcAddress(mod_user, "CloseTouchInputHandle"));
#endif
#endif
	}

	MsgInputEngine::~MsgInputEngine()
	{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		on_raw_input_.disconnect();
#elif defined KLAYGE_PLATFORM_ANDROID
		on_key_down_.disconnect();
		on_key_up_.disconnect();
#endif
		on_touch_.disconnect();
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
			
		UINT devices;
		if (::GetRawInputDeviceList(nullptr, &devices, sizeof(RAWINPUTDEVICELIST)) != 0)
		{
			THR(errc::function_not_supported);
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
				device = MakeSharedPtr<MsgInputKeyboard>(hwnd, raw_input_devices[i].hDevice);
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
					UINT size;
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
			on_raw_input_ = main_wnd->OnRawInput().connect(bind(&MsgInputEngine::OnRawInput, this,
				placeholders::_1, placeholders::_2));
		}
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
		on_pointer_down_ = main_wnd->OnPointerDown().connect(KlayGE::bind(&MsgInputEngine::OnPointerDown, this,
			KlayGE::placeholders::_2, placeholders::_3));
		on_pointer_up_ = main_wnd->OnPointerUp().connect(KlayGE::bind(&MsgInputEngine::OnPointerUp, this,
			KlayGE::placeholders::_2, KlayGE::placeholders::_3));
		on_pointer_update_ = main_wnd->OnPointerUpdate().connect(KlayGE::bind(&MsgInputEngine::OnPointerUpdate, this,
			KlayGE::placeholders::_2, KlayGE::placeholders::_3, KlayGE::placeholders::_4));
		on_pointer_wheel_ = main_wnd->OnPointerWheel().connect(KlayGE::bind(&MsgInputEngine::OnPointerWheel, this,
			KlayGE::placeholders::_2, KlayGE::placeholders::_3, KlayGE::placeholders::_4));
		devices_.push_back(MakeSharedPtr<MsgInputTouch>());
#elif (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)
		if (::GetSystemMetrics(SM_DIGITIZER) & NID_READY)
		{
			if (this->RegisterTouchWindow(hwnd, TWF_WANTPALM))
			{
				on_touch_ = main_wnd->OnTouch().connect(KlayGE::bind(&MsgInputEngine::OnTouch, this,
					KlayGE::placeholders::_1, KlayGE::placeholders::_2, KlayGE::placeholders::_3));
				devices_.push_back(MakeSharedPtr<MsgInputTouch>());
			}
		}
#endif
#elif (defined KLAYGE_PLATFORM_WINDOWS_METRO) || (defined KLAYGE_PLATFORM_ANDROID)
		on_pointer_down_ = main_wnd->OnPointerDown().connect(KlayGE::bind(&MsgInputEngine::OnPointerDown, this,
			KlayGE::placeholders::_2, placeholders::_3));
		on_pointer_up_ = main_wnd->OnPointerUp().connect(KlayGE::bind(&MsgInputEngine::OnPointerUp, this,
			KlayGE::placeholders::_2, KlayGE::placeholders::_3));
		on_pointer_update_ = main_wnd->OnPointerUpdate().connect(KlayGE::bind(&MsgInputEngine::OnPointerUpdate, this,
			KlayGE::placeholders::_2, KlayGE::placeholders::_3, KlayGE::placeholders::_4));
		on_pointer_wheel_ = main_wnd->OnPointerWheel().connect(KlayGE::bind(&MsgInputEngine::OnPointerWheel, this,
			KlayGE::placeholders::_2, KlayGE::placeholders::_3, KlayGE::placeholders::_4));
		devices_.push_back(MakeSharedPtr<MsgInputTouch>());
#if defined KLAYGE_PLATFORM_ANDROID
		on_key_down_ = main_wnd->OnKeyDown().connect(KlayGE::bind(&MsgInputEngine::OnKeyDown, this,
			KlayGE::placeholders::_2));
		on_key_up_ = main_wnd->OnKeyDown().connect(KlayGE::bind(&MsgInputEngine::OnKeyUp, this,
			KlayGE::placeholders::_2));
		devices_.push_back(MakeSharedPtr<MsgInputKeyboard>());
#endif
#endif

#if (defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) && (defined KLAYGE_HAVE_LIBOVR)
		OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
		devices_.push_back(MakeSharedPtr<MsgInputOVR>());
#endif

#if ((defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) && (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)) \
			|| (defined KLAYGE_PLATFORM_WINDOWS_METRO) || (defined KLAYGE_PLATFORM_ANDROID)
		devices_.push_back(MakeSharedPtr<MsgInputSensor>());
#endif
	}

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	void MsgInputEngine::OnRawInput(Window const & /*wnd*/, HRAWINPUT ri)
	{
		UINT size;
		if (0 == ::GetRawInputData(ri, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)))
		{
			std::vector<uint8_t> data(size);
			::GetRawInputData(ri, RID_INPUT, &data[0], &size, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(&data[0]);

			typedef KLAYGE_DECLTYPE(devices_) DevicesType;
			KLAYGE_FOREACH(DevicesType::reference device, devices_)
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

#if (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)
	void MsgInputEngine::OnTouch(Window const & wnd, HTOUCHINPUT hti, uint32_t num_inputs)
	{
		typedef KLAYGE_DECLTYPE(devices_) DevicesType;
		KLAYGE_FOREACH(DevicesType::reference device, devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnTouch(wnd, hti, num_inputs);
			}
		}
	}
#endif
#endif

	void MsgInputEngine::OnPointerDown(int2 const & pt, uint32_t id)
	{
		typedef KLAYGE_DECLTYPE(devices_) DevicesType;
		KLAYGE_FOREACH(DevicesType::reference device, devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnPointerDown(pt, id);
			}
		}
	}

	void MsgInputEngine::OnPointerUp(int2 const & pt, uint32_t id)
	{
		typedef KLAYGE_DECLTYPE(devices_) DevicesType;
		KLAYGE_FOREACH(DevicesType::reference device, devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnPointerUp(pt, id);
			}
		}
	}
	
	void MsgInputEngine::OnPointerUpdate(int2 const & pt, uint32_t id, bool down)
	{
		typedef KLAYGE_DECLTYPE(devices_) DevicesType;
		KLAYGE_FOREACH(DevicesType::reference device, devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnPointerUpdate(pt, id, down);
			}
		}
	}
	
	void MsgInputEngine::OnPointerWheel(int2 const & pt, uint32_t id, int32_t wheel_delta)
	{
		typedef KLAYGE_DECLTYPE(devices_) DevicesType;
		KLAYGE_FOREACH(DevicesType::reference device, devices_)
		{
			if (InputEngine::IDT_Touch == device->Type())
			{
				checked_pointer_cast<MsgInputTouch>(device)->OnPointerWheel(pt, id, wheel_delta);
			}
		}
	}

#if defined KLAYGE_PLATFORM_ANDROID
	void MsgInputEngine::OnKeyDown(uint32_t key)
	{
		typedef KLAYGE_DECLTYPE(devices_) DevicesType;
		KLAYGE_FOREACH(DevicesType::reference device, devices_)
		{
			if (InputEngine::IDT_Keyboard == device->Type())
			{
				checked_pointer_cast<MsgInputKeyboard>(device)->OnKeyDown(key);
			}
		}
	}

	void MsgInputEngine::OnKeyUp(uint32_t key)
	{
		typedef KLAYGE_DECLTYPE(devices_) DevicesType;
		KLAYGE_FOREACH(DevicesType::reference device, devices_)
		{
			if (InputEngine::IDT_Keyboard == device->Type())
			{
				checked_pointer_cast<MsgInputKeyboard>(device)->OnKeyUp(key);
			}
		}
	}
#endif

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

#if (_WIN32_WINNT >= 0x0601 /*_WIN32_WINNT_WIN7*/)
	BOOL MsgInputEngine::RegisterTouchWindow(HWND hWnd, ULONG ulFlags) const
	{
		return DynamicRegisterTouchWindow_(hWnd, ulFlags);
	}

	BOOL MsgInputEngine::GetTouchInputInfo(HTOUCHINPUT hTouchInput, UINT cInputs, PTOUCHINPUT pInputs, int cbSize) const
	{
		return DynamicGetTouchInputInfo_(hTouchInput, cInputs, pInputs, cbSize);
	}

	BOOL MsgInputEngine::CloseTouchInputHandle(HTOUCHINPUT hTouchInput) const
	{
		return DynamicCloseTouchInputHandle_(hTouchInput);
	}
#endif
#endif
}
