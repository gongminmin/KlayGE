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
	D3D9VertexStream::D3D9VertexStream(VertexStreamType type, U8 elementSize, U8 elementNum)
			: VertexStream(type, elementSize, elementNum)
	{
	}

	D3D9VertexStream::~D3D9VertexStream()
	{
	}


	bool D3D9DynamicVertexStream::IsStatic() const
	{
		return false;
	}

	D3D9DynamicVertexStream::D3D9DynamicVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum)
		: D3D9VertexStream(type, elementSize, elementNum)
	{
	}

	size_t D3D9DynamicVertexStream::NumVertices() const
	{
		return data_.size() / this->ElementSize() / this->ElementNum();
	}

	void D3D9DynamicVertexStream::Assign(const void* src, size_t numVertices, size_t stride)
	{
		const size_t vertexSize(this->ElementSize() * this->ElementNum());
		const size_t size(vertexSize * numVertices);
		data_.resize(size);

        if (stride != 0)
		{
			U8* destPtr(&data_[0]);
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
			Engine::MemoryInstance().Cpy(&data_[0], src, size);
		}
	}

	void D3D9DynamicVertexStream::D3D9Buffer(COMPtr<IDirect3DVertexBuffer9>& buffer, size_t& size) const
	{
		const size_t vertexSize(this->ElementSize() * this->ElementNum());
		const size_t bufferSize(vertexSize * this->NumVertices());

		if (size < bufferSize)
		{
			size = bufferSize;

			COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DVertexBuffer9* theBuffer;
			TIF(d3dDevice->CreateVertexBuffer(size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
				0, D3DPOOL_DEFAULT, &theBuffer, NULL));

			buffer = COMPtr<IDirect3DVertexBuffer9>(theBuffer);
		}

		void* dest;
 		TIF(buffer->Lock(0, 0, &dest, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD));
		Engine::MemoryInstance().Cpy(dest, &data_[0], size);
		buffer->Unlock();
	}


	D3D9StaticVertexStream::D3D9StaticVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum)
		: D3D9VertexStream(type, elementSize, elementNum),
			numVertices_(0)
	{
	}

	bool D3D9StaticVertexStream::IsStatic() const
	{
		return true;
	}

	size_t D3D9StaticVertexStream::NumVertices() const
	{
		return numVertices_;
	}

	void D3D9StaticVertexStream::Assign(const void* src, size_t numVertices, size_t stride)
	{
		const size_t vertexSize(this->ElementSize() * this->ElementNum());
		const size_t size(vertexSize * numVertices);

		if (numVertices_ * vertexSize < size)
		{
			numVertices_ = numVertices;

			COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DVertexBuffer9* theBuffer;
			TIF(d3dDevice->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY,
				0, D3DPOOL_DEFAULT, &theBuffer, NULL));

			buffer_ = COMPtr<IDirect3DVertexBuffer9>(theBuffer);
		}

		void* dest;
 		TIF(buffer_->Lock(0, 0, &dest, D3DLOCK_NOSYSLOCK));
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

	void D3D9StaticVertexStream::D3D9Buffer(COMPtr<IDirect3DVertexBuffer9>& buffer, size_t& size) const
	{
		buffer = buffer_;
		size = this->ElementSize() * this->ElementNum() * this->NumVertices();
	}
}
