// DSAudioEngine.cpp
// KlayGE DirectSound音频引擎类 实现文件
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
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cassert>
#include <cmath>

#include <KlayGE/DSound/DSAudio.hpp>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dsound.lib")

namespace KlayGE
{
	// 从AudioDataSource获取WAVEFORMATEX
	/////////////////////////////////////////////////////////////////////////////////
	WAVEFORMATEX WaveFormatEx(const AudioDataSourcePtr& dataSource)
	{
		WAVEFORMATEX wfx;

		wfx.wFormatTag		= WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec	= dataSource->Freq();
		wfx.cbSize			= 0;

		switch (dataSource->Format())
		{
		case AF_Mono8:
			wfx.wBitsPerSample	= 8;
			wfx.nChannels		= 1;
			break;

		case AF_Mono16:
			wfx.wBitsPerSample	= 16;
			wfx.nChannels		= 1;
			break;

		case AF_Stereo8:
			wfx.wBitsPerSample	= 8;
			wfx.nChannels		= 2;
			break;

		case AF_Stereo16:
			wfx.wBitsPerSample = 16;
			wfx.nChannels		= 2;
			break;
		}

		wfx.nBlockAlign		= wfx.nChannels * wfx.wBitsPerSample / 8;
		wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;

		return wfx;
	}

	// 从0--1的音量转化为dB为单位的音量
	/////////////////////////////////////////////////////////////////////////////////
	long LinearGainToDB(float vol)
	{
		long dB;
		if (vol > 0)
		{
			dB = static_cast<long>(2000 * std::log10f(vol));
		}
		else
		{
			dB = -10000;
		}

		return dB;
	}


	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DSAudioEngine::DSAudioEngine()
	{
		IDirectSound* dsound(NULL);
		TIF(DirectSoundCreate(&DSDEVID_DefaultPlayback, &dsound, NULL));
		dsound_ = COMPtr<IDirectSound>(dsound);

		TIF(dsound_->SetCooperativeLevel(::GetForegroundWindow(), DSSCL_PRIORITY));

		DSBUFFERDESC desc;
		Engine::MemoryInstance().Zero(&desc, sizeof(desc));
		desc.dwSize  = sizeof(desc);
		desc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

		IDirectSoundBuffer* pDSBPrimary(NULL);
		TIF(dsound_->CreateSoundBuffer(&desc, &pDSBPrimary, NULL));
		COMPtr<IDirectSoundBuffer> primaryBuffer(pDSBPrimary);

		WAVEFORMATEX wfx;
		Engine::MemoryInstance().Zero(&wfx, sizeof(wfx));
		wfx.wFormatTag		= WAVE_FORMAT_PCM; 
		wfx.nChannels		= 2;
		wfx.nSamplesPerSec	= 22050;
		wfx.wBitsPerSample	= 16;
		wfx.nBlockAlign		= wfx.wBitsPerSample * wfx.nChannels / 8;
		wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;

		TIF(pDSBPrimary->SetFormat(&wfx));

		TIF(primaryBuffer.QueryInterface<IID_IDirectSound3DListener>(ds3dListener_));


		this->SetListenerPos(MakeVector(0.0f, 0.0f, 0.0f));
		this->SetListenerVel(MakeVector(0.0f, 0.0f, 0.0f));
		this->SetListenerOri(MakeVector(0.0f, 0.0f, 1.0f), MakeVector(0.0f, 1.0f, 0.0f));
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DSAudioEngine::~DSAudioEngine()
	{
		audioBufs_.clear();
	}

	// 音频引擎名字
	/////////////////////////////////////////////////////////////////////////////////
	const WString& DSAudioEngine::Name() const
	{
		static WString name(L"DirectSound Audio Engine");
		return name;
	}

	// 获取3D听者位置
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 DSAudioEngine::GetListenerPos() const
	{
		D3DVECTOR vec;
		this->ds3dListener_->GetPosition(&vec);
		return MakeVector(vec.x, vec.y, vec.z);
	}

	// 设置3D听者位置
	/////////////////////////////////////////////////////////////////////////////////
	void DSAudioEngine::SetListenerPos(const Vector3& v)
	{
		this->ds3dListener_->SetPosition(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
	}

	// 获取3D听者速度
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 DSAudioEngine::GetListenerVel() const
	{
		D3DVECTOR vec;
		this->ds3dListener_->GetVelocity(&vec);
		return MakeVector(vec.x, vec.y, vec.z);
	}

	// 设置3D听者速度
	/////////////////////////////////////////////////////////////////////////////////
	void DSAudioEngine::SetListenerVel(const Vector3& v)
	{
		this->ds3dListener_->SetVelocity(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
	}

	// 获取3D听者方向
	/////////////////////////////////////////////////////////////////////////////////
	void DSAudioEngine::GetListenerOri(Vector3& face, Vector3& up) const
	{
		D3DVECTOR d3dFace, d3dUp;
		this->ds3dListener_->GetOrientation(&d3dFace, &d3dUp);

		face = MakeVector(d3dFace.x, d3dFace.y, d3dFace.z);
		up = MakeVector(d3dUp.x, d3dUp.y, d3dUp.z);
	}

	// 获取3D听者方向
	/////////////////////////////////////////////////////////////////////////////////
	void DSAudioEngine::SetListenerOri(const Vector3& face, const Vector3& up)
	{
		this->ds3dListener_->SetOrientation(face.x(), face.y(), face.z(),
			up.x(), up.y(), up.z(), DS3D_IMMEDIATE);
	}
}
