// Audio.hpp
// KlayGE 声音引擎 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/MathTypes.hpp>

#include <map>

#include <KlayGE/AudioDataSource.hpp>
#include <KlayGE/MgrBase.hpp>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	// 声音缓冲区抽象接口
	/////////////////////////////////////////////////////////////////////////////////
	class AudioBuffer
	{
	public:
		AudioBuffer(const AudioDataSourcePtr& dataSource);
		virtual ~AudioBuffer();

		static AudioBufferPtr NullObject();

		virtual void Play(bool loop = false) = 0;
		virtual void Reset() = 0;
		virtual void Stop() = 0;

		virtual void Volume(float vol) = 0;

		virtual bool IsPlaying() const = 0;
		virtual bool IsSound() const = 0;

		virtual Vector3 Position() const = 0;
		virtual void Position(const Vector3& v) = 0;
		virtual Vector3 Velocity() const = 0;
		virtual void Velocity(const Vector3& v) = 0;
		virtual Vector3 Direction() const = 0;
		virtual void Direction(const Vector3& v) = 0;

	protected:
		AudioDataSourcePtr dataSource_;

		AudioFormat	format_;
		U32			freq_;
	};

	// 声音缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	class SoundBuffer : public AudioBuffer
	{
	public:
		SoundBuffer(const AudioDataSourcePtr& dataSource);
		virtual ~SoundBuffer();

		virtual void Reset();

		bool IsSound() const;

	protected:
		virtual void DoReset() = 0;
	};

	// 音乐缓冲区，流式结构
	/////////////////////////////////////////////////////////////////////////////////
	class MusicBuffer : public AudioBuffer
	{
	public:
		MusicBuffer(const AudioDataSourcePtr& dataSource);
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
		static U32	PreSecond;
	};

	// 管理音频播放
	/////////////////////////////////////////////////////////////////////////////////
	class AudioEngine
	{
		typedef ManagerBase<AudioBufferPtr>		AudioBufs;
		typedef AudioBufs::iterator				AudioBufsIter;
		typedef AudioBufs::const_iterator		AudioBufsConstIter;

	public:
		AudioEngine();
		virtual ~AudioEngine();

		static AudioEnginePtr NullObject();

		virtual const WString& Name() const = 0;

		void AddBuffer(size_t id, const AudioBufferPtr& buffer);

		size_t BufferNum() const;
		AudioBufferPtr Buffer(size_t bufID);
		const AudioBufferPtr Buffer(size_t bufID) const;

		void Play(size_t bufID, bool loop = false);
		void Stop(size_t bufID);
		void PlayAll(bool loop = false);
		void StopAll();

		// 设置和获取音量
		void  SoundVolume(float vol);
		float SoundVolume() const;
		void  MusicVolume(float vol);
		float MusicVolume() const;

		virtual Vector3 GetListenerPos() const = 0;
		virtual void SetListenerPos(const Vector3& v) = 0;
		virtual Vector3 GetListenerVel() const = 0;
		virtual void SetListenerVel(const Vector3& v) = 0;
		virtual void GetListenerOri(Vector3& face, Vector3& up) const = 0;
		virtual void SetListenerOri(const Vector3& face, const Vector3& up) = 0;

	protected:
		AudioBufs	audioBufs_;

		float		soundVol_;
		float		musicVol_;
	};
}

#endif		// _AUDIO_HPP
