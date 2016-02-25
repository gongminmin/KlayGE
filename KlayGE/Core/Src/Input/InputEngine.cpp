// InputEngine.cpp
// KlayGE 输入引擎类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
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
#include <KFL/Util.hpp>

#include <vector>

#include <boost/assert.hpp>

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
	void InputEngine::ActionMap(InputActionMap const & actionMap, action_handler_t handler)
	{
		// 保存新的动作格式
		action_handlers_.emplace_back(actionMap, handler);

		if (devices_.empty())
		{
			this->EnumDevices();
		}

		// 对当前设备应用新的动作映射
		for (uint32_t id = 0; id < action_handlers_.size(); ++ id)
		{
			for (auto const & device : devices_)
			{
				device->ActionMap(id, action_handlers_[id].first);
			}
		}
	}

	// 获取输入设备个数
	//////////////////////////////////////////////////////////////////////////////////
	size_t InputEngine::NumDevices() const
	{
		return devices_.size();
	}

	// 刷新输入状态
	//////////////////////////////////////////////////////////////////////////////////
	void InputEngine::Update()
	{
		elapsed_time_ = static_cast<float>(timer_.elapsed());
		if (elapsed_time_ > 0.01f)
		{
			timer_.restart();

			for (auto const & device : devices_)
			{
				device->UpdateInputs();
			}

			for (uint32_t id = 0; id < action_handlers_.size(); ++ id)
			{
				boost::container::flat_map<uint16_t, InputActionParamPtr> actions;

				// 访问所有设备
				for (auto const & device : devices_)
				{
					InputActionsType const theAction(device->UpdateActionMap(id));

					// 去掉重复的动作
					for (auto const & act : theAction)
					{
						if (actions.find(act.first) == actions.end())
						{
							actions.insert(act);

							// 处理动作
							(*action_handlers_[id].second)(*this, act);
						}
					}
				}
			}
		}
	}

	// 获取刷新时间间隔
	//////////////////////////////////////////////////////////////////////////////////
	float InputEngine::ElapsedTime() const
	{
		return elapsed_time_;
	}

	// 获取设备接口
	//////////////////////////////////////////////////////////////////////////////////
	InputDevicePtr InputEngine::Device(size_t index) const
	{
		BOOST_ASSERT(index < this->NumDevices());

		return devices_[index];
	}

	void InputEngine::Suspend()
	{
		this->DoSuspend();
	}

	void InputEngine::Resume()
	{
		this->DoResume();
	}
}
