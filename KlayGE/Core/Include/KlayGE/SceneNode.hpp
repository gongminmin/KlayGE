#ifndef _SCENENODE_HPP
#define _SCENENODE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <vector>

namespace KlayGE
{
	class SceneNode
	{
	public:
		virtual ~SceneNode()
			{ }

		virtual void AddRenderable(RenderablePtr const & renderable)
			{ renderables_.push_back(renderable); }

	protected:
		typedef std::vector<RenderablePtr> RenderablesType;
		RenderablesType renderables_;
	};
}

#endif		// _SCENENODE_HPP
