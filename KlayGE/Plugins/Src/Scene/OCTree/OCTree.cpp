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

#include <KlayGE/OCTree/Frustum.hpp>
#include <KlayGE/OCTree/OCTree.hpp>

namespace KlayGE
{
	OCTree::OCTree(Box const & box, uint32_t maxNumObjInANode)
		: root_box_(box), maxNumObjInANode_(maxNumObjInANode)
	{
	}

	void OCTree::ClipScene(Camera const & camera)
	{
		for (linear_octree_t::iterator iter = octree_.begin(); iter != octree_.end(); ++ iter)
		{
			SceneObjectsType& objs = iter->second;

			for (SceneObjectsType::iterator obj_iter = objs.begin(); obj_iter != objs.end();)
			{
				if ((*obj_iter)->Moveable())
				{
					if (!this->InsideChild(iter->first, *obj_iter))
					{
						obj_iter = objs.erase(obj_iter);
					}
					else
					{
						++ obj_iter;
					}
				}
				else
				{
					++ obj_iter;
				}
			}
		}

		for (SceneObjectsType::iterator iter = scene_objs_.begin(); iter != scene_objs_.end(); ++ iter)
		{
			if ((*iter)->Cullable() && (*iter)->Moveable())
			{
				this->InsertSceneObject("0", *iter);
			}
		}

		for (linear_octree_t::iterator iter = octree_.begin(); iter != octree_.end();)
		{
			SceneObjectsType& objs = iter->second;
			if (objs.empty())
			{
				iter = octree_.erase(iter);
			}
			else
			{
				++ iter;
			}
		}

		Frustum frustum(camera.ViewMatrix() * camera.ProjMatrix());

		SetVector<tree_id_t> nodes;
		SetVector<SceneObjectPtr> visables;
		for (linear_octree_t::iterator iter = octree_.begin(); iter != octree_.end(); ++ iter)
		{
			for (size_t i = 1; i <= iter->first.size(); ++ i)
			{
				tree_id_t id = iter->first.substr(0, i);

				if (nodes.find(id) == nodes.end())
				{
					Box const & box = this->AreaBox(id);

					Frustum::VIS const vis = frustum.Visiable(box);
					if ((Frustum::VIS_YES == vis)
						|| ((Frustum::VIS_PART == vis) && (i == iter->first.size())))
					{
						// 全部看见树或部分看见叶子

						visables.insert(iter->second.begin(), iter->second.end());
						nodes.insert(id);

#ifdef KLAYGE_DEBUG
						RenderablePtr box_helper(new RenderableBox(box, Color(1, 1, 1, 1)));
						box_helper->AddToRenderQueue();
#endif
					}
					else
					{
						if (Frustum::VIS_NO == vis)
						{
							// 跳过此树下的所有子树

							++ iter;
							while ((iter != octree_.end()) && (iter->first.substr(0, i) == id))
							{
								++ iter;
							}
							-- iter;

							break;
						}
					}
				}
				else
				{
					visables.insert(iter->second.begin(), iter->second.end());
				}
			}
		}

		for (SceneObjectsType::iterator iter = scene_objs_.begin(); iter != scene_objs_.end(); ++ iter)
		{
			if ((*iter)->Cullable())
			{
				if (visables.find(*iter) != visables.end())
				{
					visables.erase(*iter);
					visible_objs_.push_back(*iter);
				}
			}
			else
			{
				visible_objs_.push_back(*iter);
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

		if (obj->Cullable())
		{
			this->InsertSceneObject("0", obj);
		}
	}

	SceneManager::SceneObjectsType::iterator OCTree::DoDelSceneObject(SceneManager::SceneObjectsType::iterator iter)
	{
		for (linear_octree_t::iterator tree_iter = octree_.begin();
			tree_iter != octree_.end();)
		{
			SceneObjectsType& objs = tree_iter->second;
			for (SceneObjectsType::iterator obj_iter = objs.begin();
				obj_iter != objs.end();)
			{
				if (*obj_iter == *iter)
				{
					obj_iter = objs.erase(obj_iter);
				}
				else
				{
					++ obj_iter;
				}
			}

			if (objs.empty())
			{
				tree_iter = octree_.erase(tree_iter);
			}
			else
			{
				++ tree_iter;
			}
		}

		return scene_objs_.erase(iter);
	}

	OCTree::tree_id_t OCTree::Child(tree_id_t const & id, int child_no)
	{
		BOOST_ASSERT(child_no >= 0);
		BOOST_ASSERT(child_no < 8);

		tree_id_t child_id = id;
		child_id.push_back(static_cast<char>(child_no + '0'));

		return child_id;
	}

	Box OCTree::AreaBox(tree_id_t const & id)
	{
		Box ret = root_box_;

		for (tree_id_t::const_iterator iter = id.begin() + 1; iter != id.end(); ++ iter)
		{
			float3 const & min = ret.Min();
			float3 const & max = ret.Max();
			float3 const & center = ret.Center();

			switch (*iter - '0')
			{
			case 0:
				ret = Box(min, center);
				break;

			case 1:
				ret = Box(float3(center.x(), min.y(), min.z()),
					float3(max.x(), center.y(), center.z()));
				break;

			case 2:
				ret = Box(float3(min.x(), center.y(), min.z()),
					float3(center.x(), max.y(), center.z()));
				break;

			case 3:
				ret = Box(float3(center.x(), center.y(), min.z()),
					float3(max.x(), max.y(), center.z()));
				break;

			case 4:
				ret = Box(float3(min.x(), min.y(), center.z()),
					float3(center.x(), center.y(), max.z()));
				break;

			case 5:
				ret = Box(float3(center.x(), min.y(), center.z()),
					float3(max.x(), center.y(), max.z()));
				break;

			case 6:
				ret = Box(float3(min.x(), center.y(), center.z()),
					float3(center.x(), max.y(), max.z()));
				break;

			case 7:
				ret = Box(center, max);
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		return ret;
	}

	bool OCTree::InsideChild(tree_id_t const & id, SceneObjectPtr const & obj)
	{
		Box const & area_box = this->AreaBox(id);
		Box const & box(obj->GetBound());

		for (size_t i = 0; i < 8; ++ i)
		{
			float3 vec(MathLib::transform_coord(box[i], obj->GetModelMatrix()));
			if (MathLib::vec_in_box(area_box, vec))
			{
				return true;
			}
		}
		return false;
	}

	void OCTree::InsertSceneObject(tree_id_t const & id, SceneObjectPtr const & obj)
	{
		BOOST_ASSERT(this->InsideChild(id, obj));

		linear_octree_t::iterator node = octree_.find(id);

		if (node == octree_.end())
		{
			octree_.insert(std::make_pair(id, SceneObjectsType(1, obj)));

			BOOST_ASSERT(octree_[id].front() == obj);
		}
		else
		{
			if (node->second.size() < maxNumObjInANode_)
			{
				node->second.push_back(obj);
			}
			else
			{
				SceneObjectsType const & old_objs = node->second;

				for (SceneObjectsType::const_iterator iter = old_objs.begin();
					iter != old_objs.end(); ++ iter)
				{
					for (int i = 0; i < 8; ++ i)
					{
						tree_id_t const child_id = this->Child(id, i);

						if (this->InsideChild(child_id, *iter))
						{
							this->InsertSceneObject(child_id, *iter);
						}

						if (this->InsideChild(child_id, obj))
						{
							this->InsertSceneObject(child_id, obj);
						}
					}
				}

				octree_.erase(node);
			}
		}
	}
}
