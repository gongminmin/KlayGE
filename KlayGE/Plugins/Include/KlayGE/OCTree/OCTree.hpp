// OCTree.hpp
// KlayGE 八叉树类 头文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.2
// 初次建立 (2004.6.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OCTREE_HPP
#define _OCTREE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Box.hpp>

#include <vector>
#include <boost/array.hpp>

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
		~OCTreeNode();

		void Clear();

		void AddRenderable(const RenderablePtr& renderable);
		bool InsideNode(const RenderablePtr& renderable);

		void Clip(const Frustum& frustum);

		void GetRenderables(std::vector<RenderablePtr>& renderables);

	private:
		typedef boost::array<OCTreeNodePtr, 8> ChildrenType;
		ChildrenType children_;

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
