// OCTree.cpp
// KlayGE 八叉树类 实现文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/RenderableHelper.hpp>

#include <set>

#include <KlayGE/OCTree/OCTreeFrustum.hpp>
#include <KlayGE/OCTree/OCTree.hpp>

namespace KlayGE
{
	OCTree::OCTree(Box const & box, uint32_t maxNumObjInANode)
		: root_box_(box), maxNumObjInANode_(maxNumObjInANode)
	{
	}

	void OCTree::ClipScene(Camera const & camera)
	{
		for (RenderItemsType::iterator iter = renderItems_.begin(); iter != renderItems_.end(); ++ iter)
		{
			this->InsertRenderable("0", *iter);
		}

		OCTreeFrustum frustum(camera.ViewMatrix() * camera.ProjMatrix());

		std::set<RenderablePtr> renderables;
		for (linear_octree_t::iterator iter = octree_.begin(); iter != octree_.end(); ++ iter)
		{
			Box const & box = this->AreaBox(iter->first);

			if (frustum.Visiable(box, Matrix4::Identity()))
			{
				renderables.insert(iter->second.begin(), iter->second.end());

#ifdef KLAYGE_DEBUG
				renderables.insert(RenderablePtr(new RenderableBox(box)));
#endif
			}
		}

		for (std::set<RenderablePtr>::iterator iter = renderables.begin();
			iter != renderables.end(); ++ iter)
		{
			this->AddToRenderQueue(*iter);
		}

		octree_.clear();
	}

	OCTree::tree_id_t OCTree::Child(tree_id_t const & id, int child_no)
	{
		assert(child_no >= 0);
		assert(child_no < 8);

		tree_id_t child_id = id;
		child_id.push_back(static_cast<char>(child_no + '0'));

		return child_id;
	}

	Box OCTree::AreaBox(tree_id_t const & id)
	{
		Box ret = root_box_;

		for (tree_id_t::const_iterator iter = id.begin() + 1; iter != id.end(); ++ iter)
		{
			Vector3 const & min = ret.Min();
			Vector3 const & max = ret.Max();
			Vector3 const & center = ret.Center();

			switch (*iter - '0')
			{
			case 0:
				ret = Box(min, center);
				break;

			case 1:
				ret = Box(Vector3(center.x(), min.y(), min.z()),
					Vector3(max.x(), center.y(), center.z()));
				break;

			case 2:
				ret = Box(Vector3(min.x(), center.y(), min.z()),
					Vector3(center.x(), max.y(), center.z()));
				break;

			case 3:
				ret = Box(Vector3(center.x(), center.y(), min.z()),
					Vector3(max.x(), max.y(), center.z()));
				break;

			case 4:
				ret = Box(Vector3(min.x(), min.y(), center.z()),
					Vector3(center.x(), center.y(), max.z()));
				break;

			case 5:
				ret = Box(Vector3(center.x(), min.y(), center.z()),
					Vector3(max.x(), center.y(), max.z()));
				break;

			case 6:
				ret = Box(Vector3(min.x(), center.y(), center.z()),
					Vector3(center.x(), max.y(), max.z()));
				break;

			case 7:
				ret = Box(center, max);
				break;

			default:
				assert(false);
				break;
			}
		}

		return ret;
	}

	bool OCTree::InsideChild(tree_id_t const & id, RenderablePtr const & renderable)
	{
		Box const & area_box = this->AreaBox(id);
		Box const & box(renderable->GetBound());

		for (size_t i = 0; i < 8; ++ i)
		{
			Vector3 vec;
			MathLib::TransformCoord(vec, box[i], renderable->GetWorld());
			if (MathLib::VecInBox(area_box, vec))
			{
				return true;
			}
		}
		return false;
	}

	void OCTree::InsertRenderable(tree_id_t const & id, RenderablePtr const & renderable)
	{
		assert(this->InsideChild(id, renderable));

		linear_octree_t::iterator node = octree_.find(id);

		if (node == octree_.end())
		{
			octree_.insert(std::make_pair(id, renderable_ptrs_t(1, renderable)));

			assert(octree_[id].front() == renderable);
		}
		else
		{
			if (node->second.size() < maxNumObjInANode_)
			{
				node->second.push_back(renderable);
			}
			else
			{
				renderable_ptrs_t const & old_renderables = node->second;

				for (renderable_ptrs_t::const_iterator iter = old_renderables.begin();
					iter != old_renderables.end(); ++ iter)
				{
					for (int i = 0; i < 8; ++ i)
					{
						tree_id_t const child_id = this->Child(id, i);

						if (this->InsideChild(child_id, *iter))
						{
							this->InsertRenderable(child_id, *iter);
						}

						if (this->InsideChild(child_id, renderable))
						{
							this->InsertRenderable(child_id, renderable);
						}
					}
				}

				octree_.erase(node);
			}
		}
	}
}
