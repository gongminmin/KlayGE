// SceneManager.hpp
// KlayGE 场景管理器类 头文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2003-2010
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
#include <KlayGE/MapVector.hpp>
#include <KlayGE/Frustum.hpp>

#include <vector>

#include <boost/noncopyable.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneManager : boost::noncopyable
	{
	public:
		typedef std::vector<SceneObjectPtr> SceneObjectsType;

	protected:
		typedef std::vector<RenderablePtr> RenderItemsType;
		typedef std::vector<std::pair<RenderTechniquePtr, RenderItemsType> > RenderQueueType;

	public:
		SceneManager();
		virtual ~SceneManager();

		static SceneManagerPtr NullObject();

		virtual void ClipScene();
		void AddSceneObject(SceneObjectPtr const & obj);
		SceneObjectsType::iterator DelSceneObject(SceneObjectsType::iterator iter);
		void AddRenderable(RenderablePtr const & obj);

		SceneObjectsType& SceneObjects();
		SceneObjectsType const & SceneObjects() const;

		virtual bool AABBVisible(Box const & box);

		virtual void Clear();

		void Update();

		size_t NumObjectsRendered() const;
		size_t NumRenderablesRendered() const;
		size_t NumPrimitivesRendered() const;
		size_t NumVerticesRendered() const;

	protected:
		void Flush();

		virtual void OnAddSceneObject(SceneObjectPtr const & obj) = 0;
		virtual void OnDelSceneObject(SceneObjectsType::iterator iter) = 0;

	protected:
		Frustum frustum_;
		SceneObjectsType scene_objs_;
		std::vector<boost::shared_ptr<Box> > scene_obj_bbs_;
		std::vector<char> visible_marks_;
		uint32_t urt_;

	private:
		void FlushScene();

	private:
		RenderQueueType render_queue_;

		size_t numObjectsRendered_;
		size_t numRenderablesRendered_;
		size_t numPrimitivesRendered_;
		size_t numVerticesRendered_;
	};
}

#endif			// _SCENEMANAGER_HPP
