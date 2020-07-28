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
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>

#include <algorithm>
#include <boost/assert.hpp>

#ifdef KLAYGE_DRAW_NODES
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
			octree_[0].visible = BoundOverlap::No;
			for (auto* sn : all_scene_nodes_)
			{
				auto const & node = *sn;
				uint32_t const attr = node.Attrib();
				if (node.Updated() && (attr & SceneNode::SOA_Cullable) && !(attr & SceneNode::SOA_Moveable))
				{
					bb_root |= node.PosBoundWS();
					octree_[0].node_ptrs.push_back(sn);
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

		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& viewport = *re.CurFrameBuffer()->Viewport();
		uint32_t const num_cameras = viewport.NumCameras();

		bool omni_directional = false;
		for (uint32_t i = 0; i < num_cameras; ++i)
		{
			omni_directional |= viewport.Camera(i)->OmniDirectionalMode();
		}

		if (omni_directional)
		{
			for (auto* sn : all_scene_nodes_)
			{
				auto& node = *sn;
				if (node.Visible())
				{
					if (node.Updated())
					{
						uint32_t const attr = node.Attrib();
						if (attr & SceneNode::SOA_Cullable)
						{
							if (small_obj_threshold_ > 0)
							{
								AABBox const& aabb_ws = node.PosBoundWS();
								for (uint32_t i = 0; i < num_cameras; ++i)
								{
									auto const& camera = *viewport.Camera(i);
									float4x4 const& view_proj = camera_view_projs_[i];

									node.VisibleMark(
										i, ((MathLib::ortho_area(camera.ForwardVec(), aabb_ws) > small_obj_threshold_) &&
											   (MathLib::perspective_area(camera.EyePos(), view_proj, aabb_ws) > small_obj_threshold_))
											   ? BoundOverlap::Yes
											   : BoundOverlap::No);
								}
							}
							else
							{
								for (uint32_t i = 0; i < num_cameras; ++i)
								{
									node.VisibleMark(i, BoundOverlap::Yes);
								}
							}
						}
						else
						{
							for (uint32_t i = 0; i < num_cameras; ++i)
							{
								node.VisibleMark(i, BoundOverlap::Yes);
							}
						}
					}
					else
					{
						for (uint32_t i = 0; i < num_cameras; ++i)
						{
							node.VisibleMark(i, BoundOverlap::Yes);
						}
					}
				}
				else
				{
					for (uint32_t i = 0; i < num_cameras; ++i)
					{
						node.VisibleMark(i, BoundOverlap::No);
					}
				}
			}
		}
		else
		{
			if (!octree_.empty())
			{
				this->MarkNodeObjs(0, false);
			}

			for (auto* sn : all_scene_nodes_)
			{
				auto& node = *sn;
				uint32_t const attr = node.Attrib();
				if (node.Visible() && (attr & SceneNode::SOA_Cullable) && (attr & SceneNode::SOA_Moveable))
				{
					for (uint32_t i = 0; i < num_cameras; ++i)
					{
						if (node.VisibleMark(i) == BoundOverlap::Partial)
						{
							node.VisibleMark(i, camera_frustums_[i]->Intersect(node.PosBoundWS()));
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

	void OCTree::OnSceneChanged()
	{
		rebuild_tree_ = true;
	}

	void OCTree::DoSuspend()
	{
		// TODO
	}

	void OCTree::DoResume()
	{
		// TODO
	}

	void OCTree::DivideNode(size_t index, uint32_t curr_depth)
	{
		if (octree_[index].node_ptrs.size() > 1)
		{
			size_t const this_size = octree_.size();
			AABBox const parent_bb = octree_[index].bb;
			float3 const parent_center = parent_bb.Center();
			octree_[index].first_child_index = static_cast<int>(this_size);
			octree_[index].visible = BoundOverlap::No;

			octree_.resize(this_size + 8);
			for (auto* node : octree_[index].node_ptrs)
			{
				AABBox const & aabb = node->PosBoundWS();
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
						octree_[this_size + j].node_ptrs.push_back(node);
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

			octree_[index].node_ptrs.clear();
			octree_[index].node_ptrs.shrink_to_fit();
		}
	}

	void OCTree::NodeVisible(size_t index)
	{
		BOOST_ASSERT(index < octree_.size());

		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& viewport = *re.CurFrameBuffer()->Viewport();
		uint32_t const num_cameras = viewport.NumCameras();

		auto& octree_node = octree_[index];
		bool large_enough;
		if (small_obj_threshold_ <= 0)
		{
			large_enough = true;
		}
		else
		{
			large_enough = false;
			for (uint32_t i = 0; i < num_cameras; ++i)
			{
				auto const& camera = *viewport.Camera(i);
				float4x4 const& view_proj = camera_view_projs_[i];
				if (((MathLib::ortho_area(camera.ForwardVec(), octree_node.bb) > small_obj_threshold_)
					&& (MathLib::perspective_area(camera.EyePos(), view_proj, octree_node.bb) > small_obj_threshold_)))
				{
					large_enough = true;
					break;
				}
			}
		}

		if (large_enough)
		{
			octree_node.visible = SceneManager::AABBVisible(octree_node.bb);
			if (BoundOverlap::Partial == octree_node.visible)
			{
				if (octree_node.first_child_index != -1)
				{
					for (int i = 0; i < 8; ++ i)
					{
						this->NodeVisible(octree_node.first_child_index + i);
					}
				}
			}
		}
		else
		{
			octree_node.visible = BoundOverlap::No;
		}

#ifdef KLAYGE_DRAW_NODES
		if ((vis != BoundOverlap::No) && (-1 == node.first_child_index))
		{
			checked_pointer_cast<NodeRenderable>(node_renderable_)
				->AddInstance(MathLib::scaling(node.bb.HalfSize()) * MathLib::translation(node.bb.Center()));
		}
#endif
	}

	void OCTree::MarkNodeObjs(size_t index, bool force)
	{
		BOOST_ASSERT(index < octree_.size());

		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		auto const& viewport = *re.CurFrameBuffer()->Viewport();
		uint32_t const num_cameras = viewport.NumCameras();

		auto const & octree_node = octree_[index];
		if ((octree_node.visible != BoundOverlap::No) || force)
		{
			for (auto* node : octree_node.node_ptrs)
			{
				if (node->Visible())
				{
					if (node->Updated())
					{
						for (uint32_t i = 0; i < num_cameras; ++i)
						{
							if (node->VisibleMark(i) == BoundOverlap::No)
							{
								auto visible = this->VisibleTestFromParent(*node, i);
								if (BoundOverlap::Partial == visible)
								{
									if (node->Parent())
									{
										visible = camera_frustums_[i]->Intersect(node->PosBoundWS());
									}
									else
									{
										visible = BoundOverlap::No;
									}
								}

								node->VisibleMark(i, visible);
							}
						}
					}
					else
					{
						for (uint32_t i = 0; i < num_cameras; ++i)
						{
							node->VisibleMark(i, BoundOverlap::Yes);
						}
					}

					for (uint32_t i = 0; i < num_cameras; ++i)
					{
						if (node->VisibleMark(i) != BoundOverlap::No)
						{
							auto* override_node = node->Parent();
							while ((override_node != nullptr) && (override_node->VisibleMark(i) == BoundOverlap::No))
							{
								override_node->VisibleMark(i, BoundOverlap::Partial);
								override_node = override_node->Parent();
							}
						}
					}
				}
				else
				{
					for (uint32_t i = 0; i < num_cameras; ++i)
					{
						node->VisibleMark(i, BoundOverlap::No);
					}
				}
			}

			if (octree_node.first_child_index != -1)
			{
				for (int i = 0; i < 8; ++ i)
				{
					this->MarkNodeObjs(octree_node.first_child_index + i, (BoundOverlap::Yes == octree_node.visible) || force);
				}
			}
		}
	}

	BoundOverlap OCTree::AABBVisible(AABBox const & aabb) const
	{
		// Frustum VS node
		BoundOverlap visible = BoundOverlap::Yes;
		if (!octree_.empty())
		{
			if (MathLib::intersect_aabb_aabb(octree_[0].bb, aabb))
			{
				visible = this->BoundVisible(0, aabb);
			}
			else
			{
				// Out of scene
				visible = BoundOverlap::Yes;
			}
		}
		if (visible != BoundOverlap::No)
		{
			// Frustum VS AABB
			visible = SceneManager::AABBVisible(aabb);
		}
		return visible;
	}

	BoundOverlap OCTree::OBBVisible(OBBox const & obb) const
	{
		// Frustum VS node
		BoundOverlap visible = BoundOverlap::Yes;
		if (!octree_.empty())
		{
			if (MathLib::intersect_aabb_obb(octree_[0].bb, obb))
			{
				visible = this->BoundVisible(0, obb);
			}
			else
			{
				// Out of scene
				visible = BoundOverlap::Yes;
			}
		}
		if (visible != BoundOverlap::No)
		{
			// Frustum VS OBB
			visible = SceneManager::OBBVisible(obb);
		}
		return visible;
	}

	BoundOverlap OCTree::SphereVisible(Sphere const & sphere) const
	{
		// Frustum VS node
		BoundOverlap visible = BoundOverlap::Yes;
		if (!octree_.empty())
		{
			if (MathLib::intersect_aabb_sphere(octree_[0].bb, sphere))
			{
				visible = this->BoundVisible(0, sphere);
			}
			else
			{
				// Out of scene
				visible = BoundOverlap::Yes;
			}
		}
		if (visible != BoundOverlap::No)
		{
			// Frustum VS Sphere
			visible = SceneManager::SphereVisible(sphere);
		}
		return visible;
	}

	BoundOverlap OCTree::FrustumVisible(Frustum const& frustum) const
	{
		// Frustum VS node
		BoundOverlap visible = BoundOverlap::Yes;
		if (!octree_.empty())
		{
			if (MathLib::intersect_aabb_frustum(octree_[0].bb, frustum) != BoundOverlap::No)
			{
				visible = this->BoundVisible(0, frustum);
			}
			else
			{
				// Out of scene
				visible = BoundOverlap::Yes;
			}
		}
		if (visible != BoundOverlap::No)
		{
			// Frustum VS Frustum
			visible = SceneManager::FrustumVisible(frustum);
		}
		return visible;
	}

	BoundOverlap OCTree::BoundVisible(size_t index, AABBox const & aabb) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if ((node.visible != BoundOverlap::No) && MathLib::intersect_aabb_aabb(node.bb, aabb))
		{
			if (BoundOverlap::Yes == node.visible)
			{
				return BoundOverlap::Yes;
			}
			else
			{
				BOOST_ASSERT(BoundOverlap::Partial == node.visible);

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
							BoundOverlap const bo = this->BoundVisible(node.first_child_index + j, aabb);
							if (bo != BoundOverlap::No)
							{
								return bo;
							}
						}
					}

					return BoundOverlap::No;
				}
				else
				{
					return BoundOverlap::Partial;
				}
			}
		}
		else
		{
			return BoundOverlap::No;
		}
	}

	BoundOverlap OCTree::BoundVisible(size_t index, OBBox const & obb) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if ((node.visible != BoundOverlap::No) && MathLib::intersect_aabb_obb(node.bb, obb))
		{
			if (BoundOverlap::Yes == node.visible)
			{
				return BoundOverlap::Yes;
			}
			else
			{
				BOOST_ASSERT(BoundOverlap::Partial == node.visible);

				if (node.first_child_index != -1)
				{
					for (int i = 0; i < 8; ++ i)
					{
						BoundOverlap const bo = this->BoundVisible(node.first_child_index + i, obb);
						if (bo != BoundOverlap::No)
						{
							return bo;
						}
					}

					return BoundOverlap::No;
				}
				else
				{
					return BoundOverlap::Partial;
				}
			}
		}
		else
		{
			return BoundOverlap::No;
		}
	}

	BoundOverlap OCTree::BoundVisible(size_t index, Sphere const & sphere) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if ((node.visible != BoundOverlap::No) && MathLib::intersect_aabb_sphere(node.bb, sphere))
		{
			if (BoundOverlap::Yes == node.visible)
			{
				return BoundOverlap::Yes;
			}
			else
			{
				BOOST_ASSERT(BoundOverlap::Partial == node.visible);

				if (node.first_child_index != -1)
				{
					for (int i = 0; i < 8; ++ i)
					{
						BoundOverlap const bo = this->BoundVisible(node.first_child_index + i, sphere);
						if (bo != BoundOverlap::No)
						{
							return bo;
						}
					}

					return BoundOverlap::No;
				}
				else
				{
					return BoundOverlap::Partial;
				}
			}
		}
		else
		{
			return BoundOverlap::No;
		}
	}

	BoundOverlap OCTree::BoundVisible(size_t index, Frustum const & frustum) const
	{
		BOOST_ASSERT(index < octree_.size());

		octree_node_t const & node = octree_[index];
		if ((node.visible != BoundOverlap::No) && (MathLib::intersect_aabb_frustum(node.bb, frustum) != BoundOverlap::No))
		{
			if (BoundOverlap::Yes == node.visible)
			{
				return BoundOverlap::Yes;
			}
			else
			{
				BOOST_ASSERT(BoundOverlap::Partial == node.visible);

				if (node.first_child_index != -1)
				{
					for (int i = 0; i < 8; ++ i)
					{
						BoundOverlap const bo = this->BoundVisible(node.first_child_index + i, frustum);
						if (bo != BoundOverlap::No)
						{
							return bo;
						}
					}

					return BoundOverlap::No;
				}
				else
				{
					return BoundOverlap::Partial;
				}
			}
		}
		else
		{
			return BoundOverlap::No;
		}
	}
}
