#ifndef _SCENENODE_HPP
#define _SCENENODE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <vector>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneNode
	{
	public:
		virtual ~SceneNode()
			{ }

		virtual void AddRenderable(Renderable* renderable)
			{ renderables_.push_back(renderable); }

	protected:
		std::vector<Renderable*> renderables_;
	};
}

#endif		// _SCENENODE_HPP
