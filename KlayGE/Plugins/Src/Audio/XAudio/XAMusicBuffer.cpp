/**
 * @file XAMusicBuffer.hpp
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

#include <boost/assert.hpp>

#include <KlayGE/XAudio/XAAudio.hpp>

namespace KlayGE
{
	class MusicVoiceContext : public IXAudio2VoiceCallback
	{
	public:
		MusicVoiceContext()
			: buffer_end_event_(::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS))
		{
		}
		virtual ~MusicVoiceContext()
		{
			::CloseHandle(buffer_end_event_);
		}

		HANDLE GetBufferEndEvent() const
		{
			return buffer_end_event_;
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
		}
		STDMETHOD_(void, OnBufferEnd)(void*)
		{
			::SetEvent(buffer_end_event_);
		}
		STDMETHOD_(void, OnLoopEnd)(void*)
		{
		}
		STDMETHOD_(void, OnVoiceError)(void*, HRESULT)
		{
		}

	private:
		HANDLE buffer_end_event_;
	};

	XAMusicBuffer::XAMusicBuffer(AudioDataSourcePtr const & data_source, uint32_t buffer_seconds, float volume)
					: MusicBuffer(data_source),
						voice_call_back_(MakeUniquePtr<MusicVoiceContext>()),
						buffer_count_(buffer_seconds * BUFFERS_PER_SECOND), curr_buffer_index_(0),
						played_(false), stopped_(true),
						emitter_{}, dsp_settings_{}
	{
		WAVEFORMATEX wfx = WaveFormatEx(data_source);
		audio_data_.resize(wfx.nAvgBytesPerSec * buffer_seconds);
		buffer_size_ = wfx.nAvgBytesPerSec / BUFFERS_PER_SECOND;

		auto const & ae = *checked_cast<XAAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance());

		auto xaudio = ae.XAudio();

		IXAudio2SourceVoice* source_voice;
		TIFHR(xaudio->CreateSourceVoice(&source_voice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voice_call_back_.get(), nullptr, nullptr));
		source_voice_ = std::shared_ptr<IXAudio2SourceVoice>(source_voice, std::mem_fn(&IXAudio2SourceVoice::DestroyVoice));

		emitter_.ChannelCount = 1;
		emitter_.CurveDistanceScaler = FLT_MIN;
		emitter_.OrientTop = { 0, 1, 0 };

		output_matrix_.resize(ae.MasteringVoiceChannels());
		dsp_settings_.SrcChannelCount = 1;
		dsp_settings_.DstChannelCount = ae.MasteringVoiceChannels();
		dsp_settings_.pMatrixCoefficients = output_matrix_.data();

		this->Position(float3::Zero());
		this->Velocity(float3::Zero());
		this->Direction(float3::Zero());

		this->Volume(volume);

		this->Reset();
	}

	XAMusicBuffer::~XAMusicBuffer()
	{
		this->Stop();
	}

	void XAMusicBuffer::LoopUpdateBuffer()
	{
		std::unique_lock<std::mutex> lock(play_mutex_);
		while (!played_)
		{
			play_cond_.wait(lock);
		}
		played_ = false;

		while (!stopped_)
		{
			XAUDIO2_VOICE_STATE state;
			for (;;)
			{
				source_voice_->GetState(&state);
				if (state.BuffersQueued < buffer_count_ - 1)
				{
					break;
				}

				::WaitForSingleObjectEx(checked_cast<MusicVoiceContext*>(voice_call_back_.get())->GetBufferEndEvent(),
					INFINITE, FALSE);
			}

			if (this->FillData(buffer_size_))
			{
				if (loop_)
				{
					stopped_ = false;
					this->DoReset();
				}
				else
				{
					stopped_ = true;

					HRESULT hr = source_voice_->Stop();
					if (SUCCEEDED(hr))
					{
						source_voice_->FlushSourceBuffers();
					}
				}
			}
		}
	}

	void XAMusicBuffer::DoReset()
	{
		data_source_->Reset();
	}

	void XAMusicBuffer::DoPlay(bool loop)
	{
		auto const & ae = *checked_cast<XAAudioEngine const *>(&Context::Instance().AudioFactoryInstance().AudioEngineInstance());

		ae.X3DAudioCalculate(&emitter_, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dsp_settings_);

		source_voice_->SetOutputMatrix(ae.MasteringVoice(), 1, ae.MasteringVoiceChannels(), dsp_settings_.pMatrixCoefficients);
		source_voice_->SetFrequencyRatio(dsp_settings_.DopplerFactor);

		curr_buffer_index_ = 0;
		loop_ = loop;

		play_thread_ = Context::Instance().ThreadPool()([this] { this->LoopUpdateBuffer(); });

		stopped_ = false;
		{
			std::lock_guard<std::mutex> lock(play_mutex_);
			played_ = true;
		}
		play_cond_.notify_one();

		source_voice_->Start(0, 0);
	}

	void XAMusicBuffer::DoStop()
	{
		if (!stopped_)
		{
			stopped_ = true;
			::SetEvent(checked_cast<MusicVoiceContext*>(voice_call_back_.get())->GetBufferEndEvent());
			play_thread_();
		}

		HRESULT hr = source_voice_->Stop();
		if (SUCCEEDED(hr))
		{
			hr = source_voice_->FlushSourceBuffers();
		}
	}

	bool XAMusicBuffer::FillData(uint32_t size)
	{
		bool ret = false;

		size_t read_size = data_source_->Read(&audio_data_[curr_buffer_index_ * buffer_size_], size);
		XAUDIO2_BUFFER buf{};
		buf.AudioBytes = static_cast<uint32_t>(read_size);
		buf.pAudioData = &audio_data_[curr_buffer_index_ * buffer_size_];
		if (curr_buffer_index_ * buffer_size_ + read_size >= data_source_->Size())
		{
			buf.Flags = XAUDIO2_END_OF_STREAM;
			ret = true;
		}

		source_voice_->SubmitSourceBuffer(&buf);
		curr_buffer_index_ = (curr_buffer_index_ + 1) % buffer_count_;

		return ret;
	}

	bool XAMusicBuffer::IsPlaying() const
	{
		if (source_voice_)
		{
			XAUDIO2_VOICE_STATE state;
			source_voice_->GetState(&state);
			return (state.BuffersQueued > 0);
		}

		return false;
	}

	void XAMusicBuffer::Volume(float vol)
	{
		source_voice_->SetVolume(vol);
	}

	float3 XAMusicBuffer::Position() const
	{
		return float3(&emitter_.Position.x);
	}

	void XAMusicBuffer::Position(float3 const & v)
	{
		emitter_.Position = { v.x(), v.y(), v.z() };
	}

	float3 XAMusicBuffer::Velocity() const
	{
		return float3(&emitter_.Velocity.x);
	}

	void XAMusicBuffer::Velocity(float3 const & v)
	{
		emitter_.Velocity = { v.x(), v.y(), v.z() };
	}

	float3 XAMusicBuffer::Direction() const
	{
		return float3(&emitter_.OrientFront.x);
	}

	void XAMusicBuffer::Direction(float3 const & v)
	{
		emitter_.OrientFront = { v.x(), v.y(), v.z() };
	}
}
