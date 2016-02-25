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

namespace KlayGE
{
	AudioDataSource::~AudioDataSource()
	{
	}

	AudioFormat AudioDataSource::Format() const
	{
		return this->format_;
	}

	uint32_t AudioDataSource::Freq() const
	{
		return this->freq_;
	}


	void AudioDataSourceFactory::Suspend()
	{
		this->DoSuspend();
	}

	void AudioDataSourceFactory::Resume()
	{
		this->DoResume();
	}
}
