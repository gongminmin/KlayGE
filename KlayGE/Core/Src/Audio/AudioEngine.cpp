// AudioEngine.cpp
// KlayGE 音频引擎类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cassert>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	class NullAudioEngine : public AudioEngine
	{
	public:
		const WString& Name() const
		{
			static WString name(L"Null Audio Engine");
			return name;
		}

		void AddBuffer(size_t id, const AudioBufferPtr& buffer)
			{ }

		size_t BufferNum() const
			{ return 0; }
		AudioBufferPtr Buffer(size_t bufID)
			{ return AudioBuffer::NullObject(); }
		const AudioBufferPtr Buffer(size_t bufID) const
			{ return AudioBuffer::NullObject(); }

		void Play(size_t bufID, bool loop = false)
			{ }
		void Stop(size_t bufID)
			{ }
		void PlayAll(bool loop = false)
			{ }
		void StopAll()
			{ }

		// 设置和获取音量
		void  SoundVolume(float vol)
			{ }
		float SoundVolume() const
			{ }
		void  MusicVolume(float vol)
			{ }
		float MusicVolume() const
			{ }

		Vector3 GetListenerPos() const
			{ return Vector3::Zero(); }
		void SetListenerPos(const Vector3& v)
			{ }
		Vector3 GetListenerVel() const
			{ return Vector3::Zero(); }
		void SetListenerVel(const Vector3& v)
			{ }
		void GetListenerOri(Vector3& face, Vector3& up) const
		{
			face = Vector3::Zero();
			up = Vector3::Zero();
		}
		void SetListenerOri(const Vector3& face, const Vector3& up)
			{ }
	};

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

	// 获取空对象
	/////////////////////////////////////////////////////////////////////////////////
	AudioEnginePtr AudioEngine::NullObject()
	{
		static AudioEnginePtr obj(new NullAudioEngine);
		return obj;
	}

	// 往列表里添加一个音频缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::AddBuffer(size_t id, const AudioBufferPtr& buffer)
	{
		audioBufs_.insert(id, buffer);
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
		for (AudioBufsIter iter = audioBufs_.begin(); iter != audioBufs_.end(); ++ iter)
		{
			audioBufs_.data(iter)->Play(loop);
		}
	}

	// 停止所有的声音
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::StopAll()
	{
		for (AudioBufsIter iter = audioBufs_.begin(); iter != audioBufs_.end(); ++ iter)
		{
			audioBufs_.data(iter)->Stop();
		}
	}

	// 列表里缓冲区的数目
	/////////////////////////////////////////////////////////////////////////////////
	size_t AudioEngine::BufferNum() const
	{
		return audioBufs_.size();
	}

	// 获取声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	AudioBufferPtr AudioEngine::Buffer(size_t bufID)
	{
		AudioBufsIter iter(audioBufs_.find(bufID));
		if (iter != audioBufs_.end())
		{
			return audioBufs_.data(iter);
		}
		THR(E_FAIL);
	}

	const AudioBufferPtr AudioEngine::Buffer(size_t bufID) const
	{
		AudioBufsConstIter iter(audioBufs_.find(bufID));
		if (iter != audioBufs_.end())
		{
			return audioBufs_.data(iter);
		}
		THR(E_FAIL);
	}

	// 设置音效音量，vol的取值范围为0--1.0f
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::SoundVolume(float vol)
	{
		soundVol_ = vol;

		for (AudioBufsIter iter = audioBufs_.begin(); iter != audioBufs_.end(); ++ iter)
		{
			if (audioBufs_.data(iter)->IsSound())
			{
				audioBufs_.data(iter)->Volume(vol);
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

		for (AudioBufsIter iter = audioBufs_.begin(); iter != audioBufs_.end(); ++ iter)
		{
			if (!(audioBufs_.data(iter)->IsSound()))
			{
				audioBufs_.data(iter)->Volume(vol);
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
