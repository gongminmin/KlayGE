// AudioFactory.hpp
// KlayGE 音频引擎抽象工厂 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _AUDIOFACTORY_HPP
#define _AUDIOFACTORY_HPP

namespace KlayGE
{
	class AudioFactory
	{
	public:
		virtual ~AudioFactory()
			{ }

		virtual const WString& Name() const = 0;

		virtual AudioEngine& AudioEngineInstance() = 0;
		virtual AudioBufferPtr MakeSoundBuffer(const AudioDataSourcePtr& dataSource, U32 sourceNum = 1) = 0;
		virtual AudioBufferPtr MakeMusicBuffer(const AudioDataSourcePtr& dataSource, U32 bufferSeconds = 2) = 0;
	};
}

#endif			// _AUDIOFACTORY_HPP