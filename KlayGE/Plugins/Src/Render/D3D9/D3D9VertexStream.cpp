#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>

namespace KlayGE
{
	D3D9VertexStream::D3D9VertexStream(VertexStreamType type, U8 elementSize, U8 elementNum, bool staticStream)
			: VertexStream(type, elementSize, elementNum),
				staticStream_(staticStream),
				numVertices_(0)
	{
	}

	void D3D9VertexStream::Assign(const void* src, size_t numVertices, size_t stride)
	{
		const size_t vertexSize(this->ElementSize() * this->ElementNum());
		const size_t size(vertexSize * numVertices);

		if (numVertices_ < numVertices)
		{
			numVertices_ = numVertices;

			COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DVertexBuffer9* theBuffer;
			TIF(d3dDevice->CreateVertexBuffer(size,
				(this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC) | D3DUSAGE_WRITEONLY,
				0, D3DPOOL_DEFAULT, &theBuffer, NULL));

			buffer_ = COMPtr<IDirect3DVertexBuffer9>(theBuffer);
		}

		void* dest;
 		TIF(buffer_->Lock(0, 0, &dest,
			this->IsStatic() ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD));
		if (stride != 0)
		{
			U8* destPtr(static_cast<U8*>(dest));
			const U8* srcPtr(static_cast<const U8*>(static_cast<const void*>(src)));
			for (size_t i = 0; i < numVertices; ++ i)
			{
				memcpy(destPtr, srcPtr, vertexSize);

				destPtr += vertexSize;
				srcPtr += vertexSize + stride;
			}
		}
		else
		{
			Engine::MemoryInstance().Cpy(dest, src, size);
		}
		buffer_->Unlock();
	}
}
