#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>

namespace KlayGE
{
	D3D9VertexStream::D3D9VertexStream(VertexStreamType type, U8 sizeElement, U8 ElementsPerVertex, bool staticStream)
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

	void D3D9VertexStream::Assign(const void* src, size_t numVertices, size_t stride)
	{
		numVertices_ = numVertices;

		const size_t vertexSize(this->sizeElement() * this->ElementsPerVertex());
		const size_t size(vertexSize * numVertices);

		if (currentSize_ < size)
		{
			currentSize_ = size;

			COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DVertexBuffer9* theBuffer;
			TIF(d3dDevice->CreateVertexBuffer(static_cast<::UINT>(size),
				D3DUSAGE_WRITEONLY | (this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC),
				0, D3DPOOL_DEFAULT, &theBuffer, NULL));

			buffer_ = COMPtr<IDirect3DVertexBuffer9>(theBuffer);
		}

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest,
			D3DLOCK_NOSYSLOCK | (this->IsStatic() ? 0 : D3DLOCK_DISCARD)));
		if (stride != 0)
		{
			U8* destPtr(static_cast<U8*>(dest));
			const U8* srcPtr(static_cast<const U8*>(static_cast<const void*>(src)));
			for (size_t i = 0; i < numVertices; ++ i)
			{
				MemoryLib::Copy(destPtr, srcPtr, vertexSize);

				destPtr += vertexSize;
				srcPtr += vertexSize + stride;
			}
		}
		else
		{
			MemoryLib::Copy(dest, src, size);
		}
		buffer_->Unlock();
	}

	COMPtr<IDirect3DVertexBuffer9> D3D9VertexStream::D3D9Buffer() const
	{
		return buffer_;
	}
}
