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

#ifndef _DINPUT_HPP
#define _DINPUT_HPP

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <KlayGE/Input.hpp>
#include <KlayGE/COMPtr.hpp>

#pragma comment(lib, "KlayGE_InputEngine_DInput.lib")

namespace KlayGE
{
	// 管理输入设备
	/////////////////////////////////////////////////////////////////////////////////
	class DInputEngine : public InputEngine
	{
	public:
		DInputEngine();
		~DInputEngine();

		const WString& Name() const;
		void EnumDevices();

		const COMPtr<IDirectInput8W>& DInput() const
			{ return dinput_; }

	private:
		COMPtr<IDirectInput8W> dinput_;

	private:
		static BOOL CALLBACK EnumDevicesCB(LPCDIDEVICEINSTANCEW didi, void* pvRef);

	private:
		DInputEngine(const DInputEngine&);
		DInputEngine& operator=(const DInputEngine&);
	};

	class DInputKeyboard : public InputKeyboard
	{
	public:
		DInputKeyboard(REFGUID guid, InputEngine& inputEng);
		~DInputKeyboard();

		const WString& Name() const;

		void Acquire();
		void Unacquire();

	private:
		void UpdateKeys();

		COMPtr<IDirectInputDevice8W> device_;
	};

	class DInputMouse : public InputMouse
	{
	public:
		DInputMouse(REFGUID guid, InputEngine& inputEng);
		~DInputMouse();

		const WString& Name() const;

		void Acquire();
		void Unacquire();

	private:
		void UpdateKeys();

		COMPtr<IDirectInputDevice8W> device_;
	};

	class DInputJoystick : public InputJoystick
	{
	public:
		DInputJoystick(REFGUID guid, InputEngine& inputEng);
		~DInputJoystick();

		const WString& Name() const;

		void Acquire();
		void Unacquire();

	private:
		void UpdateKeys();

		COMPtr<IDirectInputDevice8W> device_;
	};
}

#endif		// _DINPUT_HPP
