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
#include <KFL/COMPtr.hpp>

#include <vector>
#include <system_error>
#include <boost/assert.hpp>

#include <KlayGE/D3D11/D3D11AdapterList.hpp>

namespace KlayGE
{
	// ���캯��
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

	// ��ȡϵͳ�Կ���Ŀ
	/////////////////////////////////////////////////////////////////////////////////
	size_t D3D11AdapterList::NumAdapter() const
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
	uint32_t D3D11AdapterList::CurrentAdapterIndex() const
	{
		return current_adapter_;
	}

	// ���õ�ǰ�Կ�����
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11AdapterList::CurrentAdapterIndex(uint32_t index)
	{
		current_adapter_ = index;
	}

	// ö��ϵͳ�Կ�
	/////////////////////////////////////////////////////////////////////////////////
	void D3D11AdapterList::Enumerate(IDXGIFactory1Ptr const & gi_factory)
	{
		// ö��ϵͳ�е�������
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

		// ���û���ҵ����ݵ��豸���׳�����
		if (adapters_.empty())
		{
			TERRC(std::errc::function_not_supported);
		}
	}
}
