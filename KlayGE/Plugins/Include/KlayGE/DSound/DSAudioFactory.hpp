// DSAudioFactory.hpp
// KlayGE DirectSound声音引擎抽象工厂类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DSAUDIOFACTORY_HPP
#define _DSAUDIOFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/AudioFactory.hpp>

#pragma comment(lib, "KlayGE_AudioEngine_DSound.lib")

namespace KlayGE
{
	class DSAudioFactory : public AudioFactory
	{
	public:
		const WString& Name() const;

		AudioEngine& AudioEngineInstance();

		AudioBufferPtr MakeSoundBuffer(const AudioDataSourcePtr& dataSource, U32 sourceNum = 1);
		AudioBufferPtr MakeMusicBuffer(const AudioDataSourcePtr& dataSource, U32 bufferSeconds = 2);
	};
}

#endif			// _DSAUDIOFACTORY_HPP
