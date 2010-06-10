// AudioFactory.hpp
// KlayGE 音频引擎抽象工厂 头文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// 增加了NullObject (2005.10.29)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _AUDIOFACTORY_HPP
#define _AUDIOFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <string>
#include <boost/noncopyable.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API AudioFactory
	{
	public:
		virtual ~AudioFactory()
			{ }

		static AudioFactoryPtr NullObject();

		virtual std::wstring const & Name() const = 0;

		AudioEngine& AudioEngineInstance();
		virtual AudioBufferPtr MakeSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource = 1) = 0;
		virtual AudioBufferPtr MakeMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds = 2) = 0;

	private:
		virtual AudioEnginePtr MakeAudioEngine() = 0;

	private:
		AudioEnginePtr ae_;
	};

	template <typename AudioEngineType, typename SoundBufferType, typename MusicBufferType>
	class ConcreteAudioFactory : boost::noncopyable, public AudioFactory
	{
	public:
		ConcreteAudioFactory(std::wstring const & name)
			: name_(name)
			{ }

		std::wstring const & Name() const
			{ return name_; }

		AudioBufferPtr MakeSoundBuffer(AudioDataSourcePtr const & dataSource, uint32_t numSource = 1)
		{
			return MakeSharedPtr<SoundBufferType>(dataSource, numSource,
				this->AudioEngineInstance().SoundVolume());
		}

		AudioBufferPtr MakeMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds = 2)
		{
			return MakeSharedPtr<MusicBufferType>(dataSource, bufferSeconds,
				this->AudioEngineInstance().MusicVolume());
		}

	private:
		AudioEnginePtr MakeAudioEngine()
		{
			return MakeSharedPtr<AudioEngineType>();
		}

	private:
		std::wstring const name_;
	};
}

#endif			// _AUDIOFACTORY_HPP
