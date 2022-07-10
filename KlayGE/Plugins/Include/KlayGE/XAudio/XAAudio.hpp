/**
 * @file XAAudio.hpp
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

#ifndef _KLAYGE_PLUGINS_AUDIO_XA_AUDIO_HPP
#define _KLAYGE_PLUGINS_AUDIO_XA_AUDIO_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Thread.hpp>

#include <vector>
#include <windows.h>
#include <KlayGE/SALWrapper.hpp>
#include <xaudio2.h>

#ifdef KLAYGE_COMPILER_GCC
#define X3DAUDIO_SPEED_OF_SOUND 343.5f
#define X3DAUDIO_CALCULATE_MATRIX 0x00000001
#define X3DAUDIO_CALCULATE_DOPPLER 0x00000020

typedef uint8_t X3DAUDIO_HANDLE[20];

struct X3DAUDIO_DSP_SETTINGS
{
	float* pMatrixCoefficients;
	float* pDelayTimes;
	uint32_t SrcChannelCount;
	uint32_t DstChannelCount;

	float LPFDirectCoefficient;
	float LPFReverbCoefficient;
	float ReverbLevel;
	float DopplerFactor;
	float EmitterToListenerAngle;

	float EmitterToListenerDistance;
	float EmitterVelocityComponent;
	float ListenerVelocityComponent;
};

struct X3DAUDIO_VECTOR
{
	float x;
	float y;
	float z;
};

struct X3DAUDIO_CONE
{
	float InnerAngle;
	float OuterAngle;

	float InnerVolume;
	float OuterVolume;
	float InnerLPF;
	float OuterLPF;
	float InnerReverb;
	float OuterReverb;
};

struct X3DAUDIO_DISTANCE_CURVE_POINT
{
	float Distance;
	float DSPSetting;
};

struct X3DAUDIO_DISTANCE_CURVE
{
	X3DAUDIO_DISTANCE_CURVE_POINT* pPoints;
	uint32_t PointCount;
};

struct X3DAUDIO_EMITTER
{
	X3DAUDIO_CONE* pCone;
	X3DAUDIO_VECTOR OrientFront;
	X3DAUDIO_VECTOR OrientTop;

	X3DAUDIO_VECTOR Position;
	X3DAUDIO_VECTOR Velocity;

	float InnerRadius;
	float InnerRadiusAngle;

	uint32_t ChannelCount;
	float ChannelRadius;
	float* pChannelAzimuths;

	X3DAUDIO_DISTANCE_CURVE* pVolumeCurve;
	X3DAUDIO_DISTANCE_CURVE* pLFECurve;
	X3DAUDIO_DISTANCE_CURVE* pLPFDirectCurve;
	X3DAUDIO_DISTANCE_CURVE* pLPFReverbCurve;
	X3DAUDIO_DISTANCE_CURVE* pReverbCurve;

	float CurveDistanceScaler;
	float DopplerScaler;
};

struct X3DAUDIO_LISTENER
{
	X3DAUDIO_VECTOR OrientFront;
	X3DAUDIO_VECTOR OrientTop;

	X3DAUDIO_VECTOR Position;
	X3DAUDIO_VECTOR Velocity;

	X3DAUDIO_CONE* pCone;
};
#else
#if defined(KLAYGE_COMPILER_MSVC) && (_MSC_VER >= 1929)
#pragma warning(push)
#pragma warning(disable : 5246) // Turn of warnings of initializing X3DAudioDefault_LinearCurvePoints
#endif
#include <x3daudio.h>
#if defined(KLAYGE_COMPILER_MSVC) && (_MSC_VER >= 1929)
#pragma warning(pop)
#endif
#endif

#if (_WIN32_WINNT <= _WIN32_WINNT_WIN7)
#define XAUDIO2_DLL_A  "xaudio2_7.dll"
#define XAUDIO2_DLL_W L"xaudio2_7.dll"
#endif

#include <KFL/com_ptr.hpp>
#include <KFL/DllLoader.hpp>
#include <KlayGE/Audio.hpp>

namespace KlayGE
{
	typedef std::shared_ptr<IXAudio2SourceVoice> IXAudio2SourceVoicePtr;

	WAVEFORMATEX WaveFormatEx(AudioDataSourcePtr const & data_source);

	class XASoundBuffer final : public SoundBuffer
	{
	public:
		XASoundBuffer(AudioDataSourcePtr const & data_source, uint32_t num_sources, float volume);
		~XASoundBuffer() override;

		void Play(bool loop = false) override;
		void Stop() override;

		void Volume(float vol) override;

		bool IsPlaying() const override;

		float3 Position() const override;
		void Position(float3 const & v) override;
		float3 Velocity() const override;
		void Velocity(float3 const & v) override;
		float3 Direction() const override;
		void Direction(float3 const & v) override;

	private:
		struct SourceVoice
		{
			IXAudio2SourceVoicePtr voice;
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#if KLAYGE_COMPILER_VERSION >= 142
#pragma warning(disable: 5205) // IXAudio2VoiceCallback doesn't have virtual destructor
#endif
#endif
			std::unique_ptr<IXAudio2VoiceCallback> voice_call_back;
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
			X3DAUDIO_DSP_SETTINGS dsp_settings;
			std::vector<float> output_matrix;
			bool is_playing;
		};

	private:
		void DoReset() override;
		SourceVoice& FreeSource();

	private:
		std::vector<uint8_t> audio_data_;
		std::vector<SourceVoice> sources_;

		float3 pos_;
		float3 vel_;
		float3 dir_;
	};

	class XAMusicBuffer final : public MusicBuffer
	{
	public:
		XAMusicBuffer(AudioDataSourcePtr const & data_source, uint32_t buffer_seconds, float volume);
		~XAMusicBuffer() override;

		void Volume(float vol) override;

		bool IsPlaying() const override;

		float3 Position() const override;
		void Position(float3 const & v) override;
		float3 Velocity() const override;
		void Velocity(float3 const & v) override;
		float3 Direction() const override;
		void Direction(float3 const & v) override;

	private:
		void LoopUpdateBuffer();

		void DoReset() override;
		void DoPlay(bool loop) override;
		void DoStop() override;

		bool FillData(uint32_t size);

	private:
		IXAudio2SourceVoicePtr source_voice_;
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#if KLAYGE_COMPILER_VERSION >= 142
#pragma warning(disable: 5205) // IXAudio2VoiceCallback doesn't have virtual destructor
#endif
#endif
		std::unique_ptr<IXAudio2VoiceCallback> voice_call_back_;
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
		std::vector<uint8_t> audio_data_;
		uint32_t buffer_size_;
		uint32_t buffer_count_;
		uint32_t curr_buffer_index_;

		bool loop_;

		bool played_;
		bool stopped_;
		std::condition_variable play_cond_;
		std::mutex play_mutex_;
		std::future<void> play_thread_;

		X3DAUDIO_EMITTER emitter_;
		X3DAUDIO_DSP_SETTINGS dsp_settings_;
		std::vector<float> output_matrix_;
	};

	class XAAudioEngine final : public AudioEngine
	{
	public:
		XAAudioEngine();
		~XAAudioEngine() override;

		IXAudio2* XAudio() const
		{
			return xaudio_.get();
		}

		IXAudio2MasteringVoice* MasteringVoice() const
		{
			return mastering_voice_.get();
		}
		uint32_t MasteringVoiceChannels() const
		{
			return mastering_channels_;
		}

		void X3DAudioCalculate(X3DAUDIO_EMITTER const * emitter, uint32_t flags, X3DAUDIO_DSP_SETTINGS* dsp_settings) const;

		std::wstring const & Name() const override;

		float3 GetListenerPos() const override;
		void SetListenerPos(float3 const & v) override;
		float3 GetListenerVel() const override;
		void SetListenerVel(float3 const & v) override;
		void GetListenerOri(float3& face, float3& up) const override;
		void SetListenerOri(float3 const & face, float3 const & up) override;

	private:
		void DoSuspend() override;
		void DoResume() override;

	private:
		com_ptr<IXAudio2> xaudio_;
		std::shared_ptr<IXAudio2MasteringVoice>	mastering_voice_;
		uint32_t mastering_channels_;

		X3DAUDIO_HANDLE x3d_instance_;
		X3DAUDIO_LISTENER listener_{};

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		DllLoader mod_xaudio2_;
#endif
		typedef HRESULT (WINAPI *XAudio2CreateFunc)(IXAudio2** ppXAudio2, UINT32 flags, XAUDIO2_PROCESSOR XAudio2Processor);
		XAudio2CreateFunc DynamicXAudio2Create_;
		typedef HRESULT (WINAPI *X3DAudioInitializeFunc)(UINT32 SpeakerChannelMask, float SpeedOfSound, X3DAUDIO_HANDLE Instance);
		X3DAudioInitializeFunc DynamicX3DAudioInitialize_;
		typedef void (WINAPI *X3DAudioCalculateFunc)(const X3DAUDIO_HANDLE Instance, const X3DAUDIO_LISTENER* pListener,
			const X3DAUDIO_EMITTER* pEmitter, UINT32 Flags, X3DAUDIO_DSP_SETTINGS* pDSPSettings);
		X3DAudioCalculateFunc DynamicX3DAudioCalculate_;
	};
}

#endif		// _KLAYGE_PLUGINS_AUDIO_XA_AUDIO_HPP
