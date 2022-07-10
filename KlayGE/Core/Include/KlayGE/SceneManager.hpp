// SceneManager.hpp
// KlayGE ������������ ͷ�ļ�
// Ver 3.12.0
// ��Ȩ����(C) ������, 2003-2011
// Homepage: http://www.klayge.org
//
// 3.11.0
// ��DoAddSceneObject/DoDelSceneObject��ΪOnAddSceneObject/OnDelSceneObject (2010.11.16)
//
// 3.9.0
// ������SceneObjects (2009.7.30)
//
// 2.4.0
// ������NumObjectsRendered��NumPrimitivesRendered��NumVerticesRendered (2005.3.20)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SCENEMANAGER_HPP
#define _SCENEMANAGER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/SceneNode.hpp>
#include <KlayGE/Renderable.hpp>
#include <KFL/Frustum.hpp>
#include <KFL/Thread.hpp>

#include <optional>
#include <vector>
#include <unordered_map>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneManager : boost::noncopyable
	{
	public:
		SceneManager();
		virtual ~SceneManager();

		void Suspend();
		void Resume();

		void SmallObjectThreshold(float area);
		void SceneUpdateElapse(float elapse);
		virtual void ClipScene();

		uint32_t NumFrameCameras() const;
		Camera* GetFrameCamera(uint32_t index);
		Camera const* GetFrameCamera(uint32_t index) const;

		uint32_t NumFrameLights() const;
		LightSource* GetFrameLight(uint32_t index);
		LightSource const* GetFrameLight(uint32_t index) const;

		SceneNode& SceneRootNode()
		{
			return scene_root_;
		}
		SceneNode const & SceneRootNode() const
		{
			return scene_root_;
		}

		SceneNode& OverlayRootNode()
		{
			return overlay_root_;
		}
		SceneNode const & OverlayRootNode() const
		{
			return overlay_root_;
		}

		std::mutex& MutexForUpdate()
		{
			return update_mutex_;
		}

		void AddRenderable(Renderable* node);

		virtual BoundOverlap AABBVisible(AABBox const & aabb) const;
		virtual BoundOverlap OBBVisible(OBBox const & obb) const;
		virtual BoundOverlap SphereVisible(Sphere const & sphere) const;
		virtual BoundOverlap FrustumVisible(Frustum const & frustum) const;

		virtual void ClearObject();

		void Update();

		uint32_t NumObjectsRendered() const;
		uint32_t NumRenderablesRendered() const;
		uint32_t NumPrimitivesRendered() const;
		uint32_t NumVerticesRendered() const;
		uint32_t NumDrawCalls() const;
		uint32_t NumDispatchCalls() const;

		virtual void OnSceneChanged() = 0;

		bool NodesUpdated() const
		{
			return nodes_updated_;
		}

	protected:
		void Flush(uint32_t urt);

		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

		void UpdateThreadFunc();

		BoundOverlap VisibleTestFromParent(SceneNode const & node, uint32_t camera_index);

	protected:
		std::vector<CameraPtr> frame_cameras_;
		std::vector<Frustum const*> camera_frustums_;
		std::vector<float4x4> camera_view_projs_;
		std::vector<LightSourcePtr> frame_lights_;
		SceneNode scene_root_;
		SceneNode overlay_root_;

		std::unordered_map<size_t, std::unique_ptr<std::array<BoundOverlap, RenderEngine::PredefinedCameraCBuffer::max_num_cameras>[]>>
			visible_marks_map_;

		float small_obj_threshold_;
		float update_elapse_;

		std::vector<SceneNode*> all_scene_nodes_;
		std::vector<SceneNode*> all_overlay_nodes_;

	private:
		void FlushScene();

	private:
		uint32_t urt_;

		std::vector<std::pair<RenderTechnique const *, std::vector<Renderable*>>> render_queue_;

		uint32_t num_objects_rendered_;
		uint32_t num_renderables_rendered_;
		uint32_t num_primitives_rendered_;
		uint32_t num_vertices_rendered_;
		uint32_t num_draw_calls_;
		uint32_t num_dispatch_calls_;

		std::mutex update_mutex_;
		std::optional<std::future<void>> update_thread_;
		volatile bool quit_;

		bool deferred_mode_;

		bool nodes_updated_ = false;
	};
}

#endif			// _SCENEMANAGER_HPP
