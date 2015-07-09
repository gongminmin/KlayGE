// DSSoundBuffer.cpp
// KlayGE DirectSound������������ ʵ���ļ�
// Ver 2.1.3
// ��Ȩ����(C) ������, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.10.4)
//
// �޸ļ�¼
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

const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

namespace
{
	// ���һ����Ƶ�������Ƿ����
	/////////////////////////////////////////////////////////////////////////////////
	bool IsSourceFree(KlayGE::IDSBufferPtr pDSB)
	{
		if (pDSB)
		{
			DWORD status;
			pDSB->GetStatus(&status);
			return (0 == (status & DSBSTATUS_PLAYING));
		}

		return false;
	}
}

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	DSSoundBuffer::DSSoundBuffer(AudioDataSourcePtr const & dataSource,
									uint32_t numSource, float volume)
					: SoundBuffer(dataSource),
						sources_(numSource)
	{
		WAVEFORMATEX wfx(WaveFormatEx(dataSource));

		// ���� DirectSound ��������Ҫ��������ʹ�ý�����־��
		// ��Ϊʹ��̫�಻��Ҫ�ı�־��Ӱ��Ӳ����������
		DSBUFFERDESC dsbd;
		std::memset(&dsbd, 0, sizeof(dsbd));
		dsbd.dwSize				= sizeof(dsbd);
		dsbd.dwFlags			= DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
		dsbd.guid3DAlgorithm	= GUID_NULL;
		dsbd.dwBufferBytes		= static_cast<uint32_t>(dataSource->Size());
		dsbd.lpwfxFormat		= &wfx;

		std::shared_ptr<IDirectSound> const & dsound = checked_cast<DSAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance())->DSound();

		// DirectSoundֻ�ܲ��� PCM ���ݡ�������ʽ���ܲ��ܹ�����
		IDirectSoundBuffer* temp;
		TIF(dsound->CreateSoundBuffer(&dsbd, &temp, nullptr));
		sources_[0] = IDSBufferPtr(temp);

		// ���ƻ�������ʹ���л�����ʹ��ͬһ������
		for (auto iter = sources_.begin() + 1; iter != sources_.end(); ++ iter)
		{
			TIF(dsound->DuplicateSoundBuffer(sources_[0].get(), &temp));
			*iter = IDSBufferPtr(temp);
		}

		// ����������
		uint8_t* lockedBuffer;			// ָ�򻺳����������ڴ��ָ��
		DWORD lockedBufferSize;		// �������ڴ��С
		TIF(sources_[0]->Lock(0, static_cast<DWORD>(dataSource_->Size()),
			reinterpret_cast<void**>(&lockedBuffer), &lockedBufferSize,
			nullptr, nullptr, DSBLOCK_FROMWRITECURSOR));

		dataSource_->Reset();

		// ������waveFile��仺����
		std::vector<uint8_t> data(dataSource_->Size());
		dataSource_->Read(&data[0], data.size());

		if (data.empty())
		{
			// ���wav�հף��þ������
			std::fill_n(lockedBuffer, lockedBufferSize, 128);
		}
		else
		{
			if (data.size() <= lockedBufferSize)
			{
				std::copy(data.begin(), data.end(), lockedBuffer);

				// �����Ƶ����Դ�Ȼ�����С��������Ƶ������仺����
				std::fill_n(lockedBuffer + data.size(), lockedBufferSize - data.size(), 128);
			}
		}

		// ����������
		sources_[0]->Unlock(lockedBuffer, lockedBufferSize, nullptr, 0);

		this->Position(float3(0, 0, 0));
		this->Velocity(float3(0, 0, 0));
		this->Direction(float3(0, 0, 0));

		this->Reset();

		this->Volume(volume);
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	DSSoundBuffer::~DSSoundBuffer()
	{
		//this->Stop();
		sources_.clear();
	}

	// ���ؿ��еĻ�����
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

	// ����3D�������Ľӿ�
	/////////////////////////////////////////////////////////////////////////////////
	std::shared_ptr<IDirectSound3DBuffer> DSSoundBuffer::Get3DBufferInterface(std::vector<IDSBufferPtr>::iterator iter)
	{
		IDirectSound3DBuffer* ds3DBuffer;
		(*iter)->QueryInterface(IID_IDirectSound3DBuffer, reinterpret_cast<void**>(&ds3DBuffer));

		return MakeCOMPtr(ds3DBuffer);
	}

	// ������Դ
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

	// ֹͣ����
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Stop()
	{
		typedef decltype(sources_) SourcesType;
		for (SourcesType::reference src : sources_)
		{
			src->Stop();
		}
	}

	// ������������λ
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::DoReset()
	{
		typedef decltype(sources_) SourcesType;
		for (SourcesType::reference src : sources_)
		{
			src->SetCurrentPosition(0);
		}
	}

	// ��黺�����Ƿ��ڲ���
	/////////////////////////////////////////////////////////////////////////////////
	bool DSSoundBuffer::IsPlaying() const
	{
		return (std::find_if(sources_.begin(), sources_.end(),
			std::bind(std::logical_not<bool>(), std::bind(IsSourceFree, std::placeholders::_1))) != sources_.end());
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Volume(float vol)
	{
		long const dB(LinearGainToDB(vol));
		typedef decltype(sources_) SourcesType;
		for (SourcesType::reference src : sources_)
		{
			src->SetVolume(dB);
		}
	}

	// ��ȡ��Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSSoundBuffer::Position() const
	{
		return pos_;
	}

	// ������Դλ��
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Position(float3 const & v)
	{
		pos_ = v;
	}

	// ��ȡ��Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSSoundBuffer::Velocity() const
	{
		return vel_;
	}

	// ������Դ�ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Velocity(float3 const & v)
	{
		vel_ = v;
	}

	// ��ȡ��Դ����
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSSoundBuffer::Direction() const
	{
		return dir_;
	}

	// ������Դ����
	/////////////////////////////////////////////////////////////////////////////////
	void DSSoundBuffer::Direction(float3 const & v)
	{
		dir_ = v;
	}
}
