#ifndef _D3D9VERTEXSTREAM_HPP
#define _D3D9VERTEXSTREAM_HPP

#include <KlayGE/COMPtr.hpp>

#include <d3d9.h>

#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
	class D3D9VertexStream : public VertexStream
	{
	public:
		D3D9VertexStream(VertexStreamType type, U8 elementSize, U8 elementNum, bool staticStream);

		bool IsStatic() const
			{ return staticStream_; }
		size_t NumVertices() const
			{ return numVertices_; }

		void Assign(const void* src, size_t numVertices, size_t stride = 0);

		COMPtr<IDirect3DVertexBuffer9> D3D9Buffer() const
			{ return buffer_; }

	protected:
		COMPtr<IDirect3DVertexBuffer9> buffer_;
		size_t numVertices_;

		bool staticStream_;
	};
}

#endif			// _D3D9VERTEXSTREAM_HPP
