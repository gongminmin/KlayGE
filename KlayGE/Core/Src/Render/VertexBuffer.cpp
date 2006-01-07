// VertexBuffer.cpp
// KlayGE 顶点缓冲区类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/MapVector.hpp>

#include <boost/bind.hpp>

#include <KlayGE/VertexBuffer.hpp>

namespace KlayGE
{
	class NullVertexStream : public VertexStream
	{
	public:
		NullVertexStream()
			: VertexStream(BU_Static)
		{
		}

		void* Map(BufferAccess /*ba*/)
		{
			return NULL;
		}

		void Unmap()
		{
		}

		void DoCreate()
		{
		}
	};

	VertexStream::VertexStream(BufferUsage usage)
			: usage_(usage), size_in_byte_(0),
				type_(ST_Geometry), freq_(1)
	{
	}

	VertexStream::~VertexStream()
	{
	}

	VertexStreamPtr VertexStream::NullObject()
	{
		static VertexStreamPtr obj(new NullVertexStream);
		return obj;
	}

	void VertexStream::Resize(uint32_t size_in_byte)
	{
		size_in_byte_ = size_in_byte;
		this->DoCreate();
	}

	void VertexStream::CopyToStream(VertexStream& rhs)
	{
		VertexStream::Mapper lhs_mapper(*this, BA_Read_Only);
		VertexStream::Mapper rhs_mapper(rhs, BA_Write_Only);
		std::copy(lhs_mapper.Pointer<uint8_t>(), lhs_mapper.Pointer<uint8_t>() + size_in_byte_,
			rhs_mapper.Pointer<uint8_t>());
	}

	void VertexStream::FrequencyDivider(stream_type type, uint32_t freq)
	{
		type_ = type;
		freq_ = freq;
	}

	VertexStream::stream_type VertexStream::StreamType() const
	{
		return type_;
	}

    uint32_t VertexStream::Frequency() const
	{
		return freq_;
	}


	class NullIndexStream : public IndexStream
	{
	public:
		NullIndexStream()
			: IndexStream(BU_Static)
		{
		}

		void* Map(BufferAccess /*ba*/)
		{
			return NULL;
		}
		void Unmap()
		{
		}

	private:
		void DoCreate()
		{
		}
	};

	IndexStream::IndexStream(BufferUsage usage)
		: usage_(usage)
	{
	}

	IndexStream::~IndexStream()
	{
	}

	IndexStreamPtr IndexStream::NullObject()
	{
		static IndexStreamPtr obj(new NullIndexStream);
		return obj;
	}

	void IndexStream::Resize(uint32_t size_in_byte)
	{
		size_in_byte_ = size_in_byte;
		this->DoCreate();
	}

	void IndexStream::CopyToStream(IndexStream& rhs)
	{
		IndexStream::Mapper lhs_mapper(*this, BA_Read_Only);
		IndexStream::Mapper rhs_mapper(rhs, BA_Write_Only);
		std::copy(lhs_mapper.Pointer<uint8_t>(), lhs_mapper.Pointer<uint8_t>() + size_in_byte_,
			rhs_mapper.Pointer<uint8_t>());
	}


	class NullVertexBuffer : public VertexBuffer
	{
	public:
		NullVertexBuffer()
			: VertexBuffer(BT_PointList)
		{
		}
	};

