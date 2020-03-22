/**
 * @file AudioDataSource.hpp
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

#ifndef _KLAYGE_CORE_AUDIO_DATA_SOURCE_HPP
#define _KLAYGE_CORE_AUDIO_DATA_SOURCE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	enum AudioFormat
	{
		AF_Mono8,
		AF_Mono16,
		AF_Stereo8,
		AF_Stereo16,

		AF_Unknown,
	};

	class KLAYGE_CORE_API AudioDataSource : boost::noncopyable
	{
	public:
		virtual ~AudioDataSource() noexcept;

		virtual void Open(ResIdentifierPtr const & file) = 0;
		virtual void Close() = 0;

		AudioFormat Format() const;
		uint32_t Freq() const;

		virtual size_t Size() = 0;

		virtual size_t Read(void* data, size_t size) = 0;
		virtual void Reset() = 0;

	protected:
		AudioFormat format_;
		uint32_t freq_;
	};

	class KLAYGE_CORE_API AudioDataSourceFactory : boost::noncopyable
	{
	public:
		virtual ~AudioDataSourceFactory() noexcept;

		void Suspend();
		void Resume();

		virtual std::wstring const & Name() const = 0;

		virtual AudioDataSourcePtr MakeAudioDataSource() = 0;

	private:
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;
	};
}

#endif			// _KLAYGE_CORE_AUDIO_DATA_SOURCE_HPP
