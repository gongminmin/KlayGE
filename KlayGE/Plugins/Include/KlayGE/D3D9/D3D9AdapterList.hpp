#ifndef _D3D9ADAPTERLIST_HPP
#define _D3D9ADAPTERLIST_HPP

#include <d3d9.h>

#include <KlayGE/COMPtr.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/alloc.hpp>
#include <KlayGE/D3D9/D3D9Adapter.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	class D3D9AdapterList
	{
	public:
		D3D9AdapterList();

		void Enumerate(const COMPtr<IDirect3D9>& d3d);

		size_t NumAdapter() const;
		const D3D9Adapter& Adapter(size_t index) const;

		U32 CurrentAdapterIndex() const;
		void CurrentAdapterIndex(U32 index);

	private:
		std::vector<D3D9Adapter, alloc<D3D9Adapter> > adapters_;
		U32			currentAdapter_;
	};

	typedef SharedPtr<D3D9AdapterList> D3D9AdapterListPtr;
}

#endif			// _D3D9ADAPTERLIST_HPP