#ifndef _D3D9IBCONVERTER_HPP
#define _D3D9IBCONVERTER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/COMPtr.hpp>

#include <vector>
#include <d3d9.h>

namespace KlayGE
{
	struct HardwareIndexBuffer
	{
		HardwareIndexBuffer()
			: count(0)
			{ }

		HardwareIndexBuffer(const HardwareIndexBuffer& rhs)
			: buffer(rhs.buffer),
				count(rhs.count)
			{ }

		COMPtr<IDirect3DIndexBuffer9> buffer;
		U32 count;
	};

	class D3D9IBConverter
	{
	public:
		COMPtr<IDirect3DIndexBuffer9> Update(const VertexBuffer& vb);

	private:
		HardwareIndexBuffer indicies_;
	};
}

#endif			// _D3D9VBCONVERTER_HPP