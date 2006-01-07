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
	D3D9IndexStream::D3D9IndexStream(BufferUsage usage)
						: IndexStream(usage),
							format_(IF_Index16)
	{
	}

	void D3D9IndexStream::SwitchFormat(IndexFormat format)
	{
		BOOST_ASSERT(d3d_device_);

		if (format_ != format)
		{
			IndexFormat old_format = format_;
			format_ = format;

			if (IF_Index16 == old_format)
			{
				IDirect3DIndexBuffer9* buffer;
				TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(this->Size() * (sizeof(uint32_t) / sizeof(uint16_t))), 
					(BU_Dynamic == usage_) ? D3DUSAGE_DYNAMIC : 0,
					(IF_Index32 == format_) ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_DEFAULT, &buffer, NULL));

				uint16_t* src;
				uint32_t* dest;
				TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));
				TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK | ((BU_Dynamic == usage_) ? D3DLOCK_DISCARD : 0)));

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
				BOOST_ASSERT(IF_Index32 == old_format);

				IDirect3DIndexBuffer9* buffer;
				TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(this->Size() / (sizeof(uint32_t) / sizeof(uint16_t))), 
					(BU_Dynamic == usage_) ? D3DUSAGE_DYNAMIC : 0,
					(IF_Index32 == format_) ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_DEFAULT, &buffer, NULL));

				uint32_t* src;
				uint16_t* dest;
				TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));
				TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK | ((BU_Dynamic == usage_) ? D3DLOCK_DISCARD : 0)));

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

	void D3D9IndexStream::DoCreate()
	{
		BOOST_ASSERT(size_in_byte_ != 0);

		uint32_t ib_size = 0;
		if (buffer_)
		{
			D3DINDEXBUFFER_DESC desc;
			buffer_->GetDesc(&desc);
			ib_size = desc.Size;
		}
		if (this->Size() > ib_size)
		{
			D3D9RenderEngine const & renderEngine(*checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			d3d_device_ = renderEngine.D3DDevice();

			IDirect3DIndexBuffer9* buffer;
			TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(this->Size()), 
				(BU_Dynamic == usage_) ? D3DUSAGE_DYNAMIC : 0,
				(IF_Index32 == format_) ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_DEFAULT, &buffer, NULL));
			buffer_ = MakeCOMPtr(buffer);
		}
	}

	void* D3D9IndexStream::Map(BufferAccess ba)
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

	void D3D9IndexStream::Unmap()
	{
		BOOST_ASSERT(buffer_);

		buffer_->Unlock();
	}

	boost::shared_ptr<IDirect3DIndexBuffer9> D3D9IndexStream::D3D9Buffer() const
	{
		return buffer_;
	}

	void D3D9IndexStream::DoOnLostDevice()
	{
		D3D9RenderEngine const & renderEngine(*checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();

		IDirect3DIndexBuffer9* temp;
		TIF(d3d_device_->CreateIndexBuffer(static_cast<UINT>(this->Size()), D3DUSAGE_DYNAMIC,
			(IF_Index32 == format_) ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &temp, NULL));
		boost::shared_ptr<IDirect3DIndexBuffer9> buffer = MakeCOMPtr(temp);

		uint8_t* src;
		uint8_t* dest;
		TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));
		TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK | ((BU_Dynamic == usage_) ? D3DLOCK_DISCARD : 0)));

		std::copy(src, src + this->Size(), dest);

		buffer->Unlock();
		buffer_->Unlock();

		buffer_ = buffer;
	}

	void D3D9IndexStream::DoOnResetDevice()
	{
		D3D9RenderEngine& renderEngine(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		boost::shared_ptr<IDirect3DDevice9> d3dDevice(renderEngine.D3DDevice());

		IDirect3DIndexBuffer9* temp;
		TIF(d3dDevice->CreateIndexBuffer(static_cast<UINT>(this->Size()), 
				(BU_Dynamic == usage_) ? D3DUSAGE_DYNAMIC : 0,
				(IF_Index32 == format_) ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_DEFAULT, &temp, NULL));
		boost::shared_ptr<IDirect3DIndexBuffer9> buffer = MakeCOMPtr(temp);

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
