// OALAudioFactory.cpp
// KlayGE OpenAL音频引擎抽象工厂类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/SharePtr.hpp>
#include <KlayGE/OpenAL/OALAudio.hpp>

#include <KlayGE/OpenAL/OALAudioFactory.hpp>

namespace KlayGE
{
	// 音频工厂的名字
	/////////////////////////////////////////////////////////////////////////////////
	const WString& OALAudioFactory::Name() const
	{
		static WString name(L"OpenAL Audio Factory");
		return name;
	}

	// 获取音频引擎实例
	/////////////////////////////////////////////////////////////////////////////////
	AudioEngine& OALAudioFactory::AudioEngineInstance()
	{
		static OALAudioEngine audioEngine;
		return audioEngine;
	}

	// 建立声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	AudioBufferPtr OALAudioFactory::MakeSoundBuffer(const AudioDataSourcePtr& dataSource, U32 sourceNum)
	{
		return AudioBufferPtr(new OALSoundBuffer(dataSource, sourceNum,
			this->AudioEngineInstance().SoundVolume()));
	}

	// 建立音乐缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	AudioBufferPtr OALAudioFactory::MakeMusicBuffer(const AudioDataSourcePtr& dataSource, U32 bufferSeconds)
	{
		return AudioBufferPtr(new OALMusicBuffer(dataSource, bufferSeconds,
			this->AudioEngineInstance().MusicVolume()));
	}
}
