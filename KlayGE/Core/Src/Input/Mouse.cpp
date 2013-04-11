// Mouse.cpp
// KlayGE 鼠标管理类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// 增加了Action map id (2005.4.3)
//
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
	InputMouse::InputMouse()
		: abs_pos_(0, 0), offset_(0, 0, 0), num_buttons_(0), index_(false),
			shift_ctrl_alt_(0),
			action_param_(MakeSharedPtr<InputMouseActionParam>())
	{
		buttons_[0].fill(false);
		buttons_[1].fill(false);

		action_param_->type = InputEngine::IDT_Mouse;
	}

	InputMouse::~InputMouse()
	{
	}

	// 窗口坐标的X
	//////////////////////////////////////////////////////////////////////////////////
	long InputMouse::AbsX() const
	{
		return abs_pos_.x();
	}

	// 窗口坐标的Y
	//////////////////////////////////////////////////////////////////////////////////
	long InputMouse::AbsY() const
	{
		return abs_pos_.y();
	}

	// X轴
	//////////////////////////////////////////////////////////////////////////////////
	long InputMouse::X() const
	{
		return offset_.x();
	}

	// Y轴
	//////////////////////////////////////////////////////////////////////////////////
	long InputMouse::Y() const
	{
		return offset_.y();
	}

	// Z轴，也就是滚轮
	//////////////////////////////////////////////////////////////////////////////////
	long InputMouse::Z() const
	{
		return offset_.z();
	}

	// 左键
	//////////////////////////////////////////////////////////////////////////////////
	bool InputMouse::LeftButton() const
	{
		return this->Button(0);
	}

	// 右键
	//////////////////////////////////////////////////////////////////////////////////
	bool InputMouse::RightButton() const
	{
		return this->Button(1);
	}

	// 中键
	//////////////////////////////////////////////////////////////////////////////////
	bool InputMouse::MiddleButton() const
	{
		return this->Button(2);
	}

	// 获取键的数量
	//////////////////////////////////////////////////////////////////////////////////
	size_t InputMouse::NumButtons() const
	{
		return num_buttons_;
	}

	// 获取某按钮是否按下
	//////////////////////////////////////////////////////////////////////////////////
	bool InputMouse::Button(size_t n) const
	{
		BOOST_ASSERT(n < buttons_[index_].size());

		return buttons_[index_][n];
	}

	bool InputMouse::ButtonDown(size_t n) const
	{
		BOOST_ASSERT(n < buttons_[index_].size());

		return (buttons_[index_][n] && !buttons_[!index_][n]);
	}

	bool InputMouse::ButtonUp(size_t n) const
	{
		BOOST_ASSERT(n < buttons_[index_].size());

		return (!buttons_[index_][n] && buttons_[!index_][n]);
	}

	// 实现动作映射
	//////////////////////////////////////////////////////////////////////////////////
	void InputMouse::ActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = MS_X; i <= MS_AnyButton; ++ i)
		{
			if (actionMap.HasAction(i))
			{
				iam.AddAction(InputActionDefine(actionMap.Action(i), i));
			}
		}
	}

	// 更新鼠标动作
	//////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputMouse::UpdateActionMap(uint32_t id)
	{
		InputActionsType ret;

		InputActionMap& iam = actionMaps_[id];

		action_param_->move_vec = int2(offset_.x(), offset_.y());
		action_param_->wheel_delta = offset_.z();
		action_param_->abs_coord = abs_pos_;
		action_param_->buttons_state = 0;
		action_param_->buttons_down = 0;
		action_param_->buttons_up = shift_ctrl_alt_;
		for (size_t i = 0; i < this->NumButtons(); ++ i)
		{
			action_param_->buttons_state |= (this->Button(i) ? (1UL << i) : 0);
			action_param_->buttons_down |= (this->ButtonDown(i) ? (1UL << i) : 0);
			action_param_->buttons_up |= (this->ButtonUp(i) ? (1UL << i) : 0);
		}

		if (offset_.x() != 0)
		{
			iam.UpdateInputActions(ret, MS_X, action_param_);
		}
		if (offset_.y() != 0)
		{
			iam.UpdateInputActions(ret, MS_Y, action_param_);
		}
		if (offset_.z() != 0)
		{
			iam.UpdateInputActions(ret, MS_Z, action_param_);
		}
		bool any_button = false;
		for (uint16_t i = 0; i < this->NumButtons(); ++ i)
		{
			if (buttons_[index_][i] || buttons_[!index_][i])
			{
				iam.UpdateInputActions(ret, static_cast<uint16_t>(MS_Button0 + i), action_param_);
				any_button = true;
			}
		}
		if (any_button)
		{
			iam.UpdateInputActions(ret, MS_AnyButton, action_param_);
		}

		return ret;
	}
}
