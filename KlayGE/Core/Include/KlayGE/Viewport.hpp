// Viewport.hpp
// KlayGE 渲染视口类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.0.0
// camera改为指针 (2005.8.18)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _VIEWPORT_HPP
#define _VIEWPORT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	struct KLAYGE_CORE_API Viewport : boost::noncopyable
	{
		Viewport();
		Viewport(int _left, int _top, int _width, int _height);

		int left;
		int top;
		int width;
		int height;

		CameraPtr camera;
	};
}

#endif			// _VIEWPORT_HPP
