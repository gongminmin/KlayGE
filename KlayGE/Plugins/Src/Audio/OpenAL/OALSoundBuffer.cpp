// OALSoundBuffer.cpp
// KlayGE OpenAL������������ ʵ���ļ�
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
#include <KlayGE/AudioDataSource.hpp>

#include <random>

#include <boost/assert.hpp>

#include <KlayGE/OpenAL/OALAudio.hpp>

namespace
{
	// ���һ����Ƶ�������Ƿ����
	/////////////////////////////////////////////////////////////////////////////////
	bool IsSourceFree(ALuint source)
	{
		ALint value;
		alGetSourcei(source, AL_SOURCE_STATE, &value);

		return (AL_PLAYING != (value & AL_PLAYING));
	}
}

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	OALSoundBuffer::OALSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource, float volume)
						: SoundBuffer(dataSource),
							sources_(numSource)
	{
		alGenBuffers(1, &buffer_);

		// ������waveFile��仺����
		std::vector<uint8_t> data(dataSource_->Size());
		dataSource_->Read(&data[0], data.size());

		alBufferData(buffer_, Convert(format_), &data[0], static_cast<ALsizei>(data.size()), freq_);

		alGenSources(static_cast<ALsizei>(sources_.size()), &sources_[0]);

		typedef decltype(sources_) SourcesType;
		for (SourcesType::reference source : sources_)
		{
			alSourcef(source, AL_PITCH, 1);
			alSourcef(source, AL_GAIN, volume);
			alSourcei(source, AL_BUFFER, buffer_);
		}

		this->Position(float3(0, 0, 0.1f));
		this->Velocity(float3(0, 0, 0));
		this->Direction(float3(0, 0, 0));

		this->Reset();
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	OALSoundBuffer::~OALSoundBuffer()
	{
		this->Stop();

		alDeleteBuffers(1, &buffer_);
		alDeleteSources(static_cast<ALsizei>(sources_.size()), &sources_[0]);
	}

	// ���ؿ��еĻ�����
	/////////////////////////////////////////////////////////////////////////////////
	std::vector<ALuint>::iterator OALSoundBuffer::FreeSource()
	{
		BOOST_ASSERT(!sources_.empty());

		auto iter = std::find_if(sources_.begin(), sources_.end(), IsSourceFree);
		if (iter == sources_.end())
		{
			iter = sources_.begin();
			std::ranlux24_base gen;
			std::uniform_int_distribution<> dis(0, static_cast<int>(sources_.size()));
			std::advance(iter, dis(gen));
		}

		return iter;
	}

	// ������Դ
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Play(bool loop)
	{
		ALuint& source(*this->FreeSource());

		alSourcefv(source, AL_POSITION, &pos_[0]);
		alSourcefv(source, AL_VELOCITY, &vel_[0]);
		alSourcefv(source, AL_DIRECTION, &dir_[0]);
		alSourcei(source, AL_LOOPING, loop);

		alSourcePlay(source);
	}

	// ֹͣ����
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Stop()
	{
		alSourceStopv(static_cast<ALsizei>(sources_.size()), &sources_[0]);
	}

	// ������������λ
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::DoReset()
	{
		alSourceRewindv(static_cast<ALsizei>(sources_.size()), &sources_[0]);
	}

	// ��黺�����Ƿ��ڲ���
	/////////////////////////////////////////////////////////////////////////////////
	bool OALSoundBuffer::IsPlaying() const
	{
		return (std::find_if(sources_.begin(), sources_.end(),
			std::bind(std::logical_not<bool>(), std::bind(IsSourceFree, std::placeholders::_1))) != sources_.end());
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Volume(float vol)
	{
		std::for_each(sources_.begin(), sources_.end(),
			std::bind(alSourcef, std::placeholders::_1, AL_GAIN, vol));
	}

	// ��ȡ��Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALSoundBuffer::Position() const
	{
		return ALVecToVec(pos_);
	}

	// ������Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Position(float3 const & v)
	{
		pos_ = VecToALVec(v);
	}

	// ��ȡ��Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALSoundBuffer::Velocity() const
	{
		return ALVecToVec(vel_);
	}

	// ������Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Velocity(float3 const & v)
	{
		vel_ = VecToALVec(v);
	}

	// ��ȡ��Դ����
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALSoundBuffer::Direction() const
	{
		return ALVecToVec(dir_);
	}

	// ������Դ����
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Direction(float3 const & v)
	{
		dir_ = VecToALVec(v);
	}
}
