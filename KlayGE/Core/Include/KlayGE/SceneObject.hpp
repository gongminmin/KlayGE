// SceneObject.hpp
// KlayGE 场景对象类 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2005-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 增加了Overlay标志 (2009.5.13)
// 增加了Update (2009.5.14)
//
// 3.1.0
// 初次建立 (2005.10.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _SCENEOBJECT_HPP
#define _SCENEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Renderable.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneObject : public std::enable_shared_from_this<SceneObject>
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
		explicit SceneObject(uint32_t attrib);
		virtual ~SceneObject();

		SceneObject* Parent() const;
		void Parent(SceneObject* so);
		uint32_t NumChildren() const;
		const SceneObjectPtr& Child(uint32_t index) const;

		RenderablePtr const & GetRenderable() const;

		virtual void ModelMatrix(float4x4 const & mat);
		virtual float4x4 const & ModelMatrix() const;
		virtual float4x4 const & AbsModelMatrix() const;
		virtual AABBox const & PosBoundWS() const;
		void UpdateAbsModelMatrix();
		void VisibleMark(BoundOverlap vm);
		BoundOverlap VisibleMark() const;

		virtual void OnAttachRenderable(bool add_to_scene);

		virtual void AddToSceneManager();
		virtual void AddToSceneManagerLocked();
		virtual void DelFromSceneManager();
		virtual void DelFromSceneManagerLocked();

		void BindSubThreadUpdateFunc(std::function<void(SceneObject&, float, float)> const & update_func);
		void BindMainThreadUpdateFunc(std::function<void(SceneObject&, float, float)> const & update_func);

		virtual void SubThreadUpdate(float app_time, float elapsed_time);
		virtual bool MainThreadUpdate(float app_time, float elapsed_time);

		uint32_t Attrib() const;
		bool Visible() const;
		void Visible(bool vis);

		vertex_elements_type const & InstanceFormat() const;
		virtual void const * InstanceData() const;

		// For select mode
		virtual void ObjectID(uint32_t id);
		virtual void SelectMode(bool select_mode);
		bool SelectMode() const;

		// For deferred only
		virtual void Pass(PassType type);

		bool TransparencyBackFace() const;
		bool TransparencyFrontFace() const;
		bool SSS() const;
		bool Reflection() const;
		bool SimpleForward() const;
		bool VDM() const;

	protected:
		uint32_t attrib_;

		SceneObject* parent_;
		std::vector<SceneObjectPtr> children_;

		RenderablePtr renderable_;
		bool renderable_hw_res_ready_;
		vertex_elements_type instance_format_;

		float4x4 model_;
		float4x4 abs_model_;
		std::unique_ptr<AABBox> pos_aabb_ws_;
		BoundOverlap visible_mark_;

		std::function<void(SceneObject&, float, float)> sub_thread_update_func_;
		std::function<void(SceneObject&, float, float)> main_thread_update_func_;
	};
}

#endif		// _SCENEOBJECT_HPP
