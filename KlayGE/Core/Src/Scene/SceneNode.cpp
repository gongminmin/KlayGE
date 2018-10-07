/**
 * @file SceneNode.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/string_view.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Renderable.hpp>

#include <boost/assert.hpp>

#include <KlayGE/SceneNode.hpp>

namespace KlayGE
{
	SceneNode::SceneNode(uint32_t attrib)
		: attrib_(attrib)
	{
		if (!(attrib & SOA_Overlay) && (attrib & (SOA_Cullable | SOA_Moveable)))
		{
			pos_aabb_os_ = MakeUniquePtr<AABBox>();
			pos_aabb_ws_ = MakeUniquePtr<AABBox>();
		}
	}

	SceneNode::SceneNode(std::wstring_view name, uint32_t attrib)
		: SceneNode(attrib)
	{
		name_ = std::wstring(name);
	}

	SceneNode::SceneNode(RenderablePtr const & renderable, uint32_t attrib)
		: SceneNode(attrib)
	{
		this->AddRenderable(renderable);
	}

	SceneNode::SceneNode(RenderablePtr const & renderable, std::wstring_view name, uint32_t attrib)
		: SceneNode(renderable, attrib)
	{
		name_ = std::wstring(name);
	}

	SceneNode::~SceneNode()
	{
	}

	std::wstring_view SceneNode::Name() const
	{
		return name_;
	}

	void SceneNode::Name(std::wstring_view name)
	{
		name_ = std::wstring(name);
	}

	SceneNode* SceneNode::FindFirstNode(std::wstring_view name)
	{
		SceneNode* ret = nullptr;
		if (name_ == name)
		{
			ret = this;
		}
		else
		{
			for (auto const & child : children_)
			{
				ret = child->FindFirstNode(name);
				if (ret != nullptr)
				{
					break;
				}
			}
		}

		return ret;
	}

	std::vector<SceneNode*> SceneNode::FindAllNode(std::wstring_view name)
	{
		std::vector<SceneNode*> ret;
		this->FindAllNode(ret, name);
		return ret;
	}

	void SceneNode::FindAllNode(std::vector<SceneNode*>& nodes, std::wstring_view name)
	{
		this->Traverse([&nodes, name](SceneNode& node)
			{
				if (node.name_ == name)
				{
					nodes.push_back(&node);
				}

				return true;
			});
	}

	bool SceneNode::IsNodeInSubTree(SceneNode const * node)
	{
		if (node == this)
		{
			return true;
		}
		else
		{
			for (auto const & child : children_)
			{
				if (child->IsNodeInSubTree(node))
				{
					return true;
				}
			}
		}

		return false;
	}

	SceneNode* SceneNode::Parent() const
	{
		return parent_;
	}

	void SceneNode::Parent(SceneNode* so)
	{
		parent_ = so;
	}

	std::vector<SceneNodePtr> const & SceneNode::Children() const
	{
		return children_;
	}

	void SceneNode::AddChild(SceneNodePtr const & node)
	{
		auto iter = std::find(children_.begin(), children_.end(), node);
		if (iter == children_.end())
		{
			pos_aabb_dirty_ = true;
			node->Parent(this);
			children_.push_back(node);
		}
	}

	void SceneNode::RemoveChild(SceneNodePtr const & node)
	{
		auto iter = std::find(children_.begin(), children_.end(), node);
		if (iter != children_.end())
		{
			pos_aabb_dirty_ = true;
			node->Parent(nullptr);
			children_.erase(iter);
		}
	}

	void SceneNode::ClearChildren()
	{
		for (auto const & child : children_)
		{
			child->Parent(nullptr);
		}

		pos_aabb_dirty_ = true;
		children_.clear();
	}

	void SceneNode::MainThreadUpdateNode(float app_time, float elapsed_time)
	{
		this->Traverse([app_time, elapsed_time](SceneNode& node)
			{
				node.MainThreadUpdate(app_time, elapsed_time);
				node.UpdateTransforms();

				return true;
			});
	}

	void SceneNode::SubThreadUpdateNode(float app_time, float elapsed_time)
	{
		this->Traverse([app_time, elapsed_time](SceneNode& node)
			{
				node.SubThreadUpdate(app_time, elapsed_time);

				return true;
			});
	}

	void SceneNode::Traverse(std::function<bool(SceneNode&)> const & callback)
	{
		if (callback(*this))
		{
			for (auto const & child : children_)
			{
				child->Traverse(callback);
			}
		}
	}

	uint32_t SceneNode::NumRenderables() const
	{
		return static_cast<uint32_t>(renderables_.size());
	}

	RenderablePtr const & SceneNode::GetRenderable() const
	{
		return this->GetRenderable(0);
	}

	RenderablePtr const & SceneNode::GetRenderable(uint32_t i) const
	{
		return renderables_[i];
	}

	void SceneNode::AddRenderable(RenderablePtr const & renderable)
	{
		renderables_.push_back(renderable);
		pos_aabb_dirty_ = true;
	}

	void SceneNode::DelRenderable(RenderablePtr const & renderable)
	{
		auto iter = std::find(renderables_.begin(), renderables_.end(), renderable);
		if (iter != renderables_.end())
		{
			renderables_.erase(iter);
			pos_aabb_dirty_ = true;
		}
	}

	void SceneNode::ClearRenderables()
	{
		renderables_.clear();
		pos_aabb_dirty_ = true;
	}

	void SceneNode::ForEachRenderable(std::function<void(Renderable&)> const & callback) const
	{
		for (auto const & renderable : renderables_)
		{
			if (renderable)
			{
				callback(*renderable);
			}
		}
	}

	void SceneNode::TransformToParent(float4x4 const & mat)
	{
		xform_to_parent_ = mat;
		pos_aabb_dirty_ = true;
	}

	void SceneNode::TransformToWorld(float4x4 const & mat)
	{
		if (parent_)
		{
			xform_to_parent_ = mat * MathLib::inverse(parent_->TransformToWorld());
		}
		else
		{
			xform_to_parent_ = mat;
		}
		pos_aabb_dirty_ = true;
	}

	float4x4 const & SceneNode::TransformToParent() const
	{
		return xform_to_parent_;
	}

	float4x4 const & SceneNode::TransformToWorld() const
	{
		return xform_to_world_;
	}

	AABBox const & SceneNode::PosBoundOS() const
	{
		return *pos_aabb_os_;
	}

	AABBox const & SceneNode::PosBoundWS() const
	{
		return *pos_aabb_ws_;
	}

	void SceneNode::UpdateTransforms()
	{
		if (parent_)
		{
			xform_to_world_ = xform_to_parent_ * parent_->TransformToWorld();
		}
		else
		{
			xform_to_world_ = xform_to_parent_;
		}

		if (pos_aabb_ws_)
		{
			this->UpdatePosBound();
			*pos_aabb_ws_ = MathLib::transform_aabb(*pos_aabb_os_, xform_to_world_);
		}
	}

	void SceneNode::VisibleMark(BoundOverlap vm)
	{
		visible_mark_ = vm;
	}

	BoundOverlap SceneNode::VisibleMark() const
	{
		return visible_mark_;
	}

	void SceneNode::SubThreadUpdate(float app_time, float elapsed_time)
	{
		sub_thread_update_event_(app_time, elapsed_time);
	}

	void SceneNode::MainThreadUpdate(float app_time, float elapsed_time)
	{
		main_thread_update_event_(app_time, elapsed_time);
	}

	uint32_t SceneNode::Attrib() const
	{
		return attrib_;
	}

	bool SceneNode::Visible() const
	{
		return (0 == (attrib_ & SOA_Invisible));
	}

	void SceneNode::Visible(bool vis)
	{
		if (vis)
		{
			attrib_ &= ~SOA_Invisible;
		}
		else
		{
			attrib_ |= SOA_Invisible;
		}

		for (auto const & child : children_)
		{
			child->Visible(vis);
		}
	}

	std::vector<VertexElement> const & SceneNode::InstanceFormat() const
	{
		return instance_format_;
	}

	void const * SceneNode::InstanceData() const
	{
		return nullptr;
	}

	void SceneNode::SelectMode(bool select_mode)
	{
		for (auto const & renderable : renderables_)
		{
			renderable->SelectMode(select_mode);
		}
	}

	void SceneNode::ObjectID(uint32_t id)
	{
		for (auto const & renderable : renderables_)
		{
			renderable->ObjectID(id);
		}
	}

	bool SceneNode::SelectMode() const
	{
		if (renderables_[0])
		{
			return renderables_[0]->SelectMode();
		}
		else
		{
			return false;
		}
	}

	void SceneNode::Pass(PassType type)
	{
		for (auto const & renderable : renderables_)
		{
			renderable->Pass(type);
		}

		if (attrib_ & SOA_NotCastShadow)
		{
			this->Visible(PC_ShadowMap != GetPassCategory(type));
		}
	}

	bool SceneNode::TransparencyBackFace() const
	{
		if (renderables_[0])
		{
			return renderables_[0]->TransparencyBackFace();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::TransparencyFrontFace() const
	{
		if (renderables_[0])
		{
			return renderables_[0]->TransparencyFrontFace();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::SSS() const
	{
		if (renderables_[0])
		{
			return renderables_[0]->SSS();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::Reflection() const
	{
		if (renderables_[0])
		{
			return renderables_[0]->Reflection();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::SimpleForward() const
	{
		if (renderables_[0])
		{
			return renderables_[0]->SimpleForward();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::VDM() const
	{
		if (renderables_[0])
		{
			return renderables_[0]->VDM();
		}
		else
		{
			return false;
		}
	}

	void SceneNode::UpdatePosBound()
	{
		for (auto const & child : children_)
		{
			child->UpdatePosBound();
		}

		if (pos_aabb_dirty_)
		{
			if (pos_aabb_os_)
			{
				pos_aabb_os_->Min() = float3(+1e10f, +1e10f, +1e10f);
				pos_aabb_os_->Max() = float3(-1e10f, -1e10f, -1e10f);

				if (!renderables_.empty())
				{
					*pos_aabb_os_ = renderables_[0]->PosBound();
					for (size_t i = 1; i < renderables_.size(); ++ i)
					{
						*pos_aabb_os_ |= renderables_[i]->PosBound();
					}
				}

				for (auto const & child : children_)
				{
					if (child->pos_aabb_os_)
					{
						if ((child->pos_aabb_os_->Min().x() < child->pos_aabb_os_->Max().x())
							&& (child->pos_aabb_os_->Min().y() < child->pos_aabb_os_->Max().y())
							&& (child->pos_aabb_os_->Min().z() < child->pos_aabb_os_->Max().z()))
						{
							*pos_aabb_os_ |= MathLib::transform_aabb(*child->pos_aabb_os_, child->TransformToParent());
						}
					}
				}
			}

			pos_aabb_dirty_ = false;
		}
	}
}
