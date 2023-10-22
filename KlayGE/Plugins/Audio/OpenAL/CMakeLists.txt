if (KLAYGE_PLATFORM_WINDOWS_STORE)
	return()
endif()

SET(LIB_NAME KlayGE_AudioEngine_OpenAL)

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/OALAudio.hpp
	Source/OALAudioEngine.cpp
	Source/OALAudioFactory.cpp
	Source/OALMusicBuffer.cpp
	Source/OALSoundBuffer.cpp
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	PROJECT_LABEL ${LIB_NAME}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Audio"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(${LIB_NAME} PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

KLAYGE_ADD_PRECOMPILED_HEADER(${LIB_NAME} "${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp")

target_link_libraries(${LIB_NAME}
	PRIVATE
		OpenAL
		${KLAYGE_CORELIB_NAME}
)

ADD_DEPENDENCIES(AllInEngine ${LIB_NAME})
