// OALAudioEngine.cpp
// KlayGE OpenAL音频引擎类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.7.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/AudioDataSource.hpp>

#include <cassert>

#include <KlayGE/OpenAL/OALAudio.hpp>

#pragma comment(lib, "OpenAL32.lib")

namespace KlayGE
{
	// 从AudioFormat转化为OpenAL的格式标志
	/////////////////////////////////////////////////////////////////////////////////
	ALenum Convert(AudioFormat format)
	{
		ALenum out;

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
		}

		return out;
	}

	// 从左手坐标系的Vector3转化为OpenAL的右手坐标系
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 VecToALVec(const Vector3& v)
	{
		return Vector3(v.x(), v.y(), -v.z());
	}

	// 从OpenAL的右手坐标系转化为左手坐标系的Vector3
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 ALVecToVec(const Vector3& v)
	{
		return Vector3(v.x(), v.y(), -v.z());
	}

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OALAudioEngine::OALAudioEngine()
	{
		ALCdevice* Device(alcOpenDevice(NULL));
		ALvoid* Context(alcCreateContext(Device, 0));

		alcMakeContextCurrent(Context);

		this->SetListenerPos(Vector3(0, 0, 0));
		this->SetListenerVel(Vector3(0, 0, 0));
		this->SetListenerOri(Vector3(0, 0, 1), Vector3(0, 1, 0));

		alDistanceModel(AL_INVERSE_DISTANCE);

		alDopplerFactor(1);		// 按照现实的多普勒效应
		alDopplerVelocity(343); // 以 米/秒 为单位
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	OALAudioEngine::~OALAudioEngine()
	{
		audioBufs_.clear();

		ALvoid* context(alcGetCurrentContext());
		ALCdevice* device(alcGetContextsDevice(static_cast<ALCcontext*>(context)));

		alcMakeContextCurrent(0);

		alcDestroyContext(context);
		alcCloseDevice(device);
	}

	// 音频引擎名字
	/////////////////////////////////////////////////////////////////////////////////
	const std::wstring& OALAudioEngine::Name() const
	{
		static std::wstring name(L"OpenAL Audio Engine");
		return name;
	}

	// 获取3D听者位置
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 OALAudioEngine::GetListenerPos() const
	{
		Vector3 v;
		alGetListener3f(AL_POSITION, &v.x(), &v.y(), &v.z());
		return ALVecToVec(v);
	}

	// 设置3D听者位置
	/////////////////////////////////////////////////////////////////////////////////
	void OALAudioEngine::SetListenerPos(const Vector3& v)
	{
		Vector3 alv(VecToALVec(v));
		alListener3f(AL_POSITION, alv.x(), alv.y(), alv.z());
	}

	// 获取3D听者速度
	/////////////////////////////////////////////////////////////////////////////////
	Vector3 OALAudioEngine::GetListenerVel() const
	{
		Vector3 v;
		alGetListener3f(AL_VELOCITY, &v.x(), &v.y(), &v.z());
		return ALVecToVec(v);
	}

	// 设置3D听者速度
	/////////////////////////////////////////////////////////////////////////////////
	void OALAudioEngine::SetListenerVel(const Vector3& v)
	{
		Vector3 alv(VecToALVec(v));
		alListener3f(AL_VELOCITY, alv.x(), alv.y(), alv.z());
	}

	// 获取3D听者方向
	/////////////////////////////////////////////////////////////////////////////////
	void OALAudioEngine::GetListenerOri(Vector3& face, Vector3& up) const
	{
		float v[6];
		alGetListenerfv(AL_ORIENTATION, v);
		face = ALVecToVec(Vector3(v));
		up = ALVecToVec(Vector3(&v[3]));
	}

	// 获取3D听者方向
	/////////////////////////////////////////////////////////////////////////////////
	void OALAudioEngine::SetListenerOri(const Vector3& face, const Vector3& up)
	{
		Vector3 alface(VecToALVec(face));
		Vector3 alup(VecToALVec(up));
		float v[6] = { alface.x(), alface.y(), alface.z(), alup.x(), alup.y(), alup.z() };
		alListenerfv(AL_ORIENTATION, v);
	}
}
