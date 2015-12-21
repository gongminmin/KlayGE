// DSMusicBuffer.cpp
// KlayGE DirectSound音乐缓冲区类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
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
#include <KFL/ThrowErr.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/DSound/DSAudio.hpp>

namespace KlayGE
{
	// 构造函数。建立一个可以用于流式播放的缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	DSMusicBuffer::DSMusicBuffer(AudioDataSourcePtr const & dataSource, uint32_t bufferSeconds, float volume)
					: MusicBuffer(dataSource),
						played_(false), stopped_(true)
	{
		WAVEFORMATEX wfx(WaveFormatEx(dataSource));
		fillSize_	= wfx.nAvgBytesPerSec / PreSecond;
		fillCount_	= bufferSeconds * PreSecond;

		bool const mono(1 == wfx.nChannels);

		std::shared_ptr<IDirectSound> const & dsound = checked_cast<DSAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance())->DSound();

		// 建立 DirectSound 缓冲区，要尽量减少使用建立标志，
		// 因为使用太多不必要的标志会影响硬件加速性能
		DSBUFFERDESC dsbd;
		dsbd.dwSize = sizeof(dsbd);
		dsbd.dwFlags = DSBCAPS_CTRLVOLUME;
		if (mono)
		{
			dsbd.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
			dsbd.guid3DAlgorithm	= GUID_NULL;
		}
		dsbd.dwBufferBytes		= fillSize_ * fillCount_;
		dsbd.dwReserved			= 0;
		dsbd.lpwfxFormat		= &wfx;

		// DirectSound只能播放PCM数据。其他格式可能不能工作。
		IDirectSoundBuffer* buffer;
		TIF(dsound->CreateSoundBuffer(&dsbd, &buffer, nullptr));
		buffer_ = MakeCOMPtr(buffer);

		if (mono)
		{
			IDirectSound3DBuffer* ds3DBuffer;
			buffer_->QueryInterface(IID_IDirectSound3DBuffer,
				reinterpret_cast<void**>(&ds3DBuffer));
			ds3DBuffer_ = MakeCOMPtr(ds3DBuffer);
		}

		this->Position(float3::Zero());
		this->Velocity(float3::Zero());
		this->Direction(float3::Zero());

		this->Volume(volume);

		this->Reset();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DSMusicBuffer::~DSMusicBuffer()
	{
		this->Stop();
	}

	void DSMusicBuffer::LoopUpdateBuffer()
	{
		std::unique_lock<std::mutex> lock(play_mutex_);
		while (!played_)
		{
			play_cond_.wait(lock);
		}
		played_ = false;

		while (!stopped_)
		{
			DWORD play_cursor, write_cursor;
			buffer_->GetCurrentPosition(&play_cursor, &write_cursor);

			uint32_t const next_cursor = play_cursor + fillSize_;
			if (next_cursor >= write_cursor)
			{
				if (this->FillData(fillSize_))
				{
					if (loop_)
					{
						stopped_ = false;
						this->DoReset();
					}
					else
					{
						stopped_ = true;
						buffer_->Stop();
					}
				}
			}

			Sleep(1000 / PreSecond);
		}
	}

	// 缓冲区复位以便于从头播放
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoReset()
	{
		dataSource_->Reset();

		buffer_->SetCurrentPosition(0);
	}

	// 播放音频流
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoPlay(bool loop)
	{
		play_thread_ = Context::Instance().ThreadPool()(std::bind(&DSMusicBuffer::LoopUpdateBuffer, this));

		loop_ = loop;

		stopped_ = false;
		{
			std::lock_guard<std::mutex> lock(play_mutex_);
			played_ = true;
		}
		play_cond_.notify_one();

		buffer_->Play(0, 0, DSBPLAY_LOOPING);
	}

	// 停止播放音频流
	////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoStop()
	{
		if (!stopped_)
		{
			stopped_ = true;
			play_thread_();
		}

		buffer_->Stop();
	}

	bool DSMusicBuffer::FillData(uint32_t size)
	{
		std::vector<uint8_t> data(size);
		data.resize(dataSource_->Read(&data[0], size));

		uint8_t* locked_buff[2];
		DWORD locked_buff_size[2];
		TIF(buffer_->Lock(0, size,
			reinterpret_cast<void**>(&locked_buff[0]), &locked_buff_size[0],
			reinterpret_cast<void**>(&locked_buff[1]), &locked_buff_size[1],
			DSBLOCK_FROMWRITECURSOR));

		bool ret;
		if (data.empty())
		{
			memset(locked_buff[0], 0, locked_buff_size[0]);
			if (locked_buff[1] != nullptr)
			{
				memset(locked_buff[1], 0, locked_buff_size[1]);
			}

			ret = true;
		}
		else
		{
			memcpy(locked_buff[0], &data[0], locked_buff_size[0]);
			if (locked_buff_size[0] > data.size())
			{
				memset(locked_buff[0], 0, locked_buff_size[0] - data.size());
			}
			if (locked_buff[1] != nullptr)
			{
				memcpy(locked_buff[1], &data[locked_buff_size[0]], locked_buff_size[1]);
				if (locked_buff_size[1] > data.size() - locked_buff_size[0])
				{
					memset(locked_buff[1], 0, locked_buff_size[1] - data.size() + locked_buff_size[0]);
				}
			}

			ret = false;
		}

		buffer_->Unlock(locked_buff[0], locked_buff_size[0], locked_buff[1], locked_buff_size[1]);
		return ret;
	}

	// 检查缓冲区是否在播放
	/////////////////////////////////////////////////////////////////////////////////
	bool DSMusicBuffer::IsPlaying() const
	{
		if (buffer_)
		{
			DWORD status;
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
	float3 DSMusicBuffer::Position() const
	{
		float3 ret(float3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetPosition(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	// 设置声源位置
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Position(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetPosition(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	// 获取声源速度
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSMusicBuffer::Velocity() const
	{
		float3 ret(float3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetVelocity(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	// 设置声源速度
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Velocity(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetVelocity(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	// 获取声源方向
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSMusicBuffer::Direction() const
	{
		float3 ret(float3::Zero());

		if (ds3DBuffer_)
		{
			D3DVECTOR v;
			ds3DBuffer_->GetConeOrientation(&v);
			ret = float3(v.x, v.y, v.z);
		}

		return ret;
	}

	// 设置声源方向
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Direction(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetConeOrientation(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}
}
