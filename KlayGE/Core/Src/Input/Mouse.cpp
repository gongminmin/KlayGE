// Mouse.cpp
// KlayGE 鼠标管理类 实现文件
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
	// 获取某按钮是否按下
	//////////////////////////////////////////////////////////////////////////////////
	bool InputMouse::Button(size_t index) const
	{
		assert(index < buttons_.size());

		return buttons_[index];
	}

	// 实现动作映射
	//////////////////////////////////////////////////////////////////////////////////
	void InputMouse::DoActionMap(const InputActionMap& actionMap)
	{
		if (actionMap.HasAction(MS_X))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(MS_X), MS_X));
		}
		if (actionMap_.HasAction(MS_Y))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(MS_Y), MS_Y));
		}
		if (actionMap_.HasAction(MS_Z))
		{
			actionMap_.AddAction(InputAction(actionMap.Action(MS_Z), MS_Z));
		}
		for (size_t i = 0; i < buttons_.size(); ++ i)
		{
			if (actionMap_.HasAction(MS_Button0 + i))
			{
				actionMap_.AddAction(InputAction(actionMap.Action(MS_Button0 + i), MS_Button0 + i));
			}
		}
	}

	// 更新鼠标动作
	//////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputMouse::Update()
	{
		this->UpdateKeys();

		InputActionsType ret;

		if (this->X() != 0)
		{
			actionMap_.UpdateInputActions(ret, MS_X, this->X());
		}
		if (this->Y() != 0)
		{
			actionMap_.UpdateInputActions(ret, MS_Y, this->Y());
		}
		if (this->Z() != 0)
		{
			actionMap_.UpdateInputActions(ret, MS_Z, this->Z());
		}
		for (size_t i = 0; i < buttons_.size(); ++ i)
		{
			if (this->Button(i))
			{
				actionMap_.UpdateInputActions(ret, MS_Button0 + i);
			}
		}

		return ret;
	}
}
