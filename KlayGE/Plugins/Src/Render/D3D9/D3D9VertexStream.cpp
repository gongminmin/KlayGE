#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Util.hpp>

#include <algorithm>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>

namespace KlayGE
{
	D3D9VertexStream::D3D9VertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t ElementsPerVertex, bool staticStream)
			: VertexStream(type, sizeElement, ElementsPerVertex),
				currentSize_(0), numVertices_(0), 
				staticStream_(staticStream)
	{
	}

	bool D3D9VertexStream::IsStatic() const
	{
		return staticStream_;
	}

	size_t D3D9VertexStream::NumVertices() const
	{
		return numVertices_;
	}

	void D3D9VertexStream::Assign(void const * src, size_t numVertices, size_t stride)
	{
		numVertices_ = numVertices;

		size_t const vertexSize(this->sizeElement() * this->ElementsPerVertex());
		size_t const size(vertexSize * numVertices);

		if (currentSize_ < size)
		{
			currentSize_ = size;

			boost::shared_ptr<IDirect3DDevice9> d3dDevice(static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DVertexBuffer9* theBuffer;
			TIF(d3dDevice->CreateVertexBuffer(static_cast<UINT>(size),
				D3DUSAGE_WRITEONLY | (this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC),
				0, D3DPOOL_DEFAULT, &theBuffer, NULL));

			buffer_ = MakeCOMPtr(theBuffer);
		}

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest,
			D3DLOCK_NOSYSLOCK | (this->IsStatic() ? 0 : D3DLOCK_DISCARD)));

		uint8_t* destPtr(static_cast<uint8_t*>(dest));
		uint8_t const * srcPtr(static_cast<uint8_t const *>(src));

		if (stride != 0)
		{
			for (size_t i = 0; i < numVertices; ++ i)
			{
				std::copy(srcPtr, srcPtr + vertexSize, destPtr);

				destPtr += vertexSize;
				srcPtr += vertexSize + stride;
			}
		}
		else
		{
			std::copy(srcPtr, srcPtr + size, destPtr);
		}

		buffer_->Unlock();
	}

	boost::shared_ptr<IDirect3DVertexBuffer9> D3D9VertexStream::D3D9Buffer() const
	{
		return buffer_;
	}
}
