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
	VertexStream::VertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t ElementsPerVertex)
			: type_(type),
				sizeElement_(sizeElement), elementPerVertex_(ElementsPerVertex)
	{
	}

	VertexStream::~VertexStream()
	{
	}

	VertexStreamType VertexStream::Type() const
	{
		return type_;
	}

	size_t VertexStream::sizeElement() const
	{
		return sizeElement_;
	}

	size_t VertexStream::ElementsPerVertex() const
	{
		return elementPerVertex_;
	}


	IndexStream::~IndexStream()
	{
	}


	VertexBuffer::VertexBuffer(BufferType type)
			: type_(type)
	{
		vertexStreams_.reserve(16);
	}

	VertexBuffer::BufferType VertexBuffer::Type() const
	{
		return type_;
	}

	size_t VertexBuffer::NumVertices() const
	{
		return vertexStreams_.empty() ? 0 : vertexStreams_[0]->NumVertices();
	}

	void VertexBuffer::AddVertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t numElement, bool staticStream)
	{
		if (!this->GetVertexStream(type))
		{
			vertexStreams_.push_back(Context::Instance().RenderFactoryInstance().MakeVertexStream(type,
				sizeElement, numElement, staticStream));
		}
		else
		{
			assert(false);
		}
	}

	VertexStreamPtr VertexBuffer::GetVertexStream(VertexStreamType type) const
	{
		VertexStreamConstIterator iter = std::find_if(this->VertexStreamBegin(), this->VertexStreamEnd(),
			boost::bind(std::equal_to<VertexStreamType>(), boost::bind(&VertexStream::Type, _1), type));
		if (iter != this->VertexStreamEnd())
		{
			return *iter;
		}

		assert(false);
		return VertexStreamPtr();
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

	size_t VertexBuffer::NumIndices() const
	{
		return (!indexStream_) ? 0 : indexStream_->NumIndices();
	}

	void VertexBuffer::AddIndexStream(bool staticStream)
	{
		indexStream_ = Context::Instance().RenderFactoryInstance().MakeIndexStream(staticStream);
	}

	IndexStreamPtr VertexBuffer::GetIndexStream() const
	{
		assert(indexStream_);
		return indexStream_;
	}
}
