// Joystick.cpp
// KlayGE 游戏杆管理类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// 增加了Action map id (2005.4.3)
//
// 2.0.0
// 初次建立 (2003.8.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	InputJoystick::InputJoystick() : action_param_(MakeSharedPtr<InputJoystickActionParam>())
	{
		action_param_->type = InputEngine::IDT_Joystick;
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	InputJoystick::~InputJoystick()
	{
	}

	float3 const& InputJoystick::LeftThumb() const
	{
		return thumbs_[0];
	}

	float3 const& InputJoystick::RightThumb() const
	{
		return thumbs_[1];
	}

	float InputJoystick::LeftTrigger() const
	{
		return triggers_[0];
	}
	
	float InputJoystick::RightTrigger() const
	{
		return triggers_[1];
	}

	// 获取键的数量
	//////////////////////////////////////////////////////////////////////////////////
	uint32_t InputJoystick::NumButtons() const
	{
		return num_buttons_;
	}
	
	// 获取指定键是否按下
	/////////////////////////////////////////////////////////////////////////////////
	bool InputJoystick::Button(uint32_t n) const
	{
		BOOST_ASSERT(n < buttons_[index_].size());

		return buttons_[index_][n];
	}

	bool InputJoystick::ButtonDown(uint32_t n) const
	{
		BOOST_ASSERT(n < buttons_[index_].size());

		return (buttons_[index_][n] && !buttons_[!index_][n]);
	}

	bool InputJoystick::ButtonUp(uint32_t n) const
	{
		BOOST_ASSERT(n < buttons_[index_].size());

		return (!buttons_[index_][n] && buttons_[!index_][n]);
	}

	uint32_t InputJoystick::NumVibrationMotors() const
	{
		return num_vibration_motors_;
	}

	void InputJoystick::VibrationMotorSpeed(uint32_t n, float motor_speed)
	{
		KFL_UNUSED(n);
		KFL_UNUSED(motor_speed);
	}

	// 实现动作映射
	//////////////////////////////////////////////////////////////////////////////////
	void InputJoystick::ActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = JS_LeftThumbX; i <= JS_AnyButton; ++i)
		{
			if (actionMap.HasAction(i))
			{
				iam.AddAction(InputActionDefine(actionMap.Action(i), i));
			}
		}
	}

	// 更新游戏杆动作
	/////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputJoystick::UpdateActionMap(uint32_t id)
	{
		InputActionsType ret;

		InputActionMap& iam = actionMaps_[id];

		action_param_->thumbs[0] = thumbs_[0];
		action_param_->thumbs[1] = thumbs_[1];
		action_param_->triggers[0] = triggers_[0];
		action_param_->triggers[1] = triggers_[1];
		action_param_->buttons_state = 0;
		action_param_->buttons_down = 0;
		action_param_->buttons_up = 0;
		for (uint32_t i = 0; i < this->NumButtons(); ++ i)
		{
			action_param_->buttons_state |= (this->Button(i)? (1UL << i) : 0);
			action_param_->buttons_down |= (this->ButtonDown(i)? (1UL << i) : 0);
			action_param_->buttons_up |= (this->ButtonUp(i)? (1UL << i) : 0);
		}

		iam.UpdateInputActions(ret, JS_LeftThumbX, action_param_);
		iam.UpdateInputActions(ret, JS_LeftThumbY, action_param_);
		iam.UpdateInputActions(ret, JS_LeftThumbZ, action_param_);
		iam.UpdateInputActions(ret, JS_RightThumbX, action_param_);
		iam.UpdateInputActions(ret, JS_RightThumbY, action_param_);
		iam.UpdateInputActions(ret, JS_RightThumbZ, action_param_);
		iam.UpdateInputActions(ret, JS_LeftTrigger, action_param_);
		iam.UpdateInputActions(ret, JS_RightTrigger, action_param_);

		bool any_button = false;
		for (uint16_t i = 0; i < this->NumButtons(); ++ i)
		{
			if (buttons_[index_][i] || buttons_[!index_][i])
			{
				iam.UpdateInputActions(ret, static_cast<uint16_t>(JS_Button0 + i), action_param_);
				any_button = true;
			}
		}
		if (any_button)
		{
			iam.UpdateInputActions(ret, JS_AnyButton, action_param_);
		}

		return ret;
	}
}
