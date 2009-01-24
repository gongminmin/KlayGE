// DInputEngine.cpp
// KlayGE DirectInput输入引擎类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://klayge.sourceforge.net
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#define INITGUID
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/DInput/DInput.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "dinput8.lib")
#endif

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputEngine::DInputEngine()
	{
		// 建立 DirectInput 对象
		IDirectInput8W* di;
		DirectInput8Create(::GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
			IID_IDirectInput8W, reinterpret_cast<void**>(&di), NULL);
		dinput_ = MakeCOMPtr(di);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputEngine::~DInputEngine()
	{
		devices_.clear();
	}

	// 获取DirectInput接口
	/////////////////////////////////////////////////////////////////////////////////
	boost::shared_ptr<IDirectInput8W> const & DInputEngine::DInput() const
	{
		return dinput_;
	}

	// 获取窗口句柄
	/////////////////////////////////////////////////////////////////////////////////
	HWND DInputEngine::HWnd() const
	{
		return hwnd_;
	}

	// 输入引擎名称
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & DInputEngine::Name() const
	{
		static std::wstring const name(L"DirectInput Input Engine");
		return name;
	}

	// 枚举设备
	/////////////////////////////////////////////////////////////////////////////////
	void DInputEngine::EnumDevices()
	{
		TIF(dinput_->EnumDevices(DI8DEVCLASS_ALL, EnumDevicesCB, this, DIEDFL_ALLDEVICES));
		hwnd_ = Context::Instance().AppInstance().MainWnd()->HWnd();
	}

	// 枚举设备的回调函数
	//////////////////////////////////////////////////////////////////////////////////
	BOOL CALLBACK DInputEngine::EnumDevicesCB(LPCDIDEVICEINSTANCEW didi, void* pvRef)
	{
		DInputEngine& inputEng(*(reinterpret_cast<DInputEngine*>(pvRef)));

		InputDevicePtr device;

		switch (GET_DIDEVICE_TYPE(didi->dwDevType))
		{
		case DI8DEVTYPE_KEYBOARD:
			device = MakeSharedPtr<DInputKeyboard>(didi->guidInstance, inputEng);
			break;

		case DI8DEVTYPE_MOUSE:
			device = MakeSharedPtr<DInputMouse>(didi->guidInstance, inputEng);
			break;

		case DI8DEVTYPE_JOYSTICK:
			device = MakeSharedPtr<DInputJoystick>(didi->guidInstance, inputEng);
			break;
		}

		if (device)
		{
			inputEng.devices_.push_back(device);
		}

		return DIENUM_CONTINUE;
	}
}
