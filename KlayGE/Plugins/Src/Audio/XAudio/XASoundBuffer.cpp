/**
 * @file XASoundBuffer.hpp
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
#include <KFL/ErrorHandling.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/AudioFactory.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <functional>
#include <random>

#include <boost/assert.hpp>

#include <KlayGE/XAudio/XAAudio.hpp>

namespace
{
	class SoundVoiceContext : public IXAudio2VoiceCallback
	{
	public:
		explicit SoundVoiceContext(bool* is_playing_holder)
			: is_playing_holder_(is_playing_holder)
		{
		}
		virtual ~SoundVoiceContext()
		{
			*is_playing_holder_ = false;
		}

		STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32)
		{
		}
		STDMETHOD_(void, OnVoiceProcessingPassEnd)()
		{
		}
		STDMETHOD_(void, OnStreamEnd)()
		{
		}
		STDMETHOD_(void, OnBufferStart)(void*)
		{
			*is_playing_holder_ = true;
		}
		STDMETHOD_(void, OnBufferEnd)(void*)
		{
			*is_playing_holder_ = false;
		}
		STDMETHOD_(void, OnLoopEnd)(void*)
		{
		}
		STDMETHOD_(void, OnVoiceError)(void*, HRESULT)
		{
		}

	private:
		bool* is_playing_holder_;
	};
}

namespace KlayGE
{
	XASoundBuffer::XASoundBuffer(AudioDataSourcePtr const & data_source,
									uint32_t num_sources, float volume)
					: SoundBuffer(data_source),
						sources_(num_sources)
	{
		WAVEFORMATEX wfx = WaveFormatEx(data_source);

		data_source_->Reset();

		audio_data_.resize(data_source_->Size());
		data_source_->Read(audio_data_.data(), audio_data_.size());

		auto const & ae = *checked_cast<XAAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance());

		auto xaudio = ae.XAudio();

		IXAudio2SourceVoice* source_voice;
		for (auto& src : sources_)
		{
			src.is_playing = false;
			src.voice_call_back = MakeUniquePtr<SoundVoiceContext>(&src.is_playing);
			TIFHR(xaudio->CreateSourceVoice(&source_voice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, src.voice_call_back.get(),
				nullptr, nullptr));
			src.voice = std::shared_ptr<IXAudio2SourceVoice>(source_voice, std::mem_fn(&IXAudio2SourceVoice::DestroyVoice));

			uint32_t mastering_channels = ae.MasteringVoiceChannels();
			src.output_matrix.resize(mastering_channels);

			src.dsp_settings = {};
			src.dsp_settings.SrcChannelCount = 1;
			src.dsp_settings.DstChannelCount = mastering_channels;
			src.dsp_settings.pMatrixCoefficients = src.output_matrix.data();
		}

		this->Position(float3(0, 0, 0));
		this->Velocity(float3(0, 0, 0));
		this->Direction(float3(0, 0, 0));

		this->Reset();

		this->Volume(volume);
	}

	XASoundBuffer::~XASoundBuffer()
	{
		this->Stop();
		sources_.clear();
	}

	XASoundBuffer::SourceVoice& XASoundBuffer::FreeSource()
	{
		BOOST_ASSERT(!sources_.empty());

		for (auto& src : sources_)
		{
			if (!src.is_playing)
			{
				return src;
			}
		}

		auto iter = sources_.begin();
		std::ranlux24_base gen;
		std::uniform_int_distribution<> dis(0, static_cast<int>(sources_.size()));
		std::advance(iter, dis(gen));

		return *iter;
	}

	void XASoundBuffer::Play(bool loop)
	{
		auto& source = this->FreeSource();
		
		{
			X3DAUDIO_EMITTER emitter{};
			emitter.ChannelCount = 1;
			emitter.CurveDistanceScaler = FLT_MIN;
			emitter.OrientFront = { dir_.x(), dir_.y(), dir_.z() };
			emitter.OrientTop = { 0, 1, 0 };
			emitter.Position = { pos_.x(), pos_.y(), pos_.z() };
			emitter.Velocity = { vel_.x(), vel_.y(), vel_.z() };

			auto const & ae = *checked_cast<XAAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance());

			ae.X3DAudioCalculate(&emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &source.dsp_settings);

			source.voice->SetOutputMatrix(ae.MasteringVoice(), 1, ae.MasteringVoiceChannels(), source.dsp_settings.pMatrixCoefficients);
			source.voice->SetFrequencyRatio(source.dsp_settings.DopplerFactor);
		}

		HRESULT hr = source.voice->Stop();
		if (SUCCEEDED(hr))
		{
			source.voice->FlushSourceBuffers();
		}

		XAUDIO2_BUFFER play_buffer{};
		play_buffer.AudioBytes = static_cast<uint32_t>(audio_data_.size());
		play_buffer.pAudioData = audio_data_.data();
		play_buffer.Flags = XAUDIO2_END_OF_STREAM;

		hr = source.voice->SubmitSourceBuffer(&play_buffer);
		if (SUCCEEDED(hr))
		{
			source.voice->Start(0, loop ? XAUDIO2_LOOP_INFINITE : XAUDIO2_COMMIT_NOW);
		}
	}

	void XASoundBuffer::Stop()
	{
		for (auto const & src : sources_)
		{
			src.voice->Stop();
		}
	}

	void XASoundBuffer::DoReset()
	{
		for (auto const & src : sources_)
		{
			HRESULT hr = src.voice->Stop();
			if (SUCCEEDED(hr))
			{
				src.voice->FlushSourceBuffers();
			}
		}
	}

	bool XASoundBuffer::IsPlaying() const
	{
		for (auto const & src : sources_)
		{
			if (src.is_playing)
			{
				return true;
			}
		}
		return false;
	}

	void XASoundBuffer::Volume(float vol)
	{
		for (auto const & src : sources_)
		{
			src.voice->SetVolume(vol);
		}
	}

	float3 XASoundBuffer::Position() const
	{
		return pos_;
	}

	void XASoundBuffer::Position(float3 const & v)
	{
		pos_ = v;
	}

	float3 XASoundBuffer::Velocity() const
	{
		return vel_;
	}

	void XASoundBuffer::Velocity(float3 const & v)
	{
		vel_ = v;
	}

	float3 XASoundBuffer::Direction() const
	{
		return dir_;
	}

	void XASoundBuffer::Direction(float3 const & v)
	{
		dir_ = v;
	}
}
