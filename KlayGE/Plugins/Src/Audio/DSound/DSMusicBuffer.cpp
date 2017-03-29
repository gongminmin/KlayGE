// DSMusicBuffer.cpp
// KlayGE DirectSound���ֻ������� ʵ���ļ�
// Ver 2.0.4
// ��Ȩ����(C) ������, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.0.4
// ����timeSetEventʵ�� (2004.3.28)
//
// 2.0.0
// ���ν��� (2003.10.4)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
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
	// ���캯��������һ������������ʽ���ŵĻ�����
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

		// ���� DirectSound ��������Ҫ��������ʹ�ý�����־��
		// ��Ϊʹ��̫�಻��Ҫ�ı�־��Ӱ��Ӳ����������
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

		// DirectSoundֻ�ܲ���PCM���ݡ�������ʽ���ܲ��ܹ�����
		IDirectSoundBuffer* buffer;
		TIFHR(dsound->CreateSoundBuffer(&dsbd, &buffer, nullptr));
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

	// ��������
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

	// ��������λ�Ա��ڴ�ͷ����
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::DoReset()
	{
		dataSource_->Reset();

		buffer_->SetCurrentPosition(0);
	}

	// ������Ƶ��
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

	// ֹͣ������Ƶ��
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
		TIFHR(buffer_->Lock(0, size,
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

	// ��黺�����Ƿ��ڲ���
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

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Volume(float vol)
	{
		buffer_->SetVolume(LinearGainToDB(vol));
	}


	// ��ȡ��Դλ��
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

	// ������Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Position(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetPosition(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	// ��ȡ��Դ�ٶ�
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

	// ������Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Velocity(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetVelocity(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}

	// ��ȡ��Դ����
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

	// ������Դ����
	/////////////////////////////////////////////////////////////////////////////////
	void DSMusicBuffer::Direction(float3 const & v)
	{
		if (ds3DBuffer_)
		{
			ds3DBuffer_->SetConeOrientation(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
		}
	}
}
