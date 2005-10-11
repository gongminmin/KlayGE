// InputEngine.cpp
// KlayGE 输入引擎类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.5.0
// 增加了Action map id (2005.4.3)
//
// 2.1.3
// 用算法代替手写循环 (2004.10.16)
//
// 2.0.0
// 初次建立 (2003.8.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/MapVector.hpp>

#include <vector>

#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// 析构函数
	//////////////////////////////////////////////////////////////////////////////////
	InputEngine::~InputEngine()
	{
	}

	// 设置动作格式
	//////////////////////////////////////////////////////////////////////////////////
	void InputEngine::ActionMap(InputActionMap const & actionMap,
		action_handler_t handler, bool reenumerate)
	{
		// 保存新的动作格式
		action_handlers_.push_back(std::make_pair(actionMap, handler));

		// 只有当调用时指定要重新枚举时才销毁并重枚举设备
		// 设备列表有可能在循环中使用，如果这时枚举设备有可能造成问题
		if (reenumerate)
		{
			// 清除以前枚举的设备
			devices_.clear();

			// 重新枚举合适的设备
			this->EnumDevices();
		}

		// 对当前设备应用新的动作映射
		for (uint32_t id = 0; id < action_handlers_.size(); ++ id)
		{
			for (InputDevicesType::iterator iter = devices_.begin();
				iter != devices_.end(); ++ iter)
			{
				(*iter)->ActionMap(id, action_handlers_[id].first);
			}
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
	void InputEngine::Update()
	{
		for (uint32_t id = 0; id < action_handlers_.size(); ++ id)
		{
			typedef MapVector<uint16_t, long> ActionSetType;
			ActionSetType actions;

			// 访问所有设备
			for (InputDevicesType::iterator iter = devices_.begin(); iter != devices_.end(); ++ iter)
			{
				InputActionsType const theAction((*iter)->Update(id));

				// 去掉重复的动作
				for (InputActionsType::const_iterator i = theAction.begin(); i != theAction.end(); ++ i)
				{
					if (actions.find(i->first) == actions.end())
					{
						actions.insert(*i);

						// 处理动作
						action_handlers_[id].second(*i);
					}
				}
			}
		}
	}

	// 获取设备接口
	//////////////////////////////////////////////////////////////////////////////////
	InputDevicePtr InputEngine::Device(size_t index) const
	{
		BOOST_ASSERT(index < this->NumDevice());

		return devices_[index];
	}
}
