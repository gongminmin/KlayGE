// OggVorbisSourceFactory.cpp
// KlayGE Ogg vorbis audio data source implement file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.8.22)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/OggVorbis/OggVorbisSource.hpp>
#include <KlayGE/OggVorbis/OggVorbisSourceFactory.hpp>

namespace KlayGE
{
	class OggVorbisAudioDataSourceFactory : public AudioDataSourceFactory
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"OggVorbis Audio Data Source Factory");
			return name;
		}

	private:
		AudioDataSourcePtr MakeAudioDataSource()
		{
			return MakeSharedPtr<OggVorbisSource>();
		}

		virtual void DoSuspend() KLAYGE_OVERRIDE
		{
			// TODO
		}
		virtual void DoResume() KLAYGE_OVERRIDE
		{
			// TODO
		}
	};
}

void MakeAudioDataSourceFactory(KlayGE::AudioDataSourceFactoryPtr& ptr)
{
	ptr = KlayGE::MakeSharedPtr<KlayGE::OggVorbisAudioDataSourceFactory>();
}
