// OGLESRenderFactory.h
// KlayGE OpenGL ES渲染工厂类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERFACTORY_HPP
#define _OGLESRENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_OGLES_RE_SOURCE				// Build dll
	#define KLAYGE_OGLES_RE_API KLAYGE_SYMBOL_EXPORT
#else										// Use dll
	#define KLAYGE_OGLES_RE_API KLAYGE_SYMBOL_IMPORT
#endif

extern "C"
{
	KLAYGE_OGLES_RE_API void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr);
}

#endif			// _OGLESRENDERFACTORY_HPP
