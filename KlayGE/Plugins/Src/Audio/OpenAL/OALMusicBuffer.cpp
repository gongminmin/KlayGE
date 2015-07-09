// OALMusicBuffer.cpp
// KlayGE OpenAL���ֻ������� ʵ���ļ�
// Ver 3.2.0
// ��Ȩ����(C) ������, 2003-2006
// Homepage: http://www.klayge.org
//
// 3.2.0
// �Ľ����̵߳�ʹ�� (2006.4.29)
//
// 2.2.0
// ������DoStop������ (2004.10.22)
//
// 2.0.4
// ������ѭ�����Ź��� (2004.3.22)
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

#include <KlayGE/OpenAL/OALAudio.hpp>

size_t const READSIZE(88200);

namespace KlayGE
{
	// ���캯��������һ������������ʽ���ŵĻ�����
	/////////////////////////////////////////////////////////////////////////////////
	OALMusicBuffer::OALMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds, float volume)
							: MusicBuffer(dataSource),
								bufferQueue_(bufferSeconds * PreSecond),
								played_(false), stopped_(true)
	{
		alGenBuffers(static_cast<ALsizei>(bufferQueue_.size()), &bufferQueue_[0]);

		alGenSources(1, &source_);
		alSourcef(source_, AL_PITCH, 1);

		this->Position(float3(0, 0, 0.1f));
		this->Velocity(float3(0, 0, 0));
		this->Direction(float3(0, 0, 0));

		this->Volume(volume);

		this->Reset();
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	OALMusicBuffer::~OALMusicBuffer()
	{
		this->Stop();

		alDeleteBuffers(static_cast<ALsizei>(bufferQueue_.size()), &bufferQueue_[0]);
		alDeleteSources(1, &source_);
	}

	// ���»�����
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::LoopUpdateBuffer()
	{
		std::unique_lock<std::mutex> lock(play_mutex_);
		while (!played_)
		{
			play_cond_.wait(lock);
		}
		played_ = false;

		while (!stopped_)
		{
			ALint processed;
			alGetSourcei(source_, AL_BUFFERS_PROCESSED, &processed);
			if (processed > 0)
			{
				while (processed > 0)
				{
					-- processed;

					ALuint buf;
					alSourceUnqueueBuffers(source_, 1, &buf);

					std::vector<uint8_t> data(READSIZE);
					data.resize(dataSource_->Read(&data[0], data.size()));
					if (data.empty())
					{
						if (loop_)
						{
							stopped_ = false;
							alSourceStopv(1, &source_);
							this->DoReset();
							alSourcePlay(source_);
						}
						else
						{
							stopped_ = true;
						}
					}
					else
					{
						alBufferData(buf, Convert(format_), &data[0],
							static_cast<ALsizei>(data.size()), freq_);
						alSourceQueueBuffers(source_, 1, &buf);
					}
				}
			}
			else
			{
				Sleep(500 / PreSecond);
			}
		}
	}

	// ��������λ�Ա��ڴ�ͷ����
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::DoReset()
	{
		ALint queued_;
		alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued_);
		if (queued_ > 0)
		{
			std::vector<ALuint> cur_queue(queued_);
			alSourceUnqueueBuffers(source_, queued_, &cur_queue[0]);
		}

		ALenum const format(Convert(format_));
		std::vector<uint8_t> data(READSIZE);

		dataSource_->Reset();

		ALsizei non_empty_buf = 0;
		// ÿ����������װ1 / PreSecond�������
		typedef decltype(bufferQueue_) BufferQueueType;
		for (BufferQueueType::reference buf : bufferQueue_)
		{
			data.resize(dataSource_->Read(&data[0], data.size()));
			if (data.empty())
			{
				break;
			}
			else
			{
				++ non_empty_buf;
				alBufferData(buf, format, &data[0],
					static_cast<ALuint>(data.size()), static_cast<ALuint>(freq_));
			}
		}

		alSourceQueueBuffers(source_, non_empty_buf, &bufferQueue_[0]);

		alSourceRewindv(1, &source_);
	}

	// ������Ƶ��
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::DoPlay(bool loop)
	{
		play_thread_ = Context::Instance().ThreadPool()(std::bind(&OALMusicBuffer::LoopUpdateBuffer, this));

		loop_ = loop;

		stopped_ = false;
		{
			std::lock_guard<std::mutex> lock(play_mutex_);
			played_ = true;
		}
		play_cond_.notify_one();

		alSourcei(source_, AL_LOOPING, false);
		alSourcePlay(source_);
	}

	// ֹͣ������Ƶ��
	////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::DoStop()
	{
		if (!stopped_)
		{
			stopped_ = true;
			play_thread_();
		}

		alSourceStopv(1, &source_);
	}

	// ��黺�����Ƿ��ڲ���
	/////////////////////////////////////////////////////////////////////////////////
	bool OALMusicBuffer::IsPlaying() const
	{
		ALint value;
		alGetSourcei(source_, AL_SOURCE_STATE, &value);

		return (AL_PLAYING == (value & AL_PLAYING));
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Volume(float vol)
	{
		alSourcef(source_, AL_GAIN, vol);
	}

	// ��ȡ��Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALMusicBuffer::Position() const
	{
		float pos[3];
		alGetSourcefv(source_, AL_POSITION, pos);
		return ALVecToVec(float3(pos[0], pos[1], pos[2]));
	}

	// ������Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Position(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alSourcefv(source_, AL_POSITION, &alv.x());
	}

	// ��ȡ��Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALMusicBuffer::Velocity() const
	{
		float vel[3];
		alGetSourcefv(source_, AL_VELOCITY, vel);
		return ALVecToVec(float3(vel[0], vel[1], vel[2]));
	}

	// ������Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Velocity(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alSourcefv(source_, AL_VELOCITY, &alv.x());
	}

	// ��ȡ��Դ����
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALMusicBuffer::Direction() const
	{
		float dir[3];
		alGetSourcefv(source_, AL_DIRECTION, dir);
		return ALVecToVec(float3(dir[0], dir[1], dir[2]));
	}

	// ������Դ����
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Direction(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alSourcefv(source_, AL_DIRECTION, &alv.x());
	}
}
