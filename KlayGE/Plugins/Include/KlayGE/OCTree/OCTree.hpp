// OCTree.hpp
// KlayGE 八叉树类 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
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

#define KLAYGE_LIB_NAME KlayGE_Scene_OCTree
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Box.hpp>
#include <KlayGE/ClosedHashSet.hpp>
#include <KlayGE/OCTree/Frustum.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif
#include <boost/functional/hash.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <vector>
#include <map>
#include <string>

namespace KlayGE
{
	class OCTree : public SceneManager
	{
	public:
		explicit OCTree(uint32_t max_tree_depth);

	private:
		void ClipScene(Camera const & camera);
		void Clear();

		void DoAddSceneObject(SceneObjectPtr const & obj);
		SceneObjectsType::iterator DoDelSceneObject(SceneObjectsType::iterator iter);

		void Visit(size_t index, Frustum const & frustum);

	private:
		OCTree(OCTree const & rhs);
		OCTree& operator=(OCTree const & rhs);

	private:
		typedef std::vector<size_t, boost::pool_allocator<size_t> > ObjIndicesTypes;
		typedef std::vector<Box, boost::pool_allocator<Box> > AABBsTypes;
		struct octree_node_t
		{
			Box bounding_box;

			ObjIndicesTypes obj_indices;

			int parent_index;
			int first_child_index;
		};

		std::vector<octree_node_t, boost::pool_allocator<octree_node_t> > octree_;
		std::vector<size_t, boost::pool_allocator<size_t> > base_address_;
		AABBsTypes aabbs_in_ws_;

		uint32_t max_tree_depth_;

		bool rebuild_tree_;

		closed_hash_set<size_t, boost::hash<size_t>, std::equal_to<size_t>, boost::pool_allocator<size_t> > visables_set_;
	};
}

#endif		// _OCTREE_HPP
