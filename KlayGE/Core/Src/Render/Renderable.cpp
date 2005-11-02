// Renderable.cpp
// KlayGE 可渲染对象类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// GetWorld改名为GetModelMatrix (2005.6.17)
//
// 2.3.0
// 增加了Render (2005.1.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/SceneObject.hpp>

#include <KlayGE/Renderable.hpp>

namespace KlayGE
{
	Renderable::~Renderable()
	{
	}

	void Renderable::OnRenderBegin()
	{
	}

	void Renderable::OnRenderEnd()
	{
	}

	void Renderable::AddToRenderQueue()
	{
		Context::Instance().SceneManagerInstance().AddRenderable(this->shared_from_this());
	}

	void Renderable::Render()
	{
		this->UpdateInstanceStream();

		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		this->OnRenderBegin();
		renderEngine.Render(*this->GetVertexBuffer());
		this->OnRenderEnd();
	}

	void Renderable::AddInstance(SceneObjectPtr obj)
	{
		instances_.push_back(obj);
	}

	void Renderable::UpdateInstanceStream()
	{
		VertexStreamPtr inst_stream = this->GetVertexBuffer()->InstanceStream();
		if (inst_stream)
		{
			uint32_t const size = inst_stream->VertexSize();

			std::vector<uint8_t> buffer;
			buffer.reserve(size * instances_.size());
			for (size_t i = 0; i < instances_.size(); ++ i)
			{
				uint8_t const * src = static_cast<uint8_t const *>(instances_[i]->InstanceData());
				buffer.insert(buffer.end(), src, src + size);
			}

			inst_stream->Assign(&buffer[0], instances_.size());

			for (VertexBuffer::VertexStreamIterator iter = this->GetVertexBuffer()->VertexStreamBegin();
				iter != this->GetVertexBuffer()->VertexStreamEnd(); ++ iter)
			{
				(*iter)->FrequencyDivider(VertexStream::ST_Geometry, instances_.size());
			}
		}
	}
}
