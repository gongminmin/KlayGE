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
	D3D9IndexStream::D3D9IndexStream(bool staticStream)
						: numIndices_(0),
							staticStream_(staticStream)
	{
	}

	void D3D9IndexStream::Assign(const void* src, size_t numIndices)
	{
		const size_t size(sizeof(U16) * numIndices);

		if (numIndices_ < numIndices)
		{
			numIndices_ = numIndices;

			COMPtr<IDirect3DDevice9> d3dDevice(static_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DIndexBuffer9* buffer;
			TIF(d3dDevice->CreateIndexBuffer(size,
				(this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC) | D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16, D3DPOOL_DEFAULT, &buffer, NULL));

			buffer_ = COMPtr<IDirect3DIndexBuffer9>(buffer);
		}

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest,
			this->IsStatic() ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD));
		Engine::MemoryInstance().Cpy(dest, src, size);
		buffer_->Unlock();
	}
}
