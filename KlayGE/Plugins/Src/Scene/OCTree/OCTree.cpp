// OCTree.cpp
// KlayGE 八叉树类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 保证了绘制顺序 (2005.8.17)
//
// 2.6.0
// 修正了CanBeCulled的bug (2005.5.26)
//
// 2.4.0
// 改用线性八叉树 (2005.3.20)
//
// 2.1.2
// 初次建立 (2004.6.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Plane.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SetVector.hpp>

#include <functional>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/OCTree/Frustum.hpp>
#include <KlayGE/OCTree/OCTree.hpp>

namespace
{
	KlayGE::OCTree::tree_id_t const ROOT_ID = 0x0FFFFFFFFFFFFFFFULL;
	int const NUM_BITS = sizeof(KlayGE::OCTree::tree_id_t) * 8;
}

namespace KlayGE
{
	OCTree::OCTree(uint32_t max_tree_depth)
		: root_box_(Box(float3(0, 0, 0), float3(0, 0, 0))),
			max_tree_depth_(std::min(max_tree_depth, 16UL)),
			rebuild_tree_(false)
	{
	}

	void OCTree::ClipScene(Camera const & camera)
	{
		if (rebuild_tree_)
		{
			octree_.clear();
			root_box_ = Box(float3(0, 0, 0), float3(0, 0, 0));
			BOOST_FOREACH(BOOST_TYPEOF(scene_objs_)::reference obj, scene_objs_)
			{
				if (obj->Cullable() && !obj->ShortAge())
				{
					Box const & box = obj->GetBound();
					float4x4 const & mat = obj->GetModelMatrix();

					float3 min(1e10f, 1e10f, 1e10f), max(-1e10f, -1e10f, -1e10f);
					for (size_t i = 0; i < 8; ++ i)
					{
						float3 vec = MathLib::transform_coord(box[i], mat);
						min = MathLib::minimize(min, vec);
						max = MathLib::maximize(max, vec);
					}

					Box bb_in_ws(min, max);

					root_box_ |= bb_in_ws;
					octree_[ROOT_ID].objs.push_back(obj);
					octree_[ROOT_ID].bbs_in_ws.push_back(bb_in_ws);
				}
			}
			{
				float3 const & size = root_box_.Max() - root_box_.Min();
				float max_dim = std::max(std::max(size.x(), size.y()), size.z()) / 2;
				float3 const & center = root_box_.Center();
				root_box_ = Box(center - float3(max_dim, max_dim, max_dim),
					center + float3(max_dim, max_dim, max_dim));
			}

			for (uint32_t d = 0; d < max_tree_depth_; ++ d)
			{
				for (BOOST_AUTO(iter, octree_.begin()); iter != octree_.end();)
				{
					if (iter->second.objs.size() > 1)
					{
						tree_id_t old_id = iter->first;
						SceneObjectsType& old_objs = iter->second.objs;
						BoxesTypes& old_bbs = iter->second.bbs_in_ws;

						int offset = NUM_BITS - 8;
						while (0 == ((old_id >> offset) & 0x8))
						{
							offset -= 4;
						}

						for (tree_id_t i = 0; i < 8; ++ i)
						{
							tree_id_t id = old_id & (~(tree_id_t(0xF) << offset)) | (i << offset);
							Box const & old_box = this->AreaBox(id);
							float3 const & old_center = old_box.Center();
							float3 const & old_half_size = old_box.HalfSize();

							octree_.insert(std::make_pair(id, octree_node_t()));
							octree_node_t& new_node = octree_[id];

							for (size_t j = 0; j < old_objs.size(); ++ j)
							{
								SceneObjectPtr const & old_obj = old_objs[j];
								Box const & bb = old_bbs[j];

								float3 const t = bb.Center() - old_center;
								float3 const e = bb.HalfSize() + old_half_size;
								if ((abs(t.x()) <= e.x()) && (abs(t.y()) <= e.y()) && (abs(t.y()) <= e.y()))
								{
									new_node.objs.push_back(old_obj);
									new_node.bbs_in_ws.push_back(bb);
								}
							}
						}

						iter = octree_.erase(iter);
					}
					else
					{
						++ iter;
					}
				}
			}

			for (BOOST_AUTO(iter, octree_.begin()); iter != octree_.end();)
			{
				SceneObjectsType& objs = iter->second.objs;
				if (objs.empty())
				{
					octree_.erase(iter ++);
				}
				else
				{
					++ iter;
				}
			}

			rebuild_tree_ = false;
		}

		std::vector<tree_id_t, boost::pool_allocator<tree_id_t> > id_in_tree;
		for (BOOST_AUTO(iter, octree_.begin()); iter != octree_.end(); ++ iter)
		{
			id_in_tree.push_back(iter->first);
		}

		Frustum frustum(camera.ViewMatrix() * camera.ProjMatrix());

		std::vector<tree_id_t, boost::pool_allocator<tree_id_t> > filter_queue(1, ROOT_ID);
		for (uint32_t d = 0; d < max_tree_depth_; ++ d)
		{
			int const offset = NUM_BITS - 8 - 4 * d;
			size_t original_size = filter_queue.size();
			for (size_t i = 0; i < original_size; ++ i)
			{
				for (tree_id_t j = 0; j < 8; ++ j)
				{
					filter_queue.push_back(filter_queue[i] & (~(tree_id_t(0xF) << offset)) | (j << offset));
				}
			}
			filter_queue.erase(filter_queue.begin(), filter_queue.begin() + original_size);

			for (BOOST_AUTO(iter, filter_queue.begin()); iter != filter_queue.end();)
			{
				Box const & box = this->AreaBox(*iter);

				Frustum::VIS const vis = frustum.Visiable(box);
				if (Frustum::VIS_NO == vis)
				{
					BOOST_AUTO(end_iter, std::lower_bound(id_in_tree.begin(), id_in_tree.end(), *iter));
					if ((end_iter != id_in_tree.end()) && (*end_iter == *iter))
					{
						++ end_iter;
					}

					tree_id_t beg_id = *iter;
					for (uint32_t i = d + 1; i < max_tree_depth_; ++ i)
					{
						beg_id = beg_id & (~(tree_id_t(0xF) << ((NUM_BITS - 8 - 4 * i) - 4)));
					}
					BOOST_AUTO(beg_iter, std::lower_bound(id_in_tree.begin(), id_in_tree.end(), beg_id));
					if (beg_iter != id_in_tree.end())
					{
						if ((end_iter == id_in_tree.end())
							|| ((end_iter != id_in_tree.end()) && (*beg_iter < *end_iter)))
						{
							id_in_tree.erase(beg_iter, end_iter);
							iter = filter_queue.erase(iter);
						}
						else
						{
							 ++ iter;
						}
					}
					else
					{
						 ++ iter;
					}
				}
				else
				{
					 ++ iter;
				}
			}
		}

		SetVector<SceneObjectPtr, std::less<SceneObjectPtr>, boost::pool_allocator<SceneObjectPtr> > visables;
		BOOST_FOREACH(BOOST_TYPEOF(id_in_tree)::reference node_id, id_in_tree)
		{
			visables.insert(octree_[node_id].objs.begin(), octree_[node_id].objs.end());

#ifdef KLAYGE_DEBUG
			RenderablePtr box_helper(new RenderableLineBox(this->AreaBox(node_id), Color(1, 1, 1, 1)));
			box_helper->AddToRenderQueue();
#endif
		}

		BOOST_FOREACH(BOOST_TYPEOF(scene_objs_)::reference obj, scene_objs_)
		{
			if (obj->Cullable())
			{
				if (visables.find(obj) != visables.end())
				{
					visables.erase(obj);
					visible_objs_.push_back(obj);
				}
			}
			else
			{
				visible_objs_.push_back(obj);
			}
		}
	}

