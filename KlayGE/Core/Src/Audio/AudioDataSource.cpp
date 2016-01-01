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
