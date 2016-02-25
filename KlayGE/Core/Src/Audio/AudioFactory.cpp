// AudioFactory.cpp
// KlayGE ��Ƶ������󹤳� ʵ���ļ�
// Ver 3.1.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 3.1.0
// ���ν��� (2005.10.29)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Audio.hpp>
#include <KlayGE/AudioFactory.hpp>

namespace KlayGE
{
	AudioEngine& AudioFactory::AudioEngineInstance()
	{
		if (!ae_)
		{
			ae_ = this->MakeAudioEngine();
		}

		return *ae_;
	}

	void AudioFactory::Suspend()
	{
		if (ae_)
		{
			ae_->Suspend();
		}
		this->DoSuspend();
	}

	void AudioFactory::Resume()
	{
		this->DoResume();
		if (ae_)
		{
			ae_->Resume();
		}
	}
}
