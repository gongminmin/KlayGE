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
#include <vector>
#include <boost/noncopyable.hpp>

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

	class KLAYGE_CORE_API GraphicsBuffer
	{
	public:
		class Mapper : boost::noncopyable
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
		GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte);
		virtual ~GraphicsBuffer();

		uint32_t Size() const
		{
			return size_in_byte_;
		}

		BufferUsage Usage() const
		{
			return usage_;
		}

		uint32_t AccessHint() const
		{
			return access_hint_;
		}

		virtual void CopyToBuffer(GraphicsBuffer& rhs) = 0;

		virtual void CreateHWResource(void const * init_data) = 0;
		virtual void DeleteHWResource() = 0;

	private:
		virtual void* Map(BufferAccess ba) = 0;
		virtual void Unmap() = 0;

	protected:
		BufferUsage usage_;
		uint32_t access_hint_;

		uint32_t size_in_byte_;
	};
}

#endif		// _GRAPHICSBUFFER_HPP
