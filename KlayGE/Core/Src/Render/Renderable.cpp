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

	void Renderable::OnInstanceBegin(uint32_t /*id*/)
	{
	}

	void Renderable::OnInstanceEnd(uint32_t /*id*/)
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

		VertexStreamPtr inst_stream = this->GetVertexBuffer()->InstanceStream();
		if (inst_stream)
		{
			this->OnRenderBegin();
			renderEngine.Render(*this->GetVertexBuffer());
			this->OnRenderEnd();
		}
		else
		{
			this->OnRenderBegin();
			if (instances_.empty())
			{
				renderEngine.Render(*this->GetVertexBuffer());
			}
			else
			{
				for (uint32_t i = 0; i < instances_.size(); ++ i)
				{
					this->OnInstanceBegin(i);
					renderEngine.Render(*this->GetVertexBuffer());
					this->OnInstanceEnd(i);
				}
			}
			this->OnRenderEnd();
		}
	}

	void Renderable::AddInstance(SceneObjectPtr obj)
	{
		instances_.push_back(boost::weak_ptr<SceneObject>(obj));
	}

	void Renderable::UpdateInstanceStream()
	{
		if (!instances_.empty() && !instances_[0].lock()->InstanceFormat().empty())
		{
			VertexStreamPtr inst_stream = this->GetVertexBuffer()->InstanceStream();
			if (!inst_stream)
			{
				RenderFactory& rf(Context::Instance().RenderFactoryInstance());

				inst_stream = rf.MakeVertexStream(instances_[0].lock()->InstanceFormat(), false);
				inst_stream->FrequencyDivider(VertexStream::ST_Instance, 1);
				this->GetVertexBuffer()->AddVertexStream(inst_stream);
			}
			else
			{
				for (size_t i = 0; i < instances_.size(); ++ i)
				{
					BOOST_ASSERT(inst_stream->Elements() == instances_[i].lock()->InstanceFormat());
				}
			}

			uint32_t const size = inst_stream->VertexSize();

			std::vector<uint8_t> buffer;
			buffer.reserve(size * instances_.size());
			for (size_t i = 0; i < instances_.size(); ++ i)
			{
				uint8_t const * src = static_cast<uint8_t const *>(instances_[i].lock()->InstanceData());
				buffer.insert(buffer.end(), src, src + size);
			}

			inst_stream->Assign(&buffer[0], static_cast<uint32_t>(instances_.size()));

			for (VertexBuffer::VertexStreamIterator iter = this->GetVertexBuffer()->VertexStreamBegin();
				iter != this->GetVertexBuffer()->VertexStreamEnd(); ++ iter)
			{
				(*iter)->FrequencyDivider(VertexStream::ST_Geometry, static_cast<uint32_t>(instances_.size()));
			}
		}
	}
}
