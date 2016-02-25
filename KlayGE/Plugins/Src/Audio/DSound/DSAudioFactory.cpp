// DSAudioFactory.cpp
// KlayGE DSound音频引擎抽象工厂类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.klayge.org
//
// 2.0.0
// 初次建立 (2003.10.4)
//
// 修改记录
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
