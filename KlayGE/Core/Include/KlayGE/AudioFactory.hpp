/**
 * @file AudioFactory.hpp
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

#ifndef _KLAYGE_CORE_AUDIO_FACTORY_HPP
#define _KLAYGE_CORE_AUDIO_FACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API AudioFactory : boost::noncopyable
	{
	public:
		virtual ~AudioFactory() noexcept;

		virtual std::wstring const & Name() const = 0;

		AudioEngine& AudioEngineInstance();

		void Suspend();
		void Resume();

		virtual AudioBufferPtr MakeSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource = 1) = 0;
		virtual AudioBufferPtr MakeMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds = 2) = 0;

	private:
		virtual std::unique_ptr<AudioEngine> MakeAudioEngine() = 0;
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

	private:
		std::unique_ptr<AudioEngine> ae_;
	};

	template <typename AudioEngineType, typename SoundBufferType, typename MusicBufferType>
	class ConcreteAudioFactory : public AudioFactory
	{
	public:
		explicit ConcreteAudioFactory(std::wstring const & name)
			: name_(name)
		{
		}

		std::wstring const & Name() const override
		{
			return name_;
		}

		AudioBufferPtr MakeSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource = 1) override
		{
			return MakeSharedPtr<SoundBufferType>(dataSource, numSource,
				this->AudioEngineInstance().SoundVolume());
		}

		AudioBufferPtr MakeMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds = 2) override
		{
			return MakeSharedPtr<MusicBufferType>(dataSource, bufferSeconds,
				this->AudioEngineInstance().MusicVolume());
		}

	private:
		std::unique_ptr<AudioEngine> MakeAudioEngine() override
		{
			return MakeUniquePtr<AudioEngineType>();
		}

		virtual void DoSuspend() override
		{
		}
		virtual void DoResume() override
		{
		}

	private:
		std::wstring const name_;
	};
}

#endif			// _KLAYGE_CORE_AUDIO_FACTORY_HPP
