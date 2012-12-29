// AudioFactory.cpp
// KlayGE 音频引擎抽象工厂 实现文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 3.1.0
// 初次建立 (2005.10.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Audio.hpp>
#include <KlayGE/AudioFactory.hpp>

namespace KlayGE
{
	class NullAudioFactory : public AudioFactory
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Audio Factory");
			return name;
		}

		AudioEnginePtr MakeAudioEngine()
		{
			return AudioEngine::NullObject();
		}

		AudioBufferPtr MakeSoundBuffer(AudioDataSourcePtr const & /*dataSource*/, uint32_t /*numSource*/)
		{
			return AudioBuffer::NullObject();
		}

		AudioBufferPtr MakeMusicBuffer(AudioDataSourcePtr const & /*dataSource*/, uint32_t /*bufferSeconds*/)
		{
			return AudioBuffer::NullObject();
		}
	};

	AudioFactoryPtr AudioFactory::NullObject()
	{
		static AudioFactoryPtr obj = MakeSharedPtr<NullAudioFactory>();
		return obj;
	}

	AudioEngine& AudioFactory::AudioEngineInstance()
	{
		if (!ae_)
		{
			ae_ = this->MakeAudioEngine();
		}

		return *ae_;
	}
}
