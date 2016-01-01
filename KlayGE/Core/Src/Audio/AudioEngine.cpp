// AudioEngine.cpp
// KlayGE 音频引擎类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	AudioEngine::AudioEngine()
					: soundVol_(1),
						musicVol_(1)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	AudioEngine::~AudioEngine()
	{
	}

	void AudioEngine::Suspend()
	{
		for (auto const & ab : audioBufs_)
		{
			ab.second->Suspend();
		}
		this->DoSuspend();
	}

	void AudioEngine::Resume()
	{
		this->DoResume();
		for (auto const & ab : audioBufs_)
		{
			ab.second->Resume();
		}
	}

	// 往列表里添加一个音频缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::AddBuffer(size_t id, AudioBufferPtr const & buffer)
	{
		audioBufs_.emplace(id, buffer);
	}

	// 播放id所指定的声音
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::Play(size_t bufID, bool loop)
	{
		this->Buffer(bufID)->Play(loop);
	}

	// 停止id所指定的声音
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::Stop(size_t bufID)
	{
		this->Buffer(bufID)->Stop();
	}

	// 播放所有的声音
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::PlayAll(bool loop)
	{
		for (auto const & ab : audioBufs_)
		{
			ab.second->Play(loop);
		}
	}

	// 停止所有的声音
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::StopAll()
	{
		for (auto const & ab : audioBufs_)
		{
			ab.second->Stop();
		}
	}

	// 列表里缓冲区的数目
	/////////////////////////////////////////////////////////////////////////////////
	size_t AudioEngine::NumBuffer() const
	{
		return audioBufs_.size();
	}

	// 获取声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	AudioBufferPtr AudioEngine::Buffer(size_t bufID) const
	{
		auto iter = audioBufs_.find(bufID);
		if (iter != audioBufs_.end())
		{
			return iter->second;
		}

		BOOST_ASSERT(false);
		return AudioBufferPtr();
	}

	// 设置音效音量，vol的取值范围为0--1.0f
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::SoundVolume(float vol)
	{
		soundVol_ = vol;

		for (auto const & ab : audioBufs_)
		{
			if (ab.second->IsSound())
			{
				ab.second->Volume(vol);
			}
		}
	}

	// 获取音效音量
	/////////////////////////////////////////////////////////////////////////////////
	float AudioEngine::SoundVolume() const
	{
		return soundVol_;
	}

	// 设置音乐音量，vol的取值范围为0--1.0f
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::MusicVolume(float vol)
	{
		musicVol_ = vol;

		for (auto const & ab : audioBufs_)
		{
			if (!(ab.second->IsSound()))
			{
				ab.second->Volume(vol);
			}
		}
	}

	// 获取音乐音量
	/////////////////////////////////////////////////////////////////////////////////
	float AudioEngine::MusicVolume() const
	{
		return musicVol_;
	}
}
