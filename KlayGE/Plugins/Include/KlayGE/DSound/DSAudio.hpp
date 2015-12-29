// DSAudio.hpp
// KlayGE DirectSound8声音引擎 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DSAUDIO_HPP
#define _DSAUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Thread.hpp>

#include <vector>
#include <windows.h>
#include <KlayGE/SALWrapper.hpp>
#if defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANG)
#define __null
#endif
#include <dsound.h>

#include <boost/noncopyable.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	typedef std::shared_ptr<IDirectSoundBuffer> IDSBufferPtr;

	WAVEFORMATEX WaveFormatEx(AudioDataSourcePtr const & dataSource);
	long LinearGainToDB(float vol);

	// 声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	class DSSoundBuffer : boost::noncopyable, public SoundBuffer
	{
	public:
		DSSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource, float volume);
		~DSSoundBuffer();

		void Play(bool loop = false);
		void Stop();

		void Volume(float vol);

		bool IsPlaying() const;

		float3 Position() const;
		void Position(float3 const & v);
		float3 Velocity() const;
		void Velocity(float3 const & v);
		float3 Direction() const;
		void Direction(float3 const & v);

	private:
		std::shared_ptr<IDirectSound3DBuffer> Get3DBufferInterface(std::vector<IDSBufferPtr>::iterator iter);

		void DoReset();
		std::vector<IDSBufferPtr>::iterator FreeSource();

	private:
		std::vector<IDSBufferPtr>		sources_;

		float3		pos_;
		float3		vel_;
		float3		dir_;
	};

	// 音乐缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	class DSMusicBuffer : boost::noncopyable, public MusicBuffer
	{
	public:
		DSMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds, float volume);
		~DSMusicBuffer();

		void Volume(float vol);

		bool IsPlaying() const;

		float3 Position() const;
		void Position(float3 const & v);
		float3 Velocity() const;
		void Velocity(float3 const & v);
		float3 Direction() const;
		void Direction(float3 const & v);

	private:
		void LoopUpdateBuffer();

		void DoReset();
		void DoPlay(bool loop);
		void DoStop();

		bool FillData(uint32_t size);

	private:
		IDSBufferPtr	buffer_;
		uint32_t		fillSize_;
		uint32_t		fillCount_;

		std::shared_ptr<IDirectSound3DBuffer> ds3DBuffer_;

		bool		loop_;

		bool played_;
		bool stopped_;
		std::condition_variable play_cond_;
		std::mutex play_mutex_;
		joiner<void> play_thread_;
	};

	// 管理音频播放
	/////////////////////////////////////////////////////////////////////////////////
	class DSAudioEngine : boost::noncopyable, public AudioEngine
	{
	public:
		DSAudioEngine();
		~DSAudioEngine();

		std::shared_ptr<IDirectSound> const & DSound() const
			{ return dsound_; }

		std::wstring const & Name() const;

		float3 GetListenerPos() const;
		void SetListenerPos(float3 const & v);
		float3 GetListenerVel() const;
		void SetListenerVel(float3 const & v);
		void GetListenerOri(float3& face, float3& up) const;
		void SetListenerOri(float3 const & face, float3 const & up);

	private:
		virtual void DoSuspend() override;
		virtual void DoResume() override;

	private:
		std::shared_ptr<IDirectSound>				dsound_;
		std::shared_ptr<IDirectSound3DListener>	ds3dListener_;

		HMODULE mod_dsound_;
		typedef HRESULT (WINAPI *DirectSoundCreateFunc)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
		DirectSoundCreateFunc DynamicDirectSoundCreate_;
	};
}

#endif		// _DS8AUDIO_HPP
