// OCTree.cpp
// KlayGE 八叉树类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2004-2007
// Homepage: http://www.klayge.org
//
// 3.7.0
// 提升了遍历速度 (2007.12.18)
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
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/Plane.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/App3D.hpp>

#include <algorithm>
#include <functional>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

#ifdef KLAYGE_DRAW_NODES
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#endif

#include <KlayGE/OCTree/OCTree.hpp>

#ifdef KLAYGE_DRAW_NODES
namespace
{
	using namespace KlayGE;

	class NodeRenderable : public RenderableLineBox
	{
	public:
		NodeRenderable()
			: RenderableLineBox(AABBox(float3(-1, -1, -1), float3(1, 1, 1)), Color(1, 1, 1, 1))
		{
		}

		void Render()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			this->OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view_proj = app.ActiveCamera().ViewProjMatrix();

			*(technique_->Effect().ParameterByName("color")) = float4(1, 1, 1, 1);
			for (uint32_t i = 0; i < instances_.size(); ++ i)
			{
				*(technique_->Effect().ParameterByName("matViewProj")) = instances_[i] * view_proj;

				re.Render(*technique_, *rl_);
			}

			this->OnRenderEnd();
		}

		void ClearInstances()
		{
			instances_.resize(0);
		}

		void AddInstance(float4x4 const & mat)
		{
			instances_.push_back(mat);
		}

	private:
		std::vector<float4x4> instances_;
	};
}
#endif

namespace KlayGE
{
	OCTree::OCTree()
		: max_tree_depth_(4), rebuild_tree_(false)
	{
		base_address_.resize(2);
		base_address_[0] = 0;
		base_address_[1] = 1;
	}

	void OCTree::MaxTreeDepth(uint32_t max_tree_depth)
	{
		max_tree_depth_ = std::min<uint32_t>(max_tree_depth, 16UL);
	}

	uint32_t OCTree::MaxTreeDepth() const
	{
		return max_tree_depth_;
	}

	void OCTree::ClipScene()
	{
		if (rebuild_tree_)
		{
			octree_.resize(1);
			AABBox bb_root(float3(0, 0, 0), float3(0, 0, 0));
			octree_[0].first_child_index = -1;
			octree_[0].visible = BO_No;
			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				SceneObjectPtr const & obj = scene_objs_[i];
				uint32_t const attr = obj->Attrib();
				if ((attr & SceneObject::SOA_Cullable)
					&& !(attr & SceneObject::SOA_Overlay)
					&& !(attr & SceneObject::SOA_Moveable))
				{
					AABBox const & aabb_in_ws = *scene_obj_bbs_[i];

					bb_root |= aabb_in_ws;
					octree_[0].obj_indices.push_back(i);
				}
			}
			octree_[0].bb = bb_root;

			typedef BOOST_TYPEOF(octree_[0].obj_indices) ObjIndicesType;

			base_address_.resize(2);
			for (uint32_t d = 1; d <= max_tree_depth_; ++ d)
			{
				size_t const original_size = octree_.size();
				for (size_t i = base_address_[d - 1]; i < base_address_[d]; ++ i)
				{
					if (octree_[i].obj_indices.size() > 1)
					{
						size_t const this_size = octree_.size();
						float3 const parent_center = octree_[i].bb.Center();
						float3 const new_half_size = octree_[i].bb.HalfSize() / 2.0f;
						octree_[i].first_child_index = static_cast<int>(base_address_[d] + this_size - original_size);
						octree_[i].visible = BO_No;

						octree_.resize(this_size + 8);
						for (size_t j = 0; j < 8; ++ j)
						{
							octree_node_t& new_node = octree_[this_size + j];
							new_node.first_child_index = -1;
							ObjIndicesType& new_node_obj_indices = new_node.obj_indices;
							ObjIndicesType& parent_obj_indices = octree_[i].obj_indices;

							float3 bb_center = parent_center;
							if (j & 1)
							{
								bb_center.x() += new_half_size.x();
							}
							else
							{
								bb_center.x() -= new_half_size.x();
							}
							if (j & 2)
							{
								bb_center.y() += new_half_size.y();
							}
							else
							{
								bb_center.y() -= new_half_size.y();
							}
							if (j & 4)
							{
								bb_center.z() += new_half_size.z();
							}
							else
							{
								bb_center.z() -= new_half_size.z();
							}
							new_node.bb = AABBox(bb_center - new_half_size, bb_center + new_half_size);

							BOOST_FOREACH(size_t obj_index, parent_obj_indices)
							{
								AABBox const & aabb_in_ws = *scene_obj_bbs_[obj_index];
								if (MathLib::intersect_aabb_aabb(aabb_in_ws, new_node.bb))
								{
									new_node_obj_indices.push_back(obj_index);
								}
							}
						}

						ObjIndicesType empty;
						octree_[i].obj_indices.swap(empty);
					}
				}

				base_address_.push_back(base_address_.back() + octree_.size() - original_size);
			}

			rebuild_tree_ = false;
		}

#ifdef KLAYGE_DRAW_NODES
		if (!node_renderable_)
		{
			node_renderable_ = MakeSharedPtr<NodeRenderable>();
		}
		checked_pointer_cast<NodeRenderable>(node_renderable_)->ClearInstances();
#endif

