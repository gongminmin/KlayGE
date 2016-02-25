// OALAudioFactory.cpp
// KlayGE OpenAL��Ƶ������󹤳��� ʵ���ļ�
// Ver 2.0.3
// ��Ȩ����(C) ������, 2003-2004
// Homepage: http://www.klayge.org
//
// 2.0.3
// ��Ϊtemplateʵ�� (2004.3.4)
//
// 2.0.0
// ���ν��� (2003.8.15)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/AudioFactory.hpp>

#include <KlayGE/OpenAL/OALAudio.hpp>
#include <KlayGE/OpenAL/OALAudioFactory.hpp>

void MakeAudioFactory(std::unique_ptr<KlayGE::AudioFactory>& ptr)
{
	ptr = KlayGE::MakeUniquePtr<KlayGE::ConcreteAudioFactory<KlayGE::OALAudioEngine,
		KlayGE::OALSoundBuffer, KlayGE::OALMusicBuffer>>(L"OpenAL Audio Factory");
}
