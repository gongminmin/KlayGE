#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/VertexBuffer.hpp>

namespace KlayGE
{
	void VertexBuffer::AddVertexStream(VertexStreamType type, U8 elementSize, U8 elementNum, bool staticStream)
	{
		vertexStreams_.push_back(Engine::RenderFactoryInstance().MakeVertexStream(type,
			elementSize, elementNum, staticStream));
	}

	VertexStreamPtr VertexBuffer::GetVertexStream(VertexStreamType type) const
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

	void VertexBuffer::AddIndexStream(bool staticStream)
	{
		indexStream_ = Engine::RenderFactoryInstance().MakeIndexStream(staticStream);
	}
}