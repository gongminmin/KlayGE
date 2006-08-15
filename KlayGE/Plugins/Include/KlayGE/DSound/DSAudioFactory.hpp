// DSAudioFactory.hpp
// KlayGE DirectSound声音引擎抽象工厂类 头文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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

#define KLAYGE_LIB_NAME KlayGE_AudioEngine_DSound
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/AudioFactory.hpp>

namespace KlayGE
{
	AudioFactory& DSAudioFactoryInstance();
}

#endif			// _DSAUDIOFACTORY_HPP
