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
#include <windows.h>
#include <dsound.h>

#include <KlayGE/COMPtr.hpp>
#include <KlayGE/alloc.hpp>
#include <KlayGE/Audio.hpp>

#pragma comment(lib, "KlayGE_AudioEngine_DSound.lib")

namespace KlayGE
{
	class DSAudioFactory;

	typedef COMPtr<IDirectSoundBuffer> DSBufferType;

	WAVEFORMATEX WaveFormatEx(const AudioDataSourcePtr& dataSource);
	long LinearGainToDB(float vol);

	// 声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	class DSSoundBuffer : public SoundBuffer
	{
		friend class DSAudioFactory;

	private:
		typedef std::vector<DSBufferType, alloc<DSBufferType> >	Sources;
		typedef Sources::iterator					SourcesIter;
		typedef Sources::const_iterator				SourcesConstIter;

	public:
		void Play(bool loop = false);
		void Stop();

		void Volume(float vol);

		bool IsPlaying() const;

		Vector3 Position() const;
		void Position(const Vector3& v);
		Vector3 Velocity() const;
		void Velocity(const Vector3& v);
		Vector3 Direction() const;
		void Direction(const Vector3& v);

	public:
		~DSSoundBuffer();

	private:
		DSSoundBuffer(const AudioDataSourcePtr& dataSource, U32 sourceNum, float volume);

		COMPtr<IDirectSound3DBuffer> Get3DBufferInterface(SourcesIter iter);

		void DoReset();
		SourcesIter FreeSource();

	private:
		Sources		sources_;

		Vector3		pos_;
		Vector3		vel_;
		Vector3		dir_;

	private:
		DSSoundBuffer(const DSSoundBuffer&);
		DSSoundBuffer& operator=(const DSSoundBuffer&);
	};

	// 音乐缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	class DSMusicBuffer : public MusicBuffer
	{
		friend class DSAudioFactory;

	public:
		void Volume(float vol);

		bool IsPlaying() const;

		Vector3 Position() const;
		void Position(const Vector3& v);
		Vector3 Velocity() const;
		void Velocity(const Vector3& v);
		Vector3 Direction() const;
		void Direction(const Vector3& v);

	public:
		virtual ~DSMusicBuffer();

	private:
		DSMusicBuffer(const AudioDataSourcePtr& dataSource, U32 bufferSeconds, float volume);

		virtual void LoopUpdateBuffer();

		virtual void DoReset();
		virtual void DoPlay(bool loop);
		virtual void DoStop();

	private:
		DSBufferType	buffer_;
		U32				notifySize_;
		U32				notifyCount_;
		U32				writePos_;

		COMPtr<IDirectSound3DBuffer> ds3DBuffer_;

		HANDLE			notifyEvent_;

	private:
		DSMusicBuffer(const DSMusicBuffer&);
		DSMusicBuffer& operator=(const DSMusicBuffer&);
	};

	// 管理音频播放
	/////////////////////////////////////////////////////////////////////////////////
	class DSAudioEngine : public AudioEngine
	{
		friend class DSAudioFactory;

	public:
		const COMPtr<IDirectSound>& DSound() const
			{ return dsound_; }

		const WString& Name() const;

		Vector3 ListenerPos() const;
		void ListenerPos(const Vector3& v);
		Vector3 ListenerVel() const;
		void ListenerVel(const Vector3& v);
		void ListenerOri(Vector3& face, Vector3& up) const;
		void ListenerOri(const Vector3& face, const Vector3& up);

	private:
		DSAudioEngine();
		~DSAudioEngine();

	private:
		COMPtr<IDirectSound>			dsound_;
		COMPtr<IDirectSound3DListener>	ds3dListener_;

	private:
		DSAudioEngine(const DSAudioEngine&);
		DSAudioEngine& operator=(const DSAudioEngine&);
	};
}

#endif		// _DS8AUDIO_HPP
