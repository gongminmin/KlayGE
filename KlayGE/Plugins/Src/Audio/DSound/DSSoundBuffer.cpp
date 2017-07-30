/**
 * @file DSSoundBuffer.cpp
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

#include <cstring>
#include <random>

#include <boost/assert.hpp>

#include <KlayGE/DSound/DSAudio.hpp>

namespace
{
	bool IsSourceFree(IDirectSoundBuffer* dsb)
	{
		if (dsb)
		{
			DWORD status;
			dsb->GetStatus(&status);
			return (0 == (status & DSBSTATUS_PLAYING));
		}

		return false;
	}
}

namespace KlayGE
{
	DSSoundBuffer::DSSoundBuffer(AudioDataSourcePtr const & data_source, uint32_t num_sources, float volume)
					: SoundBuffer(data_source),
						sources_(num_sources)
	{
		WAVEFORMATEX wfx = WaveFormatEx(data_source);

		DSBUFFERDESC dsbd{};
		dsbd.dwSize = sizeof(dsbd);
		dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
		dsbd.dwBufferBytes = static_cast<uint32_t>(data_source->Size());
		dsbd.lpwfxFormat = &wfx;

		auto dsound = checked_cast<DSAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance())->DSound();

		IDirectSoundBuffer* temp;
		TIFHR(dsound->CreateSoundBuffer(&dsbd, &temp, nullptr));
		sources_[0] = MakeCOMPtr(temp);

		for (auto iter = sources_.begin() + 1; iter != sources_.end(); ++ iter)
		{
			TIFHR(dsound->DuplicateSoundBuffer(sources_[0].get(), &temp));
			*iter = MakeCOMPtr(temp);
		}

		uint8_t* locked_buff;
		DWORD locked_buff_size;
		TIFHR(sources_[0]->Lock(0, static_cast<DWORD>(data_source_->Size()),
			reinterpret_cast<void**>(&locked_buff), &locked_buff_size,
			nullptr, nullptr, DSBLOCK_FROMWRITECURSOR));

		data_source_->Reset();

		std::vector<uint8_t> data(data_source_->Size());
		data_source_->Read(&data[0], data.size());

		if (data.empty())
		{
			memset(locked_buff, 128, locked_buff_size);
		}
		else if (data.size() <= locked_buff_size)
		{
			memcpy(locked_buff, data.data(), data.size());
			memset(locked_buff + data.size(), 128, locked_buff_size - data.size());
		}

		sources_[0]->Unlock(locked_buff, locked_buff_size, nullptr, 0);

		this->Position(float3(0, 0, 0));
		this->Velocity(float3(0, 0, 0));
		this->Direction(float3(0, 0, 0));

		this->Reset();

		this->Volume(volume);
	}

	DSSoundBuffer::~DSSoundBuffer()
	{
		this->Stop();
		sources_.clear();
	}

	IDirectSoundBuffer* DSSoundBuffer::FreeSource()
	{
		BOOST_ASSERT(!sources_.empty());

		for (auto const & src : sources_)
		{
			if (IsSourceFree(src.get()))
			{
				return src.get();
			}
		}

		auto iter = sources_.begin();
		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(0, static_cast<int>(sources_.size()));
		std::advance(iter, dis(gen));

		return iter->get();
	}

	std::shared_ptr<IDirectSound3DBuffer> DSSoundBuffer::Get3DBufferInterface(IDirectSoundBuffer* ds_buff)
	{
		IDirectSound3DBuffer* ds_3d_buffer;
		ds_buff->QueryInterface(IID_IDirectSound3DBuffer, reinterpret_cast<void**>(&ds_3d_buffer));
		return MakeCOMPtr(ds_3d_buffer);
	}

	void DSSoundBuffer::Play(bool loop)
	{
		auto ds_buff = this->FreeSource();
		BOOST_ASSERT(ds_buff);

		auto ds_3d_buffer = this->Get3DBufferInterface(ds_buff);
		if (ds_3d_buffer)
		{
			ds_3d_buffer->SetPosition(pos_[0], pos_[1], pos_[2], DS3D_IMMEDIATE);
			ds_3d_buffer->SetVelocity(vel_[0], vel_[1], vel_[2], DS3D_IMMEDIATE);
			ds_3d_buffer->SetConeOrientation(dir_[0], dir_[1], dir_[2], DS3D_IMMEDIATE);
		}

		ds_buff->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);
	}

	void DSSoundBuffer::Stop()
	{
		for (auto const & src : sources_)
		{
			src->Stop();
		}
	}

	void DSSoundBuffer::DoReset()
	{
		for (auto const & src : sources_)
		{
			src->SetCurrentPosition(0);
		}
	}

	bool DSSoundBuffer::IsPlaying() const
	{
		for (auto const & src : sources_)
		{
			if (!IsSourceFree(src.get()))
			{
				return true;
			}
		}
		return false;
	}

	void DSSoundBuffer::Volume(float vol)
	{
		auto const db = LinearGainToDB(vol);
		for (auto const & src : sources_)
		{
			src->SetVolume(db);
		}
	}

	float3 DSSoundBuffer::Position() const
	{
		return pos_;
	}

	void DSSoundBuffer::Position(float3 const & v)
	{
		pos_ = v;
	}

	float3 DSSoundBuffer::Velocity() const
	{
		return vel_;
	}

	void DSSoundBuffer::Velocity(float3 const & v)
	{
		vel_ = v;
	}

	float3 DSSoundBuffer::Direction() const
	{
		return dir_;
	}

	void DSSoundBuffer::Direction(float3 const & v)
	{
		dir_ = v;
	}
}
