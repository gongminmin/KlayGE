// DIMouse.cpp
// KlayGE DInput鼠标管理类 实现文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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
#include <boost/lambda/lambda.hpp>

#include <KlayGE/DInput/DInput.hpp>
#include <KlayGE/DInput/DInputDeviceImpl.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputMouse::DInputMouse(REFGUID guid, InputEngine& inputEng)
	{
		boost::shared_ptr<DInputDeviceImpl> didImpl(new DInputDeviceImpl(guid, inputEng));
		impl_ = didImpl;

		didImpl->DataFormat(c_dfDIMouse);
		didImpl->CooperativeLevel(::GetActiveWindow(), DISCL_EXCLUSIVE | DISCL_FOREGROUND);

		// 把鼠标的设为相对模式
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize		= sizeof(dipdw);
		dipdw.diph.dwHeaderSize	= sizeof(dipdw.diph);
		dipdw.diph.dwObj		= 0;
		dipdw.diph.dwHow		= DIPH_DEVICE;
		dipdw.dwData			= DIPROPAXISMODE_REL;
		didImpl->Property(DIPROP_AXISMODE, dipdw.diph);

		this->Acquire();
	}

	// 设备名称
	//////////////////////////////////////////////////////////////////////////////////
	std::wstring const & DInputMouse::Name() const
	{
		static std::wstring const name(L"DirectInput Mouse");
		return name;
	}

	// 更新鼠标状态
	//////////////////////////////////////////////////////////////////////////////////
	void DInputMouse::UpdateInputs()
	{
		assert(dynamic_cast<DInputDeviceImpl*>(impl_.get()) != NULL);

		DIMOUSESTATE diMouseState;
		static_cast<DInputDeviceImpl*>(impl_.get())->DeviceState(&diMouseState, sizeof(diMouseState));

		pos_ = Vector_T<long, 3>(diMouseState.lX, diMouseState.lY, diMouseState.lZ);

		std::transform(diMouseState.rgbButtons, diMouseState.rgbButtons + buttons_.size(),
			buttons_.begin(), boost::lambda::_1 != 0);
	}
}
