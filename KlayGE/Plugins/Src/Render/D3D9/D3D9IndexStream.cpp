#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9IndexStream.hpp>

#include <cstring>

#include <boost/smart_ptr.hpp>
#include <boost/mem_fn.hpp>

namespace KlayGE
{
	D3D9IndexStream::D3D9IndexStream(bool staticStream)
						: staticStream_(staticStream),
							currentSize_(0), numIndices_(0)
	{
	}

	void D3D9IndexStream::Assign(void const * src, size_t numIndices)
	{
		numIndices_ = numIndices;

		size_t const size(sizeof(U16) * numIndices);

		if (currentSize_ < size)
		{
			currentSize_ = size;

			boost::shared_ptr<IDirect3DDevice9> d3dDevice(static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()).D3DDevice());

			IDirect3DIndexBuffer9* buffer;
			TIF(d3dDevice->CreateIndexBuffer(static_cast<::UINT>(size), 
				D3DUSAGE_WRITEONLY | (this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC),
				D3DFMT_INDEX16, D3DPOOL_DEFAULT, &buffer, NULL));

			buffer_ = MakeCOMPtr(buffer);
		}

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest, D3DLOCK_NOSYSLOCK | (this->IsStatic() ? 0 : D3DLOCK_DISCARD)));
		std::memcpy(dest, src, size);
		buffer_->Unlock();
	}

	size_t D3D9IndexStream::NumIndices() const
	{
		return numIndices_;
	}

	boost::shared_ptr<IDirect3DIndexBuffer9> D3D9IndexStream::D3D9Buffer() const
	{
		return buffer_;
	}

	bool D3D9IndexStream::IsStatic() const
	{
		return staticStream_;
	}
}
