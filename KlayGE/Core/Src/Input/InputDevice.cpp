// InputDevice.cpp
// Clay! 输入设备基类 实现文件
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
	void InputDevice::ActionMap(const InputActionMap& actionMap)
	{
		actionMap_ = actionMap;

		this->DoActionMap(actionMap_);
	}

	// 通过键和值生成动作
	/////////////////////////////////////////////////////////////////////////////////
	U32 InputDevice::MakeAction(U16 key, long value)
	{
		return ((value & 0xFFFF) << 16) | actionMap_.Action(key);
	}
}
