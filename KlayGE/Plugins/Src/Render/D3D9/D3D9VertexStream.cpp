// D3D9VertexStream.cpp
// KlayGE D3D9顶点流类 实现文件
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
	D3D9VertexStream::D3D9VertexStream(BufferUsage usage)
			: GraphicsBuffer(usage)
	{
	}

	void D3D9VertexStream::DoCreate()
	{
		BOOST_ASSERT(size_in_byte_ != 0);

		uint32_t vb_size = 0;
		if (buffer_)
		{
			D3DVERTEXBUFFER_DESC desc;
			buffer_->GetDesc(&desc);
			vb_size = desc.Size;
		}
		if (this->Size() > vb_size)
		{
			D3D9RenderEngine const & renderEngine(*checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			d3d_device_ = renderEngine.D3DDevice();

			IDirect3DVertexBuffer9* buffer;
			TIF(d3d_device_->CreateVertexBuffer(static_cast<UINT>(this->Size()),
					(BU_Dynamic == usage_) ? D3DUSAGE_DYNAMIC : 0,
					0, D3DPOOL_DEFAULT, &buffer, NULL));
			buffer_ = MakeCOMPtr(buffer);
		}
	}

	void* D3D9VertexStream::Map(BufferAccess ba)
	{
		BOOST_ASSERT(buffer_);

		uint32_t flags = 0;
		switch (ba)
		{
		case BA_Read_Only:
			break;

		case BA_Write_Only:
			if (BU_Dynamic == usage_)
			{
				flags = D3DLOCK_DISCARD;
			}
			break;

		case BA_Read_Write:
			break;
		}

		void* ret;
		TIF(buffer_->Lock(0, 0, &ret, D3DLOCK_NOSYSLOCK | flags));
		return ret;
	}

	void D3D9VertexStream::Unmap()
	{
		BOOST_ASSERT(buffer_);

		buffer_->Unlock();
	}

	boost::shared_ptr<IDirect3DVertexBuffer9> D3D9VertexStream::D3D9Buffer() const
	{
		return buffer_;
	}

	void D3D9VertexStream::DoOnLostDevice()
	{
		IDirect3DVertexBuffer9* temp;
		TIF(d3d_device_->CreateVertexBuffer(static_cast<UINT>(this->Size()),
			D3DUSAGE_DYNAMIC, 0, D3DPOOL_SYSTEMMEM, &temp, NULL));
		boost::shared_ptr<IDirect3DVertexBuffer9> buffer = MakeCOMPtr(temp);

		uint8_t* src;
		uint8_t* dest;
		TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));
		TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK | ((BU_Dynamic == usage_) ? D3DLOCK_DISCARD : 0)));

		std::copy(src, src + this->Size(), dest);

		buffer->Unlock();
		buffer_->Unlock();

		buffer_ = buffer;
	}
	
	void D3D9VertexStream::DoOnResetDevice()
	{
		D3D9RenderEngine const & renderEngine(*checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();

		IDirect3DVertexBuffer9* temp;
		TIF(d3d_device_->CreateVertexBuffer(static_cast<UINT>(this->Size()),
				(BU_Dynamic == usage_) ? D3DUSAGE_DYNAMIC : 0,
				0, D3DPOOL_DEFAULT, &temp, NULL));
		boost::shared_ptr<IDirect3DVertexBuffer9> buffer = MakeCOMPtr(temp);

		uint8_t* src;
		uint8_t* dest;
		TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));
		TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK | ((BU_Dynamic == usage_) ? D3DLOCK_DISCARD : 0)));

		std::copy(src, src + this->Size(), dest);

		buffer->Unlock();
		buffer_->Unlock();

		buffer_ = buffer;
	}
}
