// OALAudioFactory.cpp
// KlayGE OpenAL音频引擎抽象工厂类 实现文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 改为template实现 (2004.3.4)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/AudioFactory.hpp>

#include <KlayGE/OpenAL/OALAudio.hpp>
#include <KlayGE/OpenAL/OALAudioFactory.hpp>

extern "C"
{
	void MakeAudioFactory(KlayGE::AudioFactoryPtr& ptr, boost::program_options::variables_map const & /*vm*/)
	{
		ptr = KlayGE::MakeSharedPtr<KlayGE::ConcreteAudioFactory<KlayGE::OALAudioEngine,
			KlayGE::OALSoundBuffer, KlayGE::OALMusicBuffer> >(L"OpenAL Audio Factory");
	}

	bool Match(char const * name, char const * compiler)
	{
		std::string cur_compiler_str = KLAYGE_COMPILER_TOOLSET;
#ifdef KLAYGE_DEBUG
		cur_compiler_str += "_d";
#endif

		if ((std::string("OpenAL") == name) && (cur_compiler_str == compiler))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
