#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
	VertexStream::VertexStream(VertexStreamType type, U8 elementSize, U8 elementNum)
			: type_(type),
				elementSize_(elementSize), elementNum_(elementNum)
	{
	}

	VertexStream::~VertexStream()
	{
	}

	VertexStreamType VertexStream::Type() const
	{
		return type_;
	}

	size_t VertexStream::ElementSize() const
	{
		return elementSize_;
	}

	size_t VertexStream::ElementNum() const
	{
		return elementNum_;
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

	void RenderBuffer::AddVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum, bool staticStream)
	{
		if (!this->GetVertexStream(type))
		{
			vertexStreams_.push_back(Engine::RenderFactoryInstance().MakeVertexStream(type,
				elementSize, elementNum, staticStream));
		}
	}

	VertexStreamPtr RenderBuffer::GetVertexStream(VertexStreamType type) const
	{
		for (VertexStreamConstIterator iter = this->VertexStreamBegin();
			iter != this->VertexStreamEnd(); ++ iter)
		{
			if ((*iter)->Type() == type)
			{
				return *iter;
			}
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
		indexStream_ = Engine::RenderFactoryInstance().MakeIndexStream(staticStream);
	}

	IndexStreamPtr RenderBuffer::GetIndexStream() const
	{
		return indexStream_;
	}
}
