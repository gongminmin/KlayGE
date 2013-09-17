// SceneManager.hpp
// KlayGE 场景管理器类 头文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2003-2011
// Homepage: http://www.klayge.org
//
// 3.11.0
// 把DoAddSceneObject/DoDelSceneObject改为OnAddSceneObject/OnDelSceneObject (2010.11.16)
//
// 3.9.0
// 增加了SceneObjects (2009.7.30)
//
// 2.4.0
// 增加了NumObjectsRendered，NumPrimitivesRendered和NumVerticesRendered (2005.3.20)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SCENEMANAGER_HPP
#define _SCENEMANAGER_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/Renderable.hpp>
#include <KFL/Frustum.hpp>
#include <KFL/Thread.hpp>

#include <vector>

#include <boost/noncopyable.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneManager : boost::noncopyable
	{
	protected:
		struct SceneObjAABB
		{
			SceneObjectPtr so;
			AABBoxPtr aabb_ws;
			bool visible;

			SceneObjAABB(SceneObjectPtr const & s, AABBoxPtr const & abw, bool v)
				: so(s), aabb_ws(abw), visible(v)
			{
			}
		};

		typedef shared_ptr<SceneObjAABB> SceneObjAABBPtrType;
		typedef std::vector<SceneObjAABBPtrType> SceneObjAABBsType;
		typedef std::vector<RenderablePtr> RenderItemsType;
		typedef std::vector<std::pair<RenderTechniquePtr, RenderItemsType> > RenderQueueType;

	public:
		SceneManager();
		virtual ~SceneManager();

		void SmallObjectThreshold(float area);
		void SceneUpdateElapse(float elapse);
		virtual void ClipScene();

		void AddCamera(CameraPtr const & camera);
		void DelCamera(CameraPtr const & camera);

		uint32_t NumCameras() const;
		CameraPtr& GetCamera(uint32_t index);
		CameraPtr const & GetCamera(uint32_t index) const;

		void AddLight(LightSourcePtr const & light);
		void DelLight(LightSourcePtr const & light);

		uint32_t NumLights() const;
		LightSourcePtr& GetLight(uint32_t index);
		LightSourcePtr const & GetLight(uint32_t index) const;

		void AddSceneObject(SceneObjectPtr const & obj);
		void DelSceneObject(SceneObjectPtr const & obj);
		void AddRenderable(RenderablePtr const & obj);

		uint32_t NumSceneObjects() const;
		SceneObjectPtr& GetSceneObject(uint32_t index);
		SceneObjectPtr const & GetSceneObject(uint32_t index) const;

		virtual bool AABBVisible(AABBox const & aabb);
		virtual bool OBBVisible(OBBox const & obb);
		virtual bool SphereVisible(Sphere const & sphere);
		virtual bool FrustumVisible(Frustum const & frustum);

		virtual void ClearCamera();
		virtual void ClearLight();
		virtual void ClearObject();

		void Update();

		size_t NumObjectsRendered() const;
		size_t NumRenderablesRendered() const;
		size_t NumPrimitivesRendered() const;
		size_t NumVerticesRendered() const;

	protected:
		void Flush(uint32_t urt);

		std::vector<CameraPtr>::iterator DelCamera(std::vector<CameraPtr>::iterator iter);
		std::vector<LightSourcePtr>::iterator DelLight(std::vector<LightSourcePtr>::iterator iter);
		SceneObjAABBsType::iterator DelSceneObject(SceneObjAABBsType::iterator iter);
		virtual void OnAddSceneObject(SceneObjectPtr const & obj) = 0;
		virtual void OnDelSceneObject(SceneObjAABBsType::iterator iter) = 0;

		void UpdateThreadFunc();

	protected:
		std::vector<CameraPtr> cameras_;
		Frustum const * frustum_;
		std::vector<LightSourcePtr> lights_;
		SceneObjAABBsType scene_objs_;

		unordered_map<size_t, shared_ptr<std::vector<char> > > visible_marks_map_;

		float small_obj_threshold_;
		float update_elapse_;

	private:
		void FlushScene();

	private:
		uint32_t urt_;

		RenderQueueType render_queue_;

		size_t numObjectsRendered_;
		size_t numRenderablesRendered_;
		size_t numPrimitivesRendered_;
		size_t numVerticesRendered_;

		mutex update_mutex_;
		shared_ptr<joiner<void> > update_thread_;
		bool quit_;
	};
}

#endif			// _SCENEMANAGER_HPP
