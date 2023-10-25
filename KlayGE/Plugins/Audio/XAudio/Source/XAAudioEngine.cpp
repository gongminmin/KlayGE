/**
 * @file XAAudioEngine.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#define INITGUID
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cmath>
#include <cstring>
#include <functional>
#include <ostream>

#include <boost/assert.hpp>

#include "XAAudio.hpp"

#if (_WIN32_WINNT <= _WIN32_WINNT_WIN7) && defined(KLAYGE_COMPILER_GCC)
#ifndef __IXAudio27_INTERFACE_DEFINED__

struct XAUDIO2_DEVICE_DETAILS
{
	WCHAR DeviceID[256];
	WCHAR DisplayName[256];
	XAUDIO2_DEVICE_ROLE Role;
	WAVEFORMATEXTENSIBLE OutputFormat;
};

DEFINE_GUID(IID_IXAudio27, 0x8bcf1f58, 0x9fe7, 0x4583, 0x8a, 0xc6, 0xe2, 0xad, 0xc4, 0x65, 0xc8, 0xbb);
struct IXAudio27 : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE GetDeviceCount(uint32_t* pCount) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetDeviceDetails(uint32_t Index, XAUDIO2_DEVICE_DETAILS* pDeviceDetails) = 0;

	virtual HRESULT STDMETHODCALLTYPE Initialize(uint32_t Flags = 0, XAUDIO2_PROCESSOR XAudio2Processor = XAUDIO2_DEFAULT_PROCESSOR) = 0;

	virtual HRESULT STDMETHODCALLTYPE RegisterForCallbacks(IXAudio2EngineCallback* pCallback) = 0;

	virtual void STDMETHODCALLTYPE UnregisterForCallbacks(IXAudio2EngineCallback* pCallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateSourceVoice(IXAudio2SourceVoice** ppSourceVoice, const WAVEFORMATEX* pSourceFormat,
		uint32_t Flags = 0, float MaxFrequencyRatio = XAUDIO2_DEFAULT_FREQ_RATIO, IXAudio2VoiceCallback* pCallback = 0,
		const XAUDIO2_VOICE_SENDS* pSendList = 0, const XAUDIO2_EFFECT_CHAIN* pEffectChain = 0) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateSubmixVoice(IXAudio2SubmixVoice** ppSubmixVoice, uint32_t InputChannels,
		uint32_t InputSampleRate, uint32_t Flags = 0, uint32_t ProcessingStage = 0, const XAUDIO2_VOICE_SENDS* pSendList = 0,
		const XAUDIO2_EFFECT_CHAIN* pEffectChain = 0) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateMasteringVoice(IXAudio2MasteringVoice** ppMasteringVoice,
		uint32_t InputChannels = XAUDIO2_DEFAULT_CHANNELS, uint32_t InputSampleRate = XAUDIO2_DEFAULT_SAMPLERATE, uint32_t Flags = 0,
		uint32_t DeviceIndex = 0, const XAUDIO2_EFFECT_CHAIN* pEffectChain = 0) = 0;

	virtual HRESULT STDMETHODCALLTYPE StartEngine() = 0;

	virtual void STDMETHODCALLTYPE StopEngine() = 0;

	virtual HRESULT STDMETHODCALLTYPE CommitChanges(uint32_t OperationSet) = 0;

	virtual void STDMETHODCALLTYPE GetPerformanceData(XAUDIO2_PERFORMANCE_DATA* pPerfData) = 0;

	virtual void STDMETHODCALLTYPE SetDebugConfiguration(const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration, void* pReserved = 0) = 0;
};

#endif

DEFINE_UUID_OF(IXAudio27);
#endif

namespace KlayGE
{
	WAVEFORMATEX WaveFormatEx(AudioDataSourcePtr const & data_source)
	{
		WAVEFORMATEX wfx;

		wfx.wFormatTag		= WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec	= data_source->Freq();
		wfx.cbSize			= 0;
		wfx.wBitsPerSample	= 8;
		wfx.nChannels		= 1;

		switch (data_source->Format())
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

		default:
			KFL_UNREACHABLE("Invalid audio format");
		}

		wfx.nBlockAlign		= wfx.nChannels * wfx.wBitsPerSample / 8;
		wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;

		return wfx;
	}

	XAAudioEngine::XAAudioEngine()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		if (!mod_xaudio2_.Load(XAUDIO2_DLL_A))
		{
			LogError() << "COULDN'T load " XAUDIO2_DLL_A << std::endl;
			Verify(false);
		}

#if (_WIN32_WINNT > _WIN32_WINNT_WIN7)
		DynamicXAudio2Create_ = reinterpret_cast<XAudio2CreateFunc>(mod_xaudio2_.GetProcAddress("XAudio2Create"));
#else
		DynamicXAudio2Create_ = ::XAudio2Create;
#endif
		DynamicX3DAudioInitialize_ = reinterpret_cast<X3DAudioInitializeFunc>(mod_xaudio2_.GetProcAddress("X3DAudioInitialize"));
		DynamicX3DAudioCalculate_ = reinterpret_cast<X3DAudioCalculateFunc>(mod_xaudio2_.GetProcAddress("X3DAudioCalculate"));
#else
		DynamicXAudio2Create_ = ::XAudio2Create;
		DynamicX3DAudioInitialize_ = ::X3DAudioInitialize;
		DynamicX3DAudioCalculate_ = ::X3DAudioCalculate;
#endif

		uint32_t flags = 0;
#if (_WIN32_WINNT <= _WIN32_WINNT_WIN7) && defined(KLAYGE_DEBUG)
		flags |= XAUDIO2_DEBUG_ENGINE;
#endif
		TIFHR(DynamicXAudio2Create_(xaudio_.put(), flags, Processor1));

		IXAudio2MasteringVoice*	mastering_voice;
		TIFHR(xaudio_->CreateMasteringVoice(&mastering_voice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE));
		mastering_voice_ = std::shared_ptr<IXAudio2MasteringVoice>(mastering_voice, std::mem_fn(&IXAudio2MasteringVoice::DestroyVoice));

#if (_WIN32_WINNT <= _WIN32_WINNT_WIN7)
		XAUDIO2_DEVICE_DETAILS details;
#ifdef KLAYGE_COMPILER_GCC
		com_ptr<IXAudio27> xaudio27 = xaudio_.as<IXAudio27>();
		TIFHR(xaudio27->GetDeviceDetails(0, &details));
#else
		TIFHR(xaudio_->GetDeviceDetails(0, &details));
#endif

		DWORD channel_mask = details.OutputFormat.dwChannelMask;
		mastering_channels_ = details.OutputFormat.Format.nChannels;
#else
		DWORD channel_mask;
		TIFHR(mastering_voice_->GetChannelMask(&channel_mask));

		XAUDIO2_VOICE_DETAILS voice_details;
		mastering_voice_->GetVoiceDetails(&voice_details);
		mastering_channels_ = voice_details.InputChannels;
#endif

		TIFHR(DynamicX3DAudioInitialize_(channel_mask, X3DAUDIO_SPEED_OF_SOUND, x3d_instance_));

		this->SetListenerPos(float3(0, 0, 0));
		this->SetListenerVel(float3(0, 0, 0));
		this->SetListenerOri(float3(0, 0, 1), float3(0, 1, 0));
	}

	XAAudioEngine::~XAAudioEngine()
	{
		audio_buffs_.clear();
		mastering_voice_.reset();
		xaudio_.reset();

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		mod_xaudio2_.Free();
#endif
	}

	void XAAudioEngine::X3DAudioCalculate(X3DAUDIO_EMITTER const * emitter, uint32_t flags, X3DAUDIO_DSP_SETTINGS* dsp_settings) const
	{
		DynamicX3DAudioCalculate_(x3d_instance_, &listener_, emitter, flags, dsp_settings);
	}

	void XAAudioEngine::DoSuspend()
	{
		xaudio_->StopEngine();
	}

	void XAAudioEngine::DoResume()
	{
		TIFHR(xaudio_->StartEngine());
	}

	std::wstring const & XAAudioEngine::Name() const
	{
		static std::wstring const name(L"XAudio Audio Engine");
		return name;
	}

	float3 XAAudioEngine::GetListenerPos() const
	{
		return float3(&listener_.Position.x);
	}

	void XAAudioEngine::SetListenerPos(float3 const & v)
	{
		listener_.Position = { v.x(), v.y(), v.z() };
	}

	float3 XAAudioEngine::GetListenerVel() const
	{
		return float3(&listener_.Velocity.x);
	}

	void XAAudioEngine::SetListenerVel(float3 const & v)
	{
		listener_.Velocity = { v.x(), v.y(), v.z() };
	}

	void XAAudioEngine::GetListenerOri(float3& face, float3& up) const
	{
		face = float3(&listener_.OrientFront.x);
		up = float3(&listener_.OrientTop.x);
	}

	void XAAudioEngine::SetListenerOri(float3 const & face, float3 const & up)
	{
		listener_.OrientFront = { face.x(), face.y(), face.z() };
		listener_.OrientTop = { up.x(), up.y(), up.z() };
	}
}
