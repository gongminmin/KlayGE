/**
 * @file OALAudio.hpp
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

#ifndef _KLAYGE_PLUGINS_OAL_AUDIO_HPP
#define _KLAYGE_PLUGINS_OAL_AUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Thread.hpp>

#if (defined KLAYGE_PLATFORM_DARWIN) || (defined KLAYGE_PLATFORM_IOS)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <vector>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	ALenum Convert(AudioFormat format);
	float3 VecToALVec(float3 const & v);
	float3 ALVecToVec(float3 const & v);

	class OALSoundBuffer final : public SoundBuffer
	{
	public:
		OALSoundBuffer(AudioDataSourcePtr const & data_source, uint32_t num_sources, float volume);
		~OALSoundBuffer() override;

		void Play(bool loop = false) override;
		void Stop() override;

		void Volume(float vol) override;

		bool IsPlaying() const override;

		float3 Position() const override;
		void Position(float3 const & v) override;
		float3 Velocity() const override;
		void Velocity(float3 const & v) override;
		float3 Direction() const override;
		void Direction(float3 const & v) override;

	private:
		void DoReset() override;
		ALuint FreeSource();

	private:
		std::vector<ALuint>	sources_;
		ALuint buffer_;

		float3 pos_;
		float3 vel_;
		float3 dir_;
	};

	class OALMusicBuffer final : public MusicBuffer
	{
	public:
		OALMusicBuffer(AudioDataSourcePtr const & data_source, uint32_t buffer_seconds, float volume);
		~OALMusicBuffer() override;

		void Volume(float vol) override;

		bool IsPlaying() const override;

		float3 Position() const override;
		void Position(float3 const & v) override;
		float3 Velocity() const override;
		void Velocity(float3 const & v) override;
		float3 Direction() const override;
		void Direction(float3 const & v) override;

		void LoopUpdateBuffer();

	private:
		void DoReset() override;
		void DoPlay(bool loop) override;
		void DoStop() override;

	private:
		ALuint source_;
		std::vector<ALuint> buffer_queue_;

		bool loop_;

		bool played_;
		bool stopped_;
		std::condition_variable play_cond_;
		std::mutex play_mutex_;
		joiner<void> play_thread_;
	};

	class OALAudioEngine final : public AudioEngine
	{
	public:
		OALAudioEngine();
		~OALAudioEngine() override;

		std::wstring const & Name() const override;

		float3 GetListenerPos() const override;
		void SetListenerPos(float3 const & v) override;
		float3 GetListenerVel() const override;
		void SetListenerVel(float3 const & v) override;
		void GetListenerOri(float3& face, float3& up) const override;
		void SetListenerOri(float3 const & face, float3 const & up) override;

	private:
		void DoSuspend() override;
		void DoResume() override;
	};
}

#endif		// _KLAYGE_PLUGINS_OAL_AUDIO_HPP
