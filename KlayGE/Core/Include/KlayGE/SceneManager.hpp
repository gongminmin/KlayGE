#ifndef _SCENEMANAGER_HPP
#define _SCENEMANAGER_HPP

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/MapVector.hpp>

#include <vector>

#include <boost/utility.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	class Clipper
	{
	public:
		virtual ~Clipper()
			{ }

		static ClipperPtr NullObject();
		virtual void ClipScene(const Camera& camera) = 0;
	};

	class SceneManager : boost::noncopyable
	{
	protected:
		typedef std::vector<RenderablePtr>	RenderItemsType;
		typedef MapVector<RenderEffectPtr, RenderItemsType>			RenderQueueType;

	public:
		static SceneManager& Instance();

		void AttachClipper(const SharedPtr<Clipper>& clipper);

		void PushRenderable(const RenderablePtr& obj);

		void Update();
		void Flash();

	private:
		SceneManager();

	private:
		RenderQueueType renderQueue_;

		SharedPtr<Clipper> clipper_;
	};
}

#endif			// _SCENEMANAGER_HPP
