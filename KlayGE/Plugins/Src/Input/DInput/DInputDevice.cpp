// DInputDevice.cpp
// KlayGE DInput设备实现类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://www.klayge.org
//
// 2.8.0
// 改名为DInputDevice (2005.8.11)
//
// 2.1.2
// 初次建立 (2004.9.5)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/COMPtr.hpp>

#include <boost/assert.hpp>

#include <KlayGE/DInput/DInput.hpp>
#include <KlayGE/DInput/DInputDevice.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputDevice::DInputDevice(REFGUID guid, InputEngine const & inputEng)
	{
		DInputEngine const & dinputEng = *checked_cast<DInputEngine const *>(&inputEng);

		IDirectInputDevice8W* device;
		dinputEng.DInput()->CreateDevice(guid, &device, nullptr);
		device_ = MakeCOMPtr(device);

		this->Unacquire();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputDevice::~DInputDevice()
	{
		this->Unacquire();
	}

	// 获取设备
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::Acquire()
	{
		if (DIERR_OTHERAPPHASPRIO == device_->Acquire())
		{
			device_->Acquire();
		}
	}

	// 释放设备
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::Unacquire()
	{
		HRESULT hr = device_->Unacquire();
		if ((hr != DI_OK) && (hr != DI_NOEFFECT))
		{
			device_->Unacquire();
		}
	}

	// 设置数据格式
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::DataFormat(const DIDATAFORMAT& df)
	{
		device_->SetDataFormat(&df);
	}

	// 设置协作级别
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::CooperativeLevel(HWND hwnd, DWORD flags)
	{
		device_->SetCooperativeLevel(hwnd, flags);
	}

	// 设置设备属性
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::Property(REFGUID rguidProp, const DIPROPHEADER& diph)
	{
		TIF(device_->SetProperty(rguidProp, &diph));
	}

	// 轮循设备
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::Poll()
	{
		HRESULT hr = device_->Poll();
		if ((DIERR_INPUTLOST == hr) || (DIERR_NOTACQUIRED == hr))
		{
			this->Acquire();

			device_->Poll();
		}
	}

	// 获取设备状态
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::DeviceState(void* data, size_t size)
	{
		HRESULT hr = device_->GetDeviceState(static_cast<DWORD>(size), data);
		if ((DIERR_INPUTLOST == hr) || (DIERR_NOTACQUIRED == hr))
		{
			this->Acquire();

			device_->GetDeviceState(static_cast<DWORD>(size), data);
		}
	}

	// 获取设备数据，缓冲状态
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::DeviceData(size_t size, DIDEVICEOBJECTDATA* rgdod, uint32_t& num_elements)
	{
		HRESULT hr = device_->GetDeviceData(static_cast<DWORD>(size), rgdod,
						reinterpret_cast<DWORD*>(&num_elements), 0);
		if ((DIERR_INPUTLOST == hr) || (DIERR_NOTACQUIRED == hr))
		{
			this->Acquire();

			device_->GetDeviceData(static_cast<DWORD>(size), rgdod,
						reinterpret_cast<DWORD*>(&num_elements), 0);
		}
	}
}
