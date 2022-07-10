/**
 * @file MIJoystick.cpp
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
#include <KlayGE/Context.hpp>
#include <KlayGE/InputFactory.hpp>

#include <KlayGE/MsgInput/MInput.hpp>

#if (defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) || (defined KLAYGE_PLATFORM_ANDROID)
namespace KlayGE
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	MsgInputJoystick::MsgInputJoystick(HANDLE device)
#elif defined KLAYGE_PLATFORM_ANDROID
	MsgInputJoystick::MsgInputJoystick()
#endif
	{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		auto const& mie = checked_cast<MsgInputEngine const&>(Context::Instance().InputFactoryInstance().InputEngineInstance());

		UINT size = 0;
		if (0 == ::GetRawInputDeviceInfo(device, RIDI_PREPARSEDDATA, nullptr, &size))
		{
			auto buf = MakeUniquePtr<uint8_t[]>(size);
			::GetRawInputDeviceInfo(device, RIDI_PREPARSEDDATA, buf.get(), &size);

			PHIDP_PREPARSED_DATA preparsed_data = reinterpret_cast<PHIDP_PREPARSED_DATA>(&buf[0]);

			HIDP_CAPS caps;
			if (HIDP_STATUS_SUCCESS == mie.HidP_GetCaps(preparsed_data, &caps))
			{
				auto button_caps = MakeUniquePtr<HIDP_BUTTON_CAPS[]>(caps.NumberInputButtonCaps);

				uint16_t caps_length = caps.NumberInputButtonCaps;
				if (HIDP_STATUS_SUCCESS == mie.HidP_GetButtonCaps(HidP_Input, button_caps.get(), &caps_length, preparsed_data))
				{
					num_buttons_ = std::min<uint32_t>(static_cast<uint32_t>(buttons_[0].size()),
						button_caps[0].Range.UsageMax - button_caps[0].Range.UsageMin + 1);
				}
			}
		}

		size = 0;
		if (0 == ::GetRawInputDeviceInfo(device, RIDI_DEVICEINFO, nullptr, &size))
		{
			auto buf = MakeUniquePtr<uint8_t[]>(size);
			::GetRawInputDeviceInfo(device, RIDI_DEVICEINFO, buf.get(), &size);

			RID_DEVICE_INFO* info = reinterpret_cast<RID_DEVICE_INFO*>(buf.get());
			device_id_ = info->hid.dwProductId;
		}
#elif defined KLAYGE_PLATFORM_ANDROID
		num_buttons_ = 32;
#endif
	}
	
	const std::wstring& MsgInputJoystick::Name() const
	{
		static std::wstring const name(L"MsgInput Joystick");
		return name;
	}

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	void MsgInputJoystick::OnRawInput(RAWINPUT const & ri)
	{
		BOOST_ASSERT(RIM_TYPEHID == ri.header.dwType);

		UINT size = 0;
		if (0 == ::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_DEVICEINFO, nullptr, &size))
		{
			auto buf = MakeUniquePtr<uint8_t[]>(size);
			::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_DEVICEINFO, buf.get(), &size);

			RID_DEVICE_INFO* info = reinterpret_cast<RID_DEVICE_INFO*>(buf.get());
			if (device_id_ != info->hid.dwProductId)
			{
				return;
			}
		}

		auto const& mie = checked_cast<MsgInputEngine const&>(Context::Instance().InputFactoryInstance().InputEngineInstance());

		size = 0;
		if (0 == ::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_PREPARSEDDATA, nullptr, &size))
		{
			auto buf = MakeUniquePtr<uint8_t[]>(size);
			::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_PREPARSEDDATA, buf.get(), &size);

			PHIDP_PREPARSED_DATA preparsed_data = reinterpret_cast<PHIDP_PREPARSED_DATA>(buf.get());

			HIDP_CAPS caps;
			if (HIDP_STATUS_SUCCESS == mie.HidP_GetCaps(preparsed_data, &caps))
			{
				auto button_caps = MakeUniquePtr<HIDP_BUTTON_CAPS[]>(caps.NumberInputButtonCaps);

				uint16_t caps_length = caps.NumberInputButtonCaps;
				if (HIDP_STATUS_SUCCESS == mie.HidP_GetButtonCaps(HidP_Input, button_caps.get(), &caps_length, preparsed_data))
				{
					auto value_caps = MakeUniquePtr<HIDP_VALUE_CAPS[]>(caps.NumberInputValueCaps);
					caps_length = caps.NumberInputValueCaps;
					if (HIDP_STATUS_SUCCESS == mie.HidP_GetValueCaps(HidP_Input, value_caps.get(), &caps_length, preparsed_data))
					{
						USAGE usage[32];
						ULONG usage_length = num_buttons_;
						if (HIDP_STATUS_SUCCESS == mie.HidP_GetUsages(HidP_Input, button_caps[0].UsagePage,
							0, usage, &usage_length, preparsed_data,
							reinterpret_cast<CHAR*>(const_cast<BYTE*>(ri.data.hid.bRawData)), ri.data.hid.dwSizeHid))
						{
							buttons_state_.fill(false);
							for (uint32_t i = 0; i < usage_length; ++ i)
							{
								buttons_state_[usage[i] - button_caps[0].Range.UsageMin] = true;
							}

							for (uint32_t i = 0; i < caps.NumberInputValueCaps; ++ i)
							{
								float offset;
								float scale;
								switch (value_caps[i].BitField)
								{
								case 1:
									offset = 128;
									scale = 128;
									break;

								case 2:
									offset = 32768;
									scale = 32768;
									break;

								default:
									offset = 0;
									scale = 1;
									break;
								}

								ULONG value;
								if (HIDP_STATUS_SUCCESS == mie.HidP_GetUsageValue(HidP_Input, value_caps[i].UsagePage,
									0, value_caps[i].Range.UsageMin, &value, preparsed_data,
									reinterpret_cast<CHAR*>(const_cast<BYTE*>(ri.data.hid.bRawData)), ri.data.hid.dwSizeHid))
								{
									switch (value_caps[i].Range.UsageMin)
									{
									case HID_USAGE_GENERIC_X:
										thumbs_state_[0].x() = (value - offset) / scale;
										break;

									case HID_USAGE_GENERIC_Y:
										thumbs_state_[0].y() = (value - offset) / scale;
										break;

									case HID_USAGE_GENERIC_Z:
										thumbs_state_[0].z() = (value - offset) / scale;
										break;

									case HID_USAGE_GENERIC_RX:
										thumbs_state_[1].x() = (value - offset) / scale;
										break;

									case HID_USAGE_GENERIC_RY:
										thumbs_state_[1].y() = (value - offset) / scale;
										break;

									case HID_USAGE_GENERIC_RZ:
										thumbs_state_[1].z() = (value - offset) / scale;
										break;

									case HID_USAGE_GENERIC_SLIDER:
										triggers_state_[0] = (value - offset) / scale;
										break;

									case HID_USAGE_GENERIC_DIAL:
										triggers_state_[1] = (value - offset) / scale;
										break;

									default:
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
#elif defined KLAYGE_PLATFORM_ANDROID
	void MsgInputJoystick::OnJoystickAxis(uint32_t axis, int32_t value)
	{
		// TODO: Is it correct?
		switch (axis)
		{
		case 0:
			thumbs_state_[0].x() = value;
			break;

		case 1:
			thumbs_state_[0].y() = value;
			break;

		case 2:
			thumbs_state_[0].z() = value;
			break;

		case 3:
			thumbs_state_[1].x() = value;
			break;

		case 4:
			thumbs_state_[1].y() = value;
			break;

		case 5:
			thumbs_state_[1].z() = value;
			break;

		case 6:
			triggers_state_[0] = value;
			break;

		case 7:
			triggers_state_[1] = value;
			break;

		default:
			break;
		}
	}

	void MsgInputJoystick::OnJoystickButtons(uint32_t buttons)
	{
		for (size_t i = 0; i < buttons_state_.size(); ++ i)
		{
			buttons_state_[i] = (buttons & (1UL << i)) ? true : false;
		}
	}
#endif

	void MsgInputJoystick::UpdateInputs()
	{
		thumbs_ = thumbs_state_;
		triggers_ = triggers_state_;

		index_ = !index_;
		buttons_[index_] = buttons_state_;
	}
}
#endif
