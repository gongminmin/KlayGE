#ifndef _D3D9IBCONVERTER_HPP
#define _D3D9IBCONVERTER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/COMPtr.hpp>

#include <vector>
#include <d3d9.h>

namespace KlayGE
{
	class D3D9IBConverter
	{
	public:
		COMPtr<IDirect3DIndexBuffer9> Update(const RenderBuffer& rb);

	private:
		std::pair<COMPtr<IDirect3DIndexBuffer9>, size_t> indexPool_;
	};
}

#endif			// _D3D9VBCONVERTER_HPP