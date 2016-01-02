// DSAudioFactory.hpp
// KlayGE DirectSound声音引擎抽象工厂类 头文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.0.3
// 改为template实现 (2004.3.4)
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
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
