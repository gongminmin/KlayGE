option(KLAYGE_BUILD_PLUGIN_OGG_VORBIS_AUDIO_DATA_SOURCE "Build OggVorbis audio data source plugin" ON)
if(NOT KLAYGE_BUILD_PLUGIN_OGG_VORBIS_AUDIO_DATA_SOURCE)
	return()
endif()

ADD_LIBRARY(KlayGE_AudioDataSource_OggVorbis ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/OggVorbisSource.cpp
	Source/OggVorbisSource.hpp
	Source/OggVorbisSourceFactory.cpp
)

SET_TARGET_PROPERTIES(KlayGE_AudioDataSource_OggVorbis PROPERTIES
	OUTPUT_NAME KlayGE_AudioDataSource_OggVorbis${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Audio"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_AudioDataSource_OggVorbis PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_AudioDataSource_OggVorbis
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_AudioDataSource_OggVorbis
	PRIVATE
		KlayGE_Core
		vorbisfile vorbis ogg
)

ADD_DEPENDENCIES(AllInEngine KlayGE_AudioDataSource_OggVorbis)
