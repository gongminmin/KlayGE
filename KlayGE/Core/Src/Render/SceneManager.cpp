// SceneManager.cpp
// KlayGE 场景管理器类 实现文件
// Ver 2.0.2
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 修改记录
//
// 2.0.0
// 初次建立(2003.10.1)
//
// 2.0.2
// 增强了PushRenderable (2003.11.25)
//
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/SharePtr.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Renderable.hpp>

#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Engine.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::SceneManager()
	{
	}

	// 假如渲染物体
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::PushRenderable(const RenderablePtr& obj, const Matrix4& worldMat)
	{
		for (size_t i = 0; i < obj->NumSubs(); ++ i)
		{
			const RenderEffectPtr& effect(obj->GetRenderEffect(i));
			const VertexBufferPtr& vb(obj->GetVertexBuffer(i));

			RenderQueueType::iterator iter(renderQueue_.find(effect));
			if (iter != renderQueue_.end())
			{
				iter->second.push_back(RenderItemType(vb, worldMat));
			}
			else
			{
				renderQueue_.insert(RenderQueueType::value_type(effect,
					RenderItemsType(1, RenderItemType(vb, worldMat))));
			}
		}
	}

	// 更新场景管理器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Update(const ViewPointPtr& viewPoint)
	{
		Engine::AppInstance().Update();

		if (viewPoint.Get() != NULL)
		{
			this->ClipScene(*viewPoint);
		}

		RenderEngine& renderEngine(Engine::RenderFactoryInstance().RenderEngineInstance());

		renderEngine.BeginFrame();

		for (RenderQueueType::iterator queueIter = renderQueue_.begin();
			queueIter != renderQueue_.end(); ++ queueIter)
		{
			renderEngine.SetRenderEffect(queueIter->first);

			for (RenderItemsType::iterator itemIter = queueIter->second.begin();
				itemIter != queueIter->second.end(); ++ itemIter)
			{
				if (itemIter->second != Matrix4::Identity())
				{
					const Matrix4 oldWorld(renderEngine.WorldMatrix());
					const Matrix4 newWorld(oldWorld * itemIter->second);

					renderEngine.WorldMatrix(newWorld);
					renderEngine.Render(*(itemIter->first));
					renderEngine.WorldMatrix(oldWorld);
				}
				else
				{
					renderEngine.Render(*(itemIter->first));
				}
			}
		}
		renderQueue_.clear();

		renderEngine.EndFrame();

		Engine::AppInstance().RenderOver();
	}
}
