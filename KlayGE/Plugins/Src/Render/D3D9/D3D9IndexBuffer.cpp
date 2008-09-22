// D3D9IndexBuffer.cpp
// KlayGE D3D9索引缓冲区类 实现文件
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
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>

namespace KlayGE
{
	D3D9IndexBuffer::D3D9IndexBuffer(BufferUsage usage, uint32_t access_hint)
						: D3D9GraphicsBuffer(usage, access_hint),
							format_(EF_R16)
	{
	}

	void D3D9IndexBuffer::SwitchFormat(ElementFormat format)
	{
		BOOST_ASSERT(d3d_device_);
		BOOST_ASSERT((EF_R16 == format) || (EF_R32 == format));

		if (format_ != format)
		{
			uint32_t usage = 0;
			if ((access_hint_ & EAH_CPU_Write) && !(access_hint_ & EAH_CPU_Read))
			{
				usage = D3DUSAGE_WRITEONLY;
			}

			ElementFormat old_format = format_;
			format_ = format;

			if (EF_R16 == old_format)
			{
				IDirect3DIndexBuffer9* buffer;
				TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(this->Size() * (sizeof(uint32_t) / sizeof(uint16_t))), 
					usage, (EF_R32 == format_) ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_MANAGED, &buffer, NULL));

				uint16_t* src;
				uint32_t* dest;
				TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));
				TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK));

				for (size_t i = 0; i < this->Size() / sizeof(uint16_t); ++ i)
				{
					src[i] = static_cast<uint16_t>(dest[i] & 0xFFFF);
				}

				buffer->Unlock();
				buffer_->Unlock();

				buffer_ = MakeCOMPtr(buffer);
			}
			else
			{
				BOOST_ASSERT(EF_R32 == old_format);

				IDirect3DIndexBuffer9* buffer;
				TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(this->Size() / (sizeof(uint32_t) / sizeof(uint16_t))), 
					usage, (EF_R32 == format_) ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_MANAGED, &buffer, NULL));

				uint32_t* src;
				uint16_t* dest;
				TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));
				TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK));

				for (size_t i = 0; i < this->Size() / sizeof(uint32_t); ++ i)
				{
					src[i] = dest[i];
				}

				buffer->Unlock();
				buffer_->Unlock();

				buffer_ = MakeCOMPtr(buffer);
			}
		}
	}

	void D3D9IndexBuffer::DoResize()
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

			IDirect3DIndexBuffer9* buffer;
			TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(this->Size()), 
				usage, (EF_R32 == format_) ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_MANAGED, &buffer, NULL));
			buffer_ = MakeCOMPtr(buffer);
			hw_buf_size_ = this->Size();
		}
	}

	void* D3D9IndexBuffer::Map(BufferAccess ba)
	{
		BOOST_ASSERT(buffer_);

		uint32_t flags = 0;
		switch (ba)
		{
		case BA_Read_Only:
			flags = D3DLOCK_READONLY;
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

	void D3D9IndexBuffer::Unmap()
	{
		BOOST_ASSERT(buffer_);

		buffer_->Unlock();
	}

	void D3D9IndexBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		GraphicsBuffer::Mapper lhs_mapper(*this, BA_Read_Only);
		GraphicsBuffer::Mapper rhs_mapper(rhs, BA_Write_Only);
		std::copy(lhs_mapper.Pointer<uint8_t>(), lhs_mapper.Pointer<uint8_t>() + size_in_byte_,
			rhs_mapper.Pointer<uint8_t>());
	}

	ID3D9IndexBufferPtr D3D9IndexBuffer::D3D9Buffer() const
	{
		return buffer_;
	}

	void D3D9IndexBuffer::DoOnLostDevice()
	{
	}

	void D3D9IndexBuffer::DoOnResetDevice()
	{
	}
}
