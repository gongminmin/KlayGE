// InputActionMap.cpp
// KlayGE ���붯��ӳ���� ʵ���ļ�
// Ver 2.0.5
// ��Ȩ����(C) ������, 2004
// Homepage: http://www.klayge.org
//
// 2.0.5
// ���ν��� (2004.4.10)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// ���Ӷ���
	//////////////////////////////////////////////////////////////////////////////////
	void InputActionMap::AddAction(InputActionDefine const & action_define)
	{
		actionMap_.emplace(action_define.second, action_define.first);
	}

	// �������붯��
	//////////////////////////////////////////////////////////////////////////////////
	void InputActionMap::UpdateInputActions(InputActionsType& actions, uint16_t key, InputActionParamPtr const & param)
	{
		if (this->HasAction(key))
		{
			actions.emplace_back(this->Action(key), param);
		}
	}

	// ӳ���д������key
	//////////////////////////////////////////////////////////////////////////////////
	bool InputActionMap::HasAction(uint16_t key) const
	{
		return (actionMap_.find(key) != actionMap_.end());
	}

	// ��key��ȡ����
	//////////////////////////////////////////////////////////////////////////////////
	uint16_t InputActionMap::Action(uint16_t key) const
	{
		return actionMap_.find(key)->second;
	}
}
