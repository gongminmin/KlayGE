// SceneManager.cpp
// KlayGE 场景管理器类 实现文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Sort objects by depth (2010.9.21)
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
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>

#include <map>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/SceneManager.hpp>

namespace
{
	using namespace KlayGE;

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

		void OnAddSceneObject(SceneObjectPtr const & /*obj*/)
		{
		}

		void OnDelSceneObject(SceneObjAABBsType::iterator /*iter*/)
		{
		}

	private:
		NullSceneManager(NullSceneManager const & rhs);
		NullSceneManager& operator=(NullSceneManager const & rhs);
	};

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::SceneManager()
		: frustum_(NULL),
			numObjectsRendered_(0),
			numRenderablesRendered_(0),
			numPrimitivesRendered_(0),
			numVerticesRendered_(0)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::~SceneManager()
	{
		this->ClearLight();
		this->ClearObject();
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
		App3DFramework& app = Context::Instance().AppInstance();
		Camera& camera = app.ActiveCamera();

		for (size_t i = 0; i < scene_objs_.size(); ++ i)
		{
			bool visible;

			SceneObjAABBPtrType const & soaabb = scene_objs_[i];
			SceneObjectPtr const & obj = soaabb->so;
			if (!(obj->Attrib() & SceneObject::SOA_Overlay) && obj->Visible())
			{
				if (camera.OmniDirectionalMode())
				{
					visible = true;
				}
				else
				{
					if (obj->Attrib() & SceneObject::SOA_Cullable)
					{
						AABBox aabb_ws;
						if (obj->Attrib() & SceneObject::SOA_Moveable)
						{
							AABBox const & aabb = obj->Bound();
							float4x4 const & mat = obj->ModelMatrix();

							aabb_ws = MathLib::transform_aabb(aabb, mat);
						}
						else
						{
							aabb_ws = *soaabb->aabb_ws;
						}

						visible = this->AABBVisible(aabb_ws);
					}
					else
					{
						visible = true;
					}
				}
			}
			else
			{
				visible = false;
			}

			soaabb->visible = visible;
		}
	}

	void SceneManager::AddLight(LightSourcePtr const & light)
	{
		lights_.push_back(light);
	}
	
	void SceneManager::DelLight(LightSourcePtr const & light)
	{
		BOOST_AUTO(iter, std::find(lights_.begin(), lights_.end(), light));
		lights_.erase(iter);
	}

	uint32_t SceneManager::NumLights() const
	{
		return static_cast<uint32_t>(lights_.size());
	}

	LightSourcePtr& SceneManager::GetLight(uint32_t index)
	{
		return lights_[index];
	}

	LightSourcePtr const & SceneManager::GetLight(uint32_t index) const
	{
		return lights_[index];
	}

	// 加入渲染物体
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::AddSceneObject(SceneObjectPtr const & obj)
	{
		AABBoxPtr aabb_ws;
		uint32_t const attr = obj->Attrib();
		if ((attr & SceneObject::SOA_Cullable)
			&& !(attr & SceneObject::SOA_Overlay)
			&& !(attr & SceneObject::SOA_Moveable))
		{
			AABBox const & aabb = obj->Bound();
			float4x4 const & mat = obj->ModelMatrix();
			aabb_ws = MakeSharedPtr<AABBox>(MathLib::transform_aabb(aabb, mat));
		}

		scene_objs_.push_back(MakeSharedPtr<SceneObjAABB>(obj, aabb_ws, false));

		this->OnAddSceneObject(obj);
	}

	// 删除渲染物体
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::DelSceneObject(SceneObjectPtr const & obj)
	{
		for (SceneObjAABBsType::iterator iter = scene_objs_.begin(); iter != scene_objs_.end(); ++ iter)
		{
			if ((*iter)->so == obj)
			{
				this->DelSceneObject(iter);
				break;
			}
		}
	}

	SceneManager::SceneObjAABBsType::iterator SceneManager::DelSceneObject(SceneManager::SceneObjAABBsType::iterator iter)
	{
		this->OnDelSceneObject(iter);

		return scene_objs_.erase(iter);
	}

	// 加入渲染队列
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::AddRenderable(RenderablePtr const & obj)
	{
		bool add;
		if (urt_ != 0)
		{
			if (urt_ & App3DFramework::URV_Opaque_Only)
			{
				add = !(obj->TransparencyBackFace() || obj->TransparencyFrontFace()) && !obj->SimpleForward();
			}
			else
			{
				if (urt_ & App3DFramework::URV_Transparency_Back_Only)
				{
					add = obj->TransparencyBackFace() && !obj->SimpleForward();
				}
				else
				{
					if (urt_ & App3DFramework::URV_Transparency_Front_Only)
					{
						add = obj->TransparencyFrontFace() && !obj->SimpleForward();
					}
					else
					{
						add = true;
					}
				}
			}

			if (urt_ & App3DFramework::URV_Special_Shading_Only)
			{
				add &= obj->SpecialShading() && !obj->SimpleForward();
			}

			if (urt_ & App3DFramework::URV_Simple_Forward_Only)
			{
				add &= obj->SimpleForward();
			}
		}
		else
		{
			add = true;
		}

		if (add)
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
	}

	bool SceneManager::AABBVisible(AABBox const & aabb)
	{
		if (frustum_)
		{
			return frustum_->Intersect(aabb) != BO_No;
		}
		else
		{
			return true;
		}
	}

	bool SceneManager::OBBVisible(OBBox const & obb)
	{
		if (frustum_)
		{
			return frustum_->Intersect(obb) != BO_No;
		}
		else
		{
			return true;
		}
	}

	bool SceneManager::SphereVisible(Sphere const & sphere)
	{
		if (frustum_)
		{
			return frustum_->Intersect(sphere) != BO_No;
		}
		else
		{
			return true;
		}
	}

	bool SceneManager::FrustumVisible(Frustum const & frustum)
	{
		if (frustum_)
		{
			return frustum_->Intersect(frustum) != BO_No;
		}
		else
		{
			return true;
		}
	}

	uint32_t SceneManager::NumSceneObjects() const
	{
		return static_cast<uint32_t>(scene_objs_.size());
	}

	SceneObjectPtr& SceneManager::GetSceneObject(uint32_t index)
	{
		return scene_objs_[index]->so;
	}

	SceneObjectPtr const & SceneManager::GetSceneObject(uint32_t index) const
	{
		return scene_objs_[index]->so;
	}

	void SceneManager::ClearLight()
	{
		lights_.resize(0);
	}

	void SceneManager::ClearObject()
	{
		scene_objs_.resize(0);
	}

	// 更新场景管理器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Update()
	{
		InputEngine& ie = Context::Instance().InputFactoryInstance().InputEngineInstance();
		ie.Update();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BeginFrame();

		App3DFramework& app = Context::Instance().AppInstance();
		float const app_time = app.AppTime();
		float const frame_time = app.FrameTime();

		typedef BOOST_TYPEOF(lights_) LightsType;
		BOOST_FOREACH(LightsType::const_reference light, lights_)
		{
			if (light->Enabled())
			{
				light->Update(app_time, frame_time);
			}
		}

		typedef BOOST_TYPEOF(scene_objs_) SceneObjsType;
		BOOST_FOREACH(SceneObjsType::const_reference scene_obj, scene_objs_)
		{
			if (!(scene_obj->so->Attrib() & SceneObject::SOA_Overlay))
			{
				scene_obj->so->Update(app_time, frame_time);
			}
		}

		this->FlushScene();

		for (BOOST_AUTO(iter, scene_objs_.begin()); iter != scene_objs_.end();)
		{
			if ((*iter)->so->Attrib() & SceneObject::SOA_Overlay)
			{
				iter = this->DelSceneObject(iter);
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
	void SceneManager::Flush(uint32_t urt)
	{
		urt_ = urt;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		App3DFramework& app = Context::Instance().AppInstance();
		float const app_time = app.AppTime();
		float const frame_time = app.FrameTime();

		numObjectsRendered_ = 0;
		numRenderablesRendered_ = 0;
		numPrimitivesRendered_ = 0;
		numVerticesRendered_ = 0;

		Camera& camera = app.ActiveCamera();

		BOOST_FOREACH(SceneObjAABBsType::const_reference scene_obj, scene_objs_)
		{
			scene_obj->visible = false;
		}
		if (urt & App3DFramework::URV_Need_Flush)
		{
			frustum_ = &camera.ViewFrustum();

			std::vector<uint32_t> visible_list((scene_objs_.size() + 31) / 32, 0);
			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				SceneObjectPtr const & obj = scene_objs_[i]->so;
				if (!(obj->Attrib() & SceneObject::SOA_Overlay) && obj->Visible())
				{
					visible_list[i / 32] |= (1UL << (i & 31));
				}
			}
			size_t seed = 0;
			boost::hash_range(seed, visible_list.begin(), visible_list.end());
			boost::hash_combine(seed, camera.OmniDirectionalMode());
			boost::hash_combine(seed, &camera);

			BOOST_AUTO(vmiter, visible_marks_map_.find(seed));
			if (vmiter == visible_marks_map_.end())
			{
				this->ClipScene();

				boost::shared_ptr<std::vector<char> > visible_marks = MakeSharedPtr<std::vector<char> >(scene_objs_.size());
				for (size_t i = 0; i < scene_objs_.size(); ++ i)
				{
					(*visible_marks)[i] = scene_objs_[i]->visible;
				}

				visible_marks_map_.insert(std::make_pair(seed, visible_marks));
			}
			else
			{
				for (size_t i = 0; i < scene_objs_.size(); ++ i)
				{
					scene_objs_[i]->visible = (*vmiter->second)[i] ? true : false;
				}
			}
		}
		if (urt & App3DFramework::URV_Overlay)
		{
			BOOST_FOREACH(SceneObjAABBsType::const_reference scene_obj, scene_objs_)
			{
				if (scene_obj->so->Attrib() & SceneObject::SOA_Overlay)
				{
					scene_obj->so->Update(app_time, frame_time);
					scene_obj->visible = scene_obj->so->Visible();
				}
			}
		}

		std::vector<std::pair<RenderablePtr, std::vector<SceneObjectPtr> >,
			boost::pool_allocator<std::pair<RenderablePtr, std::vector<SceneObjectPtr> > > > renderables;
		std::map<RenderablePtr, size_t, std::less<RenderablePtr>,
			boost::fast_pool_allocator<std::pair<RenderablePtr const, size_t> > > renderables_map;
		BOOST_FOREACH(SceneObjAABBsType::const_reference scene_obj, scene_objs_)
		{
			if (scene_obj->visible)
			{
				SceneObjectPtr const & so = scene_obj->so;
				RenderablePtr const & renderable = so->GetRenderable();

				BOOST_AUTO(iter, renderables_map.lower_bound(renderable));
				if ((iter != renderables_map.end()) && (iter->first == renderable))
				{
					renderables[iter->second].second.push_back(so);
				}
				else
				{
					renderables_map.insert(std::make_pair(renderable, renderables.size()));
					renderables.push_back(std::make_pair(renderable, std::vector<SceneObjectPtr>(1, so)));
				}

				++ numObjectsRendered_;
			}
		}
		renderables_map.clear();
		typedef BOOST_TYPEOF(renderables) RenderablesType;
		BOOST_FOREACH(RenderablesType::const_reference renderable, renderables)
		{
			Renderable& ra(*renderable.first);
			ra.AssignInstances(renderable.second.begin(), renderable.second.end());
			ra.AddToRenderQueue();
		}

		std::sort(render_queue_.begin(), render_queue_.end(), cmp_weight<std::pair<RenderTechniquePtr, RenderItemsType> >);

		float4 const & view_mat_z = camera.ViewMatrix().Col(2);
		typedef BOOST_TYPEOF(render_queue_) RenderQueueType;
		BOOST_FOREACH(RenderQueueType::reference items, render_queue_)
		{
			if (!items.first->Transparent() && !items.first->HasDiscard() && (items.second.size() > 1))
			{
				std::vector<std::pair<float, uint32_t> > min_depthes(items.second.size());
				for (size_t j = 0; j < min_depthes.size(); ++ j)
				{
					RenderablePtr const & renderable = items.second[j];
					AABBox const & box = renderable->Bound();
					uint32_t const num = renderable->NumInstances();
					float md = 1e10f;
					for (uint32_t i = 0; i < num; ++ i)
					{
						float4x4 const & mat = renderable->GetInstance(i)->ModelMatrix();
						float4 const zvec(MathLib::dot(mat.Row(0), view_mat_z),
							MathLib::dot(mat.Row(1), view_mat_z), MathLib::dot(mat.Row(2), view_mat_z),
							MathLib::dot(mat.Row(3), view_mat_z));
						for (int k = 0; k < 8; ++ k)
						{
							float3 const v = box[k];
							md = std::min(md, v.x() * zvec.x() + v.y() * zvec.y() + v.z() * zvec.z() + zvec.w());
						}
					}

					min_depthes[j] = std::make_pair(md, static_cast<uint32_t>(j));
				}

				std::sort(min_depthes.begin(), min_depthes.end());

				RenderItemsType sorted_items(min_depthes.size());
				for (size_t j = 0; j < min_depthes.size(); ++ j)
				{
					sorted_items[j] = items.second[min_depthes[j].second];
				}
				items.second.swap(sorted_items);
			}

			typedef BOOST_TYPEOF(items.second) ItemsType;
			BOOST_FOREACH(ItemsType::reference item, items.second)
			{
				item->Render();

				numPrimitivesRendered_ += re.NumPrimitivesJustRendered();
				numVerticesRendered_ += re.NumVerticesJustRendered();
			}
			numRenderablesRendered_ += items.second.size();
		}
		render_queue_.resize(0);

		app.RenderOver();

		urt_ = 0;
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
		FrameBufferPtr fb = re.CurFrameBuffer();

		visible_marks_map_.clear();

		uint32_t urt;
		App3DFramework& app = Context::Instance().AppInstance();
		for (uint32_t pass = 0;; ++ pass)
		{
			re.BeginPass();

			urt = app.Update(pass);

			if (urt & (App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished))
			{
				this->Flush(urt);
			}

			re.EndPass();

			if (urt & App3DFramework::URV_Finished)
			{
				break;
			}
		}

		re.PostProcess((urt & App3DFramework::URV_Skip_Postprocess) != 0);

		this->Flush(App3DFramework::URV_Overlay);

		re.BindFrameBuffer(fb);
	}
}
