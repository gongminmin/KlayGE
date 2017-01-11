// DSSoundBuffer.cpp
// KlayGE DirectSound声音缓冲区类 实现文件
// Ver 2.1.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://www.klayge.org
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
#include <random>

#include <boost/assert.hpp>

#include <KlayGE/DSound/DSAudio.hpp>

GUID const GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

namespace
{
	// 检查一个音频缓冲区是否空闲
	/////////////////////////////////////////////////////////////////////////////////
	bool IsSourceFree(KlayGE::IDSBufferPtr const & dsb)
	{
		if (dsb)
		{
			DWORD status;
			dsb->GetStatus(&status);
			return (0 == (status & DSBSTATUS_PLAYING));
		}

		return false;
	}
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DSSoundBuffer::DSSoundBuffer(AudioDataSourcePtr const & dataSource,
									uint32_t numSource, float volume)
					: SoundBuffer(dataSource),
						sources_(numSource)
	{
		WAVEFORMATEX wfx(WaveFormatEx(dataSource));

		// 建立 DirectSound 缓冲区，要尽量减少使用建立标志，
		// 因为使用太多不必要的标志会影响硬件加速性能
		DSBUFFERDESC dsbd;
		dsbd.dwSize				= sizeof(dsbd);
		dsbd.dwFlags			= DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
		dsbd.dwBufferBytes		= static_cast<uint32_t>(dataSource->Size());
		dsbd.dwReserved			= 0;
		dsbd.lpwfxFormat		= &wfx;
		dsbd.guid3DAlgorithm = GUID_NULL;

		std::shared_ptr<IDirectSound> const & dsound = checked_cast<DSAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance())->DSound();

		// DirectSound只能播放 PCM 数据。其他格式可能不能工作。
		IDirectSoundBuffer* temp;
		TIF(dsound->CreateSoundBuffer(&dsbd, &temp, nullptr));
		sources_[0] = MakeCOMPtr(temp);

		// 复制缓冲区，使所有缓冲区使用同一段数据
		for (auto iter = sources_.begin() + 1; iter != sources_.end(); ++ iter)
		{
			TIF(dsound->DuplicateSoundBuffer(sources_[0].get(), &temp));
			*iter = MakeCOMPtr(temp);
		}

		// 锁定缓冲区
		uint8_t* lockedBuffer;			// 指向缓冲区锁定的内存的指针
		DWORD lockedBufferSize;		// 锁定的内存大小
		TIF(sources_[0]->Lock(0, static_cast<DWORD>(dataSource_->Size()),
			reinterpret_cast<void**>(&lockedBuffer), &lockedBufferSize,
			nullptr, nullptr, DSBLOCK_FROMWRITECURSOR));

		dataSource_->Reset();

		// 用整个waveFile填充缓冲区
		std::vector<uint8_t> data(dataSource_->Size());
		dataSource_->Read(&data[0], data.size());

		if (data.empty())
		{
			// 如果wav空白，用静音填充
			memset(lockedBuffer, 128, lockedBufferSize);
		}
		else
		{
			if (data.size() <= lockedBufferSize)
			{
				std::copy(data.begin(), data.end(), lockedBuffer);

				// 如果音频数据源比缓冲区小，则用音频数据填充缓冲区
				memset(lockedBuffer + data.size(), 128, lockedBufferSize - data.size());
			}
		}

		// 缓冲区解锁
		sources_[0]->Unlock(lockedBuffer, lockedBufferSize, nullptr, 0);

		this->Position(float3(0, 0, 0));
		this->Velocity(float3(0, 0, 0));
		this->Direction(float3(0, 0, 0));

		this->Reset();

		this->Volume(volume);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DSSoundBuffer::~DSSoundBuffer()
	{
		this->Stop();
		sources_.clear();
	}

	// 返回空闲的缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	std::vector<IDSBufferPtr>::iterator DSSoundBuffer::FreeSource()
	{
		BOOST_ASSERT(!sources_.empty());

		auto iter = std::find_if(sources_.begin(), sources_.end(), IsSourceFree);
		if (iter == sources_.end())
		{
			iter = sources_.begin();
			std::ranlux24_base gen;
			std::uniform_int_distribution<> dis(0, static_cast<int>(sources_.size()));
			std::advance(iter, dis(gen));
		}

		return iter;
	}

	// 返回3D缓冲区的接口
	/////////////////////////////////////////////////////////////////////////////////
	std::shared_ptr<IDirectSound3DBuffer> DSSoundBuffer::Get3DBufferInterface(std::vector<IDSBufferPtr>::iterator iter)
	{
		IDirectSound3DBuffer* ds3DBuffer;
		(*iter)->QueryInterface(IID_IDirectSound3DBuffer, reinterpret_cast<void**>(&ds3DBuffer));

		return MakeCOMPtr(ds3DBuffer);
	}

	// 播放音源
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Play(bool loop)
	{
		auto iter = this->FreeSource();
		auto ds3DBuf = this->Get3DBufferInterface(iter);
		if (ds3DBuf)
		{
			ds3DBuf->SetPosition(pos_[0], pos_[1], pos_[2], DS3D_IMMEDIATE);
			ds3DBuf->SetVelocity(vel_[0], vel_[1], vel_[2], DS3D_IMMEDIATE);
			ds3DBuf->SetConeOrientation(dir_[0], dir_[1], dir_[2], DS3D_IMMEDIATE);
		}

		(*iter)->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);
	}

	// 停止播放
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Stop()
	{
		for (auto const & src : sources_)
		{
			src->Stop();
		}
	}

	// 声音缓冲区复位
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::DoReset()
	{
		for (auto const & src : sources_)
		{
			src->SetCurrentPosition(0);
		}
	}

	// 检查缓冲区是否在播放
	/////////////////////////////////////////////////////////////////////////////////
	bool DSSoundBuffer::IsPlaying() const
	{
		return (std::find_if(sources_.begin(), sources_.end(),
			std::bind(std::logical_not<bool>(), std::bind(IsSourceFree, std::placeholders::_1))) != sources_.end());
	}

	// 设置音量
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Volume(float vol)
	{
		long const dB(LinearGainToDB(vol));
		for (auto const & src : sources_)
		{
			src->SetVolume(dB);
		}
	}

	// 获取声源位置
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSSoundBuffer::Position() const
	{
		return pos_;
	}

	// 设置声源位置
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Position(float3 const & v)
	{
		pos_ = v;
	}

	// 获取声源速度
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSSoundBuffer::Velocity() const
	{
		return vel_;
	}

	// 设置声源速度
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Velocity(float3 const & v)
	{
		vel_ = v;
	}

	// 获取声源方向
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSSoundBuffer::Direction() const
	{
		return dir_;
	}

	// 设置声源方向
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Direction(float3 const & v)
	{
		dir_ = v;
	}
}
