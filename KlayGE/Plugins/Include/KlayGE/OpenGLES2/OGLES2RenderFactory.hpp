// OGLES2RenderFactory.h
// KlayGE OpenGL ES 2渲染工厂类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLES2RENDERFACTORY_HPP
#define _OGLES2RENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_OGL_RE_SOURCE					// Build dll
		#define KLAYGE_OGLES2_RE_API __declspec(dllexport)
	#else										// Use dll
		#define KLAYGE_OGLES2_RE_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_OGLES2_RE_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_OGLES2_RE_API void MakeRenderFactory(KlayGE::RenderFactoryPtr& ptr, KlayGE::XMLNodePtr const & extra_param);
}

#endif			// _OGLRENDERFACTORY_HPP
