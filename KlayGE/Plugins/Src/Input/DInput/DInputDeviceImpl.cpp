// DInputDeviceImpl.cpp
// KlayGE DInput设备实现类 实现文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.2
// 初次建立 (2004.9.5)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/DInput/DInput.hpp>
#include <KlayGE/DInput/DInputDeviceImpl.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DInputDeviceImpl::DInputDeviceImpl(REFGUID guid, InputEngine& inputEng)
	{
		DInputEngine& dinputEng(static_cast<DInputEngine&>(inputEng));

		IDirectInputDevice8W* device;
		dinputEng.DInput()->CreateDevice(guid, &device, NULL);
		device_ = MakeCOMPtr(device);
	}

	// 获取设备
	/////////////////////////////////////////////////////////////////////////////////
	void DInputDeviceImpl::Acquire()
	{
		TIF(device_->Acquire());
	}

	// 释放设备
	//////////////////////////////////////////////////////////////////////////////////
	void DInputDeviceImpl::Unacquire()
	{
		TIF(device_->Unacquire());
	}

	// 设置数据格式
	//////////////////////////////////////////////////////////////////////////////////
	void DInputDeviceImpl::DataFormat(const DIDATAFORMAT& df)
	{
		device_->SetDataFormat(&df);
	}

	// 设置协作级别
	//////////////////////////////////////////////////////////////////////////////////
	void DInputDeviceImpl::CooperativeLevel(HWND hwnd, DWORD flags)
	{
		device_->SetCooperativeLevel(hwnd, flags);
	}

	// 设置设备属性
	//////////////////////////////////////////////////////////////////////////////////
	void DInputDeviceImpl::Property(REFGUID rguidProp, const DIPROPHEADER& diph)
	{
		TIF(device_->SetProperty(rguidProp, &diph));
	}

	// 轮循设备
	//////////////////////////////////////////////////////////////////////////////////
	void DInputDeviceImpl::Poll()
	{
		TIF(device_->Poll());
	}

	// 获取设备状态
	//////////////////////////////////////////////////////////////////////////////////
	void DInputDeviceImpl::DeviceState(void* data, size_t size)
	{
		bool done;
		do
		{
			HRESULT hr = device_->GetDeviceState(size, data);
			if ((DIERR_INPUTLOST == hr) || (DIERR_NOTACQUIRED == hr))
			{
				this->Acquire();
				done = false;
			}
			else
			{
				done = true;
			}
		} while (!done);
	}
}
