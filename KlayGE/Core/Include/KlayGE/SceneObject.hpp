// SceneObject.hpp
// KlayGE 场景对象类 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2005-2009
// Homepage: http://klayge.sourceforge.net
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

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011)
#endif
#include <boost/smart_ptr.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	class KLAYGE_CORE_API SceneObject : public boost::enable_shared_from_this<SceneObject>
	{
	public:
		enum SOAttrib
		{
			SOA_Cullable = 1UL << 0,
			SOA_Overlay = 1UL << 1,
			SOA_Moveable = 1UL << 2,
			SOA_Unvisible = 1UL << 3
		};

	public:
		explicit SceneObject(uint32_t attrib);
		virtual ~SceneObject();

		RenderablePtr const & GetRenderable() const;

		virtual float4x4 const & GetModelMatrix() const;
		virtual Box const & GetBound() const;

		virtual void AddToSceneManager();

		virtual void Update();

		bool Cullable() const;
		bool Overlay() const;
		bool Moveable() const;
		bool Visible() const;
		void Visible(bool vis);

		vertex_elements_type const & InstanceFormat() const;
		virtual void const * InstanceData() const;

	protected:
		uint32_t attrib_;

		RenderablePtr renderable_;
		vertex_elements_type instance_format_;
	};
}

#endif		// _SCENEOBJECT_HPP
