// Renderable.cpp
// KlayGE 可渲染对象类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 修正了Render没有设置Technique的bug (2006.7.26)
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
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/SceneObject.hpp>

#include <boost/bind.hpp>

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

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		GraphicsBufferPtr const & inst_stream = this->GetRenderLayout()->InstanceStream();
		RenderTechniquePtr const & tech = this->GetRenderTechnique();
		RenderLayoutPtr const & layout = this->GetRenderLayout();
		if (inst_stream)
		{
			this->OnRenderBegin();
			re.Render(*tech, *layout);
			this->OnRenderEnd();
		}
		else
		{
			this->OnRenderBegin();
			if (instances_.empty())
			{
				re.Render(*tech, *layout);
			}
			else
			{
				for (uint32_t i = 0; i < instances_.size(); ++ i)
				{
					this->OnInstanceBegin(i);
					re.Render(*tech, *layout);
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
			RenderLayoutPtr const & rl = this->GetRenderLayout();

			GraphicsBufferPtr inst_stream = rl->InstanceStream();
			if (!inst_stream)
			{
				RenderFactory& rf(Context::Instance().RenderFactoryInstance());

				inst_stream = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
				rl->BindVertexStream(inst_stream, instances_[0].lock()->InstanceFormat(), RenderLayout::ST_Instance, 1);
			}
			else
			{
				for (size_t i = 0; i < instances_.size(); ++ i)
				{
					BOOST_ASSERT(rl->InstanceStreamFormat() == instances_[i].lock()->InstanceFormat());
				}
			}

			uint32_t const size = rl->InstanceSize();

			inst_stream->Resize(static_cast<uint32_t>(size * instances_.size()));
			{
				GraphicsBuffer::Mapper mapper(*inst_stream, BA_Write_Only);
				for (size_t i = 0; i < instances_.size(); ++ i)
				{
					uint8_t const * src = static_cast<uint8_t const *>(instances_[i].lock()->InstanceData());
					std::copy(src, src + size, mapper.Pointer<uint8_t>() + i * size);
				}
			}

			for (uint32_t i = 0; i < rl->NumVertexStreams(); ++ i)
			{
				rl->VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, static_cast<uint32_t>(instances_.size()));
			}
		}
	}
}
