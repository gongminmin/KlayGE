// DShowFactory.hpp
// KlayGE DirectShow����������󹤳� ͷ�ļ�
// Ver 3.4.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// ���ν��� (2006.7.15)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DSHOWFACTORY_HPP
#define _DSHOWFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_DSHOW_SE_SOURCE				// Build dll
	#define KLAYGE_DSHOW_SE_API KLAYGE_SYMBOL_EXPORT
#else										// Use dll
	#define KLAYGE_DSHOW_SE_API KLAYGE_SYMBOL_IMPORT
#endif

extern "C"
{
	KLAYGE_DSHOW_SE_API void MakeShowFactory(std::unique_ptr<KlayGE::ShowFactory>& ptr);
}

#endif			// _DSHOWFACTORY_HPP
