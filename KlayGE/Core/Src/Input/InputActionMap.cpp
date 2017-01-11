// InputActionMap.cpp
// KlayGE 输入动作映射类 实现文件
// Ver 2.0.5
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://www.klayge.org
//
// 2.0.5
// 初次建立 (2004.4.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// 增加动作
	//////////////////////////////////////////////////////////////////////////////////
	void InputActionMap::AddAction(InputActionDefine const & action_define)
	{
		actionMap_.emplace(action_define.semantic, action_define.action);
	}

	// 更新输入动作
	//////////////////////////////////////////////////////////////////////////////////
	void InputActionMap::UpdateInputActions(InputActionsType& actions, uint16_t key, InputActionParamPtr const & param)
	{
		if (this->HasAction(key))
		{
			actions.emplace_back(this->Action(key), param);
		}
	}

	// 映射中存在这个key
	//////////////////////////////////////////////////////////////////////////////////
	bool InputActionMap::HasAction(uint16_t key) const
	{
		return (actionMap_.find(key) != actionMap_.end());
	}

	// 从key获取动作
	//////////////////////////////////////////////////////////////////////////////////
	uint16_t InputActionMap::Action(uint16_t key) const
	{
		return actionMap_.find(key)->second;
	}
}
