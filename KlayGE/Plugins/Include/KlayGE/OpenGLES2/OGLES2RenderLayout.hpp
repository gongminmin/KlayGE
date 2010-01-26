// OGLES2RenderLayout.hpp
// KlayGE OpenGL ES 2渲染分布类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2RENDERLAYOUT_HPP
#define _OGLES2RENDERLAYOUT_HPP

#pragma once

#include <KlayGE/RenderLayout.hpp>

namespace KlayGE
{
	class OGLES2RenderLayout : public RenderLayout
	{
	public:
		OGLES2RenderLayout();
		~OGLES2RenderLayout();

		void Active(ShaderObjectPtr const & so) const;
		void Deactive(ShaderObjectPtr const & so) const;
	};
}

#endif			// _OGLES2RENDERLAYOUT_HPP
