/**
 * @file OALAudioEngine.cpp
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
#include <KlayGE/AudioDataSource.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OpenAL/OALAudio.hpp>

namespace KlayGE
{
	ALenum Convert(AudioFormat format)
	{
		ALenum out = AL_FORMAT_MONO8;

		switch (format)
		{
		case AF_Mono8:
			out = AL_FORMAT_MONO8;
			break;

		case AF_Mono16:
			out = AL_FORMAT_MONO16;
			break;

		case AF_Stereo8:
			out = AL_FORMAT_STEREO8;
			break;

		case AF_Stereo16:
			out = AL_FORMAT_STEREO16;
			break;

		default:
			KFL_UNREACHABLE("Invalid audio format");
		}

		return out;
	}

	float3 VecToALVec(float3 const & v)
	{
		return float3(v.x(), v.y(), -v.z());
	}

	float3 ALVecToVec(float3 const & v)
	{
		return float3(v.x(), v.y(), -v.z());
	}

	OALAudioEngine::OALAudioEngine()
	{
		ALCdevice* device = alcOpenDevice(nullptr);
		ALCcontext* context = alcCreateContext(device, 0);

		alcMakeContextCurrent(context);

		this->SetListenerPos(float3(0, 0, 0));
		this->SetListenerVel(float3(0, 0, 0));
		this->SetListenerOri(float3(0, 0, 1), float3(0, 1, 0));

		alDistanceModel(AL_INVERSE_DISTANCE);

		alDopplerFactor(1);
		alDopplerVelocity(343); // m/s
	}

	OALAudioEngine::~OALAudioEngine()
	{
		audio_buffs_.clear();

		ALCcontext* context = alcGetCurrentContext();
		ALCdevice* device = alcGetContextsDevice(context);

		alcMakeContextCurrent(0);

		alcDestroyContext(context);
		alcCloseDevice(device);
	}

	void OALAudioEngine::DoSuspend()
	{
		// TODO
	}

	void OALAudioEngine::DoResume()
	{
		// TODO
	}

	std::wstring const & OALAudioEngine::Name() const
	{
		static std::wstring const name(L"OpenAL Audio Engine");
		return name;
	}

	float3 OALAudioEngine::GetListenerPos() const
	{
		float3 v;
		alGetListener3f(AL_POSITION, &v[0], &v[1], &v[2]);
		return ALVecToVec(v);
	}

	void OALAudioEngine::SetListenerPos(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alListener3f(AL_POSITION, alv.x(), alv.y(), alv.z());
	}

	float3 OALAudioEngine::GetListenerVel() const
	{
		float3 v;
		alGetListener3f(AL_VELOCITY, &v[0], &v[1], &v[2]);
		return ALVecToVec(v);
	}

	void OALAudioEngine::SetListenerVel(float3 const & v)
	{
		float3 alv = VecToALVec(v);
		alListener3f(AL_VELOCITY, alv.x(), alv.y(), alv.z());
	}

	void OALAudioEngine::GetListenerOri(float3& face, float3& up) const
	{
		float v[6];
		alGetListenerfv(AL_ORIENTATION, v);
		face = ALVecToVec(float3(v));
		up = ALVecToVec(float3(&v[3]));
	}

	void OALAudioEngine::SetListenerOri(float3 const & face, float3 const & up)
	{
		float3 al_face = VecToALVec(face);
		float3 al_up = VecToALVec(up);
		float v[6] = { al_face.x(), al_face.y(), al_face.z(), al_up.x(), al_up.y(), al_up.z() };
		alListenerfv(AL_ORIENTATION, v);
	}
}
