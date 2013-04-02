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
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/MsgInput/MInput.hpp>

namespace KlayGE
{
	MsgInputMouse::MsgInputMouse(HANDLE device)
		: device_(device)
	{
		buttons_state_.fill(false);
	}

	std::wstring const & MsgInputMouse::Name() const
	{
		static std::wstring const name(L"MsgInput Mouse");
		return name;
	}

	void MsgInputMouse::OnRawInput(RAWINPUT const & ri)
	{
		if ((RIM_TYPEMOUSE == ri.header.dwType) && (ri.header.hDevice == device_))
		{
			for (int i = 0; i < 5; ++ i)
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

			if (MOUSE_MOVE_RELATIVE == ri.data.mouse.usFlags)
			{
				offset_state_.x() += ri.data.mouse.lLastX;
				offset_state_.y() += ri.data.mouse.lLastY;
			}
			if (ri.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
			{
				offset_state_.z() += static_cast<short>(ri.data.mouse.usButtonData);
			}
		}
	}

	void MsgInputMouse::UpdateInputs()
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(Context::Instance().AppInstance().MainWnd()->HWnd(), &pt);
		abs_pos_ = Vector_T<long, 2>(pt.x, pt.y);
		offset_ = offset_state_;

		index_ = !index_;
		buttons_[index_] = buttons_state_;

		offset_state_ = Vector_T<long, 3>(0, 0, 0);
	}
}
