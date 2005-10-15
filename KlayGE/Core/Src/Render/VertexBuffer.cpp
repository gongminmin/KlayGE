// VertexBuffer.cpp
// KlayGE 顶点缓冲区类 实现文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 改名为VertexBuffer (2005.3.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <boost/bind.hpp>

#include <KlayGE/VertexBuffer.hpp>

namespace KlayGE
{
	VertexStream::VertexStream(vertex_elements_type const & vertex_elems)
			: vertex_elems_(vertex_elems)
	{
	}

	VertexStream::~VertexStream()
	{
	}

	uint32_t VertexStream::NumElements() const
	{
		return vertex_elems_.size();
	}

	vertex_element const & VertexStream::Element(uint32_t index) const
	{
		BOOST_ASSERT(index < vertex_elems_.size());
		
		return vertex_elems_[index];
	}

	uint16_t VertexStream::VertexSize() const
	{
		uint16_t ret = 0;
		for (vertex_elements_type::const_iterator iter = vertex_elems_.begin();
			iter != vertex_elems_.end(); ++ iter)
		{
			ret += iter->element_size();
		}
		return ret;
	}

	uint32_t VertexStream::StreamSize() const
	{
		return this->VertexSize() * this->NumVertices();
	}

	VertexStream& VertexStream::Combine(VertexStreamPtr rhs)
	{
		BOOST_ASSERT(this->NumVertices() == rhs->NumVertices());

		std::vector<uint8_t> lhs_buffer(this->StreamSize());
		this->CopyToMemory(&lhs_buffer[0]);
		uint32_t const lhs_ver_size = this->VertexSize();

		std::vector<uint8_t> rhs_buffer(rhs->StreamSize());
		rhs->CopyToMemory(&rhs_buffer[0]);
		uint32_t const rhs_ver_size = rhs->VertexSize();

		std::vector<uint8_t> target_buffer(this->StreamSize() + rhs->StreamSize());

		std::vector<uint8_t>::iterator lhs_iter = lhs_buffer.begin();
		std::vector<uint8_t>::iterator rhs_iter = rhs_buffer.begin();
		std::vector<uint8_t>::iterator target_iter = target_buffer.begin();
		for (uint32_t i = 0; i < this->NumVertices(); ++ i)
		{
			std::copy(lhs_iter, lhs_iter + lhs_ver_size, target_iter);
			lhs_iter += lhs_ver_size;
			target_iter += lhs_ver_size;

			std::copy(rhs_iter, rhs_iter + rhs_ver_size, target_iter);
			rhs_iter += rhs_ver_size;
			target_iter += rhs_ver_size;
		}

		vertex_elems_.insert(vertex_elems_.end(), rhs->vertex_elems_.begin(), rhs->vertex_elems_.end());
		this->Assign(&target_buffer[0], this->NumVertices());

		return *this;
	}

	VertexStream& VertexStream::Append(VertexStreamPtr rhs)
	{
		BOOST_ASSERT(this->NumElements() == rhs->NumElements());
		BOOST_ASSERT(this->VertexSize() == rhs->VertexSize());

		std::vector<uint8_t> target_buffer(this->StreamSize());
		this->CopyToMemory(&target_buffer[0]);

		std::vector<uint8_t> rhs_buffer(rhs->StreamSize());
		rhs->CopyToMemory(&rhs_buffer[0]);

		target_buffer.insert(target_buffer.end(), rhs_buffer.begin(), rhs_buffer.end());
		this->Assign(&target_buffer[0], this->NumVertices() + rhs->NumVertices());

		return *this;
	}


	IndexStream::~IndexStream()
	{
	}

	uint32_t IndexStream::StreamSize() const
	{
		return sizeof(uint16_t) * this->NumIndices();
	}

	IndexStream& IndexStream::Append(IndexStreamPtr rhs)
	{
		std::vector<uint8_t> target_buffer(this->StreamSize());
		this->CopyToMemory(&target_buffer[0]);

		std::vector<uint8_t> rhs_buffer(rhs->StreamSize());
		rhs->CopyToMemory(&rhs_buffer[0]);

		target_buffer.insert(target_buffer.end(), rhs_buffer.begin(), rhs_buffer.end());
		this->Assign(&target_buffer[0], this->NumIndices() + rhs->NumIndices());

		return *this;
	}


	VertexBuffer::VertexBuffer(BufferType type)
			: type_(type)
	{
		vertexStreams_.reserve(8);
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	VertexBuffer::BufferType VertexBuffer::Type() const
	{
		return type_;
	}

	uint32_t VertexBuffer::NumVertices() const
	{
		return vertexStreams_.empty() ? 0 : vertexStreams_[0]->NumVertices();
	}

	void VertexBuffer::AddVertexStream(VertexStreamPtr vertex_stream)
	{
		vertexStreams_.push_back(vertex_stream);
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
		return (!indexStream_) ? 0 : indexStream_->NumIndices();
	}

	void VertexBuffer::SetIndexStream(IndexStreamPtr index_stream)
	{
		indexStream_ = index_stream;
	}

	IndexStreamPtr VertexBuffer::GetIndexStream() const
	{
		BOOST_ASSERT(indexStream_);
		return indexStream_;
	}
}
