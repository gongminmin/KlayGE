// DInputDevice.cpp
// KlayGE DInput设备实现类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>

#include <boost/assert.hpp>

#include <KlayGE/DInput/DInput.hpp>
#include <KlayGE/DInput/DInputDevice.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputDevice::DInputDevice(REFGUID guid, InputEngine const & inputEng)
	{
		DInputEngine const & dinputEng(*checked_cast<DInputEngine const *>(&inputEng));

		IDirectInputDevice8W* device;
		dinputEng.DInput()->CreateDevice(guid, &device, NULL);
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
		while (DIERR_OTHERAPPHASPRIO == device_->Acquire())
		{
			Sleep(1);
		}
	}

	// 释放设备
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::Unacquire()
	{
		for (;;)
		{
			HRESULT hr = device_->Unacquire();
			if ((DI_OK == hr) || (DI_NOEFFECT == hr))
			{
				break;
			}
			else
			{
				Sleep(1);
			}
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
		for (;;)
		{
			HRESULT hr = device_->Poll();
			if ((DIERR_INPUTLOST == hr) || (DIERR_NOTACQUIRED == hr))
			{
				this->Acquire();
			}
			else
			{
				BOOST_ASSERT(DIERR_NOTINITIALIZED != hr);
				break;
			}
		}
	}

	// 获取设备状态
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::DeviceState(void* data, size_t size)
	{
		for (;;)
		{
			HRESULT hr = device_->GetDeviceState(static_cast<DWORD>(size), data);
			if ((DIERR_INPUTLOST == hr) || (DIERR_NOTACQUIRED == hr))
			{
				this->Acquire();
			}
			else
			{
				BOOST_ASSERT(DIERR_NOTINITIALIZED != hr);
				break;
			}
		}
	}

	// 获取设备数据，缓冲状态
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDevice::DeviceData(size_t size, DIDEVICEOBJECTDATA* rgdod, uint32_t& num_elements)
	{
		for (;;)
		{
			HRESULT hr = device_->GetDeviceData(size, rgdod, &num_elements, 0);
			if ((DIERR_INPUTLOST == hr) || (DIERR_NOTACQUIRED == hr))
			{
				this->Acquire();
			}
			else
			{
				BOOST_ASSERT(DIERR_NOTINITIALIZED != hr);
				break;
			}
		}
	}
}
