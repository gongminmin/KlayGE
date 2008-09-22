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
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <algorithm>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9RenderView.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>

namespace KlayGE
{
	D3D9VertexBuffer::D3D9VertexBuffer(BufferUsage usage, uint32_t access_hint)
			: D3D9GraphicsBuffer(usage, access_hint)
	{
	}

	void D3D9VertexBuffer::DoResize()
	{
		BOOST_ASSERT(size_in_byte_ != 0);

		if (this->Size() > hw_buf_size_)
		{
			uint32_t usage = 0;
			if ((access_hint_ & EAH_CPU_Write) && !(access_hint_ & EAH_CPU_Read))
			{
				usage = D3DUSAGE_WRITEONLY;
			}

			D3D9RenderEngine const & renderEngine(*checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			d3d_device_ = renderEngine.D3DDevice();

			IDirect3DVertexBuffer9* buffer;
			TIF(d3d_device_->CreateVertexBuffer(static_cast<UINT>(this->Size()),
					usage, 0, D3DPOOL_MANAGED, &buffer, NULL));
			buffer_ = MakeCOMPtr(buffer);
			hw_buf_size_ = this->Size();
		}
	}

	void* D3D9VertexBuffer::Map(BufferAccess ba)
	{
		BOOST_ASSERT(buffer_);

		uint32_t flags = 0;
		switch (ba)
		{
		case BA_Read_Only:
			flags |= D3DLOCK_READONLY;
			break;

		case BA_Write_Only:
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

	void D3D9VertexBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		GraphicsBuffer::Mapper lhs_mapper(*this, BA_Read_Only);
		GraphicsBuffer::Mapper rhs_mapper(rhs, BA_Write_Only);
		std::copy(lhs_mapper.Pointer<uint8_t>(), lhs_mapper.Pointer<uint8_t>() + size_in_byte_,
			rhs_mapper.Pointer<uint8_t>());
	}

	ID3D9VertexBufferPtr D3D9VertexBuffer::D3D9Buffer() const
	{
		return buffer_;
	}

	void D3D9VertexBuffer::DoOnLostDevice()
	{
	}
	
	void D3D9VertexBuffer::DoOnResetDevice()
	{
	}
}
