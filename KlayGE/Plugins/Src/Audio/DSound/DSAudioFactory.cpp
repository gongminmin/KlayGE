// DSAudioFactory.cpp
// KlayGE DSound音频引擎抽象工厂类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/SharePtr.hpp>
#include <KlayGE/DSound/DSAudio.hpp>

#include <KlayGE/DSound/DSAudioFactory.hpp>

namespace KlayGE
{
	// 音频工厂的名字
	/////////////////////////////////////////////////////////////////////////////////
	const WString& DSAudioFactory::Name() const
	{
		static WString name(L"DirectSound Audio Factory");
		return name;
	}

	// 获取音频引擎实例
	/////////////////////////////////////////////////////////////////////////////////
	AudioEngine& DSAudioFactory::AudioEngineInstance()
	{
		static DSAudioEngine audioEngine;
		return audioEngine;
	}

	// 建立声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	AudioBufferPtr DSAudioFactory::MakeSoundBuffer(const AudioDataSourcePtr& dataSource, U32 sourceNum)
	{
		return AudioBufferPtr(new DSSoundBuffer(dataSource, sourceNum,
			this->AudioEngineInstance().SoundVolume()));
	}

	// 建立音乐缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	AudioBufferPtr DSAudioFactory::MakeMusicBuffer(const AudioDataSourcePtr& dataSource, U32 bufferSeconds)
	{
		return AudioBufferPtr(new DSMusicBuffer(dataSource, bufferSeconds,
			this->AudioEngineInstance().MusicVolume()));
	}
}
