// Joystick.cpp
// KlayGE 游戏杆管理类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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
	void InputJoystick::DoActionMap(const InputActionMap& actionMap)
	{
		if (actionMap.HasAction(JS_XPos))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(JS_XPos), JS_XPos));
		}
		if (actionMap_.HasAction(JS_XPos))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(JS_YPos), JS_YPos));
		}
		if (actionMap_.HasAction(JS_ZPos))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(JS_ZPos), JS_ZPos));
		}
		if (actionMap.HasAction(JS_XRot))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(JS_XRot), JS_XRot));
		}
		if (actionMap_.HasAction(JS_XRot))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(JS_YRot), JS_YRot));
		}
		if (actionMap_.HasAction(JS_ZRot))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(JS_ZRot), JS_ZRot));
		}

		for (U16 i = 0; i < static_cast<U16>(slider_.size()); ++ i)
		{
			if (actionMap_.HasAction(JS_Slider0 + i))
			{
				actionMap_.AddAction(InputAction(actionMap.Action(JS_Slider0 + i), JS_Slider0 + i));
			}
		}
		for (U16 i = 0; i < static_cast<U16>(buttons_.size()); ++ i)
		{
			if (actionMap_.HasAction(JS_Button0 + i))
			{
				actionMap_.AddAction(InputAction(actionMap.Action(JS_Button0 + i), JS_Button0 + i));
			}
		}
	}

	// 更新游戏杆动作
	/////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputJoystick::Update()
	{
		this->UpdateKeys();

		InputActionsType ret;

		actionMap_.UpdateInputActions(ret, JS_XPos, this->XPos());
		actionMap_.UpdateInputActions(ret, JS_YPos, this->YPos());
		actionMap_.UpdateInputActions(ret, JS_ZPos, this->ZPos());
		actionMap_.UpdateInputActions(ret, JS_XRot, this->XRot());
		actionMap_.UpdateInputActions(ret, JS_YRot, this->YRot());
		actionMap_.UpdateInputActions(ret, JS_ZRot, this->ZRot());

		for (U16 i = 0; i < static_cast<U16>(slider_.size()); ++ i)
		{
			actionMap_.UpdateInputActions(ret, JS_Slider0 + i, this->Slider(i));
		}
		for (U16 i = 0; i < static_cast<U16>(buttons_.size()); ++ i)
		{
			if (this->Button(i))
			{
				actionMap_.UpdateInputActions(ret, JS_Button0 + i);
			}
		}

		return ret;
	}
}
