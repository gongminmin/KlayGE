/**
 * @file AudioEngine.cpp
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
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	AudioEngine::AudioEngine() = default;
	AudioEngine::~AudioEngine() noexcept = default;

	void AudioEngine::Suspend()
	{
		for (auto const & ab : audio_buffs_)
		{
			ab.second->Suspend();
		}
		this->DoSuspend();
	}

	void AudioEngine::Resume()
	{
		this->DoResume();
		for (auto const & ab : audio_buffs_)
		{
			ab.second->Resume();
		}
	}

	void AudioEngine::AddBuffer(size_t id, AudioBufferPtr const & buffer)
	{
		audio_buffs_.emplace(id, buffer);
	}

	void AudioEngine::Play(size_t buf_id, bool loop)
	{
		this->Buffer(buf_id)->Play(loop);
	}

	void AudioEngine::Stop(size_t buf_id)
	{
		this->Buffer(buf_id)->Stop();
	}

	void AudioEngine::PlayAll(bool loop)
	{
		for (auto const & ab : audio_buffs_)
		{
			ab.second->Play(loop);
		}
	}

	void AudioEngine::StopAll()
	{
		for (auto const & ab : audio_buffs_)
		{
			ab.second->Stop();
		}
	}

	size_t AudioEngine::NumBuffer() const
	{
		return audio_buffs_.size();
	}

	AudioBufferPtr AudioEngine::Buffer(size_t buff_id) const
	{
		auto iter = audio_buffs_.find(buff_id);
		if (iter != audio_buffs_.end())
		{
			return iter->second;
		}

		KFL_UNREACHABLE("Invalid buffer id");
	}

	void AudioEngine::SoundVolume(float vol)
	{
		sound_vol_ = vol;

		for (auto const & ab : audio_buffs_)
		{
			if (ab.second->IsSound())
			{
				ab.second->Volume(vol);
			}
		}
	}

	float AudioEngine::SoundVolume() const
	{
		return sound_vol_;
	}

	void AudioEngine::MusicVolume(float vol)
	{
		music_vol_ = vol;

		for (auto const & ab : audio_buffs_)
		{
			if (!(ab.second->IsSound()))
			{
				ab.second->Volume(vol);
			}
		}
	}

	float AudioEngine::MusicVolume() const
	{
		return music_vol_;
	}
}
