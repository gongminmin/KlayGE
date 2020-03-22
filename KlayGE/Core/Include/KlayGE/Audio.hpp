/**
 * @file Audio.hpp
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

#ifndef _KLAYGE_CORE_AUDIO_HPP
#define _KLAYGE_CORE_AUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Vector.hpp>

#include <map>

#include <KlayGE/AudioDataSource.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API AudioBuffer : boost::noncopyable
	{
	public:
		explicit AudioBuffer(AudioDataSourcePtr const & data_source);
		virtual ~AudioBuffer() noexcept;

		void Suspend();
		void Resume();

		virtual void Play(bool loop = false) = 0;
		virtual void Reset() = 0;
		virtual void Stop() = 0;

		virtual void Volume(float vol) = 0;

		virtual bool IsPlaying() const = 0;
		virtual bool IsSound() const = 0;

		virtual float3 Position() const = 0;
		virtual void Position(float3 const & v) = 0;
		virtual float3 Velocity() const = 0;
		virtual void Velocity(float3 const & v) = 0;
		virtual float3 Direction() const = 0;
		virtual void Direction(float3 const & v) = 0;

	protected:
		AudioDataSourcePtr data_source_;

		AudioFormat	format_;
		uint32_t freq_;

		bool resume_playing_{false};
	};

	class KLAYGE_CORE_API SoundBuffer : public AudioBuffer
	{
	public:
		explicit SoundBuffer(AudioDataSourcePtr const & data_source);
		~SoundBuffer() noexcept override;

		void Reset() override;

		bool IsSound() const override;

	protected:
		virtual void DoReset() = 0;
	};

	class KLAYGE_CORE_API MusicBuffer : public AudioBuffer
	{
	public:
		explicit MusicBuffer(AudioDataSourcePtr const & data_source);
		~MusicBuffer() noexcept override;

		void Play(bool loop = false) override;
		void Stop() override;
		void Reset() override;

		bool IsSound() const override;

	protected:
		virtual void DoReset() = 0;
		virtual void DoPlay(bool loop) = 0;
		virtual void DoStop() = 0;

		static uint32_t constexpr BUFFERS_PER_SECOND = 2;
	};

	class KLAYGE_CORE_API AudioEngine : boost::noncopyable
	{
	public:
		AudioEngine();
		virtual ~AudioEngine() noexcept;

		void Suspend();
		void Resume();

		virtual std::wstring const & Name() const = 0;

		virtual void AddBuffer(size_t id, AudioBufferPtr const & buffer);

		size_t NumBuffer() const;
		virtual AudioBufferPtr Buffer(size_t buff_id) const;

		void Play(size_t buff_id, bool loop = false);
		void Stop(size_t buff_id);
		void PlayAll(bool loop = false);
		void StopAll();

		void  SoundVolume(float vol);
		float SoundVolume() const;
		void  MusicVolume(float vol);
		float MusicVolume() const;

		virtual float3 GetListenerPos() const = 0;
		virtual void SetListenerPos(float3 const & v) = 0;
		virtual float3 GetListenerVel() const = 0;
		virtual void SetListenerVel(float3 const & v) = 0;
		virtual void GetListenerOri(float3& face, float3& up) const = 0;
		virtual void SetListenerOri(float3 const & face, float3 const & up) = 0;

	private:
		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

	protected:
		std::map<size_t, AudioBufferPtr> audio_buffs_;

		float sound_vol_{1};
		float music_vol_{1};
	};
}

#endif		// _KLAYGE_CORE_AUDIO_HPP
