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

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
namespace KlayGE
{
	MsgInputJoystick::MsgInputJoystick(HANDLE device)
		: device_(device),
			pos_state_(0, 0, 0), rot_state_(0, 0, 0), slider_state_(0, 0)
	{
		buttons_state_.fill(false);
	}
	
	const std::wstring& MsgInputJoystick::Name() const
	{
		static std::wstring const name(L"MsgInput Joystick");
		return name;
	}

	void MsgInputJoystick::OnRawInput(RAWINPUT const & ri)
	{
		if ((RIM_TYPEHID == ri.header.dwType) && (ri.header.hDevice == device_))
		{
			MsgInputEngine const & mie = *checked_cast<MsgInputEngine const *>(&Context::Instance().InputFactoryInstance().InputEngineInstance());

			UINT size;
			if (0 == ::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_PREPARSEDDATA, nullptr, &size))
			{
				std::vector<uint8_t> buf(size);
				if (::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_PREPARSEDDATA, &buf[0], &size) >= 0)
				{
					PHIDP_PREPARSED_DATA preparsed_data = reinterpret_cast<PHIDP_PREPARSED_DATA>(&buf[0]);

					HIDP_CAPS caps;
					if (HIDP_STATUS_SUCCESS == mie.HidP_GetCaps(preparsed_data, &caps))
					{
						std::vector<HIDP_BUTTON_CAPS> button_caps(caps.NumberInputButtonCaps);

						uint16_t caps_length = caps.NumberInputButtonCaps;
						if (HIDP_STATUS_SUCCESS == mie.HidP_GetButtonCaps(HidP_Input, &button_caps[0], &caps_length, preparsed_data))
						{
							int num_buttons = std::min<int>(32, button_caps[0].Range.UsageMax - button_caps[0].Range.UsageMin + 1);

							std::vector<HIDP_VALUE_CAPS> value_caps(caps.NumberInputValueCaps);
							caps_length = caps.NumberInputValueCaps;
							if (HIDP_STATUS_SUCCESS == mie.HidP_GetValueCaps(HidP_Input, &value_caps[0], &caps_length, preparsed_data))
							{
								USAGE usage[32];
								ULONG usage_length = num_buttons;
								if (HIDP_STATUS_SUCCESS == mie.HidP_GetUsages(HidP_Input, button_caps[0].UsagePage,
									0, usage, &usage_length, preparsed_data,
									reinterpret_cast<CHAR*>(const_cast<BYTE*>(ri.data.hid.bRawData)), ri.data.hid.dwSizeHid))
								{
									buttons_[!index_].fill(false);
									for (uint32_t i = 0; i < usage_length; ++ i)
									{
										buttons_[!index_][usage[i] - button_caps[0].Range.UsageMin] = true;
									}

									for (uint32_t i = 0; i < caps.NumberInputValueCaps; ++ i)
									{
										ULONG value;
										if (HIDP_STATUS_SUCCESS == mie.HidP_GetUsageValue(HidP_Input, value_caps[i].UsagePage,
											0, value_caps[i].Range.UsageMin, &value, preparsed_data,
											reinterpret_cast<CHAR*>(const_cast<BYTE*>(ri.data.hid.bRawData)), ri.data.hid.dwSizeHid))
										{
											switch (value_caps[i].Range.UsageMin)
											{
											case HID_USAGE_GENERIC_X:
												pos_state_.x() = static_cast<long>(value) - 128;
												break;

											case HID_USAGE_GENERIC_Y:
												pos_state_.y() = static_cast<long>(value) - 128;
												break;

											case HID_USAGE_GENERIC_Z:
												pos_state_.z() = static_cast<long>(value) - 128;
												break;

											case HID_USAGE_GENERIC_RX:
												rot_state_.x() = static_cast<long>(value) - 128;
												break;

											case HID_USAGE_GENERIC_RY:
												rot_state_.y() = static_cast<long>(value) - 128;
												break;

											case HID_USAGE_GENERIC_RZ:
												rot_state_.z() = static_cast<long>(value) - 128;
												break;

											case HID_USAGE_GENERIC_SLIDER:
												slider_state_.x() = static_cast<long>(value) - 128;
												break;

											case HID_USAGE_GENERIC_DIAL:
												slider_state_.y() = static_cast<long>(value) - 128;
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
		}
	}

	void MsgInputJoystick::UpdateInputs()
	{
		pos_ = pos_state_;
		rot_ = rot_state_;
		slider_ = slider_state_;

		index_ = !index_;
		buttons_[index_] = buttons_state_;

		buttons_state_.fill(false);
	}
}
#endif
