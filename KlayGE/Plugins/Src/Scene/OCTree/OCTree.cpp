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
#include <KlayGE/SceneObjectHelper.hpp>
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
			: RenderableLineBox(Box(float3(-1, -1, -1), float3(1, 1, 1)), Color(1, 1, 1, 1))
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
	OCTree::OCTree(uint32_t max_tree_depth)
		: max_tree_depth_(std::min<uint32_t>(max_tree_depth, 16UL)),
			rebuild_tree_(false)
	{
	}

	void OCTree::ClipScene()
	{
		if (rebuild_tree_)
		{
			typedef std::vector<size_t> ObjIndicesTypes;
			std::vector<ObjIndicesTypes> obj_indices(1);
			octree_.resize(1);
			Box bb_root(float3(0, 0, 0), float3(0, 0, 0));
			octree_[0].first_child_index = -1;
			std::vector<float3> aabbs_center_in_ws(scene_objs_.size());
			std::vector<float3> aabbs_half_size_in_ws(scene_objs_.size());
			for (size_t i = 0; i < scene_objs_.size(); ++ i)
			{
				SceneObjectPtr const & obj = scene_objs_[i];
				if (obj->Cullable() && !obj->Overlay() && !obj->Moveable())
				{
					Box const & box = obj->GetBound();
					float4x4 const & mat = obj->GetModelMatrix();

					float3 min, max;
					min = max = MathLib::transform_coord(box[0], mat);
					for (size_t j = 1; j < 8; ++ j)
					{
						float3 vec = MathLib::transform_coord(box[j], mat);
						min = MathLib::minimize(min, vec);
						max = MathLib::maximize(max, vec);
					}

					Box aabb_in_ws(min, max);

					bb_root |= aabb_in_ws;
					obj_indices[0].push_back(i);

					aabbs_center_in_ws[i] = aabb_in_ws.Center();
					aabbs_half_size_in_ws[i] = aabb_in_ws.HalfSize();
				}
			}
			{
				float3 const & size = bb_root.Max() - bb_root.Min();
				float max_dim = std::max(std::max(size.x(), size.y()), size.z()) / 2;
				octree_[0].bb_center = bb_root.Center();
				octree_[0].bb_half_size = float3(max_dim, max_dim, max_dim);
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
						float3 const parent_center = octree_[i].bb_center;
						float3 const new_half_size = octree_[i].bb_half_size / 2;
						octree_[i].first_child_index = static_cast<int>(base_address_[d] + octree_.size() - original_size);

						for (size_t j = 0; j < 8; ++ j)
						{
							octree_.push_back(octree_node_t());
							octree_node_t& new_node = octree_.back();
							new_node.first_child_index = -1;
							new_node.bb_half_size = new_half_size;
							obj_indices.push_back(ObjIndicesTypes());
							ObjIndicesTypes& new_node_obj_indices = obj_indices.back();
							ObjIndicesTypes& parent_obj_indices = obj_indices[i];

							if (j & 1)
							{
								new_node.bb_center.x() = parent_center.x() + new_half_size.x();
							}
							else
							{
								new_node.bb_center.x() = parent_center.x() - new_half_size.x();
							}
							if (j & 2)
							{
								new_node.bb_center.y() = parent_center.y() + new_half_size.y();
							}
							else
							{
								new_node.bb_center.y() = parent_center.y() - new_half_size.y();
							}
							if (j & 4)
							{
								new_node.bb_center.z() = parent_center.z() + new_half_size.z();
							}
							else
							{
								new_node.bb_center.z() = parent_center.z() - new_half_size.z();
							}

							BOOST_FOREACH(size_t obj_index, parent_obj_indices)
							{
								float3 const t = aabbs_center_in_ws[obj_index] - new_node.bb_center;
								float3 const e = aabbs_half_size_in_ws[obj_index] + new_node.bb_half_size;
								if ((abs(t.x()) <= e.x()) && (abs(t.y()) <= e.y()) && (abs(t.z()) <= e.z()))
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
		scene_objs_.resize(0);
		octree_.clear();
		rebuild_tree_ = true;
	}

	void OCTree::DoAddSceneObject(SceneObjectPtr const & obj)
	{
		scene_objs_.push_back(obj);
		if (obj->Cullable() && !obj->Overlay() && !obj->Moveable())
		{
			rebuild_tree_ = true;
		}
	}

	SceneManager::SceneObjectsType::iterator OCTree::DoDelSceneObject(SceneManager::SceneObjectsType::iterator iter)
	{
		if ((*iter)->Cullable() && !(*iter)->Overlay() && !(*iter)->Moveable())
		{
			rebuild_tree_ = true;
		}
		return scene_objs_.erase(iter);
	}

	void OCTree::NodeVisible(size_t index)
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t& node = octree_[index];
		Frustum::VIS const vis = frustum_.Visiable(Box(node.bb_center - node.bb_half_size, node.bb_center + node.bb_half_size));
		node.visible = vis;
		if (Frustum::VIS_PART == vis)
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
			checked_pointer_cast<NodeRenderable>(node_renderable_)->AddInstance(MathLib::scaling(node.bb_half_size) * MathLib::translation(node.bb_center));
		}
#endif
	}

	bool OCTree::AABBVisible(Box const & box)
	{
		// Frustum VS node
		bool visible = true;
		if (!octree_.empty())
		{
			visible = this->BBVisible(0, box.Center(), box.HalfSize());
		}
		if (visible)
		{
			// Frustum VS AABB
			Frustum::VIS const vis = frustum_.Visiable(box);
			visible = (vis != Frustum::VIS_NO);
		}
		return visible;
	}

	bool OCTree::BBVisible(size_t index, float3 const & bb_center, float3 const & bb_half_size)
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		float3 const t = bb_center - node.bb_center;
		float3 const e = bb_half_size + node.bb_half_size;
		if ((abs(t.x()) <= e.x()) && (abs(t.y()) <= e.y()) && (abs(t.z()) <= e.z()))
		{
			Frustum::VIS const vis = node.visible;
			switch (vis)
			{
			case Frustum::VIS_YES:
				return true;

			case Frustum::VIS_NO:
				return false;

			case Frustum::VIS_PART:
				if (node.first_child_index != -1)
				{
					for (int i = node.first_child_index, i_end = node.first_child_index + 8; i < i_end; ++ i)
					{
						if (this->BBVisible(i, bb_center, bb_half_size))
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
