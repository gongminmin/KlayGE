// SoundBuffer.cpp
// KlayGE 声音缓冲区类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cassert>

#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	SoundBuffer::SoundBuffer(const AudioDataSourcePtr& dataSource)
					: AudioBuffer(dataSource)
	{
	}

	// 缓冲区复位
	/////////////////////////////////////////////////////////////////////////////////
	void SoundBuffer::Reset()
	{
		this->Stop();

		dataSource_->Reset();

		this->DoReset();
	}
}
