// DSAudio.hpp
// KlayGE DirectSound8声音引擎 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _DSAUDIO_HPP
#define _DSAUDIO_HPP

#include <KlayGE/PreDeclare.hpp>

#include <vector>
#define NOMINMAX
#include <windows.h>
#include <dsound.h>

#include <boost/utility.hpp>

#include <boost/smart_ptr.hpp>
#include <KlayGE/Audio.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_AudioEngine_DSound_d.lib")
#else
	#pragma comment(lib, "KlayGE_AudioEngine_DSound.lib")
#endif

namespace KlayGE
{
	typedef boost::shared_ptr<IDirectSoundBuffer> DSBufferType;

	WAVEFORMATEX WaveFormatEx(AudioDataSourcePtr const & dataSource);
	long LinearGainToDB(float vol);

	// 声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	class DSSoundBuffer : boost::noncopyable, public SoundBuffer
	{
	private:
		typedef std::vector<DSBufferType>	Sources;
		typedef Sources::iterator			SourcesIter;
		typedef Sources::const_iterator		SourcesConstIter;

	public:
		DSSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource, float volume);
		~DSSoundBuffer();

		void Play(bool loop = false);
		void Stop();

		void Volume(float vol);

		bool IsPlaying() const;

		Vector3 Position() const;
		void Position(Vector3 const & v);
		Vector3 Velocity() const;
		void Velocity(Vector3 const & v);
		Vector3 Direction() const;
		void Direction(Vector3 const & v);

	private:
		boost::shared_ptr<IDirectSound3DBuffer> Get3DBufferInterface(SourcesIter iter);

		void DoReset();
		SourcesIter FreeSource();

	private:
		Sources		sources_;

		Vector3		pos_;
		Vector3		vel_;
		Vector3		dir_;
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

		Vector3 Position() const;
		void Position(Vector3 const & v);
		Vector3 Velocity() const;
		void Velocity(Vector3 const & v);
		Vector3 Direction() const;
		void Direction(Vector3 const & v);

	private:
		void LoopUpdateBuffer();

		void DoReset();
		void DoPlay(bool loop);
		void DoStop();

	private:
		DSBufferType	buffer_;
		uint32_t		fillSize_;
		uint32_t		fillCount_;
		uint32_t		writePos_;

		boost::shared_ptr<IDirectSound3DBuffer> ds3DBuffer_;

		static void WINAPI TimerProc(UINT uTimerID, UINT uMsg,
			DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
		UINT timerID_;

		void FillBuffer();
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

		Vector3 GetListenerPos() const;
		void SetListenerPos(Vector3 const & v);
		Vector3 GetListenerVel() const;
		void SetListenerVel(Vector3 const & v);
		void GetListenerOri(Vector3& face, Vector3& up) const;
		void SetListenerOri(Vector3 const & face, Vector3 const & up);

	private:
		boost::shared_ptr<IDirectSound>				dsound_;
		boost::shared_ptr<IDirectSound3DListener>	ds3dListener_;
	};
}

#endif		// _DS8AUDIO_HPP