	void OCTree::Clear()
	{
		scene_objs_.resize(0);
		octree_.clear();
	}

	void OCTree::DoAddSceneObject(SceneObjectPtr const & obj)
	{
		scene_objs_.push_back(obj);
		if (obj->Cullable() && !obj->ShortAge())
		{
			rebuild_tree_ = true;
		}
	}

	SceneManager::SceneObjectsType::iterator OCTree::DoDelSceneObject(SceneManager::SceneObjectsType::iterator iter)
	{
		if ((*iter)->Cullable() && !(*iter)->ShortAge())
		{
			rebuild_tree_ = true;
		}
		return scene_objs_.erase(iter);
	}

	Box OCTree::AreaBox(tree_id_t const & id)
	{
		Box ret = root_box_;

		int offset = NUM_BITS - 8;
		while (0 == ((id >> offset) & 0x8))
		{
			int mark = static_cast<int>((id >> offset) & 0x7);

			float3 const & center = ret.Center();

			if (mark & 1)
			{
				ret.Min().x() = center.x();
			}
			else
			{
				ret.Max().x() = center.x();
			}
			if (mark & 2)
			{
				ret.Min().y() = center.y();
			}
			else
			{
				ret.Max().y() = center.y();
			}
			if (mark & 4)
			{
				ret.Min().z() = center.z();
			}
			else
			{
				ret.Max().z() = center.z();
			}

			offset -= 4;
		}

		return ret;
	}
}
