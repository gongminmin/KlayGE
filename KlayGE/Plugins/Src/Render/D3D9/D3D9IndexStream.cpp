// D3D9IndexStream.cpp
// KlayGE D3D9索引流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了CopyToMemory (2005.7.24)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9IndexStream.hpp>

#include <algorithm>

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
		BOOST_ASSERT(dynamic_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()) != NULL);

		numIndices_ = numIndices;

		size_t const size(sizeof(uint16_t) * numIndices);

		if (currentSize_ < size)
		{
			currentSize_ = size;

			D3D9RenderEngine const & renderEngine(static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			d3d_device_ = renderEngine.D3DDevice();

			IDirect3DIndexBuffer9* buffer;
			TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(size), 
				this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC,
				D3DFMT_INDEX16, D3DPOOL_DEFAULT, &buffer, NULL));
			buffer_ = MakeCOMPtr(buffer);
		}

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest, D3DLOCK_NOSYSLOCK | (this->IsStatic() ? 0 : D3DLOCK_DISCARD)));
		std::copy(static_cast<uint8_t const *>(src), static_cast<uint8_t const *>(src) + size, static_cast<uint8_t*>(dest));
		buffer_->Unlock();
	}

	void D3D9IndexStream::CopyToMemory(void* data)
	{
		size_t const size(sizeof(uint16_t) * numIndices_);

		void* src;
		TIF(buffer_->Lock(0, 0, &src, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		uint8_t* destPtr(static_cast<uint8_t*>(data));
		uint8_t const * srcPtr(static_cast<uint8_t const *>(src));

		std::copy(srcPtr, srcPtr + size, destPtr);

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

	void D3D9IndexStream::DoOnLostDevice()
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()) != NULL);

		D3D9RenderEngine const & renderEngine(static_cast<D3D9RenderEngine const &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();

		size_t const size(sizeof(uint16_t) * numIndices_);

		IDirect3DIndexBuffer9* temp;
		TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(size), 0,
				D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &temp, NULL));
		boost::shared_ptr<IDirect3DIndexBuffer9> buffer = MakeCOMPtr(temp);

		uint8_t* src;
		uint8_t* dest;
		TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK));
		TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK));

		std::copy(src, src + size, dest);

		buffer->Unlock();
		buffer_->Unlock();

		buffer_ = buffer;
		currentSize_ = size;
	}

	void D3D9IndexStream::DoOnResetDevice()
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()) != NULL);

		D3D9RenderEngine& renderEngine(static_cast<D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		boost::shared_ptr<IDirect3DDevice9> d3dDevice(renderEngine.D3DDevice());
		size_t const size(sizeof(uint16_t) * numIndices_);

		IDirect3DIndexBuffer9* temp;
		TIF(d3dDevice->CreateIndexBuffer(static_cast<UINT>(size), 
				this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC,
				D3DFMT_INDEX16, D3DPOOL_DEFAULT, &temp, NULL));
		boost::shared_ptr<IDirect3DIndexBuffer9> buffer = MakeCOMPtr(temp);

		uint8_t* src;
		uint8_t* dest;
		TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK));
		TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK));

		std::copy(src, src + size, dest);

		buffer->Unlock();
		buffer_->Unlock();

		buffer_ = buffer;
		currentSize_ = size;
	}
}
