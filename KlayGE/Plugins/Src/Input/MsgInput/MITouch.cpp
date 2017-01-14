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
#include <KlayGE/Window.hpp>

#include <KlayGE/MsgInput/MInput.hpp>

namespace KlayGE
{
	MsgInputTouch::MsgInputTouch()
		: wheel_delta_state_(0)
	{
		touch_down_state_.fill(false);
	}
	
	const std::wstring& MsgInputTouch::Name() const
	{
		static std::wstring const name(L"MsgInput Touch");
		return name;
	}

	void MsgInputTouch::OnPointerDown(int2 const & pt, uint32_t id)
	{
		if (id > 0)
		{
			-- id;
			touch_coord_state_[id] = this->AdjustPoint(pt);
			touch_down_state_[id] = true;
		}
	}

	void MsgInputTouch::OnPointerUp(int2 const & pt, uint32_t id)
	{
		if (id > 0)
		{
			-- id;
			touch_coord_state_[id] = this->AdjustPoint(pt);
			touch_down_state_[id] = false;
		}
	}

	void MsgInputTouch::OnPointerUpdate(int2 const & pt, uint32_t id, bool down)
	{
		if (id > 0)
		{
			-- id;
			touch_coord_state_[id] = this->AdjustPoint(pt);
			touch_down_state_[id] = down;
		}
	}

	void MsgInputTouch::OnPointerWheel(int2 const & pt, uint32_t id, int32_t wheel_delta)
	{
		if (id > 0)
		{
			-- id;
			touch_coord_state_[id] = this->AdjustPoint(pt);
		}
		wheel_delta_state_ += wheel_delta;
	}

	void MsgInputTouch::UpdateInputs()
	{
		index_ = !index_;
		touch_coords_[index_] = touch_coord_state_;
		touch_downs_[index_] = touch_down_state_;
		wheel_delta_ = wheel_delta_state_;
		num_available_touch_ = 0;
		for (auto const & tds : touch_down_state_)
		{
			num_available_touch_ += tds;
		}

		wheel_delta_state_ = 0;
		gesture_ = TS_None;
		action_param_->move_vec = float2(0.0f, 0.0f);
		curr_gesture_(static_cast<float>(timer_.elapsed()));
		timer_.restart();
	}

	int2 MsgInputTouch::AdjustPoint(int2 const & pt) const
	{
		return pt;
	}
}
