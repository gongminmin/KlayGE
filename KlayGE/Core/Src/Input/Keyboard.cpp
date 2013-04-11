// Keyboard.cpp
// KlayGE 键盘管理类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
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
	InputKeyboard::InputKeyboard()
		: index_(false),
			action_param_(MakeSharedPtr<InputKeyboardActionParam>())
	{
		keys_[0].fill(false);
		keys_[1].fill(false);

		action_param_->type = InputEngine::IDT_Keyboard;
	}

	InputKeyboard::~InputKeyboard()
	{
	}

	// 获取键的数量
	//////////////////////////////////////////////////////////////////////////////////
	size_t InputKeyboard::NumKeys() const
	{
		return keys_[index_].size();
	}

	// 获取某键是否按下
	//////////////////////////////////////////////////////////////////////////////////
	bool InputKeyboard::Key(size_t n) const
	{
		BOOST_ASSERT(n < keys_[index_].size());

		return keys_[index_][n];
	}

	bool const * InputKeyboard::Keys() const
	{
		return &keys_[index_][0];
	}

	bool InputKeyboard::KeyDown(size_t n) const
	{
		BOOST_ASSERT(n < keys_[index_].size());

		return (keys_[index_][n] && !keys_[!index_][n]);
	}

	bool InputKeyboard::KeyUp(size_t n) const
	{
		BOOST_ASSERT(n < keys_[index_].size());

		return (!keys_[index_][n] && keys_[!index_][n]);
	}

	// 实现动作映射
	//////////////////////////////////////////////////////////////////////////////////
	void InputKeyboard::ActionMap(uint32_t id, InputActionMap const & actionMap)
	{
		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = KS_Escape; i <= KS_AnyKey; ++ i)
		{
			if (actionMap.HasAction(i))
			{
				iam.AddAction(InputActionDefine(actionMap.Action(i), i));
			}
		}
	}

	// 更新键盘动作
	//////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputKeyboard::UpdateActionMap(uint32_t id)
	{
		InputActionsType ret;

		InputActionMap& iam = actionMaps_[id];

		for (uint16_t i = 0; i < this->NumKeys(); ++ i)
		{
			action_param_->buttons_state[i] = this->Key(i);
			action_param_->buttons_down[i] = this->KeyDown(i);
			action_param_->buttons_up[i] = this->KeyUp(i);
		}

		bool any_key = false;
		for (uint16_t i = 0; i < this->NumKeys(); ++ i)
		{
			if (keys_[index_][i] || keys_[!index_][i])
			{
				iam.UpdateInputActions(ret, i, action_param_);
				any_key = true;
			}
		}
		if (any_key)
		{
			iam.UpdateInputActions(ret, KS_AnyKey, action_param_);
		}

		return ret;
	}
}
