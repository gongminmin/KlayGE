// DInput.hpp
// KlayGE DirectInput输入引擎类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 改为多继承结构 (2005.8.11)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DINPUT_HPP
#define _DINPUT_HPP

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <KlayGE/Input.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

#include <KlayGE/DInput/DInputDevice.hpp>

namespace KlayGE
{
	// 管理输入设备
	/////////////////////////////////////////////////////////////////////////////////
	class DInputEngine : boost::noncopyable, public InputEngine
	{
	public:
		DInputEngine();
		~DInputEngine();

		std::wstring const & Name() const;
		void EnumDevices();

		boost::shared_ptr<IDirectInput8W> const & DInput() const;
		HWND HWnd() const;

	private:
		boost::shared_ptr<IDirectInput8W> dinput_;
		HWND hwnd_;

	private:
		static BOOL CALLBACK EnumDevicesCB(LPCDIDEVICEINSTANCEW didi, void* pvRef);
	};

	class DInputKeyboard : public InputKeyboard, public DInputDevice
	{
	public:
		DInputKeyboard(REFGUID guid, InputEngine const & inputEng);

		std::wstring const & Name() const;

		void Acquire();
		void Unacquire();

	private:
		void UpdateInputs();
	};

	class DInputMouse : public InputMouse, public DInputDevice
	{
	public:
		DInputMouse(REFGUID guid, InputEngine const & inputEng);

		std::wstring const & Name() const;

		void Acquire();
		void Unacquire();

	private:
		void UpdateInputs();
	};

	class DInputJoystick : public InputJoystick, public DInputDevice
	{
	public:
		DInputJoystick(REFGUID guid, InputEngine const & inputEng);

		std::wstring const & Name() const;

		void Acquire();
		void Unacquire();

	private:
		void UpdateInputs();
	};
}

#endif		// _DINPUT_HPP
