// SceneObject.hpp
// KlayGE 场景对象类 头文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.1.0
// 初次建立 (2005.10.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _SCENEOBJECT_HPP
#define _SCENEOBJECT_HPP

#include <KlayGE/PreDeclare.hpp>

#include <boost/smart_ptr.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class SceneObject : public boost::enable_shared_from_this<SceneObject>
	{
	public:
		virtual ~SceneObject();

		RenderablePtr GetRenderable() const;

		virtual Matrix4 GetModelMatrix() const;
		virtual Box GetBound() const;

		virtual void AddToSceneManager();

		virtual bool CanBeCulled() const;
		virtual bool ShortAge() const;

		virtual void const * InstanceData() const;

	protected:
		RenderablePtr renderable_;
	};
}

#endif		// _SCENEOBJECT_HPP
