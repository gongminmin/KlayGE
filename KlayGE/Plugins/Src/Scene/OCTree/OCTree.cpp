#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Plane.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

namespace KlayGE
{
	OCTree::OCTree(const Box& box)
		: root_(box)
	{
	}

	void OCTree::ClipScene(const Camera& camera)
	{
		frustum_.CalculateFrustum(camera.ViewMatrix(), camera.ProjMatrix());

		root_.Clip(frustum_);

		std::vector<RenderablePtr> renderables;
		root_.GetRenderables(renderables);

		renderQueue_.clear();

		for (std::vector<RenderablePtr>::iterator iter = renderables.begin();
			iter != renderables.end(); ++ iter)
		{
			SceneManager::PushRenderable(*iter);
		}

		root_.Clear();
	}

	void OCTree::PushRenderable(const RenderablePtr& obj)
	{
		root_.AddRenderable(obj);
	}
}
