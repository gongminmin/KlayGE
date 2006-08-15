#ifndef _D3D9ADAPTERLIST_HPP
#define _D3D9ADAPTERLIST_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D9
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <boost/smart_ptr.hpp>
#include <KlayGE/D3D9/D3D9Adapter.hpp>

#include <d3d9.h>

namespace KlayGE
{
	class D3D9AdapterList
	{
	public:
		D3D9AdapterList();

		void Enumerate(ID3D9Ptr const & d3d);

		size_t NumAdapter() const;
		D3D9Adapter const & Adapter(size_t index) const;

		uint32_t CurrentAdapterIndex() const;
		void CurrentAdapterIndex(uint32_t index);

	private:
		std::vector<D3D9Adapter> adapters_;
		uint32_t			currentAdapter_;
	};

	typedef boost::shared_ptr<D3D9AdapterList> D3D9AdapterListPtr;
}

#endif			// _D3D9ADAPTERLIST_HPP