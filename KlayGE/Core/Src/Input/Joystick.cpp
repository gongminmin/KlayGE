// Joystick.cpp
// KlayGE 游戏杆管理类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

#include <cassert>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	InputJoystick::~InputJoystick()
	{
	}

	// X轴位置
	/////////////////////////////////////////////////////////////////////////////////
	long InputJoystick::XPos() const
	{
		return pos_.x();
	}

	// Y轴位置
	/////////////////////////////////////////////////////////////////////////////////
	long InputJoystick::YPos() const
	{
		return pos_.y();
	}
	
	// Z轴位置
	/////////////////////////////////////////////////////////////////////////////////
	long InputJoystick::ZPos() const
	{
		return pos_.z();
	}

	// X轴旋转
	/////////////////////////////////////////////////////////////////////////////////
	long InputJoystick::XRot() const
	{
		return rot_.x();
	}

	// Y轴旋转
	/////////////////////////////////////////////////////////////////////////////////
	long InputJoystick::YRot() const
	{
		return rot_.y();
	}
	
	// Z轴旋转
	/////////////////////////////////////////////////////////////////////////////////
	long InputJoystick::ZRot() const
	{
		return rot_.z();
	}

	// 获取滑杆的值
	/////////////////////////////////////////////////////////////////////////////////
	long InputJoystick::Slider(size_t index) const
	{
		assert(index < slider_.size());

		return slider_[index];
	}

	// 获取指定键是否按下
	/////////////////////////////////////////////////////////////////////////////////
	bool InputJoystick::Button(size_t index) const
	{
		assert(index < buttons_.size());

		return buttons_[index];
	}

	// 实现动作映射
	//////////////////////////////////////////////////////////////////////////////////
	void InputJoystick::DoActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = JS_XPos; i < JS_Button31; ++ i)
		{
			if (actionMap.HasAction(i))
			{
				iam.AddAction(InputAction(actionMap.Action(i), i));
			}
		}
	}

	// 更新游戏杆动作
	/////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputJoystick::DoUpdate(uint32_t id)
	{
		InputActionsType ret;

		actionMaps_[id].UpdateInputActions(ret, JS_XPos, this->XPos());
		actionMaps_[id].UpdateInputActions(ret, JS_YPos, this->YPos());
		actionMaps_[id].UpdateInputActions(ret, JS_ZPos, this->ZPos());
		actionMaps_[id].UpdateInputActions(ret, JS_XRot, this->XRot());
		actionMaps_[id].UpdateInputActions(ret, JS_YRot, this->YRot());
		actionMaps_[id].UpdateInputActions(ret, JS_ZRot, this->ZRot());

		for (uint16_t i = 0; i < static_cast<uint16_t>(slider_.size()); ++ i)
		{
			actionMaps_[id].UpdateInputActions(ret, static_cast<uint16_t>(JS_Slider0 + i), this->Slider(i));
		}
		for (uint16_t i = 0; i < static_cast<uint16_t>(buttons_.size()); ++ i)
		{
			if (this->Button(i))
			{
				actionMaps_[id].UpdateInputActions(ret, static_cast<uint16_t>(JS_Button0 + i));
			}
		}

		return ret;
	}
}
