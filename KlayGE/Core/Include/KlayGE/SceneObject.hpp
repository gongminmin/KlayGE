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

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Renderable.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneObject : public enable_shared_from_this<SceneObject>
	{
	public:
		enum SOAttrib
		{
			SOA_Cullable = 1UL << 0,
			SOA_Overlay = 1UL << 1,
			SOA_Moveable = 1UL << 2,
			SOA_Invisible = 1UL << 3,
			SOA_NotCastShadow = 1UL << 4
		};

	public:
		explicit SceneObject(uint32_t attrib);
		virtual ~SceneObject();

		RenderablePtr const & GetRenderable() const;

		virtual void ModelMatrix(float4x4 const & mat);
		virtual float4x4 const & ModelMatrix() const;
		virtual AABBox const & PosBound() const;
		virtual AABBox const & TexcoordBound() const;

		virtual void AddToSceneManager();
		virtual void DelFromSceneManager();

		void BindSubThreadUpdateFunc(function<void(SceneObject&, float, float)> const & update_func);
		void BindMainThreadUpdateFunc(function<void(SceneObject&, float, float)> const & update_func);

		virtual void SubThreadUpdate(float app_time, float elapsed_time);
		virtual void MainThreadUpdate(float app_time, float elapsed_time);

		uint32_t Attrib() const;
		bool Visible() const;
		void Visible(bool vis);

		vertex_elements_type const & InstanceFormat() const;
		virtual void const * InstanceData() const;

		// For deferred only
		virtual void Pass(PassType type);

		bool TransparencyBackFace() const
		{
			return renderable_->TransparencyBackFace();
		}
		bool TransparencyFrontFace() const
		{
			return renderable_->TransparencyFrontFace();
		}
		bool Reflection() const
		{
			return renderable_->Reflection();
		}
		bool SimpleForward() const
		{
			return renderable_->SimpleForward();
		}

	protected:
		uint32_t attrib_;

		RenderablePtr renderable_;
		vertex_elements_type instance_format_;

		float4x4 model_;

		function<void(SceneObject&, float, float)> sub_thread_update_func_;
		function<void(SceneObject&, float, float)> main_thread_update_func_;
	};
}

#endif		// _SCENEOBJECT_HPP
