// OCTree.hpp
// KlayGE 八叉树类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2004-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 提升了遍历速度 (2007.12.18)
//
// 2.5.0
// 提升了裁剪效率 (2005.3.30)
//
// 2.4.0
// 改用线性八叉树 (2005.3.20)
//
// 2.1.2
// 初次建立 (2004.6.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OCTREE_HPP
#define _OCTREE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Box.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <vector>

namespace KlayGE
{
	class OCTree : public SceneManager
	{
	public:
		explicit OCTree(uint32_t max_tree_depth);

		bool AABBVisible(Box const & box);

	private:
		void ClipScene();
		void Clear();

		void DoAddSceneObject(SceneObjectPtr const & obj);
		SceneObjectsType::iterator DoDelSceneObject(SceneObjectsType::iterator iter);

		void NodeVisible(size_t index);
		bool BBVisible(size_t index, float3 const & bb_center, float3 const & bb_half_size);

	private:
		OCTree(OCTree const & rhs);
		OCTree& operator=(OCTree const & rhs);

	private:
		struct octree_node_t
		{
			float3 bb_center;
			float3 bb_half_size;

			int first_child_index;

			Frustum::VIS visible;
		};

		std::vector<octree_node_t, boost::pool_allocator<octree_node_t> > octree_;
		std::vector<size_t, boost::pool_allocator<size_t> > base_address_;

		uint32_t max_tree_depth_;

		bool rebuild_tree_;

#ifdef KLAYGE_DEBUG
		RenderablePtr node_renderable_;
#endif
	};
}

#endif		// _OCTREE_HPP
