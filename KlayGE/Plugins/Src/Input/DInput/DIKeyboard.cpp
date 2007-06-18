// DIKeyboard.cpp
// KlayGE DInput键盘管理类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
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
#include <KlayGE/Util.hpp>

#include <boost/assert.hpp>
#include <boost/array.hpp>
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

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputKeyboard::DInputKeyboard(REFGUID guid, InputEngine const & inputEng)
						: DInputDevice(guid, inputEng)
	{
		this->DataFormat(c_dfDIKeyboard);
		this->CooperativeLevel(checked_cast<DInputEngine const *>(&inputEng)->HWnd(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	}

	// 设备名称
	//////////////////////////////////////////////////////////////////////////////////
	std::wstring const & DInputKeyboard::Name() const
	{
		static std::wstring const name(L"DirectInput Keyboard");
		return name;
	}

	// 获取设备
	//////////////////////////////////////////////////////////////////////////////////
	void DInputKeyboard::Acquire()
	{
		DInputDevice::Acquire();
	}

	// 释放设备
	//////////////////////////////////////////////////////////////////////////////////
	void DInputKeyboard::Unacquire()
	{
		DInputDevice::Unacquire();
	}

	// 更新键盘状态
	//////////////////////////////////////////////////////////////////////////////////
	void DInputKeyboard::UpdateInputs()
	{
		boost::array<uint8_t, 256> keys;
		this->DeviceState(&keys[0], keys.size());

		std::transform(keys.begin(), keys.end(), keys_.begin(),
			(boost::lambda::_1 & 0x80) != 0);
	}
}
