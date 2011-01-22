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
#include <KlayGE/MapVector.hpp>
#include <KlayGE/Frustum.hpp>

#include <vector>

#include <boost/noncopyable.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011 6334)
#endif
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

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

		void AddLight(LightSourcePtr const & light);
		void DelLight(LightSourcePtr const & light);

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
		Frustum const * frustum_;
		std::vector<LightSourcePtr> lights_;
		SceneObjectsType scene_objs_;
		std::vector<boost::shared_ptr<Box> > scene_obj_bbs_;
		boost::shared_ptr<std::vector<char> > visible_marks_;
		uint32_t urt_;

		boost::unordered_map<size_t, boost::shared_ptr<std::vector<char> >, boost::hash<size_t>, std::equal_to<size_t>,
			boost::fast_pool_allocator<std::pair<size_t, boost::shared_ptr<std::vector<char> > > > > visible_marks_map_;

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