	VertexBuffer::VertexBuffer(BufferType type)
			: type_(type)
	{
		vertexStreams_.reserve(8);
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	VertexBufferPtr VertexBuffer::NullObject()
	{
		static VertexBufferPtr obj(new NullVertexBuffer);
		return obj;
	}

	VertexBuffer::BufferType VertexBuffer::Type() const
	{
		return type_;
	}

	uint32_t VertexBuffer::NumVertices() const
	{
		uint32_t vertex_size = 0;
		for (size_t i = 0; i < vertex_stream_formats_[0].size(); ++ i)
		{
			vertex_size += vertex_stream_formats_[0][i].element_size();
		}
		return vertexStreams_.empty() ? 0 : (vertexStreams_[0]->Size() / vertex_size);
	}

	void VertexBuffer::AddVertexStream(VertexStreamPtr vertex_stream, vertex_elements_type const & vet)
	{
		BOOST_ASSERT(vertex_stream);

		uint32_t size = 0;
		for (size_t i = 0; i < vet.size(); ++ i)
		{
			size += vet[i].element_size();
		}

		if (VertexStream::ST_Geometry == vertex_stream->StreamType())
		{
			vertexStreams_.push_back(vertex_stream);
			vertex_stream_formats_.push_back(vet);
			vertex_sizes_.push_back(size);
		}
		else
		{
			BOOST_ASSERT(!instance_stream_);
			instance_stream_ = vertex_stream;
			instance_format_ = vet;
			instance_size_ = size;
		}
	}

	VertexBuffer::VertexStreamIterator VertexBuffer::VertexStreamBegin()
	{
		return vertexStreams_.begin();
	}

	VertexBuffer::VertexStreamIterator VertexBuffer::VertexStreamEnd()
	{
		return vertexStreams_.end();
	}

	VertexBuffer::VertexStreamConstIterator VertexBuffer::VertexStreamBegin() const
	{
		return vertexStreams_.begin();
	}

	VertexBuffer::VertexStreamConstIterator VertexBuffer::VertexStreamEnd() const
	{
		return vertexStreams_.end();
	}

	bool VertexBuffer::UseIndices() const
	{
		return this->NumIndices() != 0;
	}

	uint32_t VertexBuffer::NumIndices() const
	{
		if (indexStream_)
		{
			if (IF_Index16 == index_format_)
			{
				return indexStream_->Size() / sizeof(uint16_t);
			}
			else
			{
				return indexStream_->Size() / sizeof(uint32_t);
			}
		}
		else
		{
			return 0;
		}
	}

	void VertexBuffer::SetIndexStream(IndexStreamPtr index_stream, IndexFormat format)
	{
		indexStream_ = index_stream;
		index_format_ = format;
	}

	IndexStreamPtr VertexBuffer::GetIndexStream() const
	{
		BOOST_ASSERT(indexStream_);
		return indexStream_;
	}

	VertexStreamPtr VertexBuffer::InstanceStream() const
	{
		return instance_stream_;
	}

	uint32_t VertexBuffer::NumInstance() const
	{
		return vertexStreams_[0]->Frequency();
	}

	void VertexBuffer::ExpandInstance(VertexStreamPtr& hint, uint32_t inst_no) const
	{
		BOOST_ASSERT(instance_stream_);

		uint32_t num_instance = this->NumInstance();
		BOOST_ASSERT(inst_no < num_instance);

		uint32_t num_vertices = this->NumVertices();
		
		if (!hint || (instance_size_ * num_vertices != hint->Size()))
		{
			hint = this->ExpandInstance(inst_no);
		}
		else
		{
			VertexStream::Mapper src_mapper(*instance_stream_, BA_Read_Only);
			hint->Resize(instance_size_ * num_vertices);
			VertexStream::Mapper dst_mapper(*hint, BA_Write_Only);

			for (uint32_t i = 0; i < num_vertices; ++ i)
			{
				std::copy(src_mapper.Pointer<uint8_t>() + inst_no * instance_size_,
					src_mapper.Pointer<uint8_t>() + (inst_no + 1) * instance_size_,
					dst_mapper.Pointer<uint8_t>() + i * instance_size_);
			}

			hint->FrequencyDivider(VertexStream::ST_Geometry, 1);
		}
	}

	VertexStreamPtr VertexBuffer::ExpandInstance(uint32_t inst_no) const
	{
		BOOST_ASSERT(instance_stream_);

		uint32_t num_instance = this->NumInstance();
		BOOST_ASSERT(inst_no < num_instance);

		uint32_t num_vertices = this->NumVertices();

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		VertexStreamPtr ret = rf.MakeVertexStream(BU_Dynamic);

		VertexStream::Mapper src_mapper(*instance_stream_, BA_Read_Only);
		ret->Resize(instance_size_ * num_vertices);
		VertexStream::Mapper dst_mapper(*ret, BA_Write_Only);

		for (uint32_t i = 0; i < num_vertices; ++ i)
		{
			std::copy(src_mapper.Pointer<uint8_t>() + inst_no * instance_size_,
				src_mapper.Pointer<uint8_t>() + (inst_no + 1) * instance_size_,
				dst_mapper.Pointer<uint8_t>() + i * instance_size_);
		}

		ret->FrequencyDivider(VertexStream::ST_Geometry, 1);

		return ret;
	}
}
