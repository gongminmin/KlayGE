// OCTreeFactory.hpp
// KlayGE OCTree场景管理类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.10.17)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OCTREEFACTORY_HPP
#define _OCTREEFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_OCTREE_SM_SOURCE				// Build dll
	#define KLAYGE_OCTREE_SM_API KLAYGE_SYMBOL_EXPORT
#else										// Use dll
	#define KLAYGE_OCTREE_SM_API KLAYGE_SYMBOL_IMPORT
#endif

extern "C"
{
	KLAYGE_OCTREE_SM_API void MakeSceneManager(std::unique_ptr<KlayGE::SceneManager>& ptr);
}

#endif			// _OCTREEFACTORY_HPP
