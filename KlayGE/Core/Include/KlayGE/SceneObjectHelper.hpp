// SceneObjectHelper.hpp
// KlayGE 一些常用的场景对象 头文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.1.0
// 初次建立 (2005.10.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SCENEOBJECTHELPER_HPP
#define _SCENEOBJECTHELPER_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/SceneObject.hpp>
#include <KlayGE/Box.hpp>

namespace KlayGE
{
	class SceneObjectHelper : public SceneObject
	{
	public:
		SceneObjectHelper(bool can_be_culled, bool short_age);
		SceneObjectHelper(RenderablePtr renderable, bool can_be_culled, bool short_age);
		virtual ~SceneObjectHelper()
		{
		}

		virtual bool CanBeCulled() const;
		virtual bool ShortAge() const;

	protected:
		bool can_be_culled_;
		bool short_age_;
	};

	class SceneObjectSkyBox : public SceneObjectHelper
	{
	public:
		SceneObjectSkyBox();
		virtual ~SceneObjectSkyBox()
		{
		}

		void CubeMap(TexturePtr const & cube);
	};
}

#endif		// _RENDERABLEHELPER_HPP
