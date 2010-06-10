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
#include <KlayGE/thread.hpp>

#include <vector>
#include <windows.h>
#include <dsound.h>

#include <boost/noncopyable.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	typedef boost::shared_ptr<IDirectSoundBuffer> IDSBufferPtr;

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
		boost::shared_ptr<IDirectSound3DBuffer> Get3DBufferInterface(std::vector<IDSBufferPtr>::iterator iter);

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

	private:
		IDSBufferPtr	buffer_;
		uint32_t		fillSize_;
		uint32_t		fillCount_;
		uint32_t		writePos_;

		boost::shared_ptr<IDirectSound3DBuffer> ds3DBuffer_;

		bool		loop_;

		bool played_;
		bool stopped_;
		boost::condition_variable play_cond_;
		boost::mutex play_mutex_;
		joiner<void> play_thread_;
	};

	// 管理音频播放
	/////////////////////////////////////////////////////////////////////////////////
	class DSAudioEngine : boost::noncopyable, public AudioEngine
	{
	public:
		DSAudioEngine();
		~DSAudioEngine();

		boost::shared_ptr<IDirectSound> const & DSound() const
			{ return dsound_; }

		std::wstring const & Name() const;

		float3 GetListenerPos() const;
		void SetListenerPos(float3 const & v);
		float3 GetListenerVel() const;
		void SetListenerVel(float3 const & v);
		void GetListenerOri(float3& face, float3& up) const;
		void SetListenerOri(float3 const & face, float3 const & up);

	private:
		boost::shared_ptr<IDirectSound>				dsound_;
		boost::shared_ptr<IDirectSound3DListener>	ds3dListener_;
	};
}

#endif		// _DS8AUDIO_HPP
