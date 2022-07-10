// D3D11AdapterList.hpp
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

#ifndef _D3D11ADAPTERLIST_HPP
#define _D3D11ADAPTERLIST_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/D3D11/D3D11Util.hpp>

namespace KlayGE
{
	class D3D11Adapter;

	class D3D11AdapterList final
	{
	public:
		D3D11AdapterList() noexcept;

		void Destroy();

		void Enumerate(IDXGIFactory2* gi_factory);
		void Enumerate(IDXGIFactory6* gi_factory);

		size_t NumAdapter() const noexcept;
		D3D11Adapter& Adapter(size_t index) const;

		uint32_t CurrentAdapterIndex() const noexcept;
		void CurrentAdapterIndex(uint32_t index) noexcept;

	private:
		std::vector<std::unique_ptr<D3D11Adapter>> adapters_;
		uint32_t current_adapter_{0};
	};

	typedef std::shared_ptr<D3D11AdapterList> D3D11AdapterListPtr;
}

#endif			// _D3D11ADAPTERLIST_HPP
