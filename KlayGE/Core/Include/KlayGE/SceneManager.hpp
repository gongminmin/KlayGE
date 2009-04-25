// SceneManager.hpp
// KlayGE 场景管理器类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

#include <vector>

#include <boost/utility.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneManager : boost::noncopyable
	{
	protected:
		typedef std::vector<SceneObjectPtr> SceneObjectsType;

		typedef std::vector<RenderablePtr> RenderItemsType;
		typedef std::vector<std::pair<RenderTechniquePtr, RenderItemsType> > RenderQueueType;

	public:
		SceneManager();
		virtual ~SceneManager();

		static SceneManagerPtr NullObject();

		virtual void ClipScene(Camera const & camera) = 0;
		void AddSceneObject(SceneObjectPtr const & obj);
		void AddRenderable(RenderablePtr const & obj);

		virtual void Clear() = 0;

		void Update();
		void Flush();

		size_t NumObjectsRendered() const;
		size_t NumRenderablesRendered() const;
		size_t NumPrimitivesRendered() const;
		size_t NumVerticesRendered() const;

	protected:
		virtual void DoAddSceneObject(SceneObjectPtr const & obj) = 0;
		virtual SceneObjectsType::iterator DoDelSceneObject(SceneObjectsType::iterator iter) = 0;

	protected:
		SceneObjectsType scene_objs_;
		std::vector<char> visible_marks_;
		size_t start_index_;

	private:
		RenderQueueType render_queue_;

		size_t numObjectsRendered_;
		size_t numRenderablesRendered_;
		size_t numPrimitivesRendered_;
		size_t numVerticesRendered_;
	};
}

#endif			// _SCENEMANAGER_HPP
