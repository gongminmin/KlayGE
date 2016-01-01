// Audio.hpp
// KlayGE 声音引擎 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.0.4
// 增加了NullObject (2004.4.7)
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _AUDIO_HPP
#define _AUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Vector.hpp>

#include <map>

#include <KlayGE/AudioDataSource.hpp>

namespace KlayGE
{
	// 声音缓冲区抽象接口
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API AudioBuffer
	{
	public:
		AudioBuffer(AudioDataSourcePtr const & dataSource);
		virtual ~AudioBuffer();

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
		AudioDataSourcePtr dataSource_;

		AudioFormat	format_;
		uint32_t			freq_;

		bool resume_playing_;
	};

	// 声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API SoundBuffer : public AudioBuffer
	{
	public:
		SoundBuffer(AudioDataSourcePtr const & dataSource);
		virtual ~SoundBuffer();

		virtual void Reset();

		bool IsSound() const;

	protected:
		virtual void DoReset() = 0;
	};

	// 音乐缓冲区，流式结构
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API MusicBuffer : public AudioBuffer
	{
	public:
		MusicBuffer(AudioDataSourcePtr const & dataSource);
		virtual ~MusicBuffer();

		void Play(bool loop = false);
		void Stop();
		void Reset();

		bool IsSound() const;

	protected:
		virtual void DoReset() = 0;
		virtual void DoPlay(bool loop) = 0;
		virtual void DoStop() = 0;

		// 每秒读取的次数
		static uint32_t	PreSecond;
	};

	// 管理音频播放
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API AudioEngine
	{
	public:
		AudioEngine();
		virtual ~AudioEngine();

		void Suspend();
		void Resume();

		virtual std::wstring const & Name() const = 0;

		virtual void AddBuffer(size_t id, AudioBufferPtr const & buffer);

		size_t NumBuffer() const;
		virtual AudioBufferPtr Buffer(size_t bufID) const;

		void Play(size_t bufID, bool loop = false);
		void Stop(size_t bufID);
		void PlayAll(bool loop = false);
		void StopAll();

		// 设置和获取音量
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
		std::map<size_t, AudioBufferPtr> audioBufs_;

		float		soundVol_;
		float		musicVol_;
	};
}

#endif		// _AUDIO_HPP
