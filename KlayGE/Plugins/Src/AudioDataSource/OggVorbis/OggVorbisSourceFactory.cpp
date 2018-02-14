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

		virtual void DoSuspend() override
		{
			// TODO
		}
		virtual void DoResume() override
		{
			// TODO
		}
	};
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeAudioDataSourceFactory(std::unique_ptr<KlayGE::AudioDataSourceFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::OggVorbisAudioDataSourceFactory>();
	}
}
