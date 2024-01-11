option(KLAYGE_BUILD_PLUGIN_NULL_AUDIO_DATA_SOURCE "Build Null audio data source plugin" ON)
if(NOT KLAYGE_BUILD_PLUGIN_NULL_AUDIO_DATA_SOURCE)
	return()
endif()

ADD_LIBRARY(KlayGE_AudioDataSource_NullAudioDataSource ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/NullSource.cpp
	Source/NullSource.hpp
	Source/NullSourceFactory.cpp
)

SET_TARGET_PROPERTIES(KlayGE_AudioDataSource_NullAudioDataSource PROPERTIES
	OUTPUT_NAME KlayGE_AudioDataSource_NullAudioDataSource${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Audio"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_AudioDataSource_NullAudioDataSource PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_AudioDataSource_NullAudioDataSource
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_AudioDataSource_NullAudioDataSource
	PRIVATE
		KlayGE_Core
)

ADD_DEPENDENCIES(AllInEngine KlayGE_AudioDataSource_NullAudioDataSource)
