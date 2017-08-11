/**
 * @file NullSoundBuffer.cpp
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

#include <KlayGE/NullAudio/NullAudio.hpp>

namespace KlayGE
{
	NullSoundBuffer::NullSoundBuffer(AudioDataSourcePtr const & data_source, uint32_t num_sources, float volume)
					: SoundBuffer(data_source)
	{
		KFL_UNUSED(num_sources);

		this->Position(float3(0, 0, 0));
		this->Velocity(float3(0, 0, 0));
		this->Direction(float3(0, 0, 0));

		this->Reset();

		this->Volume(volume);
	}

	NullSoundBuffer::~NullSoundBuffer()
	{
		this->Stop();
	}

	void NullSoundBuffer::Play(bool loop)
	{
		KFL_UNUSED(loop);
	}

	void NullSoundBuffer::Stop()
	{
	}

	void NullSoundBuffer::DoReset()
	{
	}

	bool NullSoundBuffer::IsPlaying() const
	{
		return false;
	}

	void NullSoundBuffer::Volume(float vol)
	{
		KFL_UNUSED(vol);
	}

	float3 NullSoundBuffer::Position() const
	{
		return pos_;
	}

	void NullSoundBuffer::Position(float3 const & v)
	{
		pos_ = v;
	}

	float3 NullSoundBuffer::Velocity() const
	{
		return vel_;
	}

	void NullSoundBuffer::Velocity(float3 const & v)
	{
		vel_ = v;
	}

	float3 NullSoundBuffer::Direction() const
	{
		return dir_;
	}

	void NullSoundBuffer::Direction(float3 const & v)
	{
		dir_ = v;
	}
}
