#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9VertexBuffer.hpp>

namespace KlayGE
{
	void D3D9StaticVertexStream::Assign(const void* src, size_t vertexNum, size_t stride)
	{
		vertexNum_ = vertexNum;
		const size_t size(this->ElementSize() * this->ElementNum() * vertexNum_);

		COMPtr<IDirect3DDevice9> d3dDevice(reinterpret_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

		IDirect3DVertexBuffer9* theBuffer;
		TIF(d3dDevice->CreateVertexBuffer(size, D3DUSAGE_DYNAMIC, 0,
			D3DPOOL_DEFAULT, &theBuffer, NULL));

		buffer_ = COMPtr<IDirect3DVertexBuffer9>(theBuffer);

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest, D3DLOCK_DISCARD));
		Engine::MemoryInstance().Cpy(dest, src, size);
		buffer_->Unlock();
	}

	void D3D9StaticVertexStream::CopyTo(void* dest, size_t vertexNum) const
	{
		const size_t size(this->ElementSize() * this->ElementNum() * vertexNum);

		U8* src;
		TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_READONLY));
		Engine::MemoryInstance().Cpy(dest, src, size);
		buffer_->Unlock();
	}


	void D3D9StaticIndexStream::Assign(const void* src, size_t indexNum)
	{
		indexNum_ = indexNum;
		const size_t size(sizeof(U16) * indexNum_);

		COMPtr<IDirect3DDevice9> d3dDevice(reinterpret_cast<const D3D9RenderEngine&>(Engine::RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

		IDirect3DIndexBuffer9* buffer;
		TIF(d3dDevice->CreateIndexBuffer(size, D3DUSAGE_DYNAMIC, D3DFMT_INDEX16,
			D3DPOOL_DEFAULT, &buffer, NULL));

		buffer_ = COMPtr<IDirect3DIndexBuffer9>(buffer);

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest, D3DLOCK_DISCARD));
		Engine::MemoryInstance().Cpy(dest, src, size);
		buffer_->Unlock();
	}

	void D3D9StaticIndexStream::CopyTo(void* dest, size_t indexNum) const
	{
		void* src;
		TIF(buffer_->Lock(0, 0, &src, D3DLOCK_READONLY));
		Engine::MemoryInstance().Cpy(dest, src, sizeof(U16) * indexNum);
		buffer_->Unlock();
	}
}
