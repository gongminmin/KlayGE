#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Engine.hpp>

#include <KlayGE/RenderBuffer.hpp>

namespace KlayGE
{
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

	void RenderBuffer::AddIndexStream(bool staticStream)
	{
		indexStream_ = Engine::RenderFactoryInstance().MakeIndexStream(staticStream);
	}
}