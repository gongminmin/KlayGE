// D3D11AdapterList.cpp
// KlayGE D3D11适配器列表 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/COMPtr.hpp>

#include <vector>
#include <system_error>
#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11Adapter.hpp>
#include <KlayGE/D3D11/D3D11AdapterList.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D11AdapterList::D3D11AdapterList()
						: current_adapter_(0)
	{
	}

	void D3D11AdapterList::Destroy()
	{
		adapters_.clear();
		current_adapter_ = 0;
	}

	// 获取系统显卡数目
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D11AdapterList::NumAdapter() const
	{
		return adapters_.size();
	}

	// 获取显卡
	/////////////////////////////////////////////////////////////////////////////////
	D3D11Adapter& D3D11AdapterList::Adapter(size_t index) const
	{
		BOOST_ASSERT(index < adapters_.size());

		return *adapters_[index];
	}

	// 获取当前显卡索引
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D11AdapterList::CurrentAdapterIndex() const
	{
		return current_adapter_;
	}

	// 设置当前显卡索引
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11AdapterList::CurrentAdapterIndex(uint32_t index)
	{
		current_adapter_ = index;
	}

	// 枚举系统显卡
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11AdapterList::Enumerate(IDXGIFactory1Ptr const & gi_factory)
	{
		// 枚举系统中的适配器
		UINT adapter_no = 0;
		IDXGIAdapter1* dxgi_adapter = nullptr;
		while (gi_factory->EnumAdapters1(adapter_no, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND)
		{
			if (dxgi_adapter != nullptr)
			{
				auto adapter = MakeUniquePtr<D3D11Adapter>(adapter_no, MakeCOMPtr(dxgi_adapter));
				adapter->Enumerate();
				adapters_.push_back(std::move(adapter));
			}

			++ adapter_no;
		}

		// 如果没有找到兼容的设备则抛出错误
		if (adapters_.empty())
		{
			TERRC(std::errc::function_not_supported);
		}
	}

	void D3D11AdapterList::Enumerate(IDXGIFactory6Ptr const & gi_factory)
	{
		UINT adapter_no = 0;
		IDXGIAdapter1* dxgi_adapter = nullptr;
		while (SUCCEEDED(gi_factory->EnumAdapterByGpuPreference(adapter_no, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_IDXGIAdapter1, reinterpret_cast<void**>(&dxgi_adapter))))
		{
			if (dxgi_adapter != nullptr)
			{
				auto adapter = MakeUniquePtr<D3D11Adapter>(adapter_no, MakeCOMPtr(dxgi_adapter));
				adapter->Enumerate();
				adapters_.push_back(std::move(adapter));
			}

			++ adapter_no;
		}

		if (adapters_.empty())
		{
			IDXGIFactory1* gif1;
			gi_factory->QueryInterface(IID_IDXGIFactory1, reinterpret_cast<void**>(&gif1));
			IDXGIFactory1Ptr gi_factory1 = MakeCOMPtr<IDXGIFactory1>(gif1);
			this->Enumerate(gi_factory1);
		}

		if (adapters_.empty())
		{
			TERRC(std::errc::function_not_supported);
		}
	}
}