		if (!octree_.empty())
		{
			this->NodeVisible(0);
		}

		App3DFramework& app = Context::Instance().AppInstance();
		Camera& camera = app.ActiveCamera();

		if (camera.OmniDirectionalMode())
		{
			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				SceneObjectPtr const & obj = scene_objs_[i];
				(*visible_marks_)[i] = (!(obj->Attrib() & SceneObject::SOA_Overlay) && obj->Visible());
			}
		}
		else
		{
			if (!octree_.empty())
			{
				this->MarkNodeObjs(0, false);
			}

			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				SceneObjectPtr const & obj = scene_objs_[i];
				if (obj->Visible())
				{
					uint32_t const attr = obj->Attrib();
					if (!(attr & SceneObject::SOA_Overlay))
					{
						if (!(attr & SceneObject::SOA_Cullable))
						{
							(*visible_marks_)[i] = true;
						}
						else if (attr & SceneObject::SOA_Moveable)
						{
							AABBox const & aabb = obj->Bound();
							float4x4 const & mat = obj->ModelMatrix();

							AABBox aabb_in_ws = MathLib::transform_aabb(aabb, mat);

							(*visible_marks_)[i] = this->AABBVisible(aabb_in_ws);
						}
					}
				}
			}
		}

#ifdef KLAYGE_DRAW_NODES
		node_renderable_->Render();
#endif
	}

	void OCTree::ClearObject()
	{
		SceneManager::ClearObject();

		octree_.clear();
		rebuild_tree_ = true;
	}

	void OCTree::OnAddSceneObject(SceneObjectPtr const & obj)
	{
		uint32_t const attr = obj->Attrib();
		if ((attr & SceneObject::SOA_Cullable)
			&& !(attr & SceneObject::SOA_Overlay)
			&& !(attr & SceneObject::SOA_Moveable))
		{
			rebuild_tree_ = true;
		}
	}

	void OCTree::OnDelSceneObject(SceneManager::SceneObjectsType::iterator iter)
	{
		if (iter != scene_objs_.end())
		{
			uint32_t const attr = (*iter)->Attrib();
			if ((attr & SceneObject::SOA_Cullable)
				&& !(attr & SceneObject::SOA_Overlay)
				&& !(attr & SceneObject::SOA_Moveable))
			{
				rebuild_tree_ = true;
			}
			else
			{
				if (iter != scene_objs_.end() - 1)
				{
					this->AdjustObjIndices(0, iter - scene_objs_.begin());
				}
			}
		}
	}

	void OCTree::NodeVisible(size_t index)
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t& node = octree_[index];
		BoundOverlap const vis = frustum_->Intersect(node.bb);
		node.visible = vis;
		if (BO_Partial == vis)
		{
			if (node.first_child_index != -1)
			{
				for (int i = 0; i < 8; ++ i)
				{
					this->NodeVisible(node.first_child_index + i);
				}
			}
		}

