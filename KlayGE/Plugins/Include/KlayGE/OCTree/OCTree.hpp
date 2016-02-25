// OCTree.hpp
// KlayGE �˲����� ͷ�ļ�
// Ver 3.7.0
// ��Ȩ����(C) ������, 2004-2007
// Homepage: http://www.klayge.org
//
// 3.7.0
// �����˱����ٶ� (2007.12.18)
//
// 2.5.0
// �����˲ü�Ч�� (2005.3.30)
//
// 2.4.0
// �������԰˲��� (2005.3.20)
//
// 2.1.2
// ���ν��� (2004.6.15)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OCTREE_HPP
#define _OCTREE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KFL/AABBox.hpp>

#include <vector>

namespace KlayGE
{
	class OCTree : public SceneManager
	{
	public:
		OCTree();

		void MaxTreeDepth(uint32_t max_tree_depth);
		uint32_t MaxTreeDepth() const;

		virtual void ClipScene() override;

		virtual BoundOverlap AABBVisible(AABBox const & aabb) const override;
		virtual BoundOverlap OBBVisible(OBBox const & obb) const override;
		virtual BoundOverlap SphereVisible(Sphere const & sphere) const override;

		virtual void ClearObject() override;

	private:
		virtual void OnAddSceneObject(SceneObjectPtr const & obj) override;
		virtual void OnDelSceneObject(std::vector<SceneObjectPtr>::iterator iter) override;
		virtual void DoSuspend() override;
		virtual void DoResume() override;

		void DivideNode(size_t index, uint32_t curr_depth);
		void NodeVisible(size_t index);
		void MarkNodeObjs(size_t index, bool force);

		BoundOverlap BoundVisible(size_t index, AABBox const & aabb) const;
		BoundOverlap BoundVisible(size_t index, OBBox const & obb) const;
		BoundOverlap BoundVisible(size_t index, Sphere const & sphere) const;
		BoundOverlap BoundVisible(size_t index, Frustum const & frustum) const;

	private:
		OCTree(OCTree const & rhs);
		OCTree& operator=(OCTree const & rhs);

	private:
		struct octree_node_t
		{
			AABBox bb;
			int first_child_index;
			BoundOverlap visible;

			std::vector<SceneObject*> obj_ptrs;
		};

		std::vector<octree_node_t> octree_;

		uint32_t max_tree_depth_;

		bool rebuild_tree_;

#ifdef KLAYGE_DRAW_NODES
		RenderablePtr node_renderable_;
#endif
	};
}

#endif		// _OCTREE_HPP
