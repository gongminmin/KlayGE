#ifndef _D3D9INDEXSTREAM_HPP
#define _D3D9INDEXSTREAM_HPP

#include <KlayGE/COMPtr.hpp>

#include <d3d9.h>

#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
	class D3D9IndexStream : public IndexStream
	{
	public:
		D3D9IndexStream(bool staticStream);

		bool IsStatic() const
			{ return staticStream_; }

		void Assign(const void* src, size_t numIndices);
		
		size_t NumIndices() const
			{ return numIndices_; }

		COMPtr<IDirect3DIndexBuffer9> D3D9Buffer() const
			{ return buffer_; }

	protected:
		COMPtr<IDirect3DIndexBuffer9> buffer_;
		size_t numIndices_;

		bool staticStream_;
	};
}

#endif			// _D3D9INDEXSTREAM_HPP
