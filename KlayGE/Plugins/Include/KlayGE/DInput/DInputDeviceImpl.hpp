// DInput.hpp
// KlayGE DirectInput输入引擎类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DINPUTDEVICEIMPL_HPP
#define _DINPUTDEVICEIMPL_HPP

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <KlayGE/Input.hpp>
#include <boost/smart_ptr.hpp>

#pragma comment(lib, "KlayGE_InputEngine_DInput.lib")

namespace KlayGE
{
	class DInputDeviceImpl : public InputDeviceImpl
	{
	public:
		DInputDeviceImpl(REFGUID guid, InputEngine& inputEng);

		void Acquire();
		void Unacquire();

		void DataFormat(const DIDATAFORMAT& df);
		void CooperativeLevel(HWND hwnd, DWORD flags);
		void Property(REFGUID rguidProp, const DIPROPHEADER& diph);

		void Poll();
		void DeviceState(void* data, size_t size);

	private:
		boost::shared_ptr<IDirectInputDevice8W> device_;
	};
}

#endif		// _DINPUTDEVICEIMPL_HPP