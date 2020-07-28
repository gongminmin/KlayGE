/**
 * @file MIXInput.cpp
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

#if defined(KLAYGE_PLATFORM_WINDOWS)
#include <windows.h>
#include <xinput.h>

#include <KlayGE/Context.hpp>
#include <KlayGE/InputFactory.hpp>

#include <KlayGE/MsgInput/MInput.hpp>

namespace KlayGE
{
	MsgInputXInput::MsgInputXInput(uint32_t device_id) : device_id_(device_id)
	{
		num_buttons_ = 12;
		num_vibration_motors_ = 2;
	}

	std::wstring const& MsgInputXInput::Name() const
	{
		static std::wstring const name(L"MsgInput XInput");
		return name;
	}

	void MsgInputXInput::UpdateInputs()
	{
		index_ = !index_;

		thumbs_.fill(float3(0, 0, 0));
		triggers_.fill(0);
		buttons_[index_].fill(false);

		XINPUT_STATE state{};
		DWORD result = XInputGetState(device_id_, &state);
		if (result == ERROR_SUCCESS)
		{
			buttons_[index_][0] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? true : false;
			buttons_[index_][1] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? true : false;
			buttons_[index_][2] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? true : false;
			buttons_[index_][3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? true : false;
			buttons_[index_][4] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? true : false;
			buttons_[index_][5] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? true : false;
			buttons_[index_][6] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? true : false;
			buttons_[index_][7] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? true : false;
			buttons_[index_][8] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? true : false;
			buttons_[index_][9] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? true : false;
			buttons_[index_][10] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? true : false;
			buttons_[index_][11] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? true : false;

			if ((state.Gamepad.sThumbLX > +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ||
				(state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
			{
				thumbs_[0].x() = state.Gamepad.sThumbLX / 32768.0f;
			}
			if ((state.Gamepad.sThumbLY > +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ||
				(state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
			{
				thumbs_[0].y() = state.Gamepad.sThumbLY / 32768.0f;
			}
			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
			{
				thumbs_[0].z() = 1.0f;
			}
			if ((state.Gamepad.sThumbRX > +XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) ||
				(state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
			{
				thumbs_[1].x() = state.Gamepad.sThumbRX / 32768.0f;
			}
			if ((state.Gamepad.sThumbRY > +XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) ||
				(state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
			{
				thumbs_[1].y() = state.Gamepad.sThumbRY / 32768.0f;
			}
			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
			{
				thumbs_[1].z() = 1.0f;
			}

			if (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			{
				triggers_[0] = state.Gamepad.bLeftTrigger / 255.0f;
			}
			if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			{
				triggers_[1] = state.Gamepad.bRightTrigger / 255.0f;
			}
		}
	}

	void MsgInputXInput::VibrationMotorSpeed(uint32_t n, float motor_speed)
	{
		BOOST_ASSERT(n < motor_speeds_.size());

		motor_speeds_[n] = static_cast<uint16_t>(MathLib::clamp(static_cast<uint32_t>(motor_speed * 65535), 0U, 65535U));

		XINPUT_VIBRATION vibration{motor_speeds_[0], motor_speeds_[1]};
		XInputSetState(device_id_, &vibration);
	}
} // namespace KlayGE
#endif // defined(KLAYGE_PLATFORM_WINDOWS)
