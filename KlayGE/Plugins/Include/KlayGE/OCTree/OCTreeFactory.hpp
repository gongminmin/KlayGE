// OCTreeFactory.hpp
// KlayGE OCTree���������� ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2008.10.17)
//
// �޸ļ�¼
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
