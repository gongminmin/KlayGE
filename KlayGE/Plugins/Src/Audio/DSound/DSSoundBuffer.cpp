// DSSoundBuffer.cpp
// KlayGE DirectSound声音缓冲区类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cassert>

#include <KlayGE/DSound/DSAudio.hpp>

namespace
{
	using namespace KlayGE;

	// 检查一个音频缓冲区是否空闲
	/////////////////////////////////////////////////////////////////////////////////
	bool IsSourceFree(DSBufferType pDSB)
	{
		if (pDSB.Get() != NULL)
		{
			U32 status;
			pDSB->GetStatus(&status);
			return (0 == (status & DSBSTATUS_PLAYING));
		}

		return false;
	}
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DSSoundBuffer::DSSoundBuffer(const AudioDataSourcePtr& dataSource,
									U32 sourceNum, float volume)
					: SoundBuffer(dataSource),
						sources_(sourceNum)
	{
		WAVEFORMATEX wfx(WaveFormatEx(dataSource));

		// 建立 DirectSound 缓冲区，要尽量减少使用建立标志，
		// 因为使用太多不必要的标志会影响硬件加速性能
		DSBUFFERDESC dsbd;
		Engine::MemoryInstance().Zero(&dsbd, sizeof(dsbd));
		dsbd.dwSize				= sizeof(dsbd);
		dsbd.dwFlags			= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
		dsbd.guid3DAlgorithm	= GUID_NULL;
		dsbd.dwBufferBytes		= static_cast<U32>(dataSource->Size());
		dsbd.lpwfxFormat		= &wfx;

		const COMPtr<IDirectSound>& dsound(static_cast<DSAudioEngine&>(Engine::AudioFactoryInstance().AudioEngineInstance()).DSound());

		// DirectSound只能播放 PCM 数据。其他格式可能不能工作。
		IDirectSoundBuffer* temp;
		TIF(dsound->CreateSoundBuffer(&dsbd, &temp, NULL));
		sources_[0] = DSBufferType(temp);

		// 复制缓冲区，使所有缓冲区使用同一段数据
		for (SourcesIter iter = sources_.begin() + 1; iter != sources_.end(); ++ iter)
		{
			TIF(dsound->DuplicateSoundBuffer(sources_[0].Get(), &temp));
			*iter = DSBufferType(temp);
		}

		// 锁定缓冲区
		PVOID lockedBuffer;			// 指向缓冲区锁定的内存的指针
		U32   lockedBufferSize;		// 锁定的内存大小
		TIF(sources_[0]->Lock(0, dataSource_->Size(), &lockedBuffer, &lockedBufferSize, 
			NULL, NULL, DSBLOCK_FROMWRITECURSOR));

		dataSource_->Reset();

		// 用整个waveFile填充缓冲区
		std::vector<U8, alloc<U8> > data(dataSource_->Size());
		dataSource_->Read(&data[0], data.size());

		if (0 == data.size())
		{
			// 如果wav空白，用静音填充
			Engine::MemoryInstance().Set(lockedBuffer, 128, lockedBufferSize);
		}
		else
		{
			if (data.size() <= lockedBufferSize)
			{
				Engine::MemoryInstance().Cpy(lockedBuffer, &data[0], data.size());

				// 如果音频数据源比缓冲区小，则用音频数据填充缓冲区
				Engine::MemoryInstance().Set(static_cast<U8*>(lockedBuffer) + data.size(), 
					128, lockedBufferSize - data.size());
			}
		}

		// 缓冲区解锁
		sources_[0]->Unlock(lockedBuffer, lockedBufferSize, NULL, 0);

		this->Position(MakeVector(0.0f, 0.0f, 0.0f));
		this->Velocity(MakeVector(0.0f, 0.0f, 0.0f));
		this->Direction(MakeVector(0.0f, 0.0f, 0.0f));

		this->Reset();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DSSoundBuffer::~DSSoundBuffer()
	{
		//this->Stop();
		sources_.clear();
	}

	// 返回空闲的缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	DSSoundBuffer::SourcesIter DSSoundBuffer::FreeSource()
	{
		assert(!sources_.empty());

		SourcesIter iter(std::find_if(sources_.begin(), sources_.end(), IsSourceFree));

		if (iter == sources_.end())
		{
			iter = sources_.begin();
			std::advance(iter, Engine::MathInstance().Random(sources_.size()));
		}

		return iter;
	}

	// 返回3D缓冲区的接口
	/////////////////////////////////////////////////////////////////////////////////
	COMPtr<IDirectSound3DBuffer> DSSoundBuffer::Get3DBufferInterface(SourcesIter iter)
	{
		COMPtr<IDirectSound3DBuffer> ds3DBuffer;
		iter->QueryInterface<IID_IDirectSound3DBuffer>(ds3DBuffer);

		return ds3DBuffer;
	}

	// 播放音源
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Play(bool loop)
	{
		SourcesIter iter(this->FreeSource());

		COMPtr<IDirectSound3DBuffer> ds3DBuf(this->Get3DBufferInterface(iter));
		if (ds3DBuf.Get() != NULL)
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
		for (SourcesIter iter = sources_.begin(); iter != sources_.end(); ++ iter)
		{
			(*iter)->Stop();
		}
	}

	// 声音缓冲区复位
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::DoReset()
	{
		for (SourcesIter iter = sources_.begin(); iter != sources_.end(); ++ iter)
		{
			(*iter)->SetCurrentPosition(0);
		}
	}

	// 检查缓冲区是否在播放
	/////////////////////////////////////////////////////////////////////////////////
	bool DSSoundBuffer::IsPlaying() const
	{
		return (std::find_if(sources_.begin(), sources_.end(),
			std::not1(std::ptr_fun(IsSourceFree))) != sources_.end());
	}

	// 设置音量
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Volume(float vol)
	{
		const long dB(LinearGainToDB(vol));
		for (SourcesIter iter = sources_.begin(); iter != sources_.end(); ++ iter)
		{
			(*iter)->SetVolume(dB);
		}
	}

	// 获取声源位置
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 DSSoundBuffer::Position() const
	{
		return pos_;
	}

	// 设置声源位置
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Position(const Vector3& v)
	{
		pos_ = v;
	}

	// 获取声源速度
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 DSSoundBuffer::Velocity() const
	{
		return vel_;
	}

	// 设置声源速度
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Velocity(const Vector3& v)
	{
		vel_ = v;
	}

	// 获取声源方向
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 DSSoundBuffer::Direction() const
	{
		return dir_;
	}

	// 设置声源方向
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Direction(const Vector3& v)
	{
		dir_ = v;
	}
}
