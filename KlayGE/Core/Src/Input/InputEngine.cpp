// InputEngine.cpp
// KlayGE ���������� ʵ���ļ�
// Ver 2.5.0
// ��Ȩ����(C) ������, 2003-2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// ������Action map id (2005.4.3)
//
// 2.1.3
// ���㷨������дѭ�� (2004.10.16)
//
// 2.0.0
// ���ν��� (2003.8.29)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>

#include <vector>

#include <boost/assert.hpp>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// ��������
	//////////////////////////////////////////////////////////////////////////////////
	InputEngine::~InputEngine()
	{
	}

	// ���ö�����ʽ
	//////////////////////////////////////////////////////////////////////////////////
	void InputEngine::ActionMap(InputActionMap const & actionMap, action_handler_t handler)
	{
		// �����µĶ�����ʽ
		action_handlers_.emplace_back(actionMap, handler);

		if (devices_.empty())
		{
			this->EnumDevices();
		}

		// �Ե�ǰ�豸Ӧ���µĶ���ӳ��
		for (uint32_t id = 0; id < action_handlers_.size(); ++ id)
		{
			for (auto const & device : devices_)
			{
				device->ActionMap(id, action_handlers_[id].first);
			}
		}
	}

	// ��ȡ�����豸����
	//////////////////////////////////////////////////////////////////////////////////
	size_t InputEngine::NumDevices() const
	{
		return devices_.size();
	}

	// ˢ������״̬
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

				// ���������豸
				for (auto const & device : devices_)
				{
					InputActionsType const theAction(device->UpdateActionMap(id));

					// ȥ���ظ��Ķ���
					for (auto const & act : theAction)
					{
						if (actions.find(act.first) == actions.end())
						{
							actions.insert(act);

							// ������
							(*action_handlers_[id].second)(*this, act);
						}
					}
				}
			}
		}
	}

	// ��ȡˢ��ʱ����
	//////////////////////////////////////////////////////////////////////////////////
	float InputEngine::ElapsedTime() const
	{
		return elapsed_time_;
	}

	// ��ȡ�豸�ӿ�
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
