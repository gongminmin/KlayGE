/**
 * @file NullMusicBuffer.hpp
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
	NullMusicBuffer::NullMusicBuffer(AudioDataSourcePtr const & data_source, uint32_t buffer_seconds, float volume)
					: MusicBuffer(data_source)
	{
		KFL_UNUSED(buffer_seconds);

		this->Position(float3::Zero());
		this->Velocity(float3::Zero());
		this->Direction(float3::Zero());

		this->Volume(volume);

		this->Reset();
	}

	NullMusicBuffer::~NullMusicBuffer()
	{
		this->Stop();
	}

	void NullMusicBuffer::DoReset()
	{
		data_source_->Reset();
	}

	void NullMusicBuffer::DoPlay(bool loop)
	{
		KFL_UNUSED(loop);
	}

	void NullMusicBuffer::DoStop()
	{
	}

	bool NullMusicBuffer::IsPlaying() const
	{
		return false;
	}

	void NullMusicBuffer::Volume(float vol)
	{
		KFL_UNUSED(vol);
	}

	float3 NullMusicBuffer::Position() const
	{
		return pos_;
	}

	void NullMusicBuffer::Position(float3 const & v)
	{
		pos_ = v;
	}

	float3 NullMusicBuffer::Velocity() const
	{
		return vel_;
	}

	void NullMusicBuffer::Velocity(float3 const & v)
	{
		vel_ = v;
	}

	float3 NullMusicBuffer::Direction() const
	{
		return dir_;
	}

	void NullMusicBuffer::Direction(float3 const & v)
	{
		dir_ = v;
	}
}
