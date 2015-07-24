/**
 * @file MIMouse.cpp
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

#include <KlayGE/MsgInput/MInput.hpp>

#if (defined KLAYGE_PLATFORM_WINDOWS_DESKTOP) || (defined KLAYGE_PLATFORM_ANDROID)
namespace KlayGE
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	MsgInputMouse::MsgInputMouse(HWND hwnd, HANDLE device)
		: hwnd_(hwnd), device_id_(0xFFFFFFFF),
#elif defined KLAYGE_PLATFORM_ANDROID
	MsgInputMouse::MsgInputMouse()
		: last_abs_state_(-1, -1), abs_state_(0, 0),
#endif
			offset_state_(0, 0, 0)
	{
		buttons_state_.fill(false);

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		UINT size = 0;
		if (0 == ::GetRawInputDeviceInfo(device, RIDI_DEVICEINFO, nullptr, &size))
		{
			std::vector<uint8_t> buf(size);
			::GetRawInputDeviceInfo(device, RIDI_DEVICEINFO, &buf[0], &size);

			RID_DEVICE_INFO* info = reinterpret_cast<RID_DEVICE_INFO*>(&buf[0]);
			device_id_ = info->mouse.dwId;
			num_buttons_ = std::min(static_cast<uint32_t>(buttons_[0].size()),
				static_cast<uint32_t>(info->mouse.dwNumberOfButtons));
		}
#elif defined KLAYGE_PLATFORM_ANDROID
		num_buttons_ = 5;
#endif
	}

	std::wstring const & MsgInputMouse::Name() const
	{
		static std::wstring const name(L"MsgInput Mouse");
		return name;
	}

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	void MsgInputMouse::OnRawInput(RAWINPUT const & ri)
	{
		BOOST_ASSERT(RIM_TYPEMOUSE == ri.header.dwType);

		UINT size = 0;
		if (0 == ::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_DEVICEINFO, nullptr, &size))
		{
			std::vector<uint8_t> buf(size);
			::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_DEVICEINFO, &buf[0], &size);

			RID_DEVICE_INFO* info = reinterpret_cast<RID_DEVICE_INFO*>(&buf[0]);
			if (device_id_ != info->mouse.dwId)
			{
				return;
			}
		}

		for (uint32_t i = 0; i < num_buttons_; ++ i)
		{
			if (ri.data.mouse.usButtonFlags & (1UL << (i * 2 + 0)))
			{
				buttons_state_[i] = true;
			}
			if (ri.data.mouse.usButtonFlags & (1UL << (i * 2 + 1)))
			{
				buttons_state_[i] = false;
			}
		}

		if (MOUSE_MOVE_RELATIVE == (ri.data.mouse.usFlags & 1UL))
		{
			offset_state_.x() += ri.data.mouse.lLastX;
			offset_state_.y() += ri.data.mouse.lLastY;
		}
		if (ri.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
		{
			offset_state_.z() += static_cast<short>(ri.data.mouse.usButtonData);
		}
	}
#elif defined KLAYGE_PLATFORM_ANDROID
	void MsgInputMouse::OnMouseDown(int2 const & pt, uint32_t buttons)
	{
		for (uint32_t i = 0; i < num_buttons_; ++ i)
		{
			if (buttons & (1UL << i))
			{
				buttons_state_[i] = true;
			}
		}

		this->OnMouseMove(pt);
	}

	void MsgInputMouse::OnMouseUp(int2 const & pt, uint32_t buttons)
	{
		for (uint32_t i = 0; i < num_buttons_; ++ i)
		{
			if (buttons & (1UL << i))
			{
				buttons_state_[i] = false;
			}
		}

		this->OnMouseMove(pt);
	}

	void MsgInputMouse::OnMouseMove(int2 const & pt)
	{
		abs_state_ = pt;
		if ((last_abs_state_.x() < 0) && (last_abs_state_.y() < 0))
		{
			last_abs_state_ = abs_state_;
		}

		offset_state_.x() = abs_state_.x() - last_abs_state_.x();
		offset_state_.y() = abs_state_.y() - last_abs_state_.y();
		last_abs_state_ = abs_state_;
	}

	void MsgInputMouse::OnMouseWheel(int2 const & pt, int32_t wheel_delta)
	{
		offset_state_.z() = wheel_delta;

		this->OnMouseMove(pt);
	}
#endif

	void MsgInputMouse::UpdateInputs()
	{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		POINT pt;
		::GetCursorPos(&pt);
		::ScreenToClient(hwnd_, &pt);
		abs_pos_ = int2(pt.x, pt.y);
#elif defined KLAYGE_PLATFORM_ANDROID
		abs_pos_ = abs_state_;
#endif
		offset_ = offset_state_;

		index_ = !index_;
		buttons_[index_] = buttons_state_;

#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
		shift_ctrl_alt_ = ((::GetKeyState(VK_SHIFT) & 0x80) ? MB_Shift : 0)
			| ((::GetKeyState(VK_CONTROL) & 0x80) ? MB_Ctrl : 0)
			| ((::GetKeyState(VK_MENU) & 0x80) ? MB_Alt : 0);
#elif defined KLAYGE_PLATFORM_ANDROID
		// TODO
#endif

		offset_state_ = int3(0, 0, 0);
	}
}
#endif
