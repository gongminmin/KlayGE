#ifndef _D3D9VERTEXSTREAM_HPP
#define _D3D9VERTEXSTREAM_HPP

#include <boost/smart_ptr.hpp>

#include <d3d9.h>

#include <KlayGE/RenderBuffer.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	class D3D9VertexStream : public VertexStream
	{
	public:
		D3D9VertexStream(VertexStreamType type, U8 sizeElement, U8 ElementsPerVertex, bool staticStream);

		bool IsStatic() const;

		void Assign(void const * src, size_t numVertices, size_t stride = 0);

		boost::shared_ptr<IDirect3DVertexBuffer9> D3D9Buffer() const;
		size_t NumVertices() const;

	private:
		boost::shared_ptr<IDirect3DVertexBuffer9> buffer_;
		size_t currentSize_;

		size_t numVertices_;

		bool staticStream_;
	};
}

#endif			// _D3D9VERTEXSTREAM_HPP
