// OALAudioFactory.hpp
// KlayGE OpenAL声音引擎抽象工厂类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
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

#pragma comment(lib, "KlayGE_AudioEngine_OpenAL.lib")

namespace KlayGE
{
	class OALAudioFactory : public AudioFactory
	{
	public:
		const WString& Name() const;

		AudioEngine& AudioEngineInstance();

		AudioBufferPtr MakeSoundBuffer(const AudioDataSourcePtr& dataSource, U32 sourceNum = 1);
		AudioBufferPtr MakeMusicBuffer(const AudioDataSourcePtr& dataSource, U32 bufferSeconds = 2);
	};
}

#endif			// _OALAUDIOFACTORY_HPP