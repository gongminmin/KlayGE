// InputActionMap.cpp
// KlayGE 输入动作映射类 实现文件
// Ver 2.0.5
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.5
// 初次建立 (2004.4.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <cassert>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// 增加动作
	//////////////////////////////////////////////////////////////////////////////////
	void InputActionMap::AddAction(InputAction const & inputAction)
	{
		actionMap_.insert(std::make_pair(inputAction.second, inputAction.first));
	}

	// 更新输入动作
	//////////////////////////////////////////////////////////////////////////////////
	void InputActionMap::UpdateInputActions(InputActionsType& actions, U16 key, long value)
	{
		if (this->HasAction(key))
		{
			actions.push_back(std::make_pair(this->Action(key), value));
		}
	}

	// 映射中存在这个key
	//////////////////////////////////////////////////////////////////////////////////
	bool InputActionMap::HasAction(U16 key) const
	{
		return (actionMap_.find(key) != actionMap_.end());
	}

	// 从key获取动作
	//////////////////////////////////////////////////////////////////////////////////
	U16 InputActionMap::Action(U16 key) const
	{
		return actionMap_.find(key)->second;
	}
}