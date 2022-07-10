/**
 * @file NullAudio.hpp
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

#ifndef KLAYGE_PLUGINS_NULL_AUDIO_HPP
#define KLAYGE_PLUGINS_NULL_AUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	class NullSoundBuffer final : public SoundBuffer
	{
	public:
		NullSoundBuffer(AudioDataSourcePtr const & data_source, uint32_t num_sources, float volume);
		~NullSoundBuffer() override;

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

	private:
		float3 pos_;
		float3 vel_;
		float3 dir_;
	};

	class NullMusicBuffer final : public MusicBuffer
	{
	public:
		NullMusicBuffer(AudioDataSourcePtr const & data_source, uint32_t buffer_seconds, float volume);
		~NullMusicBuffer() override;

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
		void DoPlay(bool loop) override;
		void DoStop() override;

	private:
		float3 pos_;
		float3 vel_;
		float3 dir_;
	};

	class NullAudioEngine final : public AudioEngine
	{
	public:
		NullAudioEngine();

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

	private:
		float3 pos_;
		float3 vel_;
		float3 face_;
		float3 up_;
	};
}

#endif		// KLAYGE_PLUGINS_NULL_AUDIO_HPP
