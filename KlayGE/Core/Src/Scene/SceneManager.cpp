// SceneManager.cpp
// KlayGE 场景管理器类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2003-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 处理Overlay物体 (2009.5.13)
// 增加了SceneObjects (2009.7.30)
//
// 3.5.0
// 增加了根据technique权重的排序 (2007.1.14)
//
// 3.1.0
// 自动处理Instance (2005.11.13)
//
// 3.0.0
// 保证了绘制顺序 (2005.8.16)
//
// 2.6.0
// 修正了CanBeCulled的bug (2005.5.26)
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
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>

#include <map>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/SceneManager.hpp>

namespace
{
	template <typename T>
	bool cmp_weight(T const & lhs, T const & rhs)
	{
		if (!lhs.first)
		{
			return true;
		}
		else
		{
			if (!rhs.first)
			{
				return false;
			}
			else
			{
				return lhs.first->Weight() < rhs.first->Weight();
			}
		}
	}
}

namespace KlayGE
{
	class NullSceneManager : public SceneManager
	{
	public:
		NullSceneManager()
		{
		}

		void Clear()
		{
			scene_objs_.resize(0);
		}

		void DoAddSceneObject(SceneObjectPtr const & obj)
		{
			scene_objs_.push_back(obj);
		}

		SceneObjectsType::iterator DoDelSceneObject(SceneObjectsType::iterator iter)
		{
			return scene_objs_.erase(iter);
		}

	private:
		NullSceneManager(NullSceneManager const & rhs);
		NullSceneManager& operator=(NullSceneManager const & rhs);
	};

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::SceneManager()
		: numObjectsRendered_(0),
			numRenderablesRendered_(0),
			numPrimitivesRendered_(0),
			numVerticesRendered_(0)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::~SceneManager()
	{
	}

	// 返回空对象
	/////////////////////////////////////////////////////////////////////////////////
	SceneManagerPtr SceneManager::NullObject()
	{
		static SceneManagerPtr obj = MakeSharedPtr<NullSceneManager>();
		return obj;
	}

	// 场景裁减
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::ClipScene()
	{
		for (size_t i = 0; i < scene_objs_.size(); ++ i)
		{
			bool visible;

			SceneObjectPtr const & obj = scene_objs_[i];
			if (!obj->Overlay() && obj->Visible())
			{
				if (obj->Cullable())
				{
					Box const & box = obj->GetBound();
					float4x4 const & mat = obj->GetModelMatrix();

					float3 min, max;
					min = max = MathLib::transform_coord(box[0], mat);
					for (size_t j = 1; j < 8; ++ j)
					{
						float3 vec = MathLib::transform_coord(box[j], mat);
						min = MathLib::minimize(min, vec);
						max = MathLib::maximize(max, vec);
					}

					visible = this->AABBVisible(Box(min, max));
				}
				else
				{
					visible = true;
				}
			}
			else
			{
				visible = false;
			}

			visible_marks_[i] = visible;
		}
	}

	// 加入渲染物体
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::AddSceneObject(SceneObjectPtr const & obj)
	{
		this->DoAddSceneObject(obj);
	}

	// 加入渲染队列
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::AddRenderable(RenderablePtr const & obj)
	{
		RenderTechniquePtr const & tech = obj->GetRenderTechnique()->Effect().PrototypeEffect()->TechniqueByName(obj->GetRenderTechnique()->Name());
		BOOST_AUTO(iter, std::find_if(render_queue_.begin(), render_queue_.end(),
			boost::bind(select1st<RenderQueueType::value_type>(), _1) == tech));
		if (iter != render_queue_.end())
		{
			iter->second.push_back(obj);
		}
		else
		{
			render_queue_.push_back(std::make_pair(tech, RenderItemsType(1, obj)));
		}
	}

	bool SceneManager::AABBVisible(Box const & box)
	{
		return frustum_.Visiable(box) != Frustum::VIS_NO;
	}

	SceneManager::SceneObjectsType& SceneManager::SceneObjects()
	{
		return scene_objs_;
	}

	SceneManager::SceneObjectsType const & SceneManager::SceneObjects() const
	{
		return scene_objs_;
	}

