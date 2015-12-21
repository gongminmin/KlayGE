// OALMusicBuffer.cpp
// KlayGE OpenAL音乐缓冲区类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://www.klayge.org
//
// 3.2.0
// 改进了线程的使用 (2006.4.29)
//
// 2.2.0
// 修正了DoStop的死锁 (2004.10.22)
//
// 2.0.4
// 增加了循环播放功能 (2004.3.22)
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

#include <KlayGE/OpenAL/OALAudio.hpp>

size_t const READSIZE(88200);

namespace KlayGE
{
	// 构造函数。建立一个可以用于流式播放的缓冲区
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

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OALMusicBuffer::~OALMusicBuffer()
	{
		this->Stop();

		alDeleteBuffers(static_cast<ALsizei>(bufferQueue_.size()), &bufferQueue_[0]);
		alDeleteSources(1, &source_);
	}

	// 更新缓冲区
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
				Sleep(1000 / PreSecond);
			}
		}
	}

	// 缓冲区复位以便于从头播放
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
		// 每个缓冲区中装1 / PreSecond秒的数据
		for (auto const & buf : bufferQueue_)
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

	// 播放音频流
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

	// 停止播放音频流
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

	// 检查缓冲区是否在播放
	/////////////////////////////////////////////////////////////////////////////////
	bool OALMusicBuffer::IsPlaying() const
	{
		ALint value;
		alGetSourcei(source_, AL_SOURCE_STATE, &value);

		return (AL_PLAYING == (value & AL_PLAYING));
	}

	// 设置音量
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Volume(float vol)
	{
		alSourcef(source_, AL_GAIN, vol);
	}

	// 获取声源位置
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALMusicBuffer::Position() const
	{
		float pos[3];
		alGetSourcefv(source_, AL_POSITION, pos);
		return ALVecToVec(float3(pos[0], pos[1], pos[2]));
	}

	// 设置声源位置
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Position(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alSourcefv(source_, AL_POSITION, &alv.x());
	}

	// 获取声源速度
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALMusicBuffer::Velocity() const
	{
		float vel[3];
		alGetSourcefv(source_, AL_VELOCITY, vel);
		return ALVecToVec(float3(vel[0], vel[1], vel[2]));
	}

	// 设置声源速度
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Velocity(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alSourcefv(source_, AL_VELOCITY, &alv.x());
	}

	// 获取声源方向
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALMusicBuffer::Direction() const
	{
		float dir[3];
		alGetSourcefv(source_, AL_DIRECTION, dir);
		return ALVecToVec(float3(dir[0], dir[1], dir[2]));
	}

	// 设置声源方向
	/////////////////////////////////////////////////////////////////////////////////
	void OALMusicBuffer::Direction(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alSourcefv(source_, AL_DIRECTION, &alv.x());
	}
}
