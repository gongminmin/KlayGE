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

		this->FillVisibleMark(BoundOverlap::No);
	}

	SceneNode::SceneNode(std::wstring_view name, uint32_t attrib)
		: SceneNode(attrib)
	{
		name_ = std::wstring(name);
	}

	SceneNode::SceneNode(SceneComponentPtr const& component, uint32_t attrib)
		: SceneNode(attrib)
	{
		this->AddComponent(component);
	}

	SceneNode::SceneNode(SceneComponentPtr const& component, std::wstring_view name, uint32_t attrib)
		: SceneNode(component, attrib)
	{
		name_ = std::wstring(name);
	}

	SceneNode::~SceneNode()
	{
		for (auto& component : components_)
		{
			component->BindSceneNode(nullptr);
		}
		for (auto& child : children_)
		{
			child->Parent(nullptr);
		}
		if (parent_)
		{
			parent_->RemoveChild(this);
		}
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
			for (auto const& child : children_)
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

	bool SceneNode::IsNodeInSubTree(SceneNode const* node)
	{
		if (node == this)
		{
			return true;
		}
		else
		{
			return std::any_of(
				children_.begin(), children_.end(), [node](SceneNodePtr const& child) { return child->IsNodeInSubTree(node); });
		}
	}

	SceneNode* SceneNode::Parent() const
	{
		return parent_;
	}

	void SceneNode::Parent(SceneNode* so)
	{
		parent_ = so;

		pos_aabb_dirty_ = true;
		updated_ = false;
	}

	std::vector<SceneNodePtr> const& SceneNode::Children() const
	{
		return children_;
	}

	void SceneNode::AddChild(SceneNodePtr const& node)
	{
		auto iter = std::find(children_.begin(), children_.end(), node);
		if (iter == children_.end())
		{
			pos_aabb_dirty_ = true;
			node->Parent(this);
			children_.push_back(node);
		}
	}

	void SceneNode::RemoveChild(SceneNodePtr const& node)
	{
		this->RemoveChild(node.get());
	}

	void SceneNode::RemoveChild(SceneNode* node)
	{
		auto iter = std::find_if(children_.begin(), children_.end(), [node](SceneNodePtr const& child) { return child.get() == node; });
		if (iter != children_.end())
		{
			pos_aabb_dirty_ = true;
			node->Parent(nullptr);
			children_.erase(iter);

			this->EmitSceneChanged();
		}
	}

	void SceneNode::ClearChildren()
	{
		for (auto const& child : children_)
		{
			child->Parent(nullptr);
		}

		pos_aabb_dirty_ = true;
		children_.clear();

		this->EmitSceneChanged();
	}

	void SceneNode::Traverse(std::function<bool(SceneNode&)> const& callback)
	{
		if (callback(*this))
		{
			for (auto const& child : children_)
			{
				child->Traverse(callback);
			}
		}
	}

	uint32_t SceneNode::NumComponents() const
	{
		return static_cast<uint32_t>(components_.size());
	}

	SceneComponent* SceneNode::FirstComponent()
	{
		return this->ComponentByIndex(0);
	}

	SceneComponent const* SceneNode::FirstComponent() const
	{
		return this->ComponentByIndex(0);
	}

	SceneComponent* SceneNode::ComponentByIndex(uint32_t i)
	{
		return components_[i].get();
	}

	SceneComponent const* SceneNode::ComponentByIndex(uint32_t i) const
	{
		return components_[i].get();
	}

	void SceneNode::AddComponent(SceneComponentPtr const& component)
	{
		BOOST_ASSERT(component);

		auto* curr_node = component->BoundSceneNode();
		if (curr_node != nullptr)
		{
			curr_node->RemoveComponent(component);
		}

		components_.push_back(component);
		component->BindSceneNode(this);
		pos_aabb_dirty_ = true;
	}

	void SceneNode::RemoveComponent(SceneComponentPtr const& component)
	{
		this->RemoveComponent(component.get());
	}

	void SceneNode::RemoveComponent(SceneComponent* component)
	{
		auto iter =
			std::find_if(components_.begin(), components_.end(), [component](SceneComponentPtr const& comp) { return comp.get() == component; });
		if (iter != components_.end())
		{
			components_.erase(iter);
			component->BindSceneNode(nullptr);
			pos_aabb_dirty_ = true;
		}
	}

	void SceneNode::ClearComponents()
	{
		components_.clear();
		pos_aabb_dirty_ = true;
	}

	void SceneNode::ReplaceComponent(uint32_t index, SceneComponentPtr const& component)
	{
		BOOST_ASSERT(component);

		auto* curr_node = component->BoundSceneNode();
		if (curr_node != nullptr)
		{
			curr_node->RemoveComponent(component);
		}

		if (components_[index] != nullptr)
		{
			components_[index]->BindSceneNode(nullptr);
		}

		component->BindSceneNode(this);
		components_[index] = component;
		pos_aabb_dirty_ = true;
	}

	void SceneNode::ForEachComponent(std::function<void(SceneComponent&)> const& callback) const
	{
		for (auto const& component : components_)
		{
			if (component)
			{
				callback(*component);
			}
		}
	}

	void SceneNode::TransformToParent(float4x4 const& mat)
	{
		xform_to_parent_ = mat;
		inv_xform_to_parent_ = MathLib::inverse(mat);
		pos_aabb_dirty_ = true;
	}

	void SceneNode::TransformToWorld(float4x4 const& mat)
	{
		if (parent_)
		{
			xform_to_parent_ = mat * parent_->InverseTransformToWorld();
		}
		else
		{
			xform_to_parent_ = mat;
		}
		inv_xform_to_parent_ = MathLib::inverse(mat);

		pos_aabb_dirty_ = true;
	}

	float4x4 const& SceneNode::TransformToParent() const
	{
		return xform_to_parent_;
	}

	float4x4 const& SceneNode::InverseTransformToParent() const
	{
		return inv_xform_to_parent_;
	}

	float4x4 const& SceneNode::TransformToWorld() const
	{
		if (parent_ == nullptr)
		{
			return xform_to_parent_;
		}
		else
		{
			auto& scene_mgr = Context::Instance().SceneManagerInstance();
			if (!scene_mgr.NodesUpdated())
			{
				auto* parent = this->Parent();
				xform_to_world_ = xform_to_parent_;
				while (parent != nullptr)
				{
					xform_to_world_ *= parent->TransformToParent();
					parent = parent->Parent();
				}
			}
			return xform_to_world_;
		}
	}

	float4x4 const& SceneNode::PrevTransformToWorld() const
	{
		return prev_xform_to_world_;
	}

	float4x4 const& SceneNode::InverseTransformToWorld() const
	{
		if (parent_ == nullptr)
		{
			return inv_xform_to_parent_;
		}
		else
		{
			auto& scene_mgr = Context::Instance().SceneManagerInstance();
			if (!scene_mgr.NodesUpdated())
			{
				inv_xform_to_world_ = MathLib::inverse(this->TransformToWorld());
			}
			return inv_xform_to_world_;
		}
	}

	AABBox const& SceneNode::PosBoundOS() const
	{
		return *pos_aabb_os_;
	}

	AABBox const& SceneNode::PosBoundWS() const
	{
		return *pos_aabb_ws_;
	}

	void SceneNode::UpdateTransforms()
	{
		prev_xform_to_world_ = xform_to_world_;
		if (parent_)
		{
			xform_to_world_ = xform_to_parent_ * parent_->TransformToWorld();
		}
		else
		{
			xform_to_world_ = xform_to_parent_;
		}
		inv_xform_to_world_ = MathLib::inverse(xform_to_world_);

		pos_aabb_dirty_ = true;
	}

	bool SceneNode::Updated() const
	{
		return updated_ && !pos_aabb_dirty_;
	}

	void SceneNode::FillVisibleMark(BoundOverlap vm)
	{
		visible_marks_.fill(vm);
	}

	void SceneNode::VisibleMark(uint32_t camera_index, BoundOverlap vm)
	{
		BOOST_ASSERT(camera_index < visible_marks_.size());
		visible_marks_[camera_index] = vm;
	}

	BoundOverlap SceneNode::VisibleMark(uint32_t camera_index) const
	{
		BOOST_ASSERT(camera_index < visible_marks_.size());
		return visible_marks_[camera_index];
	}

	void SceneNode::SubThreadUpdate(float app_time, float elapsed_time)
	{
		sub_thread_update_event_(*this, app_time, elapsed_time);

		for (auto const& component : components_)
		{
			component->SubThreadUpdate(app_time, elapsed_time);
		}
	}

	void SceneNode::MainThreadUpdate(float app_time, float elapsed_time)
	{
		main_thread_update_event_(*this, app_time, elapsed_time);

		for (auto const& component : components_)
		{
			component->MainThreadUpdate(app_time, elapsed_time);
		}

		if (!updated_)
		{
			this->EmitSceneChanged();

			updated_ = true;
		}
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

	std::vector<VertexElement>& SceneNode::InstanceFormat()
	{
		return instance_format_;
	}

	std::vector<VertexElement> const & SceneNode::InstanceFormat() const
	{
		return instance_format_;
	}

	void SceneNode::InstanceData(void* data)
	{
		instance_data_ = data;
	}

	void const * SceneNode::InstanceData() const
	{
		return instance_data_;
	}

	void SceneNode::SelectMode(bool select_mode)
	{
		this->ForEachComponentOfType<RenderableComponent>(
			[select_mode](RenderableComponent& renderable_comp) { renderable_comp.BoundRenderable().SelectMode(select_mode); });
	}

	void SceneNode::ObjectID(uint32_t id)
	{
		this->ForEachComponentOfType<RenderableComponent>(
			[id](RenderableComponent& renderable_comp) { renderable_comp.BoundRenderable().ObjectID(id); });
	}

	bool SceneNode::SelectMode() const
	{
		auto const* renderable_comp = this->FirstComponentOfType<RenderableComponent>();
		if (renderable_comp != nullptr)
		{
			return renderable_comp->BoundRenderable().SelectMode();
		}
		else
		{
			return false;
		}
	}

	void SceneNode::Pass(PassType type)
	{
		this->ForEachComponentOfType<RenderableComponent>(
			[type](RenderableComponent& renderable_comp) { renderable_comp.BoundRenderable().Pass(type); });

		if (attrib_ & SOA_NotCastShadow)
		{
			this->Visible(PC_ShadowMap != GetPassCategory(type));
		}
	}

	bool SceneNode::TransparencyBackFace() const
	{
		auto const* renderable_comp = this->FirstComponentOfType<RenderableComponent>();
		if (renderable_comp != nullptr)
		{
			return renderable_comp->BoundRenderable().TransparencyBackFace();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::TransparencyFrontFace() const
	{
		auto const* renderable_comp = this->FirstComponentOfType<RenderableComponent>();
		if (renderable_comp != nullptr)
		{
			return renderable_comp->BoundRenderable().TransparencyFrontFace();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::SSS() const
	{
		auto const* renderable_comp = this->FirstComponentOfType<RenderableComponent>();
		if (renderable_comp != nullptr)
		{
			return renderable_comp->BoundRenderable().SSS();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::Reflection() const
	{
		auto const* renderable_comp = this->FirstComponentOfType<RenderableComponent>();
		if (renderable_comp != nullptr)
		{
			return renderable_comp->BoundRenderable().Reflection();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::SimpleForward() const
	{
		auto const* renderable_comp = this->FirstComponentOfType<RenderableComponent>();
		if (renderable_comp != nullptr)
		{
			return renderable_comp->BoundRenderable().SimpleForward();
		}
		else
		{
			return false;
		}
	}

	bool SceneNode::VDM() const
	{
		auto const* renderable_comp = this->FirstComponentOfType<RenderableComponent>();
		if (renderable_comp != nullptr)
		{
			return renderable_comp->BoundRenderable().VDM();
		}
		else
		{
			return false;
		}
	}

	void SceneNode::UpdatePosBoundSubtree()
	{
		for (auto const & child : children_)
		{
			child->UpdatePosBoundSubtree();
		}

		if (pos_aabb_dirty_)
		{
			if (pos_aabb_os_)
			{
				pos_aabb_os_->Min() = float3(+1e10f, +1e10f, +1e10f);
				pos_aabb_os_->Max() = float3(-1e10f, -1e10f, -1e10f);

				for (auto const& component : components_)
				{
					auto const* renderable_comp = boost::typeindex::runtime_cast<RenderableComponent*>(component.get());
					if (renderable_comp != nullptr)
					{
						*pos_aabb_os_ |= renderable_comp->BoundRenderable().PosBound();
					}
				}

				for (auto const & child : children_)
				{
					if (child->pos_aabb_os_)
					{
						if ((child->pos_aabb_os_->Min().x() < child->pos_aabb_os_->Max().x())
							|| (child->pos_aabb_os_->Min().y() < child->pos_aabb_os_->Max().y())
							|| (child->pos_aabb_os_->Min().z() < child->pos_aabb_os_->Max().z()))
						{
							*pos_aabb_os_ |= MathLib::transform_aabb(*child->pos_aabb_os_, child->TransformToParent());
						}
					}
				}

				*pos_aabb_ws_ = MathLib::transform_aabb(*pos_aabb_os_, xform_to_world_);
			}

			pos_aabb_dirty_ = false;
		}
	}

	void SceneNode::EmitSceneChanged()
	{
		auto& context = Context::Instance();
		if (context.SceneManagerValid())
		{
			auto* node = this;
			while (node->Parent() != nullptr)
			{
				node = node->Parent();
			}

			auto& scene_mgr = context.SceneManagerInstance();
			if (node == &scene_mgr.SceneRootNode())
			{
				scene_mgr.OnSceneChanged();
			}
		}
	}
}
