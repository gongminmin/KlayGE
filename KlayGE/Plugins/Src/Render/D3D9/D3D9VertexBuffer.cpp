// D3D9VertexBuffer.cpp
// KlayGE D3D9顶点缓冲区类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.2.0
// 初次建立 (2006.1.9)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <algorithm>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>

namespace KlayGE
{
	D3D9VertexBuffer::D3D9VertexBuffer(BufferUsage usage)
			: GraphicsBuffer(usage)
	{
	}

	void D3D9VertexBuffer::DoResize()
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

	void* D3D9VertexBuffer::Map(BufferAccess ba)
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

	void D3D9VertexBuffer::Unmap()
	{
		BOOST_ASSERT(buffer_);

		buffer_->Unlock();
	}

	RenderViewPtr D3D9VertexBuffer::CreateRenderView(uint32_t width, uint32_t height)
	{
		return RenderViewPtr(new D3D9GraphicsBufferRenderView(*this, width, height, PF_ABGR32F));
	}

	boost::shared_ptr<IDirect3DVertexBuffer9> D3D9VertexBuffer::D3D9Buffer() const
	{
		return buffer_;
	}

	void D3D9VertexBuffer::DoOnLostDevice()
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
	
	void D3D9VertexBuffer::DoOnResetDevice()
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
