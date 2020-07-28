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
		: scene_root_(L"SceenRoot", SceneNode::SOA_Cullable),
			overlay_root_(L"OverlayRoot", SceneNode::SOA_Cullable | SceneNode::SOA_Overlay),
			small_obj_threshold_(0),
			update_elapse_(1.0f / 60),
			num_objects_rendered_(0), num_renderables_rendered_(0),
			num_primitives_rendered_(0), num_vertices_rendered_(0),
			num_draw_calls_(0), num_dispatch_calls_(0),
			quit_(false), deferred_mode_(false)
	{
		scene_root_.FillVisibleMark(BoundOverlap::Partial);
		overlay_root_.FillVisibleMark(BoundOverlap::Partial);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	SceneManager::~SceneManager()
	{
		quit_ = true;
		if (update_thread_)
		{
			(*update_thread_)();
		}

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
		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& viewport = *re.CurFrameBuffer()->Viewport();
		uint32_t const num_cameras = viewport.NumCameras();

		for (auto* sn : all_scene_nodes_)
		{
			auto& node = *sn;
			node.FillVisibleMark(BoundOverlap::No);
			if (node.Visible())
			{
				if (node.Updated())
				{
					uint32_t const attr = node.Attrib();

					for (uint32_t i = 0; i < num_cameras; ++i)
					{
						auto const& camera = *viewport.Camera(i);
						float4x4 const& view_proj = camera_view_projs_[i];

						auto visible = this->VisibleTestFromParent(node, i);
						if (BoundOverlap::Partial == visible)
						{
							if (attr & SceneNode::SOA_Cullable)
							{
								visible = (small_obj_threshold_ <= 0) ||
												  ((MathLib::ortho_area(camera.ForwardVec(), node.PosBoundWS()) > small_obj_threshold_) &&
													  (MathLib::perspective_area(camera.EyePos(), view_proj, node.PosBoundWS()) >
														  small_obj_threshold_))
											  ? BoundOverlap::Yes
											  : BoundOverlap::No;
							}
							else
							{
								visible = BoundOverlap::Yes;
							}

							if (!camera.OmniDirectionalMode() && (attr & SceneNode::SOA_Cullable) && (BoundOverlap::Yes == visible))
							{
								visible = camera_frustums_[i]->Intersect(node.PosBoundWS());
							}
						}

						node.VisibleMark(i, visible);
					}
				}
				else
				{
					for (uint32_t i = 0; i < num_cameras; ++i)
					{
						node.VisibleMark(i, BoundOverlap::Yes);
					}
				}
			}
		}
	}

	uint32_t SceneManager::NumFrameCameras() const
	{
		return static_cast<uint32_t>(frame_cameras_.size());
	}

	Camera* SceneManager::GetFrameCamera(uint32_t index)
	{
		return frame_cameras_[index].get();
	}

	Camera const* SceneManager::GetFrameCamera(uint32_t index) const
	{
		return frame_cameras_[index].get();
	}

	uint32_t SceneManager::NumFrameLights() const
	{
		return static_cast<uint32_t>(frame_lights_.size());
	}

	LightSource* SceneManager::GetFrameLight(uint32_t index)
	{
		return frame_lights_[index].get();
	}

	LightSource const* SceneManager::GetFrameLight(uint32_t index) const
	{
		return frame_lights_[index].get();
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
		BoundOverlap ret;
		if (camera_frustums_.empty())
		{
			ret = BoundOverlap::Yes;
		}
		else
		{
			ret = BoundOverlap::No;
			for (auto const* camera_frustum : camera_frustums_)
			{
				ret = std::max(ret, camera_frustum->Intersect(aabb));
				if (ret == BoundOverlap::Yes)
				{
					break;
				}
			}
		}
		return ret;
	}

	BoundOverlap SceneManager::OBBVisible(OBBox const & obb) const
	{
		BoundOverlap ret;
		if (camera_frustums_.empty())
		{
			ret = BoundOverlap::Yes;
		}
		else
		{
			ret = BoundOverlap::No;
			for (auto const* camera_frustum : camera_frustums_)
			{
				ret = std::max(ret, camera_frustum->Intersect(obb));
				if (ret == BoundOverlap::Yes)
				{
					break;
				}
			}
		}
		return ret;
	}

	BoundOverlap SceneManager::SphereVisible(Sphere const & sphere) const
	{
		BoundOverlap ret;
		if (camera_frustums_.empty())
		{
			ret = BoundOverlap::Yes;
		}
		else
		{
			ret = BoundOverlap::No;
			for (auto const* camera_frustum : camera_frustums_)
			{
				ret = std::max(ret, camera_frustum->Intersect(sphere));
				if (ret == BoundOverlap::Yes)
				{
					break;
				}
			}
		}
		return ret;
	}

	BoundOverlap SceneManager::FrustumVisible(Frustum const & frustum) const
	{
		BoundOverlap ret;
		if (camera_frustums_.empty())
		{
			ret = BoundOverlap::Yes;
		}
		else
		{
			ret = BoundOverlap::No;
			for (auto const* camera_frustum : camera_frustums_)
			{
				ret = std::max(ret, camera_frustum->Intersect(frustum));
				if (ret == BoundOverlap::Yes)
				{
					break;
				}
			}
		}
		return ret;
	}

	void SceneManager::ClearObject()
	{
		std::lock_guard<std::mutex> lock(update_mutex_);
		scene_root_.ClearChildren();
		overlay_root_.ClearChildren();
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

		if (!update_thread_ && !quit_)
		{
			update_thread_ = MakeUniquePtr<joiner<void>>(Context::Instance().ThreadPool()(
				[this] { this->UpdateThreadFunc(); }));
		}

		{
			std::lock_guard<std::mutex> lock(update_mutex_);

			scene_root_.Traverse([this, app_time, frame_time](SceneNode& node) {
				node.MainThreadUpdate(app_time, frame_time);
				node.UpdateTransforms();

				if (node.Visible())
				{
					node.ForEachComponentOfType<Camera>([this](Camera& camera) {
						frame_cameras_.push_back(camera.shared_from_this());
					});

					node.ForEachComponentOfType<LightSource>([this](LightSource& light) {
						frame_lights_.push_back(light.shared_from_this());
					});
				}

				return true;
			});
			scene_root_.UpdatePosBoundSubtree();

			overlay_root_.ClearChildren();
		}

		nodes_updated_ = true;

		this->FlushScene();

		FrameBuffer& fb = *re.ScreenFrameBuffer();
		fb.SwapBuffers();

		InputEngine& ie = Context::Instance().InputFactoryInstance().InputEngineInstance();
		ie.Update();

		frame_cameras_.clear();
		frame_lights_.clear();

		fb.WaitOnSwapBuffers();

		re.EndFrame();

		nodes_updated_ = false;
	}

	// 把渲染队列中的物体渲染出来
	/////////////////////////////////////////////////////////////////////////////////
	void SceneManager::Flush(uint32_t urt)
	{
		std::lock_guard<std::mutex> lock(update_mutex_);

		urt_ = urt;

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& viewport = *re.CurFrameBuffer()->Viewport();
		uint32_t const num_cameras = viewport.NumCameras();

		App3DFramework& app = Context::Instance().AppInstance();
		float const app_time = app.AppTime();
		float const frame_time = app.FrameTime();

		num_objects_rendered_ = 0;
		num_renderables_rendered_ = 0;
		num_primitives_rendered_ = 0;
		num_vertices_rendered_ = 0;

		scene_root_.Traverse([this](SceneNode& node)
			{
				all_scene_nodes_.push_back(&node);
				return true;
			});
		overlay_root_.Traverse([this](SceneNode& node)
			{
				all_overlay_nodes_.push_back(&node);
				return true;
			});

		auto& scene_nodes = (urt & App3DFramework::URV_Overlay) ? all_overlay_nodes_ : all_scene_nodes_;

		for (auto* node : scene_nodes)
		{
			node->FillVisibleMark(BoundOverlap::No);
		}
		if (!(urt & App3DFramework::URV_Overlay))
		{
			scene_root_.Traverse([num_cameras](SceneNode& node)
				{
					uint32_t const attr = node.Attrib();
					if ((node.Parent() == nullptr)
						|| (node.Visible() && (!(attr & SceneNode::SOA_Cullable) || (attr & SceneNode::SOA_Moveable))))
					{
						for (uint32_t i = 0; i < num_cameras; ++i)
						{
							node.VisibleMark(i, BoundOverlap::Partial);
						}
					}
					return node.Visible();
				});
		}
		if (urt & App3DFramework::URV_NeedFlush)
		{
			camera_frustums_.resize(viewport.NumCameras());
			for (uint32_t i = 0; i < viewport.NumCameras(); ++i)
			{
				camera_frustums_[i] = &viewport.Camera(i)->ViewFrustum();
			}

			std::vector<uint32_t> visible_list((scene_nodes.size() + 31) / 32, 0);
			for (size_t i = 0; i < scene_nodes.size(); ++ i)
			{
				if (scene_nodes[i]->Visible())
				{
					visible_list[i / 32] |= (1UL << (i & 31));
				}
			}
			size_t seed = 0;
			HashRange(seed, visible_list.begin(), visible_list.end());
			for (uint32_t i = 0; i < viewport.NumCameras(); ++i)
			{
				auto const& camera = *viewport.Camera(i);
				HashCombine(seed, camera.OmniDirectionalMode());
				HashCombine(seed, &camera);
			}

			auto vmiter = visible_marks_map_.find(seed);
			if (vmiter == visible_marks_map_.end())
			{
				camera_view_projs_.resize(viewport.NumCameras());
				for (uint32_t i = 0; i < viewport.NumCameras(); ++i)
				{
					camera_view_projs_[i] = viewport.Camera(i)->ViewProjMatrix();
				}
				auto drl = Context::Instance().DeferredRenderingLayerInstance();
				if (drl)
				{
					int32_t cas_index = drl->CurrCascadeIndex();
					if (cas_index >= 0)
					{
						float4x4 const& ccm = drl->GetCascadedShadowLayer().CascadeCropMatrix(cas_index);
						for (uint32_t i = 0; i < viewport.NumCameras(); ++i)
						{
							camera_view_projs_[i] *= ccm;
						}
					}
				}

				this->ClipScene();

				auto visible_marks =
					MakeUniquePtr<std::array<BoundOverlap, RenderEngine::PredefinedCameraCBuffer::max_num_cameras>[]>(scene_nodes.size());
				for (size_t i = 0; i < scene_nodes.size(); ++ i)
				{
					for (uint32_t j = 0; j < num_cameras; ++j)
					{
						visible_marks[i][j] = scene_nodes[i]->VisibleMark(j);
					}
				}

				visible_marks_map_.emplace(seed, std::move(visible_marks));
			}
			else
			{
				for (size_t i = 0; i < scene_nodes.size(); ++ i)
				{
					for (uint32_t j = 0; j < num_cameras; ++j)
					{
						scene_nodes[i]->VisibleMark(j, vmiter->second[i][j]);
					}
				}
			}
		}
		if (urt & App3DFramework::URV_Overlay)
		{
			for (auto const & scene_node : scene_nodes)
			{
				scene_node->MainThreadUpdate(app_time, frame_time);
				for (uint32_t j = 0; j < num_cameras; ++j)
				{
					scene_node->VisibleMark(j, scene_node->Visible() ? BoundOverlap::Yes : BoundOverlap::No);
				}
			}
		}

		auto node_visible = MakeUniquePtr<bool[]>(scene_nodes.size());
		for (size_t i = 0; i < scene_nodes.size(); ++i)
		{
			node_visible[i] = false;
			for (uint32_t j = 0; j < num_cameras; ++j)
			{
				if (scene_nodes[i]->VisibleMark(j) != BoundOverlap::No)
				{
					node_visible[i] = true;
					break;
				}
			}
		}

		for (size_t i = 0; i < scene_nodes.size(); ++i)
		{
			if (node_visible[i])
			{
				scene_nodes[i]->ForEachComponentOfType<RenderableComponent>(
					[](RenderableComponent& renderable_comp) { renderable_comp.BoundRenderable().ClearInstances(); });
			}
		}

		for (size_t i = 0; i < scene_nodes.size(); ++i)
		{
			if (node_visible[i])
			{
				auto* node = scene_nodes[i];
				node->ForEachComponentOfType<RenderableComponent>([node](RenderableComponent& renderable_comp) {
					auto& renderable = renderable_comp.BoundRenderable();
					if (renderable_comp.Enabled() && (renderable.GetRenderTechnique() != nullptr))
					{
						if (0 == renderable.NumInstances())
						{
							renderable.AddToRenderQueue();
						}
						renderable.AddInstance(node);
					}
				});

				++ num_objects_rendered_;
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

		float4 view_mat_z;
		if (viewport.NumCameras() == 1)
		{
			view_mat_z = viewport.Camera(0)->ViewMatrix().Col(2);
		}
		for (auto& items : render_queue_)
		{
			if ((viewport.NumCameras() == 1) && !items.first->Transparent() && !items.first->HasDiscard() && (items.second.size() > 1))
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
						float4x4 const & mat = renderable->GetInstance(i)->TransformToWorld();
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

		all_scene_nodes_.clear();
		all_overlay_nodes_.clear();

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

					auto updater = [app_time, frame_time](SceneNode& node) {
						node.SubThreadUpdate(app_time, frame_time);
						return true;
					};
					scene_root_.Traverse(updater);
					overlay_root_.Traverse(updater);
				}

				if (frame_time < update_elapse_)
				{
					Sleep(static_cast<uint32_t>((update_elapse_ - frame_time) * 1000));
				}
			}
		}
	}

	BoundOverlap SceneManager::VisibleTestFromParent(SceneNode const & node, uint32_t camera_index)
	{
		BoundOverlap visible;
		if (node.Parent())
		{
			BoundOverlap const parent_bo = node.Parent()->VisibleMark(camera_index);
			if (BoundOverlap::No == parent_bo)
			{
				visible = BoundOverlap::No;
			}
			else
			{
				uint32_t const attr = node.Attrib();
				if (attr & SceneNode::SOA_Cullable)
				{
					if (small_obj_threshold_ > 0)
					{
						auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
						auto const& viewport = *re.CurFrameBuffer()->Viewport();
						auto const& camera = *viewport.Camera(camera_index);
						float4x4 const& view_proj = camera_view_projs_[camera_index];

						visible = ((MathLib::ortho_area(camera.ForwardVec(), node.PosBoundWS()) > small_obj_threshold_)
							&& (MathLib::perspective_area(camera.EyePos(), view_proj, node.PosBoundWS()) > small_obj_threshold_))
							? parent_bo : BoundOverlap::No;
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
			visible = BoundOverlap::Partial;
		}

		return visible;
	}
}
