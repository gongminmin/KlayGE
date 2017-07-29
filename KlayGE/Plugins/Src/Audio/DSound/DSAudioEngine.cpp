/**
 * @file DSAudioEngine.cpp
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
#include <KFL/COMPtr.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cmath>

#include <KlayGE/DSound/DSAudio.hpp>

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

	long LinearGainToDB(float vol)
	{
		long db;
		if (vol > 0)
		{
			db = static_cast<long>(2000 * std::log10(vol));
		}
		else
		{
			db = -10000;
		}

		return db;
	}


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

		IDirectSound* dsound = nullptr;
		TIFHR(DynamicDirectSoundCreate_(&DSDEVID_DefaultPlayback, &dsound, nullptr));
		dsound_ = MakeCOMPtr(dsound);

		TIFHR(dsound_->SetCooperativeLevel(::GetForegroundWindow(), DSSCL_PRIORITY));

		DSBUFFERDESC desc{};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

		IDirectSoundBuffer* ds_buff = nullptr;
		TIFHR(dsound_->CreateSoundBuffer(&desc, &ds_buff, nullptr));
		auto ds_buff_primary = MakeCOMPtr(ds_buff);

		WAVEFORMATEX wfx;
		wfx.wFormatTag		= WAVE_FORMAT_PCM;
		wfx.nChannels		= 2;
		wfx.nSamplesPerSec	= 22050;
		wfx.wBitsPerSample	= 16;
		wfx.nBlockAlign		= wfx.wBitsPerSample * wfx.nChannels / 8;
		wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize			= 0;

		TIFHR(ds_buff_primary->SetFormat(&wfx));

		IDirectSound3DListener* ds_3d_listener;
		TIFHR(ds_buff_primary->QueryInterface(IID_IDirectSound3DListener, reinterpret_cast<void**>(&ds_3d_listener)));
		ds_3d_listener_ = MakeCOMPtr(ds_3d_listener);

		this->SetListenerPos(float3(0, 0, 0));
		this->SetListenerVel(float3(0, 0, 0));
		this->SetListenerOri(float3(0, 0, 1), float3(0, 1, 0));
	}

	DSAudioEngine::~DSAudioEngine()
	{
		audio_buffs_.clear();
		ds_3d_listener_.reset();
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

	std::wstring const & DSAudioEngine::Name() const
	{
		static std::wstring const name(L"DirectSound Audio Engine");
		return name;
	}

	float3 DSAudioEngine::GetListenerPos() const
	{
		D3DVECTOR vec;
		ds_3d_listener_->GetPosition(&vec);
		return float3(vec.x, vec.y, vec.z);
	}

	void DSAudioEngine::SetListenerPos(float3 const & v)
	{
		ds_3d_listener_->SetPosition(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
	}

	float3 DSAudioEngine::GetListenerVel() const
	{
		D3DVECTOR vec;
		ds_3d_listener_->GetVelocity(&vec);
		return float3(vec.x, vec.y, vec.z);
	}

	void DSAudioEngine::SetListenerVel(float3 const & v)
	{
		ds_3d_listener_->SetVelocity(v.x(), v.y(), v.z(), DS3D_IMMEDIATE);
	}

	void DSAudioEngine::GetListenerOri(float3& face, float3& up) const
	{
		D3DVECTOR d3d_face, d3d_up;
		ds_3d_listener_->GetOrientation(&d3d_face, &d3d_up);

		face = float3(d3d_face.x, d3d_face.y, d3d_face.z);
		up = float3(d3d_up.x, d3d_up.y, d3d_up.z);
	}

	void DSAudioEngine::SetListenerOri(float3 const & face, float3 const & up)
	{
		ds_3d_listener_->SetOrientation(face.x(), face.y(), face.z(),
			up.x(), up.y(), up.z(), DS3D_IMMEDIATE);
	}
}
