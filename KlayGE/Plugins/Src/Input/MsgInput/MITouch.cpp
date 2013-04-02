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

namespace KlayGE
{
	MsgInputTouch::MsgInputTouch()
	{
	}
	
	const std::wstring& MsgInputTouch::Name() const
	{
		static std::wstring const name(L"MsgInput Touch");
		return name;
	}

	void MsgInputTouch::OnTouch(std::vector<TOUCHINPUT> const & inputs)
	{
		num_available_input_ = static_cast<uint32_t>(inputs.size());
		input_state_.resize(num_available_input_);

		bool all_up = true;
		for (uint32_t i = 0; i < num_available_input_; ++ i)
		{
			input_state_[i].coord = int2(inputs[i].x, inputs[i].y);
			input_state_[i].flags = 0;
			if (inputs[i].dwFlags & TOUCHEVENTF_MOVE)
			{
				input_state_[i].flags |= TIF_Move;
			}
			if (inputs[i].dwFlags & TOUCHEVENTF_DOWN)
			{
				input_state_[i].flags |= TIF_Down;
			}
			if (inputs[i].dwFlags & TOUCHEVENTF_UP)
			{
				input_state_[i].flags |= TIF_Up;
			}
			input_state_[i].finger_id = inputs[i].dwID;

			if (!(inputs[i].dwFlags & TOUCHEVENTF_UP))
			{
				all_up = false;
			}
		}
		if (all_up)
		{
			num_available_input_ = 0;
		}

		curr_gesture_(static_cast<float>(timer_.elapsed()));
		timer_.restart();
	}

	void MsgInputTouch::UpdateInputs()
	{
		inputs_ = input_state_;

		input_state_.clear();
		has_gesture_ = false;
	}
}
