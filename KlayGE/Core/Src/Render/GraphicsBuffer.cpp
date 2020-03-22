// VertexBuffer.cpp
// KlayGE 顶点缓冲区类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://www.klayge.org
//
// 3.2.0
// 支持32位的IndexStream (2006.1.4)
//
// 3.1.0
// 分离了实例和几何流 (2005.10.31)
//
// 3.0.0
// 支持instancing (2005.10.22)
//
// 2.4.0
// 改名为VertexBuffer (2005.3.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderView.hpp>

#include <cstring>
#include <system_error>

#include <KlayGE/GraphicsBuffer.hpp>

namespace KlayGE
{
	GraphicsBuffer::GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, uint32_t structure_byte_stride)
			: usage_(usage), access_hint_(access_hint), size_in_byte_(size_in_byte), structure_byte_stride_(structure_byte_stride)
	{
	}

	GraphicsBuffer::~GraphicsBuffer() noexcept = default;


	SoftwareGraphicsBuffer::SoftwareGraphicsBuffer(uint32_t size_in_byte, bool ref_only)
		: GraphicsBuffer(BU_Dynamic, EAH_CPU_Read | EAH_CPU_Write, size_in_byte, 0),
			ref_only_(ref_only)
	{
	}

	void SoftwareGraphicsBuffer::CopyToBuffer(GraphicsBuffer& target)
	{
		this->CopyToSubBuffer(target, 0, 0, size_in_byte_);
	}

	void SoftwareGraphicsBuffer::CopyToSubBuffer(GraphicsBuffer& target,
		uint32_t dst_offset, uint32_t src_offset, uint32_t size)
	{
		target.UpdateSubresource(dst_offset, size, subres_data_ + src_offset);
	}

	void SoftwareGraphicsBuffer::CreateHWResource(void const * init_data)
	{
		uint8_t const * ptr = static_cast<uint8_t const *>(init_data);
		if (ref_only_)
		{
			subres_data_ = static_cast<uint8_t*>(const_cast<void*>(init_data));
			data_block_.clear();
		}
		else
		{
			if (init_data != nullptr)
			{
				data_block_.assign(ptr, ptr + size_in_byte_);
			}
			else
			{
				data_block_.assign(size_in_byte_, 0);
			}

			subres_data_ = data_block_.data();
		}
	}

	void SoftwareGraphicsBuffer::DeleteHWResource()
	{
		subres_data_ = nullptr;
		data_block_.clear();
	}

	bool SoftwareGraphicsBuffer::HWResourceReady() const
	{
		return subres_data_ != nullptr;
	}

	void SoftwareGraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const * data)
	{
		std::memcpy(subres_data_ + offset, data, size);
	}

	void* SoftwareGraphicsBuffer::Map(BufferAccess ba)
	{
		KFL_UNUSED(ba);

		void* ret;
		bool already_mapped = false;
		if (mapped_.compare_exchange_strong(already_mapped, true))
		{
			ret = subres_data_;
		}
		else
		{
			ret = nullptr;
			TERRC(std::errc::device_or_resource_busy);
		}
		return ret;
	}

	void SoftwareGraphicsBuffer::Unmap()
	{
		bool already_mapped = true;
		if (!mapped_.compare_exchange_strong(already_mapped, false))
		{
			TERRC(std::errc::device_or_resource_busy);
		}
	}
}
