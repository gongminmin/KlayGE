// SceneManager.cpp
// KlayGE 场景管理器类 实现文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 增加了NumObjectsRendered，NumPrimitivesRendered和NumVerticesRendered (2005.3.20)
//
// 2.0.0
// 初次建立(2003.10.1)
//
// 2.0.2
// 增强了PushRenderable (2003.11.25)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Renderable.hpp>

#include <KlayGE/SceneManager.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::SceneManager()
	{
	}

	// 空的裁减器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::ClipScene(Camera const & /*camera*/)
	{
		for (RenderItemsType::iterator iter = renderItems_.begin(); iter != renderItems_.end(); ++ iter)
		{
			this->AddToRenderQueue(*iter);
		}
	}

	// 加入渲染物体
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::AddRenderable(RenderablePtr const & obj)
	{
		if (obj->CanBeCulled())
		{
			this->DoAddRenderable(obj);
		}
		else
		{
			this->AddToRenderQueue(obj);
		}
	}

	void SceneManager::DoAddRenderable(RenderablePtr const & obj)
	{
		renderItems_.push_back(obj);
	}

	// 加入渲染队列
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::AddToRenderQueue(RenderablePtr const & obj)
	{
		RenderEffectPtr const & effect(obj->GetRenderEffect());

		RenderQueueType::iterator iter(renderQueue_.find(effect));
		if (iter != renderQueue_.end())
		{
			iter->second.push_back(obj);
		}
		else
		{
			renderQueue_.insert(RenderQueueType::value_type(effect,
				RenderItemsType(1, obj)));
		}
	}

	// 清空场景管理器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Clear()
	{
		renderItems_.clear();
	}

	// 更新场景管理器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Update()
	{
		Context::Instance().AppInstance().Update();

		this->Flush();
	}

	// 把渲染队列中的物体渲染出来
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Flush()
	{
		numObjectsRendered_ = 0;
		numPrimitivesRendered_ = 0;
		numVerticesRendered_ = 0;

		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		this->ClipScene((*renderEngine.ActiveRenderTarget())->GetViewport().camera);

		renderEngine.BeginFrame();

		for (RenderQueueType::iterator queueIter = renderQueue_.begin();
			queueIter != renderQueue_.end(); ++ queueIter)
		{
			renderEngine.SetRenderEffect(queueIter->first);

			for (RenderItemsType::iterator itemIter = queueIter->second.begin();
				itemIter != queueIter->second.end(); ++ itemIter)
			{
				Renderable& obj(*(*itemIter));

				++ numObjectsRendered_;

				obj.OnRenderBegin();
				renderEngine.Render(*obj.GetVertexBuffer());
				obj.OnRenderEnd();

				numPrimitivesRendered_ += renderEngine.NumPrimitivesJustRendered();
				numVerticesRendered_ += renderEngine.NumVerticesJustRendered();
			}
		}
		renderQueue_.clear();

		renderEngine.EndFrame();

		Context::Instance().AppInstance().RenderOver();
	}

	// 获取渲染的物体数量
	/////////////////////////////////////////////////////////////////////////////////
	size_t SceneManager::NumObjectsRendered() const
	{
		return numObjectsRendered_;
	}

	// 获取渲染的图元数量
	/////////////////////////////////////////////////////////////////////////////////
	size_t SceneManager::NumPrimitivesRendered() const
	{
		return numPrimitivesRendered_;
	}

	// 获取渲染的顶点数量
	/////////////////////////////////////////////////////////////////////////////////
	size_t SceneManager::NumVerticesRendered() const
	{
		return numVerticesRendered_;
	}
}
