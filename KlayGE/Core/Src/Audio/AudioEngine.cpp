// AudioEngine.cpp
// KlayGE ��Ƶ������ ʵ���ļ�
// Ver 2.0.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.7.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <boost/assert.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	AudioEngine::AudioEngine()
					: soundVol_(1),
						musicVol_(1)
	{
	}

	// ��������
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

	// ���б������һ����Ƶ������
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::AddBuffer(size_t id, AudioBufferPtr const & buffer)
	{
		audioBufs_.emplace(id, buffer);
	}

	// ����id��ָ��������
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::Play(size_t bufID, bool loop)
	{
		this->Buffer(bufID)->Play(loop);
	}

	// ֹͣid��ָ��������
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::Stop(size_t bufID)
	{
		this->Buffer(bufID)->Stop();
	}

	// �������е�����
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::PlayAll(bool loop)
	{
		for (auto const & ab : audioBufs_)
		{
			ab.second->Play(loop);
		}
	}

	// ֹͣ���е�����
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::StopAll()
	{
		for (auto const & ab : audioBufs_)
		{
			ab.second->Stop();
		}
	}

	// �б��ﻺ��������Ŀ
	/////////////////////////////////////////////////////////////////////////////////
	size_t AudioEngine::NumBuffer() const
	{
		return audioBufs_.size();
	}

	// ��ȡ����������
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

	// ������Ч������vol��ȡֵ��ΧΪ0--1.0f
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

	// ��ȡ��Ч����
	/////////////////////////////////////////////////////////////////////////////////
	float AudioEngine::SoundVolume() const
	{
		return soundVol_;
	}

	// ��������������vol��ȡֵ��ΧΪ0--1.0f
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

	// ��ȡ��������
	/////////////////////////////////////////////////////////////////////////////////
	float AudioEngine::MusicVolume() const
	{
		return musicVol_;
	}
}