#ifdef KLAYGE_DRAW_NODES
		if ((vis != BO_No) && (-1 == node.first_child_index))
		{
			checked_pointer_cast<NodeRenderable>(node_renderable_)->AddInstance(MathLib::scaling(node.bb.HalfSize()) * MathLib::translation(node.bb.Center()));
		}
#endif
	}

	void OCTree::MarkNodeObjs(size_t index, bool force)
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if ((node.visible != BO_No) || force)
		{
			BOOST_FOREACH(size_t obj_index, node.obj_indices)
			{
				if (!(*visible_marks_)[obj_index] && scene_objs_[obj_index]->Visible())
				{
					AABBox const & aabb_in_ws = *scene_obj_bbs_[obj_index];
					BoundOverlap const bo = frustum_->Intersect(aabb_in_ws);
					(*visible_marks_)[obj_index] = (bo != BO_No);
				}
			}

			if (node.first_child_index != -1)
			{
				for (int i = 0; i < 8; ++ i)
				{
					this->MarkNodeObjs(node.first_child_index + i, (BO_Yes == node.visible) || force);
				}
			}
		}
	}

	void OCTree::AdjustObjIndices(size_t index, size_t obj_index)
	{
		if (index < octree_.size())
		{
			octree_node_t& node = octree_[index];
			for (BOOST_AUTO(iter, node.obj_indices.begin()); iter != node.obj_indices.end();)
			{
				if (*iter == obj_index)
				{
					iter = node.obj_indices.erase(iter);
				}
				else
				{
					if (*iter > obj_index)
					{
						-- *iter;
					}
					++ iter;
				}
			}

			if (node.first_child_index != -1)
			{
				for (int i = 0; i < 8; ++ i)
				{
					this->AdjustObjIndices(node.first_child_index + i, obj_index);
				}
			}
		}
	}

	bool OCTree::AABBVisible(AABBox const & aabb)
	{
		// Frustum VS node
		bool visible = true;
		if (!octree_.empty())
		{
			if (MathLib::intersect_aabb_aabb(octree_[0].bb, aabb))
			{
				visible = this->BoundVisible(0, aabb);
			}
			else
			{
				// Out of scene
				visible = true;
			}
		}
		if (visible)
		{
			if (frustum_)
			{
				// Frustum VS AABB
				BoundOverlap const bo = frustum_->Intersect(aabb);
				visible = (bo != BO_No);
			}
			else
			{
				visible = true;
			}
		}
		return visible;
	}

	bool OCTree::OBBVisible(OBBox const & obb)
	{
		// Frustum VS node
		bool visible = true;
		if (!octree_.empty())
		{
			if (MathLib::intersect_aabb_obb(octree_[0].bb, obb))
			{
				visible = this->BoundVisible(0, obb);
			}
			else
			{
				// Out of scene
				visible = true;
			}
		}
		if (visible)
		{
			if (frustum_)
			{
				// Frustum VS OBB
				BoundOverlap const bo = frustum_->Intersect(obb);
				visible = (bo != BO_No);
			}
			else
			{
				visible = true;
			}
		}
		return visible;
	}

	bool OCTree::SphereVisible(Sphere const & sphere)
	{
		// Frustum VS node
		bool visible = true;
		if (!octree_.empty())
		{
			if (MathLib::intersect_aabb_sphere(octree_[0].bb, sphere))
			{
				visible = this->BoundVisible(0, sphere);
			}
			else
			{
				// Out of scene
				visible = true;
			}
		}
		if (visible)
		{
			if (frustum_)
			{
				// Frustum VS OBB
				BoundOverlap const bo = frustum_->Intersect(sphere);
				visible = (bo != BO_No);
			}
			else
			{
				visible = true;
			}
		}
		return visible;
	}

	bool OCTree::BoundVisible(size_t index, AABBox const & aabb) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if (MathLib::intersect_aabb_aabb(node.bb, aabb))
		{
			BoundOverlap const vis = node.visible;
			switch (vis)
			{
			case BO_Yes:
				return true;

			case BO_No:
				return false;

			case BO_Partial:
				if (node.first_child_index != -1)
				{
					float3 const center = node.bb.Center();
					bool mark[6];
					mark[0] = aabb.Min().x() < center.x();
					mark[1] = aabb.Min().y() < center.y();
					mark[2] = aabb.Min().z() < center.z();
					mark[3] = aabb.Max().x() > center.x();
					mark[4] = aabb.Max().y() > center.y();
					mark[5] = aabb.Max().z() > center.z();

					if (mark[0] && mark[1] && mark[2])
					{
						if (this->BoundVisible(node.first_child_index + 0, aabb))
						{
							return true;
						}
					}
					if (mark[3] && mark[1] && mark[2])
					{
						if (this->BoundVisible(node.first_child_index + 1, aabb))
						{
							return true;
						}
					}
					if (mark[0] && mark[4] && mark[2])
					{
						if (this->BoundVisible(node.first_child_index + 2, aabb))
						{
							return true;
						}
					}
					if (mark[3] && mark[4] && mark[2])
					{
						if (this->BoundVisible(node.first_child_index + 3, aabb))
						{
							return true;
						}
					}
					if (mark[0] && mark[1] && mark[5])
					{
						if (this->BoundVisible(node.first_child_index + 4, aabb))
						{
							return true;
						}
					}
					if (mark[3] && mark[1] && mark[5])
					{
						if (this->BoundVisible(node.first_child_index + 5, aabb))
						{
							return true;
						}
					}
					if (mark[0] && mark[4] && mark[5])
					{
						if (this->BoundVisible(node.first_child_index + 6, aabb))
						{
							return true;
						}
					}
					if (mark[3] && mark[4] && mark[5])
					{
						if (this->BoundVisible(node.first_child_index + 7, aabb))
						{
							return true;
						}
					}

					return false;
				}
				else
				{
					return true;
				}
				break;

			default:
				BOOST_ASSERT(false);
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	bool OCTree::BoundVisible(size_t index, OBBox const & obb) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if (MathLib::intersect_aabb_obb(node.bb, obb))
		{
			BoundOverlap const vis = node.visible;
			switch (vis)
			{
			case BO_Yes:
				return true;

			case BO_No:
				return false;

			case BO_Partial:
				if (node.first_child_index != -1)
				{
					for (int i = 0; i < 8; ++ i)
					{
						if (this->BoundVisible(node.first_child_index + i, obb))
						{
							return true;
						}
					}

					return false;
				}
				else
				{
					return true;
				}
				break;

			default:
				BOOST_ASSERT(false);
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	bool OCTree::BoundVisible(size_t index, Sphere const & sphere) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if (MathLib::intersect_aabb_sphere(node.bb, sphere))
		{
			BoundOverlap const vis = node.visible;
			switch (vis)
			{
			case BO_Yes:
				return true;

			case BO_No:
				return false;

			case BO_Partial:
				if (node.first_child_index != -1)
				{
					for (int i = 0; i < 8; ++ i)
					{
						if (this->BoundVisible(node.first_child_index + i, sphere))
						{
							return true;
						}
					}

					return false;
				}
				else
				{
					return true;
				}
				break;

			default:
				BOOST_ASSERT(false);
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	bool OCTree::BoundVisible(size_t index, Frustum const & frustum) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if (MathLib::intersect_aabb_frustum(node.bb, frustum))
		{
			BoundOverlap const vis = node.visible;
			switch (vis)
			{
			case BO_Yes:
				return true;

			case BO_No:
				return false;

			case BO_Partial:
				if (node.first_child_index != -1)
				{
					for (int i = 0; i < 8; ++ i)
					{
						if (this->BoundVisible(node.first_child_index + i, frustum))
						{
							return true;
						}
					}

					return false;
				}
				else
				{
					return true;
				}
				break;

			default:
				BOOST_ASSERT(false);
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}
