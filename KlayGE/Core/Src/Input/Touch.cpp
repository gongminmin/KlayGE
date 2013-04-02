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
	float const ROTATE_THRESHOLD = 0.995f;
	float const ZOOM_THRESHOLD = 0.999f;
	float const ZOOM_DISTANCE_THRESHOLD = 0.02f;
}

namespace KlayGE
{
	InputTouch::InputTouch()
		: num_available_input_(0), has_gesture_(false),
			curr_state_(GS_None),
			one_finger_tap_timer_(0), two_finger_tap_timer_(0)
	{
		gesture_funcs_[GS_None] = bind(&InputTouch::GestureNone, this, placeholders::_1);
		gesture_funcs_[GS_Pan] = bind(&InputTouch::GesturePan, this, placeholders::_1);
		gesture_funcs_[GS_Tap] = bind(&InputTouch::GestureTap, this, placeholders::_1);
		gesture_funcs_[GS_PressAndTap] = bind(&InputTouch::GesturePressAndTap, this, placeholders::_1);
		gesture_funcs_[GS_TwoFingerIntermediate] = bind(&InputTouch::GestureTwoFingerIntermediate, this, placeholders::_1);
		gesture_funcs_[GS_Zoom] = bind(&InputTouch::GestureZoom, this, placeholders::_1);
		gesture_funcs_[GS_Rotate] = bind(&InputTouch::GestureRotate, this, placeholders::_1);

		curr_gesture_ = gesture_funcs_[GS_None];
	}

	InputTouch::~InputTouch()
	{
	}

	bool InputTouch::HasGesture() const
	{
		return has_gesture_;
	}

	TouchSemantic InputTouch::Gesture() const
	{
		BOOST_ASSERT(has_gesture_);

		return gesture_;
	}

