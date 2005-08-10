// InputDevice.cpp
// KlayGE 输入设备基类 实现文件
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

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	InputDevice::~InputDevice()
	{
	}

	// 设置动作映射
	/////////////////////////////////////////////////////////////////////////////////
	void InputDevice::ActionMap(action_maps_t const & ams)
	{
		actionMaps_ = ams;

		for (action_maps_t::iterator iter = actionMaps_.begin();
			iter != actionMaps_.end(); ++ iter)
		{
			this->DoActionMap(iter->first, iter->second);
		}
	}

	// 更新动作
	//////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputDevice::Update(uint32_t id)
	{
		this->UpdateInputs();

		return this->DoUpdate(id);
	}
}
