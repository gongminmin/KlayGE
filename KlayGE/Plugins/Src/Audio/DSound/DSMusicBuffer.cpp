// DSMusicBuffer.cpp
// KlayGE DirectSound音乐缓冲区类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 改用timeSetEvent实现 (2004.3.28)
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cassert>
#include <algorithm>
#include <cstring>

#include <KlayGE/DSound/DSAudio.hpp>

#pragma comment(lib, "winmm.lib")

namespace KlayGE
{
	// 构造函数。建立一个可以用于流式播放的缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	DSMusicBuffer::DSMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds, float volume)
					: MusicBuffer(dataSource),
						writePos_(0)
	{
		WAVEFORMATEX wfx(WaveFormatEx(dataSource));
		fillSize_	= wfx.nAvgBytesPerSec / PreSecond;
		fillCount_	= bufferSeconds * PreSecond;

		bool const mono(1 == wfx.nChannels);

		boost::shared_ptr<IDirectSound> const & dsound(static_cast<DSAudioEngine&>(Context::Instance().AudioFactoryInstance().AudioEngineInstance()).DSound());

		// 建立 DirectSound 缓冲区，要尽量减少使用建立标志，
		// 因为使用太多不必要的标志会影响硬件加速性能
		DSBUFFERDESC dsbd;
		std::memset(&dsbd, 0, sizeof(dsbd));
		dsbd.dwSize = sizeof(dsbd);
		dsbd.dwFlags = DSBCAPS_CTRLVOLUME;
		if (mono)
		{
			dsbd.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
			dsbd.guid3DAlgorithm	= GUID_NULL;
		}
		dsbd.dwBufferBytes		= fillSize_ * fillCount_;
		dsbd.lpwfxFormat		= &wfx;

		// DirectSound只能播放PCM数据。其他格式可能不能工作。
		IDirectSoundBuffer* buffer;
		TIF(dsound->CreateSoundBuffer(&dsbd, &buffer, NULL));
		buffer_ = MakeCOMPtr(buffer);

		if (mono)
		{
			IDirectSound3DBuffer* ds3DBuffer;
			buffer_->QueryInterface(IID_IDirectSound3DBuffer,
				reinterpret_cast<void**>(&ds3DBuffer));
			ds3DBuffer_ = MakeCOMPtr(ds3DBuffer);
		}

		this->Position(Vector3::Zero());
		this->Velocity(Vector3::Zero());
		this->Direction(Vector3::Zero());

		this->Volume(volume);

		this->Reset();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DSMusicBuffer::~DSMusicBuffer()
	{
		this->Stop();
	}

	// 更新缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::TimerProc(UINT timerID, UINT /*uMsg*/,
										DWORD_PTR dwUser, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
	{
		DSMusicBuffer* buffer(reinterpret_cast<DSMusicBuffer*>(dwUser));

		if (timerID != buffer->timerID_)
		{
			return;
		}

		buffer->FillBuffer();
	}

	void DSMusicBuffer::FillBuffer()
	{
		// 锁定缓冲区
		uint8_t* lockedBuffer;			// 指向缓冲区锁定的内存的指针
		uint32_t lockedBufferSize;		// 锁定的内存大小
		TIF(buffer_->Lock(fillSize_ * writePos_, fillSize_,
			reinterpret_cast<void**>(&lockedBuffer), &lockedBufferSize,
			NULL, NULL, 0));

		std::vector<uint8_t> data(fillSize_);
		data.resize(dataSource_->Read(&data[0], fillSize_));

		if (data.empty())
		{
			std::fill_n(lockedBuffer, lockedBufferSize, 0);
			this->Stop();
		}
		else
		{
			std::copy(data.begin(), data.end(), lockedBuffer);

			std::fill_n(lockedBuffer + data.size(), lockedBufferSize - data.size(), 0);
		}

		// 缓冲区解锁
		buffer_->Unlock(lockedBuffer, lockedBufferSize, NULL, 0);

		// 形成环形缓冲区
		++ writePos_;
		writePos_ %= fillCount_;
	}

	// 缓冲区复位以便于从头播放
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoReset()
	{
		this->writePos_	= 0;

		dataSource_->Reset();

		// 锁定缓冲区
		uint8_t* lockedBuffer;			// 指向缓冲区锁定的内存的指针
		uint32_t lockedBufferSize;		// 锁定的内存大小
		TIF(buffer_->Lock(0, fillSize_ * fillCount_,
			reinterpret_cast<void**>(&lockedBuffer), &lockedBufferSize, NULL, NULL, 0));

		std::vector<uint8_t> data(fillSize_ * fillCount_);
		data.resize(dataSource_->Read(&data[0], fillSize_ * fillCount_));

		if (data.empty())
		{
			// 如果音频数据空白，用静音填充
			std::fill_n(lockedBuffer, lockedBufferSize, 0);
		}
		else
		{
			// 如果数据源比缓冲区小，则用音频数据填充缓冲区
			std::copy(data.begin(), data.end(), lockedBuffer);

			// 剩下的区域用空白填充
			std::fill_n(lockedBuffer + data.size(), lockedBufferSize - data.size(), 0);
		}

		// 缓冲区解锁
		buffer_->Unlock(lockedBuffer, lockedBufferSize, NULL, 0);

		buffer_->SetCurrentPosition(0);
	}

	// 播放音频流
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoPlay(bool /*loop*/)
	{
		buffer_->Play(0, 0, DSBPLAY_LOOPING);

		timerID_ = timeSetEvent(1000 / this->PreSecond, 0, TimerProc,
								reinterpret_cast<DWORD_PTR>(this), TIME_PERIODIC);
	}

	// 停止播放音频流
	////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoStop()
	{
		timeKillEvent(timerID_);

		buffer_->Stop();
	}

	// 检查缓冲区是否在播放
	/////////////////////////////////////////////////////////////////////////////////
	bool DSMusicBuffer::IsPlaying() const
	{
		if (buffer_)
		{
			uint32_t status;
			buffer_->GetStatus(&status);
			return ((status & DSBSTATUS_PLAYING) != 0);
		}

		return false;
	}

	// 设置音量
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Volume(float vol)
	{
		buffer_->SetVolume(LinearGainToDB(vol));
	}


	// 获取声源位置
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 DSMusicBuffer::Position() const
	{
		Vector3 ret(Vector3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetPosition(&v);
			ret = Vector3(v.x, v.y, v.z);
		}

		return ret;
	}

	// 设置声源位置
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Position(Vector3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetPosition(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	// 获取声源速度
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 DSMusicBuffer::Velocity() const
	{
		Vector3 ret(Vector3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetVelocity(&v);
			ret = Vector3(v.x, v.y, v.z);
		}

		return ret;
	}

	// 设置声源速度
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Velocity(Vector3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetVelocity(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	// 获取声源方向
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 DSMusicBuffer::Direction() const
	{
		Vector3 ret(Vector3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetConeOrientation(&v);
			ret = Vector3(v.x, v.y, v.z);
		}

		return ret;
	}

	// 设置声源方向
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Direction(Vector3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetConeOrientation(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}
}
