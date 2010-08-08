// D3D9RenderFactory.hpp
// KlayGE D3D9渲染引擎抽象工厂 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 2.4.0
// 增加了resource_pool_成员 (2005.3.3)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 2.0.3
// 改为template实现 (2004.3.4)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERFACTORY_HPP
#define _D3D9RENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_D3D9_RE_SOURCE				// Build dll
		#define KLAYGE_D3D9_RE_API __declspec(dllexport)
	#else										// Use dll
		#define KLAYGE_D3D9_RE_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_D3D9_RE_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_D3D9_RE_API void MakeRenderFactory(KlayGE::RenderFactoryPtr& ptr);
}

#endif			// _D3D9RENDERFACTORY_HPP
