// InputEngine.cpp
// KlayGE 输入引擎类 实现文件
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
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/alloc.hpp>

#include <cassert>
#include <set>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// 析构函数
	//////////////////////////////////////////////////////////////////////////////////
	InputEngine::~InputEngine()
	{
		this->UnacquireDevices();
	}

	// 设置动作格式
	//////////////////////////////////////////////////////////////////////////////////
	void InputEngine::ActionMap(const InputActionMap& actionMap, bool reenumerate)
	{
		// 保存新的动作格式
		actionMap_ = actionMap;

		// 只有当调用时指定要重枚举才销毁并重枚举设备
		// 设备列表有可能在循环中使用，如果这时枚举设备有可能造成问题
		if (reenumerate)
		{
			// 清除以前枚举的设备
			devices_.clear();

			// 重新枚举合适的设备
			this->EnumDevices();
		}
		else // 应用新的动作映射
		{
			// 在新的动作映射设置前设备必须全部被释放
			this->UnacquireDevices();
		}

		// 对当前设备应用新的动作映射
		for (InputDevicesType::iterator iter = devices_.begin(); iter != devices_.end(); ++ iter)
		{
			(*iter)->ActionMap(actionMap_);
		}
	}

	// 获取输入设备个数
	//////////////////////////////////////////////////////////////////////////////////
	size_t InputEngine::NumDevice() const
	{
		return devices_.size();
	}

	// 刷新输入状态
	//////////////////////////////////////////////////////////////////////////////////
	InputActionsType InputEngine::Update()
	{
		typedef std::set<std::pair<U16, long>, std::less<std::pair<U16, long> >,
			alloc<std::pair<U16, long> > > ActionSetType;
		ActionSetType actions;

		// 访问所有设备
		for (InputDevicesType::iterator iter = devices_.begin(); iter != devices_.end(); ++ iter)
		{
			InputActionsType theAction((*iter)->Update());

			for (InputActionsType::iterator it = theAction.begin(); it != theAction.end(); ++ it)
			{
				actions.insert(*it);
			}
		}

		// 添加到动作列表
		return InputActionsType(actions.begin(), actions.end());
	}

	// 释放所有设备
	//////////////////////////////////////////////////////////////////////////////////
	void InputEngine::UnacquireDevices()
	{
		for (InputDevicesType::iterator iter = devices_.begin(); iter != devices_.end(); ++ iter)
		{
			(*iter)->Unacquire();
		}
	}

	// 获取设备接口
	//////////////////////////////////////////////////////////////////////////////////
	InputDevicePtr InputEngine::Device(size_t index) const
	{
		assert(index < this->NumDevice());

		return devices_[index];
	}
}
