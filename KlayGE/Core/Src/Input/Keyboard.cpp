// Keyboard.cpp
// KlayGE 键盘管理类 实现文件
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
	// 获取某键是否按下
	//////////////////////////////////////////////////////////////////////////////////
	bool InputKeyboard::Key(size_t n) const
	{
		assert(n < keys_.size());

		return keys_[n];
	}

	// 实现动作映射
	//////////////////////////////////////////////////////////////////////////////////
	void InputKeyboard::DoActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		for (uint16_t i = 0; i < static_cast<uint16_t>(keys_.size()); ++ i)
		{
			if (actionMap.HasAction(i))
			{
				actionMaps_[id].AddAction(InputAction(actionMap.Action(i), i));
			}
		}
	}

	// 更新键盘动作
	//////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputKeyboard::DoUpdate(uint32_t id)
	{
		InputActionsType ret;

		for (uint16_t i = 0; i < static_cast<uint16_t>(keys_.size()); ++ i)
		{
			if (this->Key(i))
			{
				actionMaps_[id].UpdateInputActions(ret, i);
			}
		}

		return ret;
	}
}
