// DIKeyboard.cpp
// KlayGE DInput键盘管理类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <KlayGE/DInput/DInput.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputKeyboard::DInputKeyboard(REFGUID guid, InputEngine& inputEng)
	{
		DInputEngine& dinputEng(reinterpret_cast<DInputEngine&>(inputEng));

		IDirectInputDevice8W* device;
		dinputEng.DInput()->CreateDevice(guid, &device, NULL);
		device_ = COMPtr<IDirectInputDevice8W>(device);

		device_->SetDataFormat(&c_dfDIKeyboard);
		device_->SetCooperativeLevel(::GetActiveWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

		this->Acquire();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputKeyboard::~DInputKeyboard()
	{
		this->Unacquire();
	}

	// 设备名称
	//////////////////////////////////////////////////////////////////////////////////
	const WString& DInputKeyboard::Name() const
	{
		static WString name(L"DirectInput Keyboard");
		return name;
	}

	// 获取设备
	/////////////////////////////////////////////////////////////////////////////////
	void DInputKeyboard::Acquire()
	{
		TIF(device_->Acquire());
	}

	// 释放设备
	//////////////////////////////////////////////////////////////////////////////////
	void DInputKeyboard::Unacquire()
	{
		TIF(device_->Unacquire());
	}

	// 更新键盘状态
	//////////////////////////////////////////////////////////////////////////////////
	void DInputKeyboard::UpdateKeys()
	{
		U8 keys[256];

		bool done;
		do
		{
			HRESULT hr(device_->GetDeviceState(sizeof(keys), &keys));
			if ((DIERR_INPUTLOST == hr) || (DIERR_NOTACQUIRED == hr))
			{
				this->Acquire();
				done = false;
			}
			else
			{
				done = true;
			}
		} while (!done);

		for (size_t i = 0; i < sizeof(keys); ++ i)
		{
			keys_[i] = ((keys[i] & 0x80) != 0);
		}
	}
}
