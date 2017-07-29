/**
 * @file DSAudio.hpp
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

#ifndef _KLAYGE_PLUGINS_DS_AUDIO_HPP
#define _KLAYGE_PLUGINS_DS_AUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Thread.hpp>

#include <vector>
#include <windows.h>
#include <KlayGE/SALWrapper.hpp>
#if (defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)) && !defined(KLAYGE_COMPILER_CLANGC2)
#define __null
#endif
#include <dsound.h>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	typedef std::shared_ptr<IDirectSoundBuffer> IDSBufferPtr;

	WAVEFORMATEX WaveFormatEx(AudioDataSourcePtr const & data_source);

	// Convert linear 0 to 1 volumn to dB
	long LinearGainToDB(float vol);

	class DSSoundBuffer : public SoundBuffer
	{
	public:
		DSSoundBuffer(AudioDataSourcePtr const & data_source, uint32_t num_sources, float volume);
		~DSSoundBuffer() override;

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
		std::shared_ptr<IDirectSound3DBuffer> Get3DBufferInterface(IDirectSoundBuffer* ds_buff);

		void DoReset() override;
		IDirectSoundBuffer* FreeSource();

	private:
		std::vector<IDSBufferPtr> sources_;

		float3 pos_;
		float3 vel_;
		float3 dir_;
	};

	class DSMusicBuffer : public MusicBuffer
	{
	public:
		DSMusicBuffer(AudioDataSourcePtr const & data_source, uint32_t buffer_seconds, float volume);
		~DSMusicBuffer() override;

		void Volume(float vol) override;

		bool IsPlaying() const override;

		float3 Position() const override;
		void Position(float3 const & v) override;
		float3 Velocity() const override;
		void Velocity(float3 const & v) override;
		float3 Direction() const override;
		void Direction(float3 const & v) override;

	private:
		void LoopUpdateBuffer();

		void DoReset() override;
		void DoPlay(bool loop) override;
		void DoStop() override;

		bool FillData(uint32_t size);

	private:
		IDSBufferPtr buffer_;
		uint32_t fill_size_;
		uint32_t fill_count_;

		std::shared_ptr<IDirectSound3DBuffer> ds_3d_buffer_;

		bool loop_;

		bool played_;
		bool stopped_;
		std::condition_variable play_cond_;
		std::mutex play_mutex_;
		joiner<void> play_thread_;
	};

	class DSAudioEngine : public AudioEngine
	{
	public:
		DSAudioEngine();
		~DSAudioEngine() override;

		IDirectSound* DSound() const
		{
			return dsound_.get();
		}

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
		std::shared_ptr<IDirectSound> dsound_;
		std::shared_ptr<IDirectSound3DListener> ds_3d_listener_;

		HMODULE mod_dsound_;
		typedef HRESULT (WINAPI *DirectSoundCreateFunc)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
		DirectSoundCreateFunc DynamicDirectSoundCreate_;
	};
}

#endif		// _DS8AUDIO_HPP
