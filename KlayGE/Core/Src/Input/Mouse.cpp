// Mouse.cpp
// KlayGE 鼠标管理类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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
		: index_(false)
	{
		buttons_[0].assign(false);
		buttons_[1].assign(false);
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
		return buttons_[index_].size();
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

		for (uint16_t i = MS_X; i < MS_Button0 + this->NumButtons(); ++ i)
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

		if (this->X() != 0)
		{
			iam.UpdateInputActions(ret, MS_X, this->X());
		}
		if (this->Y() != 0)
		{
			iam.UpdateInputActions(ret, MS_Y, this->Y());
		}
		if (this->Z() != 0)
		{
			iam.UpdateInputActions(ret, MS_Z, this->Z());
		}
		for (uint16_t i = 0; i < this->NumButtons(); ++ i)
		{
			if (this->Button(i))
			{
				iam.UpdateInputActions(ret, static_cast<uint16_t>(MS_Button0 + i));
			}
		}

		return ret;
	}
}
