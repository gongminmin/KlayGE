/**
 * @file OALSoundBuffer.cpp
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
#include <KlayGE/AudioDataSource.hpp>

#include <random>

#include <boost/assert.hpp>

#include <KlayGE/OpenAL/OALAudio.hpp>

namespace
{
	bool IsSourceFree(ALuint source)
	{
		ALint value;
		alGetSourcei(source, AL_SOURCE_STATE, &value);

		return (AL_PLAYING != (value & AL_PLAYING));
	}
}

namespace KlayGE
{
	OALSoundBuffer::OALSoundBuffer(AudioDataSourcePtr const & data_source, uint32_t num_sources, float volume)
						: SoundBuffer(data_source),
							sources_(num_sources)
	{
		alGenBuffers(1, &buffer_);

		size_t const data_size = data_source_->Size();
		auto data = MakeUniquePtr<uint8_t[]>(data_size);
		data_source_->Read(data.get(), data_size);

		alBufferData(buffer_, Convert(format_), data.get(), static_cast<ALsizei>(data_size), freq_);

		alGenSources(static_cast<ALsizei>(sources_.size()), sources_.data());

		for (auto const & source : sources_)
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

	OALSoundBuffer::~OALSoundBuffer()
	{
		this->Stop();

		alDeleteBuffers(1, &buffer_);
		alDeleteSources(static_cast<ALsizei>(sources_.size()), &sources_[0]);
	}

	ALuint OALSoundBuffer::FreeSource()
	{
		BOOST_ASSERT(!sources_.empty());

		for (auto const & src : sources_)
		{
			if (IsSourceFree(src))
			{
				return src;
			}
		}

		auto iter = sources_.begin();
		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(0, static_cast<int>(sources_.size()));
		std::advance(iter, dis(gen));

		return *iter;
	}

	void OALSoundBuffer::Play(bool loop)
	{
		ALuint source = this->FreeSource();

		alSourcefv(source, AL_POSITION, &pos_[0]);
		alSourcefv(source, AL_VELOCITY, &vel_[0]);
		alSourcefv(source, AL_DIRECTION, &dir_[0]);
		alSourcei(source, AL_LOOPING, loop);

		alSourcePlay(source);
	}

	void OALSoundBuffer::Stop()
	{
		alSourceStopv(static_cast<ALsizei>(sources_.size()), sources_.data());
	}

	void OALSoundBuffer::DoReset()
	{
		alSourceRewindv(static_cast<ALsizei>(sources_.size()), sources_.data());
	}

	bool OALSoundBuffer::IsPlaying() const
	{
		for (auto const & src : sources_)
		{
			if (!IsSourceFree(src))
			{
				return true;
			}
		}
		return false;
	}

	void OALSoundBuffer::Volume(float vol)
	{
		for (auto const & src : sources_)
		{
			alSourcef(src, AL_GAIN, vol);
		}
	}

	float3 OALSoundBuffer::Position() const
	{
		return ALVecToVec(pos_);
	}

	void OALSoundBuffer::Position(float3 const & v)
	{
		pos_ = VecToALVec(v);
	}

	float3 OALSoundBuffer::Velocity() const
	{
		return ALVecToVec(vel_);
	}

	void OALSoundBuffer::Velocity(float3 const & v)
	{
		vel_ = VecToALVec(v);
	}

	float3 OALSoundBuffer::Direction() const
	{
		return ALVecToVec(dir_);
	}

	void OALSoundBuffer::Direction(float3 const & v)
	{
		dir_ = VecToALVec(v);
	}
}
