#ifndef _OCTREE_HPP
#define _OCTREE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Box.hpp>
#include <vector>

#include <KlayGE/OCTree/Frustum.hpp>

#pragma comment(lib, "KlayGE_Scene_OCTree.lib")

namespace KlayGE
{
	class OCTreeNode;
	typedef boost::shared_ptr<OCTreeNode> OCTreeNodePtr;

	class OCTreeNode : public SceneNode
	{
	public:
		explicit OCTreeNode(const Box& box);

		void Clear();

		void AddRenderable(const RenderablePtr& renderable);
		bool InsideNode(const RenderablePtr& renderable);

		void Clip(const Frustum& frustum);

		void GetRenderables(std::vector<RenderablePtr>& renderables);

	private:
		OCTreeNodePtr children_[8];

		Box box_;
	};

	class OCTree : public SceneManager
	{
	public:
		OCTree(const Box& box);

		void ClipScene(const Camera& camera);
		void PushRenderable(const RenderablePtr& obj);

	private:
		OCTreeNode root_;

		Frustum frustum_;
	};
}

#endif		// _OCTREE_HPP
