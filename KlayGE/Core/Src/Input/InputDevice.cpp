// InputDevice.cpp
// KlayGE 输入设备基类 实现文件
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
	void InputDevice::ActionMap(InputActionMap const & actionMap)
	{
		actionMap_ = actionMap;

		this->DoActionMap(actionMap_);
	}
}
