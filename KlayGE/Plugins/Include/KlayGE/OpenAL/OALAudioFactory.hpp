// OALAudioFactory.hpp
// KlayGE OpenAL声音引擎抽象工厂类 头文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 改为template实现 (2004.3.4)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OALAUDIOFACTORY_HPP
#define _OALAUDIOFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/OpenAL/OALAudio.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_AudioEngine_OpenAL_d.lib")
#else
	#pragma comment(lib, "KlayGE_AudioEngine_OpenAL.lib")
#endif

namespace KlayGE
{
	AudioFactory& OALAudioFactoryInstance();
}

#endif			// _OALAUDIOFACTORY_HPP
