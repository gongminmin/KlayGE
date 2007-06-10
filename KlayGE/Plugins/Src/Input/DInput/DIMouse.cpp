// DIMouse.cpp
// KlayGE DInput鼠标管理类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 改为非独占模式 (2005.7.26)
// 改用多继承结构 (2005.8.11)
//
// 2.1.2
// 改用Bridge模式实现 (2004.9.5)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <algorithm>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4512)
#endif
#include <boost/lambda/lambda.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/DInput/DInput.hpp>
#include <KlayGE/DInput/DInputDevice.hpp>

#include <iostream>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputMouse::DInputMouse(REFGUID guid, InputEngine const & inputEng)
					: DInputDevice(guid, inputEng)
	{
		this->DataFormat(c_dfDIMouse);
		this->CooperativeLevel(::GetActiveWindow(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

		// 把鼠标的设为相对模式
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize		= sizeof(dipdw);
		dipdw.diph.dwHeaderSize	= sizeof(dipdw.diph);
		dipdw.diph.dwObj		= 0;
		dipdw.diph.dwHow		= DIPH_DEVICE;
		dipdw.dwData			= DIPROPAXISMODE_REL;
		this->Property(DIPROP_AXISMODE, dipdw.diph);
	}

	// 设备名称
	//////////////////////////////////////////////////////////////////////////////////
	std::wstring const & DInputMouse::Name() const
	{
		static std::wstring const name(L"DirectInput Mouse");
		return name;
	}

	// 获取设备
	//////////////////////////////////////////////////////////////////////////////////
	void DInputMouse::Acquire()
	{
		DInputDevice::Acquire();
	}

	// 释放设备
	//////////////////////////////////////////////////////////////////////////////////
	void DInputMouse::Unacquire()
	{
		DInputDevice::Unacquire();
	}

	// 更新鼠标状态
	//////////////////////////////////////////////////////////////////////////////////
	void DInputMouse::UpdateInputs()
	{
		DIMOUSESTATE diMouseState;
		this->DeviceState(&diMouseState, sizeof(diMouseState));

		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(::GetActiveWindow(), &pt);
		abs_pos_ = Vector_T<long, 2>(pt.x, pt.y);
		offset_ = Vector_T<long, 3>(diMouseState.lX, diMouseState.lY, diMouseState.lZ);

		std::transform(diMouseState.rgbButtons, diMouseState.rgbButtons + buttons_.size(),
			buttons_.begin(), boost::lambda::_1 != 0);
	}
}
