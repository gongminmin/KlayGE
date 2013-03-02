// DInputDevice.hpp
// KlayGE DirectInput输入引擎类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 2.8.0
// 改为基类 (2005.8.11)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DINPUTDEVICE_HPP
#define _DINPUTDEVICE_HPP

#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <KlayGE/Input.hpp>

namespace KlayGE
{
	class DInputDevice
	{
	public:
		DInputDevice(REFGUID guid, InputEngine const & inputEng);
		virtual ~DInputDevice();

		void Acquire();
		void Unacquire();

		void DataFormat(DIDATAFORMAT const & df);
		void CooperativeLevel(HWND hwnd, DWORD flags);
		void Property(REFGUID rguidProp, DIPROPHEADER const & diph);

		void Poll();
		void DeviceState(void* data, size_t size);
		void DeviceData(size_t size, DIDEVICEOBJECTDATA* rgdod, uint32_t& num_elements);

	protected:
		shared_ptr<IDirectInputDevice8W> device_;
	};
}

#endif		// _DINPUTDEVICE_HPP
