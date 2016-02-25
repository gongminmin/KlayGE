// AudioBuffer.cpp
// KlayGE �������� ͷ�ļ�
// Ver 2.0.4
// ��Ȩ����(C) ������, 2004
// Homepage: http://www.klayge.org
//
// 2.0.4
// ���ν��� (2004.4.7)
//
// �޸ļ�¼
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
