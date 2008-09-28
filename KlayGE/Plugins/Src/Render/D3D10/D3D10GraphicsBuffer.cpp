// D3D10IndexBuffer.cpp
// KlayGE D3D10索引缓冲区类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
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

#include <KlayGE/D3D10/D3D10RenderEngine.hpp>
#include <KlayGE/D3D10/D3D10GraphicsBuffer.hpp>

namespace KlayGE
{
	D3D10GraphicsBuffer::D3D10GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags)
						: GraphicsBuffer(usage, access_hint),
							bind_flags_(bind_flags), hw_buf_size_(0)
	{
	}

	void D3D10GraphicsBuffer::DoResize()
	{
		BOOST_ASSERT(size_in_byte_ != 0);

		if (this->Size() > hw_buf_size_)
		{
			D3D10_BUFFER_DESC desc;
			desc.ByteWidth = size_in_byte_;
			desc.BindFlags = bind_flags_;
			desc.MiscFlags = 0;
			if (!(access_hint_ & EAH_CPU_Read) && !(access_hint_ & EAH_CPU_Write))
			{
				desc.Usage = D3D10_USAGE_DEFAULT;
			}
			else
			{
				if (!(access_hint_ & EAH_CPU_Read) && !(access_hint_ & EAH_GPU_Write) && !(access_hint_ & EAH_GPU_Read) && (access_hint_ & EAH_CPU_Write))
				{
					desc.Usage = D3D10_USAGE_DYNAMIC;
				}
				else
				{
					desc.Usage = D3D10_USAGE_STAGING;
				}
			}
			desc.CPUAccessFlags = 0;
			if (access_hint_ & EAH_CPU_Read)
			{
				desc.CPUAccessFlags |= D3D10_CPU_ACCESS_READ;
			}
			if (access_hint_ & EAH_CPU_Read)
			{
				desc.CPUAccessFlags |= D3D10_CPU_ACCESS_WRITE;
			}

			D3D10RenderEngine const & renderEngine(*checked_cast<D3D10RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			d3d_device_ = renderEngine.D3DDevice();

			ID3D10Buffer* buffer;
			TIF(d3d_device_->CreateBuffer(&desc, NULL, &buffer));
			buffer_ = MakeCOMPtr(buffer);
			hw_buf_size_ = this->Size();
		}
	}

	void* D3D10GraphicsBuffer::Map(BufferAccess ba)
	{
		BOOST_ASSERT(buffer_);

		D3D10_MAP type;
		switch (ba)
		{
		case BA_Read_Only:
			type = D3D10_MAP_READ;
			break;

		case BA_Write_Only:
			type = D3D10_MAP_WRITE;
			break;

		case BA_Read_Write:
			type = D3D10_MAP_READ_WRITE;
			break;

		default:
			BOOST_ASSERT(false);
			type = D3D10_MAP_READ;
			break;
		}

		void* ret;
		TIF(buffer_->Map(type, 0, &ret));
		return ret;
	}

	void D3D10GraphicsBuffer::Unmap()
	{
		BOOST_ASSERT(buffer_);

		buffer_->Unmap();
	}

	ID3D10BufferPtr D3D10GraphicsBuffer::D3DBuffer() const
	{
		return buffer_;
	}

	void D3D10GraphicsBuffer::CopyToBuffer(GraphicsBuffer& rhs)
	{
		D3D10GraphicsBuffer& d3d_gb = *checked_cast<D3D10GraphicsBuffer*>(&rhs);
		d3d_device_->CopyResource(d3d_gb.D3DBuffer().get(), buffer_.get());
	}
}
