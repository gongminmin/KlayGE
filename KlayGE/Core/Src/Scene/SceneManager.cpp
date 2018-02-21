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
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
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
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KFL/Hash.hpp>

#include <map>
#include <algorithm>

#include <KlayGE/SceneManager.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::SceneManager()
		: frustum_(nullptr),
			small_obj_threshold_(0),
			update_elapse_(1.0f / 60),
			num_objects_rendered_(0), num_renderables_rendered_(0),
			num_primitives_rendered_(0), num_vertices_rendered_(0),
			num_draw_calls_(0), num_dispatch_calls_(0),
			quit_(false), deferred_mode_(false)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::~SceneManager()
	{
		quit_ = true;
		(*update_thread_)();

		this->ClearLight();
		this->ClearCamera();
		this->ClearObject();
	}

	void SceneManager::Suspend()
	{
		this->DoSuspend();
	}

	void SceneManager::Resume()
	{
		this->DoResume();
	}

	void SceneManager::SmallObjectThreshold(float area)
	{
		small_obj_threshold_ = area;
	}

	void SceneManager::SceneUpdateElapse(float elapse)
	{
		update_elapse_ = elapse;
	}

	// 场景裁减
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::ClipScene()
	{
		App3DFramework& app = Context::Instance().AppInstance();
		Camera& camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewProjMatrix();
		auto drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			int32_t cas_index = drl->CurrCascadeIndex();
			if (cas_index >= 0)
			{
				view_proj *= drl->GetCascadedShadowLayer()->CascadeCropMatrix(cas_index);
			}
		}

		for (auto const & obj : scene_objs_)
		{
			auto so = obj.get();
			BoundOverlap visible;
			uint32_t const attr = so->Attrib();
			if (so->Visible())
			{
				visible = this->VisibleTestFromParent(so, camera.ForwardVec(), camera.EyePos(), view_proj);
				if (BO_Partial == visible)
				{
					if (attr & SceneObject::SOA_Moveable)
					{
						so->UpdateAbsModelMatrix();
					}

					if (attr & SceneObject::SOA_Cullable)
					{
						if (small_obj_threshold_ > 0)
						{
							visible = ((MathLib::ortho_area(camera.ForwardVec(), so->PosBoundWS()) > small_obj_threshold_)
								&& (MathLib::perspective_area(camera.EyePos(), view_proj, so->PosBoundWS()) > small_obj_threshold_))
								? BO_Yes : BO_No;
						}
						else
						{
							visible = BO_Yes;
						}
					}
					else
					{
						visible = BO_Yes;
					}

					if (!camera.OmniDirectionalMode() && (attr & SceneObject::SOA_Cullable)
						&& (BO_Yes == visible))
					{
						visible = this->AABBVisible(so->PosBoundWS());
					}
				}
			}
			else
			{
				visible = BO_No;
			}

			so->VisibleMark(visible);
		}
	}

	void SceneManager::AddCamera(CameraPtr const & camera)
	{
		cameras_.push_back(camera);
	}
	
	void SceneManager::DelCamera(CameraPtr const & camera)
	{
		auto iter = std::find(cameras_.begin(), cameras_.end(), camera);
		cameras_.erase(iter);
	}

	std::vector<CameraPtr>::iterator SceneManager::DelCamera(std::vector<CameraPtr>::iterator iter)
	{
		std::lock_guard<std::mutex> lock(update_mutex_);
		return cameras_.erase(iter);
	}

	uint32_t SceneManager::NumCameras() const
	{
		return static_cast<uint32_t>(cameras_.size());
	}

	CameraPtr& SceneManager::GetCamera(uint32_t index)
	{
		return cameras_[index];
	}

	CameraPtr const & SceneManager::GetCamera(uint32_t index) const
	{
		return cameras_[index];
	}

	void SceneManager::AddLight(LightSourcePtr const & light)
	{
		lights_.push_back(light);
	}
	
	void SceneManager::DelLight(LightSourcePtr const & light)
	{
		auto iter = std::find(lights_.begin(), lights_.end(), light);
		lights_.erase(iter);
	}

	std::vector<LightSourcePtr>::iterator SceneManager::DelLight(std::vector<LightSourcePtr>::iterator iter)
	{
		std::lock_guard<std::mutex> lock(update_mutex_);
		return lights_.erase(iter);
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
		std::lock_guard<std::mutex> lock(update_mutex_);
		this->AddSceneObjectLocked(obj);
	}

	void SceneManager::AddSceneObjectLocked(SceneObjectPtr const & obj)
	{
		App3DFramework& app = Context::Instance().AppInstance();
		float const app_time = app.AppTime();
		float const frame_time = app.FrameTime();
		obj->MainThreadUpdate(app_time, frame_time);

		uint32_t const attr = obj->Attrib();
		if (attr & SceneObject::SOA_Overlay)
		{
			overlay_scene_objs_.push_back(obj);
		}
		else
		{
			if ((attr & SceneObject::SOA_Cullable)
				&& !(attr & SceneObject::SOA_Moveable))
			{
				obj->UpdateAbsModelMatrix();
			}

			scene_objs_.push_back(obj);
			this->OnAddSceneObject(obj);
		}
	}

	// 删除渲染物体
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::DelSceneObject(SceneObjectPtr const & obj)
	{
		std::lock_guard<std::mutex> lock(update_mutex_);
		this->DelSceneObjectLocked(obj);
	}

	void SceneManager::DelSceneObjectLocked(SceneObjectPtr const & obj)
	{
		for (auto iter = scene_objs_.begin(); iter != scene_objs_.end(); ++ iter)
		{
			if (*iter == obj)
			{
				this->DelSceneObjectLocked(iter);
				break;
			}
		}
	}

	std::vector<SceneObjectPtr>::iterator SceneManager::DelSceneObject(std::vector<SceneObjectPtr>::iterator iter)
	{
		std::lock_guard<std::mutex> lock(update_mutex_);
		return this->DelSceneObjectLocked(iter);
	}

	std::vector<SceneObjectPtr>::iterator SceneManager::DelSceneObjectLocked(std::vector<SceneObjectPtr>::iterator iter)
	{
		this->OnDelSceneObject(iter);
		return scene_objs_.erase(iter);
	}

	// 加入渲染队列
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::AddRenderable(Renderable* obj)
	{
		if (obj->HWResourceReady())
		{
			bool add;
			if (obj->SelectMode())
			{
				add = true;
			}
			else
			{
				if (urt_ & App3DFramework::URV_OpaqueOnly)
				{
					add = !(obj->TransparencyBackFace() || obj->TransparencyFrontFace());
				}
				else if (urt_ & App3DFramework::URV_TransparencyBackOnly)
				{
					add = obj->TransparencyBackFace();
				}
				else if (urt_ & App3DFramework::URV_TransparencyFrontOnly)
				{
					add = obj->TransparencyFrontFace();
				}
				else
				{
					add = true;
				}

				if (deferred_mode_)
				{
					if (urt_ & App3DFramework::URV_SpecialShadingOnly)
					{
						add &= obj->SpecialShading();
					}
					else if (urt_ & App3DFramework::URV_ReflectionOnly)
					{
						add &= obj->Reflection();
					}
				}

				add &= (deferred_mode_ && !obj->SimpleForward() && !(urt_ & App3DFramework::URV_SimpleForwardOnly)
						&& !obj->VDM() && !(urt_ & App3DFramework::URV_VDMOnly))
					|| !deferred_mode_;

				add |= (deferred_mode_ && obj->SimpleForward() && (urt_ & App3DFramework::URV_SimpleForwardOnly));
				add |= (deferred_mode_ && obj->VDM() && (urt_ & App3DFramework::URV_VDMOnly));
			}

			if (add)
			{
				RenderTechnique const * obj_tech = obj->GetRenderTechnique();
				BOOST_ASSERT(obj_tech);
				bool found = false;
				for (auto& items : render_queue_)
				{
					if (items.first == obj_tech)
					{
						items.second.push_back(obj);
						found = true;
						break;
					}
				}
				if (!found)
				{
					render_queue_.emplace_back(obj_tech, std::vector<Renderable*>(1, obj));
				}
			}
		}
	}

	BoundOverlap SceneManager::AABBVisible(AABBox const & aabb) const
	{
		if (frustum_)
		{
			return frustum_->Intersect(aabb);
		}
		else
		{
			return BO_Yes;
		}
	}

	BoundOverlap SceneManager::OBBVisible(OBBox const & obb) const
	{
		if (frustum_)
		{
			return frustum_->Intersect(obb);
		}
		else
		{
			return BO_Yes;
		}
	}

	BoundOverlap SceneManager::SphereVisible(Sphere const & sphere) const
	{
		if (frustum_)
		{
			return frustum_->Intersect(sphere);
		}
		else
		{
			return BO_Yes;
		}
	}

	BoundOverlap SceneManager::FrustumVisible(Frustum const & frustum) const
	{
		if (frustum_)
		{
			return frustum_->Intersect(frustum);
		}
		else
		{
			return BO_Yes;
		}
	}

	uint32_t SceneManager::NumSceneObjects() const
	{
		return static_cast<uint32_t>(scene_objs_.size());
	}

	SceneObjectPtr& SceneManager::GetSceneObject(uint32_t index)
	{
		return scene_objs_[index];
	}

	SceneObjectPtr const & SceneManager::GetSceneObject(uint32_t index) const
	{
		return scene_objs_[index];
	}

	void SceneManager::ClearCamera()
	{
		cameras_.resize(0);
	}

	void SceneManager::ClearLight()
	{
		lights_.resize(0);
	}

	void SceneManager::ClearObject()
	{
		std::lock_guard<std::mutex> lock(update_mutex_);
		scene_objs_.resize(0);
		overlay_scene_objs_.resize(0);
	}

	// 更新场景管理器
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Update()
	{
		deferred_mode_ = !!Context::Instance().DeferredRenderingLayerInstance();

		App3DFramework& app = Context::Instance().AppInstance();
		float const app_time = app.AppTime();
		float const frame_time = app.FrameTime();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BeginFrame();

		this->FlushScene();

		if (!update_thread_ && !quit_)
		{
			update_thread_ = MakeUniquePtr<joiner<void>>(Context::Instance().ThreadPool()(
				[this] { this->UpdateThreadFunc(); }));
		}

		std::vector<SceneObjectPtr> added_scene_objs;
		{
			std::lock_guard<std::mutex> lock(update_mutex_);

			for (auto const & scene_obj : scene_objs_)
			{
				if (scene_obj->MainThreadUpdate(app_time, frame_time))
				{
					added_scene_objs.push_back(scene_obj);
				}
			}

			overlay_scene_objs_.clear();
			for (auto iter = lights_.begin(); iter != lights_.end();)
			{
				if ((*iter)->Attrib() & LightSource::LSA_Temporary)
				{
					iter = this->DelLight(iter);
				}
				else
				{
					++ iter;
				}
			}

			for (auto const & scene_obj : added_scene_objs)
			{
				scene_obj->OnAttachRenderable(true);
				this->OnAddSceneObject(scene_obj);
			}
		}

		FrameBuffer& fb = *re.ScreenFrameBuffer();
		fb.SwapBuffers();

		InputEngine& ie = Context::Instance().InputFactoryInstance().InputEngineInstance();
		ie.Update();

		for (auto const & camera : cameras_)
		{
			camera->Update(app_time, frame_time);
		}

		for (auto const & light : lights_)
		{
			if (light->Enabled())
			{
				light->Update(app_time, frame_time);
			}
		}

		fb.WaitOnSwapBuffers();

		re.EndFrame();
	}

	// 把渲染队列中的物体渲染出来
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Flush(uint32_t urt)
	{
		std::lock_guard<std::mutex> lock(update_mutex_);

		urt_ = urt;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		App3DFramework& app = Context::Instance().AppInstance();
		float const app_time = app.AppTime();
		float const frame_time = app.FrameTime();

		num_objects_rendered_ = 0;
		num_renderables_rendered_ = 0;
		num_primitives_rendered_ = 0;
		num_vertices_rendered_ = 0;

		Camera& camera = app.ActiveCamera();
		auto const & scene_objs = (urt & App3DFramework::URV_Overlay) ? overlay_scene_objs_ : scene_objs_;

		for (auto const & scene_obj : scene_objs)
		{
			scene_obj->VisibleMark(BO_No);
		}
		if (urt & App3DFramework::URV_NeedFlush)
		{
			frustum_ = &camera.ViewFrustum();

			std::vector<uint32_t> visible_list((scene_objs.size() + 31) / 32, 0);
			for (size_t i = 0; i < scene_objs.size(); ++ i)
			{
				if (scene_objs[i]->Visible())
				{
					visible_list[i / 32] |= (1UL << (i & 31));
				}
			}
			size_t seed = 0;
			HashRange(seed, visible_list.begin(), visible_list.end());
			HashCombine(seed, camera.OmniDirectionalMode());
			HashCombine(seed, &camera);

			auto vmiter = visible_marks_map_.find(seed);
			if (vmiter == visible_marks_map_.end())
			{
				this->ClipScene();

				auto visible_marks = MakeUniquePtr<std::vector<BoundOverlap>>(scene_objs.size());
				for (size_t i = 0; i < scene_objs.size(); ++ i)
				{
					(*visible_marks)[i] = scene_objs[i]->VisibleMark();
				}

				visible_marks_map_.emplace(seed, std::move(visible_marks));
			}
			else
			{
				for (size_t i = 0; i < scene_objs.size(); ++ i)
				{
					scene_objs[i]->VisibleMark((*vmiter->second)[i]);
				}
			}
		}
		if (urt & App3DFramework::URV_Overlay)
		{
			for (auto const & scene_obj : scene_objs)
			{
				scene_obj->MainThreadUpdate(app_time, frame_time);
				scene_obj->VisibleMark(scene_obj->Visible() ? BO_Yes : BO_No);
			}
		}

		for (auto const & obj : scene_objs)
		{
			auto so = obj.get();
			if ((so->VisibleMark() != BO_No) && (0 == so->NumChildren()))
			{
				auto renderable = so->GetRenderable().get();
				if (renderable)
				{
					renderable->ClearInstances();
				}
			}
		}

		for (auto const & obj : scene_objs)
		{
			auto so = obj.get();
			if ((so->VisibleMark() != BO_No) && (0 == so->NumChildren()))
			{
				auto renderable = so->GetRenderable().get();
				if (renderable)
				{
					if (0 == renderable->NumInstances())
					{
						renderable->AddToRenderQueue();
					}
					renderable->AddInstance(so);
					++ num_objects_rendered_;
				}
			}
		}

		std::sort(render_queue_.begin(), render_queue_.end(),
			[](std::pair<RenderTechnique const *, std::vector<Renderable*>> const & lhs,
				std::pair<RenderTechnique const *, std::vector<Renderable*>> const & rhs)
			{
				BOOST_ASSERT(lhs.first);
				BOOST_ASSERT(rhs.first);

				return lhs.first->Weight() < rhs.first->Weight();
			});

		float4 const & view_mat_z = camera.ViewMatrix().Col(2);
		for (auto& items : render_queue_)
		{
			if (!items.first->Transparent() && !items.first->HasDiscard() && (items.second.size() > 1))
			{
				std::vector<std::pair<float, uint32_t>> min_depths(items.second.size());
				for (size_t j = 0; j < min_depths.size(); ++ j)
				{
					Renderable const * renderable = items.second[j];
					AABBox const & box = renderable->PosBound();
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
							float3 const v = box.Corner(k);
							md = std::min(md, v.x() * zvec.x() + v.y() * zvec.y() + v.z() * zvec.z() + zvec.w());
						}
					}

					min_depths[j] = std::make_pair(md, static_cast<uint32_t>(j));
				}

				std::sort(min_depths.begin(), min_depths.end());

				std::vector<Renderable*> sorted_items(min_depths.size());
				for (size_t j = 0; j < min_depths.size(); ++ j)
				{
					sorted_items[j] = items.second[min_depths[j].second];
				}
				items.second.swap(sorted_items);
			}

			for (auto const & item : items.second)
			{
				item->Render();
			}
			num_renderables_rendered_ += static_cast<uint32_t>(items.second.size());
		}
		render_queue_.resize(0);

		num_primitives_rendered_ += re.NumPrimitivesJustRendered();
		num_vertices_rendered_ += re.NumVerticesJustRendered();

		urt_ = 0;
	}

	// 获取渲染的物体数量
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t SceneManager::NumObjectsRendered() const
	{
		return num_objects_rendered_;
	}

	// 获取渲染的可渲染对象数量
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t SceneManager::NumRenderablesRendered() const
	{
		return num_renderables_rendered_;
	}

	// 获取渲染的图元数量
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t SceneManager::NumPrimitivesRendered() const
	{
		return num_primitives_rendered_;
	}

	// 获取渲染的顶点数量
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t SceneManager::NumVerticesRendered() const
	{
		return num_vertices_rendered_;
	}

	uint32_t SceneManager::NumDrawCalls() const
	{
		return num_draw_calls_;
	}

	uint32_t SceneManager::NumDispatchCalls() const
	{
		return num_dispatch_calls_;
	}

	void SceneManager::FlushScene()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		visible_marks_map_.clear();

		uint32_t urt;
		App3DFramework& app = Context::Instance().AppInstance();
		for (uint32_t pass = 0;; ++ pass)
		{
			re.BeginPass();

			urt = app.Update(pass);

			if (urt & App3DFramework::URV_NeedFlush)
			{
				this->Flush(urt);
			}

			re.EndPass();

			if (urt & App3DFramework::URV_Finished)
			{
				break;
			}
		}

		re.PostProcess((urt & App3DFramework::URV_SkipPostProcess) != 0);

		if ((re.Stereo() != STM_None) || (re.DisplayOutput() != DOM_sRGB))
		{
			re.BindFrameBuffer(re.OverlayFrameBuffer());
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);
		}
		this->Flush(App3DFramework::URV_Overlay);

		re.ConvertToDisplay();

		num_draw_calls_ = re.NumDrawsJustCalled();
		num_dispatch_calls_ = re.NumDispatchesJustCalled();
	}

	void SceneManager::UpdateThreadFunc()
	{
		Timer timer;
		float app_time = 0;
		while (!quit_)
		{
			float const frame_time = static_cast<float>(timer.elapsed());
			timer.restart();
			app_time += frame_time;

			if (Context::Instance().AppValid())
			{
				WindowPtr const & win = Context::Instance().AppInstance().MainWnd();
				if (win && win->Active())
				{
					std::lock_guard<std::mutex> lock(update_mutex_);

					for (auto const & scene_obj : scene_objs_)
					{
						scene_obj->SubThreadUpdate(app_time, frame_time);
					}
					for (auto const & scene_obj : overlay_scene_objs_)
					{
						scene_obj->SubThreadUpdate(app_time, frame_time);
					}
				}

				if (frame_time < update_elapse_)
				{
					Sleep(static_cast<uint32_t>((update_elapse_ - frame_time) * 1000));
				}
			}
		}
	}

	BoundOverlap SceneManager::VisibleTestFromParent(SceneObject* obj, float3 const & view_dir, float3 const & eye_pos,
		float4x4 const & view_proj)
	{
		BoundOverlap visible;
		if (obj->Parent())
		{
			BoundOverlap parent_bo = obj->Parent()->VisibleMark();
			if (BO_No == parent_bo)
			{
				visible = BO_No;
			}
			else
			{
				uint32_t const attr = obj->Attrib();
				if (attr & SceneObject::SOA_Moveable)
				{
					obj->UpdateAbsModelMatrix();
				}

				if (attr & SceneObject::SOA_Cullable)
				{
					if (small_obj_threshold_ > 0)
					{
						visible = ((MathLib::ortho_area(view_dir, obj->PosBoundWS()) > small_obj_threshold_)
							&& (MathLib::perspective_area(eye_pos, view_proj, obj->PosBoundWS()) > small_obj_threshold_))
							? parent_bo : BO_No;
					}
					else
					{
						visible = parent_bo;
					}
				}
				else
				{
					visible = parent_bo;
				}
			}
		}
		else
		{
			visible = BO_Partial;
		}

		return visible;
	}
}
