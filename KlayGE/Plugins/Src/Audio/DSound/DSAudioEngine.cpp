// DSAudioEngine.cpp
// KlayGE DirectSound音频引擎类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#define INITGUID
#include <KFL/COMPtr.hpp>
#include <KFL/ThrowErr.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cmath>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/DSound/DSAudio.hpp>

namespace KlayGE
{
	// 从AudioDataSource获取WAVEFORMATEX
	/////////////////////////////////////////////////////////////////////////////////
	WAVEFORMATEX WaveFormatEx(AudioDataSourcePtr const & dataSource)
	{
		WAVEFORMATEX wfx;

		wfx.wFormatTag		= WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec	= dataSource->Freq();
		wfx.cbSize			= 0;
		wfx.wBitsPerSample	= 8;
		wfx.nChannels		= 1;

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

		case AF_Unknown:
			BOOST_ASSERT(false);
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
			dB = static_cast<long>(2000 * std::log10(vol));
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
		mod_dsound_ = ::LoadLibraryEx(TEXT("dsound.dll"), nullptr, 0);
		if (nullptr == mod_dsound_)
		{
			::MessageBoxW(nullptr, L"Can't load dsound.dll", L"Error", MB_OK);
		}

		if (mod_dsound_ != nullptr)
		{
			DynamicDirectSoundCreate_ = reinterpret_cast<DirectSoundCreateFunc>(::GetProcAddress(mod_dsound_, "DirectSoundCreate"));
		}

		IDirectSound* dsound(nullptr);
		TIF(DynamicDirectSoundCreate_(&DSDEVID_DefaultPlayback, &dsound, nullptr));
		dsound_ = MakeCOMPtr(dsound);

		TIF(dsound_->SetCooperativeLevel(::GetForegroundWindow(), DSSCL_PRIORITY));

		DSBUFFERDESC desc;
		std::memset(&desc, 0, sizeof(desc));
		desc.dwSize  = sizeof(desc);
		desc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

		IDirectSoundBuffer* pDSBPrimary(nullptr);
		TIF(dsound_->CreateSoundBuffer(&desc, &pDSBPrimary, nullptr));

		WAVEFORMATEX wfx;
		wfx.wFormatTag		= WAVE_FORMAT_PCM;
		wfx.nChannels		= 2;
		wfx.nSamplesPerSec	= 22050;
		wfx.wBitsPerSample	= 16;
		wfx.nBlockAlign		= wfx.wBitsPerSample * wfx.nChannels / 8;
		wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize			= 0;

		TIF(pDSBPrimary->SetFormat(&wfx));

		IDirectSound3DListener* ds3dListener;
		TIF(pDSBPrimary->QueryInterface(IID_IDirectSound3DListener,
			reinterpret_cast<void**>(&ds3dListener)));
		ds3dListener_ = MakeCOMPtr(ds3dListener);


		this->SetListenerPos(float3(0, 0, 0));
		this->SetListenerVel(float3(0, 0, 0));
		this->SetListenerOri(float3(0, 0, 1), float3(0, 1, 0));

		pDSBPrimary->Release();
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DSAudioEngine::~DSAudioEngine()
	{
		audioBufs_.clear();
		ds3dListener_.reset();
		dsound_.reset();

		::FreeLibrary(mod_dsound_);
	}

	void DSAudioEngine::DoSuspend()
	{
		// TODO
	}

	void DSAudioEngine::DoResume()
	{
		// TODO
	}

	// 音频引擎名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & DSAudioEngine::Name() const
	{
		static std::wstring const name(L"DirectSound Audio Engine");
		return name;
	}

	// 获取3D听者位置
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSAudioEngine::GetListenerPos() const
	{
		D3DVECTOR vec;
		this->ds3dListener_->GetPosition(&vec);
		return float3(vec.x, vec.y, vec.z);
	}

	// 设置3D听者位置
	/////////////////////////////////////////////////////////////////////////////////
	void DSAudioEngine::SetListenerPos(float3 const & v)
	{
		this->ds3dListener_->SetPosition(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
	}

	// 获取3D听者速度
	/////////////////////////////////////////////////////////////////////////////////
	float3 DSAudioEngine::GetListenerVel() const
	{
		D3DVECTOR vec;
		this->ds3dListener_->GetVelocity(&vec);
		return float3(vec.x, vec.y, vec.z);
	}

	// 设置3D听者速度
	/////////////////////////////////////////////////////////////////////////////////
	void DSAudioEngine::SetListenerVel(float3 const & v)
	{
		this->ds3dListener_->SetVelocity(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
	}

	// 获取3D听者方向
	/////////////////////////////////////////////////////////////////////////////////
	void DSAudioEngine::GetListenerOri(float3& face, float3& up) const
	{
		D3DVECTOR d3dFace, d3dUp;
		this->ds3dListener_->GetOrientation(&d3dFace, &d3dUp);

		face = float3(d3dFace.x, d3dFace.y, d3dFace.z);
		up = float3(d3dUp.x, d3dUp.y, d3dUp.z);
	}

	// 获取3D听者方向
	/////////////////////////////////////////////////////////////////////////////////
	void DSAudioEngine::SetListenerOri(float3 const & face, float3 const & up)
	{
		this->ds3dListener_->SetOrientation(face.x(), face.y(), face.z(),
			up.x(), up.y(), up.z(), DS3D_IMMEDIATE);
	}
}
