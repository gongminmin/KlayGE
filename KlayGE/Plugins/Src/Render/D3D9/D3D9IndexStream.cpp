#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9IndexStream.hpp>

namespace KlayGE
{
	D3D9IndexStream::~D3D9IndexStream()
	{
	}


	void D3D9DynamicIndexStream::Assign(const void* src, size_t numIndices)
	{
		data_.resize(numIndices);
		Engine::MemoryInstance().Cpy(&data_[0], src, numIndices * sizeof(U16));
	}

	void D3D9DynamicIndexStream::D3D9Buffer(COMPtr<IDirect3DIndexBuffer9>& buffer, size_t& size) const
	{
		const size_t bufferSize(sizeof(U16) * this->NumIndices());

		if (size < bufferSize)
		{
			size = bufferSize;

			COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DIndexBuffer9* theBuffer;
			TIF(d3dDevice->CreateIndexBuffer(size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16, D3DPOOL_DEFAULT, &theBuffer, NULL));

			buffer = COMPtr<IDirect3DIndexBuffer9>(theBuffer);
		}

		void* dest;
		TIF(buffer->Lock(0, 0, &dest, D3DLOCK_DISCARD));
		Engine::MemoryInstance().Cpy(dest, &data_[0], size);
		buffer->Unlock();
	}


	D3D9StaticIndexStream::D3D9StaticIndexStream()
						: numIndices_(0)
	{
	}

	void D3D9StaticIndexStream::Assign(const void* src, size_t numIndices)
	{
		const size_t size(sizeof(U16) * numIndices);

		if (numIndices_ < numIndices)
		{
			numIndices_ = numIndices;

			COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DIndexBuffer9* buffer;
			TIF(d3dDevice->CreateIndexBuffer(size, D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16, D3DPOOL_DEFAULT, &buffer, NULL));

			buffer_ = COMPtr<IDirect3DIndexBuffer9>(buffer);
		}

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest, 0));
		Engine::MemoryInstance().Cpy(dest, src, size);
		buffer_->Unlock();
	}

	void D3D9StaticIndexStream::D3D9Buffer(COMPtr<IDirect3DIndexBuffer9>& buffer, size_t& size) const
	{
		buffer = buffer_;
		size = sizeof(U16) * this->NumIndices();
	}
}
