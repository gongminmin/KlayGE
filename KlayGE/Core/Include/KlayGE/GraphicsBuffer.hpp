// GraphicsBuffer.hpp
// KlayGE 图形缓冲区类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2003-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 增加了access_hint (2008.9.20)
// 增加了ElementInitData (2008.10.1)
//
// 3.2.0
// 把IndexStream和VertexStream合并成GraphicsBuffer (2006.1.9)
//
// 3.1.0
// 分离了实例和几何流 (2005.10.31)
//
// 3.0.0
// 支持在单流中存储多种成员 (2005.10.15)
// 支持instancing (2005.10.21)
//
// 2.8.0
// 增加了CopyToMemory (2005.7.24)
//
// 2.4.0
// 改名为VertexBuffer (2005.3.7)
//
// 2.0.4
// 修改了纹理坐标 (2004.3.16)
//
// 2.0.3
// 去掉了VO_2D (2004.3.1)
// 改用vector存放数据 (2004.3.13)
//
// 2.0.0
// 初次建立 (2003.8.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _GRAPHICSBUFFER_HPP
#define _GRAPHICSBUFFER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <atomic>
#include <vector>

namespace KlayGE
{
	enum BufferUsage
	{
		BU_Static,
		BU_Dynamic
	};
	enum BufferAccess
	{
		BA_Read_Only,
		BA_Write_Only,
		BA_Read_Write,
		BA_Write_No_Overwrite
	};

	class KLAYGE_CORE_API GraphicsBuffer : boost::noncopyable
	{
	public:
		class Mapper final : boost::noncopyable
		{
			friend class GraphicsBuffer;

		public:
			Mapper(GraphicsBuffer& buffer, BufferAccess ba)
				: buffer_(buffer)
			{
				data_ = buffer_.Map(ba);
			}
			~Mapper()
			{
				buffer_.Unmap();
			}

			template <typename T>
			const T* Pointer() const
			{
				return static_cast<T*>(data_);
			}
			template <typename T>
			T* Pointer()
			{
				return static_cast<T*>(data_);
			}

		private:
			GraphicsBuffer& buffer_;
			void* data_;
		};

	public:
		GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, uint32_t structure_byte_stride);
		virtual ~GraphicsBuffer() noexcept;

		uint32_t Size() const noexcept
		{
			return size_in_byte_;
		}

		BufferUsage Usage() const noexcept
		{
			return usage_;
		}

		uint32_t AccessHint() const noexcept
		{
			return access_hint_;
		}

		uint32_t StructureByteStride() const noexcept
		{
			return structure_byte_stride_;
		}

		virtual void CopyToBuffer(GraphicsBuffer& target) = 0;
		virtual void CopyToSubBuffer(GraphicsBuffer& target,
			uint32_t dst_offset, uint32_t src_offset, uint32_t size) = 0;

		virtual void CreateHWResource(void const * init_data) = 0;
		virtual void DeleteHWResource() = 0;
		virtual bool HWResourceReady() const = 0;

		virtual void UpdateSubresource(uint32_t offset, uint32_t size, void const * data) = 0;

	private:
		virtual void* Map(BufferAccess ba) = 0;
		virtual void Unmap() = 0;

	protected:
		BufferUsage usage_;
		uint32_t access_hint_;

		uint32_t size_in_byte_;
		uint32_t structure_byte_stride_;
	};

	class KLAYGE_CORE_API SoftwareGraphicsBuffer : public GraphicsBuffer
	{
	public:
		SoftwareGraphicsBuffer(uint32_t size_in_byte, bool ref_only);

		void CopyToBuffer(GraphicsBuffer& target) override;
		void CopyToSubBuffer(GraphicsBuffer& target,
			uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

		void CreateHWResource(void const * init_data) override;
		void DeleteHWResource() override;
		bool HWResourceReady() const override;

		void UpdateSubresource(uint32_t offset, uint32_t size, void const * data) override;

	private:
		void* Map(BufferAccess ba) override;
		void Unmap() override;

	private:
		bool ref_only_;

		uint8_t* subres_data_ = nullptr;
		std::vector<uint8_t> data_block_;
		std::atomic<bool> mapped_{false};
	};
}

#endif		// _GRAPHICSBUFFER_HPP
