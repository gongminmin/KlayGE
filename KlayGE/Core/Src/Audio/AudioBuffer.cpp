// AudioBuffer.cpp
// KlayGE 声音引擎 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 初次建立 (2004.4.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	class NullAudioBuffer : public AudioBuffer
	{
	public:
		NullAudioBuffer()
			: AudioBuffer(AudioDataSource::NullObject())
		{
		}

		void Play(bool loop = false)
			{ }
		void Reset()
			{ }
		void Stop()
			{ }

		void Volume(float vol)
			{ }

		bool IsPlaying() const
			{ return false; }
		bool IsSound() const
			{ return true; }

		Vector3 Position() const
			{ return Vector3::Zero(); }
		void Position(const Vector3& v)
			{ }
		Vector3 Velocity() const
			{ return Vector3::Zero(); }
		void Velocity(const Vector3& v)
			{ }
		Vector3 Direction() const
			{ return Vector3::Zero(); }
		void Direction(const Vector3& v)
			{ }
	};


	AudioBuffer::AudioBuffer(const AudioDataSourcePtr& dataSource)
			: dataSource_(dataSource),
				format_(dataSource->Format()),
				freq_(dataSource->Freq())
	{
	}

	AudioBuffer::~AudioBuffer()
	{
	}

	AudioBufferPtr AudioBuffer::NullObject()
	{
		static AudioBufferPtr obj(new NullAudioBuffer);
		return obj;
	}
}
