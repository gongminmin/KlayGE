// DIMouse.cpp
// KlayGE DInput鼠标管理类 实现文件
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
	DInputMouse::DInputMouse(REFGUID guid, InputEngine& inputEng)
	{
		device_ = CreateDevice(guid, inputEng);

		device_->SetDataFormat(&c_dfDIMouse);
		device_->SetCooperativeLevel(::GetActiveWindow(), DISCL_EXCLUSIVE | DISCL_FOREGROUND);

		// 把鼠标的设为相对模式
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize		= sizeof(dipdw);
		dipdw.diph.dwHeaderSize	= sizeof(dipdw.diph);
		dipdw.diph.dwObj		= 0;
		dipdw.diph.dwHow		= DIPH_DEVICE;
		dipdw.dwData			= DIPROPAXISMODE_REL;
		device_->SetProperty(DIPROP_AXISMODE, &(dipdw.diph));

		this->Acquire();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputMouse::~DInputMouse()
	{
		this->Unacquire();
	}

	// 设备名称
	//////////////////////////////////////////////////////////////////////////////////
	const std::wstring& DInputMouse::Name() const
	{
		static std::wstring name(L"DirectInput Mouse");
		return name;
	}

	// 获取设备
	/////////////////////////////////////////////////////////////////////////////////
	void DInputMouse::Acquire()
	{
		TIF(device_->Acquire());
	}

	// 释放设备
	//////////////////////////////////////////////////////////////////////////////////
	void DInputMouse::Unacquire()
	{
		TIF(device_->Unacquire());
	}

	// 更新鼠标状态
	//////////////////////////////////////////////////////////////////////////////////
	void DInputMouse::UpdateKeys()
	{
		DIMOUSESTATE diMouseState;
		bool done;
		do
		{
			HRESULT hr(device_->GetDeviceState(sizeof(diMouseState), &diMouseState));
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

		pos_ = Vector_T<long, 3>(diMouseState.lX, diMouseState.lY, diMouseState.lZ);

		for (size_t i = 0; i < buttons_.size(); ++ i)
		{
			buttons_[i] = (diMouseState.rgbButtons[i] != 0);
		}
	}
}
