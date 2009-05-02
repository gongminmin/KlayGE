// D3D10RenderFactory.hpp
// KlayGE D3D10渲染引擎抽象工厂 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10RENDERFACTORY_HPP
#define _D3D10RENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_D3D10_RE_SOURCE				// Build dll
		#define KLAYGE_D3D10_RE_API __declspec(dllexport)
	#else										// Use dll
		#define KLAYGE_D3D10_RE_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_D3D10_RE_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_D3D10_RE_API void MakeRenderFactory(KlayGE::RenderFactoryPtr& ptr, KlayGE::XMLNodePtr const & extra_param);
}

#endif			// _D3D10RENDERFACTORY_HPP
