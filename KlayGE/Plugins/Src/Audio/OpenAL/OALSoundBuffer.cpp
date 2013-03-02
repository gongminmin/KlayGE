// OALSoundBuffer.cpp
// KlayGE OpenAL声音缓冲区类 实现文件
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
#include <KlayGE/AudioDataSource.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OpenAL/OALAudio.hpp>

namespace
{
	// 检查一个音频缓冲区是否空闲
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
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OALSoundBuffer::OALSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource, float volume)
						: SoundBuffer(dataSource),
							sources_(numSource)
	{
		alGenBuffers(1, &buffer_);

		// 用整个waveFile填充缓冲区
		std::vector<uint8_t> data(dataSource_->Size());
		dataSource_->Read(&data[0], data.size());

		alBufferData(buffer_, Convert(format_), &data[0], static_cast<ALsizei>(data.size()), freq_);

		alGenSources(static_cast<ALsizei>(sources_.size()), &sources_[0]);

		typedef KLAYGE_DECLTYPE(sources_) SourcesType;
		KLAYGE_FOREACH(SourcesType::reference source, sources_)
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

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OALSoundBuffer::~OALSoundBuffer()
	{
		this->Stop();

		alDeleteBuffers(1, &buffer_);
		alDeleteSources(static_cast<ALsizei>(sources_.size()), &sources_[0]);
	}

	// 返回空闲的缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	std::vector<ALuint>::iterator OALSoundBuffer::FreeSource()
	{
		BOOST_ASSERT(!sources_.empty());

		KLAYGE_AUTO(iter, std::find_if(sources_.begin(), sources_.end(), IsSourceFree));
		if (iter == sources_.end())
		{
			iter = sources_.begin();
			ranlux24_base gen;
			uniform_int_distribution<> dis(0, static_cast<int>(sources_.size()));
			std::advance(iter, dis(gen));
		}

		return iter;
	}

	// 播放音源
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

	// 停止播放
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Stop()
	{
		alSourceStopv(static_cast<ALsizei>(sources_.size()), &sources_[0]);
	}

	// 声音缓冲区复位
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::DoReset()
	{
		alSourceRewindv(static_cast<ALsizei>(sources_.size()), &sources_[0]);
	}

	// 检查缓冲区是否在播放
	/////////////////////////////////////////////////////////////////////////////////
	bool OALSoundBuffer::IsPlaying() const
	{
		return (std::find_if(sources_.begin(), sources_.end(),
			KlayGE::bind(std::logical_not<bool>(), KlayGE::bind(IsSourceFree, KlayGE::placeholders::_1))) != sources_.end());
	}

	// 设置音量
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Volume(float vol)
	{
		std::for_each(sources_.begin(), sources_.end(),
			KlayGE::bind(alSourcef, KlayGE::placeholders::_1, AL_GAIN, vol));
	}

	// 获取声源位置
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALSoundBuffer::Position() const
	{
		return ALVecToVec(pos_);
	}

	// 设置声源位置
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Position(float3 const & v)
	{
		pos_ = VecToALVec(v);
	}

	// 获取声源速度
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALSoundBuffer::Velocity() const
	{
		return ALVecToVec(vel_);
	}

	// 设置声源速度
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Velocity(float3 const & v)
	{
		vel_ = VecToALVec(v);
	}

	// 获取声源方向
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALSoundBuffer::Direction() const
	{
		return ALVecToVec(dir_);
	}

	// 设置声源方向
	/////////////////////////////////////////////////////////////////////////////////
	void OALSoundBuffer::Direction(float3 const & v)
	{
		dir_ = VecToALVec(v);
	}
}
