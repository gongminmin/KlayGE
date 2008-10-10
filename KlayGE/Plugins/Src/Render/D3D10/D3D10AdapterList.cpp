// D3D10AdapterList.cpp
// KlayGE D3D10适配器列表 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/COMPtr.hpp>

#include <vector>
#include <boost/assert.hpp>

#include <KlayGE/D3D10/D3D10AdapterList.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D10AdapterList::D3D10AdapterList()
						: current_adapter_(0)
	{
	}

	// 获取系统显卡数目
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D10AdapterList::NumAdapter() const
	{
		return adapters_.size();
	}

	// 获取显卡
	/////////////////////////////////////////////////////////////////////////////////
	D3D10AdapterPtr const & D3D10AdapterList::Adapter(size_t index) const
	{
		BOOST_ASSERT(index < adapters_.size());

		return adapters_[index];
	}

	// 获取当前显卡索引
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D10AdapterList::CurrentAdapterIndex() const
	{
		return current_adapter_;
	}

	// 设置当前显卡索引
	/////////////////////////////////////////////////////////////////////////////////
	void D3D10AdapterList::CurrentAdapterIndex(uint32_t index)
	{
		current_adapter_ = index;
	}

	// 枚举系统显卡
	/////////////////////////////////////////////////////////////////////////////////
	void D3D10AdapterList::Enumerate(IDXGIFactoryPtr const & gi_factory)
	{
		// 枚举系统中的适配器
		UINT adapter_no = 0;
		IDXGIAdapter* dxgi_adapter = NULL;
		while (gi_factory->EnumAdapters(adapter_no, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND)
		{
			if (dxgi_adapter != NULL)
			{
				DXGI_ADAPTER_DESC ad;
				dxgi_adapter->GetDesc(&ad);

				if (0 == wcscmp(ad.Description, L"NVIDIA PerfHUD"))
				{
					current_adapter_ = adapter_no;
				}

				D3D10AdapterPtr adapter(new D3D10Adapter(adapter_no, MakeCOMPtr(dxgi_adapter)));
				adapters_.push_back(adapter);
			}

			++ adapter_no;
		}

		// 如果没有找到兼容的设备则抛出错误
		if (adapters_.empty())
		{
			THR(boost::system::posix_error::not_supported);
		}
	}
}
