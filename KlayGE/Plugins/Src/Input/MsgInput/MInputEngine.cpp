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
		mod_hid_ = ::LoadLibraryW(L"hid.dll");
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
	}

	MsgInputEngine::~MsgInputEngine()
	{
		on_raw_input_.disconnect();
		devices_.clear();

		::FreeLibrary(mod_hid_);
	}

	std::wstring const & MsgInputEngine::Name() const
	{
		static std::wstring const name(L"Message-based Input Engine");
		return name;
	}

	void MsgInputEngine::EnumDevices()
	{
		WindowPtr const & main_wnd = Context::Instance().AppInstance().MainWnd();
		HWND hwnd = main_wnd->HWnd();
			
		UINT devices;
		if (GetRawInputDeviceList(NULL, &devices, sizeof(RAWINPUTDEVICELIST)) != 0)
		{
			THR(errc::function_not_supported);
		}

		std::vector<RAWINPUTDEVICELIST> raw_input_devices(devices);
		GetRawInputDeviceList(&raw_input_devices[0], &devices, sizeof(raw_input_devices[0]));

		RAWINPUTDEVICE rid;
		std::vector<RAWINPUTDEVICE> rids;
		for (size_t i = 0; i < raw_input_devices.size(); ++ i)
		{
			InputDevicePtr device;
			switch (raw_input_devices[i].dwType)
			{
			case RIM_TYPEKEYBOARD:
				device = MakeSharedPtr<MsgInputKeyboard>();
				rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
				rid.usUsage = HID_USAGE_GENERIC_KEYBOARD;
				rid.dwFlags = RIDEV_INPUTSINK;
				rid.hwndTarget = hwnd;
				rids.push_back(rid);
				break;

			case RIM_TYPEMOUSE:
				device = MakeSharedPtr<MsgInputMouse>();
				rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
				rid.usUsage = HID_USAGE_GENERIC_MOUSE;
				rid.dwFlags = 0;
				rid.hwndTarget = hwnd;
				rids.push_back(rid);
				break;

			case RIM_TYPEHID:
				device = MakeSharedPtr<MsgInputJoystick>();
				rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
				rid.usUsage = HID_USAGE_GENERIC_GAMEPAD;
				rid.dwFlags = RIDEV_INPUTSINK;
				rid.hwndTarget = hwnd;
				rids.push_back(rid);
				rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
				rid.usUsage = HID_USAGE_GENERIC_JOYSTICK;
				rid.dwFlags = RIDEV_INPUTSINK;
				rid.hwndTarget = hwnd;
				rids.push_back(rid);
				break;

			default:
				break;
			}
			
			if (device)
			{
				devices_.push_back(device);
			}
		}

		if (RegisterRawInputDevices(&rids[0], rids.size(), sizeof(rids[0])))
		{
			on_raw_input_ = main_wnd->OnRawInput().connect(bind(&MsgInputEngine::OnRawInput, this,
				placeholders::_1, placeholders::_2));
		}
	}

	void MsgInputEngine::OnRawInput(Window const & /*wnd*/, uint64_t param)
	{
		HRAWINPUT ri = reinterpret_cast<HRAWINPUT>(param);
		UINT size;
		if (0 == GetRawInputData(ri, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER)))
		{
			std::vector<uint8_t> data(size);
			if (GetRawInputData(ri, RID_INPUT, &data[0], &size, sizeof(RAWINPUTHEADER)) >= 0)
			{
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(&data[0]);

				typedef KLAYGE_DECLTYPE(devices_) DevicesType;
				KLAYGE_FOREACH(DevicesType::reference device, devices_)
				{
					dynamic_pointer_cast<MsgInputDevice>(device)->OnRawInput(*raw);
				}
			}
		}
	}

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
}
