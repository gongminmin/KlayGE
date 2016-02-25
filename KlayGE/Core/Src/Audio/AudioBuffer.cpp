// AudioBuffer.cpp
// KlayGE 声音引擎 头文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://www.klayge.org
//
// 2.0.4
// 初次建立 (2004.4.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	AudioBuffer::AudioBuffer(AudioDataSourcePtr const & dataSource)
			: dataSource_(dataSource),
				format_(dataSource->Format()),
				freq_(dataSource->Freq()),
				resume_playing_(false)
	{
	}

	AudioBuffer::~AudioBuffer()
	{
	}

	void AudioBuffer::Suspend()
	{
		if (this->IsPlaying())
		{
			resume_playing_ = true;
			this->Stop();
		}
	}

	void AudioBuffer::Resume()
	{
		if (resume_playing_)
		{
			this->Play();
			resume_playing_ = false;
		}
	}
}
