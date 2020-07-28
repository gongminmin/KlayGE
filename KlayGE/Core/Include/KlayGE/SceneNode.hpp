/**
 * @file SceneNode.hpp
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

#ifndef KLAYGE_CORE_SCENE_NODE_HPP
#define KLAYGE_CORE_SCENE_NODE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/SceneComponent.hpp>
#include <KlayGE/Signal.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneNode final : boost::noncopyable, public std::enable_shared_from_this<SceneNode>
	{
	public:
		enum SOAttrib
		{
			SOA_Cullable = 1UL << 0,
			SOA_Overlay = 1UL << 1,
			SOA_Moveable = 1UL << 2,
			SOA_Invisible = 1UL << 3,
			SOA_NotCastShadow = 1UL << 4,
			SOA_SSS = 1UL << 5
		};

	public:
		explicit SceneNode(uint32_t attrib);
		SceneNode(std::wstring_view name, uint32_t attrib);
		SceneNode(SceneComponentPtr const& component, uint32_t attrib);
		SceneNode(SceneComponentPtr const& component, std::wstring_view name, uint32_t attrib);
		~SceneNode();

		std::wstring_view Name() const;
		void Name(std::wstring_view name);

		SceneNode* FindFirstNode(std::wstring_view name);
		std::vector<SceneNode*> FindAllNode(std::wstring_view name);

		bool IsNodeInSubTree(SceneNode const * node);

		SceneNode* Parent() const;

		std::vector<SceneNodePtr> const & Children() const;
		void AddChild(SceneNodePtr const & node);
		void RemoveChild(SceneNodePtr const & node);
		void RemoveChild(SceneNode* node);
		void ClearChildren();

		void Traverse(std::function<bool(SceneNode&)> const & callback);

		uint32_t NumComponents() const;
		template <typename T>
		uint32_t NumComponentsOfType() const
		{
			uint32_t ret = 0;
			this->ForEachComponentOfType<T>([&ret](T& component) {
				KFL_UNUSED(component);
				++ret;
			});
			return ret;
		}
		SceneComponent* FirstComponent();
		SceneComponent const* FirstComponent() const;
		SceneComponent* ComponentByIndex(uint32_t i);
		SceneComponent const* ComponentByIndex(uint32_t i) const;
		template <typename T>
		T* FirstComponentOfType()
		{
			for (auto const& component : components_)
			{
				T* casted = boost::typeindex::runtime_cast<T*>(component.get());
				if (casted != nullptr)
				{
					return casted;
				}
			}
			return nullptr;
		}
		template <typename T>
		T const* FirstComponentOfType() const
		{
			for (auto const& component : components_)
			{
				T const* casted = boost::typeindex::runtime_cast<T*>(component.get());
				if (casted != nullptr)
				{
					return casted;
				}
			}
			return nullptr;
		}

		void AddComponent(SceneComponentPtr const& component);
		void RemoveComponent(SceneComponentPtr const& component);
		void RemoveComponent(SceneComponent* component);
		void ClearComponents();
		void ReplaceComponent(uint32_t index, SceneComponentPtr const& component);

		void ForEachComponent(std::function<void(SceneComponent&)> const & callback) const;
		template <typename T>
		void ForEachComponentOfType(std::function<void(T&)> const & callback) const
		{
			this->ForEachComponent([&](SceneComponent& component) {
				T* casted = boost::typeindex::runtime_cast<T*>(&component);
				if (casted != nullptr)
				{
					callback(*casted);
				}
			});
		}

		void TransformToParent(float4x4 const& mat);
		void TransformToWorld(float4x4 const& mat);
		float4x4 const& TransformToParent() const;
		float4x4 const& InverseTransformToParent() const;
		float4x4 const& TransformToWorld() const;
		float4x4 const& InverseTransformToWorld() const;
		float4x4 const& PrevTransformToWorld() const;
		AABBox const& PosBoundOS() const;
		AABBox const& PosBoundWS() const;
		void UpdateTransforms();
		void UpdatePosBoundSubtree();
		bool Updated() const;
		void FillVisibleMark(BoundOverlap vm);
		void VisibleMark(uint32_t camera_index, BoundOverlap vm);
		BoundOverlap VisibleMark(uint32_t camera_index) const;

		using UpdateEvent = Signal::Signal<void(SceneNode&, float, float)>;
		UpdateEvent& OnSubThreadUpdate()
		{
			return sub_thread_update_event_;
		}
		UpdateEvent& OnMainThreadUpdate()
		{
			return main_thread_update_event_;
		}

		void SubThreadUpdate(float app_time, float elapsed_time);
		void MainThreadUpdate(float app_time, float elapsed_time);

		uint32_t Attrib() const;
		bool Visible() const;
		void Visible(bool vis);

		std::vector<VertexElement>& InstanceFormat();
		std::vector<VertexElement> const & InstanceFormat() const;
		void InstanceData(void* data);
		void const * InstanceData() const;

		// For select mode
		void ObjectID(uint32_t id);
		void SelectMode(bool select_mode);
		bool SelectMode() const;

		// For deferred only
		void Pass(PassType type);

		bool TransparencyBackFace() const;
		bool TransparencyFrontFace() const;
		bool SSS() const;
		bool Reflection() const;
		bool SimpleForward() const;
		bool VDM() const;

	private:
		void FindAllNode(std::vector<SceneNode*>& nodes, std::wstring_view name);

		void Parent(SceneNode* so);
		void EmitSceneChanged();

	protected:
		std::wstring name_;

		uint32_t attrib_;

		SceneNode* parent_ = nullptr;
		std::vector<SceneNodePtr> children_;

		std::vector<SceneComponentPtr> components_;
		std::vector<VertexElement> instance_format_;
		void* instance_data_;

		float4x4 xform_to_parent_  = float4x4::Identity();
		mutable float4x4 xform_to_world_ = float4x4::Identity();
		mutable float4x4 prev_xform_to_world_ = float4x4::Identity();
		float4x4 inv_xform_to_parent_ = float4x4::Identity();
		mutable float4x4 inv_xform_to_world_ = float4x4::Identity();
		std::unique_ptr<AABBox> pos_aabb_os_;
		std::unique_ptr<AABBox> pos_aabb_ws_;
		bool pos_aabb_dirty_ = true;
		std::array<BoundOverlap, RenderEngine::PredefinedCameraCBuffer::max_num_cameras> visible_marks_;

		UpdateEvent sub_thread_update_event_;
		UpdateEvent main_thread_update_event_;

		bool updated_ = false;
	};
}

#endif		// KLAYGE_CORE_SCENE_NODE_HPP
