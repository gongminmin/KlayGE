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
#ifdef KLAYGE_DEBUG
#include <KlayGE/Camera.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#endif

#include <algorithm>
#include <functional>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#ifdef KLAYGE_DEBUG
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

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();
			float4x4 view_proj = view * proj;

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
			typedef std::vector<size_t> ObjIndicesTypes;
			std::vector<ObjIndicesTypes> obj_indices(1);
			octree_.resize(1);
			AABBox bb_root(float3(0, 0, 0), float3(0, 0, 0));
			octree_[0].first_child_index = -1;
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
					obj_indices[0].push_back(i);
				}
			}
			{
				octree_[0].bb = bb_root;
			}
			base_address_.push_back(0);
			base_address_.push_back(1);

			for (uint32_t d = 1; d <= max_tree_depth_; ++ d)
			{
				size_t const original_size = octree_.size();
				for (size_t i = base_address_[d - 1]; i < base_address_[d]; ++ i)
				{
					if (obj_indices[i].size() > 1)
					{
						float3 const parent_center = octree_[i].bb.Center();
						float3 const new_half_size = octree_[i].bb.HalfSize() / 2.0f;
						octree_[i].first_child_index = static_cast<int>(base_address_[d] + octree_.size() - original_size);

						for (size_t j = 0; j < 8; ++ j)
						{
							octree_.push_back(octree_node_t());
							octree_node_t& new_node = octree_.back();
							new_node.first_child_index = -1;
							obj_indices.push_back(ObjIndicesTypes());
							ObjIndicesTypes& new_node_obj_indices = obj_indices.back();
							ObjIndicesTypes& parent_obj_indices = obj_indices[i];

							float3 bb_center;
							if (j & 1)
							{
								bb_center.x() = parent_center.x() + new_half_size.x();
							}
							else
							{
								bb_center.x() = parent_center.x() - new_half_size.x();
							}
							if (j & 2)
							{
								bb_center.y() = parent_center.y() + new_half_size.y();
							}
							else
							{
								bb_center.y() = parent_center.y() - new_half_size.y();
							}
							if (j & 4)
							{
								bb_center.z() = parent_center.z() + new_half_size.z();
							}
							else
							{
								bb_center.z() = parent_center.z() - new_half_size.z();
							}
							new_node.bb = AABBox(bb_center - new_half_size, bb_center + new_half_size);

							BOOST_FOREACH(size_t obj_index, parent_obj_indices)
							{
								AABBox const & aabb_in_ws = *scene_obj_bbs_[obj_index];
								if (((aabb_in_ws.Min().x() <= new_node.bb.Max().x()) && (aabb_in_ws.Max().x() >= new_node.bb.Min().x()))
									&& ((aabb_in_ws.Min().y() <= new_node.bb.Max().y()) && (aabb_in_ws.Max().y() >= new_node.bb.Min().y()))
									&& ((aabb_in_ws.Min().z() <= new_node.bb.Max().z()) && (aabb_in_ws.Max().z() >= new_node.bb.Min().z())))
								{
									new_node_obj_indices.push_back(obj_index);
								}
							}
						}

						obj_indices[i].clear();
					}
				}

				base_address_.push_back(base_address_.back() + octree_.size() - original_size);
			}

			rebuild_tree_ = false;
		}

#ifdef KLAYGE_DEBUG
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

		SceneManager::ClipScene();

#ifdef KLAYGE_DEBUG
		node_renderable_->Render();
#endif
	}

	void OCTree::Clear()
	{
		scene_obj_bbs_.resize(0);
		scene_objs_.resize(0);
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
		uint32_t const attr = (*iter)->Attrib();
		if ((attr & SceneObject::SOA_Cullable)
			&& !(attr & SceneObject::SOA_Overlay)
			&& !(attr & SceneObject::SOA_Moveable))
		{
			rebuild_tree_ = true;
		}
	}

	void OCTree::NodeVisible(size_t index)
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t& node = octree_[index];
		BoundOverlap const vis = frustum_->CollisionDet(node.bb);
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

#ifdef KLAYGE_DEBUG
		if ((vis != Frustum::VIS_NO) && (-1 == node.first_child_index))
		{
			checked_pointer_cast<NodeRenderable>(node_renderable_)->AddInstance(MathLib::scaling(node.bb.HalfSize()) * MathLib::translation(node.bb.Center()));
		}
#endif
	}

	bool OCTree::AABBVisible(AABBox const & box)
	{
		// Frustum VS node
		bool visible = true;
		if (!octree_.empty())
		{
			octree_node_t const & node = octree_[0];
			if (((box.Min().x() <= node.bb.Max().x()) && (box.Max().x() >= node.bb.Min().x()))
				&& ((box.Min().y() <= node.bb.Max().y()) && (box.Max().y() >= node.bb.Min().y()))
				&& ((box.Min().z() <= node.bb.Max().z()) && (box.Max().z() >= node.bb.Min().z())))
			{
				visible = this->BBVisible(0, box);
			}
			else
			{
				// Out of scene
				visible = true;
			}
		}
		if (visible)
		{
			// Frustum VS AABB
			BoundOverlap const bo = frustum_->CollisionDet(box);
			visible = (bo != BO_No);
		}
		return visible;
	}

	bool OCTree::BBVisible(size_t index, AABBox const & box) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if (((box.Min().x() <= node.bb.Max().x()) && (box.Max().x() >= node.bb.Min().x()))
			&& ((box.Min().y() <= node.bb.Max().y()) && (box.Max().y() >= node.bb.Min().y()))
			&& ((box.Min().z() <= node.bb.Max().z()) && (box.Max().z() >= node.bb.Min().z())))
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
					mark[0] = box.Min().x() < center.x();
					mark[1] = box.Min().y() < center.y();
					mark[2] = box.Min().z() < center.z();
					mark[3] = box.Max().x() > center.x();
					mark[4] = box.Max().y() > center.y();
					mark[5] = box.Max().z() > center.z();

					if (mark[0] && mark[1] && mark[2])
					{
						if (this->BBVisible(node.first_child_index + 0, box))
						{
							return true;
						}
					}
					if (mark[3] && mark[1] && mark[2])
					{
						if (this->BBVisible(node.first_child_index + 1, box))
						{
							return true;
						}
					}
					if (mark[0] && mark[4] && mark[2])
					{
						if (this->BBVisible(node.first_child_index + 2, box))
						{
							return true;
						}
					}
					if (mark[3] && mark[4] && mark[2])
					{
						if (this->BBVisible(node.first_child_index + 3, box))
						{
							return true;
						}
					}
					if (mark[0] && mark[1] && mark[5])
					{
						if (this->BBVisible(node.first_child_index + 4, box))
						{
							return true;
						}
					}
					if (mark[3] && mark[1] && mark[5])
					{
						if (this->BBVisible(node.first_child_index + 5, box))
						{
							return true;
						}
					}
					if (mark[0] && mark[4] && mark[5])
					{
						if (this->BBVisible(node.first_child_index + 6, box))
						{
							return true;
						}
					}
					if (mark[3] && mark[4] && mark[5])
					{
						if (this->BBVisible(node.first_child_index + 7, box))
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
