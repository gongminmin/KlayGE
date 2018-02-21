/**
 * @file OALMusicBuffer.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <KlayGE/OpenAL/OALAudio.hpp>

size_t constexpr READ_SIZE = 88200;

namespace KlayGE
{
	OALMusicBuffer::OALMusicBuffer(AudioDataSourcePtr const & data_source, uint32_t buffer_seconds, float volume)
							: MusicBuffer(data_source),
								buffer_queue_(buffer_seconds * BUFFERS_PER_SECOND),
								played_(false), stopped_(true)
	{
		alGenBuffers(static_cast<ALsizei>(buffer_queue_.size()), buffer_queue_.data());

		alGenSources(1, &source_);
		alSourcef(source_, AL_PITCH, 1);

		this->Position(float3(0, 0, 0.1f));
		this->Velocity(float3(0, 0, 0));
		this->Direction(float3(0, 0, 0));

		this->Volume(volume);

		this->Reset();
	}

	OALMusicBuffer::~OALMusicBuffer()
	{
		this->Stop();

		alDeleteBuffers(static_cast<ALsizei>(buffer_queue_.size()), buffer_queue_.data());
		alDeleteSources(1, &source_);
	}

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

					std::vector<uint8_t> data(READ_SIZE);
					data.resize(data_source_->Read(data.data(), data.size()));
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
						alBufferData(buf, Convert(format_), data.data(), static_cast<ALsizei>(data.size()), freq_);
						alSourceQueueBuffers(source_, 1, &buf);
					}
				}
			}
			else
			{
				Sleep(1000 / BUFFERS_PER_SECOND);
			}
		}
	}

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
		std::vector<uint8_t> data(READ_SIZE);

		data_source_->Reset();

		ALsizei non_empty_buf = 0;
		// Load 1 / BUFFERS_PER_SECOND second data to each buffer
		for (auto const & buf : buffer_queue_)
		{
			data.resize(data_source_->Read(data.data(), data.size()));
			if (data.empty())
			{
				break;
			}
			else
			{
				++ non_empty_buf;
				alBufferData(buf, format, data.data(),
					static_cast<ALuint>(data.size()), static_cast<ALuint>(freq_));
			}
		}

		alSourceQueueBuffers(source_, non_empty_buf, buffer_queue_.data());

		alSourceRewindv(1, &source_);
	}

	void OALMusicBuffer::DoPlay(bool loop)
	{
		play_thread_ = Context::Instance().ThreadPool()([this] { this->LoopUpdateBuffer(); });

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

	void OALMusicBuffer::DoStop()
	{
		if (!stopped_)
		{
			stopped_ = true;
			play_thread_();
		}

		alSourceStopv(1, &source_);
	}

	bool OALMusicBuffer::IsPlaying() const
	{
		ALint value;
		alGetSourcei(source_, AL_SOURCE_STATE, &value);

		return (AL_PLAYING == (value & AL_PLAYING));
	}

	void OALMusicBuffer::Volume(float vol)
	{
		alSourcef(source_, AL_GAIN, vol);
	}

	float3 OALMusicBuffer::Position() const
	{
		float pos[3];
		alGetSourcefv(source_, AL_POSITION, pos);
		return ALVecToVec(float3(pos[0], pos[1], pos[2]));
	}

	void OALMusicBuffer::Position(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alSourcefv(source_, AL_POSITION, &alv[0]);
	}

	float3 OALMusicBuffer::Velocity() const
	{
		float vel[3];
		alGetSourcefv(source_, AL_VELOCITY, vel);
		return ALVecToVec(float3(vel[0], vel[1], vel[2]));
	}

	void OALMusicBuffer::Velocity(float3 const & v)
	{
		float3 alv = VecToALVec(v);
		alSourcefv(source_, AL_VELOCITY, &alv[0]);
	}

	float3 OALMusicBuffer::Direction() const
	{
		float dir[3];
		alGetSourcefv(source_, AL_DIRECTION, dir);
		return ALVecToVec(float3(dir[0], dir[1], dir[2]));
	}

	void OALMusicBuffer::Direction(float3 const & v)
	{
		float3 alv = VecToALVec(v);
		alSourcefv(source_, AL_DIRECTION, &alv[0]);
	}
}
