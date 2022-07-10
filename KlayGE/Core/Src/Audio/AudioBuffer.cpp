/**
 * @file AudioBuffer.cpp
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

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	AudioBuffer::AudioBuffer(AudioDataSourcePtr const & data_source)
		: data_source_(data_source),
			format_(data_source_->Format()),
			freq_(data_source_->Freq())
	{
	}

	AudioBuffer::~AudioBuffer() noexcept = default;

	void AudioBuffer::Suspend()
	{
		if (this->IsPlaying())
		{
			resume_playing_ = true;
			this->Stop();
		}
	}

	void AudioBuffer::Resume()
	{
		if (resume_playing_)
		{
			this->Play();
			resume_playing_ = false;
		}
	}
}
