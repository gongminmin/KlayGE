/**
 * @file Touch.cpp
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

#include <boost/assert.hpp>

#include <KlayGE/Input.hpp>

namespace
{	
	float const TAP_TIMER_THRESHOLD = 0.2f;
	float const PRESS_TIMER_THRESHOLD = 2;
	float const ROTATE_THRESHOLD = 0.995f;
	float const ZOOM_THRESHOLD = 0.999f;
	float const ZOOM_DISTANCE_THRESHOLD = 0.02f;
}

namespace KlayGE
{
	InputTouch::InputTouch()
		: wheel_delta_(0), index_(false), num_available_touch_(0),
			gesture_(TS_None), action_param_(MakeSharedPtr<InputTouchActionParam>()),
			curr_state_(GS_None),
			one_finger_tap_timer_(0), two_finger_tap_timer_(0)
	{
		action_param_->type = InputEngine::IDT_Touch;

		touch_downs_[0].fill(false);
		touch_downs_[1].fill(false);

		gesture_funcs_[GS_None] = [this](float elapsed_time) { this->GestureNone(elapsed_time); };
		gesture_funcs_[GS_Pan] = [this](float elapsed_time) { this->GesturePan(elapsed_time); };
		gesture_funcs_[GS_Tap] = [this](float elapsed_time) { this->GestureTap(elapsed_time); };
		gesture_funcs_[GS_PressAndTap] = [this](float elapsed_time) { this->GesturePressAndTap(elapsed_time); };
		gesture_funcs_[GS_TwoFingerIntermediate] = [this](float elapsed_time) { this->GestureTwoFingerIntermediate(elapsed_time); };
		gesture_funcs_[GS_Zoom] = [this](float elapsed_time) { this->GestureZoom(elapsed_time); };
		gesture_funcs_[GS_Rotate] = [this](float elapsed_time) { this->GestureRotate(elapsed_time); };

		curr_gesture_ = gesture_funcs_[GS_None];
	}

	InputTouch::~InputTouch()
	{
	}

	TouchSemantic InputTouch::Gesture() const
	{
		return gesture_;
	}

	void InputTouch::ActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = TS_None; i <= TS_AnyTouch; ++ i)
		{
			if (actionMap.HasAction(i))
			{
				iam.AddAction(InputActionDefine(actionMap.Action(i), i));
			}
		}
	}

	InputActionsType InputTouch::UpdateActionMap(uint32_t id)
	{
		InputActionsType ret;

		InputActionMap& iam = actionMaps_[id];

		action_param_->gesture = gesture_;
		action_param_->wheel_delta = wheel_delta_;
		action_param_->touches_coord = touch_coords_[index_];
		action_param_->touches_state = 0;
		action_param_->touches_down = 0;
		action_param_->touches_up = 0;
		for (size_t i = 0; i < 16; ++ i)
		{
			action_param_->touches_state |= (touch_downs_[index_][i] ? (1UL << i) : 0);
			action_param_->touches_down |= ((touch_downs_[index_][i] && !touch_downs_[!index_][i]) ? (1UL << i) : 0);
			action_param_->touches_up |= ((!touch_downs_[index_][i] && touch_downs_[!index_][i]) ? (1UL << i) : 0);
		}

		if (wheel_delta_ != 0)
		{
			action_param_->center = touch_coords_[index_][0];
			action_param_->move_vec = int2(0, 0);
			action_param_->zoom = 1;
			action_param_->rotate_angle = 0;
			iam.UpdateInputActions(ret, TS_Wheel, action_param_);
		}
		if (gesture_ != TS_None)
		{
			iam.UpdateInputActions(ret, static_cast<uint16_t>(gesture_), action_param_);
		}
		bool any_touch = false;
		for (uint16_t i = 0; i < touch_coords_[index_].size(); ++ i)
		{
			if (touch_downs_[index_][i] || touch_downs_[!index_][i])
			{
				iam.UpdateInputActions(ret, static_cast<uint16_t>(TS_Touch0 + i), action_param_);
				any_touch = true;
			}
		}
		if (any_touch)
		{
			iam.UpdateInputActions(ret, TS_AnyTouch, action_param_);
		}

		return ret;
	}

	void InputTouch::CurrState(GestureState state)
	{
		BOOST_ASSERT(state >= GS_None && state < GS_NumGestures);

		curr_state_ = state;
		curr_gesture_ = gesture_funcs_[state];
	}

	void InputTouch::GestureNone(float /*elapsed_time*/)
	{
		switch (num_available_touch_)
		{
		case 1:
			if (touch_downs_[index_][0])
			{
				this->CurrState(GS_Tap);
				one_finger_tap_timer_ = 0;
				one_finger_start_pos_ = touch_coords_[index_][0];
			}
			break;

		case 2:
			if (touch_downs_[index_][0] && touch_downs_[index_][1])
			{
				this->CurrState(GS_TwoFingerIntermediate);
				two_finger_tap_timer_ = 0;
				two_finger_vec_ = float2(touch_coords_[index_][0] - touch_coords_[index_][1]);
				two_finger_start_len_ = MathLib::length(two_finger_vec_);
				two_finger_vec_ /= two_finger_start_len_;
			}
			break;

		default:
			break;
		}
	}

	void InputTouch::GesturePan(float /*elapsed_time*/)
	{
		switch (num_available_touch_)
		{
		case 0:
			this->CurrState(GS_None);
			break;
		}

		gesture_ = TS_Pan;
		action_param_->center = touch_coords_[index_][0];
		if (touch_downs_[index_][0] && touch_downs_[!index_][0])
		{
			action_param_->move_vec = touch_coords_[index_][0] - touch_coords_[!index_][0];
		}
		else
		{
			action_param_->move_vec = int2(0, 0);
		}
	}

	void InputTouch::GestureTap(float elapsed_time)
	{
		switch (num_available_touch_)
		{
		case 0:
			{
				float2 vec = float2(touch_coords_[index_][0] - one_finger_start_pos_);
				float vec_len = MathLib::length(vec);
				float distance = vec_len / one_finger_tap_timer_;
				if (distance > 1000.0f)
				{
					vec /= vec_len;
					gesture_ = TS_Flick;
				}
				else
				{
					gesture_ = TS_Tap;
				}
				action_param_->center = touch_coords_[index_][0];
				action_param_->move_vec = int2(vec);
				this->CurrState(GS_None);
			}
			break;

		case 2:
			if (touch_downs_[index_][0] && touch_downs_[index_][1])
			{
				if (one_finger_tap_timer_ < 0.5f)
				{
					this->CurrState(GS_TwoFingerIntermediate);
				}
				else
				{
					this->CurrState(GS_PressAndTap);
				}
				two_finger_tap_timer_ = 0;
				two_finger_vec_ = float2(touch_coords_[index_][0] - touch_coords_[index_][1]);
				two_finger_start_len_ = MathLib::length(two_finger_vec_);
				two_finger_vec_ /= two_finger_start_len_;
			}
			break;
		}

		one_finger_tap_timer_ += elapsed_time;
		float dist = MathLib::length(float2(touch_coords_[index_][0] - one_finger_start_pos_));
		if ((1 == num_available_touch_) && (one_finger_tap_timer_ > PRESS_TIMER_THRESHOLD) && (dist < 2))
		{
			gesture_ = TS_Press;
			action_param_->center = touch_coords_[index_][0];
		}
		else if ((one_finger_tap_timer_ > TAP_TIMER_THRESHOLD) && (dist > 2))
		{
			this->CurrState(GS_Pan);
		}
	}

	void InputTouch::GesturePressAndTap(float elapsed_time)
	{
		switch (num_available_touch_)
		{
		case 0:
		case 1:
			if (two_finger_tap_timer_ < TAP_TIMER_THRESHOLD)
			{
				gesture_ = TS_PressAndTap;
				action_param_->center = float2(touch_coords_[index_][0] + touch_coords_[index_][1]) / 2.0f;
			}
			this->CurrState(GS_None);
			break;

		case 2:
			{
				float2 vec = float2(touch_coords_[index_][0] - touch_coords_[index_][1]);
				float vec_len = MathLib::length(vec);
				vec /= vec_len;

				float d = MathLib::abs(MathLib::dot(two_finger_vec_, vec));
				if (d > ZOOM_THRESHOLD)
				{
					if (MathLib::abs(1 - MathLib::abs(vec_len / two_finger_start_len_)) > ZOOM_DISTANCE_THRESHOLD)
					{
						this->CurrState(GS_Zoom);
					}
				}
				if (d < ROTATE_THRESHOLD)
				{
					this->CurrState(GS_Rotate);
				}
			}
			break;
		}
		two_finger_tap_timer_ += elapsed_time;
	}

	void InputTouch::GestureTwoFingerIntermediate(float elapsed_time)
	{
		switch (num_available_touch_)
		{
		case 0:
			if (two_finger_tap_timer_ < TAP_TIMER_THRESHOLD)
			{
				gesture_ = TS_Tap;
				action_param_->center = float2(touch_coords_[index_][0] + touch_coords_[index_][1]) / 2.0f;
			}
			this->CurrState(GS_None);
			break;

		case 2:
			{
				float2 vec = float2(touch_coords_[index_][0] - touch_coords_[index_][1]);
				float vec_len = MathLib::length(vec);
				vec /= vec_len;

				float d = MathLib::abs(MathLib::dot(two_finger_vec_, vec));
				if (d > ZOOM_THRESHOLD)
				{
					if (MathLib::abs(1 - MathLib::abs(vec_len / two_finger_start_len_)) > ZOOM_DISTANCE_THRESHOLD)
					{
						this->CurrState(GS_Zoom);
					}
				}
				if (d < ROTATE_THRESHOLD)
				{
					this->CurrState(GS_Rotate);
				}
			}
			break;
		}
		two_finger_tap_timer_ += elapsed_time;
	}

	void InputTouch::GestureZoom(float elapsed_time)
	{
		switch (num_available_touch_)
		{
		case 0:
		case 1:
			this->CurrState(GS_None);
			break;

		case 2:
			{
				float vec_len = MathLib::length(float2(touch_coords_[index_][0] - touch_coords_[index_][1]));
				gesture_ = TS_Zoom;
				action_param_->center = float2(touch_coords_[index_][0] + touch_coords_[index_][1]) / 2.0f;
				action_param_->zoom = vec_len / two_finger_start_len_;
				two_finger_start_len_ = vec_len;
			}
			break;
		}
		two_finger_tap_timer_ += elapsed_time;
	}

	void InputTouch::GestureRotate(float elapsed_time)
	{
		switch (num_available_touch_)
		{
		case 0:
		case 1:
			this->CurrState(GS_None);
			break;

		case 2:
			{
				float2 vec = MathLib::normalize(float2(touch_coords_[index_][0] - touch_coords_[index_][1]));
				float delta_angle = atan2(two_finger_vec_.y(), two_finger_vec_.x()) - atan2(vec.y(), vec.x());
				if (MathLib::abs(delta_angle) > 0.01f)
				{
					two_finger_vec_ = vec;
					gesture_ = TS_Rotate;
					action_param_->center = float2(touch_coords_[index_][0] + touch_coords_[index_][1]) / 2.0f;
					action_param_->rotate_angle = delta_angle;
				}
			}
			break;
		}
		two_finger_tap_timer_ += elapsed_time;
	}
}
