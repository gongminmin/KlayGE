#ifndef _SCENEMANAGER_HPP
#define _SCENEMANAGER_HPP

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/alloc.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/MathTypes.hpp>
#include <KlayGE/MapVector.hpp>

#include <vector>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	class SceneManager
	{
	protected:
		typedef std::vector<RenderablePtr, alloc<RenderablePtr> >	RenderItemsType;
		typedef MapVector<RenderEffectPtr, RenderItemsType>			RenderQueueType;

	public:
		SceneManager();
		virtual ~SceneManager()
			{ }

		void PushRenderable(const RenderablePtr& obj);

		virtual void ClipScene()
			{ }

		void Update();
		virtual void Flash();

	protected:
		RenderQueueType renderQueue_;

	private:
		SceneManager(const SceneManager&);
		SceneManager& operator=(const SceneManager&);
	};
}

#endif			// _SCENEMANAGER_HPP