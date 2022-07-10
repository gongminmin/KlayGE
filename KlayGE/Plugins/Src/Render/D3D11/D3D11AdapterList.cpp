// D3D11AdapterList.cpp
// KlayGE D3D11�������б� ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/com_ptr.hpp>

#include <vector>
#include <system_error>
#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11Adapter.hpp>
#include <KlayGE/D3D11/D3D11AdapterList.hpp>

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	D3D11AdapterList::D3D11AdapterList() noexcept = default;

	void D3D11AdapterList::Destroy()
	{
		adapters_.clear();
		current_adapter_ = 0;
	}

	// ��ȡϵͳ�Կ���Ŀ
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D11AdapterList::NumAdapter() const noexcept
	{
		return adapters_.size();
	}

	// ��ȡ�Կ�
	/////////////////////////////////////////////////////////////////////////////////
	D3D11Adapter& D3D11AdapterList::Adapter(size_t index) const
	{
		BOOST_ASSERT(index < adapters_.size());

		return *adapters_[index];
	}

	// ��ȡ��ǰ�Կ�����
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t D3D11AdapterList::CurrentAdapterIndex() const noexcept
	{
		return current_adapter_;
	}

	// ���õ�ǰ�Կ�����
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11AdapterList::CurrentAdapterIndex(uint32_t index) noexcept
	{
		current_adapter_ = index;
	}

	// ö��ϵͳ�Կ�
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11AdapterList::Enumerate(IDXGIFactory2* gi_factory)
	{
		// ö��ϵͳ�е�������
		UINT adapter_no = 0;
		com_ptr<IDXGIAdapter1> dxgi_adapter;
		while (gi_factory->EnumAdapters1(adapter_no, dxgi_adapter.release_and_put()) != DXGI_ERROR_NOT_FOUND)
		{
			if (dxgi_adapter != nullptr)
			{
				auto& adapter = *adapters_.emplace_back(MakeUniquePtr<D3D11Adapter>(adapter_no, dxgi_adapter.as<IDXGIAdapter2>().get()));
				adapter.Enumerate();
			}

			++ adapter_no;
		}

		// ���û���ҵ����ݵ��豸���׳�����
		if (adapters_.empty())
		{
			TERRC(std::errc::function_not_supported);
		}
	}

	void D3D11AdapterList::Enumerate(IDXGIFactory6* gi_factory)
	{
		UINT adapter_no = 0;
		com_ptr<IDXGIAdapter2> dxgi_adapter;
		while (SUCCEEDED(gi_factory->EnumAdapterByGpuPreference(adapter_no, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			UuidOf<IDXGIAdapter2>(), dxgi_adapter.release_and_put_void())))
		{
			if (dxgi_adapter != nullptr)
			{
				auto adapter = MakeUniquePtr<D3D11Adapter>(adapter_no, dxgi_adapter.get());
				adapter->Enumerate();
				adapters_.push_back(std::move(adapter));
			}

			++ adapter_no;
		}

		if (adapters_.empty())
		{
			auto gi_factory2 = com_ptr<IDXGIFactory6>(gi_factory).as<IDXGIFactory2>();
			this->Enumerate(gi_factory2.get());
		}

		if (adapters_.empty())
		{
			TERRC(std::errc::function_not_supported);
		}
	}
}
