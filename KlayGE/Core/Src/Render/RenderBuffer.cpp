#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>

#include <boost/bind.hpp>

#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
	VertexStream::VertexStream(VertexStreamType type, uint8 sizeElement, uint8 ElementsPerVertex)
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


	RenderBuffer::RenderBuffer(BufferType type)
			: type_(type)
	{
		vertexStreams_.reserve(16);
	}

	RenderBuffer::BufferType RenderBuffer::Type() const
	{
		return type_;
	}

	size_t RenderBuffer::NumVertices() const
	{
		return vertexStreams_.empty() ? 0 : vertexStreams_[0]->NumVertices();
	}

	void RenderBuffer::AddVertexStream(VertexStreamType type, uint8 sizeElement, uint8 numElement, bool staticStream)
	{
		if (!this->GetVertexStream(type))
		{
			vertexStreams_.push_back(Context::Instance().RenderFactoryInstance().MakeVertexStream(type,
				sizeElement, numElement, staticStream));
		}
	}

	VertexStreamPtr RenderBuffer::GetVertexStream(VertexStreamType type) const
	{
		VertexStreamConstIterator iter = std::find_if(this->VertexStreamBegin(), this->VertexStreamEnd(),
			boost::bind(std::equal_to<VertexStreamType>(), boost::bind(&VertexStream::Type, _1), type));
		if (iter != this->VertexStreamEnd())
		{
			return *iter;
		}
		return VertexStreamPtr();
	}

	RenderBuffer::VertexStreamIterator RenderBuffer::VertexStreamBegin()
	{
		return vertexStreams_.begin();
	}

	RenderBuffer::VertexStreamIterator RenderBuffer::VertexStreamEnd()
	{
		return vertexStreams_.end();
	}

	RenderBuffer::VertexStreamConstIterator RenderBuffer::VertexStreamBegin() const
	{
		return vertexStreams_.begin();
	}

	RenderBuffer::VertexStreamConstIterator RenderBuffer::VertexStreamEnd() const
	{
		return vertexStreams_.end();
	}

	bool RenderBuffer::UseIndices() const
	{
		return this->NumIndices() != 0;
	}

	size_t RenderBuffer::NumIndices() const
	{
		return (!indexStream_) ? 0 : indexStream_->NumIndices();
	}

	void RenderBuffer::AddIndexStream(bool staticStream)
	{
		indexStream_ = Context::Instance().RenderFactoryInstance().MakeIndexStream(staticStream);
	}

	IndexStreamPtr RenderBuffer::GetIndexStream() const
	{
		return indexStream_;
	}
}