	void InputTouch::ActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = TS_Pan; i < TS_Flick + 1; ++ i)
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

		if (has_gesture_)
		{
			InputActionMap& iam = actionMaps_[id];
			iam.UpdateInputActions(ret, static_cast<uint16_t>(gesture_), gesture_param_);
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
		switch (num_available_input_)
		{
		case 1:
			if (inputs_[0].flags & (TIF_Move | TIF_Down))
			{
				this->CurrState(GS_Tap);
				one_finger_tap_timer_ = 0;
				one_finger_start_pos_ = float2(inputs_[0].coord);
			}
			break;

		case 2:
			if ((inputs_[0].flags & (TIF_Move | TIF_Down)) && (inputs_[1].flags & (TIF_Move | TIF_Down)))
			{
				this->CurrState(GS_TwoFingerIntermediate);
				two_finger_tap_timer_ = 0;
				two_finger_vec_ = float2(inputs_[0].coord - inputs_[1].coord);
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
		switch (num_available_input_)
		{
		case 0:
			this->CurrState(GS_None);
			break;
		}

		gesture_ = TS_Pan;
		has_gesture_ = true;
		gesture_param_.center = inputs_[0].coord;
	}

	void InputTouch::GestureTap(float elapsed_time)
	{
		switch (num_available_input_)
		{
		case 0:
			{
				float2 vec = float2(inputs_[0].coord) - one_finger_start_pos_;
				float vec_len = MathLib::length(vec);
				float distance = vec_len / one_finger_tap_timer_;
				if (distance > 1000.0f)
				{
					vec /= vec_len;
					gesture_ = TS_Flick;
					has_gesture_ = true;
					gesture_param_.move_vec = vec;
				}
				else
				{
					gesture_ = TS_Tap;
					has_gesture_ = true;
				}
				gesture_param_.center = inputs_[0].coord;
				this->CurrState(GS_None);
			}
			break;

		case 2:
			if ((inputs_[0].flags & (TIF_Move | TIF_Down)) && (inputs_[1].flags & (TIF_Move | TIF_Down)))
			{
				if(one_finger_tap_timer_ < 0.5f)
				{
					this->CurrState(GS_TwoFingerIntermediate);
				}
				else
				{
					this->CurrState(GS_PressAndTap);
				}
				two_finger_tap_timer_ = 0.0f;
				two_finger_vec_ = float2(inputs_[0].coord - inputs_[1].coord);
				two_finger_start_len_ = MathLib::length(two_finger_vec_);
				two_finger_vec_ /= two_finger_start_len_;
			}
			break;
		}

		one_finger_tap_timer_ += elapsed_time;
		if(one_finger_tap_timer_ > TAP_TIMER_THRESHOLD)
		{
			float2 vec = float2(inputs_[0].coord) - one_finger_start_pos_;
			if (MathLib::length(vec) > 2)
			{
				this->CurrState(GS_Pan);
			}
		}
	}

	void InputTouch::GesturePressAndTap(float elapsed_time)
	{
		switch (num_available_input_)
		{
		case 0:
		case 1:
			if (two_finger_tap_timer_ < TAP_TIMER_THRESHOLD)
			{
				gesture_ = TS_PressAndTap;
				has_gesture_ = true;
				gesture_param_.center = float2(inputs_[0].coord + inputs_[1].coord) / 2.0f;
			}
			this->CurrState(GS_None);
			break;

		case 2:
			{
				float2 vec = float2(inputs_[0].coord - inputs_[1].coord);
				float vec_len = MathLib::length(vec);
				vec /= vec_len;

				float d = MathLib::dot(two_finger_vec_, vec);
				if (abs(d) > ZOOM_THRESHOLD)
				{
					if (abs(1 - abs(vec_len / two_finger_start_len_)) > ZOOM_DISTANCE_THRESHOLD)
					{
						this->CurrState(GS_Zoom);
					}
				}
				if (abs(d) < ROTATE_THRESHOLD)
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
		switch (num_available_input_)
		{
		case 0:
			if (two_finger_tap_timer_ < TAP_TIMER_THRESHOLD)
			{
				gesture_ = TS_Tap;
				has_gesture_ = true;
				gesture_param_.center = float2(inputs_[0].coord + inputs_[1].coord) / 2.0f;
			}
			this->CurrState(GS_None);
			break;

		case 2:
			{
				float2 vec = float2(inputs_[0].coord - inputs_[1].coord);
				float vec_len = MathLib::length(vec);
				vec /= vec_len;

				float d = MathLib::dot(two_finger_vec_, vec);
				if (abs(d) > ZOOM_THRESHOLD)
				{
					if (abs(1 - abs(vec_len / two_finger_start_len_)) > ZOOM_DISTANCE_THRESHOLD)
					{
						this->CurrState(GS_Zoom);
					}
				}
				if (abs(d) < ROTATE_THRESHOLD)
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
		switch (num_available_input_)
		{
		case 0:
		case 1:
			this->CurrState(GS_None);
			break;

		case 2:
			{
				float vec_len = MathLib::length(float2(inputs_[0].coord - inputs_[1].coord));
				gesture_ = TS_Zoom;
				has_gesture_ = true;
				gesture_param_.center = float2(inputs_[0].coord + inputs_[1].coord) / 2.0f;
				gesture_param_.move_vec.x() = vec_len / two_finger_start_len_;
				two_finger_start_len_ = vec_len;
			}
			break;
		}
		two_finger_tap_timer_ += elapsed_time;
	}

	void InputTouch::GestureRotate(float elapsed_time)
	{
		switch (num_available_input_)
		{
		case 0:
		case 1:
			this->CurrState(GS_None);
			break;

		case 2:
			{
				float2 vec = MathLib::normalize(float2(inputs_[0].coord - inputs_[1].coord));
				float delta_angle = atan2(two_finger_vec_.y(), two_finger_vec_.x()) - atan2(vec.y(), vec.x());
				if (abs(delta_angle) > 0.01f)
				{
					two_finger_vec_ = vec;
					gesture_ = TS_Rotate;
					has_gesture_ = true;
					gesture_param_.center = float2(inputs_[0].coord + inputs_[1].coord) / 2.0f;
					gesture_param_.rotate_angle = delta_angle;
				}
			}
			break;
		}
		two_finger_tap_timer_ += elapsed_time;
	}
}
