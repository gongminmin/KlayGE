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
	class NullAudioEngine : public AudioEngine
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Audio Engine");
			return name;
		}

		void AddBuffer(size_t /*id*/, AudioBufferPtr const & /*buffer*/)
			{ }

		AudioBufferPtr Buffer(size_t /*bufID*/) const
			{ return AudioBuffer::NullObject(); }

		float3 GetListenerPos() const
			{ return float3::Zero(); }
		void SetListenerPos(float3 const & /*v*/)
			{ }
		float3 GetListenerVel() const
			{ return float3::Zero(); }
		void SetListenerVel(float3 const & /*v*/)
			{ }
		void GetListenerOri(float3& face, float3& up) const
		{
			face = float3::Zero();
			up = float3::Zero();
		}
		void SetListenerOri(float3 const & /*face*/, float3 const & /*up*/)
			{ }

	private:
		virtual void DoSuspend() KLAYGE_OVERRIDE
		{
		}
		virtual void DoResume() KLAYGE_OVERRIDE
		{
		}
	};

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
		for (AudioBufs::reference ab : audioBufs_)
		{
			ab.second->Suspend();
		}
		this->DoSuspend();
	}

	void AudioEngine::Resume()
	{
		this->DoResume();
		for (AudioBufs::reference ab : audioBufs_)
		{
			ab.second->Resume();
		}
	}

	// ��ȡ�ն���
	/////////////////////////////////////////////////////////////////////////////////
	AudioEnginePtr AudioEngine::NullObject()
	{
		static AudioEnginePtr obj = MakeSharedPtr<NullAudioEngine>();
		return obj;
	}

	// ���б������һ����Ƶ������
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::AddBuffer(size_t id, AudioBufferPtr const & buffer)
	{
		audioBufs_.insert(std::make_pair(id, buffer));
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
		for (AudioBufs::reference ab : audioBufs_)
		{
			ab.second->Play(loop);
		}
	}

	// ֹͣ���е�����
	/////////////////////////////////////////////////////////////////////////////////
	void AudioEngine::StopAll()
	{
		for (AudioBufs::reference ab : audioBufs_)
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
		AudioBufsConstIter iter(audioBufs_.find(bufID));
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

		for (AudioBufs::reference ab : audioBufs_)
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

		for (AudioBufs::reference ab : audioBufs_)
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
