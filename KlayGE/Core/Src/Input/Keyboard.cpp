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

#include <boost/assert.hpp>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// 获取键的数量
	//////////////////////////////////////////////////////////////////////////////////
	size_t InputKeyboard::NumKeys() const
	{
		return keys_.size();
	}

	// 获取某键是否按下
	//////////////////////////////////////////////////////////////////////////////////
	bool InputKeyboard::Key(size_t n) const
	{
		BOOST_ASSERT(n < keys_.size());

		return keys_[n];
	}

	bool const * InputKeyboard::Keys() const
	{
		return &keys_[0];
	}

	// 实现动作映射
	//////////////////////////////////////////////////////////////////////////////////
	void InputKeyboard::ActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = 0; i < this->NumKeys(); ++ i)
		{
			if (actionMap.HasAction(i))
			{
				iam.AddAction(InputActionDefine(actionMap.Action(i), i));
			}
		}
	}

	// 更新键盘动作
	//////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputKeyboard::DoUpdate(uint32_t id)
	{
		InputActionsType ret;

		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = 0; i < this->NumKeys(); ++ i)
		{
			if (this->Key(i))
			{
				iam.UpdateInputActions(ret, i);
			}
		}

		return ret;
	}
}
