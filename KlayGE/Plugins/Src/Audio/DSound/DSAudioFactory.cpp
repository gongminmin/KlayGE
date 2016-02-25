// DSAudioFactory.cpp
// KlayGE DSound��Ƶ������󹤳��� ʵ���ļ�
// Ver 2.0.0
// ��Ȩ����(C) ������, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// ���ν��� (2003.10.4)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/AudioFactory.hpp>

#include <KlayGE/DSound/DSAudio.hpp>
#include <KlayGE/DSound/DSAudioFactory.hpp>

void MakeAudioFactory(std::unique_ptr<KlayGE::AudioFactory>& ptr)
{
	ptr = KlayGE::MakeUniquePtr<KlayGE::ConcreteAudioFactory<KlayGE::DSAudioEngine,
		KlayGE::DSSoundBuffer, KlayGE::DSMusicBuffer>>(L"DirectSound Audio Factory");
}
