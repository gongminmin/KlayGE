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
#include <boost/smart_ptr.hpp>

#include <boost/utility.hpp>

#pragma comment(lib, "KlayGE_InputEngine_DInput.lib")

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

	private:
		boost::shared_ptr<IDirectInput8W> dinput_;

	private:
		static BOOL CALLBACK EnumDevicesCB(LPCDIDEVICEINSTANCEW didi, void* pvRef);
	};

	class DInputKeyboard : public InputKeyboard
	{
	public:
		DInputKeyboard(REFGUID guid, InputEngine& inputEng);

		std::wstring const & Name() const;

	private:
		void UpdateInputs();
	};

	class DInputMouse : public InputMouse
	{
	public:
		DInputMouse(REFGUID guid, InputEngine& inputEng);

		std::wstring const & Name() const;

	private:
		void UpdateInputs();
	};

	class DInputJoystick : public InputJoystick
	{
	public:
		DInputJoystick(REFGUID guid, InputEngine& inputEng);

		std::wstring const & Name() const;

	private:
		void UpdateInputs();
	};
}

#endif		// _DINPUT_HPP
