#ifndef _SCENEMANAGER_HPP
#define _SCENEMANAGER_HPP

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/Renderable.hpp>
#include <KlayGE/MapVector.hpp>

#include <vector>

#include <boost/utility.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	class SceneManager : boost::noncopyable
	{
	protected:
		typedef std::vector<RenderablePtr> RenderItemsType;
		typedef MapVector<RenderEffectPtr, RenderItemsType> RenderQueueType;

	public:
		SceneManager();

		virtual void ClipScene(const Camera& camera);

		virtual void PushRenderable(const RenderablePtr& obj);

		void Update();
		void Flash();

	protected:
		RenderQueueType renderQueue_;
	};
}

#endif			// _SCENEMANAGER_HPP
