if (KLAYGE_PLATFORM_WINDOWS_STORE)
	return()
endif()

ADD_LIBRARY(KlayGE_AudioEngine_OpenAL ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/OALAudio.hpp
	Source/OALAudioEngine.cpp
	Source/OALAudioFactory.cpp
	Source/OALMusicBuffer.cpp
	Source/OALSoundBuffer.cpp
)

SET_TARGET_PROPERTIES(KlayGE_AudioEngine_OpenAL PROPERTIES
	OUTPUT_NAME KlayGE_AudioEngine_OpenAL${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Audio"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_AudioEngine_OpenAL PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_AudioEngine_OpenAL
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_AudioEngine_OpenAL
	PRIVATE
		OpenAL
		KlayGE_Core
)

ADD_DEPENDENCIES(AllInEngine KlayGE_AudioEngine_OpenAL)
