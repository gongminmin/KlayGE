// OALAudio.hpp
// KlayGE OpenAL�������� ͷ�ļ�
// Ver 3.2.0
// ��Ȩ����(C) ������, 2003-2006
// Homepage: http://www.klayge.org
//
// 3.2.0
// �Ľ���OALMusicBuffer���̵߳�ʹ�� (2006.4.29)
//
// 2.0.0
// ���ν��� (2003.7.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OALAUDIO_HPP
#define _OALAUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Thread.hpp>

#ifdef KLAYGE_PLATFORM_WINDOWS
#include <al.h>
#include <alc.h>
#elif (defined KLAYGE_PLATFORM_DARWIN) || (defined KLAYGE_PLATFORM_IOS)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <vector>

#include <boost/noncopyable.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	ALenum Convert(AudioFormat format);
	float3 VecToALVec(float3 const & v);
	float3 ALVecToVec(float3 const & v);

	// ����������
	/////////////////////////////////////////////////////////////////////////////////
	class OALSoundBuffer : boost::noncopyable, public SoundBuffer
	{
	public:
		OALSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource, float volume);
		~OALSoundBuffer();

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
		void DoReset();
		std::vector<ALuint>::iterator FreeSource();

	private:
		std::vector<ALuint>	sources_;
		ALuint			buffer_;

		float3		pos_;
		float3		vel_;
		float3		dir_;
	};

	// ���ֻ�����
	/////////////////////////////////////////////////////////////////////////////////
	class OALMusicBuffer : boost::noncopyable, public MusicBuffer
	{
	public:
		OALMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds, float volume);
		~OALMusicBuffer();

		void Volume(float vol);

		bool IsPlaying() const;

		float3 Position() const;
		void Position(float3 const & v);
		float3 Velocity() const;
		void Velocity(float3 const & v);
		float3 Direction() const;
		void Direction(float3 const & v);

		void LoopUpdateBuffer();

	private:
		void DoReset();
		void DoPlay(bool loop);
		void DoStop();

	private:
		ALuint					source_;
		std::vector<ALuint>		bufferQueue_;

		bool		loop_;

		bool played_;
		bool stopped_;
		std::condition_variable play_cond_;
		std::mutex play_mutex_;
		joiner<void> play_thread_;
	};

	// ������Ƶ����
	/////////////////////////////////////////////////////////////////////////////////
	class OALAudioEngine : boost::noncopyable, public AudioEngine
	{
	public:
		OALAudioEngine();
		~OALAudioEngine();

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
	};
}

#endif		// _OALAUDIO_HPP
