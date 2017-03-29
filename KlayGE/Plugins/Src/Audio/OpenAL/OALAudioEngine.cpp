// OALAudioEngine.cpp
// KlayGE OpenAL��Ƶ������ ʵ���ļ�
// Ver 2.0.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.7.7)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OpenAL/OALAudio.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "OpenAL32.lib")
#endif

namespace KlayGE
{
	// ��AudioFormatת��ΪOpenAL�ĸ�ʽ��־
	/////////////////////////////////////////////////////////////////////////////////
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
			BOOST_ASSERT(false);
			break;
		}

		return out;
	}

	// ����������ϵ��float3ת��ΪOpenAL����������ϵ
	/////////////////////////////////////////////////////////////////////////////////
	float3 VecToALVec(float3 const & v)
	{
		return float3(v.x(), v.y(), -v.z());
	}

	// ��OpenAL����������ϵת��Ϊ��������ϵ��float3
	/////////////////////////////////////////////////////////////////////////////////
	float3 ALVecToVec(float3 const & v)
	{
		return float3(v.x(), v.y(), -v.z());
	}

	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	OALAudioEngine::OALAudioEngine()
	{
		ALCdevice* device(alcOpenDevice(nullptr));
		ALCcontext* context(alcCreateContext(device, 0));

		alcMakeContextCurrent(context);

		this->SetListenerPos(float3(0, 0, 0));
		this->SetListenerVel(float3(0, 0, 0));
		this->SetListenerOri(float3(0, 0, 1), float3(0, 1, 0));

		alDistanceModel(AL_INVERSE_DISTANCE);

		alDopplerFactor(1);		// ������ʵ�Ķ�����ЧӦ
		alDopplerVelocity(343); // �� ��/�� Ϊ��λ
	}

	// ��������
	/////////////////////////////////////////////////////////////////////////////////
	OALAudioEngine::~OALAudioEngine()
	{
		audioBufs_.clear();

		ALCcontext* context(alcGetCurrentContext());
		ALCdevice* device(alcGetContextsDevice(context));

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

	// ��Ƶ��������
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & OALAudioEngine::Name() const
	{
		static std::wstring const name(L"OpenAL Audio Engine");
		return name;
	}

	// ��ȡ3D����λ��
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALAudioEngine::GetListenerPos() const
	{
		float3 v;
		alGetListener3f(AL_POSITION, &v.x(), &v.y(), &v.z());
		return ALVecToVec(v);
	}

	// ����3D����λ��
	/////////////////////////////////////////////////////////////////////////////////
	void OALAudioEngine::SetListenerPos(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alListener3f(AL_POSITION, alv.x(), alv.y(), alv.z());
	}

	// ��ȡ3D�����ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	float3 OALAudioEngine::GetListenerVel() const
	{
		float3 v;
		alGetListener3f(AL_VELOCITY, &v.x(), &v.y(), &v.z());
		return ALVecToVec(v);
	}

	// ����3D�����ٶ�
	/////////////////////////////////////////////////////////////////////////////////
	void OALAudioEngine::SetListenerVel(float3 const & v)
	{
		float3 alv(VecToALVec(v));
		alListener3f(AL_VELOCITY, alv.x(), alv.y(), alv.z());
	}

	// ��ȡ3D���߷���
	/////////////////////////////////////////////////////////////////////////////////
	void OALAudioEngine::GetListenerOri(float3& face, float3& up) const
	{
		float v[6];
		alGetListenerfv(AL_ORIENTATION, v);
		face = ALVecToVec(float3(v));
		up = ALVecToVec(float3(&v[3]));
	}

	// ��ȡ3D���߷���
	/////////////////////////////////////////////////////////////////////////////////
	void OALAudioEngine::SetListenerOri(float3 const & face, float3 const & up)
	{
		float3 alface(VecToALVec(face));
		float3 alup(VecToALVec(up));
		float v[6] = { alface.x(), alface.y(), alface.z(), alup.x(), alup.y(), alup.z() };
		alListenerfv(AL_ORIENTATION, v);
	}
}