	// 更新场景管理器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Update()
	{
		InputEngine& ie = Context::Instance().InputFactoryInstance().InputEngineInstance();
		ie.Update();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BeginFrame();

		BOOST_FOREACH(BOOST_TYPEOF(scene_objs_)::const_reference scene_obj, scene_objs_)
		{
			scene_obj->Update();
		}

		if (re.StereoMode())
		{
			for (uint32_t i = 0; i < 2; ++ i)
			{
				re.StereoActiveEye(i);
				re.BindFrameBuffer(FrameBufferPtr());

				this->FlushScene();
			}

			re.Stereoscopic();
		}
		else
		{
			this->FlushScene();
		}

		for (BOOST_AUTO(iter, scene_objs_.begin()); iter != scene_objs_.end();)
		{
			if ((*iter)->Overlay())
			{
				iter = this->DoDelSceneObject(iter);
			}
			else
			{
				++ iter;
			}
		}

		re.EndFrame();
	}

	// 把渲染队列中的物体渲染出来
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Flush()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		App3DFramework& app = Context::Instance().AppInstance();

		numObjectsRendered_ = 0;
		numRenderablesRendered_ = 0;
		numPrimitivesRendered_ = 0;
		numVerticesRendered_ = 0;

		visible_marks_.resize(scene_objs_.size());
		if (urt_ & App3DFramework::URV_Need_Flush)
		{
			Camera& camera = app.ActiveCamera();
			frustum_.ClipMatrix(camera.ViewMatrix() * camera.ProjMatrix());

			this->ClipScene();
		}
		else
		{
			std::fill(visible_marks_.begin(), visible_marks_.end(), false);
		}
		if (urt_ & App3DFramework::URV_Finished)
		{
			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				if (scene_objs_[i]->Overlay())
				{
					scene_objs_[i]->Update();
					visible_marks_[i] = scene_objs_[i]->Visible();
				}
			}
		}

		std::vector<std::pair<RenderablePtr, SceneObjectsType>,
			boost::pool_allocator<std::pair<RenderablePtr, SceneObjectsType> > > renderables;
		std::map<RenderablePtr, size_t, std::less<RenderablePtr>,
			boost::fast_pool_allocator<std::pair<RenderablePtr const, size_t> > > renderables_map;
		for (size_t i = 0; i < visible_marks_.size(); ++ i)
		{
			if (visible_marks_[i])
			{
				SceneObjectPtr const & so = scene_objs_[i];
				RenderablePtr const & renderable = so->GetRenderable();

				BOOST_AUTO(iter, renderables_map.lower_bound(renderable));
				if ((iter != renderables_map.end()) && (iter->first == renderable))
				{
					renderables[iter->second].second.push_back(so);
				}
				else
				{
					renderables_map.insert(std::make_pair(renderable, renderables.size()));
					renderables.push_back(std::make_pair(renderable, SceneObjectsType(1, so)));
				}

				++ numObjectsRendered_;
			}
		}
		renderables_map.clear();
		BOOST_FOREACH(BOOST_TYPEOF(renderables)::const_reference renderable, renderables)
		{
			Renderable& ra(*renderable.first);
			ra.AssignInstances(renderable.second.begin(), renderable.second.end());
			ra.AddToRenderQueue();
		}

		std::sort(render_queue_.begin(), render_queue_.end(), cmp_weight<std::pair<RenderTechniquePtr, RenderItemsType> >);

		BOOST_FOREACH(BOOST_TYPEOF(render_queue_)::reference items, render_queue_)
		{
			BOOST_FOREACH(BOOST_TYPEOF(items.second)::reference item, items.second)
			{
				item->Render();

				numPrimitivesRendered_ += re.NumPrimitivesJustRendered();
				numVerticesRendered_ += re.NumVerticesJustRendered();
			}
			numRenderablesRendered_ += items.second.size();
		}
		render_queue_.resize(0);

		app.RenderOver();
	}

	// 获取渲染的物体数量
	/////////////////////////////////////////////////////////////////////////////////
	size_t SceneManager::NumObjectsRendered() const
	{
		return numObjectsRendered_;
	}

	// 获取渲染的可渲染对象数量
	/////////////////////////////////////////////////////////////////////////////////
	size_t SceneManager::NumRenderablesRendered() const
	{
		return numRenderablesRendered_;
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

	void SceneManager::FlushScene()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		App3DFramework& app = Context::Instance().AppInstance();
		for (uint32_t pass = 0;; ++ pass)
		{
			re.BeginPass();

			urt_ = app.Update(pass);

			if (urt_ & (App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished))
			{
				this->Flush();
			}

			re.EndPass();

			if (urt_ & App3DFramework::URV_Finished)
			{
				break;
			}
		}
	}
}
