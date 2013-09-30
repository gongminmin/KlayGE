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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KFL/Vector.hpp>
#include <KFL/Matrix.hpp>
#include <KFL/Plane.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <algorithm>
#include <functional>
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
			KLAYGE_FOREACH(SceneObjAABBPtrType const & soaabb, scene_objs_)
			{
				SceneObjectPtr const & obj = soaabb->so;
				uint32_t const attr = obj->Attrib();
				if ((attr & SceneObject::SOA_Cullable)
					&& !(attr & SceneObject::SOA_Overlay)
					&& !(attr & SceneObject::SOA_Moveable))
				{
					bb_root |= *soaabb->aabb_ws;
					octree_[0].obj_ptrs.push_back(soaabb);
				}
			}
			float3 const & center = bb_root.Center();
			float3 const & extent = bb_root.HalfSize();
			float longest_dim = std::max(std::max(extent.x(), extent.y()), extent.z());
			float3 new_extent(longest_dim, longest_dim, longest_dim);
			octree_[0].bb = AABBox(center - new_extent, center + new_extent);

			this->DivideNode(0, 1);

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

		float4x4 view_proj = camera.ViewProjMatrix();;
		DeferredRenderingLayerPtr const & drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			float4x4 proj = camera.ProjMatrix();
			int32_t cas_index = drl->CurrCascadeIndex();
			if (cas_index >= 0)
			{
				view_proj *= drl->GetCascadedShadowLayer()->CascadeCropMatrix(cas_index);
			}
		}

		if (camera.OmniDirectionalMode())
		{
			KLAYGE_FOREACH(SceneObjAABBPtrType const & soaabb, scene_objs_)
			{
				SceneObjectPtr const & obj = soaabb->so;
				soaabb->visible = (!(obj->Attrib() & SceneObject::SOA_Overlay) && obj->Visible());
				
				if (obj->Attrib() & SceneObject::SOA_Cullable)
				{
					AABBox aabb_ws;
					if (obj->Attrib() & SceneObject::SOA_Moveable)
					{
						AABBox const & aabb = obj->PosBound();
						float4x4 const & mat = obj->ModelMatrix();

						aabb_ws = MathLib::transform_aabb(aabb, mat);
					}
					else
					{
						aabb_ws = *soaabb->aabb_ws;
					}

					soaabb->visible &= (MathLib::perspective_area(camera.EyePos(), view_proj,
						aabb_ws) > small_obj_threshold_);
				}
			}
		}
		else
		{
			if (!octree_.empty())
			{
				this->MarkNodeObjs(0, false);
			}

			KLAYGE_FOREACH(SceneObjAABBPtrType const & soaabb, scene_objs_)
			{
				SceneObjectPtr const & obj = soaabb->so;
				if (obj->Visible())
				{
					uint32_t const attr = obj->Attrib();
					if (!(attr & SceneObject::SOA_Overlay))
					{
						if (!(attr & SceneObject::SOA_Cullable))
						{
							soaabb->visible = true;
						}
						else if (attr & SceneObject::SOA_Moveable)
						{
							AABBox const & aabb = obj->PosBound();
							float4x4 const & mat = obj->ModelMatrix();
							soaabb->visible = this->AABBVisible(MathLib::transform_aabb(aabb, mat));
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

	void OCTree::OnDelSceneObject(SceneObjAABBsType::iterator iter)
	{
		if (iter != scene_objs_.end())
		{
			uint32_t const attr = (*iter)->so->Attrib();
			if ((attr & SceneObject::SOA_Cullable)
				&& !(attr & SceneObject::SOA_Overlay)
				&& !(attr & SceneObject::SOA_Moveable))
			{
				rebuild_tree_ = true;
			}
		}
	}

	void OCTree::DivideNode(size_t index, uint32_t curr_depth)
	{
		if (octree_[index].obj_ptrs.size() > 1)
		{
			size_t const this_size = octree_.size();
			AABBox const parent_bb = octree_[index].bb;
			float3 const parent_center = parent_bb.Center();
			octree_[index].first_child_index = static_cast<int>(this_size);
			octree_[index].visible = BO_No;

			octree_.resize(this_size + 8);
			KLAYGE_FOREACH(SceneObjAABBPtrType const & soaabb, octree_[index].obj_ptrs)
			{
				AABBox const & aabb = *soaabb->aabb_ws;
				int mark[6];
				mark[0] = aabb.Min().x() >= parent_center.x() ? 1 : 0;
				mark[1] = aabb.Min().y() >= parent_center.y() ? 2 : 0;
				mark[2] = aabb.Min().z() >= parent_center.z() ? 4 : 0;
				mark[3] = aabb.Max().x() >= parent_center.x() ? 1 : 0;
				mark[4] = aabb.Max().y() >= parent_center.y() ? 2 : 0;
				mark[5] = aabb.Max().z() >= parent_center.z() ? 4 : 0;
				for (int j = 0; j < 8; ++ j)
				{
					if (j == ((j & 1) ? mark[3] : mark[0])
						+ ((j & 2) ? mark[4] : mark[1])
						+ ((j & 4) ? mark[5] : mark[2]))
					{
						octree_[this_size + j].obj_ptrs.push_back(soaabb);
					}
				}
			}

			for (size_t j = 0; j < 8; ++ j)
			{
				octree_node_t& new_node = octree_[this_size + j];
				new_node.first_child_index = -1;
				new_node.bb = AABBox(float3((j & 1) ? parent_center.x() : parent_bb.Min().x(),
						(j & 2) ? parent_center.y() : parent_bb.Min().y(),
						(j & 4) ? parent_center.z() : parent_bb.Min().z()),
					float3((j & 1) ? parent_bb.Max().x() : parent_center.x(),
						(j & 2) ? parent_bb.Max().y() : parent_center.y(),
						(j & 4) ? parent_bb.Max().z() : parent_center.z()));

				if (curr_depth < max_tree_depth_)
				{
					this->DivideNode(this_size + j, curr_depth + 1);
				}
			}

			SceneObjAABBsType empty;
			octree_[index].obj_ptrs.swap(empty);
		}
	}

	void OCTree::NodeVisible(size_t index)
	{
		BOOST_ASSERT(index < octree_.size());

		App3DFramework& app = Context::Instance().AppInstance();
		Camera& camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewProjMatrix();;
		DeferredRenderingLayerPtr const & drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			float4x4 proj = camera.ProjMatrix();
			int32_t cas_index = drl->CurrCascadeIndex();
			if (cas_index >= 0)
			{
				view_proj *= drl->GetCascadedShadowLayer()->CascadeCropMatrix(cas_index);
			}
		}

		octree_node_t& node = octree_[index];
		if (MathLib::perspective_area(camera.EyePos(), view_proj, node.bb) > small_obj_threshold_)
		{
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
		}
		else
		{
			node.visible = BO_No;
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

		App3DFramework& app = Context::Instance().AppInstance();
		Camera& camera = app.ActiveCamera();

		float4x4 view_proj = camera.ViewProjMatrix();;
		DeferredRenderingLayerPtr const & drl = Context::Instance().DeferredRenderingLayerInstance();
		if (drl)
		{
			float4x4 proj = camera.ProjMatrix();
			int32_t cas_index = drl->CurrCascadeIndex();
			if (cas_index >= 0)
			{
				view_proj *= drl->GetCascadedShadowLayer()->CascadeCropMatrix(cas_index);
			}
		}

		octree_node_t const & node = octree_[index];
		if ((node.visible != BO_No) || force)
		{
			KLAYGE_FOREACH(SceneObjAABBPtrType const & soaabb, node.obj_ptrs)
			{
				if (!soaabb->visible && soaabb->so->Visible())
				{
					if (MathLib::perspective_area(camera.EyePos(), view_proj,
						*soaabb->aabb_ws) > small_obj_threshold_)
					{
						BoundOverlap const bo = frustum_->Intersect(*soaabb->aabb_ws);
						soaabb->visible = (bo != BO_No);
					}
					else
					{
						soaabb->visible = false;
					}
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
					int mark[6];
					mark[0] = aabb.Min().x() >= center.x() ? 1 : 0;
					mark[1] = aabb.Min().y() >= center.y() ? 2 : 0;
					mark[2] = aabb.Min().z() >= center.z() ? 4 : 0;
					mark[3] = aabb.Max().x() >= center.x() ? 1 : 0;
					mark[4] = aabb.Max().y() >= center.y() ? 2 : 0;
					mark[5] = aabb.Max().z() >= center.z() ? 4 : 0;
					for (int j = 0; j < 8; ++ j)
					{
						if (j == ((j & 1) ? mark[3] : mark[0])
							+ ((j & 2) ? mark[4] : mark[1])
							+ ((j & 4) ? mark[5] : mark[2]))
						{
							if (this->BoundVisible(node.first_child_index + j, aabb))
							{
								return true;
							}
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
