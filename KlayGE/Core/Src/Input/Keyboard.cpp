// Keyboard.cpp
// KlayGE 键盘管理类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
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
	// 获取某键是否按下
	//////////////////////////////////////////////////////////////////////////////////
	bool InputKeyboard::Key(size_t n) const
	{
		assert(n < keys_.size());

		return (keys_[n]);
	}

	// 实现动作映射
	//////////////////////////////////////////////////////////////////////////////////
	void InputKeyboard::DoActionMap(const InputActionMap& actionMap)
	{
		for (size_t i = 0; i < keys_.size(); ++ i)
		{
			if (actionMap.HasAction(i))
			{
				actionMap_.AddAction(InputAction(actionMap.Action(i), i));
			}
		}
	}

	// 更新键盘动作
	//////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputKeyboard::Update()
	{
		this->UpdateKeys();

		InputActionsType ret;

		for (size_t i = 0; i < keys_.size(); ++ i)
		{
			if (this->Key(i))
			{
				if (actionMap_.HasAction(i))
				{
					ret.push_back(actionMap_.Action(i));
				}
			}
		}

		return ret;
	}
}
