// MusicBuffer.cpp
// KlayGE 音乐缓冲区类 实现文件
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
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cassert>

#include <KlayGE/Audio.hpp>

#pragma comment(lib, "libpthread.lib") 

namespace KlayGE
{
	U32 MusicBuffer::PreSecond = 2;

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	MusicBuffer::MusicBuffer(const AudioDataSourcePtr& dataSource)
					: AudioBuffer(dataSource),
						playThread_(0)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	MusicBuffer::~MusicBuffer()
	{
	}

	// 处理时间事件
	/////////////////////////////////////////////////////////////////////////////////
	void* MusicBuffer::PlayProc(void* arg)
	{
		MusicBuffer* streaming(reinterpret_cast<MusicBuffer*>(arg));

		// 更新缓冲区
		streaming->LoopUpdateBuffer();

		return NULL;
	}

	// 缓冲区复位以便于从头播放
	/////////////////////////////////////////////////////////////////////////////////
	void MusicBuffer::Reset()
	{
		this->Stop();

		this->DoReset();
	}

	// 播放音频流
	/////////////////////////////////////////////////////////////////////////////////
	void MusicBuffer::Play(bool loop)
	{
		if (this->IsPlaying())
		{
			this->Stop();
		}

		pthread_create(&playThread_, NULL, PlayProc, this);

		this->DoPlay(loop);
	}

	// 停止播放音频流
	////////////////////////////////////////////////////////////////////////////////
	void MusicBuffer::Stop()
	{
		if (playThread_ != 0)
		{
			pthread_join(playThread_, NULL);
			playThread_ = 0;
		}

		dataSource_->Reset();

		this->DoStop();
	}
}
