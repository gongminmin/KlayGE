/**
 * @file DSMusicBuffer.hpp
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
#include <KFL/COMPtr.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <algorithm>
#include <cstring>

#include <KlayGE/DSound/DSAudio.hpp>

namespace KlayGE
{
	DSMusicBuffer::DSMusicBuffer(AudioDataSourcePtr const & data_source, uint32_t buffer_seconds, float volume)
					: MusicBuffer(data_source),
						played_(false), stopped_(true)
	{
		WAVEFORMATEX wfx = WaveFormatEx(data_source);
		fill_size_ = wfx.nAvgBytesPerSec / BUFFERS_PER_SECOND;
		fill_count_	= buffer_seconds * BUFFERS_PER_SECOND;

		bool const mono = (1 == wfx.nChannels);

		auto dsound = checked_cast<DSAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance())->DSound();

		DSBUFFERDESC dsbd{};
		dsbd.dwSize = sizeof(dsbd);
		dsbd.dwFlags = DSBCAPS_CTRLVOLUME;
		if (mono)
		{
			dsbd.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
		}
		dsbd.dwBufferBytes = fill_size_ * fill_count_;
		dsbd.lpwfxFormat = &wfx;

		IDirectSoundBuffer* buffer;
		TIFHR(dsound->CreateSoundBuffer(&dsbd, &buffer, nullptr));
		buffer_ = MakeCOMPtr(buffer);

		if (mono)
		{
			IDirectSound3DBuffer* ds_3d_buffer;
			buffer_->QueryInterface(IID_IDirectSound3DBuffer, reinterpret_cast<void**>(&ds_3d_buffer));
			ds_3d_buffer_ = MakeCOMPtr(ds_3d_buffer);
		}

		this->Position(float3::Zero());
		this->Velocity(float3::Zero());
		this->Direction(float3::Zero());

		this->Volume(volume);

		this->Reset();
	}

	DSMusicBuffer::~DSMusicBuffer()
	{
		this->Stop();
	}

	void DSMusicBuffer::LoopUpdateBuffer()
	{
		std::unique_lock<std::mutex> lock(play_mutex_);
		while (!played_)
		{
			play_cond_.wait(lock);
		}
		played_ = false;

		while (!stopped_)
		{
			DWORD play_cursor, write_cursor;
			buffer_->GetCurrentPosition(&play_cursor, &write_cursor);

			uint32_t const next_cursor = play_cursor + fill_size_;
			if (next_cursor >= write_cursor)
			{
				if (this->FillData(fill_size_))
				{
					if (loop_)
					{
						stopped_ = false;
						this->DoReset();
					}
					else
					{
						stopped_ = true;
						buffer_->Stop();
					}
				}
			}

			Sleep(1000 / BUFFERS_PER_SECOND);
		}
	}

	void DSMusicBuffer::DoReset()
	{
		data_source_->Reset();

		buffer_->SetCurrentPosition(0);
	}

	void DSMusicBuffer::DoPlay(bool loop)
	{
		play_thread_ = Context::Instance().ThreadPool()([this] { this->LoopUpdateBuffer(); });

		loop_ = loop;

		stopped_ = false;
		{
			std::lock_guard<std::mutex> lock(play_mutex_);
			played_ = true;
		}
		play_cond_.notify_one();

		buffer_->Play(0, 0, DSBPLAY_LOOPING);
	}

	void DSMusicBuffer::DoStop()
	{
		if (!stopped_)
		{
			stopped_ = true;
			play_thread_();
		}

		buffer_->Stop();
	}

	bool DSMusicBuffer::FillData(uint32_t size)
	{
		std::vector<uint8_t> data(size);
		data.resize(data_source_->Read(data.data(), size));

		uint8_t* locked_buff[2];
		DWORD locked_buff_size[2];
		TIFHR(buffer_->Lock(0, size,
			reinterpret_cast<void**>(&locked_buff[0]), &locked_buff_size[0],
			reinterpret_cast<void**>(&locked_buff[1]), &locked_buff_size[1],
			DSBLOCK_FROMWRITECURSOR));

		bool ret;
		if (data.empty())
		{
			memset(locked_buff[0], 0, locked_buff_size[0]);
			if (locked_buff[1] != nullptr)
			{
				memset(locked_buff[1], 0, locked_buff_size[1]);
			}

			ret = true;
		}
		else
		{
			memcpy(locked_buff[0], &data[0], locked_buff_size[0]);
			if (locked_buff_size[0] > data.size())
			{
				memset(locked_buff[0], 0, locked_buff_size[0] - data.size());
			}
			if (locked_buff[1] != nullptr)
			{
				memcpy(locked_buff[1], &data[locked_buff_size[0]], locked_buff_size[1]);
				if (locked_buff_size[1] > data.size() - locked_buff_size[0])
				{
					memset(locked_buff[1], 0, locked_buff_size[1] - data.size() + locked_buff_size[0]);
				}
			}

			ret = false;
		}

		buffer_->Unlock(locked_buff[0], locked_buff_size[0], locked_buff[1], locked_buff_size[1]);
		return ret;
	}

	bool DSMusicBuffer::IsPlaying() const
	{
		if (buffer_)
		{
			DWORD status;
			buffer_->GetStatus(&status);
			return ((status & DSBSTATUS_PLAYING) != 0);
		}

		return false;
	}

	void DSMusicBuffer::Volume(float vol)
	{
		buffer_->SetVolume(LinearGainToDB(vol));
	}

	float3 DSMusicBuffer::Position() const
	{
		float3 ret = float3::Zero();

		if (ds_3d_buffer_)
		{
			D3DVECTOR v;
			ds_3d_buffer_->GetPosition(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	void DSMusicBuffer::Position(float3 const & v)
	{
		if (ds_3d_buffer_)
		{
			ds_3d_buffer_->SetPosition(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	float3 DSMusicBuffer::Velocity() const
	{
		float3 ret = float3::Zero();

		if (ds_3d_buffer_)
		{
			D3DVECTOR v;
			ds_3d_buffer_->GetVelocity(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	void DSMusicBuffer::Velocity(float3 const & v)
	{
		if (ds_3d_buffer_)
		{
			ds_3d_buffer_->SetVelocity(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	float3 DSMusicBuffer::Direction() const
	{
		float3 ret = float3::Zero();

		if (ds_3d_buffer_)
		{
			D3DVECTOR v;
			ds_3d_buffer_->GetConeOrientation(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	void DSMusicBuffer::Direction(float3 const & v)
	{
		if (ds_3d_buffer_)
		{
			ds_3d_buffer_->SetConeOrientation(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}
}
