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
#include <KlayGE/SharedPtr.hpp>
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
	// 空场景剪裁器
	class NullClipper : public Clipper
	{
	public:
		void ClipScene(const Camera& camera)
			{ }
	};

	ClipperPtr Clipper::NullObject()
	{
		static ClipperPtr obj(new NullClipper);
		return obj;
	}

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::SceneManager()
		: clipper_(Clipper::NullObject())
	{
	}

	// 单件实例
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager& SceneManager::Instance()
	{
		static SceneManager sceneMgr;
		return sceneMgr;
	}

	// 假如渲染物体
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::PushRenderable(const RenderablePtr& obj)
	{
		const RenderEffectPtr& effect(obj->GetRenderEffect());

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

	// 更新场景管理器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Update()
	{
		Context::Instance().AppInstance().Update();

		this->Flash();
	}

	// 把渲染队列中的物体渲染出来
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Flash()
	{
		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		clipper_->ClipScene((*renderEngine.ActiveRenderTarget())->GetViewport().camera);

		renderEngine.BeginFrame();

		for (RenderQueueType::iterator queueIter = renderQueue_.begin();
			queueIter != renderQueue_.end(); ++ queueIter)
		{
			renderEngine.SetRenderEffect(queueIter->first);

			for (RenderItemsType::iterator itemIter = queueIter->second.begin();
				itemIter != queueIter->second.end(); ++ itemIter)
			{
				Renderable& obj(*(*itemIter));

				obj.OnRenderBegin();
				renderEngine.Render(*obj.GetRenderBuffer());
				obj.OnRenderEnd();
			}
		}
		renderQueue_.clear();

		renderEngine.EndFrame();

		Context::Instance().AppInstance().RenderOver();
	}

	// 连接场景剪裁器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::AttachClipper(const SharedPtr<Clipper>& clipper)
	{
		if (!clipper)
		{
			clipper_ = Clipper::NullObject();
		}
		else
		{
			clipper_ = clipper;
		}
	}
}
