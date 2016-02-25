// DSAudioFactory.hpp
// KlayGE DirectSound����������󹤳��� ͷ�ļ�
// Ver 2.0.3
// ��Ȩ����(C) ������, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.0.3
// ��Ϊtemplateʵ�� (2004.3.4)
//
// 2.0.0
// ���ν��� (2003.10.4)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DSAUDIOFACTORY_HPP
#define _DSAUDIOFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_DSOUND_AE_SOURCE				// Build dll
	#define KLAYGE_DSOUND_AE_API KLAYGE_SYMBOL_EXPORT
#else										// Use dll
	#define KLAYGE_DSOUND_AE_API KLAYGE_SYMBOL_IMPORT
#endif

extern "C"
{
	KLAYGE_DSOUND_AE_API void MakeAudioFactory(std::unique_ptr<KlayGE::AudioFactory>& ptr);
}

#endif			// _DSAUDIOFACTORY_HPP
